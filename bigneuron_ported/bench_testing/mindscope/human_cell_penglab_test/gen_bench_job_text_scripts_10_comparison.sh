#!/bin/bash
#
#This is a shell program to batch reconstruct images using 10 different methods for the comparison.
#

function write_neuron_tracing_command {

  outputScript=$1;
  METHOD=$2;
  vaa3dProgramPath=$3;
  inimgfileTracing=$4;
  finalfileFolder=$5;

  smooth_inimgfileTracing=${inimgfileTracing}.g.v3draw;

#APP1

  if [ $METHOD == "1" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x Region_Neuron2 -f trace_app1 -i $inimgfileTracing -p 1 40 1;chmod -R 777 $inimgfileTracing*_region_APP1.swc;mv  $inimgfileTracing*_region_APP1.swc $finalfileFolder" >> $outputScript;
  fi;

#APP2

  if [ $METHOD == "2" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x Region_Neuron2 -f trace_app2 -i $inimgfileTracing -p 1 10 1 0 0 5;chmod -R 777 $inimgfileTracing*_region_APP2.swc;mv  $inimgfileTracing*_region_APP2.swc $finalfileFolder" >> $outputScript;
  fi;

#MOST

  if [ $METHOD == "3" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x MOST -f MOST_trace -i $inimgfileTracing -p 1 40;chmod -R 777 $inimgfileTracing*_MOST.swc;mv  $inimgfileTracing*_MOST.swc $finalfileFolder" >> $outputScript;
  fi;

#NEUTUBE

  if [ $METHOD == "4" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x neuTube -f neutube_trace -i $inimgfileTracing -p 1 1;chmod -R 777 $inimgfileTracing*_neutube.swc;mv  $inimgfileTracing*_neutube.swc $finalfileFolder" >> $outputScript;
  fi;

#FARSIGHT Snake

  if [ $METHOD == "5" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x snake -f snake_trace -i $inimgfileTracing -p 1;chmod -R 777 $inimgfileTracing*_snake.swc;mv  $inimgfileTracing*_snake.swc $finalfileFolder" >> $outputScript;
  fi;

#TreMap

  if [ $METHOD == "9" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x TReMap -f trace_mip -i $inimgfileTracing -p 0 1 10 0 1 0 5;chmod -R 777 $inimgfileTracing*_TreMap.swc;mv  $inimgfileTracing*_TreMap.swc $finalfileFolder" >> $outputScript;
  fi;

#NeuroGPSTree

  if [ $METHOD == "11" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuroGPSTree -f tracing_func -i $inimgfileTracing -p 1 1 1 10;chmod -R 777 $inimgfileTracing*_NeuroGPSTree.swc;mv  $inimgfileTracing*_NeuroGPSTree.swc $finalfileFolder" >> $outputScript;
  fi;

#NeuronChaser

  if [ $METHOD == "20" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x Region_neuronchaser -f  trace_neuronchaser -i $inimgfileTracing;chmod -R 777 $inimgfileTracing*_region_neuronchaser.swc;mv  $inimgfileTracing*_region_neuronchaser.swc $finalfileFolder" >> $outputScript;
  fi;

#neutu_autotrace

  if [ $METHOD == "22" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x neutu_autotrace -f tracing -i $inimgfileTracing ;chmod -R 777 $inimgfileTracing*_neutu_autotrace.swc;mv  $inimgfileTracing*_neutu_autotrace.swc $finalfileFolder" >> $outputScript;
  fi;

#Advantra

  if [ $METHOD == "23" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x Region_advantra -f trace_advantra -i $inimgfileTracing;chmod -R 777 $inimgfileTracing*_region_advantra.swc;mv  $inimgfileTracing*_region_advantra.swc $finalfileFolder" >> $outputScript;
  fi;
}

if [ ! $# -ge 1 ]; then
	echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
	exit
fi

if [ $# -ge 2 ]; then
	if [ ! -f $2 ]; then
		echo "Can not find file [$2]"
		exit
	fi
else
	echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
	exit
fi

if [ $# -ge 3 ]; then
	if [ ! -d $3 ]; then
		mkdir $3
	fi
else
	echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
	exit
fi

if [ $# -ge 4 ]; then
        if [ ! -d $4 ]; then
                echo "Can not find Vaa3D executable folder [$4]"
                exit
        fi
else
        echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
        exit
fi


#copy the names
tracingMethod=$1
inputImgFile=$2
finalfileFolder=$3
vaa3dProgramPath=$4
jobScriptFile=$5


#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

#generate the actual tracing code
if [ ! -d $finalfileFolder ]; then
  mkdir $finalfileFolder
fi

write_neuron_tracing_command $jobScriptFile $tracingMethod $vaa3dProgramPath $inputImgFile $finalfileFolder

