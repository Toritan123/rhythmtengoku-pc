/*
 * Definitions for game-state global variables that live in EWRAM/IWRAM on GBA
 * but have no C definition in the decompiled source.  On PC they are simply
 * zero-initialised static data.
 */
#ifdef PLATFORM_PC

#include <stdio.h>
#include "global.h"
#include "include/types.h"
#include "include/scenes.h"
#include "src/code_0800b778.h"
#include "src/code_08001360.h"
#include "src/scenes/cafe.h"
#include "src/scenes/riq_main_scene.h"
#include "src/scenes/warning.h"
#include "src/scenes/title.h"
#include "src/code_080068f8.h"
#include "include/sequence_data.h"
#include "src/scenes/main_menu.h"
#include "src/scenes/game_select.h"
#include "src/scenes/options.h"
#include "src/scenes/data_room.h"
#include "src/scenes/gameplay.h"
#include "src/scenes/results.h"
#include "src/memory.h"
#include "include/levels.h"

// Forward-declare game engines (defined in games/*/engine.c).
extern struct GameEngine karate_man_engine;

// [0x030055A0] Play Session info – written by cafe_scene_init_memory
struct PlaySessionInfo gSessionInfo = {{0}};

// Pause menu state – written by the pause handler
struct PauseMenu gPauseMenu = {0};

// Scene descriptors – used as targets for set_next_scene().
struct Scene scene_debug_menu = {0};

// Studio: not yet implemented – return to main_menu immediately.
static u32 pc_studio_loop(void) {
    set_next_scene(&scene_main_menu);
    return TRUE;
}
struct Scene scene_studio = {
    /* initFunc  */ NULL,                          /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_studio_loop,     /* loopParam  */ 0,
    /* endFunc   */ NULL,                          /* endParam   */ 0,
    /* memory    */ 0,
};


// ─── Warning Scene ────────────────────────────────────────────────────────────
// PC beatscript: waits ~3s for GFX+fade, then enables input via scene_set_byte,
// then waits indefinitely (the update function calls stop_beatscript_scene when
// the player presses a key or the 60-second timer expires).
static struct Beatscript script_scene_warning_pc[] = {
    /* rest 150 ticks (~3 s, lets GFX load and palette fade in finish) */
    { BS_CMD_REST, 0, 0, 150 },
    /* scene_set_byte 0, TRUE  →  gWarning->inputsEnabled = 1
       cmd=0x09, type=INT8(0), offset=0, value=TRUE(1) */
    { 0x09, /*INT8*/0, /*offset*/0, /*TRUE*/1 },
    /* Large rest: pc_warning_update will call stop_beatscript_scene() first */
    { BS_CMD_REST, 0, 0, 0x7FFF },
    /* stop (safety sentinel) */
    { BS_CMD_STOP, 0, 0, 0 },
};

// PC-specific update: when inputsEnabled and (key pressed OR timer expired),
// immediately stop all beatscript threads so the scene transition fires.
// On GBA the equivalent is set_pause_beatscript_scene(FALSE) which resumes
// the paused script; on PC we stop the threads directly instead.
static void pc_warning_update(void *sVar, s32 dArg) {
    if (gWarning->inputsEnabled) {
        if ((D_03004afc != 0) || (--gWarning->timer == 0)) {
            stop_beatscript_scene();
        }
    }
}

static struct SubScene sub_scene_warning_pc = {
    /* startFunc  */ (void (*)())warning_scene_start,  /* startParam  */ 0,
    /* pausedFunc */ NULL,                              /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_warning_update,    /* updateParam */ 0,
    /* stopFunc   */ (void (*)())warning_scene_stop,   /* stopParam   */ 0,
    /* script     */ script_scene_warning_pc,
};

struct Scene scene_warning = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_warning_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct WarningSceneData),
};


// ─── Title Scene ─────────────────────────────────────────────────────────────
// Minimal PC beatscript for the title scene.
static struct Beatscript script_scene_title_pc[] = {
    /* rest long */ { BS_CMD_REST, 0, 0, 3600 },
    /* stop      */ { BS_CMD_STOP, 0, 0, 0    },
};

