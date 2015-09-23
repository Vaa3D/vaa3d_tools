#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/img_anisosmooth/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var
  	#mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/anisosmooth/results/$filename
	for i in {25..27}
	do
	#	mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/anisosmooth/text_jobs/$i
		sh /lustre/atlas2/nro101/proj-shared/scripts/Script_Vaa3D_svn_ORNL_tmp/script_MPI/gen_bench_job_text_scripts.sh $i /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/img_anisosmooth/$filename/${filename}_anisodiff.raw /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/reconstructions_for_img_anisosmooth/$filename /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/text_jobs/$i//$var.txt
	done
var=$((var+1))	  		
done

