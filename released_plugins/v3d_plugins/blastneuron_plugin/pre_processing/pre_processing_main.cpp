//pre_processing_main.cpp
//main function for pre processing (blastneron)
//by Yinan Wan
//2012-06-02

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
#include <unistd.h>

bool pre_processing_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	printf("welcome to pre_processing\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_pre_processing();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


	if (paralist->size()!=1)
	{
		printf("Please specify all paramters in one text string.\n");
		printHelp_pre_processing();
		return false;
	}
	
	
	char * paras = paralist->at(0);
	int argc = 1;
	enum {kArgMax = 64};
	char *argv[kArgMax];
	
	//parsing parameters
	if (paras)
	{
		int len = strlen(paras);
		for (int i=0;i<len;i++)
		{
			if (paras[i]=='#')
				paras[i] = '-';
		}

		char* pch = strtok(paras, " ");
		while (pch && argc<kArgMax)
		{
			argv[argc++] = pch;
			pch = strtok(NULL, " ");
		}
	}
	else
		printHelp_pre_processing();


	//read arguments

	char *dfile_input = NULL;
	char *dfile_result = NULL;
	char *outfile = NULL;
	double step_size = 2;
	int skip_rotation = 1;
	
	int c;
    static char optstring[]="i:o:s:r:";
	extern char * optarg;
	extern int optind, opterr;
		
	while ((c = getopt(argc, argv, optstring))!=-1)
	{
		switch (c)
		{
			case 'h':
				printHelp_pre_processing();
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
			case 'r':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				skip_rotation = atoi(optarg);
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
	
	NeuronTree result;
	if (skip_rotation!=1)
	{
		printf("Aligning PCA axis\n");
		result = align_axis(resampled);
	}
	else
		result = resampled;
	if (export_listNeuron_2swc(result.listNeuron,qPrintable(outfileName)))
		printf("\t %s has been generated successfully.\n",qPrintable(outfileName));

	return 0;
}

void printHelp_pre_processing()
{
	printf("\nVaa3D plugin: Pre-processing step for BlastNeuron pipeline, including: \n\t1) delete the branches in a neuron which have a length smaller than 5 %% of neuron/tree diameter.\n\t2)resample the neurons along the segments with the step size (default: 1). \n\t3) rotate the neuron to align its longest dimension to x axis\n");
	printf("\t#i <neuron_filename> :   input neuron structure (.swc) name\n");
	printf("\t#o <output_filename> :   output file name.\n");
	printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
	printf("\t#s <step_size>       :   step size for resampling.\n");
	printf("\t                         if not specified, use 2\n");
	printf("\t#r <rotation_flag>   :   whether or not use PCA for rotation.\n");
	printf("\t                         if not specified, rotation is not perfored\n");
	printf("Usage: vaa3d -x pre_processing -p \"#i input.swc #o result.swc #s 2 #r 1\"\n");
}
