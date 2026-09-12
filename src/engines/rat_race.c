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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a154] Engine Event 00 (STUB)
void rat_race_engine_event_stub(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a158.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a164.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a164] Decay the run speed
void func_0803a164(void) {
    struct RatRaceEngineData *ratRace;

    ratRace = gRatRace;
    ratRace->unk020 -= func_0800c398();
    if (ratRace->unk020 <= 0) {
        ratRace->unk020 = 0;
        ratRace->unk028 = 0;
    } else {
        ratRace->unk028 = ((ratRace->unk020 * 3) << 9) / ratRace->unk024;
    }
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a198.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a198] Advance the track
void func_0803a198(void) {
    struct RatRaceEngineData *ratRace = gRatRace;

    if ((ratRace->unk01C != 1) && (ratRace->unk01C != 2)) return;

    func_0803a164();
    ratRace = gRatRace;
    ratRace->unk018 += ((func_0800c398() << 6) / 0x18) + ratRace->unk028;
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a3c4] Keep the speech bubble over its rat
void func_0803a3c4(void) {
    struct RatRaceEngineData *ratRace = gRatRace;
    u32 which = (ratRace->unk014 != 0) ? 1 : 2;
    s32 x;

    x = ratRace->rats[which].unk8 - ratRace->unk030 + 0x7800;
    ratRace->textX = x >> 8;

    sprite_set_x(gSpriteHandler, ratRace->textSprite, (s16)(x >> 8));
    sprite_set_x(gSpriteHandler, gRatRace->bubbleSprite, gRatRace->textX);
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a4a4] Game Engine Stop
void rat_race_engine_stop(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4a8.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a4a8] Where a crockery cue starts
//
// The track keeps moving while the cue is in flight, so the spawn point is
// integrated forward over however long the current speed will last.
s32 func_0803a4a8(u32 kind) {
    struct RatRaceEngineData *ratRace = gRatRace;
    s32 remaining = ratRace->unk020;
    s32 divisor = ratRace->unk024;
    s32 x = ratRace->unk018 - ratRace->rats[0].unk8 + 0x7800;
    s32 step = D_089e66bc[(u8)kind] << 8;

    for (;;) {
        x += step;
        if (remaining <= 0) break;
        remaining -= func_0800c398();
        if (remaining <= 0) break;
        step = ((remaining * 3) << 9) / divisor;
    }
    return x;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4f8.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a4f8] Cue - Spawn (Stop)
void rat_race_cue_spawn_stop(struct Cue *cue, struct RatRaceCue *info, u32 kind) {
    info->kind = kind;
    info->sprite = sprite_create(gSpriteHandler, anim_rat_race_crockery,
                                 0, 0x64, 0x7e, 0xa, 0, 0, 0);
    info->x = func_0803a4a8(info->kind);
    sprite_set_x(gSpriteHandler, info->sprite, (s16)(info->x >> 8));

    if (gRatRace->unk010 != 0) {
        sprite_set_anim_cel(gSpriteHandler, info->sprite, 1);
    }
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a564.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a564] Cue - Update (Stop)
u32 rat_race_cue_update_stop(struct Cue *cue, struct RatRaceCue *info, u32 runningTime, u32 duration) {
    info->x -= gRatRace->unk034;
    if ((info->x >> 8) <= -0x50) return TRUE;

    sprite_set_x(gSpriteHandler, info->sprite, (s16)(info->x >> 8));
    return FALSE;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5a4.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a5a4] Cue - Despawn (Stop)
void rat_race_cue_despawn_stop(struct Cue *cue, struct RatRaceCue *info) {
    sprite_delete(gSpriteHandler, info->sprite);
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5bc.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a5bc] Cue - Spawn (Dash)
void rat_race_cue_spawn_dash(struct Cue *cue, struct RatRaceCue *info, u32 unused) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5c0.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a5c0] Cue - Update (Dash)
u32 rat_race_cue_update_dash(struct Cue *cue, struct RatRaceCue *info, u32 runningTime, u32 duration) {
    if (runningTime > (u32)ticks_to_frames(0x78)) return TRUE;
    return FALSE;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5dc.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a5dc] Cue - Despawn (Dash)
void rat_race_cue_despawn_dash(struct Cue *cue, struct RatRaceCue *info) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5e0.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a5e0] Cue - Hit (Stop)
void rat_race_cue_hit_stop(struct Cue *cue, struct RatRaceCue *info, u32 pressed, u32 released) {
    func_0803b034(0);
    // unk11D marks this stop as commanded, so func_0803b924 does not treat it
    // as a slip.
    gRatRace->unk11D = TRUE;
    func_0803b924();
    gRatRace->unk11D = FALSE;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a610.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a610] Cue - Hit (Dash)
