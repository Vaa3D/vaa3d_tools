#!/bin/bash
cd /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/
for foldername in `ls  *.v3draw`
do	
	mkdir /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/images/$foldername
	mv $foldername /lustre/atlas/proj-shared/nro101/BigNeuron/first2000/images/$foldername
        mkdir /lustre/atlas/proj-shared/nro101/BigNeuron//first2000/reconstructions/$foldername

done
