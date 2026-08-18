#!/usr/bin/env python3
"""
bs2c — translate GBA beatscript assembly (.bs) into C for the PC build.

The .bs files are only assembled by the GBA build (Makefile: $(AS) on *.bs).
The PC build has no assembler step, which is why every scene had to be
hand-written as a stub in platform/game_globals.c.

Every .bs macro — however high-level — expands to exactly one primitive:

    beatscript_cmd <op>, <param1>, <param2>, <param3>
        .word (op & 0xff) | (param1 << 8)
        .word param2
        .word param3

which is field-for-field the PC `struct Beatscript`
{command:8, param1:24, const void *param2, uintptr_t param3}.  So the
translation is 1:1; this tool just expands the macros and prints C.

Macro definitions are read from the .inc files at run time rather than being
hardcoded, so the tool stays correct if the macro set changes.

Usage:  tools/bs2c.py <file.bs> [-o out.c]
        tools/bs2c.py --all            (translate every .bs in the tree)
"""

import argparse
import glob
import os
import re
import sys

INC_FILES = ["include/beatscript_main.inc", "include/beatscript.inc"]

# Macros we interpret ourselves instead of expanding.
STRUCTURAL = {"script", "struct", "endstruct", "text", "endtext", "beatscript_cmd"}


def strip_comments(line):
    line = re.sub(r"/\*.*?\*/", " ", line)      # C-style, used inside macro bodies
    line = re.sub(r"@.*$", "", line)            # asm line comment
    return line.strip()


def split_args(s):
    """Split on commas that are not inside parentheses."""
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


def split_macro_args(s):
    """Macro invocation args: separated by commas *or* whitespace (GNU as),
    but never inside parentheses."""
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if depth == 0 and (ch == "," or ch.isspace()):
            if cur.strip():
                out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


index_objects = set()


def build_symbol_index(root):
    """identifier -> header that declares it.

    The generated files reference game-specific symbols (engine functions,
    animations, sound banks).  Rather than hardcoding a path mapping, scan the
    headers once and include exactly what each file needs.
    """
    index = {}
    # Symbols whose declaration is a plain object — not an array, not a
    # function, not already a pointer.  Using one as an address needs an
    # explicit &, where an array or function name decays by itself.  Deciding
    # this from the header beats guessing from the name, which is what the
    # earlier *_seqData / scene_* special cases were doing.
    objects = index_objects
    pats = [
        # extern <type> name(...)   /  extern <type> *name;  /  extern <type> name[];
        re.compile(r"^\s*extern\s+[^;()]*?\b(\w+)\s*\("),
        re.compile(r"^\s*extern\s+[^;()]*?\b(\w+)\s*(?:\[[^\]]*\])?\s*;"),
        # struct/array definitions in headers
        re.compile(r"^\s*(?:const\s+)?struct\s+\w+\s+\*?(\w+)\s*(?:\[[^\]]*\])?\s*;"),
    ]
    for base in ("include", "src", "data", "games"):
        for dirpath, _, files in os.walk(os.path.join(root, base)):
            for fn in files:
                if not fn.endswith(".h"):
                    continue
                p = os.path.join(dirpath, fn)
                rel = os.path.relpath(p, root)
                try:
                    txt = open(p, errors="replace").read()
                except OSError:
                    continue
                for line in txt.split("\n"):
                    for i, pat in enumerate(pats):
                        m = pat.match(line)
                        if m:
                            index.setdefault(m.group(1), rel)
                            decays = (i == 0                      # function
                                      or "[" in line              # array
                                      or re.search(r"\*\s*" + re.escape(m.group(1)), line))
                            if not decays:
                                objects.add(m.group(1))
                            break
    return index


def collect_includes(path, root, seen=None):
    """Follow the .include chain of a source file, exactly as the assembler
    would, so game-specific macro files are picked up without a hardcoded map."""
    if seen is None:
        seen = []
    for raw in open(path, errors="replace"):
        m = re.match(r'\s*\.include\s+"([^"]+)"', raw)
        if not m:
            continue
        inc = m.group(1)
        if inc in seen:
            continue
        seen.append(inc)
        full = os.path.join(root, inc)
        if os.path.exists(full):
            collect_includes(full, root, seen)
    return seen


