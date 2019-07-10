#!/bin/bash

inputswcfolder=$1
outputfolder=$2
vaa3dProgramPath=$3


cd $inputswcfolder
 
for filename in `ls *.eswc`
do
	#sh $vaa3dProgramPath/start_vaa3d.sh  -x refine_swc -f initial_4ds -i '/home/penglab/PBserver/TeraconvertedBrain/mouse17302_teraconvert/RES(13650x8603x2461)'  ${inputswcfolder}/${filename} -o $outputfolder/${filename}_ini.eswc
	mkdir $outputfolder/${filename}/
	sh $vaa3dProgramPath/start_vaa3d.sh  -x refine_swc -f refine_v2  -i '/home/penglab/PBserver/TeraconvertedBrain/mouse17302_teraconvert/RES(54600x34412x9847)'  $inputswcfolder/${filename}_ini.eswc -o $outputfolder/${filename}/${filename}_refined_v2.eswc  -p 100
	

done
