#!/usr/bin/env bash

# Build an AppImage for the Qt port. Run from a configured build directory:
#   cd qt/build && ../scripts/makeappimage.sh
#
# The resulting AppImage inherits the glibc requirement of the machine that
# built it, so for shipping run this inside the oldest distro you want to
# support (Ubuntu 22.04 is the practical minimum — the oldest LTS whose
# archive carries Qt 6).

set -e

# Let the AppImage tools work without FUSE (also required inside containers).
export APPIMAGE_EXTRACT_AND_RUN=1

if [ ! -f appimagetool-x86_64.AppImage ]; then
    wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
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
./linuxdeploy-x86_64.AppImage --appdir=AppDir --plugin qt
./appimagetool-x86_64.AppImage AppDir super-snes9x-qt-x86_64.AppImage
