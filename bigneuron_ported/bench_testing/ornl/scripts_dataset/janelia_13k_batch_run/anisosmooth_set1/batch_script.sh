#!/bin/bash

for i in {1..27}
do
	j=7    
	# for j in {1..7}
       #  do
	
	    /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_pbs_scripts.sh 299 30 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/anisosmooth_set1/pbs_all/janelia13_set1_aniso_${i}_part${j}.pbs  /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/anisosmooth_set1/runVAA3D_all/ runVAA3D_${i}_part${j}.exe
#	done
done 
