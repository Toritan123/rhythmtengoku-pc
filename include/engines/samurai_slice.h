#pragma once

#include "global.h"
#include "engines.h"

#include "games/samurai_slice/graphics/samurai_slice_graphics.h"

// Engine Macros/Enums:
enum SamuraiSliceVersionsEnum {
    SAMURAI_SLICE_VER_0,
    SAMURAI_SLICE_VER_REMIX
};


// Engine Types:
// One of the two large demons that walk in with a shadow. 0x34 bytes.
struct SamuraiSliceDemon {
    s16 sprite;             // 0x00
    s16 shadowSprite;       // 0x02
    u8  state;              // 0x04
    u8  unk05[3];
    s32 x;                  // 0x08  16.8 fixed point
    s32 y;                  // 0x0C  16.8 fixed point
    u8  unk10[0x24];        // 0x10
};

// One of the ten small demons, each with its own affine group. 0x20 bytes.
struct SamuraiSliceMedDemon {
    s16 sprite;             // 0x00
    s8  affineGroup;        // 0x02
    u8  unk03[0x19];        // 0x03
    u8  unk1C;              // 0x1C
    u8  unk1D[3];
};

// Field offsets recovered from the engine's own assembly
// (asm/engines/samurai_slice/). sizeof() is 0x1e4 on the GBA, as the
// placeholder this replaces asserted; on a 64-bit host it grows by the one
// pointer at the front, which shifts every offset below it by four -- so do
// not reuse the raw numbers in scripts or tables.
struct SamuraiSliceEngineData {
    struct BitmapFontOBJ *objFont;              // 0x000
    u8  version;                                // 0x004
    u8  unk005[7];
    s16 samuraiSprite;                          // 0x00C
    u8  unk00E;                                 // 0x00E
    u8  unk00F;
    struct SamuraiSliceDemon demons[2];         // 0x010
    u8  unk078;                                 // 0x078
    u8  unk079[3];
    s32 bg1ScrollX;                             // 0x07C  16.8 fixed point
    s32 bg2ScrollX;                             // 0x080  16.8 fixed point
    s32 unk084;                                 // 0x084
    u8  unk088;
    u8  unk089;                                 // 0x089
    s16 textSprite;                             // 0x08A
    s16 sliceEffectSprite;                      // 0x08C
    u8  unk08E[2];
    struct SamuraiSliceMedDemon medDemons[10];  // 0x090
    u8  unk1D0;                                 // 0x1D0
    u8  unk1D1;
    s16 unk1D2;                                 // 0x1D2
    u8  unk1D4[4];
    s16 unk1D8;                                 // 0x1D8
    s16 flamesSprite;                           // 0x1DA
    u32 unk1DC;                                 // 0x1DC
    u8  unk1E0;                                 // 0x1E0
    u8  unk1E1;
    s16 unk1E2;                                 // 0x1E2
};

struct SamuraiSliceCue {
    /* add fields here */
};

struct SamuraiSlice_0805a5d4 {
    struct Animation *anim;
    u32 unk4;
    u32 unk8;
};


// Engine Data:
extern const char D_0805a5d0[];
extern const struct SamuraiSlice_0805a5d4 D_0805a5d4[];


// Engine Definition Data:
extern struct CompressedData *samurai_slice_buffered_textures[];
extern struct GraphicsTable samurai_slice_gfx_table[];
extern struct Animation *samurai_slicing_anim[];
extern struct Animation *samurai_beat_anim[];
extern struct Animation *D_089e4928[];
extern struct Animation *D_089e4940[];


// Functions:
extern void samurai_slice_init_gfx3(void); // Graphics Init. 3
extern void samurai_slice_init_gfx2(void); // Graphics Init. 2
extern void samurai_slice_init_gfx1(void); // Graphics Init. 1
extern void samurai_slice_engine_start(u32 version); // Game Engine Start
// extern ? func_08030e84(?);
extern void samurai_slice_engine_event_stub(void); // Engine Event 00 (STUB)
extern void func_08030f04(); // Engine Event 01 (?)
extern void func_08030f34(); // Engine Event 06 (?)
extern void samurai_slice_engine_update(void); // Game Engine Update
extern void func_0803113c(); // Engine Event 05 (?)
extern void func_0803118c(); // Engine Event 09 (?)
extern void samurai_slice_engine_stop(void); // Game Engine Stop
extern void samurai_slice_cue_spawn(struct Cue *, struct SamuraiSliceCue, u32 isSecondSlice); // Cue - Spawn
extern u32 samurai_slice_cue_update(struct Cue *, struct SamuraiSliceCue, u32 runningTime, u32 duration); // Cue - Update
extern void samurai_slice_cue_despawn(struct Cue *, struct SamuraiSliceCue); // Cue - Despawn
extern void samurai_slice_cue_hit(struct Cue *, struct SamuraiSliceCue, u32 pressed, u32 released); // Cue - Hit
extern void samurai_slice_cue_barely(struct Cue *, struct SamuraiSliceCue, u32 pressed, u32 released); // Cue - Barely
extern void samurai_slice_cue_miss(struct Cue *, struct SamuraiSliceCue); // Cue - Miss
// extern ? func_080316ec(?);
extern void func_08031770(); // Engine Event 08 (?)
extern void samurai_slice_input_event(u32 pressed, u32 released); // Input Event
// extern ? func_080317f4(?);
extern void samurai_slice_common_beat_animation(); // Common Event 0 (Beat Animation)
extern void samurai_slice_common_display_text(); // Common Event 1 (Display Text, Unimplemented)
extern void func_080319b4(struct SamuraiSliceDemon *demon); // Init. Large Demon
// extern ? func_08031a6c(?);
extern void func_08031bc0(); // Engine Event 02 (?)
extern void func_08031c54(); // Engine Event 04 (?)
// extern ? func_08031c68(?);
// extern ? func_08031c94(?);
// extern ? func_08032070(?);
// extern ? func_080320c8(?);
// extern ? func_080321c8(?);
extern void func_08032228(void); // Reset Background Scroll
// extern ? func_08032298(?);
// extern ? func_08032330(?);
extern void func_08032430(); // Engine Event 03 (?)
// extern ? func_08032478(?);
extern void func_080324a4(); // Engine Event 07 (?)
extern void func_080324b8(struct SamuraiSliceMedDemon *demon); // Init. Small Demon
// extern ? func_08032510(?);
// extern ? func_08032708(?);
// extern ? func_080327a4(?);
