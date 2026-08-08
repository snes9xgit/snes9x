#!/usr/bin/env bash
#
# Build a redistributable snes9x libretro core for macOS.
#
# Each architecture is built separately (a clean object tree per arch, since
# the Makefile drops .o files next to the sources) and the slices are then
# merged with lipo. Building per-arch rather than passing several -arch flags
# to one clang invocation keeps -flto working on a single-arch link, which is
# the only configuration Apple's linker reliably handles.
#
# The result is ad-hoc signed (`codesign -s -`). That is what an Apple ID
# without a $99/yr Developer Program membership can produce: it satisfies the
# arm64 requirement that all code be signed, and it lets RetroArch load the
# core locally. It is NOT notarized, so a core downloaded from the internet
# still carries the com.apple.quarantine attribute -- see README-macos.md.
#
# Usage:
#   ./build-macos.sh                 # universal: x86_64 + arm64
#   ./build-macos.sh x86_64          # single arch
#   ./build-macos.sh x86_64 arm64    # same as the default
#
# Requirements: Xcode command line tools (clang, lipo, codesign).
# Output: libretro/macos/dist/supersnes9x_libretro.dylib + matching .info

set -euo pipefail

# Repo layout: this script lives in libretro/macos/.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBRETRO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DIST_DIR="${SCRIPT_DIR}/dist"
BUILD_DIR="${SCRIPT_DIR}/build"

TARGET_DYLIB="supersnes9x_libretro.dylib"
JOBS="$(sysctl -n hw.ncpu)"

# Deployment target per arch. arm64 did not exist before macOS 11.0, so a
# lower floor there is silently clamped (and warned about) by the linker.
min_version() {
  case "$1" in
    arm64) echo "11.0" ;;
    *)     echo "10.13" ;;
  esac
}

build_arch() {
  local arch="$1" minver
  minver="$(min_version "${arch}")"

  echo ">>> Building ${TARGET_DYLIB} for ${arch} (macOS ${minver} floor) ..."

  # Objects live next to the sources in the repo root and are shared with the
  # other Makefile builds, so a clean between arches is mandatory -- otherwise
  # the second link picks up the first arch's objects and fails.
  make -C "${LIBRETRO_DIR}" clean >/dev/null 2>&1 || true
  make -C "${LIBRETRO_DIR}" -j"${JOBS}" \
    platform=osx \
    OSX_ARCHS="${arch}" \
    OSX_MIN_VERSION="${minver}"

  mkdir -p "${BUILD_DIR}"
  mv "${LIBRETRO_DIR}/${TARGET_DYLIB}" "${BUILD_DIR}/${TARGET_DYLIB}.${arch}"
  echo "    $(file -b "${BUILD_DIR}/${TARGET_DYLIB}.${arch}")"
}

main() {
  if ! command -v clang >/dev/null 2>&1; then
    echo "error: clang not found -- install the Xcode command line tools:" >&2
    echo "       xcode-select --install" >&2
    exit 1
  fi

  local -a archs=("$@")
  if [ "${#archs[@]}" -eq 0 ]; then
    archs=("x86_64" "arm64")
  fi

  rm -rf "${BUILD_DIR}"
  mkdir -p "${DIST_DIR}" "${BUILD_DIR}"

  local -a slices=()
  for arch in "${archs[@]}"; do
    build_arch "${arch}"
    slices+=("${BUILD_DIR}/${TARGET_DYLIB}.${arch}")
  done

  echo ">>> Merging ${#slices[@]} slice(s) with lipo ..."
  lipo -create "${slices[@]}" -output "${DIST_DIR}/${TARGET_DYLIB}"

  # Ad-hoc signature. Re-signing after lipo is required: lipo rewrites the
  # Mach-O headers and invalidates any signature the individual slices had.
  echo ">>> Ad-hoc signing ..."
  codesign --force --sign - --timestamp=none "${DIST_DIR}/${TARGET_DYLIB}"
  codesign --verify --verbose=2 "${DIST_DIR}/${TARGET_DYLIB}"

  # RetroArch matches a core to its .info by filename, so ship them together.
  cp "${LIBRETRO_DIR}/${TARGET_DYLIB%.dylib}.info" "${DIST_DIR}/"

  rm -rf "${BUILD_DIR}"
  make -C "${LIBRETRO_DIR}" clean >/dev/null 2>&1 || true

  echo
  echo ">>> Wrote ${DIST_DIR}/${TARGET_DYLIB}"
  lipo -info "${DIST_DIR}/${TARGET_DYLIB}"
}

main "$@"
