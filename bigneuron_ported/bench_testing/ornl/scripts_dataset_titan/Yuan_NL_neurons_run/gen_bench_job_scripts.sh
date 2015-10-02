#!/bin/bash
#
#This is a shell program to batch reconstruct images using 21 different methods.
#

function write_vaa3d_job_config {

  outputScript=$1;
  jobpath=$2;
  vaa3dProgramPath=$3;

  echo "## Check which project you may use" >> $outputScript;
#  echo "#PBS -q dque" >> $outputScript;
  echo "#PBS -A NRO101" >> $outputScript;
  echo "# Allow up to 10min of walltime.  Default is 12 hours" >> $outputScript;
  echo "#PBS -l walltime=01:00:00" >> $outputScript;
  echo "# Request just one node on the host" >> $outputScript;
  echo "#PBS -l nodes=1" >> $outputScript;
 # echo "# Give your job a descriptive name. This is visible in qstat and other job reports.  Also serves as the default basename for log files" >> $outputScript;
 # echo "#PBS -N job_${tracingMethod}" >> $outputScript;
  echo "# Should torque automatically re-run the job on error?" >> $outputScript;
  echo "#PBS -r n" >> $outputScript;
#  echo "# Merge STDOUT into STDERR" >> $outputScript;
#  echo "#PBS -j oe" >> $outputScript;
  echo "# location for stderr/stdout log files _after_ job completion" >> $outputScript;
  echo "#PBS -o ${outputScript}.out" >> $outputScript;

  echo "#" >> $outputScript;
  echo "#" >> $outputScript;

#  echo "# send email on job error" >> $outputScript;
#  echo "#PBS -m a" >> $outputScript;

  DISPLAY1=:$RANDOM;
  echo "export DISPLAY=$DISPLAY1" >> $outputScript;
  echo "Xvfb $DISPLAY1 -auth /dev/null &" >> $outputScript;

  echo "export LD_LIBRARY_PATH=$vaa3dProgramPath" >> $outputScript;
  echo "cd $vaa3dProgramPath" >> $outputScript;
  echo "sh $jobpath" >> $outputScript;

}


#copy the names
jobpath=$1
vaa3dProgramPath=$2
jobScriptFile=$3

#generate the batch script configuration
if [ -f $jobScriptFile ]; then
  rm $jobScriptFile;
fi;

write_vaa3d_job_config $jobScriptFile $jobpath $vaa3dProgramPath

# precisely kill the job

echo "kill %1" >> $jobScriptFile;


