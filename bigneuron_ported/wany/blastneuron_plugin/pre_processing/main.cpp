//main.cpp
//main function for pre processing
//by Yinan Wan
//2012-03-17

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000
#include "basic_surf_objs.h"
#include <unistd.h>
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include <QtGlobal>
#include <vector>
using namespace std;

void printHelp();

extern char *optarg;
extern int optind, opterr;

int main(int argc, char *argv[])
{
	if  (argc <=1)
	{
		printHelp();
		return 0;
	}

	//read arguments
	char *dfile_input = NULL;
	char *dfile_result = NULL;
	double step_size = 2;

	int c;
	static char optstring[] = "hi:o:s:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring))!=-1)
	{
		switch (c)
		{
			case 'h':
				printHelp();
				return 0;
				break;
			case 'i':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -l.\n");
					return 1;
				}
				dfile_input = optarg;
				break;
			case 'o':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_result = optarg;
				break;
			case 's':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				step_size = atof(optarg);
				if (step_size<=0)
				{
					fprintf(stderr, "Illegal step size. It must>0.\n");
				}
				break;
			case '?':
				fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
				return 1;
				break;
		}
	}


	QString qs_input(dfile_input);
	NeuronTree nt = readSWC_file(qs_input);
	QString outfileName = QString(dfile_result);
	if (dfile_result==NULL)
	{
		outfileName = qs_input+"_preprocessed.swc";
	}

	printf("Pruning short branches\n");
	NeuronTree pruned;
	if (!prune_branch(nt, pruned))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return 1;
	}
	
	printf("Resampling along segments\n");
	NeuronTree resampled = resample(pruned, step_size);
	

	printf("Aligning PCA axis\n");
	NeuronTree result = align_axis(resampled);

	if (export_listNeuron_2swc(result.listNeuron,qPrintable(outfileName)))
		printf("\t %s has been generated successfully.\n",qPrintable(outfileName));

	return 0;
}

void printHelp()
{
	printf("\nPre-processing step for BlastNeuron pipeline, including: \n\t1) delete the branches in a neuron which have a length smaller than 5 %% of neuron/tree diameter.\n\t2)resample the neurons along the segments with the step size (default: 1). \n\t3) rotate the neuron to align its longest dimension to x axis\n");
	printf("\t-i <neuron_filename> :   input neuron structure (.swc) name\n");
	printf("\t-o <output_filename> :   output file name.\n");
	printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
	printf("\t-s <step_size>       :   step size for resampling.\n");
	printf("\t                         if not specified, use 2\n");
	printf("Usage: ./pre_processing -i input.swc -o result.swc\n");
}
