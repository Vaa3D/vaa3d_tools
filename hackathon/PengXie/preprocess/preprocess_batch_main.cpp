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


bool preprocess_batch(QString inswclist, QString outswcdir, QString somalist, QString qctable, bool skip_existing){

//#ifdef Q_OS_WIN32
//    QString sep("\\");
//#else
//    QString sep("/");
//#endif

    QString sep("/");
    std::ifstream infile(qPrintable(inswclist));
    std::string line;
    printf("welcome to use preprocess_batch\n");

    if(!fexists(somalist)){
        printf("Error: soma list file not found!\n");
        return 0;
    }
    QList<CellAPO> soma_apo = readAPO_file(somalist);
    QList<QString> soma_nlist;
    for(int i=0; i<soma_apo.size(); i++){
        QString tp_name = soma_apo.at(i).name;
        tp_name.replace(QString(" "), QString(""));
        soma_nlist.append(tp_name);
    }

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

        // 0. Prepare files
        // 0.1 Input
        printf("Input swc:\t%s\n", qPrintable(qs_inswc));
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
        QString inswcName = inswcPrefix.right(inswcPrefix.length()-inswcPrefix.lastIndexOf(sep)-1);
        if(outswcdir.size()==0){  // If outswcdir is not specified, processed swc will be put into the same folder as input swc.
            outswcdir = inswcPrefix;
            if(inswcPrefix.lastIndexOf(sep)>=0){
                outswcdir = outswcdir.left(inswcPrefix.lastIndexOf(sep)+1);
            }
            else{
                outswcdir = QString(".")+sep;
            }
        }
        // 0.2 Temp
        // temp_apo
        QString temp_apo = outswcdir + inswcName + QString(".temp.apo");
        QString inswcID = inswcName.right(inswcName.length()-inswcName.indexOf("_")-1);
        if(inswcID.indexOf("_")>=0){
            inswcID = inswcID.left(inswcID.indexOf("_"));
        }
        int sid = soma_nlist.indexOf(inswcID);
        if(sid < 0){
            printf("Error: soma id %s not found in soma_list!\n", qPrintable(inswcID));
//            continue;
        }
        else{
            QList<CellAPO> this_soma;
            this_soma.append(soma_apo.at(sid));
            writeAPO_file(temp_apo, this_soma);
            QFile temp_apo_file(temp_apo);
        }
        // temp_swc
        QString temp_swc = outswcdir + inswcName + QString(".temp.swc");
        QFile temp_swc_file(temp_swc);
        QFile qs_inswc_file(qs_inswc);
        if(temp_swc_file.exists()){
            temp_swc_file.remove();
        }
        qs_inswc_file.copy(temp_swc);


        // 0.3 Output
        QString qs_outswc = outswcdir + inswcName + QString(".processed.swc");
        // 1. Run preprocess
        if((!skip_existing) || (!fexists(qs_outswc))){
            bool preprocessed = pre_processing(temp_swc, qs_outswc);
            if(! preprocessed){
                return 0;
            }

            // 2. Run QC
            NeuronTree nt = readSWC_file(qs_outswc);
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
//        temp_apo_file.remove();
        temp_swc_file.remove();
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
    char *somalist = NULL;
    char *swcdir = NULL;
    char *qctable = NULL;
    bool skip_existing = 0;

    int c;
    static char optstring[]="hi:o:m:q:s:";
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
            case 'o':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                swcdir = optarg;
                cout << "Output swc directory:\t" << swcdir <<endl;
            break;
            case 'm':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                somalist = optarg;
                cout << "Somalist:\t" << somalist <<endl;
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

    bool finished = preprocess_batch(QString(swclist), QString(swcdir), QString(somalist), QString(qctable), skip_existing);
    return finished;
}

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
