//add soma to swc
//by Peng Xie
//2018-07-27

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "add_soma.h"
#include "basic_surf_objs.h"

#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include <io.h>
#endif

bool add_soma(const V3DPluginArgList & input, V3DPluginArgList & output)
{
        printf("welcome to add_soma\n");


        vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
        vector<char*>* outlist = NULL;
        vector<char*>* paralist = NULL;

        if(input.size() != 2)
        {
                printf("Please specify parameter set.\n");
                printHelp_add_soma();
                return false;
        }
        paralist = (vector<char*>*)(input.at(1).p);


        if (paralist->size()!=1)
        {
                printf("Please specify all paramters in one text string.\n");
                printHelp_add_soma();
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
                printHelp_add_soma();


    //1. read arguments

        char *dfile_input = NULL;
        char *dfile_result = NULL;
        char *outfile = NULL;
        double multiply_by = 2;
        double step_size = 2;
    double thres = 2;
    double prune_size = -1; //default case
        int skip_rotation = 1;

        int c;
    static char optstring[]="i:o:m:l:s:t:r:";
        extern char * optarg;
        extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
                {
                        case 'h':
                                printHelp_add_soma();
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

             case 'm':
                multiply_by = atof(optarg);
                if (multiply_by<=0)
                {
                    fprintf(stderr, "Illegal multiply size. Size must be greater than 0.\n");
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

    QList <ImageMarker> marker_list = readMarker_file(dfile_input);
    QList <ImageMarker>::Iterator it;
    ImageMarker S;
    for(it = marker_list.begin(); it != marker_list.end(); it++){
        S = *it;
        S.x = S.x * multiply_by;
        S.y = S.y * multiply_by;
        S.z = S.z * multiply_by;

        fprintf(stderr, "%f %f %f \n", S.x, S.y, S.z);
    }
    return true;

}

void printHelp_add_soma()
{
    printf("\nVaa3D plugin: modifiled pre-processing step for BlastNeuron pipeline, including: \n");
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

    printf("Usage: vaa3d -x preprocess -f preprocess -p \"#i input.swc #o result.swc #l 3 #s 2 #t 2 #r 0\"\n");
}
