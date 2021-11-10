#!/bin/bash
cd ~/data/20151030_rhea_reconstructions_for_allen300_silver_set/checked3_fruitfly_taiwan_flycircuit/uint8_dTdc2MARCM-F000249_seg001.lsm_c_3.tif/

for swcfile in `ls *.swc`
do	
	mkdir /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/movie/$swcfile
	/Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/gen_zmovie_ano_scripts.sh  /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/checked3_fruitfly_taiwan_flycircuit/uint8_dTdc2MARCM-F000249_seg001.lsm_c_3.tif/$swcfile /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/my0.apftxt /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/movie/$swcfile /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/linker_files/${swcfile}.ano
	
done
