#!/bin/bash

function write_movie_converter_command {

  outputScript=$1;
  movieframesfile=$2;
  vaa3dexe=$3;
  outfolder=$4;

  echo "#!/bin/bash" >> $outputScript;
  echo "cd $movieframesfile" >> $outputScript;
  echo "for folder in \`ls -d *\`" >> $outputScript;
  echo "do" >> $outputScript; 
  echo "	$vaa3dexe -x Movie_Converter -f convert_frames_to_movie -i \$folder " >> $outputScript;
  echo "        mv \${folder}/movie.avi $outfolder/\${folder}.avi" >> $outputScript;
  echo "done" >> $outputScript;

}


#copy the names
movieframesfile=$1;
vaa3dexe=$2;
outfolder=$3
jobScriptFile=$4


#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

write_movie_converter_command $jobScriptFile $movieframesfile $vaa3dexe $outfolder

