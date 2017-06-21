#!/bin/sh

v3dPATH=$1
image_folder=$2
reconstruction_folder=$3
consensus_folder=$4
result_folder=$5
script_path=$6

#scriptPATH=$(dirname $(readlink -f $0))
#echo scriptPATH=$scripPATH
cd $reconstruction_folder
for neuron in `ls`
	do
	neuron_name=${neuron}
	#neuron_name=`basename $neuron`
	#neuron_name=${neuron_name%.*}
	img_name=${neuron_name}
	consensus_name=${neuron_name}_consensus.eswc
	neuron_folder=${reconstruction_folder}/$neuron
	neuron_img=${image_folder}/${img_name}/${img_name}
	neuron_consensus=${consensus_folder}/${consensus_name}
	mkdir $result_folder/${neuron_name}
	neuron_result=$result_folder/${neuron_name}
	
	cd $script_path
	./motif_for_singleNeuron.sh $v3dPATH ${neuron_folder} ${neuron_img} ${neuron_consensus} ${neuron_result}
	done


