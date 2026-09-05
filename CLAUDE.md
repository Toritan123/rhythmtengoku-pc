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
open "dist/Rhythm Tengoku.app"
sleep 25; ls /tmp/rtpc_f*.bmp     # frames 120 and 900 => it really ran
```

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
`RTPC_AUDIO_LINEAR`, `RTPC_BS`.

## The dominant bug class

**A GBA-era size or width applied to a 64-bit pointer.** Four shapes seen:

1. Callback-argument slots typed `s32`/`u32` (`onFinishArg`, `callbackArg`,
   `ScheduledFunctionTask.param`, `globalVariable`) that callers pass
   pointers through → widen to `intptr_t`/`uintptr_t`.
2. Function *return* types callers cast back to a pointer.
3. Allocations sized with `sizeof(u32)` for a pointer array.
4. Struct sizes baked into the decompiled data tables
   (`Scene.requiredMemory`, `GameEngine.gameDataSize`,
   `CueDefinition.cueInfoSize`) → `GBA_STRUCT_BYTES()` in `include/global.h`.

These crash far from their cause. The scan for shapes 1–2:

```sh
make -f Makefile.pc BUILD=build/warn EXTRA_CFLAGS="-Wpointer-to-int-cast -Wint-to-pointer-cast -Wvoid-pointer-to-int-cast"
```

For 3–4, use an **lldb watchpoint**. **AddressSanitizer does not work on
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

## State (measured 2026-09-05)

- **387 functions exist only as a no-op stub**: 152 named engine/system
  functions, 210 unnamed `func_08XXXXXX`, 23 scene/script data, 2 assets.
- Engines needing 2 or fewer: `clappy_trio`, `mechanical_horse`, `metronome`,
  `tap_trial`, `tram_pauline` (2 each); `quiz_show`, `drum_studio` (1 each).
  `rhythm_tweezers` is done as of this session.
- Largest remaining: `drum_intro` 19, `rat_race` 18, `toss_boys` /
  `rhythm_test` / `mannequin` / `bunny_hop` 14 each.
- The `*_rom` stubs (`math_sqrt_rom`, `read_sram_fast_rom`, …) are IWRAM blobs
  the GBA copied at run time; they are stubbed **on purpose**, not a backlog.
- `perfect` crashes intermittently (~1 run in 6), undiagnosed and pre-existing.

## Exercising one engine

`RTPC_SCENE=<name>` boots straight into a scene (`src/main.c`). Names come from
`gPcSceneTable` and have **no `scene_` prefix** — `rhythm_tweezers`, not
`scene_rhythm_tweezers`; an unknown name prints the whole list and exits.
Combine with `RTPC_AUTO=2` and `RTPC_HEADLESS=1`.

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
