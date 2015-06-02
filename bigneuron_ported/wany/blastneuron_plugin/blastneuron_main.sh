if [ ! $# = 4 ]
then
	echo -e "\nscript for neuron comparison, Usage:"
	echo -e "sh blastneuron_main.sh <featurebase.nfb> <query_neuron.swc> <candidate_number> <output_folder>\n"
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
	prune_short_branch/prune_short_branch -i $query_neuron -o $output_folder/query_pruned.swc
	pre_processing/pre_processing -i $query_neuron -o $output_folder/query_preprocessed.swc
	global_cmp/neuron_retrieve -d $neuron_featurebase -q $query_neuron -n $cand_num -m 1,2,4 -o $output_folder/retrieved.ano
	count=0
	seq=`jot $cand_num 4`
	for i in $seq
	do
		tgt=`sed -n "$i"p $output_folder/retrieved.ano | awk -F = '{print $2}'`
		prune_short_branch/prune_short_branch -i $tgt -o $output_folder/tgt_"$count"_pruned.swc
		#pre_processing/pre_processing -i $tgt -o $output_folder/tgt_"$count"_preprocessed.swc
		pointcloud_match/main_pointcloud_match -t  $output_folder/query_pruned.swc -s $output_folder/tgt_"$count"_pruned.swc -o $output_folder/tgt_"$count"_invp.swc
		#pointcloud_match/main_pointcloud_match -t  $output_folder/query_preprocessed.swc -s $output_folder/tgt_"$count"_preprocessed.swc -o $output_folder/tgt_"$count"_invp.swc
		tree_matching/tree_matching -t $output_folder/query_pruned.swc -s $output_folder/tgt_"$count"_pruned.swc -o "$output_folder/match$count".swc -r $output_folder/tgt_"$count"_invp.swc
		#tree_matching/tree_matching -t $output_folder/query_preprocessed.swc -s $output_folder/tgt_"$count"_preprocessed.swc -o "$output_folder/match$count".swc -r $output_folder/tgt_"$count"_invp.swc
		echo "SWCFILE=$query_neuron" >> $output_folder/match$count.ano
		echo "SWCFILE=$tgt" >> $output_folder/match$count.ano
		echo "SWCFILE=match$count.swc" >> $output_folder/match$count.ano
		#echo "SWCFILE=query_preprocessed.swc" >> $output_folder/match$count.ano
		#echo "SWCFILE=tgt_"$count"_preprocessed.swc" >> $output_folder/match$count.ano
		#echo "SWCFILE=match$count.swc" >> $output_folder/match$count.ano
		count=$[count+1]
	done
fi

