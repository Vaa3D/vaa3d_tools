# local_alignment and prune_alignment
#!/bin/sh

v3dPATH=$1
result_folder=$2
neuron_name=$3
tfilename=$4
consensus_swc=$5
cut_PARA=$6

alias v3d='$v3dPATH/vaa3d'
shortname=${tfilename%_*}
echo "shortname in local_lignment = "$shortname
inswc1="$result_folder/resample_sort/$neuron_name/$tfilename"
inswc2="$consensus_swc"
outswc="$result_folder/local_alignment/$neuron_name/${shortname}_matching.swc"
v3d -x blastneuron_bjut -f apply_blastneuron -i $inswc1 $inswc2 -o $outswc

inswc_p="$result_folder/local_alignment/$neuron_name/${shortname}_matching.swc"
outswc_p="$result_folder/prune_alignment/$neuron_name/${shortname}_matching${cut_PARA}.swc"
v3d -x blastneuron_bjut -f prune_alignment -i $inswc_p -o $outswc_p -p $cut_PARA
