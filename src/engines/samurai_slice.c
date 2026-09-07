#include "engines/samurai_slice.h"
#include "src/scenes/gameplay.h"

#ifndef PLATFORM_PC
asm(".include \"include/gba.inc\""); // Temporary
#endif

// For readability.
#define gSamuraiSlice ((struct SamuraiSliceEngineData *)gCurrentEngineData)


/* SAMURAI SLICE */


#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030c48.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08030c48] Graphics Init. 3
void samurai_slice_init_gfx3(void) {
    func_0800c604(0);
    gameplay_start_screen_fade_in();
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030c58.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08030c58] Graphics Init. 2
void samurai_slice_init_gfx2(void) {
    u32 temp;

    func_0800c604(0);
    temp = func_08002ee0(get_current_mem_id(), samurai_slice_gfx_table, 0x2000);
    run_func_after_task(temp, &samurai_slice_init_gfx3, 0);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030c88.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08030c88] Graphics Init. 1
void samurai_slice_init_gfx1(void) {
    u32 temp;

    func_0800c604(0);
    temp = start_new_texture_loader(get_current_mem_id(), samurai_slice_buffered_textures);
    run_func_after_task(temp, &samurai_slice_init_gfx2, 0);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030cb4.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08030cb4] Game Engine Start
void samurai_slice_engine_start(u32 version) {
    struct SamuraiSliceEngineData *samuraiSlice = gSamuraiSlice;
    struct PrintedTextAnim *text;
    u32 i;

    samuraiSlice->version = version;

    samurai_slice_init_gfx1();
    scene_show_obj_layer();
    scene_set_bg_layer_display(BG_LAYER_1, TRUE, 0, 0, 0, 0x1c, 0x8000);
    scene_set_bg_layer_display(BG_LAYER_2, TRUE, 0, 0, 0, 0x1e, 0x8001);
    scene_set_bg_layer_display(BG_LAYER_3, TRUE, 0, 0, 0, 0x1b, 2);

    samuraiSlice->objFont = scene_create_obj_font_printer(0x340, 2);
    text = bmp_font_obj_print_c(samuraiSlice->objFont, D_0805a5d0, 0, 0);
    gSamuraiSlice->textSprite = sprite_create(gSpriteHandler, (struct Animation *)text,
                                              0, 0x3c, 0x38, 0, 0, 0, 0);
    gameplay_set_input_buttons(A_BUTTON, 0);

    gSamuraiSlice->samuraiSprite = sprite_create(gSpriteHandler, anim_samurai_beat_1,
                                                 1, 0xe, 0x7b, 0xa, 0, 0, 0);
    gSamuraiSlice->unk00E = 0;

    gSamuraiSlice->flamesSprite = sprite_create(gSpriteHandler, anim_samurai_flames,
                                                0, 0x14, 0x78, 0x14, 1, 0, 0);
    sprite_set_visible(gSpriteHandler, gSamuraiSlice->flamesSprite, FALSE);
    gSamuraiSlice->unk1DC = 0x8000;

    for (i = 0; i < 2; i++) {
        func_080319b4(&gSamuraiSlice->demons[i]);
    }
    func_08032228();

    gSamuraiSlice->sliceEffectSprite = sprite_create(gSpriteHandler, anim_samurai_slice_effect,
                                                     0, 0x4a, 0x60, 2, 1, 0, 2);
    sprite_set_visible(gSpriteHandler, gSamuraiSlice->sliceEffectSprite, FALSE);

    for (i = 0; i < 10; i++) {
        func_080324b8(&gSamuraiSlice->medDemons[i]);
    }

    gSamuraiSlice->unk1D0 = 0;
    gSamuraiSlice->unk1D2 = 0;
    gSamuraiSlice->unk1D8 = 0;
    gSamuraiSlice->unk1E0 = 0;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030e84.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030f00.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08030f00] Engine Event 07 (STUB)
void samurai_slice_engine_event_stub(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030f04.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030f34.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08030f54.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_0803113c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_0803118c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080311b4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080311c8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080311d4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080312b4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080312b8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031588.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080316e4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080316ec.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031770.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080317c8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080317f4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_0803193c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080319b0.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_080319b0] Common Event 1 (Display Text, Unimplemented)
void samurai_slice_common_display_text(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080319b4.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_080319b4] Init. Large Demon
void func_080319b4(struct SamuraiSliceDemon *demon) {
    demon->sprite = sprite_create(gSpriteHandler, anim_med_demon_hop, 0, 0x78, 0x42, 0x800a, 0, 0, 0);
    demon->shadowSprite = sprite_create(gSpriteHandler, anim_demon_shadow, 0, 0x40, 0x40, 0x8014, 0, 0, 0);
    demon->state = 0;
    demon->x = 0xf000;
    demon->y = 0x2800;

    sprite_set_x_y(gSpriteHandler, demon->sprite, 0xf0, 0x28);
    // The shadow tracks the demon's 16.8 position; >> 8 with the low bits
    // dropped is what the original's shift pair works out to.
    sprite_set_x_y(gSpriteHandler, demon->shadowSprite,
                   (s16)(demon->x >> 8), (s16)(demon->y >> 8));
    sprite_set_visible(gSpriteHandler, demon->sprite, FALSE);
    sprite_set_visible(gSpriteHandler, demon->shadowSprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031a6c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031bc0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031c54.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031c68.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08031c94.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032070.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080320c8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080321c8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032228.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08032228] Reset Background Scroll
void func_08032228(void) {
    struct SamuraiSliceEngineData *samuraiSlice = gSamuraiSlice;

    samuraiSlice->unk078 = 0;
    samuraiSlice->bg1ScrollX = 0;
    samuraiSlice->bg2ScrollX = 0;
    scene_set_bg_layer_pos(BG_LAYER_1, (s16)(samuraiSlice->bg1ScrollX >> 8), 0);
    scene_set_bg_layer_pos(BG_LAYER_2, (s16)(gSamuraiSlice->bg2ScrollX >> 8), 0);

    gSamuraiSlice->unk089 = 0;
    gSamuraiSlice->unk084 = 0;
    D_03004b10.WININ = 0x3846;
    D_03004b10.WINOUT = 0x1000;
    gSamuraiSlice->unk1E2 = 0;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032298.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032330.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032430.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032478.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080324a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080324b8.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_080324b8] Init. Small Demon
void func_080324b8(struct SamuraiSliceMedDemon *demon) {
    demon->unk1C = 0;
    demon->affineGroup = scene_affine_group_alloc();
    demon->sprite = sprite_create(gSpriteHandler, anim_med_demon_hop, 0, 0x78, 0x42, 5, 0, 0, 0);
    assign_sprite_affine_param(demon->sprite, demon->affineGroup);
    sprite_set_visible(gSpriteHandler, demon->sprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032510.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_08032708.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/samurai_slice/asm_080327a4.s"
#endif