// PC title update: replicates what the GBA beatscript does after GFX loads.
// On GBA: beatscript unpauses → calls func_08007324(TRUE) → palette fade →
//         title_logo_appear → title_scene_complete_intro → enable inputs.
// On PC we drive this manually with a frame counter.
static s32 pc_title_frames = 0;  // reset by pc_title_start each visit

static void pc_title_start(void *sVar, s32 dArg) {
    pc_title_frames = 0;
    title_scene_start(sVar, dArg);
    // Start title BGM immediately (on GBA this is done by the ROM beatscript).
    scene_set_music(&s_title_bgm_seqData);
    // Sync scriptBaseBPM to musicBaseBPM so playerSpeed = 1.0x (no pitch-shift).
    // scene_set_music() sets musicBaseBPM but leaves scriptBaseBPM at its old
    // value, causing the MIDI player to run at scriptBPM/musicBPM rate.
    set_beatscript_tempo(D_030053c0.musicBaseBPM);
}

static void pc_title_update(void *sVar, s32 dArg) {
    pc_title_frames++;

    // Enable display-buffer flush after GFX init has fired (init_gfx2 schedules
    // at frame 2; give it a few more frames for GFX loading to begin).
    // func_08007324(TRUE) sets D_03004b10.updateDisplay so that func_08006e88
    // will copy the shadow DISPCNT (set by scene_set_bg_layer_display /
    // scene_show_obj_layer) into the real REG_DISPCNT each frame.
    if (pc_title_frames == 10) {
        func_08007324(TRUE);
    }

    // After ~2 s show the title logo and enable player input.
    if (pc_title_frames == 120) {
        title_logo_appear();
        title_scene_complete_intro();
        gTitle->inputsEnabled = TRUE;
    }

    // Delegate to the normal per-frame logic (timer, demo timer, input).
    title_scene_update(sVar, dArg);
}

static struct SubScene sub_scene_title_pc = {
    /* startFunc  */ (void (*)())pc_title_start,    /* startParam  */ 0,
    /* pausedFunc */ NULL,                           /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_title_update,   /* updateParam */ 0,
    /* stopFunc   */ (void (*)())title_scene_stop,  /* stopParam   */ 0,
    /* script     */ script_scene_title_pc,
};

struct Scene scene_title = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_title_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct TitleSceneData),
};


// ─── Beatscript data stubs ────────────────────────────────────────────────────
// On GBA these are ROM arrays; on PC we provide immediate-STOP sentinels.
struct Beatscript epilogue_end_script[1]            = {{ BS_CMD_STOP }};
struct Beatscript script_scene_studio_exit[1]       = {{ BS_CMD_STOP }};
struct Beatscript script_scene_studio_idle[1]       = {{ BS_CMD_STOP }};
struct Beatscript script_scene_studio_start_song[1] = {{ BS_CMD_STOP }};
// [D_089cfda4] Generic gameplay fade-out sequence. The beatscript engine jumps
// here (func_0801d968) when a game ends; the previous const u8[4] definition
// meant the engine then executed whatever globals followed it in memory.
struct Beatscript D_089cfda4[1]                     = {{ BS_CMD_STOP }};
// [D_089e2ad4] / [D_089e2b04] – Drum Studio scripts. Previously defined as
// const u8[4] while declared as struct Beatscript[]: any walk of these would
// have read past the end of the object.
struct Beatscript D_089e2ad4[1]                     = {{ BS_CMD_STOP }};
struct Beatscript D_089e2b04[1]                     = {{ BS_CMD_STOP }};
struct Beatscript script_scene_title_exit[1]        = {{ BS_CMD_STOP }};


// ─── Stub scenes for unimplemented GBA scenes ─────────────────────────────────
// On GBA these are full scene implementations in ROM.  On PC we provide
// minimal stubs whose loopFunc immediately returns TRUE (scene ended), causing
// the scene manager to transition to the configured next scene.  If no next
// scene is configured, D_08935fb0 (= &scene_title) is used as a fallback.
//
// IMPORTANT: auto_stubs.c previously defined these as weak *functions*; that
// caused the scene manager to interpret x86 instruction bytes as a struct Scene
// and crash.  These strong data definitions override the weak stubs.

