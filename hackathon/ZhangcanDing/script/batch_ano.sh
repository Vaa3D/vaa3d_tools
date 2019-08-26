#!/bin/bash

inputfolder=$1
outputfolder=$2
anonumber=$3

cd $inputfolder
for j in `ls -d *`
do
	cp $inputfolder/$j/$j.apo $outputfolder/
	cp $inputfolder/$j/$j.swc $outputfolder/ 
done

for ((i=1;i<$anonumber+1;i++))
do
	mkdir $outputfolder/$i
	for  file1 in $(ls $outputfolder/*swc |awk -F "/" '{print $NF}' |head -20)
	do
	vaa3d -x resample_swc -f resample_swc -i $outputfolder/$file1 -o $outputfolder/$file1 -p 100
	vaa3d -x change_type -f swc -i $outputfolder/$file1 -o $outputfolder/$file1 -p ${file1:8:1}
	mv $outputfolder/$file1 $outputfolder/$i
	done
done

for ((i=1;i<$anonumber+1;i++))
do	
	for  file1 in $(ls $outputfolder/*apo |awk -F "/" '{print $NF}' |head -20)
	do
	vaa3d -x apo_to_marker -f add_custom_name -i $outputfolder/$file1 -o $outputfolder/$file1 -p ${file1:4:5}
	mv $outputfolder/$file1 $outputfolder/$i
	done
done

cd $outputfolder
for i in `ls -d *`
do
	vaa3d -x linker_file_generator -f linker -i $outputfolder/$i -o $outputfolder/$i/$i.ano -p 7
done
