#!/usr/bin/env bash

# Build an AppImage for the Qt port. Run from a configured build directory:
#   cd qt/build && ../scripts/makeappimage.sh
#
# APPIMAGE_ARCH selects the target: x86_64 (default) or i686. The i686 build
# must run inside a 32-bit container (see makeappimage-x86-docker.sh).
#
# The resulting AppImage inherits the glibc requirement of the machine that
# built it, so for shipping run this inside the oldest distro you want to
# support (Ubuntu 22.04 is the practical minimum for x86_64 — the oldest LTS
# whose archive carries Qt 6; Debian 12 is the only current i386 archive
# with Qt 6).

set -e

# Let the AppImage tools work without FUSE (also required inside containers).
export APPIMAGE_EXTRACT_AND_RUN=1

APPIMAGE_ARCH="${APPIMAGE_ARCH:-x86_64}"
case "$APPIMAGE_ARCH" in
    x86_64) LD_ARCH=x86_64; OUT=super-snes9x-qt-x86_64.AppImage ;;
    i686)   LD_ARCH=i386;   OUT=super-snes9x-qt-x86.AppImage ;;
    *) echo "unsupported APPIMAGE_ARCH: $APPIMAGE_ARCH" >&2; exit 1 ;;
esac
# Arch hint for the tools: a 32-bit container on a 64-bit kernel still
# reports x86_64 from uname, so autodetection cannot be trusted.
export ARCH="$APPIMAGE_ARCH"

APPIMAGETOOL="appimagetool-${APPIMAGE_ARCH}.AppImage"
LINUXDEPLOY="linuxdeploy-${LD_ARCH}.AppImage"
PLUGIN_QT="linuxdeploy-plugin-qt-${LD_ARCH}.AppImage"

if [ ! -f "$APPIMAGETOOL" ]; then
    wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/$APPIMAGETOOL"
    chmod +x "$APPIMAGETOOL"
fi

if [ ! -f "$LINUXDEPLOY" ]; then
    wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$PLUGIN_QT" ]; then
    wget "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$PLUGIN_QT"
    chmod +x "$PLUGIN_QT"
fi

# linuxdeploy-plugin-qt locates Qt through qmake.
if [ -z "$QMAKE" ]; then
    for candidate in qmake6 qmake-qt6 qmake; do
        if command -v "$candidate" > /dev/null; then
            QMAKE=$(command -v "$candidate")
            export QMAKE
            break
        fi
    done
fi

rm -rf AppDir
DESTDIR=$PWD/AppDir cmake --install . --prefix /usr --strip
"./$LINUXDEPLOY" --appdir=AppDir --plugin qt
"./$APPIMAGETOOL" AppDir "$OUT"
