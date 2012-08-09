#!/bin/bash
echo '========================================'
echo ' build itk and itk_vaa3d_plugins '
echo ' version 1.0 '
echo ' author ping yu '
echo ' date 2012-8-6'
echo '========================================'

ITK_DIR=
ITK_BUILD_DIR=ITK_build
V3D_SOURCE_DIR=
V3D_SOURCE_DIR_FILE=V3D_Source_dir.tmp
V3D_BINARY_DIR=
V3D_BASIC_C_FUN_SOURCE_DIR=
PLUGINS_BUILD_DIR=plugins_build
ITK_SOURCE_DIR=InsightToolkit-4.1.0
ITK_SOURCE_GZ=InsightToolkit-4.1.0.tar.gz


echo  'build or rebuild ITK?(y/n): '
read build_ITK

build_itk_library()
{
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
}
if [ $build_ITK = "y" -o $build_ITK = "Y" ]; then
	build_itk_library
else
	if [ ! -d $ITK_BUILD_DIR ]; then
		echo 'No itk library folder found, Now build ITK !'
		build_itk_library
	fi
	ITK_DIR=`pwd`
   	ITK_DIR=$ITK_DIR'/'$ITK_BUILD_DIR
	echo "ITK library is $ITK_DIR	"
fi
echo "======================================"
validate_vaa3d_source_dir()
{
	while [ true ]; do
		echo " Please set the Vaa3D Source dir: "
		read V3D_SOURCE_DIR
		if [ -d $V3D_SOURCE_DIR/v3d_main ]; then
			break
		else
			echo 'Path is Wrong, do it again'
		fi
	done
}
echo " now build plugins"
echo " Set the Vaa3D path yourself?(y/n)"
read IS_SET_PATH
if [ $IS_SET_PATH = "Y" -o $IS_SET_PATH = "y" ]; then
	validate_vaa3d_source_dir
else
	read V3D_SOURCE_DIR < $V3D_SOURCE_DIR_FILE
	echo 'Your origin Vaa3d source dir is: ' $V3D_SOURCE_DIR
fi	
if [ ! -d $V3D_SOURCE_DIR/v3d_main ]; then 
	echo 'Your path do not have the source, please set it again'
	validate_vaa3d_source_dir
fi
echo $V3D_SOURCE_DIR > $V3D_SOURCE_DIR_FILE

V3D_BASIC_C_FUN_SOURCE_DIR=$V3D_SOURCE_DIR/v3d_main/basic_c_fun
V3D_BINARY_DIR=$V3D_SOURCE_DIR/bin
echo 'Your vaa3d basic_c_fun dir is :'$V3D_BASIC_C_FUN_SOURCE_DIR
echo 'you binary dir is :'$V3D_BINARY_DIR
if [ -d $PLUGINS_BUILD_DIR ]; then
	cd $PLUGINS_BUILD_DIR
else
	mkdir $PLUGINS_BUILD_DIR && cd $PLUGINS_BUILD_DIR
fi
cmake -DV3D_BASIC_C_FUN_SOURCE_DIR=$V3D_BASIC_C_FUN_SOURCE_DIR -DV3D_BINARY_DIR=$V3D_BINARY_DIR -DITK_DIR=$ITK_DIR -DCMAKE_BUILD_TYPE=Debug ../
make -j4
make install
cd ../
