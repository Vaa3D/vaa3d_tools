#!/bin/bash
cd /global/homes/z/zhizhou/bigneuron/BigNeuron_bench_test_LBNL_v1/testing_data/images/
for foldername in `ls -d *`
do	
	cd $foldername	
	for filename in `ls *.qsub`
	do
	    qsub $filename 
	  		
	done
	cd ..
done
