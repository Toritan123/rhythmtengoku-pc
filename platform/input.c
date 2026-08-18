#ifdef PLATFORM_PC
#include "input.h"
#include "gba_mem.h"
#include <stdint.h>
#include <stdlib.h>

// GBA REG_KEY offset inside IO array
#define IO_KEY 0x130

// GBA key bit positions (active-low in REG_KEY)
#define GBA_A       (1 << 0)
#define GBA_B       (1 << 1)
#define GBA_SELECT  (1 << 2)
#define GBA_START   (1 << 3)
#define GBA_RIGHT   (1 << 4)
#define GBA_LEFT    (1 << 5)
#define GBA_UP      (1 << 6)
#define GBA_DOWN    (1 << 7)
#define GBA_R       (1 << 8)
#define GBA_L       (1 << 9)

void input_init(void)
{
    // All buttons released → REG_KEY = 0x03FF
    *(volatile uint16_t *)(gba_io + IO_KEY) = 0x03FF;
}

int input_handle_event(const SDL_Event *ev)
{
    if (ev->type == SDL_QUIT) return 1;
    if (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_ESCAPE) return 1;
    return 0;
}

void input_update_reg_key(void)
{
    const uint8_t *ks = SDL_GetKeyboardState(NULL);
    uint16_t reg = 0x03FF; // start with all released

    // Keyboard mapping
    //   K / J     → A / B
    //   W A S D   → D-pad (arrow keys also work)
    //   Q / E     → L / R shoulder
    //   Enter     → START,  Backspace → SELECT
    if (ks[SDL_SCANCODE_K])          reg &= ~GBA_A;
    if (ks[SDL_SCANCODE_J])          reg &= ~GBA_B;
    if (ks[SDL_SCANCODE_BACKSPACE])  reg &= ~GBA_SELECT;
    if (ks[SDL_SCANCODE_RETURN])     reg &= ~GBA_START;
    if (ks[SDL_SCANCODE_D] || ks[SDL_SCANCODE_RIGHT]) reg &= ~GBA_RIGHT;
    if (ks[SDL_SCANCODE_A] || ks[SDL_SCANCODE_LEFT])  reg &= ~GBA_LEFT;
    if (ks[SDL_SCANCODE_W] || ks[SDL_SCANCODE_UP])    reg &= ~GBA_UP;
    if (ks[SDL_SCANCODE_S] || ks[SDL_SCANCODE_DOWN])  reg &= ~GBA_DOWN;
    if (ks[SDL_SCANCODE_E])          reg &= ~GBA_R;
    if (ks[SDL_SCANCODE_Q])          reg &= ~GBA_L;

    // TEMP (RTPC_AUTO=1): tap A every 30 frames — walks the menus and then
    // hammers cues during gameplay so the hit-detection path gets exercised.
    // RTPC_AUTO=2 additionally taps SELECT every 4 seconds, which skips the
    // tutorial loop so the main chart (and the results screen after it) is
    // reachable without a human player.
    {
        static int auto_on = -1;
        static uint32_t f = 0;
        if (auto_on < 0) { const char *e = getenv("RTPC_AUTO"); auto_on = e ? atoi(e) : 0; }
        if (auto_on) {
            f++;
            if ((f % 30) < 2) reg &= ~GBA_A;
            if (auto_on >= 2 && (f % 240) < 2) reg &= ~GBA_SELECT;
            // RTPC_AUTO=3: mash every button on a fixed pseudo-random schedule,
            // to reproduce crashes that only happen while keys are held.
            // Deterministic, so a failure can be replayed.
            if (auto_on >= 3) {
                static uint32_t r = 0x1234567u;
                r = r * 1103515245u + 12345u;
                reg &= ~((r >> 9) & 0x3FF);
            }
        }
    }

    *(volatile uint16_t *)(gba_io + IO_KEY) = reg;
}

#endif // PLATFORM_PC
