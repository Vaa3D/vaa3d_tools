#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1/
for i in {1..23}
do	
	/usr/bin/time -v sh /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/1/$i.txt 
done 
