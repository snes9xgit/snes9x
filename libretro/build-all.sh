#!/usr/bin/env bash
#
# Master build: Qt GUI, GTK GUI, Linux libretro core, Android libretro core.
#
# Cleans every executable/.so first so stale artifacts can never be copied,
# then rebuilds everything and prints the full path of each fresh file.
#
# Usage:
#   ./build-all.sh                # Android cores for arm64-v8a + armeabi-v7a
#   ./build-all.sh --copy         # also copy artifacts to the VMware share
#   ./build-all.sh --compress     # zip each core with its .info
#   ANDROID_ABI=all ./build-all.sh
#
# --copy copies every artifact (AppImages, cores, .info files) flat into
# SHARE_DIR (default /mnt/hgfs/Shared) after a fully successful build.
# --compress zips each core (.so) together with its matching .info next to
# the .so; AppImages are never compressed. With --copy, the share receives
# the zips instead of the bare .so/.info files.
#
# Notes:
# - The Linux core is built with linux/build-portable.sh (needs docker):
#   an Ubuntu 18.04 container with static libstdc++, so the released .so
#   loads on essentially any distro (verified GLIBC_2.14 floor).
# - The x86_64 GUI AppImages here are native builds. For public releases
#   build the Qt AppImage with qt/scripts/makeappimage-docker.sh (Ubuntu
#   22.04 baseline).
# - The x86 (32-bit) GUI AppImages are docker builds against Debian 12,
#   the only current i386 archive that ships Qt 6.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a armeabi-v7a}"
JOBS="$(nproc)"
SHARE_DIR="${SHARE_DIR:-/mnt/hgfs/Shared}"

COPY_TO_SHARE=0
COMPRESS=0
for arg in "$@"; do
    case "$arg" in
        --copy) COPY_TO_SHARE=1 ;;
        --compress) COMPRESS=1 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

QT_BIN="${REPO_DIR}/qt/build/super-snes9x-qt"
GTK_BIN="${REPO_DIR}/gtk/build/super-snes9x-gtk"
QT_APPIMAGE="${REPO_DIR}/qt/build/super-snes9x-qt-x86_64.AppImage"
GTK_APPIMAGE="${REPO_DIR}/gtk/build/super-snes9x-gtk-x86_64.AppImage"
QT_APPIMAGE_X86="${REPO_DIR}/qt/build-appimage-x86/super-snes9x-qt-x86.AppImage"
GTK_APPIMAGE_X86="${REPO_DIR}/gtk/build-appimage-x86/super-snes9x-gtk-x86.AppImage"
LINUX_SO="${SCRIPT_DIR}/linux/dist/x86_64/supersnes9x_libretro-x64.so"
LINUX_SO_X86="${SCRIPT_DIR}/linux/dist/x86/supersnes9x_libretro.so"
ANDROID_DIST="${SCRIPT_DIR}/jni/dist"

# Run the AppImage tools without FUSE (works everywhere, incl. containers).
export APPIMAGE_EXTRACT_AND_RUN=1

step() { echo; echo "==> $*"; }

# ---- Clean old artifacts -------------------------------------------------

step "Cleaning old executables and cores"
rm -f "${QT_BIN}" "${GTK_BIN}" "${QT_APPIMAGE}" "${GTK_APPIMAGE}" \
      "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
      "${LINUX_SO}" "${LINUX_SO_X86}" \
      "${SCRIPT_DIR}/linux/dist/x86_64/supersnes9x_libretro.so" \
      "${SCRIPT_DIR}/supersnes9x_libretro.so"
rm -rf "${REPO_DIR}/qt/build/AppDir" "${REPO_DIR}/gtk/build/AppDir"
rm -rf "${ANDROID_DIST}" "${SCRIPT_DIR}/libs" "${SCRIPT_DIR}/obj"
# Libretro objects live in the repo root and are shared with other Makefile
# builds; a clean here guarantees the core is rebuilt from current sources.
make -C "${SCRIPT_DIR}" clean >/dev/null 2>&1 || true

# ---- Qt GUI --------------------------------------------------------------

step "Building Qt GUI"
if [ ! -f "${REPO_DIR}/qt/build/CMakeCache.txt" ]; then
    cmake -S "${REPO_DIR}/qt" -B "${REPO_DIR}/qt/build" -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "${REPO_DIR}/qt/build" -j"${JOBS}"

step "Packaging Qt AppImage"
(cd "${REPO_DIR}/qt/build" && ../scripts/makeappimage.sh)

