#!/bin/bash

for i in {1..164}
do

	    /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_pbs_scripts.sh 27 27 /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/pbs_all/gold166_$i.pbs /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/runVAA3D_all  runVAA3D_gold166_$i.exe 1

done 
