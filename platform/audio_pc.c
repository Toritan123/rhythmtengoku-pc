#ifdef PLATFORM_PC
#include "audio_pc.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

// The MIDI library exposes these globals (defined in src/midi/directsound.c).
// They are volatile u32 arrays of 8-bit samples packed 4 per word.
//   gMidiPCMBufR  – right channel (or mono)
//   gMidiPCMBufL  – left channel
//   gMidiPCMBufSize32   – size in u32 words (= DMA_SAMPLE_BUFFER_SIZE / 4 = 392)
//   gMidiPCMBufWritePos – current write position in words

extern volatile uint32_t *gMidiPCMBufR;
extern volatile uint32_t *gMidiPCMBufL;
extern volatile uint32_t  gMidiPCMBufSize32;
extern volatile uint32_t  gMidiPCMBufWritePos;
// On GBA, advanced by the DMA2 interrupt handler (midi_interrupt_dma2).
// Each interrupt fires when the GBA sound FIFO requests a refill (every 16
// samples at GBA_SAMPLE_RATE), and advances by 4 words (16 bytes = 16 samples).
// Per second: GBA_SAMPLE_RATE/16 interrupts × 4 words = GBA_SAMPLE_RATE/4 words/s.
// On PC we replicate this with a time-based fractional accumulator so the
// MIDI engine produces audio at exactly GBA_SAMPLE_RATE samples/second,
// matching SDL's pull of PC_SAMPLE_RATE samples/second after resampling.
extern volatile uint32_t  gMidiPCMBufReadPos;

// GBA audio: 8-bit signed samples at 13379 Hz.  RTPC_MIX_MULT (midi_globals.c)
// can run the mixer at an integer multiple of that; gMidiDMASampleRate is the
// authority, so read it rather than assuming hardware rate.
extern uint32_t gMidiDMASampleRate;
#define GBA_SAMPLE_RATE ((double)gMidiDMASampleRate)
// PC output: 16-bit signed stereo at a standard rate
#define PC_SAMPLE_RATE  44100

// Hard ceiling on the SDL queue.  The proportional rate controller already
// bounds the depth at roughly target + (authority / gain) ~= 280 ms, so this
// only fires if something outside the loop goes wrong.  It used to be 200 ms,
// which the controller's own steady state exceeded on a device with a large
// clock error (Wine), turning a latency cost into a dropped frame — a click.
// Caps latency and prevents the queue from growing unboundedly if the game
// runs faster than real-time (e.g. turbo mode or high-fps display).
#define MAX_QUEUE_BYTES  ((uint32_t)(PC_SAMPLE_RATE / 2) * 2 * sizeof(int16_t))

// RTPC_AUDIO_DUMP=<path> writes everything queued to the device as a 44.1 kHz
// stereo s16 WAV, so output quality can be measured instead of guessed at.
static FILE *s_dump = NULL;
static uint32_t s_dump_bytes = 0;

static void dump_open(void)
{
    const char *path = getenv("RTPC_AUDIO_DUMP");
    uint8_t hdr[44] = {0};
    if (!path) return;
    s_dump = fopen(path, "wb");
    if (!s_dump) return;
    fwrite(hdr, 1, 44, s_dump);   // patched on close
}

static void dump_close(void)
{
    uint32_t rate = PC_SAMPLE_RATE, byte_rate = PC_SAMPLE_RATE * 4;
    uint32_t riff = 36 + s_dump_bytes;
    uint16_t fmt = 1, ch = 2, bits = 16, align = 4;
    uint32_t sz16 = 16;
    if (!s_dump) return;
    fseek(s_dump, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, s_dump); fwrite(&riff, 4, 1, s_dump);
    fwrite("WAVEfmt ", 1, 8, s_dump); fwrite(&sz16, 4, 1, s_dump);
    fwrite(&fmt, 2, 1, s_dump); fwrite(&ch, 2, 1, s_dump);
    fwrite(&rate, 4, 1, s_dump); fwrite(&byte_rate, 4, 1, s_dump);
    fwrite(&align, 2, 1, s_dump); fwrite(&bits, 2, 1, s_dump);
    fwrite("data", 1, 4, s_dump); fwrite(&s_dump_bytes, 4, 1, s_dump);
    fclose(s_dump);
    s_dump = NULL;
}

static SDL_AudioDeviceID s_dev  = 0;
uint32_t s_underruns = 0;   // times the SDL queue was empty at push time
static uint32_t s_drops = 0;   // frames of audio discarded by the queue cap
static uint32_t          s_last_write_pos = 0;

// Simple linear resampler state
static double s_resample_pos = 0.0;

