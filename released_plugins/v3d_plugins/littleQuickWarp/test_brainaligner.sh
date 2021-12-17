#!/bin/bash

#brainaligner

#mac
dir_exe=/Users/qul/work/brainaligner/jba/c++/
dir_data=/Users/qul/Desktop/testdata/

#linux
#dir_exe=/groups/peng/home/qul/myhome/work/brainaligner/jba/c++/
#dir_data=/groups/peng/home/qul/myhome/work/testdata/

#global
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/test2_256.tiff -o $dir_data/output_global.tiff -w 0 

#local
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global.tiff -o $dir_data/output_local.tiff -L $dir_data/test1_256.marker.marker -w 10 


#------------------------------------------------------------------------------------
#warp based on markers

#256*256*108*1------------------------------------------------------------------------------
#uint8
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global.tiff -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 1 
#float
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global_float32.v3draw -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 1

#512*512*216*1------------------------------------------------------------------------------
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global_512_uint8.v3draw -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 1
#float
$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global_512_float32.v3draw -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 0


#1024*1024*1024*1------------------------------------------------------------------------------
#uint8 test
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global_1024_uint8.v3draw -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 1

#float test
#$dir_exe/brainaligner -t $dir_data/test1_256.tiff -s $dir_data/output_global_1024_float32.v3draw -o $dir_data/output_warp_jba.v3draw -L $dir_data/output_target.marker -l $dir_data/output_subject.marker -w 10 -i 1



