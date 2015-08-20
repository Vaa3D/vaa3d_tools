#!/bin/bash
cd /global/project/projectdirs/m2043/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166/
for foldername in `ls -d checked*`
do	
	echo $foldername
	cd $foldername
#	mkdir /global/project/projectdirs/m2043/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_LBNL/$foldername
	for subfolder in $(ls -d */)	
	do 

		cd $subfolder
#		mkdir /global/project/projectdirs/m2043/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_LBNL/$foldername/$subfolder
		for image in $(ls *.v3dpbd *.v3draw)
		do
			 for i in {1..21}
       			 do
			 /global/homes/z/zhizhou/bigneuron/BigNeuron_bench_test_LBNL_v1/gen_bench_job_aprun_scripts.sh $i /global/project/projectdirs/m2043/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166/$foldername/$subfolder$image /global/project/projectdirs/m2043/BigNeuron/bigneuron_annotation_consolidated_20150715/gold166_LBNL/$foldername/$subfolder /global/homes/z/zhizhou/bigneuron/BigNeuron_bench_test_LBNL_v1/Vaa3D_BigNeuron_version1
			done
		done
		cd ..
	done
	cd ..
done
