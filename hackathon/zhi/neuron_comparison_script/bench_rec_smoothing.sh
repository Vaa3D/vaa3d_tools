#!/bin/bash
#
#THis is a shell progrem to batch reconstruct images using five different methods, including APP1, APP2, MOST, neuTube, snake, simpletracing, and TReMap.
#APP1,APP2,MOST, and simpletracing are built together with Vaa3D
#
#To build neuTube plugin, 1. go to /vaa3d_tools/released_plugins/v3d_plugins/neurontracing_neutube/src_neutube/, and run 
#	                                     sh build.sh <qmake_path> <qmake_spec_path>
#	              more detailed information can be found in /local1/work/vaa3d_tools/released_plugins/v3d_plugins/neurontracing_neutube/how_to_build
#		   2. go to /local1/work/vaa3d_tools/hackathon/zhi/neuTube_zhi/, and run qmake & make
#
#To build FarSight Snake Tracing plugin, please check the README file in /vaa3d_tools/hackathon/zhi/snake_tracing/README
#


#handle the input and output

if [ $# -ge 5 ]; then
  METHOD=$5;
else
  METHOD="-1";
fi

if [ $# -ge 4 ]; then
        if [ ! -d $4 ]; then
                echo "Can not find dll folder [$4]"
                exit
        fi
else
        echo "./batch_reconstruction.test <Vaa3D executable file path> <input image path> <output folder path> <dll folder for general dependency libraries>"
        exit
fi

export LD_LIBRARY_PATH=$4


if [ $# -ge 3 ]; then
        if [ ! -d $3 ]; then
                mkdir $3
        fi
else
        echo "./batch_reconstruction.test <Vaa3D executable file path> <input image path> <output folder path> <dll folder for general dependency libraries>"
        exit
fi

if [ $# -ge 2 ]; then
        if [ ! -f $2 ]; then
                echo "Can not find file [$2]"
                exit
        fi
else
        echo "./batch_reconstruction.test <Vaa3D executable file path> <input image path> <output folder path> <dll folder for general dependency libraries>"
        exit
fi

if [ $# -ge 1 ]; then
        if [ ! -f $1 ]; then
                echo "Can not find file [$1]"
                exit
        fi
else
        echo "./batch_reconstruction.test <Vaa3D executable file path> <input image path> <output folder path> <dll folder for general dependency libraries>"
        exit
fi

#generate the smoothed image

SUFF=zip;
infile=$2;
unzip $infile;
originalImgFile=${infile%.$SUFF};
inimgfileTracing="$originalImgFile"_g.v3draw;

$1 -x gaussian -f gf -i $originalImgFile -o $inimgfileTracing -p 7 7 2 1 2;
$1 -x datatypeconvert -f dtc -i $inimgfileTracing -o $inimgfileTracing -p 1;


#different tracing algorithms

DO_TRACING="YES";
if [ $DO_TRACING == "YES" ]; then

  if [ $METHOD == "1" -o $METHOD == "-1" ]; then
    $1 -x vn2 -f app1 -i $inimgfileTracing -p NULL 0 40 0    
    mv  $inimgfileTracing*_app1.swc $3
  fi;

  if [ $METHOD == "2" -o $METHOD == "-1" ]; then
    $1 -x vn2 -f app2 -i $inimgfileTracing -p NULL 0 10 0 1 1 0 5   
    mv  $inimgfileTracing*_app2.swc $3
    rm  $inimgfileTracing*_ini.swc
  fi;

  if [ $METHOD == "3" -o $METHOD == "-1" ]; then
    $1 -x MOST -f MOST_trace -i $inimgfileTracing -p 1 40   
    mv  $inimgfileTracing*_MOST.swc $3
  fi;

  if [ $METHOD == "4" -o $METHOD == "-1" ]; then
    $1 -x neuTube -f neutube_trace -i $inimgfileTracing -p 1 1   
    mv  $inimgfileTracing*_neutube.swc $3
  fi;

  if [ $METHOD == "5" -o $METHOD == "-1" ]; then
    $1 -x snake -f snake_trace -i $inimgfileTracing -p 1 
    mv  $inimgfileTracing*_snake.swc $3
  fi;

  if [ $METHOD == "6" -o $METHOD == "-1" ]; then
    $1 -x SimpleTracing -f tracing -i $inimgfileTracing -o ${inimgfileTracing}_simple.swc -p 1 
    mv  $inimgfileTracing*_simple.swc $3
  fi;

  if [ $METHOD == "7" -o $METHOD == "-1" ]; then
    $1 -x TReMap -f trace_mip -i $inimgfileTracing -p 0 1 10 0 1 0 5
    mv  $inimgfileTracing*_TreMap.swc $3
  fi;

fi;

# generate the final linker file so make it easy to load many SWC files together into Vaa3D

DO_LINKERFILE="YES";
if [ $DO_LINKERFILE == "YES" ]; then

  $1 -x linker_file_generator -f linker -i $3 -o linkerfile.ano -p 1;

fi;

#clean up for tmp files

DO_CLEANUP="YES";
if [ $DO_CLEANUP == "YES" ]; then

  rm $originalImgFile;
  rm $inimgfileTracing;

fi;

