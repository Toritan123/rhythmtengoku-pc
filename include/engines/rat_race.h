#pragma once

#include "global.h"
#include "engines.h"

#include "games/rat_race/graphics/rat_race_graphics.h"

// Engine Types:
struct Rat {
    s16 ratSprite;
    s16 sweatSprite;
    u8 unk4;
    u8 unk5;
    u32 unk8;
    u32 unkC;
};

// One of the nine dash-dust puffs. 8 bytes.
struct RatRaceDashParticle {
    s16 sprite;             // 0x00
    u8  active;             // 0x02
    u8  unk03;
    s32 x;                  // 0x04  16.8 fixed point
};

// One of the six plates on the conveyor. 8 bytes.
struct RatRacePlate {
    s16 sprite;             // 0x00
    u8  active;             // 0x02
    u8  unk03;
    s32 x;                  // 0x04  16.8 fixed point
};

// Field offsets recovered from the engine's own assembly
// (asm/engines/rat_race/). sizeof() is 0x120 on the GBA, as the placeholder
// this replaces asserted; on a 64-bit host it grows by the one pointer at
// 0x004, which shifts every offset below it by four -- so do not reuse the
// raw numbers in scripts or tables.
struct RatRaceEngineData {
    u8  version;                                // 0x000
    u8  unk001[3];
    struct BitmapFontOBJ *objFont;              // 0x004
    s16 textSprite;                             // 0x008
    u8  unk00A[2];
    s32 textX;                                  // 0x00C
    u8  unk010;                                 // 0x010
    u8  unk011;
    s16 bubbleSprite;                           // 0x012
    u8  unk014;                                 // 0x014
    u8  unk015[3];
    s32 unk018;                                 // 0x018
    u8  unk01C;                                 // 0x01C
    u8  unk01D[3];
    s32 unk020;                                 // 0x020
    s32 unk024;                                 // 0x024
    s32 unk028;                                 // 0x028
    u8  unk02C;                                 // 0x02C
    u8  unk02D[3];
    s32 unk030;                                 // 0x030
    s32 unk034;                                 // 0x034
    u8  unk038;                                 // 0x038
    u8  unk039[3];
    struct Rat rats[3];                         // 0x03C
    s16 playerLabelSprite;                      // 0x06C
    u8  unk06E[2];
    s32 unk070;                                 // 0x070
    s16 catPupilsSprite;                        // 0x074
    s16 catEyelidsSprite;                       // 0x076
    s16 catPawSprite;                           // 0x078
    s16 catPawMirrorSprite;                     // 0x07A
    u8  unk07C;                                 // 0x07C
    u8  unk07D[3];
    s32 unk080;                                 // 0x080
    s32 catX;                                   // 0x084  16.8 fixed point
    struct RatRaceDashParticle particles[9];    // 0x088
    s16 unk0D0;                                 // 0x0D0
    u8  unk0D2;                                 // 0x0D2
    u8  unk0D3;                                 // 0x0D3
    s32 unk0D4;                                 // 0x0D4
    s16 blankSprite;                            // 0x0D8
    u8  unk0DA;                                 // 0x0DA
    u8  unk0DB;
    s16 trafficLightSprite;                     // 0x0DC
    u8  unk0DE;                                 // 0x0DE
    s8  affineGroup;                            // 0x0DF
    u8  unk0E0[2];
    s16 unk0E2;                                 // 0x0E2
    s16 unk0E4;                                 // 0x0E4
    u8  unk0E6[2];
    s32 unk0E8;                                 // 0x0E8
    struct RatRacePlate plates[6];              // 0x0EC
    u8  unk11C;                                 // 0x11C
    u8  unk11D;                                 // 0x11D
    u8  unk11E;                                 // 0x11E
    u8  unk11F;
};

struct RatRaceCue {
    /* add fields here */
};


// Engine Data:
extern const char D_0805a8b4[];


// Engine Definition Data:
extern struct CompressedData *rat_race_buffered_textures[];
extern struct GraphicsTable rat_race_gfx_table[];
extern u32 D_089e66bc[];
extern u8 D_089e6834[];
extern u8 D_089e684c[];
extern struct Rat rat_race_init_rat_data[];
extern s32 D_089e68ac[];


