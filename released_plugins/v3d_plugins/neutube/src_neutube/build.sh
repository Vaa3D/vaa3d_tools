#!/bin/bash

#Example:
#sh build.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

if [ $# -lt 2 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path>"
  echo "Example: "
  echo 'sh build.sh $HOME/local/lib/Trolltech/Qt-4.8.5/bin/qmake $HOME/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++'
  exit 1
fi

QMAKE=$1
QMAKE_SPEC=$2

cd neurolabi

echo 'Building 3rd-party libraries ...'
cd lib
sh build.sh
cd ..

./update_library --release 
if [ ! -d build ]
then
  mkdir build
fi

cd build
$QMAKE -spec $QMAKE_SPEC CONFIG+=release CONFIG+=x86_64 LIBS+=-lstdc++  -o Makefile ../gui/gui.pro
make

#echo "Deploying ..."

#if [ -d neuTube.app ]
#then
#  qtbin_dir=`dirname $QMAKE`
#  cd ..
#  if [ -f $qtbin_dir/macdeployqt ]
#  then
#    $qtbin_dir/macdeployqt `pwd`/build/neuTube.app -no-plugins
#  else
#    sh gui/deploy_neutube.sh `pwd`/build
#  fi
#  echo "build/neuTube.app deployed."
#else
#  echo "neuTube deployed."
  #mv neuTube ../../
#fi

