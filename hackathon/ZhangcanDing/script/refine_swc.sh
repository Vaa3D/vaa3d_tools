#!/bin/bash

inputswcfolder=$1
outputfolder=$2
teraimgpath=$3 #the folder which has all resolutions


declare -a myarray
unset myarray
for j in ` ls $teraimgpath/RES* -d -v -r `
do 
	echo $j
	myarray+=($j)
done

teraflyfull=${myarray[0]}
terafly4ds=${myarray[2]}


cd $inputswcfolder
for inputswc in `ls *swc`
do
	vaa3d  -x refine_swc -f initial_4ds -i $terafly4ds  $inputswc -o $outputfolder/${inputswc}_ini.eswc
	vaa3d  -x refine_swc -f refine_v2  -i $teraflyfull  $outputfolder/${inputswc}_ini.eswc -o $outputfolder/${inputswc}_refined.eswc  -p 100
done

