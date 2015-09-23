#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_nopreproprcessing/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var

	sh /lustre/atlas/proj-shared/nro101/BigNeuron/gen_bench_job_text_scripts.sh aniso /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_nopreproprcessing/$filename/$filename /lustre/atlas/proj-shared/nro101/BigNeuron/data/taiwan16k/img_anisosmooth/$filename /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1 /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/aniso_jobs/$var.txt
var=$((var+1))	  		
done
