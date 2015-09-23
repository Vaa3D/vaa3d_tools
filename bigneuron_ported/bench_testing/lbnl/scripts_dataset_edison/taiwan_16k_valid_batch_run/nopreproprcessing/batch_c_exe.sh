#!/bin/bash

for i in {25..27}
do
	var=0;
	for j in {1..16}
	do
       /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_c_exe_scripts.sh /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/nopreproprcessing/runVAA3D_all/runVAA3D_${i}_part${j}.c /global/project/projectdirs/m2043/BigNeuron/zhi/taiwan_16k_valid_batch_run/nopreproprcessing/text_jobs/$i /global/project/projectdirs/m2043/BigNeuron/Bin_Vaa3D_BigNeuron_version1 $var
	var=$((var+1000))
	done

done
