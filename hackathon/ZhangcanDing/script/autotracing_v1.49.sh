#!/bin/bash

#please build hackthon/zhi/apo_to_marker and hackthon/ZhangcanDing/change_type to enable full function

vaa3dProgramPath=$1;
inimgfileFolder=$2;
markerfilefolder=$3;
outputfilefolder=$4;
brainID=$5;


cd $markerfilefolder
for markerfileName in `ls *.marker`
do
	ID=${brainID}_00${markerfileName:0:3}
	if [ ! -d $outputfilefolder/${ID} ]; then
		timeout 5m sh $vaa3dProgramPath/start_vaa3d.sh -x ultratracer -f trace_APP2_GD  -i  $inimgfileFolder -p $markerfilefolder/$markerfileName 512 1 1 AUTO
		sh $vaa3dProgramPath/start_vaa3d.sh -x ultratracer -f generate_final_result  -i $markerfilefolder/${markerfileName}_tmp_APP2/scanData.txt
		mkdir $outputfilefolder/${ID}
		sh $vaa3dProgramPath/start_vaa3d.sh -x IVSCC_scaling -f scale  -i $markerfilefolder/${markerfileName}_tmp_APP2/scanData.txtwfusion.swc -o $outputfilefolder/${ID}/${ID}.swc -p 2 2 2 2
		sh $vaa3dProgramPath/start_vaa3d.sh -x change_type -f swc -i $outputfilefolder/${ID}/${ID}.swc -o $outputfilefolder/${ID}/${ID}.swc -p 3
		sh $vaa3dProgramPath/start_vaa3d.sh -x marker2others -f marker2apo -i $markerfilefolder/$markerfileName -o $outputfilefolder/${ID}/${ID}.apo -p 2
		sh $vaa3dProgramPath/start_vaa3d.sh -x apo_to_marker -f add_custom_name -i $outputfilefolder/${ID}/${ID}.apo -o $outputfilefolder/${ID}/${ID}.apo -p soma
		sh $vaa3dProgramPath/start_vaa3d.sh -x Linker_File_Generator -f linker -i $outputfilefolder/${ID} -o $outputfilefolder/${ID}/${ID}.ano -p 7
	fi
done
