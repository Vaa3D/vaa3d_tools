#!/bin/bash

inputfolder=$1
saveoutputfolder=$2
codepart=$3

##############################################################################fuction for matching string in different arrays##########################################################################################
function arr_index()
{
 local arr_temp
 local _arr_cnt=0
 local _arr_array=`echo "$1"`
 for arr_temp in ${_arr_array[*]}
 do
    if test "$2" = "$arr_temp";then
    echo $_arr_cnt  #echo can also return your result of function
    return 
    fi
    _arr_cnt=$(($_arr_cnt + 1 ))
 done
    echo '-1'
}
##############################################################################get pruned swc files##########################################################################################
case $3 in
pruned)
mode=$4 #2 or 3
use_thre_or_not=$5 #0 or 1
thres=$6


#*****find all the swc filenames******
for file1 in $(ls $1/*swc |awk -F "/" '{print $NF}' |head -1000)
do
  echo $file1
  file_temp=${file1%.*} #delete all the string in the right of ".","%" means right and "#" means left,e.g.{file1#*.}
  arr1=(${arr1[*]} $file_temp)
  arr_swc=(${arr_swc[*]} $file1)
done
echo ${arr1[1]}
#*****find all the swc filenames******
for file in $(ls $1/*nrrd |awk -F "/" '{print $NF}' |head -1000)
do
  echo $file
  file_temp2=${file%.*}
  arr2=(${arr2[*]} $file_temp2)
  arr_nrrd=(${arr_nrrd[*]} $file)
done
echo ${arr2[1]}
#*********run vaa3d plugin***************
arr1_temp=`echo ${arr1[*]}` #ori swc
arr2_temp=`echo ${arr2[*]}` #nrrd files

for chosename in ${arr1[*]}
do
  echo $chosename
  index1=$(arr_index "$arr1_temp" "$chosename") 
  index2=$(arr_index "$arr2_temp" "$chosename") 
  if [ "$index2" != "-1" ] ;then cp $1/${arr_nrrd[index2]} $2 ;fi #then cp $1/${arr_swc[index1]} $2
  if [ "$index2" != "-1" ] ;then vaa3d -x ML_get_sample -f prune_tip_APP1 -i $1/${arr_swc[$index1]} $1/${arr_nrrd[$index2]} -p $4 $5 $6 -o $2;fi
  echo $index1
  echo ${arr_swc[$index1]}
  echo ${arr_pruned_swc[$index3]}
done
;;
##############################################################################get 2D image block from orignal image(RES)##########################################################################################
get2dsample)

imageinput=$4
#*****find all the swc filenames******
for file1 in $(ls $1/*eswc |head -10 )
do
  arr1=(${arr1[*]} $file1)
done
echo ${arr1[1]}
#*********run vaa3d plugin***************
arr_temp=`echo ${arr1[*]}`
for i in ${arr_temp[*]}
do
   echo $i 
   vaa3d -x ML_get_sample -f get_ML_sample -i $4 $i -o $2   #Change to decimal (10)
done 
;;
##############################################################################get 2D image block from block image(.nrrd or .tif)##########################################################################################
get2dimage)

#*****find all the swc f
#for file1 in $(ls $1/*swc |grep -v "purned" |awk -F "/" '{print $NF}' |head -500) #grep -v "purned" : means excluding swc. whose name has "pruned"
#do
#  echo $file1
#  file_temp=${file1%.*} #delete all the string in the right of ".","%" means right and "#" means left,e.g.{file1#*.}
#  arr1=(${arr1[*]} $file_temp)
#  arr_swc=(${arr_swc[*]} $file1)
#done
#*****find all the swc filenames******
for file in $(ls $1/*nrrd |awk -F "/" '{print $NF}') # |head -500)
do
  #echo $file
  file_temp2=${file%.*}
  arr2=(${arr2[*]} $file_temp2)
  arr_nrrd=(${arr_nrrd[*]} $file)
done
#*****find all the swc filenames******
for file2 in $(ls $1/*purned* |awk -F "/" '{print $NF}') # |head -500)
do
  #echo $file2
  file_temp3=${file2%%.*} #delete all the string in the right of last "."(count from right)
  arr3=(${arr3[*]} $file_temp3)
  arr_pruned_swc=(${arr_pruned_swc[*]} $file2)
done

#arr1_temp=`echo ${arr1[*]}` #ori swc
arr2_temp=`echo ${arr2[*]}` #nrrd files
arr3_temp=`echo ${arr3[*]}` #pruned swc

for chosename in ${arr3[*]}  #swc name
do
  echo $chosename
  #index1=$(arr_index "$arr1_temp" "$chosename") 
  index2=$(arr_index "$arr2_temp" "$chosename") 
  index3=$(arr_index "$arr3_temp" "$chosename") 
  #if [ "$index2" != "-1" ] ;then vaa3d -x ML_get_sample -f get_2D_sample -i $1/${arr_swc[$index1]} -p $1/${arr_nrrd[$index2]} -o $2;fi
  if [ "$index3" != "-1" ] ;then if [ "$index2" != "-1" ];then vaa3d -x ML_get_sample -f get_2D_sample -i $1/${arr_pruned_swc[$index3]} -p $1/${arr_nrrd[$index2]} -o $2;fi;fi
  #echo $index1
  echo ${arr_swc[$index1]}
  echo ${arr_pruned_swc[$index3]}
done
;;
##############################################################################count the total average numer of result##########################################################################################
datapro)
cd $1
if [ -e "total_result.txt" ] ;then rm total_result.txt;fi
touch total_result.txt
for txt1 in $(ls $1/*txt |awk -F "/" '{print $NF}' |head -1)  
do
  sed -n 1p $txt1 >> total_result.txt
done

for txt in $(ls $1/*txt |grep -v "result.txt" |awk -F "/" '{print $NF}')  
do
  arr=(${arr[*]} $txt)
  sed -n 2p $txt >> total_result.txt
  echo -e "\n" >> total_result.txt
done
sed -i '/^$/d' total_result.txt
cat total_result.txt |awk '{sum2+=$2;sum3+=$3;sum4+=$4;sum5+=$5;sum6+=$6;count++}END{print sum2/count,sum3/count,sum4/count,sum5/count,sum6/count}' >> total_result.txt
;;
esac































