#!/bin/bash
# pbs launching script example for NAMD job
# PLEASE DO NOT CHANGE THIS FILE. TO SUBMIT QUEUE, PLS MAKE A COPY OF THIS FILE AND MAKE THE ACCORDING CHANGES

#     job name:
#PBS -N NEUROSTALKER 
#PBS -P RDS-FEI-NRMMCI-RW
#PBS -q compute 

#     how many cpus?
#PBS -l ncpus=4

#PBS -l pmem=1600mb

# How long to run the job? (hours:minutes:seconds)
#PBS -l walltime=0:20:0

#     Name of output file:
#PBS -o teststalker-output.txt

#     Environmental varibles to make it work:
 
module load gcc;
cd $PBS_O_WORKDIR;
 
#     Launching the job!
JOBNAME='comp-gvf';

DATETIME=$(date +"%F-%T");

#     Transfer the trained model to data folder
cachefolder=/project/RDS-FEI-NRMMCI-RW/stalkerv3dcache/$JOBNAME$DATETIME;

if [ ! -d "$cachefolder" ];then
	echo "Creating cache job folder: $cachefolder";
	mkdir $cachefolder;
else
	echo "Cache job folder exists: $cachefolder";
fi

me="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"
cp $me $cachefolder/run.pbs;

#     Run Script
vaa3d=../../../../v3d_external/bin/vaa3d;
#vaa3d=/home/siqi/Vaa3D_Ubuntu_64bit_v3.045/vaa3d;

# Make Locally and Run on the server
#qmake;
#make -j4;

$vaa3d -x NeuroStalker -f tracing_func -i test/testdata/00810.FruMARCM-M002018_seg001.lsm.tif.c3.v3draw.uint8.v3draw -p 1

echo "Done"
