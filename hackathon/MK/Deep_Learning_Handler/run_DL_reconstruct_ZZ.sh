#!/bin/bash
#
#
#

VAA3Dexe=$1
imagepath=$2
tempPath=$3
AlexDeploy=$4
AlexTrained=$5
AlexMean=$6
prototxtfile=$7
modelfile=$8

$VAA3Dexe -x Deep_Learning_Handler -f imageCrop4CaffePredict -i $imagepath $tempPath -p $AlexDeploy $AlexTrained $AlexMean
$VAA3Dexe -x prediction_caffe -f Feature_Extraction -i ${imagepath} -p $tempPath/SWCs/combined.swc $prototxtfile $modelfile
mv $tempPath/SWCs/combined.swc_connected_60_z.swc $tempPath/final_connected.swc
