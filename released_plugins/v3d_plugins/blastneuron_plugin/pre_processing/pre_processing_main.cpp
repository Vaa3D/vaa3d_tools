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
#include "sort_eswc.h"
#include <unistd.h>
#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#endif

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
    double thres = 2;
    double prune_size = -1; //default case
	int skip_rotation = 1;
	
	int c;
    static char optstring[]="i:o:l:s:t:r:";
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

             case 'l':
                 if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -l.\n");
                    return 1;
                }
                prune_size = atof(optarg);
                if (prune_size<-1)
                {
                    fprintf(stderr, "Illegal prune size. Special case: 0 -- no prunning; -1 -- default prunning size.\n");
                }
                  break;
            case 's':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				step_size = atof(optarg);
                if (step_size<0)
                {
                    fprintf(stderr, "Illegal step_size.\n");
                }
                break;
            case 't':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
                    return 1;
                }
                thres = atof(optarg);
                if (thres<-1)
                {
                    fprintf(stderr, "Illegal thres.\n");
                }
                break;
           case 'r':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                skip_rotation = atoi(optarg);
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

    if (!prune_branch(nt, pruned,prune_size))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return 1;
	}

    NeuronTree resampled;
    if (step_size>0){
        printf("Resampling along segments\n");
        resampled = resample(pruned, step_size);
    }else{
        printf("Skip Resampling\n");
        resampled=pruned;
    }
    NeuronTree sorted;
    if (thres>0){
        printf("Sort \n");
        sorted = sort(resampled,VOID, thres);
    }else{
        printf("Skip sorting\n");
        sorted=resampled;

    }

	NeuronTree result;
	if (skip_rotation!=1)
	{
		printf("Aligning PCA axis\n");
        result = align_axis(sorted);
	}
    else{
        printf("Skip PCA alignment\n");
        result = sorted;
    }

    if (export_listNeuron_2swc(result.listNeuron,qPrintable(outfileName)))
		printf("\t %s has been generated successfully.\n",qPrintable(outfileName));

	return 1;
}

void printHelp_pre_processing()
{
    printf("\nVaa3D plugin: Pre-processing step for BlastNeuron pipeline, including: \n");
    printf("\t1) delete the branches in a neuron which have a length smaller the prune size ( by default the prune size is 5 %% of neuron/tree diameter.\n");
    printf("\t2) resample the neurons along the segments with the step size (default: 1). \n");
    printf("\t3) sort the neurons along the segments with the step size (deyyplt: 1). \n");
    printf("\t4) rotate the neuron to align its longest dimension to x axis\n");
	printf("\t#i <neuron_filename> :   input neuron structure (.swc) name\n");
	printf("\t#o <output_filename> :   output file name.\n");
	printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
    printf("\t#l <prune_size> :  prune short branches that has length smaller than prune_size.\n");
    printf("\t                         if not specified, it is \"inputName_preprocessed.swc\"\n");
	printf("\t#s <step_size>       :   step size for resampling.\n");
    printf("\t                         use 0 to skip, if not specified, use 2.\n");
    printf("\t#t <thres>       :    gap threshold for connecting during the sorting procedure.\n");
    printf("\t                         use 0 to skip, if not specified, use 2.\n");
    printf("\t#r <skip_rotation_flag = 1>   :   whether to skip PCA alignment.\n");
    printf("\t                         if not specified, rotation is not performed\n");

    printf("Usage: vaa3d -x blastneuron -f pre_processing -p \"#i input.swc #o result.swc #l 3 #s 2 #t 2 #r 0\"\n");
}
