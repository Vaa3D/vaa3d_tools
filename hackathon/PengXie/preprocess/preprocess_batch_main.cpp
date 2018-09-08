//preprocess_batch_main.cpp
//created by Peng Xie
//2018-09-07

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "preprocess_batch_main.h"
#include "fstream"
#include "sstream"
#include "string"


bool preprocess_batch(QString swclist, QString qctable, bool skip_existing){
    std::ifstream infile(qPrintable(swclist));
    std::string line;
    printf("welcome to use preprocess_batch\n");

    if(!fexists(qctable)){
        FILE * fp=fopen((char *)qPrintable(qctable), "wt");
        fprintf(fp, "\tType\tPercentage_unconnected(%%)\tNumber_single_trees\tExist_duplicate_nodes\n");
        fclose(fp);
    }
    std::ofstream logging;
    logging.open(qPrintable(qctable), std::ios::app);

    while (std::getline(infile, line))
    {
        cout<<line<<endl;
        std::istringstream iss(line);
        std::string a;
        if (!(iss >> a)) { break; }
        QString qs_inswc = QString::fromStdString(a);
        QString inswcPrefix;
        if (qs_inswc.endsWith(".swc") || qs_inswc.endsWith(".SWC")){
            inswcPrefix = qs_inswc.left(qs_inswc.length() - 4);
        }
        else if (qs_inswc.endsWith(".eswc") || qs_inswc.endsWith(".ESWC"))
        {
            inswcPrefix = qs_inswc.left(qs_inswc.length() - 5);
        }
        else{
            printf("Error: not all input files are swc's!\n");
        }
        // 1. Run preprocess
        QString qs_outswc = inswcPrefix + QString(".processed.swc");
        if((!skip_existing) || (!fexists(qs_outswc))){
            bool preprocessed = pre_processing(qs_inswc, qs_outswc);
            if(! preprocessed){
                return 0;
            }

            // 2. Run QC
            NeuronTree nt = readSWC_file(qs_outswc);
            QString inswcName = inswcPrefix.right(inswcPrefix.length()-inswcPrefix.lastIndexOf("/")-1);
            logging << qPrintable(inswcName) << "\t" << "Allneurons\t";
            // QC.1
            double percentage_disconnected = get_percentage_disconnected(qs_outswc);
            logging << percentage_disconnected << "\t";
            // QC.2
            QList<int> components = get_components(nt);
            int ncomponents=components.toSet().size();
            logging << ncomponents << "\t";
            // QC.3
            if(check_duplicate(nt)){
                logging << 0 << endl;
            }
            else{
                logging << 1 << endl;
            }
        }
    }
    logging.close();
    return 1;
}

bool preprocess_batch_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    if(input.size() != 2)
    {
        printf("Please specify parameter set.\n");
        printHelp_preprocess_batch();
        return false;
    }
    paralist = (vector<char*>*)(input.at(1).p);


    if (paralist->size()!=1)
    {
        printf("Please specify all paramters in one text string.\n");
        printHelp_preprocess_batch();
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
        printHelp_preprocess_batch();


    //0. read arguments

    char *swclist = NULL;
    char *qctable = NULL;
    bool skip_existing = 0;

    int c;
    static char optstring[]="hi:q:s:";
    extern char * optarg;
    extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
        {
            case 'h':
                printHelp_preprocess_batch();
                return 0;
                break;
            case 'i':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                swclist = optarg;
                cout << "Input file name:\t" << swclist <<endl;
                break;
            case 'q':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                qctable = optarg;
                break;
            case 's':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                skip_existing=(atoi(optarg)==1);
            break;
            case '?':
                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                return 1;
                break;
        }
    }

    bool finished = preprocess_batch(QString(swclist), QString(qctable), skip_existing);
    return finished;
}

void printHelp_preprocess_batch()
{
    printf("\nVaa3D plugin: preprocess SWC files in a given list\n");
    printf("\t#i <swclist> :   input (a file of an swc list)\n");
    printf("\t#q <qctable> :   output (name of the QC table.\n");
    printf("\t#s <skip_existing> :   whether to skip the preprocessing step if processed files exist \n");
    printf("Usage: vaa3d -x preprocess -f preprocess_batch -p \"#i swc.list #q QC.csv #s 1\"\n");
}