def load_macros(root, includes=None):
    """name -> (param_names, defaults, body_lines)"""
    macros = {}
    for inc in (includes if includes is not None else INC_FILES):
        path = os.path.join(root, inc)
        if not os.path.exists(path):
            continue
        txt = open(path, errors="replace").read()
        for m in re.finditer(r"^\.macro\s+(\S+)([^\n]*)\n(.*?)^\.endm", txt, re.S | re.M):
            name = m.group(1)
            params, defaults = [], {}
            for p in split_args(m.group(2).strip()):
                if "=" in p:
                    k, v = p.split("=", 1)
                    params.append(k.strip())
                    defaults[k.strip()] = v.strip()
                elif p:
                    params.append(p.strip())
            body = [strip_comments(l) for l in m.group(3).split("\n")]
            macros[name] = (params, defaults, [l for l in body if l])
    return macros


def expand(line, macros, depth=0):
    """Expand a source line into a list of terminal lines."""
    if depth > 24:
        raise RuntimeError(f"macro recursion too deep: {line}")
    line = strip_comments(line)
    if not line:
        return []

    head = line.split(None, 1)
    name = head[0]
    rest = head[1] if len(head) > 1 else ""

    if name in STRUCTURAL or name not in macros:
        return [line]

    params, defaults, body = macros[name]
    args = split_macro_args(rest)
    binding = {}
    for i, p in enumerate(params):
        binding[p] = args[i] if i < len(args) and args[i] != "" else defaults.get(p, "0")

    out = []
    for bl in body:
        sub = bl
        # longest names first so \arg1 isn't clobbered by \arg
        for p in sorted(binding, key=len, reverse=True):
            sub = sub.replace("\\" + p, binding[p])
        for piece in sub.split(";"):
            out.extend(expand(piece, macros, depth + 1))
    return out


