# Rhythm Tengoku — PC port (rtpc)

**This is not the arcade project.** This repo is a fork of the GBA
decompilation [arthurtilly/rhythmtengoku](https://github.com/arthurtilly/rhythmtengoku)
with a native PC port layered on top. The SEGA NAOMI / SH-4 arcade
decompilation is a *separate* repo at `~/rhytngk-arcade-decomp`
(`Toritan123/rhytngk-arcade-decomp`) — different ROM, different CPU,
different tooling. Do not carry facts, addresses, or memories between them.

- `origin` = `Toritan123/rhythmtengoku-pc` (this fork, branch `main`)
- `upstream` = `arthurtilly/rhythmtengoku` (branch `master`)

## Ground rules

- **Never fabricate.** A translation you could not run is "translated,
  verified against the assembly" — say so. When a previous claim (even your
  own) turns out wrong, retract it explicitly in the commit message.
- The user replies in short Japanese commands ("続ける" = continue).
  **Report in Japanese; code comments and commit messages in English.**
- **Cross-platform by default**: macOS and Windows together, not macOS first.
- Never commit `build/`, `build_pc/`, `dist/`, `.DS_Store`, ROMs, or captures.

## Build and verify

```sh
make -f Makefile.pc app        # -> dist/Rhythm Tengoku.app
open "dist/Rhythm Tengoku.app"
```

**Verify by running the .app, not `build/macos/rhythmtengoku_pc`.** The
bundle is what ships, and the raw binary misses whole failure classes (e.g.
a wrong-architecture bundled dylib).

**A live process is not proof it works** — an app blocked on a modal
NSAlert stays alive forever, so `pgrep` proves nothing. Liveness is
*frames*:

```sh
pkill -9 -f rhythmtengoku; rm -f /tmp/rtpc_f*.bmp
open --env RTPC_SAVE=/tmp/rtpc_test.sav "dist/Rhythm Tengoku.app"
sleep 40; ls /tmp/rtpc_f*.bmp     # frames 120, 900, 1800 => it really ran
```

`open --env` works here and keeps the check off the player's save. 25 s is
not always enough to reach frame 900 — give it 40.

If nothing appears, `sample <pid>`: `runModal` in the stack = a dialog is up
(only the user can dismiss it); `agb_main` / `platform_frame_sync` = running.
Quit with `osascript -e 'tell application "Rhythm Tengoku" to quit'` —
`kill -9` seeds AppKit's crash history and the next launch stops on the
"reopen windows?" modal, which is your own test artifact, not a bug.

Knobs: `OPT`, `EXTRA_CFLAGS`, `EXTRA_LDFLAGS`, `SDL2_CONFIG` (the Makefile
picks the `sdl2-config` whose `libSDL2.dylib` matches `uname -m`; arm64 SDL2
lives at `/opt/homebrew`). Runtime env vars are all `RTPC_*` — see
`grep -rhoE 'RTPC_[A-Z_]+' platform/ src/`; the load-bearing ones are
`RTPC_FRAMEDUMP`, `RTPC_SCENE`, `RTPC_SHOT_DIR`, `RTPC_HEADLESS`,
`RTPC_AUDIO_LINEAR`, `RTPC_BS`, `RTPC_SAVE`.

## Saves

`gba_sram` is a plain 64 KB array. `platform/save_pc.c` loads it at startup and
writes it back (atomically, via a .tmp + rename) at most once a frame, and only
when the game actually touched SRAM. The file lives where SDL_GetPrefPath puts
it — on macOS `~/Library/Application Support/Rhythm Tengoku/rhythmtengoku.sav`.
Delete that file to test a fresh boot.

**Always set `RTPC_SAVE=<path>` when running the game yourself.** Automated
runs drive the game with synthetic input, which can wander into menus and
change real settings; that is the player's save, not a scratch file.
`RTPC_SAVE=/dev/null` disables saving entirely.

