#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/img_gaussiansmooth/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var
#	mkdir /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/reconstructions_for_img_gaussiansmooth/$filename
	for i in {1..27}
	do
		 sh /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_job_text_scripts.sh $i /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/img_gaussiansmooth/$filename/${filename}.v3dpbd.g.v3draw /lustre/atlas2/nro101/proj-shared/BigNeuron/data/Janelia/set2_accepted_single/reconstructions_for_img_gaussiansmooth/$filename /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/gaussiansmooth_set2/text_jobs/$i/$var.txt

	done
var=$((var+1))	  
done
