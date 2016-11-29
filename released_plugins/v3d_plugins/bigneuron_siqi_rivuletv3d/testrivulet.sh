#!/bin/bash
export VAA3DPATH=../../../../v3d_external
export LD_LIBRARY_PATH=$VAA3DPATH/v3d_main/common_lib/lib
export vaa3d=$VAA3DPATH/bin/vaa3d; # Assume vaa3d
qmake;
make -j8;
echo "Build Finish"

echo "vaa3d -x Rivulet -f tracing_func -i <inimg_file> -p <channel> <threshold>"
$vaa3d -x Rivulet -f tracing_func -i test/test.tif -o fuck.swc -p 1 1;
