#!/bin/bash
vaa3d=../../../../v3d_external/bin/vaa3d; # Assume vaa
qmake;
make -j4;
echo "Build Finish"
# $vaa3d -x Rivulet -f tracing_func -i test/2000-1.v3draw -p 1 0 1 10 0 2 0.95 3 0.5 1| tee runlog.txt
# $vaa3d -x Rivulet -f tracing_func -i test/2000-12.v3draw -p 1 50 1 10 0 2 0.01 3 0.5 1
$vaa3d -x Rivulet -f tracing_func -i test/2000-1.v3draw -p 1 0 8 10 0 2 0.98 0 0.5 1
# $vaa3d -x Rivulet -f tracing_func -i test/2000-13.v3draw -p 1 0 1 10 0 2 0.95 6 2 2
# $vaa3d -x Rivulet -f tracing_func -i test/2000-14.v3draw -p 1 0 1 10 0 2 0.95 8 1 1
