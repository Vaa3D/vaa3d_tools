#!/bin/bash
export V3DPATH=../../../../v3d_external
export LD_LIBRARY_PATH=$V3DPATH/v3d_main/common_lib/lib
export vaa3d=$V3DPATH/bin/vaa3d; # Assume vaa3d
qmake;
make -j4;
echo "Build Finish"

echo "vaa3d -x Rivulet -f tracing_func -i <inimg_file> -p <channel> <threshold>"
$vaa3d -x Rivulet -f tracing_func -i test/test.small.tif -p 1 1 
