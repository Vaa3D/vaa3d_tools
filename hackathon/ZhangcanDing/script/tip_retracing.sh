#!/bin/bash

inputimagefolder=$1
inputanofolder=$2
outputswcfolder=$3
exefilename=$4

currentpath=${inputimagefolder%/*}
cd  $inputanofolder
for i in `ls -d *_ds2`
do 
	ID=${i:0:3}
	mkdir ${i}/${ID}_markers
	$exefilename -x apo_to_marker -f apo_to_individual_markers -i $inputanofolder/${i}/${ID}.ano.apo -o  $inputanofolder/${i}/${ID}_markers -p 1 #extract individual markers from apo
	cp $inputanofolder/${i}/${ID}.ano.eswc $currentpath/nc_APP2_GD.swc #copy the original reconstruction result as the reference swc
	cd $inputanofolder/${i}/${ID}_markers
	
	for filename in `ls *.marker`
	do 
		markerid=${filename:0:3}
		$exefilename -x ultratracer -f trace_APP2_GD -i $inputimagefolder -p $inputanofolder/${i}/${ID}_markers/$filename 128 0 1 AUTO	#run ultratracer on every marker			
		tmpname=${filename%.*}
		tmpname=${tmpname:4}
		$exefilename -x ultratracer -f generate_final_result -i $currentpath/tmp_APP2/scanData.txt
		cp $currentpath/tmp_APP2/scanData.txtwfusion.swc $currentpath/${filename}_fused.swc
		mv $currentpath/tmp_APP2 $currentpath/${filename}
		
		mkdir $outputswcfolder/${i}
		mkdir $outputswcfolder/${i}/${ID}
		mkdir $outputswcfolder/${i}/individual_swc
		mkdir $outputswcfolder/${i}/${ID}/$markerid
		 				
		$exefilename -x IVSCC_scaling -f scale -i $currentpath/${filename}_fused.swc -o $outputswcfolder/${i}/${ID}/${markerid}/${markerid}.swc -p 2 2 2 2 #scale the result to higher resolution 
		$exefilename -x marker2others -f marker2apo -i $inputmarkerfolder/$filename -o $outputswcfolder/${i}/${ID}/${markerid}/${markerid}.apo -p 2 
		$exefilename -x Linker_File_Generator -f linker -i $outputswcfolder/${i}/${ID}/${markerid} -o $outputswcfolder/${i}/${ID}/${markerid}/${markerid}.ano -p 7

		mkdir $outputswcfolder/${i}/${ID}/${markerid}_ds2 #this is the result for resolution of *2 down sample 
		cp $currentpath/${filename}_fused.swc $outputswcfolder/${i}/${ID}/${markerid}_ds2/${markerid}.swc
		$exefilename -x marker2others -f marker2apo -i $inputanofolder/${i}/${ID}_markers/$filename -o $outputswcfolder/${i}/${ID}/${markerid}_ds2/${markerid}.apo -p 1
		$exefilename -x Linker_File_Generator -f linker -i $outputswcfolder/${i}/${ID}/${markerid}_ds2 -o $outputswcfolder/${i}/${ID}/${markerid}/${markerid}.ano -p 7
		
		cp $outputswcfolder/${i}/${ID}/${markerid}_ds2/${markerid}.swc $outputswcfolder/${i}/individual_swc/${markerid}.swc #copy every single result to a folder 
		
		cp -R $currentpath/${filename} $outputswcfolder/${i}/${ID}/${markerid}_ds2/ #copy the results of every block to output folder for debug
		
		cd $inputanofolder
	done
		cp $inputanofolder/${i}/${ID}.ano.eswc $inputanofolder/${i}/${ID}.ano.swc
		$exefilename -x sort_neuron_swc -f sort_swc -i $inputanofolder/${i}/${ID}.ano.swc -o $inputanofolder/${i}/${ID}_sorted.ano.swc -p #sort the original initial reconstruction for fusion
		cp $inputanofolder/${i}/${ID}_sorted.ano.swc $outputswcfolder/${i}/individual_swc/000.swc #copy the original result to the individual swc folder
		$exefilename -x ultratracer -f fusion -i $outputswcfolder/${i}/individual_swc/ -o $outputswcfolder/${i}/${ID}_final.swc #fuse to the final result 

done
