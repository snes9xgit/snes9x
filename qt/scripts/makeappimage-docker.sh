#!/usr/bin/env bash

# Build a shippable Snes9x Qt AppImage against the Ubuntu 22.04 baseline —
# the oldest LTS whose archive carries Qt 6 (glibc 2.35). An AppImage built
# directly on a newer host would refuse to run on older distros, so the ship
# build happens inside a container. Requires Docker.
#
#   qt/scripts/makeappimage-docker.sh
#
# Output: qt/build-appimage/Snes9x-Qt-x86_64.AppImage

set -e

REPO=$(cd "$(dirname "$0")/../.." && pwd)

docker run --rm \
    -v "$REPO":/snes9x \
    -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" \
    ubuntu:22.04 bash -ec '
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
# PCH is disabled to avoid cmake 3.22'"'"'s Ninja+PCH+AUTOMOC ordering bug.
rm -rf /snes9x/qt/build-appimage
mkdir -p /snes9x/qt/build-appimage
cd /snes9x/qt/build-appimage
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON
ninja
../scripts/makeappimage.sh
chown -R "$HOST_UID:$HOST_GID" /snes9x/qt/build-appimage
'