# ---- GTK GUI -------------------------------------------------------------

step "Building GTK GUI"
if [ ! -f "${REPO_DIR}/gtk/build/CMakeCache.txt" ]; then
    cmake -S "${REPO_DIR}/gtk" -B "${REPO_DIR}/gtk/build" -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "${REPO_DIR}/gtk/build" -j"${JOBS}"

step "Packaging GTK AppImage"
(cd "${REPO_DIR}/gtk/build" && ../scripts/makeappimage.sh)

# ---- 32-bit (x86) GUI AppImages -----------------------------------------

step "Building Qt AppImage (x86 32-bit, Debian 12 baseline, docker)"
"${REPO_DIR}/qt/scripts/makeappimage-x86-docker.sh"

step "Building GTK AppImage (x86 32-bit, Debian 12 baseline, docker)"
"${REPO_DIR}/gtk/scripts/makeappimage-x86-docker.sh"

# ---- Linux libretro core -------------------------------------------------

step "Building Linux libretro cores (portable, Ubuntu 18.04 baseline: x86_64 + x86)"
"${SCRIPT_DIR}/linux/build-portable.sh" x86_64 x86

# ---- Android libretro core ----------------------------------------------

step "Building Android libretro core (${ANDROID_ABI})"
bash "${SCRIPT_DIR}/jni/build-android.sh" "${ANDROID_ABI}"

# ---- Summary -------------------------------------------------------------

echo
echo "================ Build artifacts ================"
missing=0
show() {
    if [ -f "$1" ]; then
        echo "  $1"
    else
        echo "  MISSING: $1"
        missing=1
    fi
}
show "${QT_BIN}"
show "${GTK_BIN}"
show "${QT_APPIMAGE}"
show "${GTK_APPIMAGE}"
show "${QT_APPIMAGE_X86}"
show "${GTK_APPIMAGE_X86}"
show "${LINUX_SO}"
show "${LINUX_SO_X86}"
found_android=0
for so in "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so; do
    [ -f "$so" ] && { echo "  $so"; found_android=1; }
done
if [ "${found_android}" -eq 0 ]; then
    echo "  MISSING: ${ANDROID_DIST}/<abi>/supersnes9x_libretro_android*.so"
    missing=1
fi
echo "================================================="

# ---- Compress cores (.so + matching .info) -------------------------------

if [ "${COMPRESS}" -eq 1 ]; then
    if [ "${missing}" -ne 0 ]; then
        echo "Not compressing: build has missing artifacts." >&2
        exit 1
    fi
    if ! command -v zip >/dev/null; then
        echo "error: zip is required for --compress (sudo apt install zip)" >&2
        exit 1
    fi
    step "Compressing cores (.so + .info)"
    for so in "${LINUX_SO}" "${LINUX_SO_X86}" \
              "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so; do
        zipf="${so%.so}.zip"
        rm -f "${zipf}"
        zip -j -q "${zipf}" "${so}" "${so%.so}.info"
        echo "  ${zipf}"
    done
fi

# ---- Copy to the VMware shared folder ------------------------------------

if [ "${COPY_TO_SHARE}" -eq 1 ]; then
    if [ "${missing}" -ne 0 ]; then
        echo "Not copying to ${SHARE_DIR}: build has missing artifacts." >&2
        exit 1
    fi
    step "Copying artifacts to ${SHARE_DIR}"
    # An unmounted /mnt/hgfs silently accepts writes onto the local disk,
    # so refuse to copy unless the target really is an hgfs mount.
    if ! df -PT "${SHARE_DIR}" 2>/dev/null | grep -q hgfs; then
        echo "error: ${SHARE_DIR} is not on a mounted hgfs share" >&2
        echo "       (mount it with: sudo mount -t fuse.vmhgfs-fuse .host:/ /mnt/hgfs -o allow_other)" >&2
        exit 1
    fi
    if [ "${COMPRESS}" -eq 1 ]; then
        cp -v "${QT_APPIMAGE}" "${GTK_APPIMAGE}" "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
              "${LINUX_SO%.so}.zip" "${LINUX_SO_X86%.so}.zip" \
              "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.zip \
              "${SHARE_DIR}/"
    else
        cp -v "${QT_APPIMAGE}" "${GTK_APPIMAGE}" "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
              "${LINUX_SO}" "${LINUX_SO%.so}.info" \
              "${LINUX_SO_X86}" "${LINUX_SO_X86%.so}.info" \
              "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so \
              "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.info \
              "${SHARE_DIR}/"
    fi
fi

exit "${missing}"
