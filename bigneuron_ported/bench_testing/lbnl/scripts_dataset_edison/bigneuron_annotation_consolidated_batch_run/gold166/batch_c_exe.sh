#!/bin/bash

for i in {1..164}
do
	echo $i
         /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_c_exe_scripts.sh /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/runVAA3D_all/runVAA3D_gold166_$i.c /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$i /global/project/projectdirs/m2043/BigNeuron/Bin_Vaa3D_BigNeuron_version1 1

done
