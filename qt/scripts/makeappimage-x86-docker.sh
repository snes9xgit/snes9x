#!/usr/bin/env bash

# Build a 32-bit (i686) Snes9x Qt AppImage. Debian 12 is the only current
# distro archive that still ships Qt 6 for i386 (Qt 6.4, glibc 2.36 floor —
# matches the Debian-based 32-bit distros still in use: antiX, MX, Debian).
# Requires Docker; runs natively on an x86_64 host, no QEMU needed.
#
#   qt/scripts/makeappimage-x86-docker.sh
#
# Output: qt/build-appimage-x86/super-snes9x-qt-x86.AppImage

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
    qt6-base-dev qt6-base-private-dev libqt6svg6-dev \
    libcurl4-openssl-dev zlib1g-dev libpng-dev \
    libwayland-dev wayland-protocols libx11-dev libxext-dev \
    libgl-dev libegl-dev libpulse-dev libasound2-dev \
    libudev-dev libdbus-1-dev libusb-1.0-0-dev libxkbcommon-dev \
    libfontconfig1-dev libfreetype-dev

# Always build from scratch: reproducible, and sidesteps stale-state issues.
rm -rf /snes9x/qt/build-appimage-x86
mkdir -p /snes9x/qt/build-appimage-x86
cd /snes9x/qt/build-appimage-x86
# VULKAN_HPP_NO_SPACESHIP_OPERATOR: on 32-bit targets all non-dispatchable
# Vulkan handles are plain uint64_t, which breaks vulkan-hpp'"'"'s defaulted
# <=> comparisons; the define drops them (snes9x never uses <=> on vk types).
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON \
    -DCMAKE_CXX_FLAGS="-DVULKAN_HPP_NO_SPACESHIP_OPERATOR"
ninja
APPIMAGE_ARCH=i686 ../scripts/makeappimage.sh
chown -R "$HOST_UID:$HOST_GID" /snes9x/qt/build-appimage-x86
'
