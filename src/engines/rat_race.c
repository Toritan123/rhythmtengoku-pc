#include "engines/rat_race.h"
#include "src/scenes/gameplay.h"

#ifndef PLATFORM_PC
asm(".include \"include/gba.inc\""); // Temporary
#endif

// For readability.
#define gRatRace ((struct RatRaceEngineData *)gCurrentEngineData)


/* RAT RACE */


#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039dfc.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08039dfc] Graphics Init. 3
void rat_race_init_gfx3(void) {
    func_0800c604(0);
    gameplay_start_screen_fade_in();
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e0c.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08039e0c] Graphics Init. 2
void rat_race_init_gfx2(void) {
    u32 temp;

    func_0800c604(0);
    temp = func_08002ee0(get_current_mem_id(), rat_race_gfx_table, 0x2000);
    run_func_after_task(temp, &rat_race_init_gfx3, 0);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e3c.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08039e3c] Graphics Init. 1
void rat_race_init_gfx1(void) {
    u32 temp;

    func_0800c604(0);
    temp = start_new_texture_loader(get_current_mem_id(), rat_race_buffered_textures);
    run_func_after_task(temp, &rat_race_init_gfx2, 0);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e68.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_08039e68] Game Engine Start
void rat_race_engine_start(u32 version) {
    struct RatRaceEngineData *ratRace = gRatRace;
    struct PrintedTextAnim *text;
    u32 i;

    ratRace->version = version;

    rat_race_init_gfx1();
    scene_show_obj_layer();
    scene_set_bg_layer_display(BG_LAYER_1, TRUE, 0, 0, 0, 0x1d, 0);
    scene_set_bg_layer_display(BG_LAYER_2, TRUE, 0, 0, 0, 0x1e, 1);
    scene_set_bg_layer_display(BG_LAYER_3, TRUE, 0, 0, 0, 0x1f, 2);
    scene_set_bg_layer_display(BG_LAYER_0, TRUE, 0, 0, 0, 0x1a, 0x4003);

    gRatRace->objFont = scene_create_obj_font_printer(0x340, 2);
    text = bmp_font_obj_print_c(gRatRace->objFont, D_0805a8b4, 0, 0);
    gRatRace->textX = 0x78;
    gRatRace->textSprite = sprite_create(gSpriteHandler, (struct Animation *)text,
                                         0, 0x78, 0x5a, 0, 0, 0, 0);
    gRatRace->unk010 = 1;
    gRatRace->unk014 = 0;

    gRatRace->bubbleSprite = sprite_create(gSpriteHandler, anim_rat_text_bubble_l,
                                           0, 0x78, 0x68, 2, 0, 0, 0);
    sprite_set_visible(gSpriteHandler, gRatRace->bubbleSprite, FALSE);

    ratRace = gRatRace;
    if (ratRace->version == 0) {
        ratRace->unk018 = -0x8800;
        ratRace->unk01C = 0;
    } else {
        ratRace->unk018 = 0x7800;
        ratRace->unk01C = 2;
    }

    ratRace = gRatRace;
    ratRace->unk020 = 0;
    ratRace->unk024 = 0;
    ratRace->unk028 = 0;
    ratRace->unk02C = 0;
    gRatRace->unk030 = 0x7800;
    gRatRace->unk034 = 0;
    gRatRace->unk038 = 1;
    gRatRace->unk070 = 0;
    gRatRace->unk0D2 = 1;
    gameplay_set_input_buttons(A_BUTTON, 0);

    for (i = 0; i < 3; i++) {
        func_0803aba4(&gRatRace->rats[i], i);
    }

    gRatRace->playerLabelSprite = sprite_create(gSpriteHandler, anim_rat_race_player_label,
                                                0, 0x40, 0x40, 3, 0, 0, 0);
    sprite_set_x_y(gSpriteHandler, gRatRace->playerLabelSprite,
                   (s16)(gRatRace->rats[0].unk8 >> 8), 0x90);

    func_0803a678();

    for (i = 0; i < 9; i++) {
        func_0803baa0(&gRatRace->particles[i]);
    }

    gRatRace->unk0D0 = 0;
    gRatRace->unk0D3 = 0;
    gRatRace->unk0D4 = 0;

    gRatRace->blankSprite = sprite_create(gSpriteHandler, anim_rat_race_blank,
                                          0, 0x40, 0x88, 1, 0, 0, 0);
    sprite_set_x(gSpriteHandler, gRatRace->blankSprite, 0x12c);
    gRatRace->unk0DA = 0;
    gRatRace->unk0DE = 0x40;
    gRatRace->unk0E2 = 0;
    gRatRace->unk0E4 = 0;

    gRatRace->affineGroup = scene_affine_group_alloc();
    gRatRace->trafficLightSprite = sprite_create(gSpriteHandler, anim_rat_traffic_light,
                                                 0, 0x40, 0x40, 6, 0, 0, 0);
    assign_sprite_affine_param(gRatRace->trafficLightSprite, gRatRace->affineGroup);
    sprite_set_visible(gSpriteHandler, gRatRace->trafficLightSprite, FALSE);
    sprite_set_y(gSpriteHandler, gRatRace->trafficLightSprite, 0x82);

    gRatRace->unk0E8 = 0;
    for (i = 0; i < 6; i++) {
        func_0803bc40(&gRatRace->plates[i]);
    }

    gRatRace->unk11C = 1;
    gRatRace->unk11D = 0;
    gRatRace->unk11E = 0xff;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a154.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a158.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a164.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a198.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1d4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1e4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1f8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a204.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a230.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a2a8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a350.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a3b8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a3c4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a41c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a434.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a458.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a47c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a490.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4a8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4f8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a564.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5bc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5c0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5dc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5e0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a610.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a640.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a644.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a648.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a64c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a650.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a654.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a678.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a678] Init. the Cat
