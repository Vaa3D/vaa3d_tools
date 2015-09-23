#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_anisosmooth/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var
  	mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/reconstructions_for_img_anisosmooth/$filename
	sh /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_svn_ORNL/gen_bench_job_text_scripts.sh 2 /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_anisosmooth/$filename/${filename}_anisodiff.raw /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/reconstructions_for_img_anisosmooth/$filename /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1 /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/APP2_aniso_jobs/$var.txt
var=$((var+1))	  		
done
