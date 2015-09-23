#!/bin/bash
cd /global/project/projectdirs/m2043/BigNeuron/Data/bigneuron_annotation_consolidated_20150715/gold166/
j=1;
for i in {1..27}
do
#	if [[ ( $i != 1 && $i != 5 ) && ( $i != 6 && $i != 8 )  && ( $i != 21 && $i != 18 )]];
#	then
	var=1;
	echo $i
#	mkdir /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$var
	for foldername in `ls -d checked*`
	do
        #	echo $foldername
        	cd $foldername
#        	mkdir /global/project/projectdirs/m2043/BigNeuron/Data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername
        	for subfolder in $(ls -d */)
        	do

                	cd $subfolder
#  	             	mkdir /global/project/projectdirs/m2043/BigNeuron/Data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername/$subfolder
                	for image in $(ls *.v3dpbd *.v3draw)
                	do
       	                      # mkdir /global/project/projectdirs/m2043/BigNeuron/gold166_batch_run/jobs_text/$i
                         	sh /global/project/projectdirs/m2043/BigNeuron/Script_Vaa3D_svn_LBNL/script_MPI/gen_bench_job_text_scripts.sh $i /global/project/projectdirs/m2043/BigNeuron/Data/bigneuron_annotation_consolidated_20150715/gold166/$foldername/$subfolder$image /global/project/projectdirs/m2043/BigNeuron/Data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername/$subfolder /global/project/projectdirs/m2043/BigNeuron/Bin_Vaa3D_BigNeuron_version1  /global/project/projectdirs/m2043/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$var/$j.txt
                       	done
                	var=$((var+1))
                cd ..
        done
        cd ..
	done
        j=$((j+1))
#	fi
done