static u32 pc_stub_scene_loop(void) { return TRUE; }

// Demo cutscene (auto-demo from title): skip directly back → falls through to
// D_08935fb0 (title), which is the correct GBA behaviour.
struct Scene scene_drum_samurai_demo_cutscene = {
    /* initFunc  */ NULL,                          /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_stub_scene_loop, /* loopParam  */ 0,
    /* endFunc   */ NULL,                          /* endParam   */ 0,
    /* memory    */ 0,
};

// Rhythm Test: not yet implemented – return to main_menu immediately.
static u32 pc_rhythm_test_loop(void) {
    set_next_scene(&scene_main_menu);
    return TRUE;
}
struct Scene scene_rhythm_test = {
    /* initFunc  */ NULL,                             /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_rhythm_test_loop,  /* loopParam  */ 0,
    /* endFunc   */ NULL,                             /* endParam   */ 0,
    /* memory    */ 0,
};

// Opening cutscene (played on first A-press from title): skip to main_menu.
// On GBA this transitions naturally; on PC we set the target explicitly.
static u32 pc_opening_cutscene_loop(void) {
    set_next_scene(&scene_main_menu);
    return TRUE;
}

struct Scene scene_drum_samurai_opening_cutscene = {
    /* initFunc  */ NULL,                               /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_opening_cutscene_loop, /* loopParam */ 0,
    /* endFunc   */ NULL,                               /* endParam   */ 0,
    /* memory    */ 0,
};


// ─── Main Menu Scene ──────────────────────────────────────────────────────────
// PC beatscript: enable display after GFX loads, then wait for scene logic.
static struct Beatscript script_scene_main_menu_pc[] = {
    { BS_CMD_REST, 0, 0, 3600 },
    { BS_CMD_STOP, 0, 0, 0    },
};

static s32 pc_main_menu_frames = 0;

static void pc_main_menu_start(void *sVar, s32 dArg) {
    pc_main_menu_frames = 0;
    main_menu_scene_start(sVar, dArg);
    // Start main-menu BGM (on GBA the ROM beatscript plays s_manza_bgm_seqData).
    scene_set_music(&s_manza_bgm_seqData);
    set_beatscript_tempo(D_030053c0.musicBaseBPM);
}

static void pc_main_menu_update(void *sVar, s32 dArg) {
    pc_main_menu_frames++;
    if (pc_main_menu_frames == 10) {
        func_08007324(TRUE);
    }
    if (pc_main_menu_frames == 60) {
        gMainMenu->inputsEnabled = TRUE;
    }
    u8 prev_inputs = gMainMenu->inputsEnabled;
    main_menu_scene_update(sVar, dArg);
    if (prev_inputs && !gMainMenu->inputsEnabled) {
        stop_beatscript_scene();
    }
}

static struct SubScene sub_scene_main_menu_pc = {
    /* startFunc  */ (void (*)())pc_main_menu_start,   /* startParam  */ 0,
    /* pausedFunc */ (void (*)())main_menu_scene_paused, /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_main_menu_update,  /* updateParam */ 0,
    /* stopFunc   */ (void (*)())main_menu_scene_stop, /* stopParam   */ 0,
    /* script     */ script_scene_main_menu_pc,
};

struct Scene scene_main_menu = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_main_menu_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct MainMenuSceneData),
};


// ─── Game Select Scene ────────────────────────────────────────────────────────
// PC beatscript: minimal – real timing driven by pc_game_select_update.
static struct Beatscript script_scene_game_select_pc[] = {
    { BS_CMD_REST, 0, 0, 3600 },
    { BS_CMD_STOP, 0, 0, 0    },
};

static s32 pc_game_select_frames = 0;

