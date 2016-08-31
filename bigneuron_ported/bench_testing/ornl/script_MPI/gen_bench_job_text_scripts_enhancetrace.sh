#!/bin/bash
#
#This is a shell program to generate a script to trace brightfield images
#
# Last change: 2016-08-20
#

function write_trace_spec_commands {

  vaa3d=$1
  outputScript=$2
  img_name=$3
  trace_img_name=$4

  echo "chmod -R 777 $trace_img_name*;" >> $outputScript;
  echo "$vaa3d -x mapping3D_swc -f mapping -i $img_name $trace_img_name;" >> $outputScript;
  echo "chmod -R 777 $trace_img_name*;" >> $outputScript;
  echo "$vaa3d -x neuron_connector -f connect_neuron_SWC -i ${trace_img_name}_3D.swc -o ${trace_img_name}_3D.swc_connected.swc -p 60 20 1 1 1 0 false 1;" >> $outputScript
  echo "chmod -R 777 $trace_img_name*;" >> $outputScript;
  echo "$vaa3d -x resample_swc -f resample_swc -i ${trace_img_name}_3D.swc_connected.swc -p 10;" >> $outputScript
  echo "chmod -R 777 $trace_img_name*;" >> $outputScript;

}


function write_neuron_tracing_command {

  vaa3d=$1
  outputScript=$2
  img=$3
  trace=$4

  image=${img}_mip.raw_enhanced.raw_soma.raw

  #Advantra tracing
  if [ $trace == "1" ]
  then
      echo "$vaa3d -x Region_Neuron2 -f trace_advantra -i $image -p 20 .5 .7 20 60 10 10 1 10 1000;" >> $outputScript;
      write_trace_spec_commands $vaa3d $outputScript $img ${image}_region_Advantra.swc
  fi

  #APP1 tracing
  if [ $trace == "2" ]
  then
      trace_img=${image}_region_APP1.swc
      echo "$vaa3d -x Region_Neuron2 -f trace_app1 -i $image -p 0 40 1 1000;" >> $outputScript;
      write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #APP2 tracing
  if [ $trace == '3' ]
  then
    trace_img=${image}_region_APP2.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_app2 -i $image -p 1 20 0 0 0 50 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #Axis Analyzer tracing
  if [ $trace == '4' ]
  then
    trace_img=${image}_region_axis_analyzer.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_axis_analyzer -i $image -p 20 1000 998 15;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #fastmarching_spanningtree tracing
  if [ $trace == '5' ]
  then
    trace_img=${image}_region_fastmarching_spanningtree.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_bjut_fast_mst -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #meanshift MST tracing
  if [ $trace == '6' ]
  then
    trace_img=${image}_region_meanshift.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_bjut_meanshift_mst -i $image -p 20 1000" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #CW Lab tracing
  if [ $trace == '7' ]
  then
    trace_img=${image}_region_CWLab.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_cwlab -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #ENT tracing
  if [ $trace == '8' ]
  then
    trace_img=${image}_region_ENT.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_ent -i $img -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $image $trace_img
  fi

  #NTSUT Ensemble Basic tracing

  if [ $trace == '9' ]
  then
    trace_img=${image}_region_EnsembleNeuronTracerBasic.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_ent_basic -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #NTSUT Ensemble V2n tracing
  if [ $trace == '10' ]
  then
    trace_img=${image}_region_EnsembleNeuronTracerV2n.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_ent_v2n -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #NTSUT Ensemble V2s tracing
  if [ $trace == '11' ]
  then
    trace_img=${image}_region_EnsembleNeuronTracerV2s.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_ent_v2s -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #nctuTW GD tracing
  if [ $trace == '12' ]
  then
    trace_img=${image}_region_nctuTW_GD.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_gd -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #Simple MST tracing
  if [ $trace == '13' ]
  then
    trace_img=${image}_region_MST_Tracing.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_mst -i $image -p 20 1000 80;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #nctuTW tracing
  if [ $trace == '14' ]
  then
    trace_img=${image}_region_nctuTW.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_nctutw -i $image -p 20 1000 .9 NULL;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #NeuronChaser tracing
  if [ $trace == '15' ]
  then
     trace_img=${image}_region_NeuronChaser.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_neuronchaser -i $image -p 20 1000 1 20 .7 60 10 10 1 0;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #NeuTu tracing
  if [ $trace == '16' ]
  then
    trace_img=${image}_region_neutu_autotrace.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_neutu -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi

  #SmartTrace tracing
  if [ $trace == '17' ]
  then
    trace_img=${image}_region_smartTrace.swc
    echo "$vaa3d -x Region_Neuron2 -f trace_smart_trace -i $image -p 20 1000;" >> $outputScript;
    write_trace_spec_commands $vaa3d $outputScript $img $trace_img
  fi
}


if [ ! $# -ge 1 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
#else
#    if [ ! -d $1 ]; then
#        echo "Cannot find Vaa3D executable folder [$1]"
#        exit
#    fi
fi

if [ ! $# -ge 2 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
fi

if [ ! $# -ge 3 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
else
    if [ ! -d $3 ]; then
        echo "Cannot find input image folder $3"
        exit
    fi
fi

if [ ! $# -ge 4 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
else
    if [ ! -f $4 ]; then
        echo "Cannot find image soma marker file"
        exit
    fi
fi

if [ ! $# -ge 5 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
fi

if [ ! $# -ge 6 ]; then
	echo "sh gen_brightfield_jobs.sh  <Vaa3D executable path> <job script name> <input image folder path> <soma marker file path> <output folder path> <trace method>"
	exit
fi

#copy the names
vaa3dProgramPath=$1
jobScriptFile=$2
inputFolderName=$3
markerFile=$4
outputFolderPath=$5
traceMethod=$6

#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

#generate the actual tracing code
if [ ! -d $outputFolderPath ]; then
  mkdir $outputFolderPath
fi;

imagename=${inputFolderName}_3D.raw

if [ $traceMethod == 'preprocessing' ]
then
    echo "$vaa3dProgramPath -x IVSCC_import -f import -i $inputFolderName -o $imagename;" >> $jobScriptFile;
    echo "$vaa3dProgramPath -x mipZ -f mip_zslices -i $imagename -p 1:1:e -o ${imagename}_mip.raw;" >> $jobScriptFile;
    echo "$vaa3dProgramPath -x multiscaleEnhancement -f adaptive_auto_2D -i ${imagename}_mip.raw -o ${imagename}_mip.raw_enhanced.raw;" >> $jobScriptFile;
    echo "$vaa3dProgramPath -x multiscaleEnhancement -f soma_detection_2D -i ${imagename}_mip.raw -p $markerFile ${imagename}_mip.raw_enhanced.raw;" >> $jobScriptFile;
else
    write_neuron_tracing_command $vaa3dProgramPath $jobScriptFile $imagename $traceMethod 
    echo "mv ${imagename}*.swc $outputFolderPath;" >> $jobScriptFile;
fi

chmod 777 $jobScriptFile
