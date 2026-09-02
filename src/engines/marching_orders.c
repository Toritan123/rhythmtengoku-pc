#include "engines/marching_orders.h"

#ifndef PLATFORM_PC
asm(".include \"include/gba.inc\""); // Temporary
#endif

// For readability.
#define gMarchingOrders ((struct MarchingOrdersEngineData *)gCurrentEngineData)


/* MARCHING ORDERS */


#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034100.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034130.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034140.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034180.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080341ac.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080343b4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080343b8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034544.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080345cc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080346b0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080346e0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080347c0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_0803481c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_0803482c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034850.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034860.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034880.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034884.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034888.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080348a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080348a8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_0803492c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_0803493c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_0803494c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034988.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080349ac.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080349d0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_080349f4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034a4c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034a50.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034ae0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/marching_orders/asm_08034ae4.s"
#endif

#ifdef PLATFORM_PC
/* ── Marching Orders engine (C translation) ─────────────────────────────────
 * From asm/engines/marching_orders/*.s — the 32 Thumb functions above, which
 * on PC were auto-stubbed to "return 0".  With the engine stubbed the scene
 * loaded its prologue and then went black the moment the beatscript ran
 * gameplay_set_current_engine, because nothing created any sprites.
 *
 * Covers scene_marching_orders, _2, _unused and the two skipped-practice
 * variants.
 */

// Marcher actions, as passed to func_080343b8 / engine event 0.
enum MarchingActionsEnum {
    MARCHING_ACT_STAND,        // 0 stop-beat pose, held
    MARCHING_ACT_STOP_BEAT,    // 1
    MARCHING_ACT_BEAT,         // 2
    MARCHING_ACT_FACE_LEFT,    // 3
    MARCHING_ACT_FACE_RIGHT,   // 4
    MARCHING_ACT_POINT_LEFT,   // 5
    MARCHING_ACT_POINT_RIGHT,  // 6
    MARCHING_ACT_STEP_R,       // 7
    MARCHING_ACT_STEP_L,       // 8
    MARCHING_ACT_CLAP          // 9
};

// Bit 8 of an engine-event-0 action includes the player (marcher 3); without
// it only the three computer-controlled marchers react.
#define MARCHING_ACT_INCLUDE_PLAYER 0x100

// [func_08034100] Animation for the current version.
struct Animation *func_08034100(u32 anim) {
    if ((s32)anim < 0) {
        return NULL;
    }
    return marching_anim_table[anim][gMarchingOrders->version];
}

// [marching_init_gfx3] Graphics Init. 3
void marching_init_gfx3(void) {
    func_0800c604(0);
    gameplay_start_screen_fade_in();
}

// [marching_init_gfx2] Graphics Init. 2
void marching_init_gfx2(void) {
    s32 task;

    func_0800c604(0);
    task = func_08002ee0(get_current_mem_id(),
                         marching_gfx_tables[gMarchingOrders->version], 0x2000);
    run_func_after_task(task, marching_init_gfx3, 0);
}

// [marching_init_gfx1] Graphics Init. 1
void marching_init_gfx1(void) {
    s32 task;

    func_0800c604(0);
    task = start_new_texture_loader(get_current_mem_id(), marching_buffered_textures);
    run_func_after_task(task, marching_init_gfx2, 0);
}

