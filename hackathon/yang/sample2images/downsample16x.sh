
# downsample z slices every 4 images
# red channel:
# time sh downsample16x.sh mouse17781/red mouse17781/level2r
# sh downsample2images2Red.sh level2r level3r ../bin/vaa3d 1
#
# green channel:
# time sh downsample16x.sh mouse17781/green mouse17781/level2g
# sh downsample2images2.sh level2g level3g ../bin/vaa3d 1

inputdir=$1
outputdir=$2


n=0
count=0


for i in ${inputdir}/*.tif
do


n=$((n+1))

if (( $n % 4 == 0))
then


count=$((count+1))


if [[ $i =~ "red" ]]
then

echo $i

filename=${i#*red/*}

time python imsample8bit16x.py $i ${outputdir}"/"${filename}

else

filename=${i#*green/*}

time python imsample16bit16x.py $i ${outputdir}"/"${filename}

fi


fi



done

