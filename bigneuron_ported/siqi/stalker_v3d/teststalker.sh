#!/bin/bash
# The unit test for Stalker Model
vaa3d=../../../../v3d_external/bin/vaa3d;
#vaa3d=/home/siqi/Vaa3D_Ubuntu_64bit_v3.045/vaa3d;
qmake;
make -j4;

#$vaa3d -x NeuroStalker -f tracing_func -i test/testdata/test.little-soma.v3draw -p 1 1 3 | tee runlog.txt
$vaa3d -x NeuroStalker -f tracing_func -i test/test.v3draw -p 1 1 1 | tee runlog.txt
#$vaa3d -x NeuroStalker -f tracing_func -i ~/Desktop/OP1.v3draw -p 1 3 3 | tee runlog.txt
#matlab -nodesktop -nosplash -r "run('test/plotall.m')";
