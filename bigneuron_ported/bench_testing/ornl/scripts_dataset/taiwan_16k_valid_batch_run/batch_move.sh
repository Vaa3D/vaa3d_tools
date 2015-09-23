#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/reconstructions_for_img_nopreproprcessing/
for foldername in `ls -d */`
do      
	echo $foldername
#        mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/images/$foldername
#        mv $foldername /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/images/$foldername
#        mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/reconstructions/$foldername
	 mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_gaussiansmooth/$foldername
	 mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_anisosmooth/$foldername

done
