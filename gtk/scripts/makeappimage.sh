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

pushd AppDir
rm snes9x.png
ln -s usr/share/icons/hicolor/256x256/apps/snes9x.png
rm snes9x-gtk.desktop
ln -s usr/share/applications/snes9x-gtk.desktop
rm AppRun
ln -s usr/bin/snes9x-gtk AppRun

chmod +x AppRun
popd

# Bundle fontconfig/freetype so they match the bundled Pango
cp -L /usr/lib/x86_64-linux-gnu/libfontconfig.so.1 \
      /usr/lib/x86_64-linux-gnu/libfreetype.so.6 AppDir/usr/lib/

./appimagetool-x86_64.AppImage --appimage-extract-and-run AppDir
