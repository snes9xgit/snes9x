#!/usr/bin/env bash
set -euo pipefail

# Builds the Android libretro core. Usage: bash build-android.sh [ABI]
# ABI defaults to arm64-v8a; pass "all" for every ABI.
NDK="$HOME/android-ndk-r27c"
ABI="${1:-arm64-v8a}"

if [ ! -d "$NDK" ]; then
    echo "NDK not found, downloading to ~ ..."
    cd ~
    wget https://dl.google.com/android/repository/android-ndk-r27c-linux.zip
    unzip -q android-ndk-r27c-linux.zip
    rm android-ndk-r27c-linux.zip
    cd -
fi

JNI_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBRETRO_DIR="$(dirname "$JNI_DIR")"

cd "$JNI_DIR"
"$NDK/ndk-build" -j"$(nproc)" APP_ABI="$ABI"

# RetroArch matches cores to their .info by filename, so each ABI dir gets a
# core plus a matching-named .info. 64-bit ABIs carry an arch suffix so cores
# from different ABIs can sit in one folder; 32-bit ABIs keep the plain name.
core_name() {
    case "$1" in
        arm64-v8a) echo "supersnes9x_libretro_android-arm64.so" ;;
        x86_64)    echo "supersnes9x_libretro_android-x64.so" ;;
        *)         echo "supersnes9x_libretro_android.so" ;;
    esac
}

OUT="$JNI_DIR/dist"
rm -rf "$OUT"
for so in "$LIBRETRO_DIR"/libs/*/libretro.so; do
    abi_dir="$(basename "$(dirname "$so")")"
    out_so="$(core_name "$abi_dir")"
    mkdir -p "$OUT/$abi_dir"
    cp "$so" "$OUT/$abi_dir/$out_so"
    cp "$LIBRETRO_DIR/supersnes9x_libretro.info" "$OUT/$abi_dir/${out_so%.so}.info"
done

echo "Done:"
find "$OUT" -name '*.so'
