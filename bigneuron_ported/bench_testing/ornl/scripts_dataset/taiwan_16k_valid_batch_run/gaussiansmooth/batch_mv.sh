#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/img_gaussiansmooth/
for filename in `ls -d *`
do
	cd $filename
	echo $filename
	for file in `ls *.swc`
	do
	#	echo $file 
		mv $file /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/reconstructions_for_img_gaussiansmooth/$filename/
	done
	cd .. 	
  #	/lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/gaussiansmooth/results/$filename
done

