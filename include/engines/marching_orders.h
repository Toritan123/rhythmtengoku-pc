#pragma once

#include "global.h"
#include "engines.h"

#include "games/marching_orders/graphics/marching_orders_graphics.h"

// Engine Macros/Enums:
enum MarchingOrdersVersionsEnum {
    MARCHING_ORDERS_VER_0,
    MARCHING_ORDERS_VER_REMIX,
    MARCHING_ORDERS_VER_2_UNUSED,
    MARCHING_ORDERS_VER_BUNNY
};

enum MarchingOrdersAnimationsEnum {
    MARCHING_ANIM_TURN_L,
    MARCHING_ANIM_TURN_R,
    MARCHING_ANIM_POINT_L,
    MARCHING_ANIM_POINT_R,
    MARCHING_ANIM_COMMANDER,
    MARCHING_ANIM_STOP_BEAT,
    MARCHING_ANIM_BEAT,
    MARCHING_ANIM_STEP_R,
    MARCHING_ANIM_STEP_L,
    MARCHING_ANIM_HEAD_L,
    MARCHING_ANIM_HEAD_R,
    MARCHING_ANIM_TUTORIAL_ICONS,
    MARCHING_ANIM_ANGRY_PUFF,
    MARCHING_ANIM_CLAP,
    MARCHING_ANIM_COMMANDER_ANNOYED
};

enum MarchingOrdersSoundEffectsEnum {
    MARCHING_SFX_SHOUT_START,
    MARCHING_SFX_SHOUT_STOP,
    MARCHING_SFX_SHOUT_TURN,
    MARCHING_SFX_CMD_ATTENTION,
    MARCHING_SFX_CMD_MARCH,
    MARCHING_SFX_CMD_HALT,
    MARCHING_SFX_CMD_RIGHT_FACE,
    MARCHING_SFX_CMD_RIGHT_FACE_F,
    MARCHING_SFX_CMD_TURN_RIGHT,
    MARCHING_SFX_CMD_LEFT_FACE,
    MARCHING_SFX_CMD_LEFT_FACE_F,
    MARCHING_SFX_CMD_TURN_LEFT
};


// Engine Types:
// One of the four marchers.  0x0C bytes on GBA; the C layout is what matters
// here, since nothing outside the engine indexes it by offset.
struct MarchingMarcher {
    s16 sprite;      // body
    s16 headSprite;  // head, drawn on top and offset by the body's anim cel
    u8  action;      // last action passed to marching_set_action
    u16 stepTimer;   // frames since the current step started; 0 = not stepping
    u8  stepLatch;   // set once the step has run long enough to settle
};

struct MarchingOrdersEngineData {
    u8 version;                         // +0x00 index into the anim/gfx/sfx tables
    struct BitmapFontOBJ *textPrinter;  // +0x04
    struct MarchingMarcher marchers[4]; // +0x08  [3] is the player
    u8  stepFoot;                       // +0x38  which foot the player steps with next
    u16 playerBusyTimer;                // +0x3A  player action lockout, frames
    s16 commanderSprite;                // +0x3C
    u16 commanderTimer;                 // +0x3E  commander reaction lockout, frames
    s16 textSprite;                     // +0x40
    s16 tutorialSprite;                 // +0x42
    u8  scrollBg;                       // +0x44  scroll BG layer 3 every frame
    u8  usePointAnims;                  // +0x45  point instead of turn on a face command
};

struct MarchingOrdersCue {
    u8 command;   // index into marching_cue_index; set by marching_cue_spawn
};

struct MarchingSfxData {
    struct SongHeader *sound;
    u16 volume;
    s16 pitch;
};


// Engine Data:
extern const char D_0805a670[];


// Engine Definition Data:
extern struct Animation **marching_anim_table[];
extern struct CompressedData *marching_buffered_textures[];
extern struct GraphicsTable *marching_gfx_tables[];
extern struct Vector2 D_089e5368[][4];
extern struct MarchingSfxData marching_sfx_table[][12];


// Functions:
extern struct Animation *func_08034100(u32 anim); // Get Animation
extern void marching_init_gfx3(void); // Graphics Init. 3
extern void marching_init_gfx2(void); // Graphics Init. 2
extern void marching_init_gfx1(void); // Graphics Init. 1
extern void marching_engine_start(u32 version); // Game Engine Start
extern void marching_engine_event_stub(void); // Engine Event 06 (STUB)
extern void func_080343b8(struct MarchingMarcher *marcher, u32 action); // Set Marcher Action
extern void func_08034544(u32 action); // Engine Event 00 (Set Action For All Marchers)
extern void func_080345cc(struct MarchingMarcher *marcher); // Update One Marcher
extern void func_080346b0(void); // Update All Marchers
extern void func_080346e0(u32 reaction); // Engine Event 01 (Commander Reaction)
extern void func_080347c0(s32 cel); // Engine Event 02 (Show/Hide Tutorial Icon)
extern void func_0803481c(void); // Engine Event 03 (Start BG Scroll)
extern void func_0803482c(void); // Scroll BG Layer 3
extern void func_08034850(u32 usePointAnims); // Engine Event 04
extern void marching_engine_update(void); // Game Engine Update
extern void marching_engine_stop(void); // Game Engine Close
extern void marching_cue_spawn(struct Cue *, struct MarchingOrdersCue *, u32 command); // Cue - Spawn
extern u32  marching_cue_update(struct Cue *, struct MarchingOrdersCue *, u32 runningTime, u32 duration); // Cue - Update
extern void marching_cue_despawn(struct Cue *, struct MarchingOrdersCue *); // Cue - Despawn
extern void marching_cue_hit(struct Cue *, struct MarchingOrdersCue *, u32 pressed, u32 released); // Cue - Hit
extern void marching_cue_barely(struct Cue *, struct MarchingOrdersCue *, u32 pressed, u32 released); // Cue - Barely
extern void marching_cue_miss(struct Cue *, struct MarchingOrdersCue *); // Cue - Miss
extern void func_0803494c(void); // Player: step
extern void func_08034988(void); // Player: turn left
extern void func_080349ac(void); // Player: turn right
extern void func_080349d0(void); // Player: halt
extern void marching_input_event(u32 pressed, u32 released); // Input Event
extern void marching_common_beat_animation(void); // Common Event 0 (Beat Animation, Unimplemented)
extern void marching_common_display_text(const char *); // Common Event 1 (Display Text)
extern void marching_common_init_tutorial(void); // Common Event 2 (Init. Tutorial, Unimplemented)
extern void func_08034ae4(u32 sound); // Engine Event 05 (Play Sound)
