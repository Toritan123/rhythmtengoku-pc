// mgba_ref: reference screenshots from the real ROM, for comparing the PC
// port's rendering against mGBA.
//
// Boots the ROM in libmgba, waits for the warning screen, redirects the
// warning -> title scene transition to the requested scene (by rewriting the
// target of the matching entry in the scene transition table in IWRAM),
// presses A to leave the warning, and then saves a PPM every N frames.
//
//   cc -arch x86_64 -I/usr/local/include tools/mgba_ref.c -L/usr/local/lib -lmgba -o mgba_ref
//   ./mgba_ref ROM SCENE_ADDR FRAMES EVERY OUTDIR [INPUTS]
//
// SCENE_ADDR is the ROM address of a struct Scene, e.g. 0x089d2c04 for
// scene_rat_race (taken from the game select level table). INPUTS, optional,
// is a file of "frame keys" lines (keys = GBA KEYINPUT bit mask, held from
// that frame on), with frames counted from the scene switch.
//
// To compare a PC run: record it with RTPC_KEYLOG=keys.txt, find the frame
// offset from an input-free run (rat_race: PC shot n == mGBA frame n + 28),
// and add one more frame to the logged input frames (keys are logged one
// frame before the shot of the same number). Anything driven by the RNG
// will differ: the GBA spins get_agb_random_var() while it waits for
// VBlank, so its RNG advances by an amount that depends on spare CPU time.
#include <mgba/core/core.h>
#include <mgba/core/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE_SCENE_PTR 0x08935fac // D_08935fac: pointer to the title scene

static void save_ppm(const char *path, const color_t *buf, unsigned w, unsigned h)
{
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%u %u\n255\n", w, h);
    for (unsigned i = 0; i < w * h; i++) {
        uint32_t c = buf[i]; // mGBA XBGR8: 0x00BBGGRR
        unsigned char px[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
        fwrite(px, 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    if (argc < 6) {
        fprintf(stderr, "usage: %s ROM SCENE_ADDR FRAMES EVERY OUTDIR [INPUTS]\n", argv[0]);
        return 1;
    }
    const char *rom = argv[1];
    uint32_t sceneAddr = strtoul(argv[2], NULL, 0);
    int frames = atoi(argv[3]), every = atoi(argv[4]);
    const char *outdir = argv[5];

    int inFrame[4096]; uint32_t inKeys[4096]; int nIn = 0;
    if (argc > 6) {
        FILE *f = fopen(argv[6], "r");
        while (f && nIn < 4096 && fscanf(f, "%d %i", &inFrame[nIn], (int *)&inKeys[nIn]) == 2) nIn++;
        if (f) fclose(f);
    }

    struct mCore *core = mCoreFind(rom);
    if (!core || !core->init(core)) { fprintf(stderr, "no core\n"); return 1; }
    mCoreInitConfig(core, NULL);
    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    color_t *buf = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, buf, w);
    if (!mCoreLoadFile(core, rom)) { fprintf(stderr, "cannot load %s\n", rom); return 1; }
    core->reset(core);

    uint32_t title = core->busRead32(core, TITLE_SCENE_PTR);
    int switched = 0, f0 = -1;
    for (int f = 0; f < 3000 && !switched; f++) {
        core->setKeys(core, 0);
        core->runFrame(core);
        // Find {initial, target = title} in IWRAM and retarget it.
        for (uint32_t a = 0x03000000; a < 0x03007ff8; a += 4) {
            if (core->busRead32(core, a + 4) == title) {
                uint32_t initial = core->busRead32(core, a);
                if (initial >= 0x08000000 && initial < 0x0a000000) {
                    core->busWrite32(core, a + 4, sceneAddr);
                    fprintf(stderr, "frame %d: transition %08x -> title at %08x retargeted\n", f, initial, a);
                    switched = 1;
                    break;
                }
            }
        }
    }
    if (!switched) { fprintf(stderr, "transition entry not found\n"); return 1; }

    // Leave the warning screen with A, then run the scene.
    int k = 0;
    for (int f = 0; f < frames; f++) {
        uint32_t keys = 0;
        if (f0 < 0) {
            keys = ((f / 10) % 2) ? 0 : 1; // tap A until the scene changes
        } else {
            while (k + 1 < nIn && inFrame[k + 1] <= f - f0) k++;
            if (nIn && inFrame[k] <= f - f0) keys = inKeys[k];
        }
        core->setKeys(core, keys);
        core->runFrame(core);
        // The warning scene has a white screen; the first frame after the
        // switch whose VRAM changed is hard to detect generically, so key the
        // frame counter on the first non-white frame after leaving it.
        if (f0 < 0 && f > 30) {
            int nonwhite = 0;
            for (unsigned i = 0; i < w * h; i += 97) if ((buf[i] & 0xFFFFFF) != 0xF8F8F8 && (buf[i] & 0xFFFFFF) != 0xFFFFFF) nonwhite++;
            if (nonwhite > (int)(w * h / 97) * 9 / 10) { f0 = f; fprintf(stderr, "scene frames start at %d\n", f0); }
        }
        if (getenv("MGBA_REF_RNG") && f0 >= 0 && f - f0 < 40) {
            fprintf(stderr, "rel %d rng %04x\n", f - f0, core->busRead16(core, 0x030000b4));
        }
        if (f0 >= 0 && ((f - f0) % every) == 0) {
            char path[512];
            snprintf(path, sizeof(path), "%s/ref_%05d.ppm", outdir, f - f0);
            save_ppm(path, buf, w, h);
        }
    }
    core->deinit(core);
    return 0;
}
