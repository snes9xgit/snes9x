#!/usr/bin/env bash

# Build a 32-bit (i686) Snes9x GTK AppImage inside a Debian 12 i386
# container (glibc 2.36 floor — matches the Debian-based 32-bit distros
# still in use: antiX, MX, Debian). Same baseline as the 32-bit Qt build.
# Requires Docker; runs natively on an x86_64 host, no QEMU needed.
#
#   gtk/scripts/makeappimage-x86-docker.sh
#
# Output: gtk/build-appimage-x86/super-snes9x-gtk-x86.AppImage

set -e

REPO=$(cd "$(dirname "$0")/../.." && pwd)

docker run --rm --platform linux/386 \
    -v "$REPO":/snes9x \
    -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" \
    debian:bookworm bash -ec '
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
    build-essential cmake ninja-build pkg-config git ca-certificates wget file \
    gettext \
    libgtkmm-3.0-dev libsdl2-dev libpng-dev zlib1g-dev libminizip-dev \
    libcurl4-openssl-dev libxrandr-dev libx11-dev libxext-dev libxv-dev \
    libgl-dev libegl-dev libpulse-dev libasound2-dev portaudio19-dev \
    libwayland-dev wayland-protocols libxkbcommon-dev

# Always build from scratch: reproducible, and sidesteps stale-state issues.
rm -rf /snes9x/gtk/build-appimage-x86
mkdir -p /snes9x/gtk/build-appimage-x86
cd /snes9x/gtk/build-appimage-x86
# USE_SLANG=OFF: the bundled vulkan-hpp headers do not compile for 32-bit
# targets (handle types differ on 32-bit), and 32-bit machines have no
# usable Vulkan drivers anyway. OpenGL/XV output remains.
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_SLANG=OFF
ninja
APPIMAGE_ARCH=i686 ../scripts/makeappimage.sh
chown -R "$HOST_UID:$HOST_GID" /snes9x/gtk/build-appimage-x86
'
