#!/bin/sh
if test $# -ne 1
then
    echo 'trace.sh <filename>'
else
    source assert_succ.sh

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
	make PROG=fly_neuron_highpass
	make PROG=fly_neuron_lowpass
	make PROG=fly_neuron_highpass_binarize
	make PROG=fly_neuron_lowpass_binarize
	make PROG=fly_neuron_mask
	make PROG=rmsobj
	make PROG=fly_neuron_holemask
	make PROG=fly_neuron_seed
	make PROG=fly_neuron_blobmask
	make PROG=fly_neuron_holeobj
	make PROG=fly_neuron_soma
	make PROG=fly_neuron_bundle
	make PROG=fly_neuron_grow_bundle
	make PROG=fly_neuron_bundle_seed
	make PROG=bwinv
	#make PROG=trace_fly_neuron2
	#make PROG=trace_neuron2
	make PROG=fly_neuron_label2
	make PROG=reconstruct_neuron

	if ! test -f $data_dir/highpass.tif
	then
	    echo 'High pass filtering ...'
	    $bin_dir/fly_neuron_highpass $1
	    assert_succ
	fi
	
	if ! test -f $data_dir/highpass_binary.tif
	then
	    echo 'Binarize high pass image ...'
	    $bin_dir/fly_neuron_highpass_binarize $1
	    assert_succ
	fi
		
	if ! test -f $data_dir/mask.tif
	then
	    echo 'Identifying foreground ...'
	    $bin_dir/fly_neuron_mask $1
	    assert_succ
	    $bin_dir/rmsobj $data_dir/mask.tif -s 1000 -o $data_dir/mask.tif
	    assert_succ
	fi

	if ! test -f $data_dir/holemask.tif
	then
	    echo 'Determining holes (dark voxels surrounded by bright voxels) ...'
	    $bin_dir/fly_neuron_holemask $1
	    assert_succ
	fi
	
	if ! test -f $data_dir/seeds.pa
	then
	    echo 'Extracing seeds ...'
	    $bin_dir/fly_neuron_seed $1
	    assert_succ
	fi

	if ! test -f $data_dir/blobmask.tif
	then
	    echo 'Identifying blobs ...'
	    $bin_dir/fly_neuron_blobmask $1
	    assert_succ
	fi

	if ! test -f $data_dir/holeimg.tif
	then
	    echo 'Finding holes ...'
	    $bin_dir/fly_neuron_holeobj $1
	    assert_succ
	fi

	if ! test -f $data_dir/soma.tif
	then
	    echo 'Detecting somas ...'
	    $bin_dir/fly_neuron_soma $1
	    assert_succ
	fi

	if ! test -f $data_dir/bundle.tif
	then
	    echo 'Detecting tubes ...'
	    $bin_dir/fly_neuron_bundle $1
	    assert_succ
	fi

	#if ! test -f $data_dir/grow_bundle.tif
	#then
	#    $bin_dir/fly_neuron_grow_bundle $1
	#fi
	
	if ! test -f $data_dir/bundle_seeds.pa
	then
	    echo 'Extracting tracing seeds ...'
	    $bin_dir/fly_neuron_bundle_seed $1
	fi

	if ! test -f $data_dir/trace_mask.tif
	then
	    $bin_dir/bwinv $data_dir/blobmask.tif -o $data_dir/trace_mask.tif
	    #cp $data_dir/blobmask.tif $data_dir/trace_mask.tif
	fi
	
	#$bin_dir/trace_neuron2 ../data/$1.tif -Sseeds -D$data_dir -canvas traced.tif -mask trace_mask.tif -r -minlen 20.0 -minr 1.5

	#$bin_dir/trace_fly_neuron3 $1
	#$bin_dir/fly_neuron_label2 $1

	#$bin_dir/reconstruct_neuron -Rregion_label.tif -Tsoma.tif -Mregion_id.txt -D../data/$1 -I../data/$1.tif -o struct.txt -minlen 20.0
    fi
fi