// Functions:
extern void rat_race_init_gfx3(void); // Graphics Init. 3
extern void rat_race_init_gfx2(void); // Graphics Init. 2
extern void rat_race_init_gfx1(void); // Graphics Init. 1
extern void rat_race_engine_start(u32 version); // Game Engine Start
extern void rat_race_engine_event_stub(); // Engine Event 00 (STUB)
extern void func_0803a158(); // Engine Event 02 (?)
// extern ? func_0803a164(?);
// extern ? func_0803a198(?);
extern void func_0803a1d4(); // Engine Event 09 (?)
extern void func_0803a1e4(); // Engine Event 0A (?)
extern void func_0803a1f8(); // Engine Event 03 (?)
extern void func_0803a204(); // Engine Event 04 (?)
// extern ? func_0803a230(?);
extern void func_0803a2a8(); // Engine Event 06 (?)
extern void func_0803a350(); // Engine Event 07 (?)
extern void func_0803a3b8(); // Engine Event 08 (?)
// extern ? func_0803a3c4(?);
extern void func_0803a41c(); // Engine Event 0C (?)
extern void func_0803a434(); // Engine Event 0F (?)
extern void rat_race_engine_update(void); // Game Engine Update
extern void func_0803a47c(); // Engine Event 11 (?)
extern void func_0803a490(); // Engine Event 12 (?)
extern void rat_race_engine_stop(void); // Game Engine Stop
// extern ? func_0803a4a8(?);
extern void rat_race_cue_spawn_stop(struct Cue *, struct RatRaceCue *, u32 param); // Cue - Spawn (Stop)
extern u32  rat_race_cue_update_stop(struct Cue *, struct RatRaceCue *, u32 runningTime, u32 duration); // Cue - Update (Stop)
extern void rat_race_cue_despawn_stop(struct Cue *, struct RatRaceCue *); // Cue - Despawn (Stop)
extern void rat_race_cue_spawn_dash(struct Cue *, struct RatRaceCue *, u32 param); // Cue - Spawn (Dash)
extern u32  rat_race_cue_update_dash(struct Cue *, struct RatRaceCue *, u32 runningTime, u32 duration); // Cue - Update (Dash)
extern void rat_race_cue_despawn_dash(struct Cue *, struct RatRaceCue *); // Cue - Despawn (Dash)
extern void rat_race_cue_hit_stop(struct Cue *, struct RatRaceCue *, u32 pressed, u32 released); // Cue - Hit (Stop)
extern void rat_race_cue_hit_dash(struct Cue *, struct RatRaceCue *, u32 pressed, u32 released); // Cue - Hit (Dash)
extern void rat_race_cue_barely(struct Cue *, struct RatRaceCue *, u32 pressed, u32 released); // Cue - Barely
extern void rat_race_cue_miss(struct Cue *, struct RatRaceCue *); // Cue - Miss
extern void rat_race_input_event(u32 pressed, u32 released); // Input Event
extern void rat_race_common_beat_animation(void); // Common Event 0 (Beat Animation, Unimplemented)
extern void rat_race_common_display_text(void); // Common Event 1 (Display Text, Unimplemented)
extern void rat_race_common_init_tutorial(struct Scene *); // Common Event 2 (Init. Tutorial)
extern void func_0803a678(void); // Init. the Cat
extern void func_0803a798(); // Engine Event 05 (?)
// extern ? func_0803a8e4(?);
extern void func_0803aa58(); // Engine Event 0B (?)
// extern ? func_0803aa9c(?);
extern void func_0803aba4(struct Rat *rat, u32 index); // Init. one Rat
extern void func_0803ac98(); // Engine Event 0D (?)
extern void func_0803ad50(); // Engine Event 10 (?)
// extern ? func_0803ad60(?);
// extern ? func_0803aef4(?);
extern void func_0803b034(); // Engine Event 01 (?)
// extern ? func_0803b1ac(?);
// extern ? func_0803b1e8(?);
// extern ? func_0803b230(?);
// extern ? func_0803b258(?);
// extern ? func_0803b37c(?);
// extern ? func_0803b924(?);
// extern ? func_0803b9fc(?);
extern void func_0803baa0(struct RatRaceDashParticle *particle); // Init. one dust puff
// extern ? func_0803baf8(?);
// extern ? func_0803bb2c(?);
// extern ? func_0803bbd8(?);
// extern ? func_0803bc08(?);
extern void func_0803bc40(struct RatRacePlate *plate); // Init. one plate
extern void func_0803bc98(); // Engine Event 0E (?)
// extern ? func_0803bd0c(?);
// extern ? func_0803bd58(?);