void func_0803a678(void) {
    struct RatRaceEngineData *ratRace = gRatRace;

    ratRace->unk080 = 0;
    ratRace->unk07C = 0;
    gRatRace->catX = 0x5000;

    gRatRace->catPupilsSprite = sprite_create(gSpriteHandler, anim_cat_pupils,
                                              4, 0x78, 0x94, 0xc00a, 0, 0, 0);
    gRatRace->catEyelidsSprite = sprite_create(gSpriteHandler, anim_cat_eyelids,
                                               0, 0x78, (s16)((gRatRace->catX >> 8) + 0x44),
                                               0xc005, 0, 0, 0);
    sprite_set_visible(gSpriteHandler, gRatRace->catEyelidsSprite, FALSE);

    // The paw exists twice: the second copy is drawn mirrored.
    gRatRace->catPawSprite = sprite_create(gSpriteHandler, anim_cat_paw,
                                           0, 0x78, 0x58, 0x8005, 0, 0, 0);
    gRatRace->catPawMirrorSprite = sprite_create_w_attr(gSpriteHandler, anim_cat_paw,
                                                        0, 0x78, 0x58, 0x8005, 0, 0, 0, 0x1000);
    sprite_set_visible(gSpriteHandler, gRatRace->catPawSprite, FALSE);
    sprite_set_visible(gSpriteHandler, gRatRace->catPawMirrorSprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a798.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a8e4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aa58.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aa9c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aba4.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803aba4] Init. one Rat
void func_0803aba4(struct Rat *rat, u32 index) {
    *rat = rat_race_init_rat_data[index];

    rat->ratSprite = sprite_create(gSpriteHandler, anim_rat_run, 0, 0x64, 0x90, 5, 1, 0, 0);
    if (rat->unk5 != 0) {
        sprite_set_base_palette(gSpriteHandler, rat->ratSprite, 1);
    }
    rat->sweatSprite = sprite_create(gSpriteHandler, anim_rat_fear_particles_barely,
                                     0, 0x40, 0x40, 4, 1, 0, 0);

    // Version 0 starts the rats off screen to the left; the others start them
    // parked at the line.
    if (gRatRace->version == 0) {
        rat->unk8 -= 0x10000;
    } else {
        rat->unk8 = 0x7800;
        rat->unk4 = 1;
    }

    sprite_set_x_y(gSpriteHandler, rat->ratSprite,
                   (s16)((rat->unk8 + D_089e68ac[rat->unk5]) >> 8), 0x90);
    sprite_set_x_y(gSpriteHandler, rat->sweatSprite,
                   (s16)((rat->unk8 + D_089e68ac[rat->unk5]) >> 8), 0x90);
    sprite_set_visible(gSpriteHandler, rat->sweatSprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ac98.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ad50.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ad60.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aef4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b034.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b1ac.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b1e8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b230.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b258.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b37c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b924.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b9fc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803baa0.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803baa0] Init. one dust puff
void func_0803baa0(struct RatRaceDashParticle *particle) {
    particle->active = FALSE;
    particle->sprite = sprite_create(gSpriteHandler, anim_rat_dash_particle,
                                     0, 0x40, 0x40, 4, 0, 0, 0);
    particle->x = 0;
    sprite_set_x_y(gSpriteHandler, particle->sprite, 0, -0x40);
    sprite_set_visible(gSpriteHandler, particle->sprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803baf8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bb2c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bbd8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc08.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc40.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803bc40] Init. one plate
void func_0803bc40(struct RatRacePlate *plate) {
    plate->sprite = sprite_create(gSpriteHandler, anim_rat_race_plates,
                                  0, 0x40, 0x40, 0xa, 0, 0, 0);
    plate->active = FALSE;
    plate->x = 0x17800;
    sprite_set_x_y(gSpriteHandler, plate->sprite, (s16)(plate->x >> 8), 0x7e);
    sprite_set_visible(gSpriteHandler, plate->sprite, FALSE);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc98.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bd0c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bd58.s"
#endif
