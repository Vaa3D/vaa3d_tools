#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/images/
var=0;
for filename in `ls -d *`
do
	echo $filename
	echo $var

	sh /lustre/atlas/proj-shared/nro101/BigNeuron/gen_bench_job_text_scripts.sh 2 /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/images/$filename/$filename /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/reconstructions/$filename /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1 /lustre/atlas/proj-shared/nro101/BigNeuron/APP2_jobs/$var.txt
var=$((var+1))	  		
done