class Emitter:
    """Collects labelled blocks and renders them as C definitions."""

    def __init__(self):
        self.blocks = []      # (kind, name, items)
        self.kind = None
        self.name = None
        self.items = []
        self.warnings = []

    def flush(self):
        if self.kind and self.items:
            self.blocks.append((self.kind, self.name, self.items))
        self.kind, self.name, self.items = None, None, []

    def start(self, kind, name):
        self.flush()
        self.kind, self.name, self.items = kind, name, []

    def add(self, item):
        if self.kind:
            self.items.append(item)

    # ---- rendering -------------------------------------------------------

    OBJECT_SYMBOL = re.compile(r"^[A-Za-z_]\w*$")

    def obj_ref(self, tok):
        """A struct field that stores the address of another object
        (e.g. Scene.initParam -> the SubScene).  Arrays decay by themselves;
        plain structs need an explicit &."""
        t = tok.strip()
        if t in ("0", "NULL") or t in getattr(self, "consts", ()):
            return "NULL"
        if re.match(r"^[A-Za-z_]\w*$", t):
            return f"&{t}"
        return t

    def int_or_zero(self, tok):
        """SubScene's callback parameters are s32, so NULL is a type error."""
        t = tok.strip()
        return "0" if t in ("NULL", "0") else f"(s32)({self.ref(t)})"

    def classify_local(self):
        """Register the blocks this file defines itself.

        index_objects only knows what the headers declare, so a Scene or
        SubScene defined in the same .bs was missing its & — 25 of the 49
        remaining failures.  Classify them the same way render_struct does."""
        self.local_objects = set()
        for kind, name, items in self.blocks:
            if kind != "struct":
                continue          # scripts and text render as arrays: they decay
            words  = [v for k, v in items if k == "w"]
            hwords = [v for k, v in items if k == "h"]
            single = (len(words) == 9 and not hwords) or \
                     ((len(words) == 6 and len(hwords) == 1) or
                      (len(words) == 7 and not hwords)) or \
                     (len(words) == 2 and len(hwords) == 4)
            if single:
                self.local_objects.add(name)

    def ref(self, tok):
        """Render an operand that the assembler would have stored as an address."""
        t = tok.strip()
        if not re.match(r"^[A-Za-z_]\w*$", t):
            return t
        # A plain object needs its address taken; arrays and functions decay.
        # Header declarations feed index_objects; blocks defined in this same
        # file are classified by classify_local().
        if t in getattr(self, "local_objects", ()) or t in index_objects:
            return f"&{t}"
        return t

    def render_script(self, name, cmds):
        out = [f"const struct Beatscript {name}[] = {{"]
        for op, p1, p2, p3 in cmds:
            p2 = "NULL" if p2.strip() in ("0", "NULL") else f"(const void *)({self.ref(p2)})"
            out.append(f"    {{ {op}, {p1}, {p2}, (uintptr_t)({self.ref(p3)}) }},")
        out.append("};")
        return "\n".join(out)

    def render_struct(self, name, items):
        words  = [v for k, v in items if k == "w"]
        hwords = [v for k, v in items if k == "h"]

        # SubScene: 4 (func, param) pairs + script pointer
        if len(words) == 9 and not hwords:
            f = words
            return (f"const struct SubScene {name} = {{\n"
                    f"    /* start  */ (void (*)()){f[0]}, {self.int_or_zero(f[1])},\n"
                    f"    /* paused */ (void (*)()){f[2]}, {self.int_or_zero(f[3])},\n"
                    f"    /* update */ (void (*)()){f[4]}, {self.int_or_zero(f[5])},\n"
                    f"    /* stop   */ (void (*)()){f[6]}, {self.int_or_zero(f[7])},\n"
                    f"    /* script */ {f[8]},\n"
                    f"}};")

        # Scene: 3 (func, param) pairs + required memory.
        # The memory field is written as .hword by define_gameplay_scene and as
        # .word by the hand-written scene structs.
        if (len(words) == 6 and len(hwords) == 1) or (len(words) == 7 and not hwords):
            f = words
            mem = hwords[0] if hwords else words[6]
            return (f"struct Scene {name} = {{\n"
                    f"    /* init   */ (void (*)()){f[0]}, (void *)({self.obj_ref(f[1])}),\n"
                    f"    /* loop   */ (u32 (*)()){f[2]}, (void *)({f[3]}),\n"
                    f"    /* end    */ (void (*)()){f[4]}, (void *)({f[5]}),\n"
                    f"    /* memory */ {mem},\n"
                    f"}};")

        # MarkingCriteria: 2 remark pointers + a packed flags hword + 3 thresholds.
        # struct MarkingCriteria splits that first hword into flags:u8 and
        # checkAverageMisses:u8.
        if len(words) == 2 and len(hwords) == 4:
            return (f"const struct MarkingCriteria {name} = {{\n"
                    f"    /* positive */ {words[0]},\n"
                    f"    /* negative */ {words[1]},\n"
                    f"    /* flags    */ ({hwords[0]}) & 0xFF, (({hwords[0]}) >> 8) & 0xFF,\n"
                    f"    /* minHitsForSuccess   */ {hwords[1]},\n"
                    f"    /* minHitsBeforeFail   */ {hwords[2]},\n"
                    f"    /* maxMissesBeforeFail */ {hwords[3]},\n"
                    f"}};")

        # A NULL/END_OF_CRITERIA-terminated table of MarkingCriteria pointers —
        # this is what import_marking_criteria receives.
        if words and not hwords and words[-1].strip() in ("END_OF_CRITERIA", "NULL", "0"):
            body = "".join(f"    (const struct MarkingCriteria *)({self.obj_ref(w)}),\n" for w in words)
            return f"const struct MarkingCriteria *{name}[] = {{\n{body}}};"

        self.warnings.append(f"{name}: unrecognised struct shape "
                             f"({len(words)} words, {len(hwords)} hwords) — skipped")
        return None

    def referenced(self):
        """Every identifier appearing in the rendered operands."""
        names = set()
        for kind, name, items in self.blocks:
            for it in items:
                for tok in it if isinstance(it, tuple) else (it,):
                    for w in re.findall(r"[A-Za-z_]\w*", str(tok)):
                        names.add(w)
        return names

    def render(self, src, index=None):
        import re as _re
        self.classify_local()
        extra = []
        if index:
            hdrs = {index[n] for n in self.referenced() if n in index}
            base = {"include/global.h", "src/main.h", "src/audio.h",
                    "src/scenes/gameplay.h", "src/scenes/results.h",
                    "src/code_0800b778.h"}
            extra = sorted(h for h in hdrs if h not in base)
        # Symbols that no header declares (engine tables, remark strings, …).
        # They are only ever used as addresses here, so an opaque array
        # declaration is enough; anything genuinely missing shows up as a link
        # error, which is exactly the list worth working through.
        defined = {nm for _, nm, _ in self.blocks}
        consts = getattr(self, "consts", set())
        unknown = sorted(n for n in self.referenced()
                         if index is not None and n not in index and n not in defined
                         and n not in consts and not n[0].isdigit()
                         and n not in ("NULL", "TRUE", "FALSE", "uintptr_t", "void",
                                       "const", "struct", "u32", "u16", "s32"))
        fwd = [f"extern char {n}[];   /* undeclared in any header */" for n in unknown]

        # Scripts may `call`/`goto` labels defined later in the same file.
        for kind, nm, items in self.blocks:
            if kind == "script":
                fwd.append(f"extern const struct Beatscript {nm}[];")
            elif kind == "struct":
                w = [v for k, v in items if k == "w"]
                h = [v for k, v in items if k == "h"]
                if len(w) == 9 and not h:
                    fwd.append(f"extern const struct SubScene {nm};")
                elif (len(w) == 6 and len(h) == 1) or (len(w) == 7 and not h):
                    fwd.append(f"extern struct Scene {nm};")
                elif len(w) == 2 and len(h) == 4:
                    fwd.append(f"extern const struct MarkingCriteria {nm};")
                elif w and not h and w[-1].strip() in ("END_OF_CRITERIA", "NULL", "0"):
                    fwd.append(f"extern const struct MarkingCriteria *{nm}[];")
        parts = [
            "/* Generated by tools/bs2c.py from %s — do not edit. */" % src,
            '#include "global.h"',
            '#include "src/main.h"',
            '#include "src/audio.h"',
            '#include "src/scenes/gameplay.h"',
            '#include "src/scenes/results.h"',
            '#include "src/code_0800b778.h"',
            '#include "include/beatscript_consts.h"',
        ] + [f'#include "{h}"' for h in extra] + [""] + fwd + [""]
        for kind, name, items in self.blocks:
            if kind == "script":
                parts.append(self.render_script(name, items))
            elif kind == "struct":
                s = self.render_struct(name, items)
                if s:
                    parts.append(s)
            parts.append("")
        return "\n".join(parts)


