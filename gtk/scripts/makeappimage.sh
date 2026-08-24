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
./linuxdeploy-x86_64.AppImage --appimage-extract-and-run --appdir=AppDir \
	--exclude-library="libX*" \
	--exclude-library="libglib-2.0.so*" \
	--exclude-library="libgobject*" \
	--exclude-library="libgdk_pixbuf*" \
	--exclude-library="libwayland*" \
	--exclude-library="libgmodule*" \
	--exclude-library="libgio-2.0.so*" \
	--exclude-library="libxcb*" \
	--exclude-library="libxkbcommon*" \
	--exclude-library="libdb*" \
	--exclude-library="libgtk-3.so*" \
	--exclude-library="libgdk-3.so*" \
	--exclude-library="libcairo.so*" \
	--exclude-library="libcairo-gobject.so*" \
	--exclude-library="libpango-1.0.so*" \
	--exclude-library="libpangocairo-1.0.so*" \
	--exclude-library="libpangoft2-1.0.so*" \
	--exclude-library="libatk-1.0.so*" \
	--exclude-library="libatk-bridge-2.0.so*"

rm AppDir/snes9x.png
pushd AppDir
ln -s usr/share/icons/hicolor/256x256/apps/snes9x.png
chmod +x AppRun
popd
./appimagetool-x86_64.AppImage --appimage-extract-and-run AppDir
