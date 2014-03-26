#!/bin/sh
if test $# -ne 1
then
    echo 'trace.sh <filename>'
else
    echo $#
    echo $1
    imgfile=../data/$1.tif
    orgfile=../data/$1_org.tif
    if ! test -f $orgfile
    then
	orgfile=$imgfile
    fi
    data_dir=../data/$1
    bin_dir=bin
    if test -d $data_dir
    then
	find $data_dir -name "*.wrl" -print | xargs rm
	find $data_dir -name "chain*.bn" -print | xargs rm
	rm $data_dir/soma*.bn
	rm $data_dir/traced.tif
	rm $data_dir/trace_mask.tif

	echo 'compiling ...'
	make PROG=imbinarize
	make PROG=bwsolid
	make PROG=trace_seed
	make PROG=detect_soma2
	make PROG=trace_neuron2

	outfile=$data_dir/bin.tif
	if ! test -f $outfile
	then
	    $bin_dir/imbinarize -Mlocmax -R$orgfile $imgfile -o $outfile
	fi	
	
	outfile=$data_dir/mask.tif
	if ! test -f $outfile
	then
	    $bin_dir/bwsolid -mf 4 $data_dir/bin.tif -o $outfile
	fi

	outfile=$data_dir/seeds
	if ! test -f $outfile
	then
	    $bin_dir/trace_seed $data_dir/mask.tif -o $outfile
	fi
	
	outfile=soma
	$bin_dir/detect_soma2 $imgfile  -mask trace_mask.tif -Sseeds -D$data_dir -o $outfile
	
	echo  "$bin_dir/trace_neuron2 $imgfile -Sseeds -D$data_dir -canvas traced.tif -mask trace_mask.tif -r -minlen 5"
	$bin_dir/trace_neuron2 $imgfile -Sseeds -D$data_dir -canvas traced.tif -mask trace_mask.tif -r -minlen 5
    fi
fi