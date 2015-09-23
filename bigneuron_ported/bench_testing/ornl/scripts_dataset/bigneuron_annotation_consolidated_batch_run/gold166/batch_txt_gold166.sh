#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/gold166/
j=1;
for i in {1..27}
do
#	if [[ ( $i != 1 && $i != 5 ) && ( $i != 6 && $i != 8 )  && ( $i != 21 && $i != 18 )]];
#	then
	var=1;
	echo $i
#	mkdir /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$var
	for foldername in `ls -d checked*`
	do
        #	echo $foldername
        	cd $foldername
#        	mkdir /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername
        	for subfolder in $(ls -d */)
        	do

                	cd $subfolder
 #               	mkdir /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername/$subfolder
                	for image in $(ls *.v3dpbd *.v3draw)
                	do
       	                      # mkdir /global/project/projectdirs/m2043/BigNeuron/gold166_batch_run/jobs_text/$i
                         	sh /lustre/atlas2/nro101/proj-shared/BigNeuron/Script_Vaa3D_svn_ORNL/script_MPI/gen_bench_job_text_scripts.sh $i /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/gold166/$foldername/$subfolder$image /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername/$subfolder /lustre/atlas2/nro101/proj-shared/BigNeuron/Bin_Vaa3D_BigNeuron_version1  /lustre/atlas2/nro101/proj-shared/BigNeuron/zhi/bigneuron_annotation_consolidated_batch_run/gold166/text_jobs/$var/$j.txt
                       	done
                	var=$((var+1))
                cd ..
        done
        cd ..
	done
        j=$((j+1))
#	fi
done
