#!/bin/bash

#source #assert_succ.sh

prog_name=$(basename $0)
usage="Usage: $prog_name [dataname -D data_folder -t trace_args -r recon_args -T trace_option -L -s score]"
minscore=0.35

if test $# -lt 1
then
    echo $usage
else
    recompile=1
    force=0
    retrace=1;
    recon=1;
    trace_args="-recover 2"
    recon_args="-rb -rz"
    trace_seed_args="-cs -boost"
    detect_line=0
    ext=tif
    cleantif=0
    data_folder=../data
    bin_dir=bin
    dataname=$1; shift

    while getopts uLRCfpD:d:m:t:r:T:E:s:B:S: option
    do
      case $option in
      u)
	recompile=0;;
      B)
	bin_dir=$OPTARG;;
      D)
	data_folder=$OPTARG;;
      m)
	minlen=$OPTARG;;
      f)
	force=1;;
      t)
	trace_args="$trace_args $OPTARG";;
      T)
	retrace=$OPTARG;;
      r)
	if [ "$OPTARG" = "no_recon" ]
	then
	  recon=0
	else
	  recon_args="$recon_args $OPTARG"
	fi
	;;
      L)
	detect_line=1;;
      R)
	recon=1;;
      E)	
	ext=$OPTARG;;
      C)
	cleantif=1;;
      s)
	minscore=$OPTARG;;
      S)
        trace_seed_args="$OPTARG";;
      \?)
	echo $usage
	exit 2;;
      esac
    done

    if [ ! -d $data_folder ]; then
      echo "$data_folder does not exist."
      exit 1
    fi
    trace_args="$trace_args -min_score $minscore"
    imgfile=$data_folder/$dataname.$ext
    orgfile=$data_folder/$dataname_org.tif
    resfile=$data_folder/$dataname.res
    if ! test -f $orgfile
    then
      orgfile=$imgfile
    fi
    data_dir=$data_folder/$dataname
    #bin_dir=bin

    if ! test -f $imgfile
    then
      echo "Cannot find $imgfile"
      exit 1
    fi

    if ! test -d $data_dir
    then
      mkdir $data_dir
    fi

    if test -d $data_dir
    then
      if [ $retrace -ge 1 ]
      then
	#find $data_dir -name "*.wrl" -print | xargs rm
	#find $data_dir -name "chain*.tb" -print | xargs rm
        rm $data_dir/chain*.tb
	#find $data_dir -name "newchain*.tb" -print | xargs rm
	#rm $data_dir/soma*.bn
	if [ -f $data_dir/traced.tif ]; then
	  rm $data_dir/traced.tif
	fi
	if [ -f $data_dir/trace_mask.tif ]; then
	  rm $data_dir/trace_mask.tif
	fi
      fi

      if [ $retrace -ge 2 ]
      then
	rm $data_dir/*
      fi

      traceprog=trace_neuron3

      if [ $recompile -eq 1 ]
      then
	echo 'compiling ...'
	make PROG=imbinarize
	make PROG=bwsolid
	make PROG=rmsobj
	make PROG=extract_line
	make PROG=imexpr
	make PROG=trace_seed
	make PROG=sort_seed
	make PROG=drawmask
	make PROG=$traceprog
	make PROG=reconstruct_neuron
      fi

      if [ $retrace -ge 1 ]
      then
	outfile=$data_dir/bin.tif
	if ! test -f $outfile
	then
	  $bin_dir/imbinarize -Mlocmax -retry 3 -R$orgfile $imgfile -o $outfile
	  #assert_succ
	fi	

	if ! test -f $outfile
	then
	  $bin_dir/imbinarize -Mhist_tr -R$orgfile $imgfile -o $outfile
	  #assert_succ
	fi

	outfile=$data_dir/mask.tif
	if ! test -f $outfile
	then
	  $bin_dir/bwsolid -mf 4 $data_dir/bin.tif -o $outfile
	  #assert_succ
	  $bin_dir/rmsobj $outfile -o $outfile -s 10
	  #assert_succ
	fi

	outfile=$data_dir/line.tif
	moutfile=$data_dir/grey_line.tif
	if [ $detect_line -eq 1 ]
	then
	  if ! test -f $outfile
	  then
	    $bin_dir/extract_line $imgfile -o $moutfile
	    #assert_succ
	    $bin_dir/imbinarize -Mlocmax -retry 5 -rsobj 27 $moutfile -o $outfile
	    #assert_succ
	  fi
	fi

	if test -f $moutfile
	then
	  if ! test -f $outfile
	  then
	    $bin_dir/imbinarize -Mhist_tr $moutfile -o $outfile
	    #assert_succ
	  fi
	fi

	#	    outfile=$data_dir/locmax.tif

	outfile=$data_dir/mask2.tif
	if ! test -f $outfile
	then
	  if test -f $data_dir/line.tif
	  then
	    $bin_dir/imexpr $data_dir/mask.tif '|' $data_dir/line.tif -o $outfile
	    #assert_succ
	  else
	    cp $data_dir/mask.tif $outfile
	    #assert_succ
	  fi
	fi

	outfile=$data_dir/seeds
	if ! test -f $outfile
	then
	  $bin_dir/trace_seed $data_dir/mask2.tif -o $outfile $trace_seed_args -mrseed $data_dir/seed_mask.tif -mrdist $data_dir/seed_dist.tif
	  #assert_succ
	  #$bin_dir/drawmask $imgfile -m $data_dir/seed_mask.tif -s 3 -o $data_dir/seed_check.tif
	  ##assert_succ
	fi

	outfile=$data_dir/chain_seeds
	if ! test -f $outfile
	then
	  $bin_dir/sort_seed $imgfile -s $data_dir/seeds -o $outfile -e $data_dir/base.tif -min_score $minscore
	  #assert_succ
	  #-res $resfile
	fi

	#outfile=soma
	#echo "$bin_dir/detect_soma $imgfile  -mask trace_mask.tif -Sseeds -D $data_dir -o $outfile"
	#$bin_dir/detect_soma $imgfile  -mask trace_mask.tif -Sseeds -D $data_dir -o $outfile

	if [ -n $minlen ]; then
	    minlen=25
	fi

	#$bin_dir/$traceprog $imgfile -Schain_seeds -D$data_dir -canvas traced.tif -mask mask2.tif -r -minlen $minlen  -sf chain_seeds_score -re -base_mask base.tif

	#$bin_dir/$traceprog $imgfile -Schain_seeds -D$data_dir -canvas traced.tif -mask mask2.tif -r -minlen $minlen  -sf chain_seeds_score -base_mask base.tif

	#$bin_dir/$traceprog $imgfile -Schain_seeds -D$data_dir -canvas traced.tif -mask mask2.tif -r -sf chain_seeds_score -re -base_mask base.tif -bs 1.5

	$bin_dir/$traceprog $imgfile -D$data_dir -Schain_seeds -canvas traced.tif -mask mask2.tif -sf chain_seeds_score -base_mask base.tif $trace_args
	#assert_succ

	if [ $cleantif -eq 1 ]
	then
	  mv $data_dir/*.tif /Users/zhaot/trash
	fi

      fi

      #./reconstruct_neuron $dataname $minlen

      if [ $recon -eq 1 ]
      then
	$bin_dir/reconstruct_neuron -Rregion_label.tif -Tsoma.tif -Mregion_id.txt -D$data_dir -I$imgfile -o struct.txt -minlen $minlen $recon_args
	#assert_succ
      fi

      # -res $resfile

      #$bin_dir/reconstruct_neuron -Rregion_label.tif -Tsoma.tif -Mregion_id.txt -D../data/$1 -I../data/$1.tif -o struct.txt -minlen $minlen
    fi
  fi
