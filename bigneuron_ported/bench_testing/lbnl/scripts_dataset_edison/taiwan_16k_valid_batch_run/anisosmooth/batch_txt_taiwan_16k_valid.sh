#!/bin/bash
cd /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/img_anisosmooth/
var=0;    
for filename in `ls -d *`
do
        echo $filename
        echo $var
        i=25;
       # mkdir /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/reconstructions_for_img_anisosmooth/$filename
       # for i in {25..27}
       # do
        #       mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/anisosmooth/text_jobs/$i
                sh /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_job_text_scripts.sh $i /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/img_anisosmooth/$filename/${filename}_anisodiff.raw /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/reconstructions_for_img_anisosmooth/$filename /global/project/projectdirs/m2043/BigNeuron/Bin_Vaa3D_BigNeuron_version1 /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/text_jobs/$i//$var.txt
#	done
	var=$((var+1))
done
