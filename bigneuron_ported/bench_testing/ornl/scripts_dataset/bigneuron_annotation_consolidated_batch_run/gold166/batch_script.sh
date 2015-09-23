#!/bin/bash

for i in {1..164}
do

	    /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_pbs_scripts.sh 27 27 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/pbs_all/gold166_$i.pbs /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/runVAA3D_all  runVAA3D_gold166_$i.exe 1

done 
