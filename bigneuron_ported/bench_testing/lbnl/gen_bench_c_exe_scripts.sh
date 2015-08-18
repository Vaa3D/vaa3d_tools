#!/bin/bash
#
#
#

function write_c_exe_script {

	outputScript=$1;
	inputfolder=$2;
	Vaa3Dfolder=$3;

	echo "#include <stdio.h>" >> $outputScript;
	echo "#include <unistd.h>" >> $outputScript;
	echo "#include \"mpi.h\"" >> $outputScript;
        echo "" >> $outputScript;

	echo "int main(int argc, char **argv)" >> $outputScript;
	echo "{" >> $outputScript;
	echo "	char command_string[1024];" >> $outputScript;
	echo "	char txt_string[256];" >> $outputScript;
	echo "	MPI_Init(&argc,&argv);" >> $outputScript;
	echo "	int my_id, num_procs;" >> $outputScript;
	echo "	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);" >> $outputScript;
	echo "	sprintf(txt_string,\"$inputfolder/%d.txt\",my_id);" >> $outputScript;
	echo "	FILE* file = fopen(txt_string, \"r\");" >> $outputScript;
	echo "	size_t len = 0;" >> $outputScript;
	echo "	int read;" >> $outputScript;
	echo "	char * line = NULL;" >> $outputScript;
	echo "	int index  = 0;" >> $outputScript;
	echo "	MPI_Barrier(MPI_COMM_WORLD);" >> $outputScript;
	echo "	while ((read = getline(&line, &len, file)) != -1) {" >> $outputScript;

	echo "		sprintf(command_string, \"cd $Vaa3Dfolder;export DISPLAY=:%d;Xvfb :%d -auth /dev/null & %s\n\",10+my_id,10+my_id,line);" >> $outputScript;
	echo "		system(command_string);" >> $outputScript;
	echo "	}" >> $outputScript;
	echo "	fclose(file);" >> $outputScript;
	echo "	MPI_Barrier(MPI_COMM_WORLD);" >> $outputScript;
	echo "	MPI_Finalize();" >> $outputScript;
	echo "	printf(\"All Done\n\");" >> $outputScript;
	echo "	return 0;" >> $outputScript;
	echo "}" >> $outputScript;
}


#copy the names
cFile=$1
inputexefolder=$2
Vaa3Dfolder=$3



#generate the batch script configuration
if [ -f $cFile ]; then
rm $cFile;
fi;

write_c_exe_script $cFile  $inputexefolder $Vaa3Dfolder

