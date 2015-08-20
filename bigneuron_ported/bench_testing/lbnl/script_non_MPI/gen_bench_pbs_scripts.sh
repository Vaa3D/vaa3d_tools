#!/bin/bash
#
#This is a shell program to batch reconstruct images using 21 different methods.
#

function write_psb_script {

  outputScript=$1;
  inputfolder=$2;
  exefilename=$3;
  jobnumbers=$4;	
  echo "#PBS -l walltime=1:00:00" >> $outputScript;
  echo "#PBS -l nodes=400" >> $outputScript;
  echo "#PBS -q regular" >> $outputScript;

  echo "cd $inputfolder" >>  $outputScript;
  echo "aprun -n $jobnumbers  ./$exefilename" >>  $outputScript;
}

				
#copy the names
jobnumbers=$1
jobScriptFile=$2
inputexefolder=$3
exefilename=$4



#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

write_psb_script $jobScriptFile $inputexefolder $exefilename $jobnumbers

