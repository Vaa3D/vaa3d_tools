#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1/
for i in {1..27}
do	
	/usr/bin/time -v sh /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/janelia_13k_batch_run/gaussiansmooth_set2/text_jobs/$i/0.txt 
done 
