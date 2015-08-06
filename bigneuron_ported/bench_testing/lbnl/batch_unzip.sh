#!/bin/bash
cd /global/homes/z/zhizhou/bigneuron/first2000/
for foldername in `ls  *.zip`
do	
	unzip $foldername
done
