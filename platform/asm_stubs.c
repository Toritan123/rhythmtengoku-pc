/*
 * PC-side replacements for GBA BIOS calls, hand-coded ARM routines, and
 * not-yet-decompiled assembly stubs.
 *
 * All symbols here have the same signature as the originals so the existing
 * C code links and runs.  Where the original implemented real hardware logic
 * (DMA, register writes, etc.) the PC version uses standard-library calls or
 * simply no-ops.
 */
#ifdef PLATFORM_PC

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
// global.h provides u8/u16/u32/s8/s16/s32 typedefs needed by types.h
#include "global.h"
#include "include/types.h"
#include "src/code_0800b778.h"
#include "src/task_pool.h"
#include "src/code_08007468.h"
#include "src/graphics_table.h"

// ─── GBA BIOS replacements ───────────────────────────────────────────────────

// GBA BIOS Div: returns quotient; original also stores remainder in R1
// For PC callers that only use the return value this is sufficient.
s32 Div(s32 num, s32 den)
{
    if (den == 0) return 0;
    return num / den;
}

// GBA BIOS CpuFastSet: bit 24 selects fill vs. copy; lower 21 bits = word count.
void CpuFastSet(const void *src, void *dest, u32 mode)
{
    u32 words = mode & 0x1FFFFF;
    if (mode & (1u << 24)) {
        // Fill mode: replicate the first source word
        u32 fill;
        memcpy(&fill, src, 4);
        u32 *d = (u32 *)dest;
        for (u32 i = 0; i < words; i++) d[i] = fill;
    } else {
        memcpy(dest, src, words * 4);
    }
}

// ─── Fast math (GBA copies ARM Thumb code into IWRAM at runtime) ─────────────



// math_sqrt: game calls this via a function pointer set by init_math_sqrt().
static s32 pc_math_sqrt(s32 value)
{
    if (value <= 0) return 0;
    return (s32)sqrtf((float)value);
}

// math_sqrt: the game installs a pointer to this function at runtime.
s32 (*math_sqrt)(s32 value) = pc_math_sqrt;

// ROM function blob end markers (the game copies these from ROM to IWRAM)
const u8 math_sqrt_rom_end[1]             = {0};
const u8 mem_heap_alloc_block_rom_end[1]  = {0};
const u8 write_int_sram_fast_rom_end[1]   = {0};

// ─── Interrupt handler ROM stubs ─────────────────────────────────────────────
// On GBA these ROM blobs are copied to IWRAM and executed.  On PC the DmaCopy
// becomes memcpy (no-op in practice) and the handler is never called via
// hardware, so we just need valid, non-NULL symbols to satisfy the linker.
const u8 interrupt_handler_rom[0x200]     = {0};
const u8 interrupt_handler_jtbl_rom[0x38] = {0};

// Destination buffers in "IWRAM" (just regular globals on PC).
u8 interrupt_handler[0x200];
u8 interrupt_handler_jtbl[0x38];

// ─── MIDI library assembly stubs ─────────────────────────────────────────────
// The MIDI library uses a run-time function-pointer table populated by
// midi_asm_init_mode / midi_asm_init_table (lib_midi.s).
// On PC we replace the whole table with C no-ops so the audio system at
// least does not crash.  Real PCM mixing is handled by audio_pc.c which
// drains gMidiPCMBufR/L after each midi_sound_main() call.

// ─── Beatscript command processor ────────────────────────────
// func_0800cb28 lives in platform/beatscript_vm.c — a full translation of
// asm/code_0800b778/asm_0800cb28.s.  Do not add a stub for it here.

// ─── MIDI DSP functions ───────────────────────────────────────────────────────
// Implemented in platform/midi_dsp.c (full C replacements for ARM assembly).
// Do NOT define them here — midi_dsp.c provides the real implementations.

// ─── MIDI PCM buffer area (lives in IWRAM on GBA) ────────────────────────────
// Declared as extern in audio/sound_players.inc.c – provide the definition here.
// 1568 samples = 7 frames at the GBA's 13379 Hz.  Sized for the maximum
// RTPC_MIX_MULT (4x) so the ring still spans the same number of frames when
// the mixer runs above hardware rate.
u8 sPCMBufferArea[2][1568 * 4];
s32 sPCMScratchArea[0x80 * 2];