static void pc_game_select_start(void *sVar, s32 dArg) {
    pc_game_select_frames = 0;
    game_select_scene_start(sVar, dArg);
    // BGM: play_game_select_bgm() sets tempo 152 then scene_set_music sets musicBaseBPM.
    // Re-sync scriptBPM to musicBaseBPM so the player speed is 1.0.
    play_game_select_bgm();
    set_beatscript_tempo(D_030053c0.musicBaseBPM);
    // On GBA the beatscript exit sequence fades and returns; on PC we go back to
    // main_menu when the scene ends (B-button cancel in game_select_scene_update).
    set_scene_trans_target(&scene_game_select, &scene_main_menu);
    // PC safety: ensure at least Karate Man is playable regardless of save state.
    if (D_030046a8 && D_030046a8->data.levelStates[LEVEL_KARATE_MAN] == LEVEL_STATE_HIDDEN) {
        D_030046a8->data.levelStates[LEVEL_KARATE_MAN] = LEVEL_STATE_OPEN;
    }
    // PC: place cursor on Karate Man (grid 2,11) so A launches it immediately.
    if (gGameSelect) {
        gGameSelect->cursorX = 2;
        gGameSelect->cursorY = 11;
        game_select_move_cursor_to_grid_xy(2, 11);
        game_select_set_info_pane_to_cursor_target();
    }
}

static void pc_game_select_update(void *sVar, s32 dArg) {
    pc_game_select_frames++;
    // start_new_texture_loader is now a real task (platform/asm_stubs.c) so the GFX
    // chain completes naturally: gfx1→gfx2→gfx3→gfx4, clearing loadingSceneGfx ~frame 4.
    //
    // Enable display output a few frames after GFX init completes.  The GBA version
    // does this via the beatscript fade-in; on PC we do it directly.
    if (pc_game_select_frames == 10) {
        func_08007324(TRUE);
    }
    if (pc_game_select_frames == 60) {
        gGameSelect->inputsEnabled = TRUE;
    }
    u8 prev_inputs = gGameSelect->inputsEnabled;
    game_select_scene_update(sVar, dArg);
    if (prev_inputs && !gGameSelect->inputsEnabled) {
        stop_beatscript_scene();
    }
}

static struct SubScene sub_scene_game_select_pc = {
    /* startFunc  */ (void (*)())pc_game_select_start,   /* startParam  */ 0,
    /* pausedFunc */ (void (*)())game_select_scene_paused, /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_game_select_update,  /* updateParam */ 0,
    /* stopFunc   */ (void (*)())game_select_scene_stop, /* stopParam   */ 0,
    /* script     */ script_scene_game_select_pc,
};

struct Scene scene_game_select = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_game_select_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct GameSelectSceneData),
};


// ─── Options Menu Scene ───────────────────────────────────────────────────────
// On GBA the ROM beatscript lowers the volume of the already-playing
// s_manza_bgm_seqData and raises it again on exit.  On PC we just let it play.
static struct Beatscript script_scene_options_menu_pc[] = {
    { BS_CMD_REST, 0, 0, 3600 },
    { BS_CMD_STOP, 0, 0, 0    },
};

static s32 pc_options_frames = 0;

static void pc_options_start(void *sVar, s32 dArg) {
    pc_options_frames = 0;
    options_scene_start(sVar, dArg);
}

static void pc_options_update(void *sVar, s32 dArg) {
    pc_options_frames++;
    if (pc_options_frames == 10) {
        func_08007324(TRUE);
    }
    if (pc_options_frames == 30) {
        gOptionsMenu->inputsEnabled = TRUE;
    }
    u8 prev_inputs = gOptionsMenu->inputsEnabled;
    options_scene_update(sVar, dArg);
    if (prev_inputs && !gOptionsMenu->inputsEnabled) {
        stop_beatscript_scene();
    }
}

static struct SubScene sub_scene_options_menu_pc = {
    /* startFunc  */ (void (*)())pc_options_start,       /* startParam  */ 0,
    /* pausedFunc */ (void (*)())options_scene_paused,   /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_options_update,      /* updateParam */ 0,
    /* stopFunc   */ (void (*)())options_scene_stop,     /* stopParam   */ 0,
    /* script     */ script_scene_options_menu_pc,
};

struct Scene scene_options_menu = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_options_menu_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct OptionsSceneData),
};


