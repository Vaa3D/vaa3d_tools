#!/bin/bash

inputswcfolder=$1
saveimagefolder=$2
inputimage1folder=$3
inputimage2folder=$4

echo ~+
dir=$(ls -l $1 |awk '/^d/ {print $NF}')
cd $2
for i in $dir
do 
   mkdir $i.tocheck
done

#*****find all the swc filenames******
cd $1
for file in `ls $1`
do
  arr=(${arr[*]} $file)
done
#*********run vaa3d plugin***************
for i in ${arr[*]}
do
   vaa3d -x get_sub_terafly -f get_block -i $1/$i/*swc -p $3/$i/54600* $4/$i/54600* -o $2/$i.tocheck
done 
#*****testing with two random different images******

length=${#arr[*]} 
randomnum1=$(($RANDOM%$length+1))
randomnum2=$(($RANDOM%$length+1))
if(($randomnum1 != $randomnum2))
then
    cd $2/$arr[randomnum1].tocheck
    ls>ztemp.txt;rdimgename1=sed -n 2p ztemp.txt ;rm -rf ztemp.txt
    cd $2/$arr[randomnum2].tocheck
    ls>ztemp.txt;rdimgename2=sed -n 2p ztemp.txt ;rm -rf ztemp.txt
    diff $($2/$arr[randomnum1].tocheck/rdimgename1)  $($2/$arr[randomnum2].tocheck/rdimgename2)
    if [$0 != 0]
    then
        echo "Both files are different,testing result passes"
    else  
        echo "Both files are same,there are may some mistakes in your codes.Please double check"
    fi
fi
#*****testing with all two different images******
for neuronfile in `ls $2`
do
  for registernum in $(ls -lSr $2/$neurnonfile/*tif |awk -F "." '{print $2}')
  do
    arr1=(${arr1[*]} $registernum)
  done
      for i in ${arr1[9]} 
      do
        diff $($2/$neuronfile/${arr1[i]}) $($2/$neuronfile/Y.${arr[i]}*)
        if [$0 != 0]
        then
          echo ${neuronfile}" files are different,testing results do not pass"
          echo "${neuronfile}">>$2/check_result.txt
        else  
          echo ${neuronfile}" files are same,there are may some mistakes in your codes.Please double check"
        fi   
      done
done
