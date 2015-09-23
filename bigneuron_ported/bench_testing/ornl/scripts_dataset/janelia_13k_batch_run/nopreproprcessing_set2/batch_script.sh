#!/bin/bash

for i in {1..27}
do
    	for j in {1..7}
        do
#	j=8	
	    /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_pbs_scripts.sh 1000 100 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/nopreproprcessing_set2/pbs_all/janelia13_set2_nopre_${i}_part${j}.pbs  /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/nopreproprcessing_set2/runVAA3D_all/ runVAA3D_${i}_part${j}.exe
	done
done 
