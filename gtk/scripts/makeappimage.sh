#!/usr/bin/env bash

# Build an AppImage for the GTK port. Run from a configured build directory:
#   cd gtk/build && ../scripts/makeappimage.sh
#
# APPIMAGE_ARCH selects the target: x86_64 (default) or i686. The i686 build
# must run inside a 32-bit container (see makeappimage-x86-docker.sh).

set -e

APPIMAGE_ARCH="${APPIMAGE_ARCH:-x86_64}"
case "$APPIMAGE_ARCH" in
    x86_64) LD_ARCH=x86_64; OUT=super-snes9x-gtk-x86_64.AppImage ;;
    i686)   LD_ARCH=i386;   OUT=super-snes9x-gtk-x86.AppImage ;;
    *) echo "unsupported APPIMAGE_ARCH: $APPIMAGE_ARCH" >&2; exit 1 ;;
esac
# Arch hint for the tools: a 32-bit container on a 64-bit kernel still
# reports x86_64 from uname, so autodetection cannot be trusted.
export ARCH="$APPIMAGE_ARCH"

APPIMAGETOOL="appimagetool-${APPIMAGE_ARCH}.AppImage"
LINUXDEPLOY="linuxdeploy-${LD_ARCH}.AppImage"

if [ ! -f "$APPIMAGETOOL" ]; then
    wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/$APPIMAGETOOL"
    chmod +x "$APPIMAGETOOL"
fi

if [ ! -f "$LINUXDEPLOY" ]; then
    wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

DESTDIR=$PWD/AppDir cmake --install . --prefix /usr --strip
"./$LINUXDEPLOY" --appimage-extract-and-run --appdir=AppDir \
	--exclude-library="libX*" \
	--exclude-library="libglib*" \
	--exclude-library="libgobject*" \
	--exclude-library="libgdk_pixbuf*" \
	--exclude-library="libwayland*" \
	--exclude-library="libgmodule*" \
	--exclude-library="libgio*" \
	--exclude-library="libxcb*" \
	--exclude-library="libxkbcommon*" \
	--exclude-library="libdb*"

# Replace any copied root icon with a symlink into the icon theme dir
# (which root-level icon linuxdeploy leaves varies by version/arch).
rm -f AppDir/snes9x.png
pushd AppDir
ln -sf usr/share/icons/hicolor/256x256/apps/snes9x.png snes9x.png
chmod +x AppRun
popd
"./$APPIMAGETOOL" --appimage-extract-and-run AppDir "$OUT"
