#!/bin/sh 
cd `dirname $0`
#export LD_LIBRARY_PATH=`pwd`
export LD_LIBRARY_PATH=`pwd`:/usr/local/MATLAB/R2015a/runtime/glnxa64:/usr/local/MATLAB/R2015a/bin/glnxa64:/usr/local/MATLAB/R2015a/sys/os/glnxa64:/usr/local/MATLAB/R2015a/sys/opengl/lib/glnxa64:/home/g/grad/zhihaozh/bigneuron/matlab
./vaa3d $*

