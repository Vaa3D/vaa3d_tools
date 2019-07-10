#! /bin/bash

inputfolder=$1

cd $inputfolder

declare -a myarray

for i in ` ls RES* -d -v -r `
do 
	myarray+=($i)
done

echo ${myarray[1]} 
