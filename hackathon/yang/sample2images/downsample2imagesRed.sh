# downsample images
# 6/15/2018 by Yang Yu
#

# This script will downsample 2 neighbor images "test_07510_mon.tif" and "test_07511_mon.tif"
# to "test_07510_07511_mon.tif"

# 1.
# ln -s mouseID17545/green/montage level0

# 2.
# time sh downsampleImages.sh level0 level1 /path/vaa3d

# similarly, generate level2 by `time sh downsample2Images.sh level1 level2 /path/vaa3d`

# ln -s /home/yangy/fmost-data/fMOST_raw_data/mouseID_321244-17545/red/montage/ ./level0
# nohup time sh downsample2Images.sh level0 level1 ../../../../v3d_external/bin/vaa3d 1 &

#export DISPLAY=:$RANDOM
#Xvfb $DISPLAY -auth /dev/null &


inputDir=$1
outputDir=$2
vaa3d=$3
SAMPLEMETHOD=$4

if [ ${SAMPLEMETHOD} == 1 ]
then
SAMPLEMETHOD=1
else
SAMPLEMETHOD=0
fi

n=0;

for i in $inputDir/*;
do

n=$((n+1))

if [ `echo "$n % 2" | bc` -eq 0 ]
then

input2=$i

n1=${input1#*_*}
n1=${n1%*_*.tif}

n2=${input2#*_*}

name=${n1}"_"${n2}

output=${outputDir}"/"${name}


if [ -f "$output" ];
then
   
echo "File $output exist."

else
   
echo "$((n/2)) downsample $input1 $input2 to $output with method $SAMPLEMETHOD"   
time $vaa3d -x sample2images -f sample2images -i $input1 $input2 -o $output -p $SAMPLEMETHOD

fi


else

input1=$i

fi

done

