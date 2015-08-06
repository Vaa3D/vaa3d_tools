#!/bin/bash
cd /data/mat/zhi/BigNeuron/first2000/images/
for foldername in `ls -d *`
do	
	cd $foldername	
	for filename in `ls *.qsub`
	do
	    qsub $filename 
	  		
	done
	cd ..
done