// Time-based readPos advance accumulator.
// The GBA DMA fires at GBA_SAMPLE_RATE/16 Hz, each advancing readPos by 4 words,
// giving a net rate of GBA_SAMPLE_RATE/4 words/second.
// We track fractional words here so we hit the exact long-term rate.
static uint64_t s_perf_last = 0;   // SDL_GetPerformanceCounter at last push
static double   s_read_acc  = 0.0; // fractional word debt carried between frames

// Convert 8-bit signed GBA sample to 16-bit signed.
static inline int16_t s8_to_s16(uint8_t s8)
{
    return (int16_t)((int8_t)s8) * 256;
}

// High-resolution tap into the MIDI mixer (platform/midi_dsp.c).  Same samples
// as the packed 8-bit ring and the same clip/wrap curve, but without the 8-bit
// quantisation step.  Set RTPC_AUDIO_8BIT=1 to fall back to the packed ring for
// an A/B comparison.
extern int16_t  gPcPcmHiR[];
extern int16_t  gPcPcmHiL[];
extern uint32_t gPcPcmHiValid;

static int s_use_hires = -1;
static int s_zoh = 0;

// Last sample of the previous block, so interpolation has a valid left-hand
// neighbour at the start of each frame's data.
static double  s_q_ema        = -1.0; // smoothed queue depth, frames
static int16_t s_carry_l = 0, s_carry_r = 0;
static int     s_carry_valid = 0;
static double  s_volume      = -1.0;  // master gain, RTPC_VOLUME/100

// Fetch sample `idx` (counted from `base` in words) from whichever source is
// active, as a pair of s16.
static inline void fetch_sample(uint32_t base, uint32_t buf_size, uint32_t idx,
                                int16_t *l, int16_t *r)
{
    uint32_t word   = (base + idx / 4) % buf_size;
    uint32_t byte_n = idx & 3;

    if (s_use_hires) {
        uint32_t hi = word * 4 + byte_n;
        *l = gPcPcmHiL[hi];
        *r = gPcPcmHiR[hi];
    } else {
        *l = s8_to_s16((uint8_t)(gMidiPCMBufL[word] >> (byte_n * 8)));
        *r = s8_to_s16((uint8_t)(gMidiPCMBufR[word] >> (byte_n * 8)));
    }
}

int audio_pc_init(void)
{
    SDL_AudioSpec want = {0}, got = {0};
    want.freq     = PC_SAMPLE_RATE;
    want.format   = AUDIO_S16SYS;
    want.channels = 2; // stereo
    // Device buffer size, added to the SDL queue depth as pure output latency —
    // audible on a rhythm game as the music trailing the picture.  Measured
    // total (queue + device) over ~2500 frames each: 1024 -> 43.6 ms,
    // 512 -> 34.6 ms, 256 -> 22.7 ms.  512 is the default because it was the
    // only one of the three with no near-empty queue frame in the sample, so
    // it buys ~9 ms with no measured underrun risk.  RTPC_AUDIO_BUF overrides
    // it; 256 is worth trying by ear on a fast machine.
    want.samples  = 512;
    {
        const char *e = getenv("RTPC_AUDIO_BUF");
        if (e) {
            int n = atoi(e);
            if (n >= 64 && n <= 4096) want.samples = (Uint16)n;
        }
    }
    want.callback = NULL; // push mode

    s_dev = SDL_OpenAudioDevice(NULL, 0, &want, &got, 0);
    if (s_dev == 0) return -1;

    // Prime the queue with silence before starting playback.  Every underrun
    // measured after the production fix landed in the first four seconds, while
    // the queue was still filling and the boot frames were slow; starting with
    // a full cushion removes them.
    {
        // 60 ms, not the 22 ms steady-state target: the first couple of
        // seconds include slow asset-loading frames that push no audio, and
        // the rate matcher has not learned the device's clock yet.  The extra
        // cushion is drained back down to target within about a second.
        uint32_t frames = (uint32_t)(PC_SAMPLE_RATE * 60 / 1000);
        int16_t *silence = (int16_t *)SDL_calloc(frames * 2, sizeof(int16_t));
        if (silence) {
            SDL_QueueAudio(s_dev, silence, frames * 2 * sizeof(int16_t));
            SDL_free(silence);
        }
    }
    if (getenv("RTPC_AUDIO_STATS"))
        fprintf(stderr, "[AUD] device: driver=%s freq=%d fmt=0x%04x ch=%d samples=%d silence=%d size=%u\n",
                SDL_GetCurrentAudioDriver(), got.freq, (unsigned)got.format,
                got.channels, got.samples, got.silence, (unsigned)got.size);
    SDL_PauseAudioDevice(s_dev, 0);
    dump_open();
    s_last_write_pos = 0;
    s_resample_pos   = 0.0;
    s_carry_valid    = 0;
    s_q_ema          = -1.0;
    s_perf_last      = 0;
    s_read_acc       = 0.0;
    return 0;
}

