#! /bin/bash

inputfolder=$1
outputfolder=$2
num=$3

cd $inputfolder
for file in `ls`
do	
	if [ ! -f $outputfolder/$file ]; then
	cp $file $outputfolder/
	fi
done
 
for ((i=1;i<6;i++))

do 
	mkdir $outputfolder/../$i
	for  file1 in $(ls $outputfolder |awk -F "/" '{print $NF}' |head -$num)
	do  
	
	mv $outputfolder/$file1 $outputfolder/../$i
	done
done

