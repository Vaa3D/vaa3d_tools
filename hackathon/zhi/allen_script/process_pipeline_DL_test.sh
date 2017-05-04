#!/bin/bash
vaa3d=$1
imagename=$2
markerfile=$3
outputswc=$4

$vaa3d -x mipZ -f mip_zslices -i $imagename -p 1:1:e -o ${imagename}_mip.raw
$vaa3d -x multiscaleEnhancement -f adaptive_auto_2D -i ${imagename}_mip.raw -o ${imagename}_mip.raw_enhanced.raw
$vaa3d -x multiscaleEnhancement -f soma_detection_2D -i ${imagename}_mip.raw -p $markerfile ${imagename}_mip.raw_enhanced.raw
$vaa3d -x Region_Neuron2 -f trace_app2 -i ${imagename}_mip.raw_enhanced.raw_soma.raw -p 1 10 0 0 0 20 500
$vaa3d -x mapping3D_swc -f mapping -i $imagename ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc
$vaa3d -x neuron_connector  -f connect_neuron_SWC -i ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc -o ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc_connected.swc -p 60 20 1 1 1 0 false 1
$vaa3d -x resample_swc -f resample_swc -i ${imagename}_mip.raw_enhanced.raw_soma.raw_region_APP2.swc_3D.swc_connected.swc -p 10 -o  ${imagename}_auto.swc
$vaa3d -x prediction_caffe -f 3D_Axon_detection_raw -i $imagename  -p '/local4/Data/IVSCC_test/comparison/Caffe_testing_3rd/cross_validation_122/deploy.prototxt' '/local4/Data/IVSCC_test/comparison/Caffe_testing_3rd/cross_validation_122/caffenet_train_1_122_iter_450000.caffemodel' '/local4/Data/IVSCC_test/comparison/Caffe_testing_3rd/cross_validation_122/imagenet_mean_1_122.binaryproto' 10 512
$vaa3d -x IVSCC_process_swc -f merge_two_swc -i  ${imagename}_auto.swc  ${imagename}_axon_3D.swc
$vaa3d -x prediction_caffe -f Feature_Extraction -i $imagename -p ${imagename}_axon_3D.swc_processed.swc /local1/work/caffe/examples/siamese/mnist_siamese.prototxt /local1/work/caffe/examples/siamese/full_siamese_iter_450000.caffemodel
$vaa3d -x IVSCC_process_swc -f connect_two_swc -i ${imagename}_auto.swc_processed.swc ${imagename}_axon_3D.swc_processed.swc_connected_60_z.swc -o $outputswc
$vaa3d -x IVSCC_process_swc -f extract_high_swc -i $outputswc



