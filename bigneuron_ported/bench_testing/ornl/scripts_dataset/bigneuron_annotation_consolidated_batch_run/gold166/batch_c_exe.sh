#!/bin/bash

for i in {1..164}
do
	echo $i
         /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_c_exe_scripts.sh /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/runVAA3D_all/runVAA3D_gold166_$i.c /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$i /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 1

done
