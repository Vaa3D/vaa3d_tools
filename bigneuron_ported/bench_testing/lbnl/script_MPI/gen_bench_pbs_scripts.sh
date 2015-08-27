#!/bin/bash
#
#This is a shell program to batch reconstruct images using 21 different methods.
#

function write_psb_script {

  outputScript=$1;
  inputfolder=$2;
  exefilename=$3;
  jobnumbers=$4;
  nodenumbers=$5;	
  echo "#PBS -l walltime=1:00:00" >> $outputScript;
  echo "#PBS -l nodes=$nodenumbers" >> $outputScript;
 # echo "#PBS -l mppwidth=1920" >> $outputScript;
  echo "#PBS -q regular" >> $outputScript;

  echo "cd $inputfolder" >>  $outputScript;
  echo "aprun -n $jobnumbers ./$exefilename" >>  $outputScript;
}

				
#copy the names
jobnumbers=$1
nodenumbers=$2
jobScriptFile=$3
inputexefolder=$4
exefilename=$5



#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

write_psb_script $jobScriptFile $inputexefolder $exefilename $jobnumbers $nodenumbers

