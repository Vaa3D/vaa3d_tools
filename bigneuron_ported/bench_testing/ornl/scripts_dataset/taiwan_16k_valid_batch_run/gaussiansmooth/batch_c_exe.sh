#!/bin/bash

for i in {1..24}
do
	var=0;
        for j in {1..16}
        do

       /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_c_exe_scripts.sh /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/gaussiansmooth/runVAA3D_all/runVAA3D_${i}_part${j}.c /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/gaussiansmooth/text_jobs/$i /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 $var
	var=$((var+1000))
	done
done
