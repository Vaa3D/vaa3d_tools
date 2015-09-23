#!/bin/bash
cd /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/gold166/
for foldername in `ls -d checked*`
do
       	cd $foldername
       	for subfolder in $(ls -d */)
       	do
               	cd $subfolder
               	for swcfile in $(ls *.swc)
               	do
			mv $swcfile  /lustre/atlas2/nro101/proj-shared/BigNeuron/data/bigneuron_annotation_consolidated_20150715/reconstructions_for_gold166/$foldername/$subfolder$swcfile
		done
                cd ..
        done
        cd ..
done
