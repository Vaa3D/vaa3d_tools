#! /bin/bash
CORE_NUM="8"
tgt="target.tif"
sbj="subject.tif"
output="out"
channel="1"
log="log"
if [ "$#" -eq "1" ]
then
	output=$1
fi

if [ "$#" -eq "3" ]
then
	tgt="$1"
	sbj="$2"
	output="$3"
fi

if [ "$#" -eq "4" ]
then
	tgt="$1"
	sbj="$2"
	output="$3"
	channel="$4"
fi

# check existence
if [ ! -e "$tgt" -o ! -e "$sbj" ]
then
	echo "error : $tgt or $sbj is not exist" >&2     # output to err
	exit 1
fi

echo "scale rotate score" >$output      # empty the output
#> $log  # empty the log file

scales="50 25 12.5 6.25"      # set scales
#scales="6.25"
rotates=`jot - 0 355 5`       # set rotation from 0~350 step by 5

tasks=()
results=()
sbj_imgs=()
for scale in $scales
do
	tgt_img="`basename -s .tif $tgt`_scale_$scale.tif"
	sbj_scale="`basename -s .tif $sbj`_scale_$scale.tif"
	echo convert $tgt -resize "${scale}%" $tgt_img
	convert $tgt -resize "${scale}%" $tgt_img &
	echo convert $sbj -resize "${scale}%" $sbj_scale
	convert $sbj -resize "${scale}%" $sbj_scale &
	wait
	crop_size=`tiffsize $sbj_scale | awk '{printf("%dx%d+0+0\n",$3,$6)}'`
	for rotate in $rotates
	do
		if [ "${#tasks[@]}" -ge "$CORE_NUM" ]
		then
			taskid=${tasks[0]}
			result=${results[0]}
			sbj_img=${sbj_imgs[0]}
			tasks=(${tasks[@]:1})
			results=(${results[@]:1})
			sbj_imgs=(${sbj_imgs[@]:1})
			#echo "tasks : ${tasks[@]} , task num :  ${#tasks[@]}"
			wait $taskid
			if [ -e "$result" ]
			then
				deform=`basename -s .txt $result | awk -F_ '{print $3,$5}'`
				echo -n "$deform " >> $output
				cat $result >> $output
				rm $result
			else
				echo "$result not output" >> $log
			fi
			rm $sbj_img
		fi
		sbj_img="`basename -s .tif $sbj`_scale_${scale}_rotate_${rotate}.tif"
		#result_img="result_scale_${scale}_rotate_${rotate}.tif"
		result_txt="result_scale_${scale}_rotate_${rotate}.txt"
		echo "convert $sbj_scale -rotate $rotate -gravity Center -crop $crop_size $sbj_img && v3d_stitch_pair -t $tgt_img -C $channel -s $sbj_img -c $channel -u $result_txt -n 0"
		convert $sbj_scale -rotate $rotate -gravity Center -crop $crop_size $sbj_img && v3d_stitch_pair -t $tgt_img -C $channel -s $sbj_img -c $channel -u $result_txt -n 0 >> /dev/null &
		taskid=$!
		#echo $taskid
		tasks=("${tasks[@]}" "$taskid")
		#echo "tasks : ${tasks[@]} , task num :  ${#tasks[@]}"
		results=("${results[@]}" "$result_txt")
		sbj_imgs=("${sbj_imgs[@]}" "$sbj_img")
	done
	while [ "${#tasks[@]}" -gt 0 ]
	do
		taskid=${tasks[0]}
		result=${results[0]}
		sbj_img=${sbj_imgs[0]}
		tasks=(${tasks[@]:1})
		results=(${results[@]:1})
		sbj_imgs=(${sbj_imgs[@]:1})
		#echo "tasks : ${tasks[@]} , task num :  ${#tasks[@]}"
		wait $taskid
		if [ -e "$result" ]
		then
			deform=`basename -s .txt $result | awk -F_ '{print $3,$5}'`
			echo -n "$deform " >> $output
			cat $result >> $output
			rm $result
		else
			echo "$result not output" >> $log
		fi
		rm $sbj_img
	done

	echo rm $tgt_img
	rm $tgt_img
	echo rm $sbj_scale
	rm $sbj_scale
done

