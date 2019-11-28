#! /bin/bash

manualfolder=$1
allmarkerfolder='/home/penglab/PBserver/tmp/m_markers'
outcomparefolder=$2
vaa3dProgramPath='/home/penglab/v3d_external/bin'
teraimgpath='/home/penglab/PBserver/TeraconvertedBrain'
outputfilefolder='/home/penglab/PBserver/tmp/m_auto_all'

cd $manualfolder

for i in `ls *swc`
do 
	echo $i
	brainid=${i:0:5}
	apoid=${i:8:3}
	ID=${i:0:11}
		
	#cd $teraimgpath/mouse${brainid}_teraconvert 

#find the second highest resolution img path
	declare -a myarray
	unset myarray
	for j in ` ls $teraimgpath/mouse${brainid}_teraconvert/RES* -d -v -r `
	do 
		myarray+=($j)
	done

	inimgfileFolder=${myarray[1]}   	
	markerfilefolder=$allmarkerfolder/$brainid
	markerfileName=` echo $markerfilefolder/${apoid}*.marker `
	
	if [ ! -d $outputfilefolder/${ID} ]; then
		timeout 5m sh $vaa3dProgramPath/start_vaa3d.sh -x ultratracer -f trace_APP2_GD  -i  $inimgfileFolder -p $markerfileName 512 1 1 AUTO
		sh $vaa3dProgramPath/start_vaa3d.sh -x ultratracer -f generate_final_result  -i ${markerfileName}_tmp_APP2/scanData.txt
		mkdir $outputfilefolder/${ID}
		sh $vaa3dProgramPath/start_vaa3d.sh -x IVSCC_scaling -f scale  -i ${markerfileName}_tmp_APP2/scanData.txtwfusion.swc -o $outputfilefolder/${ID}/${ID}.swc -p 2 2 2 2
		sh $vaa3dProgramPath/start_vaa3d.sh -x change_type -f swc -i $outputfilefolder/${ID}/${ID}.swc -o $outputfilefolder/${ID}/${ID}.swc -p 3
		sh $vaa3dProgramPath/start_vaa3d.sh -x marker2others -f marker2apo -i $markerfileName -o $outputfilefolder/${ID}/${ID}.apo -p 2
		sh $vaa3dProgramPath/start_vaa3d.sh -x apo_to_marker -f add_custom_name -i $outputfilefolder/${ID}/${ID}.apo -o $outputfilefolder/${ID}/${ID}.apo -p soma
		sh $vaa3dProgramPath/start_vaa3d.sh -x Linker_File_Generator -f linker -i $outputfilefolder/${ID} -o $outputfilefolder/${ID}/${ID}.ano -p 7
	fi
	
#cut img
	if (( `ls $outcomparefolder -l |grep ${i:0:11} |wc -l`==0)); then
	vaa3d -x test000 -f cut_block -i ${markerfileName}_tmp_APP2/scanData.txt $manualfolder/$i ${markerfileName}_tmp_APP2 -o  $outcomparefolder -p 2 
	fi
	
#get 2d image
	for  k in ` ls $outcomparefolder/${ID}*auto.swc `
	do
		pre=${k%_auto*}
		img=${pre}.v3draw   
		vaa3d -x ML_get_sample -f get_2D_block -i $k -p $img 2 -o $outcomparefolder
	done
 
	for  k in ` ls $outcomparefolder/${ID}*manual.eswc `
	do
		pre=${k%_manual*}
		img=${pre}.v3draw
		vaa3d -x ML_get_sample -f get_2D_block -i $k -p $img 1 -o $outcomparefolder
	done
done	
