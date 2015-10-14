#!/bin/bash
vaa3d=/home/siqi/v3d_external/bin/vaa3d;
qmake;
make -j4;
$vaa3d -x Rivulet -f tracing_func -i test/2000-1.v3draw -p 1 0 1 40 0 2 0.98| tee runlog.txt