//main.cpp
//main function for pruning short branch
//by Yinan Wan
//2012-03-17

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000
#include "basic_surf_objs.h"
#include <unistd.h>
#include "prune_short_branch.h"
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

	int c;
	static char optstring[] = "hi:o:t:";
	double thres = 0.05;
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
					fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
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
			case 't':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				thres = atof(optarg);
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
		outfileName = qs_input+"_pruned.swc";
	}

	NeuronTree prune_result;
	if (!prune_branch(nt, prune_result, thres))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return 1;
	}
	if (export_listNeuron_2swc(prune_result.listNeuron,qPrintable(outfileName)))
		printf("\t %s has been generated successfully.\n",qPrintable(outfileName));

	return 0;
}

void printHelp()
{
	printf("\nDelete the branches in a neuron which have a length smaller than a threshold of neuron/tree diameter.\n");
	printf("\t-i <neuron_filename> :   input neuron structure (.swc) name\n");
	printf("\t-o <output_filename> :   output file name.\n");
	printf("\t                         if not specified, it is \"inputName_pruned.swc\"\n");
	printf("\t-t <threshold> :         the thereshold to define short branch, any branch below thres * diameter will be pruned\n");
	printf("\t                         if not specified default is 0.05\n");
	printf("Usage: ./prune_short_branch -i input.swc -o result.swc -t 0.2\n\n");
}
