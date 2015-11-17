#!/bin/bash
cd /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/movie
for folder in `ls -d *`
do
	/Users/zhiz/work/v3d/v3d_external/bin/vaa3d64.app/Contents/MacOS/vaa3d64 -x Movie_Converter -f convert_frames_to_movie -i $folder 
        mv ${folder}/movie.avi /Users/zhiz/data/20151030_rhea_reconstructions_for_allen300_silver_set/movie_final/${folder}.avi
done
