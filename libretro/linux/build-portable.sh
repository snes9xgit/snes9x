#!/usr/bin/env bash
#
# Build a portable, redistributable snes9x libretro core for Linux.
#
# Builds inside an old Ubuntu container (glibc 2.27 floor) and uses the
# Makefile's `portable` target, which statically links libstdc++/libgcc
# and strips the result. The output loads on essentially every still-
# supported distro of the matching architecture without needing a
# matching libstdc++/libgcc_s or a recent glibc.
#
# The 2.27 floor also covers the Steam Linux Runtime containers used by
# RetroArch on Steam: soldier (Debian 10, glibc 2.28) and sniper
# (Debian 11, glibc 2.31), as well as native Steam Deck / SteamOS.
#
# Usage:
#   ./build-portable.sh [arch ...]
#
#   arch: x86_64 (default), x86, aarch64, armhf  -- may pass several
#
# Examples:
#   ./build-portable.sh                 # x86_64 only
#   ./build-portable.sh x86_64 aarch64  # both
#
# Non-native architectures are built via QEMU emulation. Enable it once with:
#   docker run --rm --privileged tonistiigi/binfmt --install all
# (x86 on an x86_64 host runs natively, no QEMU needed.)
#
# If your user is not in the `docker` group, run docker via sudo:
#   DOCKER='sudo docker' ./build-portable.sh
#
# Requirements: docker.
# Output: libretro/linux/dist/<arch>/supersnes9x_libretro.so + matching .info
#         (the x86_64 core is named supersnes9x_libretro-x64.so/.info)

set -euo pipefail

# Repo layout: this script lives in libretro/linux/.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBRETRO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_DIR="$(cd "${LIBRETRO_DIR}/.." && pwd)"
DIST_DIR="${SCRIPT_DIR}/dist"

IMAGE="ubuntu:18.04"   # glibc 2.27 -- covers Steam soldier/sniper + most distros
TARGET_SO="supersnes9x_libretro.so"
DOCKER="${DOCKER:-docker}"   # override, e.g. DOCKER='sudo docker'

# Map arch name -> docker --platform value.
docker_platform() {
  case "$1" in
    x86_64)  echo "linux/amd64" ;;
    x86)     echo "linux/386" ;;
    aarch64) echo "linux/arm64" ;;
    armhf)   echo "linux/arm/v7" ;;
    *) echo "unknown arch: $1" >&2; return 1 ;;
  esac
}

# Map arch name -> docker image. The multi-arch ubuntu:18.04 manifest has no
# i386 entry, so 32-bit x86 needs the explicit i386 image.
docker_image() {
  case "$1" in
    x86) echo "i386/ubuntu:18.04" ;;
    *)   echo "${IMAGE}" ;;
  esac
}

# Map arch name -> released filename (x86_64 carries an -x64 suffix).
out_name() {
  case "$1" in
    x86_64) echo "${TARGET_SO%.so}-x64.so" ;;
    *)      echo "${TARGET_SO}" ;;
  esac
}

build_arch() {
  local arch="$1" platform image out_dir out_so
  platform="$(docker_platform "$arch")"
  image="$(docker_image "$arch")"
  out_dir="${DIST_DIR}/${arch}"
  out_so="$(out_name "$arch")"
  mkdir -p "${out_dir}"

  echo ">>> Building ${out_so} for ${arch} (${platform}) in ${image} ..."

  # Build with a clean object tree so we never mix objects across archs.
  # The whole repo is mounted because libretro objects live in the repo root.
  ${DOCKER} run --rm --platform "${platform}" \
    -v "${REPO_DIR}:/build" -w /build/libretro "${image}" \
    bash -euo pipefail -c '
      export DEBIAN_FRONTEND=noninteractive
      apt-get update -qq
      apt-get install -y -qq build-essential git >/dev/null
      git config --global --add safe.directory /build || true
      make clean >/dev/null 2>&1 || true
      make portable -j"$(nproc)"
    '

  cp "${LIBRETRO_DIR}/${TARGET_SO}" "${out_dir}/${out_so}"
  # RetroArch matches cores to their .info by filename, so ship a copy named
  # after the core (supersnes9x_libretro-x64.info for the x86_64 build).
  cp "${LIBRETRO_DIR}/${TARGET_SO%.so}.info" "${out_dir}/${out_so%.so}.info"
  echo ">>> Wrote ${out_dir}/${out_so}"
  echo "    $(cd "${out_dir}" && file "${out_so}" | cut -d, -f1-3)"
}

main() {
  if ! command -v ${DOCKER%% *} >/dev/null 2>&1; then
    echo "error: docker is required but not found in PATH" >&2
    exit 1
  fi

  local -a archs=("$@")
  if [ "${#archs[@]}" -eq 0 ]; then
    archs=("x86_64")
  fi

  for arch in "${archs[@]}"; do
    build_arch "${arch}"
  done

  echo
  echo "Done. Portable cores are under ${DIST_DIR}/"
}

main "$@"
