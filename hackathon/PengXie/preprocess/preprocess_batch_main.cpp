//preprocess_batch_main.cpp
//created by Peng Xie
//2018-09-07

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 100000

#include "preprocess_batch_main.h"
#include "fstream"
#include "sstream"
#include "string"

bool preprocess_batch_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //choose a directory that contain swc files
    QString qs_dir_swc;
    qs_dir_swc=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the directory that contains files to be processed")));
    qs_dir_swc=qs_dir_swc+"/";
    return preprocess_files_in_dir(qs_dir_swc);
}

bool preprocess_files_in_dir(QString qs_dir_swc)
{
    qDebug()<<"Welcome to use the \"batch preprocessing\" plugin";
    // Step 1: Find swc files under a folder;
    QDir dir(qs_dir_swc);
    QStringList qsl_filelist, qsl_filters;
    qsl_filters << "*.swc" << "*.eswc";

    foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
    {
        qsl_filelist+=file;
    }

    if(qsl_filelist.size()==0)
    {
        v3d_msg("Cannot find swc files in the given directory!\nTry another diretory");
        return 0;
    }

    // Step 2: Specify output folder
    const QString output_dir("Processed");
    if(!dir.mkdir(output_dir)){
        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(output_dir).arg(output_dir));
    }

    // Step 3: Perform pre-processing through a loop
    for(int i=0; i<qsl_filelist.size(); i++){
        // Add prefix
        QString output_swc=qsl_filelist.at(i);
        output_swc.prepend(output_dir+"/");
        // Add suffix
        int suffix_len = 5;
        if(qsl_filelist.at(i).endsWith(".swc") || qsl_filelist.at(i).endsWith(".SWC")){suffix_len = 4;}
        output_swc = output_swc.left(output_swc.size()-suffix_len) + ".processed.eswc";
        // Add absolute dir
        qDebug()<<qs_dir_swc+qsl_filelist.at(i)<<qs_dir_swc+output_swc;
        pre_processing(qs_dir_swc+qsl_filelist.at(i), qs_dir_swc+output_swc);
    }
    return 1;
}

//bool preprocess_batch_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
//{

//    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
//    vector<char*>* outlist = NULL;
//    vector<char*>* paralist = NULL;

//    if(input.size() != 2)
//    {
//        printf("Please specify parameter set.\n");
//        printHelp_preprocess_batch();
//        return false;
//    }
//    paralist = (vector<char*>*)(input.at(1).p);


//    if (paralist->size()!=1)
//    {
//        printf("Please specify all paramters in one text string.\n");
//        printHelp_preprocess_batch();
//        return false;
//    }


//    char * paras = paralist->at(0);
//    int argc = 1;
//    enum {kArgMax = 64};
//    char *argv[kArgMax];
//    //parsing parameters
//    if (paras)
//    {
//        int len = strlen(paras);
//        for (int i=0;i<len;i++)
//        {
//            if (paras[i]=='#')
//                paras[i] = '-';
//        }

//        char* pch = strtok(paras, " ");
//        while (pch && argc<kArgMax)
//        {
//            argv[argc++] = pch;
//            pch = strtok(NULL, " ");
//        }
//    }
//    else
//        printHelp_preprocess_batch();


//    //0. read arguments

//    char *swclist = NULL;
//    char *somalist = NULL;
//    char *swcdir = NULL;
//    char *qctable = NULL;
//    bool skip_existing = 0;

//    int c;
//    static char optstring[]="hi:o:m:q:s:";
//    extern char * optarg;
//    extern int optind, opterr;
//    optind = 1;
//    while ((c = getopt(argc, argv, optstring))!=-1)
//    {
//        switch (c)
//        {
//            case 'h':
//                printHelp_preprocess_batch();
//                return 0;
//                break;
//            case 'i':
//                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
//                {
//                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
//                    return 1;
//                }
//                swclist = optarg;
//                cout << "Input file name:\t" << swclist <<endl;
//                break;
//            case 'o':
//                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
//                {
//                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
//                    return 1;
//                }
//                swcdir = optarg;
//                cout << "Output swc directory:\t" << swcdir <<endl;
//            break;
//            case 'm':
//                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
//                {
//                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
//                    return 1;
//                }
//                somalist = optarg;
//                cout << "Somalist:\t" << somalist <<endl;
//        break;
//            case 'q':
//                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
//                {
//                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
//                    return 1;
//                }
//                qctable = optarg;
//                break;
//            case 's':
//                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
//                {
//                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
//                    return 1;
//                }
//                skip_existing=(atoi(optarg)==1);
//            break;
//            case '?':
//                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
//                return 1;
//                break;
//        }
//    }

//    bool finished = preprocess_batch(QString(swclist), QString(swcdir), QString(somalist), QString(qctable), skip_existing);
//    return finished;
//}

void printHelp_preprocess_batch()
{
    printf("\nVaa3D plugin: preprocess SWC files in a given list\n");
    printf("\t#i <swclist> :   input (a file of an swc list)\n");
    printf("\t#o <dir_output_swc> :   directory of the output swc files \n");
    printf("\t#m <somalist.ano> :   an ANO file that includes soma location of all swc files; second column of each record is the cell id, e.g. 001. \n");
    printf("\t#q <qctable> :   output (name of the QC table).\n");
    printf("\t#s <skip_existing> :   whether to skip the preprocessing step if processed files exist \n");
    printf("Usage: vaa3d -x preprocess -f preprocess_batch -p \"#i swclist.txt #o ./Processed/ #m soma_list.ano.apo #q QC.csv #s 1\"\n");
}