// ─── Rhythm Data Room Scene ───────────────────────────────────────────────────
// On GBA the ROM beatscript plays s_siryo_bgm_seqData.
static struct Beatscript script_scene_data_room_pc[] = {
    { BS_CMD_REST, 0, 0, 3600 },
    { BS_CMD_STOP, 0, 0, 0    },
};

static s32 pc_data_room_frames = 0;

static void pc_data_room_start(void *sVar, s32 dArg) {
    pc_data_room_frames = 0;
    dataroom_scene_start(sVar, dArg);
    scene_set_music(&s_siryo_bgm_seqData);
    set_beatscript_tempo(D_030053c0.musicBaseBPM);
}

static void pc_data_room_update(void *sVar, s32 dArg) {
    pc_data_room_frames++;
    if (pc_data_room_frames == 10) {
        func_08007324(TRUE);
    }
    if (pc_data_room_frames == 30) {
        gDataRoom->inputsEnabled = TRUE;
    }
    u8 prev_inputs = gDataRoom->inputsEnabled;
    dataroom_scene_update(sVar, dArg);
    if (prev_inputs && !gDataRoom->inputsEnabled) {
        stop_beatscript_scene();
    }
}

static struct SubScene sub_scene_data_room_pc = {
    /* startFunc  */ (void (*)())pc_data_room_start,     /* startParam  */ 0,
    /* pausedFunc */ (void (*)())dataroom_scene_paused,  /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_data_room_update,    /* updateParam */ 0,
    /* stopFunc   */ (void (*)())dataroom_scene_stop,    /* stopParam   */ 0,
    /* script     */ script_scene_data_room_pc,
};

struct Scene scene_data_room = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_data_room_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct DataRoomSceneData),
};


// ─── Cafe (Barista) Scene ─────────────────────────────────────────────────────
// On GBA the ROM beatscript drives a dialogue loop: prints dialogue, waits,
// enables inputs (A to advance), loops.  On PC we replicate this in
// pc_cafe_update by directly calling cafe_print_dialogue /
// cafe_start_dialogue_inputs and watching textAdvReady / bypassLoops.
static struct Beatscript script_scene_cafe_pc[] = {
    { BS_CMD_REST, 0, 0, 0x7FFF },  // safety: will be cut short by stop_beatscript_scene
    { BS_CMD_STOP, 0, 0, 0         },
};

static s32 pc_cafe_frames        = 0;
static u8  pc_cafe_dialogue_done = 0; // 1 after first cafe_print_dialogue call
static u8  pc_cafe_prev_adv_ready = 0; // previous value of textAdvReady

static void pc_cafe_start(void *sVar, s32 dArg) {
    pc_cafe_frames        = 0;
    pc_cafe_dialogue_done = 0;
    pc_cafe_prev_adv_ready = 0;
    cafe_scene_start(sVar, dArg);
}

static void pc_cafe_update(void *sVar, s32 dArg) {
    pc_cafe_frames++;

    if (pc_cafe_frames == 10) {
        func_08007324(TRUE);
        scene_set_music(&s_counseling_bgm_seqData);
        set_beatscript_tempo(D_030053c0.musicBaseBPM);
    }

    // After GFX has loaded (~60 frames), start the first dialogue and
    // enable input.
    if (pc_cafe_frames == 60 && !pc_cafe_dialogue_done) {
        gCafe->inputsEnabled = TRUE;
        cafe_print_dialogue();
        cafe_start_dialogue_inputs();
        pc_cafe_dialogue_done = 1;
        pc_cafe_prev_adv_ready = 1; // cafe_start_dialogue_inputs sets textAdvReady=TRUE
    }

    // Detect the falling edge of textAdvReady (A was just pressed).
    // cafe_scene_update → cafe_update_dialogue_inputs clears it; we see
    // it as 0 in the frame AFTER the press.
    if (pc_cafe_dialogue_done && gCafe->inputsEnabled) {
        u8 adv_ready = gCafe->textAdvReady;
        if (pc_cafe_prev_adv_ready && !adv_ready) {
            // A was pressed last frame.
            if (D_030053c0.bypassLoops) {
                // All dialogue exhausted – end the scene.
                stop_beatscript_scene();
            } else {
                // Advance to next dialogue beat.
                cafe_print_dialogue();
                cafe_start_dialogue_inputs();
                adv_ready = 1; // cafe_start_dialogue_inputs sets it TRUE
            }
        }
        pc_cafe_prev_adv_ready = adv_ready;
    }

    cafe_scene_update(sVar, dArg);
}

