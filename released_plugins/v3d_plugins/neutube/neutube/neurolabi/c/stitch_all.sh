#!/bin/bash

#sh stitch_all.sh input_dir ncycle out_dir

period=$2
counter=0
imgdir=$1
filelist=''
result_id=1
result_dir=$3
logfile=$result_dir/log.txt

for imgfile in `ls "$imgdir"`
do
    filelist="$filelist '$imgdir/$imgfile'"
    counter=`expr $counter + 1`
    if [ $counter -eq $period ]
    then
	echo "$result_id : $filelist" >>$logfile
	echo "$filelist -o $result_dir/result$result_id.lsm -conn conn.txt -merge 4 -d8 1" | xargs bin/stitchstack 2>>$logfile
	counter=0;
	filelist=''
	result_id=`expr $result_id + 1`
    fi
done

#optional arguments for 'stitchstack':
# -merge <merge_style>
#   <merge_style> determines how overlapped regions are rendered. 
#   It can be 1, 2, 3 or 4.
#      1: maximum value
#      2: minimal value
#      3: average value
#      4: value of the first stack (default)
#
# -conn <file>
#   <file> is the path of the file spcifying connections. It should be a plain 
#   text file and has the following format:
#   -----------
#        n
#        a b
#        c d
#        ...
#   -----------
#   where n is the number of stacks to stitch and a, b, c, d, etc. specify
#   overlap relations. The numbers in the same row are the indices of the
#   the stacks that overlap with each other. If this option is not specified, 
#   the program will try to check all possiblities and find the best overlap
#   map.
#
# -d8 <map_option>
#   This option takes effect only for 16-bit stacks (12-bit is takens as 
#   16-bit here). When it is specified, the stitched stack will be saved as an 
#   8-bit image file. <map_option> determines the way of converting 16-bit to 
#   8-bit:
#     1: [min, max] => [0, 255]
#     2: [min, 99.99% quantile] => [0, 255]
#     3: log histogram equalization, which is a non-linear map
#
# -bgsub <option>
#   The background of each stack is subtracted before stitching when this 
#   argument is given. <option> can be one of these following values:
#     0: no subtraction (same as missing '-bgsub' option)
#     1: subtract first channel only
#     2: subtract second channel only
#     3: subtract third channel only
#     -100: subtract all channels separately
#     100: subtract all channels together
#   For single-channel images, the options have the same effect.
#
# -intv <dx> <dy> <dz>
#   Downsampling rate for alignment check.
#
# -ds <rx> <ry> <rz>
#   Downsample the stacks before stitching. The final image will be the
#   composition of the downsampled images. <rx>, <ry> and <rz> are downsampling
#   ratios along X, Y and Z dimensions respectively. They must be non-negative
#   integers.
#
# -config <cx> <cy> <cz>
#   Alignment prior for 2 stacks. -1: left; 0: middle; 1: right; 2: unknown
#