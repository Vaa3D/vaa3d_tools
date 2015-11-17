#!/bin/bash

function write_movie_ano_command {

  outputScript=$1;
  swcfile=$2;
  archorfile=$3;
  outfolder=$4;

  echo "SWCFILE=$swcfile" >> $outputScript;
  echo "ANCHORFILE=$archorfile" >> $outputScript;
  echo "OUTFOLDER=$outfolder" >> $outputScript;
}


#copy the names
swcfile=$1;
archorfile=$2;
outfolder=$3;
jobScriptFile=$4


#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

write_movie_ano_command $jobScriptFile $swcfile $archorfile $outfolder

