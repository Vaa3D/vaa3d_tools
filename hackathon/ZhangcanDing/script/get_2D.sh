#!/bin/bash

inputfolder=$1
orisaveoutputfolder=$2
mode=$3
prunedoutputfolder=$4

function arr_index()
{
 local arr_temp
 local _arr_cnt=0
 local _arr_array=`echo "$1"`
 for arr_temp in ${_arr_array[*]}
 do
    if test "$2" = "$arr_temp";then
    echo $_arr_cnt  #echo can also return your result of this function
    return 
    fi
    _arr_cnt=$(($_arr_cnt + 1 ))
 done
    echo '-1'
}



#*****find all the swc f
for file1 in $(ls $1/*swc |grep -v "purned" |awk -F "/" '{print $NF}' |head -500) #grep -v "purned" : means excluding swc. whose name has "pruned"
do
  echo $file1
  file_temp=${file1%.*} #delete all the string in the right of ".","%" means right and "#" means left,e.g.{file1#*.}
  arr1=(${arr1[*]} $file_temp)
  arr_swc=(${arr_swc[*]} $file1)
done

#*****find all the swc filenames******
for file in $(ls $1/*nrrd |awk -F "/" '{print $NF}' |head -500)
do
  echo $file
  file_temp2=${file%.*}
  arr2=(${arr2[*]} $file_temp2)
  arr_nrrd=(${arr_nrrd[*]} $file)
done
#*****find all the swc filenames******
for file2 in $(ls $1/*purned* |awk -F "/" '{print $NF}' |head -500)
do
  echo $file2
  file_temp3=${file2%%.*} #delete all the string in the right of last "."(count from right)
  arr3=(${arr3[*]} $file_temp3)
  arr_pruned_swc=(${arr_pruned_swc[*]} $file2)
done


arr1_temp=`echo ${arr1[*]}` #ori swc
arr2_temp=`echo ${arr2[*]}` #nrrd files
arr3_temp=`echo ${arr3[*]}` #pruned swc

for chosename in ${arr1[*]}
do
  echo $chosename
  index1=$(arr_index "$arr1_temp" "$chosename") 
  index2=$(arr_index "$arr2_temp" "$chosename") 
  index3=$(arr_index "$arr3_temp" "$chosename") 
  if [ "$index2" != "-1" ] ;then vaa3d -x ML_get_sample -f get_2D_sample -i $1/${arr_swc[$index1]} -p $1/${arr_nrrd[$index2]} -o $2;fi
  if [ "$index3" != "-1" ] ;then if [ "$index2" != "-1" ];then vaa3d -x ML_get_sample -f get_2D_sample -i $1/${arr_pruned_swc[$index3]} -p $1/${arr_nrrd[$index2]} -o $4;fi;fi
  echo $index1
  echo ${arr_swc[$index1]}
  echo ${arr_pruned_swc[$index3]}
done





