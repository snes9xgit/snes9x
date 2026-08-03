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
#   arch: x86_64 (default), aarch64, armhf  -- may pass several
#
# Examples:
#   ./build-portable.sh                 # x86_64 only
#   ./build-portable.sh x86_64 aarch64  # both
#
# Non-native architectures are built via QEMU emulation. Enable it once with:
#   docker run --rm --privileged tonistiigi/binfmt --install all
#
# If your user is not in the `docker` group, run docker via sudo:
#   DOCKER='sudo docker' ./build-portable.sh
#
# Requirements: docker.
# Output: libretro/linux/dist/<arch>/supersnes9x_libretro.so

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
    aarch64) echo "linux/arm64" ;;
    armhf)   echo "linux/arm/v7" ;;
    *) echo "unknown arch: $1" >&2; return 1 ;;
  esac
}

build_arch() {
  local arch="$1" platform out_dir
  platform="$(docker_platform "$arch")"
  out_dir="${DIST_DIR}/${arch}"
  mkdir -p "${out_dir}"

  echo ">>> Building ${TARGET_SO} for ${arch} (${platform}) in ${IMAGE} ..."

  # Build with a clean object tree so we never mix objects across archs.
  # The whole repo is mounted because libretro objects live in the repo root.
  ${DOCKER} run --rm --platform "${platform}" \
    -v "${REPO_DIR}:/build" -w /build/libretro "${IMAGE}" \
    bash -euo pipefail -c '
      export DEBIAN_FRONTEND=noninteractive
      apt-get update -qq
      apt-get install -y -qq build-essential git >/dev/null
      git config --global --add safe.directory /build || true
      make clean >/dev/null 2>&1 || true
      make portable -j"$(nproc)"
    '

  cp "${LIBRETRO_DIR}/${TARGET_SO}" "${out_dir}/${TARGET_SO}"
  echo ">>> Wrote ${out_dir}/${TARGET_SO}"
  echo "    $(cd "${out_dir}" && file "${TARGET_SO}" | cut -d, -f1-3)"
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