The SRAM read/write path itself has worked all along, in
`platform/asm_stubs.c` (`set_sram_fast_func`, `write_sram_fast`, `read_sram`) —
`src/lib_sram.c` is still assembly-only, but do **not** translate it into
`src/lib_sram.c` without first removing that shim, or the symbols collide.

## The dominant bug class

**A GBA-era size or width applied to a 64-bit pointer.** Four shapes seen:

1. Callback-argument slots typed `s32`/`u32` (`onFinishArg`, `callbackArg`,
   `ScheduledFunctionTask.param`, `globalVariable`) that callers pass
   pointers through → widen to `intptr_t`/`uintptr_t`. **Check the function
   pointer type too, not just the storage**: `struct Sprite::callbackArg` was
   already `uintptr_t`, but `callbackFunc` was declared
   `(struct SpriteHandler *, s16, u32, ...)`, so the call truncated the
   pointer anyway. Any engine whose sprite callback reads its third argument
   as a pointer crashed on it; rat_race was the first.
2. Function *return* types callers cast back to a pointer.
3. Allocations sized with `sizeof(u32)` for a pointer array.
4. Struct sizes baked into the decompiled data tables
   (`Scene.requiredMemory`, `GameEngine.gameDataSize`,
   `CueDefinition.cueInfoSize`) → `GBA_STRUCT_BYTES()` in `include/global.h`.

These crash far from their cause. The scan for shapes 1–2:

```sh
make -f Makefile.pc BUILD=build/warn EXTRA_CFLAGS="-Wpointer-to-int-cast -Wint-to-pointer-cast -Wvoid-pointer-to-int-cast"
```

For 3–4, use an **lldb watchpoint** -- but see below: lldb itself crashes on
this machine since the update to Darwin 27. **AddressSanitizer does not work on
this machine** — native arm64 it hangs in `get_dyld_hdr`, under Rosetta it
SIGILLs. That is Apple clang 17's runtime vs the macOS 26 dyld cache, not an
architecture problem. Don't retry it.

## Other things not to re-learn

- **Graphics are extracted already decompressed.** `platform/gfx_data.c`
  builds every `CompressedData` with `doubleCompressed = 0`, so the whole
  compression path (`decompress_gfx_rom`, the buffered-texture cache,
  `update_texture_loader_task`) is unreachable on PC. Translating it is
  correct work but expect **zero runtime change** — never attribute a
  rendering bug to it.
- Mach-O rejects a bare `section(".common_data")`; `COMMON_DATA` is empty
  under `PLATFORM_PC`.

## Measuring what is left

