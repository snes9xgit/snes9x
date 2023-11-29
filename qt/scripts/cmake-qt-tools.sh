#!/bin/sh
PARAMS="-DFEATURE_clang=OFF -DFEATURE_clangcpp=OFF -DFEATURE_designer=off -DFEATURE_kmap2qmap=OFF -DFEATURE_pixeltool=OFF -DFEATURE_qev=OFF -DFEATURE_qtattributionsscanner=OFF -DFEATURE_qtdiag=OFF -DFEATURE_qtplugininfo=OFF -DFEATURE_qdoc=OFF"

CMAKE_PREFIX_PATH=../../../qt/lib/cmake cmake .. -G Ninja $PARAMS
