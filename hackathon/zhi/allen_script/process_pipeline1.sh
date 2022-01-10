#!/bin/bash
vaa3d=$1
inputfoldername=$2
markerfile=$3
markerpatthfile=$4
outputswc=$5

imagename=$inputfoldername

#$vaa3d -x IVSCC_import -f import -i $inputfoldername -o $imagename
#$vaa3d -x mipZ -f mip_zslices -i $imagename -p 1:1:e -o ${imagename}_mip.raw
#$vaa3d -x multiscaleEnhancement -f adaptive_auto_2D -i ${imagename}_mip.raw -o ${imagename}_mip.raw_enhanced.raw
#$vaa3d -x multiscaleEnhancement -f soma_detection_2D -i ${imagename}_mip.raw -p $markerfile ${imagename}_mip.raw_enhanced.raw
#$vaa3d -x Region_Neuron2 -f trace_app2 -i ${imagename}_mip.raw_enhanced.raw_soma.raw -p 1 10 0 0 0 20 500
#$vaa3d -x mapping3D_swc -f mapping -i $imagename ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc
#$vaa3d -x neuron_connector  -f connect_neuron_SWC -i ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc -o ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc_connected.swc -p 60 20 1 1 1 0 false 1
#$vaa3d -x resample_swc -f resample_swc -i ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc_connected.swc -p 10
$vaa3d -x IVSCC_process_swc -f process_v2 -i ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc_connected.swc_resampled.swc $markerpatthfile $markerfile ${imagename}_mip.raw -o $outputswc
$vaa3d -x IVSCC_process_swc -f process_remove_artifacts -i $outputswc $imagename -o $outputswc
$vaa3d -x IVSCC_process_swc -f process_soma_correction -i $outputswc $markerfile $imagename -o ${outputswc}_soma_corrected.swc
