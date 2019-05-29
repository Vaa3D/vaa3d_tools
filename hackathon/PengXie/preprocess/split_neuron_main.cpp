#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "split_neuron_main.h"

bool split_neuron(QString qs_input, QString qs_outdir){
    // Split neurons into different components
    QString qs_tag;
    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){qs_tag = qs_input.left(qs_input.length()-4);}
    if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){qs_tag = qs_input.left(qs_input.length()-5);}

    if(qs_outdir.size()>0){
        if(!qs_outdir.endsWith("/")){qs_outdir=qs_outdir+"/";}
        qs_tag = qs_tag.right(qs_tag.size()-qs_tag.lastIndexOf(("/"))-1);
        qs_tag = qs_outdir+qs_tag;
    }
    qDebug()<<qs_tag;
    // Report 1: axon.
    neurite_analysis(qs_input, qs_tag+".axon.swc", "a");
    // Report 2: long axon only.
    neurite_analysis(qs_tag+".axon.swc", qs_tag+".long_axon.swc", "l");
    // Report 3: other axons retyped
    axon_retype(qs_tag+".axon.swc", qs_tag+".long_axon.swc", qs_tag+".axon.retype.swc", true);
//    QList<double> lpa_density = arbor_distribution(qs_tag+".axon.swc", qs_tag+".long_axon.swc");
//    export_branch_distribution(lpa_density, qs_tag+".axon_density.txt");
    // Report 4: dendrite
    neurite_analysis(qs_input, qs_tag+".dendrite.swc", "d");

    return 1;
}

bool split_neuron_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //choose a directory that contain swc files
    QString qs_input;
    qs_input=QFileDialog::getOpenFileName(
                parent,
                "Please select input file (*.swc)\n",
                QDir::currentPath(),
                "All files (*.*)" ";; swc files (*.swc *.eswc)"
                );
    QString qs_output_dir("");

    // Split neuron
    split_neuron(qs_input, qs_output_dir);
    return 1;
}

bool split_neuron_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    printf("welcome to use split_neuron\n");
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
                cout << "Output dir:\t" << dfile_result <<endl;
                break;
            case '?':
                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                return 1;
                break;
        }
    }

    QString qs_input = QString(dfile_input);
    QString qs_output = QString(dfile_result);
    // Split neuron
    qs_input = QString(qPrintable(qs_input));
    split_neuron(qs_input, qs_output);

    return 1;
}
