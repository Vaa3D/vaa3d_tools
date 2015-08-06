#!/bin/bash
cd /global/homes/z/zhizhou/bigneuron/first2000/
for foldername in `ls  *.v3draw`
do	
	mkdir /global/homes/z/zhizhou/bigneuron/first2000/images/$foldername
	mv $foldername /global/homes/z/zhizhou/bigneuron/first2000/images/$foldername
        mkdir /global/homes/z/zhizhou/bigneuron/first2000/reconstructions/$foldername

done
