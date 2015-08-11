#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166/
for foldername in `ls -d checked*`
do	
	echo $foldername
	cd $foldername
#	mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_ORNL/$foldername
	for subfolder in $(ls -d */)	
	do 

		cd $subfolder
#		mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_ORNL/$foldername/$subfolder
		for image in $(ls *.v3dpbd *.v3draw)
		do
			 for i in {1..21}
       			 do
			 /lustre/atlas/proj-shared/nro101/BigNeuron/gen_bench_job_scripts.sh $i /lustre/atlas/proj-shared/nro101/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166/$foldername/$subfolder$image /lustre/atlas/proj-shared/nro101/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_ORNL/$foldername/$subfolder /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1
			done
		done
		cd ..
	done
	cd ..
done