// Steady-state depth of the SDL output queue, in milliseconds.  This is the
// delay between a sample being generated and the player hearing it, which on a
// rhythm game is the offset between the music and what is on screen.
unsigned int audio_pc_underruns(void)
{
    return s_underruns;
}

double audio_pc_queued_ms(void)
{
    if (!s_dev) return 0.0;
    return (double)SDL_GetQueuedAudioSize(s_dev) * 1000.0
         / ((double)PC_SAMPLE_RATE * 2.0 * sizeof(int16_t));
}

void audio_pc_destroy(void)
{
    dump_close();
    if (s_dev) { SDL_CloseAudioDevice(s_dev); s_dev = 0; }
}

void audio_pc_push_frame(void)
{
    if (!s_dev || !gMidiPCMBufR || !gMidiPCMBufL) return;

    uint32_t write_pos = gMidiPCMBufWritePos;
    uint32_t buf_size  = gMidiPCMBufSize32;
    if (!buf_size) return;

    // ── 1. Advance the read cursor (simulates GBA DMA2 interrupts) ─────────────
    // Rate: GBA_SAMPLE_RATE / 4 words per second.
    // Using SDL performance counter for sub-millisecond accuracy, so the rate
    // is correct regardless of display refresh rate (60 Hz, 59.7 Hz, 144 Hz …).
    {
        // Advance by exactly one frame's worth per call, not by measured wall
        // time.  This function runs once per platform_frame_sync, and that is
        // now deadline-paced to 59.7275 Hz, so the frame counter is the better
        // clock: with measured time, a frame that came in a little short left
        // the MIDI engine with nothing to do (it only tops the ring up to
        // wordsPerFrame ahead of readPos), so it wrote *zero* words that frame
        // and double the next.  That +-1 frame swing in queue depth was what
        // drained the queue to empty and produced the crackle — 8 such frames
        // in 1737 measured, which also matched the 0.5 % production shortfall.
        s_read_acc += (double)GBA_SAMPLE_RATE / 4.0 / 59.7275;
        uint32_t advance = (uint32_t)s_read_acc;
        s_read_acc -= (double)advance;

        if (advance > 0) {
            uint32_t nr = gMidiPCMBufReadPos + advance;
            // Wrap within circular buffer.
            while (nr >= buf_size) nr -= buf_size;
            gMidiPCMBufReadPos = nr;
        }
    }

    // ── 2. Compute how many new u32 words the MIDI engine wrote this frame ──────
    uint32_t prev_write_pos = s_last_write_pos;
    uint32_t avail;
    if (write_pos >= prev_write_pos)
        avail = write_pos - prev_write_pos;
    else
        avail = buf_size - prev_write_pos + write_pos;

    s_last_write_pos = write_pos;

    if (avail == 0) {
        if (getenv("RTPC_AUDIO_STATS")) fprintf(stderr, "[AUD] avail=0 (nothing pushed)\n");
        return;
    }

    // ── 3. SDL queue size guard ─────────────────────────────────────────────────
    // If there is already ≥ 200 ms of audio queued, skip pushing this frame.
    // This caps latency and prevents unbounded queue growth when the game
    // temporarily runs faster than real-time (initial burst, turbo, etc.).
    if (SDL_GetQueuedAudioSize(s_dev) == 0) {
        // The device ran dry before this push: an audible gap.
        s_underruns++;
        if (getenv("RTPC_AUDIO_STATS"))
            fprintf(stderr, "[AUD] underrun #%u\n", s_underruns);
    }
    if (SDL_GetQueuedAudioSize(s_dev) >= MAX_QUEUE_BYTES) {
        // Dropping a frame of audio outright is also a discontinuity; count it
        // separately so it is not mistaken for an underrun.
        s_drops++;
        if (getenv("RTPC_AUDIO_STATS"))
            fprintf(stderr, "[AUD] queue-full drop #%u\n", s_drops);
        return;
    }

    // ── 4. Resample GBA samples (13379 Hz) → PC samples (44100 Hz) ─────────────
    // Each u32 holds 4 mono samples (8-bit signed).
    uint32_t gba_samples = avail * 4;

    // Pre-allocate the worst-case output buffer.  This has to allow for the
    // rate matcher below asking for more than the nominal ratio would give:
    // sizing it at the nominal ratio silently truncated the loop at 746 frames
    // whenever the correction was negative, so the block length alternated
    // between "as many as asked for" and "capped" — a several-percent swing at
    // frame rate, heard as a vibrato.  0.86 leaves margin over the +-12 % the
    // controller is allowed.
    uint32_t max_out = (uint32_t)((double)gba_samples * PC_SAMPLE_RATE
                                  / (GBA_SAMPLE_RATE * 0.86)) + 8;
    int16_t *out = (int16_t *)SDL_malloc(max_out * 2 * sizeof(int16_t));
    if (!out) return;

    uint32_t out_count = 0;

    // ── Rate matching ─────────────────────────────────────────────────────
    // The device consumes at its own clock, which never matches this code's
    // production rate exactly.  Uncorrected, the queue drifts until it empties
    // (a gap) or hits the cap (a discarded frame) — both audible.  Real
    // hardware is within ~0.01 %; Wine's emulated WASAPI measured 5.0 % slow.
    //
    // Two earlier designs failed and are worth not repeating:
    //   * A PI loop clamped to +-0.25 % could not reach Wine's 5 % offset, so
    //     the queue pinned at the cap and dropped a frame about once a second.
    //   * Deriving the ratio from measured consumption starves: once the queue
    //     empties, "consumed" is capped by what was supplied, so the loop reads
    //     its own output, lowers production further, and locks up starved.
    //
    // So: feed back on queue depth only, with an integrator to absorb any
    // clock offset, and keep every gain small enough that the ratio — which is
    // pitch — moves over seconds.  Reacting per frame to a queue quantised by
    // the device's buffer size is what produced an audible vibrato before.
    double target_ms = 30.0;
    {
        const char *e = getenv("RTPC_AUDIO_TARGET");
        if (e) { double v = atof(e); if (v >= 5.0 && v <= 200.0) target_ms = v; }
    }
    double nominal  = (double)GBA_SAMPLE_RATE / (double)PC_SAMPLE_RATE;
    double nom_out  = (double)gba_samples / nominal;   /* output frames if uncorrected */
    double step     = nominal;
    {
        double q_now  = (double)(SDL_GetQueuedAudioSize(s_dev) / 4);
        double target = target_ms * PC_SAMPLE_RATE / 1000.0;
        double lim    = nom_out * 0.12;                /* ratio authority, +-12 % */
        double err, corr;

        // The device pulls in buffer-sized chunks (512 frames = 11.6 ms), so
        // q_now is inherently jittery by about that much no matter what this
        // code does.  Feeding that jitter back is what modulates the ratio, so
        // smooth hard: the quantity actually being tracked is a clock offset,
        // which is constant over minutes.
        s_q_ema = (s_q_ema < 0.0) ? q_now : s_q_ema * 0.99 + q_now * 0.01;
        err = s_q_ema - target;                        /* + = too much queued */

        // Proportional only, deliberately.  An integrator on a loop with this
        // much lag (the depth is smoothed over ~100 frames, and the device
        // buffers 512 more) oscillated: the queue swung 0-78 ms and dragged
        // the ratio with it, which is a slow wow rather than a fix.  Without
        // one the loop cannot oscillate at all.
        //
        // The cost is a steady-state depth offset proportional to the clock
        // error.  On real hardware that error is under 0.05 %, so the queue
        // sits essentially at target.  Wine's 5 %-slow device settles it about
        // 100 ms deeper instead — still inside the 200 ms cap, so it costs
        // latency there rather than clicks.
        corr = 0.008 * err;
        if (corr >  lim) corr =  lim;
        if (corr < -lim) corr = -lim;

        {
            double want = nom_out - corr;
            if (want < 64.0) want = 64.0;
            step = (double)gba_samples / want;
        }
    }

    // Frame-start position in the ring buffer.
    uint32_t base = (write_pos >= avail) ? write_pos - avail
                                         : write_pos + buf_size - avail;

    if (s_use_hires < 0) {
        const char *e = getenv("RTPC_AUDIO_8BIT");
        const char *z = getenv("RTPC_AUDIO_ZOH");
        s_use_hires = (gPcPcmHiValid != 0) && !(e && atoi(e));
        s_zoh = (z && atoi(z)) ? 1 : 0;   // reproduce the old zero-order hold
    }

    // 13379 Hz -> 44100 Hz is a 3.3x upsample.  Taking the nearest source
    // sample (what this used to do) is a zero-order hold, which mirrors the
    // whole spectrum around 13379 Hz and back down into the audible band as
    // buzz.  Interpolating between neighbouring samples removes most of that
    // image energy for two extra multiplies per output sample.
    //
    // Interpolate *backwards*, between sample[idx-1] and sample[idx], carrying
    // the previous frame's last sample across the call.  Reading sample[idx+1]
    // instead would step one past the block the mixer just wrote, landing on
    // whatever the ring held from its previous lap — roughly seven frames of
    // stale audio — which put a discontinuity at every single frame boundary,
    // i.e. an audible 60 Hz crackle.  Reading backwards keeps every access
    // inside valid data; the cost is a constant 1-sample (75 us) delay.
    while (s_resample_pos < (double)gba_samples && out_count < max_out) {
        uint32_t idx  = (uint32_t)s_resample_pos;
        double   frac = s_resample_pos - (double)idx;
        int16_t  l0, r0, l1, r1;

        if (idx == 0) {
            if (s_carry_valid) { l0 = s_carry_l; r0 = s_carry_r; }
            else               { fetch_sample(base, buf_size, 0, &l0, &r0); }
        } else {
            fetch_sample(base, buf_size, idx - 1, &l0, &r0);
        }
        fetch_sample(base, buf_size, idx, &l1, &r1);

        if (s_zoh) { l0 = l1; r0 = r1; frac = 0.0; }
        out[out_count * 2 + 0] = (int16_t)(l0 + (int)((l1 - l0) * frac));
        out[out_count * 2 + 1] = (int16_t)(r0 + (int)((r1 - r0) * frac));
        out_count++;

        s_resample_pos += step;
    }

    // Carry the last sample of this block so the next call can interpolate
    // across the seam instead of jumping.
    fetch_sample(base, buf_size, gba_samples - 1, &s_carry_l, &s_carry_r);
    s_carry_valid = 1;
    s_resample_pos -= (double)gba_samples;
    if (s_resample_pos < 0.0) s_resample_pos = 0.0;

    // ── Master volume ─────────────────────────────────────────────────────
    // Measured over 30 s of gameplay, the game's own mix sits at -16.2 dBFS
    // RMS with peaks that genuinely touch full scale (0.007 % of samples, and
    // their neighbourhoods are loud too, so they are waveform peaks rather
    // than clipping artifacts).  That is faithful to the GBA, but there is no
    // headroom left for a plain gain, so boosting has to be soft-limited: the
    // top fifth of the range is compressed instead of clipped, which at +4 dB
    // touches well under 1 % of samples.
    if (s_volume < 0.0) {
        const char *e = getenv("RTPC_VOLUME");
        // Default 100 %: the game's own level, unaltered.  Raising it was the
        // wrong fix for "the sound is quiet" — what was actually quiet was the
        // PSG, for its own reasons.  The knob stays for anyone who wants it;
        // 160 % measures -12.8 dBFS RMS against 100 %'s -16.2, soft-limited.
        s_volume = e ? (atof(e) / 100.0) : 1.0;
        if (s_volume < 0.0)  s_volume = 0.0;
        if (s_volume > 8.0)  s_volume = 8.0;
    }

    // Mix in the PSG channels.  They are synthesised at the output rate rather
    // than the GBA's 13379 Hz DirectSound rate — there is no reason to band-limit
    // them to that, and it keeps them out of the resampler.
    if (out_count > 0) {
        extern void psg_pc_render(int16_t *out, unsigned frames, int sample_rate);
        psg_pc_render(out, out_count, PC_SAMPLE_RATE);
    }

    if (s_volume != 1.0) {
        const double knee = 32767.0 * 0.8;
        const double lim  = 32767.0 - knee;
        uint32_t n;
        for (n = 0; n < out_count * 2; n++) {
            double v = (double)out[n] * s_volume;
            double a = v < 0.0 ? -v : v;
            if (a > knee) {
                a = knee + lim * tanh((a - knee) / lim);
                v = (v < 0.0) ? -a : a;
            }
            out[n] = (int16_t)(v > 32767.0 ? 32767.0 : (v < -32768.0 ? -32768.0 : v));
        }
    }

    if (getenv("RTPC_AUDIO_STATS"))
        fprintf(stderr, "[AUD] avail=%u gba=%u out=%u q=%.1fms\n",
                avail, gba_samples, out_count, audio_pc_queued_ms());
    if (out_count > 0) {
        SDL_QueueAudio(s_dev, out, out_count * 2 * sizeof(int16_t));
        if (s_dump) {
            uint32_t n = out_count * 2 * (uint32_t)sizeof(int16_t);
            fwrite(out, 1, n, s_dump);
            s_dump_bytes += n;
        }
    }
    SDL_free(out);
}

#endif // PLATFORM_PC
