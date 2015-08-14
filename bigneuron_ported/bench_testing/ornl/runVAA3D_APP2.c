#include <stdio.h>
#include <unistd.h>
#include "mpi.h"


int main(int argc, char **argv)
{
	int *buf, i, rank, nints;

	char hostname[256];

	char command_string[1024];
	char txt_string[256];

	MPI_Init(&argc,&argv);
	int my_id, num_procs;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	sprintf(txt_string, "/lustre/atlas/proj-shared/nro101/BigNeuron/APP2_jobs/%d.txt",my_id);
	FILE* file = fopen(txt_string, "r");
	size_t len = 0;
	int read;
	char * line = NULL;// (char *) malloc (1024);
	int index  = 0;
	MPI_Barrier(MPI_COMM_WORLD);

	while ((read = getline(&line, &len, file)) != -1) {
		sprintf(command_string, "cd /lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1/;export DISPLAY=:%d;Xvfb :%d -auth /dev/null & %s\n",10+my_id,10+my_id,line);
//		printf(command_string);
			system(command_string);
	}
	fclose(file);
	MPI_Barrier(MPI_COMM_WORLD);	
	MPI_Finalize();
	printf("All Done\n");
	return 0;
}
