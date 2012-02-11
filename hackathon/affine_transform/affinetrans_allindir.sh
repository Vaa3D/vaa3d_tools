#!/bin/bash
#
# affine transform the subject image to target according to the given point sets
#
# a demo to do use this script is:
# sh affinetrans_allindir.sh /Volumes/MyBook/Wayne/clones/ /Volumes/MyBook/Wayne/clones/mean_brain_295_ref_local_with_compartment_edges.tif.marker /Volumes/MyBook/Wayne/result/
#
# by Lei Qu
# 2010-03-18


dosomething()
{
	filename=$1
	suffix=${filename##*.}

	if [ $suffix = "tif" ]
	then
		echo "do affine transform"

filename_marker_sub="$filename"".marker"

filename_img_sub_nodir=${filename##*/}
filename_img_sub_nosuffix=${filename_img_sub_nodir%.*}
filename_img_out="$dirname_output""$filename_img_sub_nosuffix""_affine.tif"

command_affinetrans="/Users/qul/work/v3d_2.0/plugin_demo/affine_transform/main_affinetransform.app/Contents/MacOS/main_affinetransform \
-t $filename_marker_tar \
-s $filename_marker_sub \
-S $filename \
-o $filename_img_out \
-f 0 \
-W 1019 \
-H 601 \
-Z 216"

		eval "$command_affinetrans"
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
filename_marker_tar=$2
dirname_output=$3

fun_traversal_dir $dirname_input


