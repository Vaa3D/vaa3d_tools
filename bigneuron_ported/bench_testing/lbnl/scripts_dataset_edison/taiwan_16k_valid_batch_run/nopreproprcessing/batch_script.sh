#!/bin/bash

for i in {1..27}
do
	#for j in {1..16}
	j=16
#	do
        /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_pbs_scripts.sh 921 200 /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/nopreproprcessing/pbs_all/taiwan16_nopre_${i}_part${j}.pbs /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/nopreproprcessing/runVAA3D_all/ runVAA3D_${i}_part${j}.exe
#	done
done
