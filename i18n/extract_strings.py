#!/usr/bin/env python3
"""Unified i18n string extractor for Snes9x.

Scans the win32 UI sources (rsrc/snes9x.rc menus + dialog controls, wlanguage.h
macros), reduces each string to its normalized lookup key, and appends any key
not already present to i18n/snes9x.pot and every i18n/<lang>.po. Existing entries
(GTK strings and translations) are never rewritten, only new win32 keys appended.

Usage:  python3 i18n/extract_strings.py [--dry-run]
"""

import glob
import html
import os
import re
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
RC = os.path.join(ROOT, "win32", "rsrc", "snes9x.rc")
WLANG = os.path.join(ROOT, "win32", "wlanguage.h")
POT = os.path.join(SCRIPT_DIR, "snes9x.pot")

TEXT_CONTROLS = (
    "LTEXT", "RTEXT", "CTEXT", "PUSHBUTTON", "DEFPUSHBUTTON",
    "GROUPBOX", "AUTOCHECKBOX", "AUTORADIOBUTTON", "CHECKBOX",
    "RADIOBUTTON", "CONTROL",
)

C_ESCAPES = {"t": "\t", "n": "\n", "r": "\r", "a": "\a", "b": "\b",
             "f": "\f", "v": "\v", "0": "\0", "\\": "\\", '"': '"', "'": "'"}


def unescape_c(s):
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            out.append(C_ESCAPES.get(s[i + 1], s[i + 1]))
            i += 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def unescape_rc(s):
    return unescape_c(s.replace('""', '"'))


def normalize_key(s):
    head = s.split("\t", 1)[0].rstrip()
    if head.endswith("…"):
        head = head[:-1].rstrip()
    else:
        m = re.search(r"\.{3,}$", head)
        if m:
            head = head[:m.start()].rstrip()
    head = head.replace("&&", "\x00").replace("&", "").replace("\x00", "&")
    return head.strip()


def keep(key):
    if not key:
        return False
    if not re.search(r"[A-Za-z]", key):
        return False
    if re.fullmatch(r"%[\-0-9.]*[A-Za-z]", key):
        return False
    return True


def extract_win32():
    keys = {}

    def add(raw, src):
        key = normalize_key(raw)
        if keep(key) and key not in keys:
            keys[key] = src

    rc_text = open(RC, encoding="utf-8", errors="replace").read()
    rc_str = r'"((?:[^"]|"")*)"'
    for m in re.finditer(r'(?:MENUITEM|POPUP)\s+' + rc_str, rc_text):
        if m.group(1) and m.group(1) != "SEPARATOR":
            add(unescape_rc(m.group(1)), "win32/rsrc/snes9x.rc")
    for m in re.finditer(r'\bCAPTION\s+' + rc_str, rc_text):
        add(unescape_rc(m.group(1)), "win32/rsrc/snes9x.rc")
    ctrl_re = re.compile(r"^\s*(" + "|".join(TEXT_CONTROLS) + r")\s+" + rc_str, re.M)
    for m in ctrl_re.finditer(rc_text):
        if m.group(2):
            add(unescape_rc(m.group(2)), "win32/rsrc/snes9x.rc")

    wl_text = open(WLANG, encoding="utf-8", errors="replace").read()
    for m in re.finditer(r'TEXT\(\s*"((?:[^"\\]|\\.)*)"\s*\)', wl_text):
        add(unescape_c(m.group(1)), "win32/wlanguage.h")

    sources = sorted(glob.glob(os.path.join(ROOT, "win32", "*.cpp")) +
                     glob.glob(os.path.join(ROOT, "win32", "*.h")))
    for path in sources:
        src = open(path, encoding="utf-8", errors="replace").read()
        rel = "win32/" + os.path.basename(path)
        for m in re.finditer(r'_L\(\s*(?:_T|TEXT)\s*\(\s*"((?:[^"\\]|\\.)*)"', src):
            add(unescape_c(m.group(1)), rel)
        for m in re.finditer(r'_L\(\s*"((?:[^"\\]|\\.)*)"', src):
            add(unescape_c(m.group(1)), rel)

    for path in sorted(glob.glob(os.path.join(ROOT, "qt", "src", "*.cpp"))):
        src = open(path, encoding="utf-8", errors="replace").read()
        rel = "qt/src/" + os.path.basename(path)
        for m in re.finditer(r'\btr\(\s*"((?:[^"\\]|\\.)*)"', src):
            add(unescape_c(m.group(1)), rel)
    for path in sorted(glob.glob(os.path.join(ROOT, "qt", "src", "*.ui"))):
        src = open(path, encoding="utf-8", errors="replace").read()
        rel = "qt/src/" + os.path.basename(path)
        for m in re.finditer(r'<string(?![^>]*notr="true")(?:\s+[^>]*)?>([^<]*)</string>', src):
            add(html.unescape(m.group(1)), rel)

    return keys


PO_ESC = [("\\", "\\\\"), ('"', '\\"'), ("\t", "\\t"), ("\r", "\\r"), ("\n", "\\n")]


def po_escape(s):
    for a, b in PO_ESC:
        s = s.replace(a, b)
    return s


def po_unescape(s):
    out, i = [], 0
    rev = {"t": "\t", "n": "\n", "r": "\r", '"': '"', "\\": "\\"}
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            out.append(rev.get(s[i + 1], s[i + 1]))
            i += 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def parse_po_token(tok):
    tok = tok.strip()
    if tok.startswith('"') and tok.endswith('"'):
        return po_unescape(tok[1:-1])
    return ""


def existing_msgids(path):
    ids = set()
    if not os.path.exists(path):
        return ids
    cur = None
    for raw in open(path, encoding="utf-8", errors="replace"):
        line = raw.strip()
        if line.startswith("msgid "):
            cur = parse_po_token(line[6:])
        elif line.startswith("msgid_plural ") or line.startswith("msgstr"):
            if cur is not None:
                ids.add(cur)
            cur = None
        elif line.startswith('"') and cur is not None:
            cur += parse_po_token(line)
        elif not line:
            cur = None
    return ids


def append_block(path, keys, srcs):
    new = [k for k in keys if k not in existing_msgids(path)]
    if not new:
        return 0
    body = open(path, encoding="utf-8", errors="replace").read()
    chunk = []
    if not body.endswith("\n"):
        chunk.append("\n")
    chunk.append("\n# === Win32 strings (added by i18n/extract_strings.py) ===\n")
    for k in new:
        chunk.append("\n#: %s\n" % srcs[k])
        chunk.append('msgid "%s"\n' % po_escape(k))
        chunk.append('msgstr ""\n')
    with open(path, "a", encoding="utf-8", newline="") as f:
        f.write("".join(chunk))
    return len(new)


def main():
    dry = "--dry-run" in sys.argv
    keys = extract_win32()
    sorted_keys = sorted(keys, key=lambda s: s.lower())
    print("win32 normalized keys: %d" % len(sorted_keys))

    targets = [POT] + sorted(
        os.path.join(SCRIPT_DIR, f)
        for f in os.listdir(SCRIPT_DIR)
        if f.endswith(".po")
    )
    for t in targets:
        present = existing_msgids(t)
        already = sum(1 for k in sorted_keys if k in present)
        todo = len(sorted_keys) - already
        name = os.path.basename(t)
        if dry:
            print("  %-16s already=%-4d new=%-4d" % (name, already, todo))
        else:
            n = append_block(t, sorted_keys, keys)
            print("  %-16s already=%-4d appended=%-4d" % (name, already, n))


if __name__ == "__main__":
    main()
