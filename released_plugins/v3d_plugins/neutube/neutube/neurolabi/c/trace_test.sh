#!/bin/sh
if test $# -ne 2
then
    echo 'trace.sh <filename> <matlab_path>'
else
    echo $#
    echo $1
    imgfile=../data/$1.tif
    orgfile=../data/$1_org.tif
    data_dir=../data/$1
    bin_dir=bin
    if test -d $data_dir
    then
	echo here
	if ! test -f $outfile
	then
	    echo here
	fi	
	
	outfile=$data_dir/mask.tif
	if ! test -f $outfile
	then
	    echo here 
	fi

	outfile=$data_dir/seeds.bn
	if ! test -f $outfile
	then
	    echo here
	fi
	
	$bin_dir/trace_neuron
    fi
fi