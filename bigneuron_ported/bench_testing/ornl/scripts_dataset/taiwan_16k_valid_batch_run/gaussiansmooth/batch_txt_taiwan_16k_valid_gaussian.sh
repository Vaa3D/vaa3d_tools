#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/img_gaussiansmooth/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var
	i=24;
#	mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/gaussiansmooth/results/$filename
	for i in {25..27}
	do
		 sh /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_job_text_scripts.sh $i /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/img_gaussiansmooth/$filename/${filename}.g.v3draw /lustre/atlas2/nro101/proj-shared/BigNeuron/data/taiwan16k/reconstructions_for_img_gaussiansmooth/$filename /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/gaussiansmooth/text_jobs/$i/$var.txt

	done
var=$((var+1))	  
done