// [marching_engine_start] Game Engine Start
void marching_engine_start(u32 version) {
    struct MarchingOrdersEngineData *data = gMarchingOrders;
    struct PrintedTextAnim *anim;
    u32 i;

    data->version = version;
    marching_init_gfx1();
    scene_show_obj_layer();
    scene_set_bg_layer_display(1, 1, 0, 0, 0, 0x1D, 2);
    scene_set_bg_layer_display(2, 0, 0, 0, 0, 0x1E, 1);
    // The third call reaches scene_set_bg_layer_display with layer 3 still in
    // r0 from setting the params argument; both are 3.
    scene_set_bg_layer_display(3, 1, 0, 0, 0, 0x1F, 3);

    data->textPrinter = scene_create_obj_font_printer(0x340, 2);
    anim = bmp_font_obj_print_l(data->textPrinter, D_0805a670, 1, 0xE);
    data->textSprite = sprite_create(gSpriteHandler, (struct Animation *)anim,
                                     0, 120, 0x16, 0, 0, 0, 0);

    for (i = 0; i < 4; i++) {
        struct MarchingMarcher *marcher = &data->marchers[i];
        s16 x = 80 + i * 40;

        marcher->sprite = sprite_create(gSpriteHandler,
            func_08034100(MARCHING_ANIM_STOP_BEAT), 0, x, 0x78, 0x4800, 1, 0x7F, 0);
        marcher->headSprite = sprite_create(gSpriteHandler,
            func_08034100(MARCHING_ANIM_HEAD_R), 0x7F, x, 0x78, 0x47F6, 1, 0x7F, 0x8000);
        sprite_attr_set(gSpriteHandler, marcher->headSprite, 0x1000000);

        // Both halves ride BG layer 3's scroll offset, so a marcher stays put
        // relative to the ground while func_0803482c scrolls it.
        sprite_set_origin_x_y(gSpriteHandler, marcher->sprite,
                              &D_03004b10.BG_OFS[3].x, &D_03004b10.BG_OFS[3].y);
        sprite_set_origin_x_y(gSpriteHandler, marcher->headSprite,
                              &D_03004b10.BG_OFS[3].x, &D_03004b10.BG_OFS[3].y);

        marcher->action = 0;
        marcher->stepTimer = 0;
        marcher->stepLatch = 0;
    }

    data->usePointAnims = 0;
    data->stepFoot = 0;
    data->playerBusyTimer = 0;

    data->tutorialSprite = sprite_create(gSpriteHandler,
        func_08034100(MARCHING_ANIM_TUTORIAL_ICONS), 0, 0xC8, 0x82, 0x479C, 0, 0, 0x8000);
    data->commanderSprite = sprite_create(gSpriteHandler,
        func_08034100(MARCHING_ANIM_COMMANDER), 0x7F, 0x1C, 0x7C, 0x4800, 1, 0x7F, 0);
    data->commanderTimer = 0;
    data->scrollBg = 0;

    if (version == MARCHING_ORDERS_VER_2_UNUSED) {
        gameplay_set_input_buttons(A_BUTTON | B_BUTTON, 0);
    } else {
        gameplay_set_input_buttons(A_BUTTON | B_BUTTON | DPAD_LEFT | DPAD_RIGHT, 0);
    }
}

// [marching_engine_event_stub] Engine Event 06 (STUB)
void marching_engine_event_stub(void) {
}

// [func_080343b8] Give one marcher an action: pick the body and head
// animations, whether the head is shown, and how the step timer restarts.
void func_080343b8(struct MarchingMarcher *marcher, u32 action) {
    struct Animation *bodyAnim = NULL;
    struct Animation *headAnim = NULL;
    u32 showHead = FALSE;
    u16 newStepTimer = 0;
    s32 bodyStartCel = 0;

    switch (action) {
        case MARCHING_ACT_STAND:
            bodyAnim = func_08034100(MARCHING_ANIM_STOP_BEAT);
            bodyStartCel = 0x7F;   // hold the pose instead of playing it
            break;

        case MARCHING_ACT_STOP_BEAT:
            bodyAnim = func_08034100(MARCHING_ANIM_STOP_BEAT);
            break;

        case MARCHING_ACT_BEAT:
            bodyAnim = func_08034100(MARCHING_ANIM_BEAT);
            break;

        case MARCHING_ACT_FACE_LEFT:
            // Mid-step, the body keeps stepping and only the head turns.
            if (marcher->stepTimer != 0) {
                newStepTimer = marcher->stepTimer;
                showHead = TRUE;
            } else {
                bodyAnim = func_08034100(gMarchingOrders->usePointAnims
                                         ? MARCHING_ANIM_POINT_L : MARCHING_ANIM_TURN_L);
            }
            headAnim = func_08034100(MARCHING_ANIM_HEAD_R);
            break;

        case MARCHING_ACT_FACE_RIGHT:
            if (marcher->stepTimer != 0) {
                newStepTimer = marcher->stepTimer;
                showHead = TRUE;
            } else {
                bodyAnim = func_08034100(gMarchingOrders->usePointAnims
                                         ? MARCHING_ANIM_POINT_R : MARCHING_ANIM_TURN_R);
            }
            headAnim = func_08034100(MARCHING_ANIM_HEAD_L);
            break;

        case MARCHING_ACT_POINT_LEFT:
            bodyAnim = func_08034100(MARCHING_ANIM_POINT_L);
            break;

        case MARCHING_ACT_POINT_RIGHT:
            bodyAnim = func_08034100(MARCHING_ANIM_POINT_R);
            break;

        case MARCHING_ACT_STEP_R:
            bodyAnim = func_08034100(MARCHING_ANIM_STEP_R);
            showHead = TRUE;
            newStepTimer = 1;
            marcher->stepLatch = 0;
            break;

        case MARCHING_ACT_STEP_L:
            bodyAnim = func_08034100(MARCHING_ANIM_STEP_L);
            showHead = TRUE;
            newStepTimer = 1;
            marcher->stepLatch = 0;
            break;

        case MARCHING_ACT_CLAP:
            bodyAnim = func_08034100(MARCHING_ANIM_CLAP);
            break;

        default:
            break;
    }

    if (bodyAnim != NULL) {
        sprite_set_anim(gSpriteHandler, marcher->sprite, bodyAnim, bodyStartCel, 1, 0x7F, 0);
    }
    if (headAnim != NULL) {
        sprite_set_anim(gSpriteHandler, marcher->headSprite, headAnim, 0, 1, 0x7F, 0);
    }
    // The remix version draws its head as part of the body, so it never gets
    // toggled separately.
    if (gMarchingOrders->version != MARCHING_ORDERS_VER_REMIX) {
        sprite_set_visible(gSpriteHandler, marcher->headSprite, showHead);
    }

    marcher->action = action;
    marcher->stepTimer = newStepTimer;

    // The original follows this with a play_sound_w_pitch_volume guarded by a
    // constant-zero test, so it never runs.  Not reproduced.
}

