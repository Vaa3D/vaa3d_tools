#!/bin/sh 
cd `dirname $0`
export LD_LIBRARY_PATH=`pwd`
./vaa3d $*

