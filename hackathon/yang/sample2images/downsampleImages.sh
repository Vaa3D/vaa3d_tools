# downsample images
# 6/15/2018 by Yang Yu
#

# This script will downsample 2 neighbor images "test_07510_mon.tif" and "test_07511_mon.tif"
# to "test_07510_07511_mon.tif"

# 1.
# ln -s mouseID17545/green/montage level0

# 2.
# time sh downsampleImages.sh level0 level1 /path/vaa3d

# similarly, generate level2 by `time sh downsampleImages.sh level1 level2 /path/vaa3d`

export DISPLAY=:$RANDOM
Xvfb $DISPLAY -auth /dev/null &


inputDir=$1
outputDir=$2
vaa3d=$3

n=0;

for i in $inputDir/*;
do

n=$((n+1))

if [ `echo "$n % 2" | bc` -eq 0 ]
then

input2=$i

n1=${input1#*test_*}
n1=${n1%*_mon.tif}

n2=${input2#*test_*}

name="test_"${n1}"_"${n2}

output=${outputDir}"/"${name}

echo "$((n/2)) downsample $input1 $input2 to $output"
time $vaa3d -x sample2images -f sample2images -i $input1 $input2 -o $output

else

input1=$i

fi

done

