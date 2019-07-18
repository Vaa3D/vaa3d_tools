#!/bin/bash
#

outputScript=$1;
vaa3dProgramPath=$2;
inimgfileFolder=$3;
inimgfileName=$4;
outputfileFolder=$5;
terafly4ds=$6
teraflyfull=$7

#generate the batch script configuration
if [ -f $outputScript ]; then
  rm $outputScript;
fi;

echo "#PBS -l walltime=24:00:00" >> $outputScript;
echo "# Request just one node on the host" >> $outputScript;
echo "#PBS -l nodes=1" >> $outputScript;
echo "#PBS -r n" >> $outputScript;
echo "# location for stderr/stdout log files _after_ job completion" >> $outputScript;
echo "#PBS -o ${outputScript}.out" >> $outputScript;

echo "#" >> $outputScript;
echo "#" >> $outputScript;


DISPLAY1=:$RANDOM;
echo "export DISPLAY=$DISPLAY1" >> $outputScript;
echo "Xvfb $DISPLAY1 -auth /dev/null &" >> $outputScript;
  
echo "sh $vaa3dProgramPath/start_vaa3d.sh  -x refine_swc -f initial_4ds -i '$terafly4ds' ${inimgfileFolder}/${inimgfileName} -o ${outputfileFolder}/${inimgfileName}_ini.eswc" >> $outputScript;
echo "sh $vaa3dProgramPath/start_vaa3d.sh  -x refine_swc -f refine_v2  -i '$teraflyfull' ${outputfileFolder}/${inimgfileName}_ini.eswc -o ${outputfileFolder}/${inimgfileName} -p 100" >> $outputScript;

# precisely kill the job
echo "kill %1" >> $outputScript;

#generate the actual tracing code
if [ ! -d $outputfileFolder ]; then
  mkdir $outputfileFolder
fi






