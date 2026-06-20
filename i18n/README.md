# Snes9x translations (shared `i18n/` folder)

This folder holds the translation catalog shared by **all** Snes9x front-ends
(win32, GTK, and Qt later). Each `<lang>.po` pairs an English source string
(`msgid`) with its translation (`msgstr`); `snes9x.pot` is the template listing
every translatable string. English is never stored here — it is the built-in
baseline, so a missing/empty translation simply falls back to English.

## How the win32 build uses these
The win32 build reads `<lang>.po` directly at runtime from an `i18n/` folder
placed next to `snes9x.exe`. The on-disk `.po` files are also what drives the
**Translations** menu: each `<lang>.po` present adds its language. Delete them and
the app runs in English. No rebuild is needed to add or edit a translation.

## How the GTK build uses these
GTK compiles `<lang>.po` → `.mo` via CMake (`gtk/CMakeLists.txt`) and loads it
with gettext. To add a language to the GTK build, add its code to the
`foreach(lang …)` loop there.

## Updating the string template
Run the unified extractor to pull win32 strings into the catalog (adds any new
`msgid` to `snes9x.pot` and every `<lang>.po`, never overwriting existing
translations):
```sh
python3 i18n/extract_strings.py
```
To refresh GTK strings as well (requires gettext), `gtk/po/updatepot.sh` merges
them into `snes9x.pot` with `xgettext -j` (join, non-destructive).

## Adding or editing a translation
* Open `<lang>.po` in [poedit](https://poedit.net) (or a UTF-8 text editor) and
  fill in the empty `msgstr` lines, **or**
* create a new `<lang>.po` from `snes9x.pot` (e.g. `msgmerge`/poedit).

Avoid a region suffix unless genuinely needed: prefer `xx.po`; only add an
`xx_XX.po` for region-specific overrides, and only the differing messages.

## Source File Encoding
All `.po`/`.pot` files must be UTF-8 encoded.