KNOWN_DIRECTIVES = (".include", ".section", ".end", ".balign", ".align",
                    ".set", ".equ", ".global", ".globl", ".text", ".data",
                    ".byte", ".2byte", ".4byte", ".ltorg", ".syntax", ".arm",
                    ".thumb", ".pool", "glabel")


def translate(path, macros, root, index=None, consts=None):
    em = Emitter()
    em.consts = consts or set()
    # Macros come from this file's own .include chain (plus the always-present
    # core set), so a game's macros.inc is honoured without any hardcoding.
    incs = collect_includes(path, root)
    if incs:
        macros = load_macros(root, INC_FILES + [i for i in incs if i not in INC_FILES])
    for raw in open(path, errors="replace"):
        for line in expand(raw, macros):
            tok = line.split(None, 1)
            head = tok[0]
            rest = tok[1].strip() if len(tok) > 1 else ""

            if head in (".include", ".section", ".end", ".balign", ".align", "glabel"):
                continue
            if head == "script":
                em.start("script", rest)
            elif head == "struct":
                em.start("struct", rest)
            elif head == "text":
                em.start("text", rest)
            elif head in ("endstruct", "endtext"):
                em.flush()
            elif head == "beatscript_cmd":
                a = split_args(rest)
                if len(a) == 4 and em.kind == "script":
                    em.add(tuple(a))
            elif head == ".word" and em.kind == "struct":
                for v in split_args(rest):
                    em.add(("w", v))
            elif head == ".hword" and em.kind == "struct":
                for v in split_args(rest):
                    em.add(("h", v))
            elif head in (".ascii", ".asciz", ".string") and em.kind == "text":
                # Text blocks are already present as hand-written C string
                # tables (games/*/\*_text.c); nothing to emit from here.
                pass
            elif head.endswith(":") or head in KNOWN_DIRECTIVES:
                pass
            elif head in (".word", ".hword"):
                pass  # outside a struct: nothing to emit
            else:
                # An unexpanded mnemonic means a macro we never loaded.  Left
                # silent this drops a whole command from the script (this is
                # how load_karate_man once vanished), so make it loud.
                em.warnings.append(f"unexpanded line dropped: {line!r}")
    em.flush()
    return em.render(os.path.relpath(path, root), index), em


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="*")
    ap.add_argument("-o", "--output")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--root", default=".")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    macros = load_macros(root)   # baseline; translate() re-derives per file
    index = build_symbol_index(root)
    consts = set(re.findall(r"^#define (\w+)", open(os.path.join(root,"include/beatscript_consts.h"),errors="replace").read(), re.M)) \
             if os.path.exists(os.path.join(root,"include/beatscript_consts.h")) else set()
    print(f"// {len(index)} symbols indexed", file=sys.stderr)
    print(f"// {len(macros)} macros loaded", file=sys.stderr)

    files = args.files
    if args.all:
        files = sorted(glob.glob(os.path.join(root, "**", "*.bs"), recursive=True))

    total_warn = 0
    for f in files:
        c, em = translate(f, macros, root, index, consts)
        out = args.output or (os.path.splitext(f)[0] + ".bs.c")
        if args.output == "-":
            print(c)
        else:
            open(out, "w").write(c)
        for w in em.warnings:
            print(f"  warn {os.path.basename(f)}: {w}", file=sys.stderr)
        total_warn += len(em.warnings)
        print(f"{os.path.relpath(f, root)} -> {len(em.blocks)} blocks", file=sys.stderr)
    print(f"// {len(files)} files, {total_warn} warnings", file=sys.stderr)


if __name__ == "__main__":
    main()
