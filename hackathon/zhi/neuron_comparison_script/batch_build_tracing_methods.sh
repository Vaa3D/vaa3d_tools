#!/bin/bash
#THis is a shell progrem to batch build tracing methods plugins, including APP1, APP2, MOST, neuTube, snake,simpletracing, TreMap, MST, and NeuroGPSTree.

#APP1 and APP2 
cd ../../../released_plugins/v3d_plugins/neurontracing_vn2/
make clean
svn up
qmake
make

#MOST 
cd ../neurontracing_MOST/
make clean
svn up
qmake
make	

#simpletracing 
cd ../neurontracing_simpletracing/
make clean
svn up
qmake
make

#TreMap 
cd  ../../../hackathon/zhi/neurontracing_mip/
make clean
svn up
qmake
make

#MST 
cd  ../../../hackathon/zhi/neurontracing_mst/
make clean
svn up
qmake
make

#snake
cd ../../../hackathon/zhi/snake_tracing/
if [ ! -d ITK_include ]; then
	unzip ITK_include.zip		
fi	
make clean
svn up
qmake
make

#NeuroGPSTree
cd ../../../bigneuron_ported/zhouhang_hust/NeuroGPSTree/
if [ ! -d eigen3.2.2 ]; then
	unzip eigen3.2.2	
fi	
make clean
svn up
qmake
make


#neuTube 
cd ../../../hackathon/zhi/neuTube_zhi/
make clean
svn up

cd ../../../released_plugins/v3d_plugins/neurontracing_neutube/src_neutube
if [ "$(uname)" == "Darwin"]; then
  sh build.sh /usr/local/Trolltech/Qt-4.7.1/bin/qmake /usr/local/Trolltech/Qt-4.7.1/mkspecs/macx-g++42
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" && ! -f ../../../released_plugins/v3d_plugins/neurontracing_neutube/src_neutube/neurolabi/c/lib/libneurolabi_debug.so]; then
  sh build.sh /usr/local/Trolltech/Qt-4.7.1/bin/qmake /usr/local/Trolltech/Qt-4.7.1/mkspecs/linux-g++
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  echo "detected windows platform" 
fi

cd ../../../../hackathon/zhi/neuTube_zhi/
qmake
make