// ─── GFX decompression ───────────────────────────────────────────────────────
//
// GBA BIOS decompression format (4-byte header):
//   byte 0      – compression type: 0x10=LZ77, 0x20=Huffman, 0x30=RLE
//   bytes 1-3   – decompressed size (little-endian)
//
// LZ77 (SWI 0x11):
//   Flag byte (MSB = first unit); for each bit:
//     0 → literal byte, copy verbatim
//     1 → back-reference: 2 bytes:
//           b0 = (len-3)<<4 | disp_hi    len  = (b0>>4)+3
//           b1 = disp_lo                 disp = ((b0&0xF)<<8|b1)+1
//           copy len bytes from out[-disp]
//
// RLE (SWI 0x14):
//   Flag byte for 8 units (MSB first); for each bit:
//     0 → literal:    1-byte count+1, copy count+1 bytes verbatim
//     1 → compressed: 1-byte count+3, 1-byte value, repeat count+3 times
//
void decompress_gfx_rom(const void *src, void *dest)
{
    const u8 *in  = (const u8 *)src;
    u8       *out = (u8 *)dest;

    if (!in || !out) return;

    u8  type             = in[0] & 0xF0;
    u32 decompressed_size = (u32)in[1] | ((u32)in[2] << 8) | ((u32)in[3] << 16);
    in += 4;

    u8 *out_end = out + decompressed_size;

    if (type == 0x10) {
        /* LZ77 */
        while (out < out_end) {
            u8 flags = *in++;
            for (int bit = 7; bit >= 0 && out < out_end; bit--) {
                if (flags & (1u << bit)) {
                    /* back-reference */
                    u8  b0  = *in++;
                    u8  b1  = *in++;
                    int len  = (b0 >> 4) + 3;
                    int disp = (((b0 & 0xF) << 8) | b1) + 1;
                    u8 *ref  = out - disp;
                    for (int j = 0; j < len && out < out_end; j++)
                        *out++ = *ref++;
                } else {
                    /* literal */
                    *out++ = *in++;
                }
            }
        }
    } else if (type == 0x30) {
        /* RLE */
        while (out < out_end) {
            u8 flag = *in++;
            if (flag & 0x80) {
                /* compressed: repeat value (count+3) times */
                int count = (flag & 0x7F) + 3;
                u8  val   = *in++;
                for (int j = 0; j < count && out < out_end; j++)
                    *out++ = val;
            } else {
                /* uncompressed: copy (count+1) bytes verbatim */
                int count = (flag & 0x7F) + 1;
                for (int j = 0; j < count && out < out_end; j++)
                    *out++ = *in++;
            }
        }
    } else {
        /* Huffman or unknown – copy raw (fallback) */
        u32 copy = decompressed_size < 0x10000u ? decompressed_size : 0x10000u;
        memcpy(dest, src + 4, copy);
    }
}

// ─── SRAM (save) I/O stubs ───────────────────────────────────────────────────

// Function pointer types declared in lib_sram.h
extern void (*read_sram_fast)(const u8 *src, u8 *dest, u32 size);
extern void (*write_int_sram_fast)(const u8 *src, u8 *dest, u32 size);
extern void (*verify_sram_fast)(const u8 *src, u8 *dest, u32 size);

static void pc_read_sram_fast(const u8 *src, u8 *dest, u32 size)
{
    memcpy(dest, src, size);
}
static void pc_write_int_sram_fast(const u8 *src, u8 *dest, u32 size)
{
    memcpy(dest, src, size);
}
static void pc_verify_sram_fast(const u8 *src, u8 *dest, u32 size)
{
    (void)src; (void)dest; (void)size;
}

// Called by init_save_buffer() to install the SRAM fast-path function pointers.
void set_sram_fast_func(void)
{
    read_sram_fast      = pc_read_sram_fast;
    write_int_sram_fast = pc_write_int_sram_fast;
    verify_sram_fast    = pc_verify_sram_fast;
}

void write_sram_fast(const u8 *src, u8 *dest, u32 size)
{
    memcpy(dest, src, size);
}

void read_sram(const u8 *src, u8 *dest, u32 size)
{
    memcpy(dest, src, size);
}

// ─── Misc. asm-only functions referenced by code_08003b28.c ─────────────────

// code_08009de4.s – text-line animation builder. Real PC implementation
// lives in src/text_printer.c (#ifdef PLATFORM_PC).

// lib_0804e418 – OAM OBJ attribute writer; real implementation is in sprite_lib_pc.c
// lib_0804e938 – compression helper
/* func_0804e418 intentionally omitted here – defined in platform/sprite_lib_pc.c */
void func_0804e938(void) {}

// ─── GFX decompression cache lookup ─────────────────────────────────────────
// On GBA this walks a linked-list cache of already-decompressed textures.
// If the src is found in the cache, returns a pointer to the cached data.
// If not found, returns src unchanged.
// On PC there is no cache, so we always return src directly.
void *func_0800869c(const void *src)
{
    return (void *)src;
}

// ─── Synchronous GFX table loader (PC replacement) ───────────────────────────
// func_08002e78: synchronous version of func_08002ee0 – loads a GFX table all
// at once (large limit = 0x20000) before returning.  Used by gameplay_start_scene
// and the beatscript engine for "must-have-now" assets.
// The GBA version keeps the GfxTableLoader on the stack; we do the same.
void func_08002e78(const struct GraphicsTable *gfxTable)
{
    struct GfxTableLoader loader;
    func_08002a6c(&loader, gfxTable, 0x20000u);
    while (loader.active) {
        func_08002b10(&loader);
    }
}

// ─── Texture loader task (PC replacement) ────────────────────────────────────
// On GBA, start_new_texture_loader decompresses sprite sheets from ROM into
// VRAM over several frames using DMA.  On PC there is no VRAM, so the task
// just needs to exist long enough for the caller to call run_func_after_task,
// then complete on the very next delayed-update tick so the callback fires.
//
// This strong definition overrides the __attribute__((weak)) stub in auto_stubs.c
// and fixes every call site that does:
//   task = start_new_texture_loader(...);
//   run_func_after_task(task, next_init_func, 0);
// so that next_init_func is actually called.

static void *pc_texture_loader_start(void *inputs)
{
    (void)inputs;
    return NULL;   // no heap allocation needed
}

static u32 pc_texture_loader_update(void *info)
{
    (void)info;
    return 1;      // signal "done" immediately; task_stop fires onFinish
}

static const struct TaskMethods pc_texture_loader_methods = {
    .start         = pc_texture_loader_start,
    .delayedUpdate = pc_texture_loader_update,
    .constantUpdate = NULL,
    .stop          = NULL,
};

u32 start_new_texture_loader(u16 memID, struct CompressedData **textureList)
{
    (void)textureList;   // nothing to decompress on PC
    return (u32)start_new_task(memID, &pc_texture_loader_methods, NULL, NULL, 0);
}

#endif // PLATFORM_PC
