#!/bin/bash

inputswcfolder=$1
inputimagefolder=$2
outputfolder=$3

cd $inputswcfolder
for filename in `ls *_sorted.swc`
do
	if [ ! -f $outputfolder/${filename}_tresh40.swc ] || [ ! -f $outputfolder/${filename}_ada.swc ];then
		vaa3d -x /home/penglab/v3d_external/bin/plugins/neuron_utilities/neuron_radius/libneuron_radius.so -f neuron_radius_terafly -i $inputimagefolder $filename -o $outputfolder/${filename}_tresh40.swc -p 40 1
		vaa3d -x /home/penglab/v3d_external/bin/plugins/neuron_utilities/neuron_radius/libneuron_radius.so -f neuron_radius_terafly -i $inputimagefolder $filename -o $outputfolder/${filename}_ada.swc -p AUTO 1
	fi
	
done


