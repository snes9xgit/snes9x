#!/usr/bin/env bash

if [ ! -f appimagetool-x86_64.AppImage ]; then
    wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

DESTDIR=AppDir ninja install
./linuxdeploy-x86_64.AppImage --appimage-extract-and-run --appdir=AppDir

rm AppDir/snes9x.png
pushd AppDir
ln -s usr/share/icons/hicolor/256x256/apps/snes9x.png
chmod +x AppRun
popd
./appimagetool-x86_64.AppImage --appimage-extract-and-run AppDir