static struct SubScene sub_scene_cafe_pc = {
    /* startFunc  */ (void (*)())pc_cafe_start,       /* startParam  */ 0,
    /* pausedFunc */ (void (*)())cafe_scene_paused,   /* pausedParam */ 0,
    /* updateFunc */ (void (*)())pc_cafe_update,      /* updateParam */ 0,
    /* stopFunc   */ (void (*)())cafe_scene_stop,     /* stopParam   */ 0,
    /* script     */ script_scene_cafe_pc,
};

struct Scene scene_cafe = {
    /* initFunc  */ (void (*)())func_0801d86c, /* initParam  */ &sub_scene_cafe_pc,
    /* loopFunc  */ (u32 (*)())func_0801d8d8,  /* loopParam  */ NULL,
    /* endFunc   */ NULL,                       /* endParam   */ NULL,
    /* memory    */ sizeof(struct CafeSceneData),
};


// ─── Results / Epilogue Scene Stubs ──────────────────────────────────────────
// On GBA these are full scenes with rankings, animations, etc.
// On PC they immediately return to game_select so the player can keep playing.

static u32 pc_results_loop(void) {
    set_next_scene(&scene_game_select);
    return TRUE;
}



struct Scene scene_results_ver_score = {
    /* initFunc  */ NULL,                              /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_results_loop,        /* loopParam  */ 0,
    /* endFunc   */ NULL,                              /* endParam   */ 0,
    /* memory    */ 0,
};

static u32 pc_epilogue_loop(void) {
    set_next_scene(&scene_game_select);
    return TRUE;
}

struct Scene scene_epilogue = {
    /* initFunc  */ NULL,                              /* initParam  */ 0,
    /* loopFunc  */ (u32 (*)())pc_epilogue_loop,       /* loopParam  */ 0,
    /* endFunc   */ NULL,                              /* endParam   */ 0,
    /* memory    */ 0,
};


// ─── PC Beatscript Helpers ────────────────────────────────────────────────────
// Macros for constructing PC-native beatscript arrays.
// The Beatscript struct fields:
//   command (u8), param1 (u8), src (const void*), param3 (u32)
// Commands:
//   0x00 REST N      — wait N beat-ticks (1 beat = 24 ticks)
//   0x01 STOP        — end thread
//   0x03 RUN fn,arg  — call fn(arg);  fn stored in src, arg in param3
//   0x0D CALL scr    — push currentCmd+1, jump to scr (stored in src)
//   0x0E RETURN      — pop from jump stack, resume caller
//   0x0F GOTO scr    — unconditional jump to scr (no push)
//   0x10 LOOP_START  — mark loop start
//   0x11 LOOP_END    — jump to loop start, or exit if bypassLoops is set
//   0x4C TEMPO bpm   — call set_beatscript_tempo(bpm)

// Beatscript struct on PC: { command:8|param1:24, param2 (const void*), param3 (uintptr_t) }
#define PC_BS_REST(n)       { BS_CMD_REST, 0, NULL, (n) }
#define PC_BS_STOP          { BS_CMD_STOP, 0, NULL, 0 }
#define PC_BS_RUN(fn, arg)  { 0x03, 0, (const void*)(fn), (uintptr_t)(arg) }
#define PC_BS_CALL(scr)     { 0x0D, 0, (const void*)(scr), 0 }
#define PC_BS_RETURN        { 0x0E, 0, NULL, 0 }
#define PC_BS_GOTO(scr)     { 0x0F, 0, (const void*)(scr), 0 }
#define PC_BS_LOOP_START    { 0x10, 0, NULL, 0 }
#define PC_BS_LOOP_END      { 0x11, 0, NULL, 0 }
#define PC_BS_TEMPO(bpm)    { 0x4C, 0, NULL, (bpm) }




