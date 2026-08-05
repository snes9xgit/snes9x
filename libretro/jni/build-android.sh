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

# RetroArch matches cores to supersnes9x_libretro.info by filename.
OUT="$JNI_DIR/dist"
rm -rf "$OUT"
for so in "$LIBRETRO_DIR"/libs/*/libretro.so; do
    abi_dir="$(basename "$(dirname "$so")")"
    mkdir -p "$OUT/$abi_dir"
    cp "$so" "$OUT/$abi_dir/supersnes9x_libretro_android.so"
done
cp "$LIBRETRO_DIR/supersnes9x_libretro.info" "$OUT/"

echo "Done:"
find "$OUT" -name '*.so'
