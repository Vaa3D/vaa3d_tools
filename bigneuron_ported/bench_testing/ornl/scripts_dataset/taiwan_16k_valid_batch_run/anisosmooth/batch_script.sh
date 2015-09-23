#!/bin/bash

for i in {1..27}
do
    	# for j in {1..16}
        # do
	j=16
	    /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_pbs_scripts.sh 921  100 /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/pbs_all/taiwan16_aniso_${i}_part${j}.pbs  /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/taiwan_16k_valid_batch_run/anisosmooth/runVAA3D_all/ runVAA3D_${i}_part${j}.exe
	#done
done 
