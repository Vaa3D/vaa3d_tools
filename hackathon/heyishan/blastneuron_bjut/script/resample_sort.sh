#!/bin/sh

v3dPATH=$1
neuron_folder=$2
neuron_name=$3
filename=$4
consensus_swc=$5
resample_PARA=$6
result_folder=$7

alias v3d='$v3dPATH/vaa3d'
shortname=${filename%.*}
shortname=${shortname##*v3dpbd_}

refswc="$consensus_swc"
inswc="$neuron_folder/$filename"
outswc="$result_folder/resample_sort/$neuron_name/${shortname}_sort.swc"

v3d -x blastneuron_bjut -f pre_process -i $inswc $consensus_swc -o $outswc -p $resample_PARA
