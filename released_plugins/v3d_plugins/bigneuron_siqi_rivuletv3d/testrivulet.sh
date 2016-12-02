#!/bin/bash
mkdir -p test_data;
export TESTIMGZIP=./test_data/peng-v3d-testdata.zip;
export TESTIMG=./test_data/v3d-testdata//neuron01.tif;
export TESTURL=https://github.com/Vaa3D/Vaa3D_Data/releases/download/data_v1.0/peng-v3d-testdata.zip;
if [ ! -f $TESTIMG ];
then
  rm -rf test_data/*;
  echo "Downloading test image from $TESTURL";
  wget -P ./test_data/ $TESTURL;
  unzip $TESTIMGZIP -d ./test_data;
fi

export VAA3DPATH=../../../../v3d_external
export LD_LIBRARY_PATH=$VAA3DPATH/v3d_main/common_lib/lib
export vaa3d=$VAA3DPATH/bin/vaa3d; # Assume vaa3d
qmake;
make -j8;
echo "Build Finish"

echo "vaa3d -x Rivulet -f tracing_func -i <inimg_file> -p <channel> <threshold>"
$vaa3d -x Rivulet -f tracing_func -i $TESTIMG -o $TESTIMG.r2.swc -p 1 10;