**Do not count `asm(...)` lines in `src/engines/*.c`** — every engine carries a
boilerplate `asm(".include \"include/gba.inc\"")`, so that grep returns all 36
and means nothing. (I made exactly this mistake and reported "20 engines still
contain assembly"; it was wrong.)

The real measure is *which weak stubs in `platform/auto_stubs.c` have no strong
definition* — those are the functions that silently no-op at run time:

```sh
make -f Makefile.pc app
nm -g build/macos/platform/auto_stubs.o | awk '$2=="T"{print $3}' | sed 's/^_//' | sort -u > /tmp/stub.txt
find build/macos -name '*.o' ! -name auto_stubs.o -exec nm -g {} \; \
  | awk '$2=="T"||$2=="D"||$2=="S"||$2=="B"||$2=="C"{print $3}' | sed 's/^_//' | sort -u > /tmp/real.txt
comm -23 /tmp/stub.txt /tmp/real.txt
```

Include the data types (`D`/`S`/`B`/`C`) on the second list — `scene_*` and
`script_studio_*` are data, and comparing against text symbols only reports
~200 of them as missing when they are not.

## State (measured 2026-10-08)

- **Every engine is translated** (2026-10-08). The weak stubs left are the
  intentional IWRAM-blob ones (`*_rom`, `func_08000a00`, `fast_blend_*`,
  `__umodsi3`, `midi_directsound_init`, `pc_fast_udivsi3_impl` -- each has
  a PC replacement) and `func_0803d2c0`..`func_0803d4e0`, which nothing
  references (orphans in auto_stubs.c, no run-time effect). No scene is a
  stub any more. From here the work is checking each game plays right.
- Done: `rhythm_tweezers`, `rhythm_test`, `clappy_trio`, `samurai_slice`,
  `rat_race`, `tap_trial`, `metronome`, `tram_and_pauline`, `polyrhythm`,
  `quiz_show`, `sick_beats`, `mechanical_horse`, `mannequin_factory`,
  `drum_studio` (lessons), `drum_intro` (Drum Samurai cutscenes,
  tanuki_and_monkey, staff credit intro), `ninja_bodyguard`, `bunny_hop`,
  `toss_boys`, `showtime`, DrumTech's `play_drumtech_note`,
  screen fades (`func_0800716c`), gradient backdrops (`func_08004070`).
- **Cleared end to end under `RTPC_AUTO=4`** (every cue hit, results
  screen reached): tanuki_and_monkey (100.0), rat_race (High Level),
  night_walk, space_dance, staff_credit, ninja_bodyguard/reincarnate,
  bunny_hop, toss_boys/toss_boys_2/remix_5, showtime (22/22 = every
  spawn_cue in its script), bouncy_road (all 4), bon_odori,
  bon_dance, cosmic_dance, clappy_trio, drum_girls_live_unused, the drum
  lessons (with RTPC_AUTO_IDLE=600). Use this as the acceptance test for
  an engine, not "it draws".
- **rat_race was "fully re-audited" on 2026-10-08 and still had a bug**: the
  cat's BG0 offset was passed as x instead of y, so the cat sat in view all
  game. Reading the asm did not catch it; comparing frames against the real
  ROM did (next item). Treat an audit by reading as weaker evidence than a
  pixel comparison. The other mismatch found then was the release filter in
  engine_start (`gameplay_set_input_buttons(A_BUTTON, A_BUTTON)`; the
  assembly passes a leftover R1 = 1). Without it the dash cue,
  `RELEASE_BUTTON(A_BUTTON)`, could never be hit. When translating, watch
  for arguments the assembly passes in a register it set for something
  else just before the call.
- **The real ROM can be run for comparison** (2026-10-09): mGBA and
  libmgba are installed (`/usr/local/lib/libmgba.dylib`, x86_64).
  `tools/mgba_ref.c` boots the ROM, retargets the warning screen's
  transition to any `struct Scene` address, replays a key log and saves
  PPMs. Scene addresses: follow each level name in
  `data/scenes/game_select/levels.inc.c` to its ROM string and the pointer
  before it (rat_race = 0x089d2c04). Recipe: run the PC game with
  `RTPC_KEYLOG=keys.txt RTPC_SHOTS=10`, find the frame offset with an
  input-free run (rat_race: PC shot n == mGBA frame n+28), shift the key log
  by offset+1, capture in mGBA, diff with PIL. rat_race then matches pixel
  for pixel except (a) anything from `agb_random` -- the GBA spins
  `get_agb_random_var()` while waiting for VBlank, so its RNG cannot be
  reproduced -- and (b) fades/text that wait for asset loading, which is
  instant on PC (a few frames early).
- `RTPC_AUTO=2` taps SELECT every 4 s, which *quits* a drum lesson
  ("セレクトde中止") -- use `RTPC_AUTO=1` (or 4) for lessons.
- **Game labels are resolved by `tools/bs2c.py`, not by
  `include/beatscript_consts.h`.** The `.set` names inside each `load_*`
  macro (`EVENT_05`, `CUE_TOM`, `PRINT_TEXT`...) collide between games, and
  the header keeps the first value it saw: staff_credit ran `PRINT_TEXT` as
  event 5 (crash), the drum lessons spawned `CUE_TOM` as 3 instead of 5,
  space_dance `CUE_TURN_RIGHT` as 2 instead of 0. bs2c now tracks
  `.set`/`.equ` in source order. Older .bs.c files still name labels
  symbolically where the header value happens to agree; that was checked
  field by field on 2026-10-08 and only the 20 scripts that differed were
  regenerated.
- **bs2c used to split macro args on any whitespace**, dropping everything
  after the first token of `table + (n * 0xC)` or `(a & 0xFF) | (b << 8)`.
  That broke tanuki_and_monkey's per-pattern text, power_calligraphy's brush
  y/state, and packed params in remix_6 / space_dance / cosmic_dance. Fixed;
  `table + byteOffset` into a local `.word` table is emitted as an element
  index, since the offset counts 4-byte GBA pointers.
- `drumtech_drum_bank` entries 44-56 have a NULL sound in the ROM too;
  `midi_player_play_header` ignores a NULL song on PC (the GBA reads the
  BIOS area as a header).
- **"Done" means no stub left for that engine — check it with the `comm`
  list above, filtered by the engine's address range, not by reading the
  .c file for `#else` blocks.** rat_race was reported done on 2026-09-23
  while 17 of its engine events (cat, sign, speech bubble, plates, goal,
  music fade) were still weak stubs; they sat in the global count but were
  never attributed to the engine. A quick per-file check: an
  `#ifndef PLATFORM_PC` / `#include "asm/..."` / `#endif` with no `#else`
  is a function with no PC body *unless* a NONMATCHING C version follows.
- **To turn a raw `D_030053c0 + 0xNNN` into a field name, anchor on a field's
  own absolute address, not on another field's offset comment.** Working back
  from `localVariables // [D_030053c0 + 0x160]` put `musicVolume` at 0x198 and
  the answer 8 bytes out; `musicVolume // [D_03005550]` settles it directly,
  since 0x03005550 - 0x030053c0 == 0x190. The `unk168`-style names agree with
  the absolute address, so that `+ 0x160` comment in `include/types.h` looks
  wrong by 8 — do not trust it.
- A cold boot under `RTPC_AUTO=1` sits in `scene_rhythm_test_opening`, and
  **that is correct behaviour, not a bug.** The click test loops for as long
  as the player keeps responding: each iteration begins with
  `beatscript_disable_loops` (assume this is the last one) and
  `rhythm_test_input_event` calls `beatscript_enable_loops` on every tap to
  keep it going. Autopilot never stops tapping, so it never ends. Note the
  names read backwards: `beatscript_enable_loops` sets `bypassLoops = FALSE`,
  which *keeps* a loop running; leaving one is `exitLoopNextUpdate`.
- **Scene-variable offsets**: beatscript op 0x09 writes into the scene data
  struct at an offset baked into the `.bs` source, computed for 4-byte GBA
  pointers. `tools/bs2c.py` now emits `offsetof()` instead (see
  `SCENE_VAR_FIELDS` there); add an entry when a new script writes a scene
  variable past a pointer. This is what froze the results screen: every
  `inputsEnabled` in `src/scenes` is assigned FALSE and never TRUE, because
  the script write was the only thing that set it.
- The `*_rom` stubs (`math_sqrt_rom`, `read_sram_fast_rom`, …) are IWRAM blobs
  the GBA copied at run time; they are stubbed **on purpose**, not a backlog.
- **The PPU did not implement windows or colour effects until 2026-10-08**
  (`platform/ppu.c`): WIN0/WIN1/OBJ window and BLDMOD alpha/brighten/darken,
  semi-transparent OBJs. Each pixel keeps its top two layers for that. If a
  game shows a layer where it should not, or something opaque that should be
  see-through, check the window and blend registers before the engine.
- `perfect` crashes intermittently (~1 run in 6), undiagnosed and pre-existing.
- **`text_printer` draws on PC** (2026-09-23). `text_print_glyph_to_vram_rom`
  (the ARM routine the GBA copies into IWRAM) is translated in C as
  `pc_print_glyph_to_vram` in `src/text_printer.c`; the `_rom` stub stays on
  purpose. Glyphs are 1bpp, 16 px wide, bit 4p+k = pixel p of row k. Mind
  ARM `LSR #32` == 0, which is undefined in C. The PC `func_08009de4` also
  built OBJ tile numbers as `X + (Y + 64) * 32`; the +64 set attr2 bit 11
  (priority) and put text behind BGs -- the options description box was the
  visible case. Two display bugs seen while checking this, both pre-existing
  and not text: brown tile garbage on the rhythm_tweezers onion (not seen in
  a screenshot after the PPU window/blend work on 2026-10-08; whether that
  fixed it is unverified), and the yellow "モノラル" label overlapping
  "ステレオ" in options -- the latter is in the cel data itself
  (`options_cel006` entries 6-7 place the palette-4 "モノラル" at (20, 0)
  behind the selected word), so most likely the intended look, not a bug.
- **`func_0800eebc` (OBJ font format parser) was a stub until 2026-10-07.**
  `scene_create_obj_font_printer` installs it on every OBJ font, so every
  string drawn through such a font came out empty -- rat_race's speech
  bubbles, marching_orders' bubble, and any other engine/scene in
  `grep -l scene_create_obj_font_printer src`. If OBJ text is missing,
  check `textObj->parseString` before the renderer.
- **The base ROM is on this machine**:
  `~/Downloads/2462 - Rhythm Tengoku (J)(WRG).gba`, sha1 matches
  `BASEROM_SHA1` in the Makefile. Read ROM bytes from it (offset = address
  - 0x08000000) instead of guessing table contents; Shift-JIS data in the
  .c files does not show up in a plain `grep -r`.
- **Things the 2026-10-07 pass found, worth checking first next time:**
  - `PC_GAME_STUB` scenes in platform/game_globals.c bounce straight back to
    game select. 7 real games hid there (Rap Men, Sick Beats, Remixes
    2/4/6/7, Polyrhythm 2); the 16 left are names nothing references.
  - `tools/bs2c.py` must read .bs as latin-1, emit `text` blocks, ignore '@'
    inside strings (Shift-JIS trail byte 0x40), and emit raw `.byte/.hword`
    script blocks. Regenerating the older 80 .bs.c changes 8 of them for
    unrelated reasons -- regenerate only what you add.
  - Raw GBA offsets in C macros: `GLOBAL_VARIABLE` was `&D_030053c0 + 0x24`
    and every `switch INT8, GLOBAL_VARIABLE` (random branches) matched no
    case. Grep for `+ 0x` against a struct base whenever a branch never fires.
  - The Makefile silences -Wimplicit-function-declaration. A missing
    prototype truncated create_affine_sprite's pointer (metronome crash).
    Rebuild with `EXTRA_CFLAGS=-Wimplicit-function-declaration` and check any
    implicitly called function that returns a pointer.
  - PPU: 32x64 maps read their lower half from the wrong screen block; HBlank
    palette DMA (gradient backdrops) is emulated via ppu_set_hblank_palette.
  - Engine data is allocated as GBA_STRUCT_BYTES and now zeroed in full.
  - `RTPC_AUDIO_DUMP=<wav>` (not headless) proves sound: drum_studio was RMS 0
    before play_drumtech_note existed.
  - "NOT TRANSLATED" placeholders are strong symbols and hide from the stub
    count: `grep -rn "NOT TRANSLATED" src`.
- **Data tables the ROM leaves unterminated** break on PC, because the GBA got
  away with whatever happened to follow in ROM. `rat_race_marking_criteria`
  was one (its .bs says `@! No criteria terminator`); `tools/bs2c.py` now
  appends END_OF_CRITERIA for it. Grep the .bs sources for `@!` comments
  before assuming a table is well formed.

## Scene overrides in platform/game_globals.c

Only `scene_studio`, `scene_title` and `scene_game_select` are still PC
rewrites. Everything else runs its real script out of `data/scenes/*.bs.c`.
Five further shims (warning, main_menu, options, data_room, cafe) and five
stub loop functions used to live here but were never wired to anything —
they were removed once the offset fix made them pointless. Before adding a
shim, check with `nm` which object actually defines the scene:

```sh
find build/macos -name '*.o' ! -name auto_stubs.o -print0 | xargs -0 nm -g -A \
  | grep ' [TDSB] _scene_<name>$'
```

`title` and `game_select` do **not** suffer the offset bug (`inputsEnabled`
sits at 0 and 4 with no pointer ahead of it); their shims exist for GFX
timing — `func_08007324(TRUE)` — so converting them to the real scripts is
its own job, not a consequence of the offset fix.

## Exercising one engine

`RTPC_SCENE=<name>` boots straight into a scene (`src/main.c`). Names come from
`gPcSceneTable` and have **no `scene_` prefix** — `rhythm_tweezers`, not
`scene_rhythm_tweezers`; an unknown name prints the whole list and exits.
Combine with `RTPC_AUTO=4` and `RTPC_HEADLESS=1`.

**`RTPC_AUTO=4` plays the cues** (2026-10-08): it presses each live cue's
button on the target frame, holds buttons the engine also judges on
release until the release cue, and only taps A (for menus and text) after
5 s with no live cue and no open play inputs. The drum lessons wait for the
player to start drumming on their own, so run them with
`RTPC_AUTO_IDLE=600` (A-tap after 10 s even with play inputs open); do not
make that the default -- in tanuki_and_monkey and rat_race those taps are
judged as stray input. `RTPC_AUTO=1/2` tap A blindly,
which in engines that treat an unrelated press as "do it again"
(tanuki_and_monkey) loops a practice forever and in rat_race costs the rank.
**`RTPC_CUETRACE=1`** logs every spawn, hit/barely with its offset, miss and
stray input with a frame number -- a run with 0 misses and 0 strays that
reaches `scene_results_*` is the strongest evidence available without lldb:

```sh
RTPC_SAVE=/dev/null RTPC_SCENE=night_walk RTPC_AUTO=4 RTPC_CUETRACE=1 RTPC_TRACE=1 \
  ./build/macos/rhythmtengoku_pc 2> log.txt &   # kill it after the results scene
```

**Do not use `RTPC_AUTO=3` to exercise an engine.** It mashes every button, so
it keeps hitting the A+B+START+SELECT soft-reset combo: the game ping-pongs
between `scene_title` and `scene_soft_reset`, the frame counter never leaves
`f=0`, and not one cue ever spawns. `RTPC_AUTO=2` (tap A, plus SELECT to skip
tutorials) reaches real gameplay — 70 cue spawns in ~70 s in rhythm_tweezers.
`RTPC_TRACE=1` is what makes the reset loop visible.

Headless
disables the renderer, so `RTPC_SHOTS`/`RTPC_FRAMEDUMP` produce nothing there —
use lldb breakpoints to prove a function was reached:

```sh
breakpoint set -n <func> --auto-continue true
breakpoint command add -s python -o "print('>>>HIT'); return False" 1
```

Note `timeout(1)` does not exist on this machine; background the process and
`kill` it instead.

**lldb crashes on `target create` since the OS moved to Darwin 27**
(lldb-1703.0.236.103), so the breakpoint recipe above does not currently
work. What does, without adding code:
- the built-in crash handler prints a symbolised backtrace on SIGSEGV to the
  log, which found both rat_race crashes;
- `RTPC_TRACE=1` scene transitions show how far a run got;
- `RTPC_SHOTS=N` with `RTPC_SHOT_DIR` and a pixel diff of consecutive frames
  (PIL `ImageChops.difference(...).getbbox()`) proves something moved.
