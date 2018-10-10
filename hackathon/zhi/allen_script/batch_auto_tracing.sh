#!/bin/bash

inputimagefolder=$1
inputmarkerfolder=$2
outputswcfolder=$3
exefilename=$4

currentpath=${inputimagefolder%/*}
cd $inputmarkerfolder
for filename in `ls *.marker`
do
	ID=${filename:0:3}
	if [ ! -d $outputswcfolder/${ID} ] && [ ! -d $outputswcfolder/${ID}_ds2 ]; then
		
		timeout 5m $exefilename -x ultratracer -f trace_APP2_GD -i $inputimagefolder -p $inputmarkerfolder/$filename 512 1 1 AUTO
		mv $currentpath/nc_APP2_GD.swc $currentpath/${filename}.swc
		tmpname=${filename%.*}
		tmpname=${tmpname:4}
		#echo $tmpname	
		$exefilename -x ultratracer -f generate_final_result -i $currentpath/tmp_APP2/scanData.txt
		cp $currentpath/tmp_APP2/scanData.txtwfusion.swc $currentpath/${filename}_fused.swc
		mv $currentpath/tmp_APP2 $currentpath/${filename}
		mkdir $outputswcfolder/$ID
		$exefilename -x IVSCC_scaling -f scale -i $currentpath/${filename}_fused.swc -o $outputswcfolder/${ID}/${ID}.swc -p 2 2 2 2
		$exefilename -x marker2others -f marker2apo -i $inputmarkerfolder/$filename -o $outputswcfolder/${ID}/${ID}.apo -p 2
		$exefilename -x Linker_File_Generator -f linker -i $outputswcfolder/${ID} -o $outputswcfolder/${ID}/${ID}.ano -p 7

		mkdir $outputswcfolder/${ID}_ds2
		cp $currentpath/${filename}_fused.swc $outputswcfolder/${ID}_ds2/${ID}.swc
		$exefilename -x marker2others -f marker2apo -i $inputmarkerfolder/$filename -o $outputswcfolder/${ID}_ds2/${ID}.apo -p 1
		$exefilename -x Linker_File_Generator -f linker -i $outputswcfolder/${ID}_ds2 -o $outputswcfolder/${ID}_ds2/${ID}.ano -p 7
	fi
	
done

