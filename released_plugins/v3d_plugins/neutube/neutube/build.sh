#!/bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path>"
  exit 1
fi

QMAKE=$1
QMAKE_SPEC=$2

cd neurolabi
#./update_library --release --disable-png
if [ ! -d build ]
then
  mkdir build
fi

cd build
$QMAKE -spec $QMAKE_SPEC CONFIG+=release CONFIG+=x86_64 -o Makefile ../gui/gui.pro
make

if [ -d neuTube.app ]
then
  cp -r neuTube.app ../../
else
  mv neuTube ../../
fi

