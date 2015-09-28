#!/bin/bash

for i in {1..9}
do
	for j in {1..27}
	do
		#sh /local3/human_cell_penglab_test/text_jobs/$i/$j.txt
		/data/mat/zhi/human_cell_penglab_test/gen_bench_job_scripts.sh /data/mat/zhi/human_cell_penglab_test/text_jobs/$i/$j.txt /data/mat/zhi/VAA3D/Bin_Vaa3D_BigNeuron_version1 /data/mat/zhi/human_cell_penglab_test/qsub_files/${i}_${j}.qsub
	done
done
