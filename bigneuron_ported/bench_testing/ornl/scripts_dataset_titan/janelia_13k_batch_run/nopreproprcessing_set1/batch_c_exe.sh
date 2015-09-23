#!/bin/bash

for i in {1..27}
do
	echo $i
	var=0;
        for j in {1..7}
        do
       /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_c_exe_scripts.sh /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/nopreproprcessing_set1/runVAA3D_all/runVAA3D_${i}_part${j}.c /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/nopreproprcessing_set1/text_jobs/$i /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1 $var
	var=$((var+1000))
	done

done
