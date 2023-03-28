#!/usr/bin/env sh

if [ ! -f appimagetool-x86_64.AppImage ]; then
    wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ ! -d AppImageKit-checkrt ]; then
    git clone https://github.com/darealshinji/AppImageKit-checkrt
fi
pushd AppImageKit-checkrt/src
make
popd

DESTDIR=AppDir ninja install
./linuxdeploy-x86_64.AppImage --appdir=AppDir

rm AppDir/snes9x.png
rm AppDir/AppRun
mkdir AppDir/usr/optional
cp AppImageKit-checkrt/src/checkrt AppDir/usr/optional
AppDir/usr/optional/checkrt -c
pushd AppDir
rm snes9x.png
rm AppRun
ln -s usr/share/icons/hicolor/256x256/apps/snes9x.png
cat > AppRun << EOF
#!/bin/sh -e

# add your command to execute here
exec=snes9x-gtk

cd "$(dirname "$0")"
if [ "x$exec" = "x" ]; then
  exec="$(sed -n 's|^Exec=||p' *.desktop | head -1)"
fi
if [ -x "./usr/optional/checkrt" ]; then
  extra_libs="$(./usr/optional/checkrt)"
fi
if [ -n "$extra_libs" ]; then
  export LD_LIBRARY_PATH="${extra_libs}${LD_LIBRARY_PATH}"
  if [ -e "$PWD/usr/optional/exec.so" ]; then
    export LD_PRELOAD="$PWD/usr/optional/exec.so:${LD_PRELOAD}"
  fi
fi

#echo ">>>>> $LD_LIBRARY_PATH"
#echo ">>>>> $LD_PRELOAD"

./usr/bin/$exec "$*"
exit $?
EOF
chmod +x AppRun
popd
./appimagetool-x86_64.AppImage AppDir