// [func_08034544] Engine Event 00: give every marcher an action.  Bit 8 of the
// action includes the player, who is otherwise driven by input.
void func_08034544(u32 action) {
    u32 total = 3;
    u32 i;

    if (action & MARCHING_ACT_INCLUDE_PLAYER) {
        total = 4;
    }
    action &= ~(u32)MARCHING_ACT_INCLUDE_PLAYER;

    for (i = 0; i < total; i++) {
        if (i == 3) {
            // The player only follows the script while not busy with an input.
            if (gMarchingOrders->playerBusyTimer != 0) {
                continue;
            }
            func_080343b8(&gMarchingOrders->marchers[3], action);
            if (action == MARCHING_ACT_STEP_R) {
                gMarchingOrders->stepFoot = 1;
            }
            if (action == MARCHING_ACT_STEP_L) {
                gMarchingOrders->stepFoot = 0;
            }
        } else {
            func_080343b8(&gMarchingOrders->marchers[i], action);
        }
    }
}

// [func_080345cc] Per-frame update for one marcher: carry the head along the
// body's per-cel offset, and end a step once it has run its course.
void func_080345cc(struct MarchingMarcher *marcher) {
    s32 x = sprite_get_data(gSpriteHandler, marcher->sprite, SPRITE_DATA_X_POS);
    s32 y = sprite_get_data(gSpriteHandler, marcher->sprite, SPRITE_DATA_Y_POS);

    if (marcher->stepTimer != 0) {
        s8 cel = sprite_get_anim_cel(gSpriteHandler, marcher->sprite);

        x += D_089e5368[cel][gMarchingOrders->version].x;
        y += D_089e5368[cel][gMarchingOrders->version].y;
        marcher->stepTimer++;

        if (marcher->stepLatch) {
            // Wait for the step animation to come back round to cel 0 before
            // dropping back to standing.
            if (cel != 0) {
                sprite_set_x_y(gSpriteHandler, marcher->headSprite, x, y);
                return;
            }
            func_080343b8(marcher, MARCHING_ACT_STAND);
            return;   // func_080343b8 has already repositioned everything
        }

        if (marcher->stepTimer > ticks_to_frames(0x30)) {
            marcher->stepLatch = 1;
            sprite_set_playback(gSpriteHandler, marcher->sprite, -1, 0, 0);
            sprite_set_anim_cel(gSpriteHandler, marcher->sprite, 3);
        }
    }

    sprite_set_x_y(gSpriteHandler, marcher->headSprite, x, y);
}

