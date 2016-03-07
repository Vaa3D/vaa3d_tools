#!/bin/bash
vaa3d=../../../../v3d_external/bin/vaa3d; # Assume vaa
qmake;
make -j4;
echo "Build Finish"

echo "vaa3d -x Rivulet -f tracing_func -i <inimg_file> -p <channel> <threshold> <length> <gap> <dumpbranch> <connectrate> <percentage> <sigma>"
$vaa3d -x Rivulet -f tracing_func -i test/2000-1.v3draw -p 1 0 8 15 0 2 0.98 0 
