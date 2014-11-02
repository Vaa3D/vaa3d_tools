#!/bin/bash
#THis is a shell progrem to batch build tracing methods plugins, including APP1, APP2, MOST, neuTube, snake,simpletracing, TreMap, and MST.

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

#neuTube 
cd ../../../hackathon/zhi/neuTube_zhi/
make clean
svn up
qmake
make