void rat_race_cue_hit_dash(struct Cue *cue, struct RatRaceCue *info, u32 pressed, u32 released) {
    func_0803b034(1);
    gRatRace->unk11D = TRUE;
    func_0803b9fc();
    gRatRace->unk11D = FALSE;
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a640.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a640] Cue - Barely
void rat_race_cue_barely(struct Cue *cue, struct RatRaceCue *info, u32 pressed, u32 released) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a644.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a644] Cue - Miss
void rat_race_cue_miss(struct Cue *cue, struct RatRaceCue *info) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a648.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a648] Input Event (STUB)
void rat_race_input_event(u32 pressed, u32 released) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a64c.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a64c] Common Event 0 (Beat Animation, Unimplemented)
void rat_race_common_beat_animation(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a650.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a650] Common Event 1 (Display Text, Unimplemented)
void rat_race_common_display_text(void) {
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a654.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803a654] Common Event 2 (Init. Tutorial)
void rat_race_common_init_tutorial(struct Scene *skipDestination) {
    if (skipDestination != NULL) {
        gameplay_enable_tutorial(TRUE);
        gameplay_set_skip_destination(skipDestination);
    } else {
        gameplay_enable_tutorial(FALSE);
    }
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803aef4] Rat animation finished
//
// Installed as the sprite callback whenever a one-shot animation is started;
// rat->unkC says which follow-up to run, and each arm leaves the rat in a new
// unk4 state.
void func_0803aef4(void *unused, s16 spriteId, struct Rat *rat) {
    struct Animation *anim;

    func_0800c604(0);

    switch (rat->unkC) {
        case 0:
            sprite_set_anim(gSpriteHandler, spriteId, anim_rat_run, 0, 1, 0, 0);
            rat->unk4 = 1;
            break;

        case 1:
            sprite_set_anim(gSpriteHandler, spriteId, anim_rat_stop, 0, 1, 0x7f, 0);
            rat->unk4 = 2;
            break;

        case 2:
            sprite_set_anim(gSpriteHandler, spriteId, anim_rat_angry_stop_r, 0, 1, 0x7f, 4);
            sprite_set_callback(gSpriteHandler, spriteId, (void *)func_0803aef4, (uintptr_t)rat);
            rat->unkC = 1;
            rat->unk4 = 2;
            break;

        case 3:
            // The two other rats run angry in opposite directions.
            anim = (rat->unk5 == 1) ? anim_rat_angry_run_r : anim_rat_angry_run_l;
            sprite_set_anim(gSpriteHandler, spriteId, anim, 0, 1, 0x7f, 4);
            sprite_set_callback(gSpriteHandler, spriteId, (void *)func_0803aef4, (uintptr_t)rat);
            rat->unkC = 0;
            rat->unk4 = 1;
            break;

        case 4:
            sprite_set_anim(gSpriteHandler, spriteId, anim_rat_prepare_dash, 0, 1, 0x7f, 0);
            rat->unk4 = 6;
            break;

        default:
            break;
    }
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b034.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803b034] Engine Event 01 (Set the Pack Gait)
//
// gait 0 = stop, 1 = run, 2 = crouch ready to dash. Nothing happens if the
// pack is already in that gait.
void func_0803b034(u32 gait) {
    struct RatRaceEngineData *ratRace = gRatRace;
    struct Rat *rat = ratRace->rats;
    s32 budget;
    u32 i;

    if (ratRace->unk038 == gait) return;
    ratRace->unk038 = gait;

    if (gait == 1) {
        // Starting to run tops the speed budget back up, by an amount that
        // depends on how the last stretch ended.
        ratRace = gRatRace;
        switch (ratRace->unk02C) {
            case 0:  budget = 0; break;
            case 1:  budget = ratRace->unk020 + 0x6000; break;
            case 2:  budget = ratRace->unk020 + 0x3000; break;
            default: budget = 0; goto no_budget;
        }
        ratRace->unk024 = budget;
        ratRace->unk020 = budget;
    no_budget:
        gRatRace->unk028 = 0;
    }

    for (i = 0; i < 3; i++, rat++) {
        if (rat->unk5 == 0) {
            // The player's rat only reacts to the dash crouch, and only from
            // a standstill.
            if (gait != 2) continue;
            if (rat->unk4 != 2) continue;
            rat->unk4 = 6;
            sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_prepare_dash,
                            0, 1, 0x7f, rat->unk5);
            continue;
        }

        if (gait == 1) {
            if (rat->unk4 == 5) {
                rat->unk4 = 1;
                sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_angry_run_r,
                                0, 1, 0x7f, 4);
                sprite_set_callback(gSpriteHandler, rat->ratSprite,
                                    (void *)func_0803aef4, (uintptr_t)rat);
                rat->unkC = 0;
            } else {
                rat->unk4 = 1;
                rat->unkC = 0;
                sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_run, 0, 1, 0, 0);
            }
        } else if (gait < 1) {
            rat->unk4 = 2;
            rat->unkC = 1;
            sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_stop, 0, 1, 0x7f, 0);
        } else if (gait == 2) {
            if (rat->unk4 == 5) {
                rat->unkC = 4;
            } else {
                rat->unk4 = 6;
                sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_prepare_dash,
                                0, 1, 0x7f, 0);
            }
        }
    }
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803b258] Player runs into the rat ahead
void func_0803b258(struct Rat *rat) {
    struct RatRaceEngineData *ratRace = gRatRace;
    s32 aheadX = ratRace->rats[1].unk8;

    // Only if the player has actually caught up.
    if (((aheadX >> 8) - ((rat->unk8 >> 8) - 0x28)) > 0x18) return;

    // Snap the player to just behind the rat ahead, and make that jump the
    // frame's scroll delta so everything else moves with it.
    ratRace->unk034 = aheadX - (rat->unk8 - 0x800);
    rat->unk8 += ratRace->unk034;
    ratRace->unk030 = rat->unk8;

    sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_collide_run, 0, 1, 0x7f, 4);
    sprite_set_callback(gSpriteHandler, rat->ratSprite, (void *)func_0803aef4, (uintptr_t)rat);
    rat->unkC = 0;
    rat->unk4 = 3;

    sprite_set_anim(gSpriteHandler, gRatRace->rats[1].ratSprite,
                    anim_rat_collide_stop, 0, 1, 0x7f, 4);
    sprite_set_callback(gSpriteHandler, gRatRace->rats[1].ratSprite,
                        (void *)func_0803aef4, (uintptr_t)&gRatRace->rats[1]);

    if ((u8)(gRatRace->rats[1].unkC - 1) <= 1) {
        gRatRace->rats[1].unkC = 2;
        gRatRace->rats[1].unk4 = 5;
    } else {
        gRatRace->rats[1].unkC = 3;
        gRatRace->rats[1].unk4 = 1;
    }

    gRatRace->unk0E4 = 0x1000;
    play_sound(&s_rat_crush_R_seqData);

    if ((s8)gRatRace->unk11E >= 0) {
        gameplay_add_cue_result(gRatRace->unk11E, 2, 0);
    }
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b37c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b924.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803b924] The player stops
void func_0803b924(void) {
    struct Rat *rat = gRatRace->rats;

    // The player's rat is the one with unk5 == 0.
    while (rat->unk5 != 0) rat++;

    switch (rat->unk4) {
        case 1:
            if (!gRatRace->unk0D2) break;
            rat->unk4 = 2;
            if (gRatRace->unk11D != 0) {
                sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_stop, 0, 1, 0x7f, 0);
            } else {
                // Stopping without being told to is a slip.
                sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_stop_barely,
                                0, 1, 0x7f, 0);
                play_sound(&s_f_rat_slip_seqData);
            }
            break;

        case 3:
            if (!gRatRace->unk0D2) break;
            sprite_set_playback(gSpriteHandler, rat->ratSprite, 1, 0x7f, 4);
            sprite_set_callback(gSpriteHandler, rat->ratSprite,
                                (void *)func_0803aef4, (uintptr_t)rat);
            rat->unkC = 1;
            break;

        default:
            break;
    }
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b9fc.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803b9fc] The player dashes
void func_0803b9fc(void) {
    struct Rat *rat = gRatRace->rats;

    while (rat->unk5 != 0) rat++;

    switch (rat->unk4) {
        case 2:
        case 6:
            if (!gRatRace->unk11C) break;
            rat->unk4 = 1;
            sprite_set_anim(gSpriteHandler, rat->ratSprite, anim_rat_run, 0, 1, 0, 0);
            break;

        case 4:
            sprite_set_playback(gSpriteHandler, rat->ratSprite, 1, 0x7f, 4);
            sprite_set_callback(gSpriteHandler, rat->ratSprite,
                                (void *)func_0803aef4, (uintptr_t)rat);
            rat->unkC = 0;
            func_0803b258(rat);
            break;

        default:
            break;
    }
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803bbd8] Scroll one dust puff
void func_0803bbd8(struct RatRaceDashParticle *particle) {
    particle->x -= gRatRace->unk034;
    sprite_set_x(gSpriteHandler, particle->sprite, (s16)(particle->x >> 8));
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc08.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803bc08] Scroll the dust puffs
void func_0803bc08(void) {
    struct RatRaceDashParticle *particle = gRatRace->particles;
    u32 i;

    gRatRace->unk0D0++;
    for (i = 0; i < 9; i++, particle++) {
        if (particle->active) func_0803bbd8(particle);
    }
}
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
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803bd0c] Scroll one plate
void func_0803bd0c(struct RatRacePlate *plate) {
    plate->x -= gRatRace->unk034;
    if ((plate->x >> 8) <= -0x30) {
        plate->active = FALSE;
        sprite_set_visible(gSpriteHandler, plate->sprite, FALSE);
    }
    sprite_set_x(gSpriteHandler, plate->sprite, (s16)(plate->x >> 8));
}
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bd58.s"
#else
// Translated from the assembly above and checked against it; not proven byte-exact.
// [func_0803bd58] Scroll the plates
void func_0803bd58(void) {
    struct RatRacePlate *plate = gRatRace->plates;
    u32 i;

    for (i = 0; i < 6; i++, plate++) {
        if (plate->active) func_0803bd0c(plate);
    }
}
#endif
