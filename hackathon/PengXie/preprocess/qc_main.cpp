#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "qc_main.h"

double get_percentage_disconnected(QString qs_input){

    printf("welcome to use get_percentage_disconnected\n");
    NeuronTree nt = readSWC_file(qs_input);
    if(nt.listNeuron.size()==0){return 100;}
    int lost_ct = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).type == 7){
            lost_ct ++;
        }
    }
    double percentage_disconnected = lost_ct * 100.0 / nt.listNeuron.size();
    return percentage_disconnected;
}

bool qc_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{
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


    //0. read arguments

    char *dfile_input = NULL;
    char *dfile_result = NULL;

    int c;
    static char optstring[]="i:o:";
    extern char * optarg;
    extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
        {
            case 'i':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                dfile_input = optarg;
                cout << "Input file name:\t" << dfile_input <<endl;
                break;
            case 'o':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                dfile_result = optarg;
                break;
            case '?':
                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                return 1;
                break;
        }
    }

    QString qs_input = QString(dfile_input);
    QString outfileLabel = QString(dfile_result);

    NeuronTree nt = readSWC_file(qs_input);
    // QC report
    FILE * fp=0;
    fp = fopen((char *)qPrintable(outfileLabel+QString(".QC.txt")), "wt");
    // QC.1
    double percentage_disconnected = get_percentage_disconnected(qs_input);
    fprintf(fp, "Percentage_disconnected(%%)\t%f\n", percentage_disconnected);
    // QC.2
    QList<int> components = get_components(nt);
    int ncomponents=components.toSet().size();
    fprintf(fp, "Number_single_trees\t%d\n", ncomponents);
//    fprintf(fp, "Number_single_trees\t%d\n", 0);
    // QC.3
    if(check_duplicate(nt)){
        fprintf(fp, "Exist_duplicate_nodes\t%d\n", 0);
    }
    else{
        fprintf(fp, "Exist_duplicate_nodes\t%d\n", 1);
    }
//    fprintf(fp, "Exist_duplicate_nodes\t%d\n", 1);
    fclose(fp);

    return 1;
}
