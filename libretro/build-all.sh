#!/usr/bin/env bash
#
# Master build: Qt GUI, GTK GUI, Linux libretro core, Android libretro core.
#
# Cleans every executable/.so first so stale artifacts can never be copied,
# then rebuilds everything and prints the full path of each fresh file.
#
# Usage:
#   ./build-all.sh                # Android core for arm64-v8a (default)
#   ANDROID_ABI=all ./build-all.sh
#
# Notes:
# - The Linux core is a native build (matches this machine's glibc). For the
#   shippable portable core use linux/build-portable.sh (needs docker).
# - Same for the GUIs: qt/scripts/makeappimage-docker.sh builds the
#   shippable Qt AppImage against the Ubuntu 22.04 baseline.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a}"
JOBS="$(nproc)"

QT_BIN="${REPO_DIR}/qt/build/super-snes9x-qt"
GTK_BIN="${REPO_DIR}/gtk/build/super-snes9x-gtk"
QT_APPIMAGE="${REPO_DIR}/qt/build/super-snes9x-qt-x86_64.AppImage"
GTK_APPIMAGE="${REPO_DIR}/gtk/build/super-snes9x-gtk-x86_64.AppImage"
LINUX_SO="${SCRIPT_DIR}/supersnes9x_libretro.so"
ANDROID_DIST="${SCRIPT_DIR}/jni/dist"

# Run the AppImage tools without FUSE (works everywhere, incl. containers).
export APPIMAGE_EXTRACT_AND_RUN=1

step() { echo; echo "==> $*"; }

# ---- Clean old artifacts -------------------------------------------------

step "Cleaning old executables and cores"
rm -f "${QT_BIN}" "${GTK_BIN}" "${QT_APPIMAGE}" "${GTK_APPIMAGE}" "${LINUX_SO}"
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

# ---- Linux libretro core -------------------------------------------------

step "Building Linux libretro core (native)"
make -C "${SCRIPT_DIR}" -j"${JOBS}"

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
show "${LINUX_SO}"
found_android=0
for so in "${ANDROID_DIST}"/*/supersnes9x_libretro_android.so; do
    [ -f "$so" ] && { echo "  $so"; found_android=1; }
done
if [ "${found_android}" -eq 0 ]; then
    echo "  MISSING: ${ANDROID_DIST}/<abi>/supersnes9x_libretro_android.so"
    missing=1
fi
echo "================================================="
exit "${missing}"
