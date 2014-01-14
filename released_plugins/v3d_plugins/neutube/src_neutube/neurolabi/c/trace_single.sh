#!/bin/sh
if test $# -ne 1
then
    echo 'trace.sh <filename>'
else
    echo $#
    echo $1
    data_dir=../data/$1
    bin_dir=bin
    if test -d $data_dir
    then
	rm $data_dir/*.wrl
	rm $data_dir/*.bn
	rm $data_dir/traced.tif
	rm $data_dir/trace_mask.tif

	echo 'compiling ...'
	make PROG=fly_neuron_mask
	make PROG=fly_neuron_seed
	make PROG=fly_neuron_blobmask
	make PROG=single_neuron_soma
	make PROG=fly_neuron_bundle
	make PROG=fly_neuron_grow_bundle
	make PROG=fly_neuron_bundle_seed
	make PROG=trace_fly_neuron2
	make PROG=fly_neuron_label2

		
	if ! test -f $data_dir/mask.tif
	then
	    $bin_dir/fly_neuron_mask $1
	fi
	
	if ! test -f $data_dir/seeds.pa
	then
	    $bin_dir/fly_neuron_seed $1
	fi

	if ! test -f $data_dir/blobmask.tif
	then
	    $bin_dir/fly_neuron_blobmask $1
	fi

	if ! test -f $data_dir/soma.tif
	then
	    $bin_dir/single_neuron_soma $1
	fi

	if ! test -f $data_dir/bundle.tif
	then
	    $bin_dir/fly_neuron_bundle $1
	fi

	if ! test -f $data_dir/grow_bundle.tif
	then
	    $bin_dir/fly_neuron_grow_bundle $1
	fi
	
	if ! test -f $data_dir/bundle_seeds.pa
	then
	    $bin_dir/fly_neuron_bundle_seed $1
	fi

	$bin_dir/trace_fly_neuron2 $1
	$bin_dir/fly_neuron_label2 $1
    fi
fi

