#!/bin/bash
mkdir -p test_data;
export TESTIMGZIP=./test_data/test.tif.zip;
export TESTIMG=./test_data/test.tif;
export TESTURL=https://s3-ap-southeast-2.amazonaws.com/rivulet/test.tif.zip;
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
$vaa3d -x SomaDetect -f tracing_func -i $TESTIMG -o $TESTIMG.r2.somapoint.swc -p 1 10 0 1;
