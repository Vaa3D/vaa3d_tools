#!/bin/bash
cd /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/img_anisosmooth/
for filename in `ls -d *`
do
        cd $filename
        echo $filename
        for file in `ls *.swc`
        do
                mv $file /global/project/projectdirs/m2043/BigNeuron/Data/taiwan16k/reconstructions_for_img_anisosmooth/$filename/
        done
        cd ..
  #     /lustre/atlas/proj-shared/nro101/BigNeuron/taiwan_16k_valid_batch_run/anisosmooth/results/$filename
done

