#!/bin/bash


dosomething()
{
	filename=$1
	suffix=${filename##*.}

	if [ $suffix = "tif" ]
	then
		echo "do affine transform"

filename_img_sub_nodir=${filename##*/}
filename_img_sub_nosuffix=${filename_img_sub_nodir%.*}
filename_img_affine="$dirname_output""$filename_img_sub_nosuffix""_affine.tif"
filename_img_tps="$dirname_output""$filename_img_sub_nosuffix""_tps.tif"

command_affinetrans="/groups/peng/home/qul/work/v3d_2.0/plugin_demo/affine_transform/main_affinetransform \
-t /groups/peng/home/qul/Desktop/Jefferies/global_align/target.marker \
-s /groups/peng/home/qul/Desktop/Jefferies/global_align/IS2_SABN2-1_01_warp_m0g80c8e1e-1x26r4.marker \
-S $filename \
-o $filename_img_affine \
-f 1 \
-W 1024 \
-H 512 \
-Z 218"
		eval "$command_affinetrans"


		echo "do tps transform"

command_tpstrans="/groups/peng/home/qul/work/v3d_2.0/v3d_main_internal/jba/c++/jba \
-t /groups/peng/home/qul/Desktop/Jefferies/flylight_mean_1000_3chan.raw \
-s $filename_img_affine \
-o $filename_img_tps \
-L /groups/peng/home/qul/Desktop/Jefferies/local_align/target_local.marker \
-l /groups/peng/home/qul/Desktop/Jefferies/local_align/subject_local.marker \
-w 10 \
-B 1024"

		eval "$command_tpstrans"
	fi	
}


fun_traversal_dir()
{
for filename in $(ls $1)
do
	echo "$1/$filename"
	echo "\n\n"
	echo "####################################################################################"

	if [ -f "$1/$filename" ]
        then
        	echo ">>$1/$filename"
		echo ">>this is a file => call processing function ..."
		dosomething "$1/$filename"
        elif [ -d "$1/$filename" ]
        then
		echo ">>$1/$filename"
        	echo ">>This is a directory => do traversal ..."
		fun_traversal_dir "$1/$filename"
        fi
done
}



# main
dirname_input=$1
dirname_output=$2

fun_traversal_dir $dirname_input


