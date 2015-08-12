#!/bin/sh
cd `dirname $0` 
export DISPLAY=:$RANDOM
Xvfb $DISPLAY -auth /dev/null & 
export LD_LIBRARY_PATH=/global/homes/z/zhizhou/bigneuron/BigNeuron_bench_test_LBNL_v1/Vaa3D_BigNeuron_version1
/global/homes/z/zhizhou/bigneuron/BigNeuron_bench_test_LBNL_v1/Vaa3D_BigNeuron_version1/vaa3d $*

