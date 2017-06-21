#!/bin/sh
v3dPATH=$1
result_folder=$2
neuron_name=$3
imgPATH=$4
method=$5
dist_PARA=$6
meth_PARA=$7
prune_THRES=$8

alias v3d='$v3dPATH/vaa3d'

if [ "$method" = "2" ]; then
	method_name="consensus"
elif [ "$method" = "3" ]; then
	method_name="consensus_mean"
else
	echo "method is wrong in overlap.sh"
fi

infolder=$result_folder/selected/$neuron_name
savename=${method_name}_${dist_PARA}_${meth_PARA}.swc
outswc=$result_folder/overlap/$neuron_name/$savename
inimg=$imgPATH

v3d -x blastneuron_bjut -f overlap_gold -i $infolder $inimg -o $outswc -p $method $dist_PARA $meth_PARA $prune_THRES

cd $result_folder/overlap/$neuron_name
if [ -f "$savename" ]; then
#if [ "`ls $savename`" != "" ]; then
	nrow=`cat $savename|wc -l`
	if [ $nrow -gt  2 ]; then 
		echo GRAYIMG=${inimg} >${result_folder}/overlap/${neuron_name}/${method_name}_${dist_PARA}_${meth_PARA}.ano
		echo SWCFILE=${savename} >>${result_folder}/overlap/${neuron_name}/${method_name}_${dist_PARA}_${meth_PARA}.ano
	else
		rm ${savename}	
	fi
fi