// [func_080346b0] Update all four marchers, and tick the player's lockout.
void func_080346b0(void) {
    u32 i;

    for (i = 0; i < 4; i++) {
        func_080345cc(&gMarchingOrders->marchers[i]);
    }
    if (gMarchingOrders->playerBusyTimer != 0) {
        gMarchingOrders->playerBusyTimer--;
    }
}

// [func_080346e0] Engine Event 01: commander reaction.
//   0 = nod along, 1 = angry (a miss), 2 = merely annoyed (a barely).
void func_080346e0(u32 reaction) {
    u32 duration;

    if (gMarchingOrders->commanderTimer != 0) {
        return;
    }

    switch (reaction) {
        case 1:
            sprite_set_anim(gSpriteHandler, gMarchingOrders->commanderSprite,
                            func_08034100(MARCHING_ANIM_COMMANDER), 0, 1, 0x7F, 0);
            sprite_create(gSpriteHandler, func_08034100(MARCHING_ANIM_ANGRY_PUFF),
                          0, 0x1E, 0x28, 0x480A, 1, 0, 3);
            play_sound(&s_guntai_ikari_seqData);
            duration = 0x18;
            break;

        case 2:
            sprite_set_anim(gSpriteHandler, gMarchingOrders->commanderSprite,
                            func_08034100(MARCHING_ANIM_COMMANDER_ANNOYED), 0, 1, 0x7F, 0);
            duration = 0x18;
            break;

        default:
            sprite_set_anim(gSpriteHandler, gMarchingOrders->commanderSprite,
                            func_08034100(MARCHING_ANIM_COMMANDER), 1, 1, 0x7F, 0);
            duration = 8;
            break;
    }

    gMarchingOrders->commanderTimer = ticks_to_frames(duration);
}

// [func_080347c0] Engine Event 02: show tutorial icon `cel`, or hide it when
// the argument is negative.
void func_080347c0(s32 cel) {
    if (cel < 0) {
        sprite_set_visible(gSpriteHandler, gMarchingOrders->tutorialSprite, FALSE);
        return;
    }
    sprite_set_visible(gSpriteHandler, gMarchingOrders->tutorialSprite, TRUE);
    sprite_set_anim_cel(gSpriteHandler, gMarchingOrders->tutorialSprite, cel);
}

// [func_0803481c] Engine Event 03: start scrolling the ground.
void func_0803481c(void) {
    gMarchingOrders->scrollBg = TRUE;
}

// [func_0803482c] Scroll BG layer 3 by one pixel per frame while enabled.
void func_0803482c(void) {
    if (gMarchingOrders->scrollBg) {
        D_03004b10.BG_OFS[3].x--;
    }
}

// [func_08034850] Engine Event 04: point instead of turning on a face command.
void func_08034850(u32 usePointAnims) {
    gMarchingOrders->usePointAnims = usePointAnims;
}

// [marching_engine_update] Game Engine Update
void marching_engine_update(void) {
    func_080346b0();
    func_0803482c();
    if (gMarchingOrders->commanderTimer != 0) {
        gMarchingOrders->commanderTimer--;
    }
}

// [marching_engine_stop] Game Engine Close
void marching_engine_stop(void) {
}

// [marching_cue_spawn] Cue - Spawn
void marching_cue_spawn(struct Cue *cue, struct MarchingOrdersCue *info, u32 command) {
    info->command = command;
}

// [marching_cue_update] Cue - Update.  Expire the cue once it has outlived its
// window by a bar and a half.
u32 marching_cue_update(struct Cue *cue, struct MarchingOrdersCue *info,
                        u32 runningTime, u32 duration) {
    return runningTime > (u32)ticks_to_frames(0x78);
}

// [marching_cue_despawn] Cue - Despawn
void marching_cue_despawn(struct Cue *cue, struct MarchingOrdersCue *info) {
}

