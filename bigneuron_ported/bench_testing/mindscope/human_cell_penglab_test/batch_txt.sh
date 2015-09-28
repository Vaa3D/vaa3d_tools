#!/bin/bash
cd /data/mat/zhi/human_cell_penglab_test/raw_testing_images/
j=1;
for i in {1..27}
do
	var=1;
	echo $i
	for foldername in `ls -d *`
	do
        	cd $foldername
        	mkdir /data/mat/zhi/human_cell_penglab_test/reconstructions/$foldername
                	for image in $(ls *.v3dpbd *.v3draw)
                	do
			sh  /local3/human_cell_penglab_test/gen_bench_job_text_scripts.sh $i /data/mat/zhi/human_cell_penglab_test/raw_testing_images/$foldername/$foldername  /data/mat/zhi/human_cell_penglab_test/reconstructions/$foldername /data/mat/zhi/VAA3D/Bin_Vaa3D_BigNeuron_version1 /data/mat/zhi/human_cell_penglab_test//text_jobs/$var/$j.txt
                       	done
                	var=$((var+1))
                cd ..
	done
        j=$((j+1))
done
