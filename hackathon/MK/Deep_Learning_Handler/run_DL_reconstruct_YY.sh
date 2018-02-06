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

$VAA3Dexe -x Deep_Learning_Handler -f imageCrop4CaffePredict -i $imagepath $tempPath -p $AlexDeploy $AlexTrained $AlexMean
$VAA3Dexe -x neurontree -f estradius -i $imagepath $tempPath/SWCs/combined.swc -p 60
$VAA3Dexe -x neurontree -f neutrace -i $imagepath $tempPath/SWCs/combined_wr.swc -o $tempPath/SWCs/lines.swc -p 1 7 5 125
mv $tempPath/SWCs/lines_traced.swc $tempPath/final_connected.swc
