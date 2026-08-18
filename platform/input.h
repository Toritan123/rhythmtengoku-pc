#pragma once
#ifdef PLATFORM_PC

#include <SDL2/SDL.h>
#include <stdint.h>

// Initialise the input layer.
void input_init(void);

// Process one SDL_Event.  Returns 0 normally, 1 if the user closed the window.
int  input_handle_event(const SDL_Event *ev);

// Rebuild REG_KEY from the current SDL keyboard state.
// GBA: bit = 0 when key is PRESSED.
void input_update_reg_key(void);

#endif // PLATFORM_PC
