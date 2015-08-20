#!/bin/bash
V3D="../../../../v3d_external/bin/vaa3d"
if [ ! $# = 4 ]
then
	echo -e "\nscript for neuron comparison, Usage:"
	echo -e "sh blastneuron_plugin.sh <featurebase.nfb> <query_neuron.swc> <candidate_number> <output_folder>\n"
else
	neuron_featurebase=$1
	query_neuron=$2
	cand_num=$3
	output_folder=$4
	if [ -d $output_folder ]
	then
		echo "warning: $output_folder already exists, now remove it"
		rm -r $output_folder
	fi
	mkdir $output_folder
	cmd="$V3D -x blastneuron -f pre_processing -p \"#i $query_neuron #o $output_folder/query_preprocessed.swc\""
	echo $cmd
	eval $cmd
	cmd="$V3D -x blastneuron -f global_retrieve -p \"#d $neuron_featurebase #q $query_neuron #n $cand_num #m 1,2,4 #o $output_folder/retrieved.ano\""
	echo $cmd
	eval $cmd
	count=0
	seq=`jot $cand_num 4`
	for i in $seq
	do
		tgt=`sed -n "$i"p $output_folder/retrieved.ano | awk -F = '{print $2}'`
		cmd="$V3D -x blastneuron -f pre_processing -p \"#i $tgt #o $output_folder/tgt_"$count"_preprocessed.swc\""
		echo $cmd
		eval $cmd
		cmd="$V3D -x blastneuron -f inverse_projection -p \"#t  $output_folder/query_preprocessed.swc #s $output_folder/tgt_"$count"_preprocessed.swc #o $output_folder/tgt_"$count"_invp.swc\""
		echo $cmd
		eval $cmd
		cmd="$V3D -x blastneuron -f local_alignment -p \"#t $output_folder/query_preprocessed.swc #s $output_folder/tgt_"$count"_preprocessed.swc #o "$output_folder/match$count".swc #r $output_folder/tgt_"$count"_invp.swc\""
		echo $cmd
		eval $cmd
		echo "SWCFILE=$query_neuron" >> $output_folder/match$count.ano
		echo "SWCFILE=$tgt" >> $output_folder/match$count.ano
		echo "SWCFILE=match$count.swc" >> $output_folder/match$count.ano
		count=$((count + 1))
	done
fi

