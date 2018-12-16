# check invalid tif file
# yy 12/20/2017

# build libtiff (e.g. tiff-4.0.9) and put "tiffinfo" in your path

# Usage:
# sh imagecheck.sh INDIR images.txt 
# grep -C 6 "not" images.txt  >> error.txt

DIR=$1
OUT=$2

for i in $DIR/*.tif
do

echo $i >> $OUT

time tiffinfo -D $i &>> $OUT

echo " " >> $OUT

done
