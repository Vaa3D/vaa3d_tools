#!/bin/bash
#
#This is a shell program to batch reconstruct images using 4 different methods using NeuronAssember.
#


function write_neuron_tracing_command {

  outputScript=$1;
  METHOD=$2;
  vaa3dProgramPath=$3;
  inimgfileTracing=$4;
  finalfileFolder=$5;

  smooth_inimgfileTracing=${inimgfileTracing}.g.v3draw;

#MOST

  if [ $METHOD == "3" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_MostVesselTracer -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_MostVesselTracer.swc;mv  $inimgfileTracing*_NeuronAssembler_MostVesselTracer.swc $finalfileFolder" >> $outputScript;
  fi;


#NEUTUBE

  if [ $METHOD == "4" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_NeuTube -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_NeuTube.swc;mv  $inimgfileTracing*_NeuronAssembler_NeuTube.swc $finalfileFolder" >> $outputScript;
  fi;

#FARSIGHT Snake

  if [ $METHOD == "5" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_Snake -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_Snake.swc;mv  $inimgfileTracing*_NeuronAssembler_Snake.swc $finalfileFolder" >> $outputScript;
  fi;


#TreMap

  if [ $METHOD == "9" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_tReMap -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_tReMap.swc;mv  $inimgfileTracing*_NeuronAssembler_tReMap.swc $finalfileFolder" >> $outputScript;
  fi;


#NeuroGPSTree

  if [ $METHOD == "11" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_neuroGPSTree -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_neuroGPSTree.swc;mv  $inimgfileTracing*_NeuronAssembler_neuroGPSTree.swc $finalfileFolder" >> $outputScript;
  fi;

#neutu_autotrace

  if [ $METHOD == "22" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x NeuronAssembler_Neutu_autotrace -f trace_raw -i $inimgfileTracing -p NULL 512 1;chmod -R 777 $inimgfileTracing*_NeuronAssembler_Neutu_autotrace.swc;mv  $inimgfileTracing*_neutu_autotrace.swc $finalfileFolder" >> $outputScript;
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

