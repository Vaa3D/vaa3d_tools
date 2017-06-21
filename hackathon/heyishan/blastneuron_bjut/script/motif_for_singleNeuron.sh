#!/bin/sh
#example: /home/hys/Desktop/bat_test3/script/motif_for_singleNeuron.sh /home/hys/v3d_external/bin '/home/hys/Desktop/bat_test3/GMR_57C10_AD_01-1xLwt_attp40_4stop1-f-A-20111026_4_A2-right_optic_lobe.v3draw.extract_0' '/home/hys/Desktop/bat_test3/1.v3dpbd' '/home/hys/Desktop/bat_test3/consensus.strict.swc' '/home/hys/Desktop/bat_test3/result'
# if consensus doesn't exist, please replace it with "0"

v3dPATH=$1
neuron_folder=$2
imgPATH=$3
consensus_in=$4
result_folder=$5
pre_process_time=30
local_alignment_time=30
overlap_time=300
resample_PARA=3
cut_PARA=20
prune_THRES=3
fileNUM=10

scriptPATH=$(dirname $(readlink -f $0))
neuron_name=`basename $neuron_folder`
del_flag=1
alias v3d='$v3dPATH/vaa3d'

# generate consensus
mkdir $result_folder/consensus_file
if [ "$consensus_in" = "0" ]; then
	echo "generate consensus"
	del_flag=0
	v3d -x consensus_swc -f consensus_swc -i $neuron_folder/*.swc -o $result_folder/consensus_file/${neuron_name}_consensus.swc -p 6 5
	consensus_swc="$result_folder/consensus_file/${neuron_name}_consensus.swc"
elif [ -f "$consensus_in" ];then
	extension=${consensus_in##*.}
	if [ "$extension" = "swc" ]; then
		consensus_swc=$consensus_in
	elif [ "$extension" = "eswc" ]; then
		v3d -x eswc_converter -f eswc_to_swc -i ${consensus_in} -o $result_folder/consensus_file/${neuron_name}_consensus.swc
		consensus_swc="$result_folder/consensus_file/${neuron_name}_consensus.swc"
	else
		echo "wrong consensus input"
		exit 0
	fi
else
	echo "wrong consensus input"
	exit 0
fi

# sort swc
consensus_sorted="$result_folder/consensus_file/${neuron_name}_consensus_sorted.swc"
v3d -x sort_neuron_swc -f sort_swc -i ${consensus_swc} -o ${consensus_sorted} -p 1000 1
# pre_process
mkdir $result_folder/resample_sort
mkdir $result_folder/resample_sort/$neuron_name

if [ "$consensus_sorted" != "0" ]; then
	cd $neuron_folder
	for filename in `ls *.swc`
		do
		echo "filename = "$filename
		timeout $pre_process_time $scriptPATH/resample_sort.sh $v3dPATH $neuron_folder $neuron_name $filename $consensus_sorted $resample_PARA $result_folder
		done

# if consensus doesn't exist
elif [ "$consensus_sorted" = "0" ]; then
	echo "You need create a consensus."
else
	echo "You need a consensus_swc as the forth input"
fi

# lcoal_alignment and prune_alignment
mkdir $result_folder/local_alignment
mkdir $result_folder/local_alignment/$neuron_name
mkdir $result_folder/prune_alignment
mkdir $result_folder/prune_alignment/$neuron_name
cd $result_folder/resample_sort/$neuron_name
for tfilename in `ls *.swc`
	do
	timeout $local_alignment_time $scriptPATH/local_alignment.sh $v3dPATH $result_folder $neuron_name $tfilename $consensus_sorted $cut_PARA
	done

# select files by files' size
if [ -d $result_folder/prune_alignment/$neuron_name ]; then
	mkdir $result_folder/selected
	mkdir $result_folder/selected/$neuron_name
	cd $result_folder/prune_alignment/$neuron_name
	for file_select in `ls -1 -S | head -$fileNUM`
		do
		cp $file_select $result_folder/selected/$neuron_name/$file_select
		done
else
	echo "You need do local_alignment first"
fi

# remove consensus_swc
cd $result_folder/consensus_file
rm -f ${neuron_name}_consensus.swc_SelectedNeurons.ano
#if [ "${del_flag}" = "1" ];then 
#	rm -f ${neuron_name}_consensus.swc
#fi

# overlap with 2 methods
mkdir $result_folder/overlap
mkdir $result_folder/overlap/$neuron_name
method1="2"
method2="3"

# method1(consensus)
for dist_PARA in 3 5;
	do
	for meth_PARA in 4 6 8;
		do
		timeout ${overlap_time} $scriptPATH/overlap.sh $v3dPATH $result_folder $neuron_name $imgPATH $method1 $dist_PARA $meth_PARA $prune_THRES
		done
	done
# method2(consensus_mean)
dist_PARA=0
for meth_PARA in 4 6 8;
	do
	timeout $overlap_time $scriptPATH/overlap.sh $v3dPATH $result_folder $neuron_name $imgPATH $method2 $dist_PARA $meth_PARA $prune_THRES
	done