// [marching_cue_hit] Cue - Hit
void marching_cue_hit(struct Cue *cue, struct MarchingOrdersCue *info,
                      u32 pressed, u32 released) {
    struct MarchingOrdersEngineData *data = gMarchingOrders;

    switch (info->command) {
        case 0:  // march
            play_sound(data->stepFoot ? &s_guntai_foot2_seqData : &s_guntai_foot1_seqData);
            func_0803494c();
            break;

        case 1:  // left face
            func_08034988();
            break;

        case 2:  // right face
            func_080349ac();
            break;

        case 3:  // halt — only lands while the player is mid-step
            if (data->marchers[3].stepTimer != 0) {
                play_sound(data->stepFoot ? &s_guntai_foot2_seqData : &s_guntai_foot1_seqData);
                func_080349d0();
            }
            break;

        default:
            break;
    }
}

// [marching_cue_barely] Cue - Barely
void marching_cue_barely(struct Cue *cue, struct MarchingOrdersCue *info,
                         u32 pressed, u32 released) {
    marching_cue_hit(cue, info, pressed, released);
    func_080346e0(2);
}

// [marching_cue_miss] Cue - Miss
void marching_cue_miss(struct Cue *cue, struct MarchingOrdersCue *info) {
    beatscript_enable_loops();
    func_080346e0(1);
}

// [func_0803494c] Player steps, alternating feet.
void func_0803494c(void) {
    func_080343b8(&gMarchingOrders->marchers[3],
                  gMarchingOrders->stepFoot ? MARCHING_ACT_STEP_L : MARCHING_ACT_STEP_R);
    gMarchingOrders->stepFoot ^= 1;
    gMarchingOrders->playerBusyTimer = ticks_to_frames(0xC);
}

// [func_08034988] Player faces left.
void func_08034988(void) {
    func_080343b8(&gMarchingOrders->marchers[3], MARCHING_ACT_FACE_LEFT);
    gMarchingOrders->playerBusyTimer = ticks_to_frames(0xC);
}

// [func_080349ac] Player faces right.
void func_080349ac(void) {
    func_080343b8(&gMarchingOrders->marchers[3], MARCHING_ACT_FACE_RIGHT);
    gMarchingOrders->playerBusyTimer = ticks_to_frames(0xC);
}

// [func_080349d0] Player halts.
void func_080349d0(void) {
    func_080343b8(&gMarchingOrders->marchers[3], MARCHING_ACT_STOP_BEAT);
    gMarchingOrders->playerBusyTimer = ticks_to_frames(0xC);
}

// [marching_input_event] Input Event — reached for input that did not satisfy
// a cue, so the player acts and the commander always objects.
void marching_input_event(u32 pressed, u32 released) {
    struct MarchingMarcher *player = &gMarchingOrders->marchers[3];

    if (pressed & A_BUTTON) {
        func_0803494c();
    }
    if ((pressed & B_BUTTON) && (player->stepTimer != 0)) {
        func_080349d0();
    }
    if (pressed & DPAD_LEFT) {
        func_08034988();
    }
    if (pressed & DPAD_RIGHT) {
        func_080349ac();
    }
    beatscript_enable_loops();
    func_080346e0(1);
}

// [marching_common_beat_animation] Common Event 0 (unimplemented on GBA too)
void marching_common_beat_animation(void) {
}

// [marching_common_display_text] Common Event 1
void marching_common_display_text(const char *text) {
    struct MarchingOrdersEngineData *data = gMarchingOrders;
    struct PrintedTextAnim *anim;

    if (text == NULL) {
        sprite_set_visible(gSpriteHandler, data->textSprite, FALSE);
        scene_hide_bg_layer(2);
        return;
    }

    anim = bmp_font_obj_print_c(data->textPrinter, text, 1, 0xC);
    delete_bmp_font_obj_text_anim(data->textPrinter, data->textSprite);
    sprite_set_anim(gSpriteHandler, data->textSprite, (struct Animation *)anim, 0, 0, 0, 0);
    sprite_set_visible(gSpriteHandler, data->textSprite, TRUE);
    scene_show_bg_layer(2);
}

// [marching_common_init_tutorial] Common Event 2 (unimplemented on GBA too)
void marching_common_init_tutorial(void) {
}

// [func_08034ae4] Engine Event 05: play one of the version's voice clips.
void func_08034ae4(u32 sound) {
    struct MarchingSfxData *sfx = &marching_sfx_table[gMarchingOrders->version][sound];

    play_sound_w_pitch_volume(sfx->sound, sfx->volume, sfx->pitch);
}
#endif /* PLATFORM_PC */