// ─── Generic gameplay stub for all other unimplemented games ─────────────────
// Returns immediately to game_select so the player can choose another game.
static u32 pc_game_stub_loop(void) {
    set_next_scene(&scene_game_select);
    return TRUE;
}

// Declare all unimplemented game scenes as proper structs.
// On GBA they come from assembled .bs files; on PC we provide stubs.
#define PC_GAME_STUB(name) \
    struct Scene name = { NULL, 0, (u32(*)())pc_game_stub_loop, 0, NULL, 0, 0 }

PC_GAME_STUB(scene_polyrhythm);
PC_GAME_STUB(scene_polyrhythm_2);
PC_GAME_STUB(scene_bouncy_road);
PC_GAME_STUB(scene_bouncy_road_2);
PC_GAME_STUB(scene_tap_trial);
PC_GAME_STUB(scene_tap_trial_2);
PC_GAME_STUB(scene_clappy_trio);
PC_GAME_STUB(scene_clappy_trio_2);
PC_GAME_STUB(scene_space_dance);
PC_GAME_STUB(scene_space_dance_2);
PC_GAME_STUB(scene_bunny_hop);
PC_GAME_STUB(scene_bunny_hop_2);
PC_GAME_STUB(scene_toss_boys);
PC_GAME_STUB(scene_toss_boys_2);
PC_GAME_STUB(scene_rat_race);
PC_GAME_STUB(scene_rat_race_2);
PC_GAME_STUB(scene_marching_orders);
PC_GAME_STUB(scene_marching_orders_2);
PC_GAME_STUB(scene_night_walk);
PC_GAME_STUB(scene_night_walk_2);
PC_GAME_STUB(scene_mr_upbeat);
PC_GAME_STUB(scene_mr_upbeat_2);
PC_GAME_STUB(scene_metronome);
PC_GAME_STUB(scene_metronome_2);
PC_GAME_STUB(scene_ninja_bodyguard);
PC_GAME_STUB(scene_ninja_bodyguard_2);
PC_GAME_STUB(scene_spaceball);
PC_GAME_STUB(scene_spaceball_2);
PC_GAME_STUB(scene_fireworks);
PC_GAME_STUB(scene_fireworks_2);
PC_GAME_STUB(scene_quiz_show);
PC_GAME_STUB(scene_quiz_show_2);
PC_GAME_STUB(scene_power_calligraphy);
PC_GAME_STUB(scene_power_calligraphy_2);
PC_GAME_STUB(scene_rap_men);
PC_GAME_STUB(scene_rap_men_2);
PC_GAME_STUB(scene_samurai_slice);
PC_GAME_STUB(scene_samurai_slice_2);
PC_GAME_STUB(scene_rhythm_tweezers);
PC_GAME_STUB(scene_rhythm_tweezers_2);
PC_GAME_STUB(scene_wizards_waltz);
PC_GAME_STUB(scene_wizards_waltz_2);
PC_GAME_STUB(scene_bon_odori);
PC_GAME_STUB(scene_bon_odori_2);
PC_GAME_STUB(scene_mechanical_horse);
PC_GAME_STUB(scene_mechanical_horse_2);
PC_GAME_STUB(scene_showtime);
PC_GAME_STUB(scene_tram_and_pauline);
PC_GAME_STUB(scene_mannequin_factory);
PC_GAME_STUB(scene_drum_live);
PC_GAME_STUB(scene_sick_beats);
PC_GAME_STUB(scene_sneaky_spirits);
PC_GAME_STUB(scene_sneaky_spirits_2);
// Remix stages
PC_GAME_STUB(scene_remix_1);
PC_GAME_STUB(scene_remix_2);
PC_GAME_STUB(scene_remix_3);
PC_GAME_STUB(scene_remix_4);
PC_GAME_STUB(scene_remix_5);
PC_GAME_STUB(scene_remix_6);
PC_GAME_STUB(scene_remix_7);
PC_GAME_STUB(scene_remix_8);
// Unused variants listed in scenes.h
PC_GAME_STUB(scene_bouncy_road_unused);
PC_GAME_STUB(scene_bouncy_road_unused_2);
PC_GAME_STUB(scene_tap_trial_unused);

#endif // PLATFORM_PC



