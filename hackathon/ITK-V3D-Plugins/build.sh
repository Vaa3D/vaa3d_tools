#!/bin/bash
echo '========================================'
echo ' build itk and itk_vaa3d_plugins '
echo ' version 1.0 '
echo ' author ping yu '
echo ' date 2012-8-6'
echo '========================================'
BUILD_ITK=
ITK_DIR=
ITK_BUILD_DIR=ITK_build
V3D_BINARY_DIR=
V3D_BASIC_C_FUN_SOURCE_DIR=
PLUGINS_BUILD_DIR=plugins_build
ITK_SOURCE_DIR=InsightToolkit-4.1.0
ITK_SOURCE_GZ=InsightToolkit-4.1.0.tar.gz


echo  'build or rebuild ITK?(y/n): '
read build_ITK
if [ $build_ITK = "y" -o $build_ITK = "Y" ]; then
	BUILD_ITK="YES"
	if [ ! -d $ITK_SOURCE_DIR ];then
		tar zxvf $ITK_SOURCE_GZ
	fi
	if [ -d $ITK_BUILD_DIR ]; then
		rm -rf $ITK_BUILD_DIR
	fi
	mkdir $ITK_BUILD_DIR && cd $ITK_BUILD_DIR
	ITK_DIR=`pwd`
	echo $ITK_DIR
	cmake -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DITK_USE_REVIEW=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug ../$ITK_SOURCE_DIR
	make -j4
	cd ../
else
	if [ ! -d $ITK_BUILD_DIR ]; then
		echo 'no itk library folder found, please set build ITK "y" !'
		exit 0
	fi
	ITK_DIR=`pwd`
   	ITK_DIR=$ITK_DIR'/'$ITK_BUILD_DIR
	echo "ITK library is $ITK_DIR	"
fi
echo "======================================"
echo " now build plugins"
echo " Please set the vaa3d basic_c_fun source dir: "
read V3D_BASIC_C_FUN_SOURCE_DIR
echo 'you vaa3d basic_c_fun dir is :'$V3D_BASIC_C_FUN_SOURCE_DIR
if [ ! -f $V3D_BASIC_C_FUN_SOURCE_DIR/v3d_interface.h ]; then
	echo 'error basic_c_fun  dir please redo it'
	exit 0
fi
echo 'Please set the vaa3d binary dir:'
read V3D_BINARY_DIR
echo 'you binary dir is :'$V3D_BINARY_DIR
if [ -d $PLUGINS_BUILD_DIR ]; then
	cd $PLUGINS_BUILD_DIR
else
	mkdir $PLUGINS_BUILD_DIR && cd $PLUGINS_BUILD_DIR
fi
cmake -DV3D_BASIC_C_FUN_SOURCE_DIR=$V3D_BASIC_C_FUN_SOURCE_DIR -DV3D_BINARY_DIR=$V3D_BINARY_DIR -DITK_DIR=$ITK_DIR -DCMAKE_BUILD_TYPE=Debug ../
make -j4
cd ../
