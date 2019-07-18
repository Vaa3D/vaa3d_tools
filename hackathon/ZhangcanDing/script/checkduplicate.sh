#! /bin/bash

inputfolder=$1

cd $inputfolder

declare -a arr;
unset arr
for i in `ls *.eswc`
do 
	arr+=($i)
done

for i in {1..9..1}
do
	ID=${arr[$i]}
	ID1=${arr[$i+1]}
	if [ ${ID:0:11} == ${ID1:0:11} ];then # == must have space 
		echo $ID $ID1
	fi
done

