#!/bin/bash

for i in {1..27}
do
	j=16
#	for j in {1..16}
#	do
        /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_pbs_scripts.sh 921 200 /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/pbs_all/taiwan16_aniso_${i}_part${j}.pbs /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/runVAA3D_all/ runVAA3D_${i}_part${j}.exe
#	done
done
