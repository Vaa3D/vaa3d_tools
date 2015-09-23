#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/img_gaussiansmooth/
for filename in `ls -d *`
do
	cd $filename
	echo $filename
	for file in `ls *.swc`
	do
		mv $file /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/reconstructions_for_img_gaussiansmooth/$filename/
	done
	cd .. 	
  #	/lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/anisosmooth/results/$filename
done

