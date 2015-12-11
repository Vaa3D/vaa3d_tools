#!/bin/bash
#
#This is a shell program to batch reconstruct images using 9 different methods from LCMboost.
#

function write_neuron_tracing_command {

  outputScript=$1;
  METHOD=$2;
  vaa3dProgramPath=$3;
  inimgfileTracing=$4;
  finalfileFolder=$5;

  smooth_inimgfileTracing=${inimgfileTracing}.g.v3draw;

#LCM_boost

  if [ $METHOD == "2" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_2 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost2.swc ;chmod -R 777 $inimgfileTracing*_LCMboost2.swc;mv  $inimgfileTracing*_LCMboost2.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "3" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_3 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost3.swc ;chmod -R 777 $inimgfileTracing*_LCMboost3.swc;mv  $inimgfileTracing*_LCMboost3.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "4" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_4 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost4.swc ;chmod -R 777 $inimgfileTracing*_LCMboost4.swc;mv  $inimgfileTracing*_LCMboost4.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "5" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_5 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost5.swc ;chmod -R 777 $inimgfileTracing*_LCMboost5.swc;mv  $inimgfileTracing*_LCMboost5.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "6" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_6 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost6.swc ;chmod -R 777 $inimgfileTracing*_LCMboost6.swc;mv  $inimgfileTracing*_LCMboost6.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "7" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_7 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost7.swc ;chmod -R 777 $inimgfileTracing*_LCMboost7.swc;mv  $inimgfileTracing*_LCMboost7.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "8" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_8 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost8.swc ;chmod -R 777 $inimgfileTracing*_LCMboost8.swc;mv  $inimgfileTracing*_LCMboost8.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "9" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_9 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost9.swc ;chmod -R 777 $inimgfileTracing*_LCMboost9.swc;mv  $inimgfileTracing*_LCMboost9.swc $finalfileFolder" >> $outputScript;
  fi;

  if [ $METHOD == "10" -o $METHOD == "-1" ]; then
    echo "./start_vaa3d.sh -x LCM_boost -f LCM_boost_10 -i $inimgfileTracing -o ${inimgfileTracing}_LCMboost10.swc ;chmod -R 777 $inimgfileTracing*_LCMboost10.swc;mv  $inimgfileTracing*_LCMboost10.swc $finalfileFolder" >> $outputScript;
  fi;

}

if [ ! $# -ge 1 ]; then
	echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
	exit
fi

#if [ $# -ge 2 ]; then
#	if [ ! -f $2 ]; then
#		echo "Can not find file [$2]"
#		exit
#	fi
#else
#	echo "sh gen_bench_job_scripts.sh  <tracing algorithm number> <input image path> <output folder path> <Vaa3D executable folder path>"
#	exit
#fi

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

