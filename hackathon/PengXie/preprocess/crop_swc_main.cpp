//crop_swc_main.cpp
//center by soma, take a single tree within certain radius
//by Peng Xie
//2018-09-11

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "crop_swc_main.h"

#if !defined(Q_OS_WIN32)
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include "io.h"
#endif

bool crop_swc(QString qs_input, QString qs_output, double radius, int soma, bool center, int resample_step,
              double xshift, double yshift, double zshift,bool rotation, bool report_single_tree){

    printf("welcome to crop_swc\n");
    qDebug()<<report_single_tree;
    // 1. read input
    NeuronTree nt = readSWC_file(qs_input);
    NeuronSWC soma_node = nt.listNeuron.at(soma);
    int soma_name = nt.listNeuron.at(soma).n;
    int soma_type = nt.listNeuron.at(soma).type;
    QList <int> plist;
    QList <int> nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].r=1;
        NeuronSWC node = nt.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    // 2. center and crop swc
    // 2.1 decide which nodes are within certain distance

    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(radius<0){inside_nlist.append(node.n);}
        else{
            if(computeDist2(node, soma_node, 0.2, 0.2, 1)<radius){
                inside_nlist.append(node.n);
            }
        }
    }

    // 2.2 create a cropped tree
    NeuronTree new_tree;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        // If node is outside of the shell, skip to the next node.
        if(inside_nlist.lastIndexOf(node.n)<0){
            continue;
        }
        // If the parent node is to be deleted, put itself as a parent node.
        if(inside_nlist.lastIndexOf(node.pn)<0){
            node.pn = -1;
        }
        new_tree.listNeuron.append(node);
    }
    new_tree = missing_parent(new_tree);
    export_list2file(new_tree.listNeuron, "crop_test.swc");

    // 2.3 return single tree
    if(report_single_tree){
        qDebug()<<"Soma name"<<soma_name;
        nt.deepCopy(my_SortSWC(new_tree, soma_name, 0));  // Sort the tree so "soma" is the root;
        nt.deepCopy(single_tree(nt, 0));
    }
    else{
        nt.deepCopy(my_SortSWC(new_tree, soma_name, 0));
    }

    // 3. resample
    if (resample_step>0){
        printf("\tResampling\n");
        nt = resample(nt, resample_step);
        // The resample step will change the order and name of nodes, but the parent node won't change.
        for(int i=0; i<nt.listNeuron.length(); i++){
            NeuronSWC node=nt.listNeuron.at(i);
            if((node.pn == -1) && (computeDist2(node, soma_node, 0.2, 0.2, 1)<1)){
                soma_name = node.n;
                nt.listNeuron[i].type = soma_type;
            }
        }
        nt.deepCopy(my_SortSWC(nt, soma_name, 0));
    }else{
        printf("\tSkip resampling\n");
    }

    //2.6 Align axis
    if (rotation)
    {
        printf("\tAligning PCA axis\n");
        nt = align_axis(nt);
    }
    else{
        printf("\tSkip PCA alignment\n");
    }


    soma_node = nt.listNeuron.at(0);
    if(center){
        xshift -= soma_node.x;
        yshift -= soma_node.y;
        zshift -= soma_node.z;
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].x += xshift;
        nt.listNeuron[i].y += yshift;
        nt.listNeuron[i].z += zshift;
    }


    // 4. save output
    export_list2file(nt.listNeuron, qs_output);
    return 1;
}

bool crop_swc_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

    //0. read arguments
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    char *dfile_input = NULL;
    char *dfile_output = NULL;
    double radius;
    int soma=0;
    char *soma_file = NULL;
    bool center=1;
    int resample_step=2;
    double xshift=0;
    double yshift=0;
    double zshift=0;
    bool rotation=0;
    bool report_single_tree=1;

    if(1==1){
        if(input.size() != 2)
        {
            printf("Please specify parameter set.\n");
            printHelp_crop_swc();
            return false;
        }
        paralist = (vector<char*>*)(input.at(1).p);

        if (paralist->size()!=1)
        {
            printf("Please specify all paramters in one text string.\n");
            printHelp_crop_swc();
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
            printHelp_crop_swc();

        int c;
        static char optstring[]="hi:o:s:m:r:c:j:t:x:y:z:a:";
        extern char * optarg;
        extern int optind, opterr;
        optind = 1;
        while ((c = getopt(argc, argv, optstring))!=-1)
        {
            switch (c)
            {
                case 'h':
                    printHelp_crop_swc();
                    return 0;
                    break;
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
                    dfile_output = optarg;
                    break;
                case 's':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
                        return 1;
                    }
                    soma = atoi(optarg);
                break;
                case 'm':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                        return 1;
                    }
                    soma_file = optarg;
                    break;
                case 'r':
                    if (strcmp(optarg,"(null)")==0)
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                        return 1;
                    }
                    radius = atof(optarg);
                break;
                case 'c':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                        return 1;
                    }
                    center = (atoi(optarg)==1);
                break;
                case 't':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                        return 1;
                    }
                    rotation = (atoi(optarg)==1);
                case 'a':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                        return 1;
                    }
                    report_single_tree = (atoi(optarg)==1);
            break;
                case 'j':
                    if (strcmp(optarg,"(null)")==0)
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -j.\n");
                        return 1;
                    }
                    resample_step = atoi(optarg);
                    break;
                case 'x':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -x.\n");
                        return 1;
                    }
                    xshift = atof(optarg);
                    break;
                case 'y':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -y.\n");
                        return 1;
                    }
                    yshift = atof(optarg);
                    break;
                case 'z':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -z.\n");
                        return 1;
                    }
                    zshift = atof(optarg);
                    break;
                case '?':
                    fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                    return 1;
                    break;
            }
        }
    }

    QString qs_input = QString(dfile_input);
    QString qs_output;
    if(dfile_output != NULL){
        qs_output = QString(dfile_output);
    }
    else{
        qs_output = QString((dfile_input));
        if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
            qs_output = qs_output.left(qs_output.length() - 4) + QString(".cropped.swc");
        }
        if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
            qs_output = qs_output.left(qs_output.length() - 5) + QString(".cropped.eswc");
        }
    }
    // Center of the cropped region can be sepecified by an swc or apo file
    if(soma_file != NULL){
        QString qs_soma = QString(soma_file);
        NeuronSWC node_soma;
        bool exist_soma = 0;
        // Case 1: input is an swc
        if(qs_soma.endsWith("swc") || qs_soma.endsWith("SWC")){
            QList<NeuronSWC> ntlist_soma = readSWC_file(qs_soma).listNeuron;
            if(ntlist_soma.size()>0){
                node_soma = ntlist_soma.at(0);
                exist_soma = 1;
                if(ntlist_soma.size()>1){
                    qDebug()<<"Input soma file contains >1 nodes! Using the first one as center";
                }
            }
        }
        // Case 2: input is an apo
        if(qs_soma.endsWith("apo") || qs_soma.endsWith("APO")){
            QList<CellAPO> markers = readAPO_file(qs_soma);
            if(markers.size()>0){
                node_soma = CellAPO_to_NeuronSWC(markers.at(0));
                exist_soma = 1;
                if(markers.size()>1){
                    qDebug()<<"Input soma file contains >1 nodes! Using the first one as center";
                }
            }
        }

        if(exist_soma){
            NeuronTree nt = readSWC_file(qs_input);
            double min_dist=radius;
            for(int i=0; i<nt.listNeuron.size();i++){
                NeuronSWC node = nt.listNeuron.at(i);
                if(computeDist2(node_soma, node, 1,1,5)<min_dist){
                    min_dist = computeDist2(node_soma, node, 1,1,5);
                    soma = i;
                }
            }
        }
        else{
            qDebug()<<"Input soma file was empty!";
        }
    }

    return crop_swc(qs_input, qs_output, radius, soma, center, resample_step,
                    xshift, yshift, zshift, rotation, report_single_tree);
}

void printHelp_crop_swc()
{
    printf("\nVaa3D plugin: \n");
    printf("\t1) crop swc by radius and center it by soma; \n");
    printf("\t2) soma-connected single tree will be reported; \n");
    printf("\t#i <neuron_filename> :   input neuron structure (.swc) name\n");
    printf("\t#o <output_filename> :   output file name; default output name: input.cropped.swc\n");
    printf("\t#s <soma> :  soma index; default is 0;\n");
    printf("\t#m <soma_file> :  soma file (either .swc or .apo); default is NULL;\n");
    printf("\t#r <radius> :  the range for cropping; set to -1 if you don't want to crop.\n");
    printf("\t#j <reample_step> :  step size for resampling; set to -1 if you'd like to skip resampling.\n");
    printf("\t#x <xshift> :  shift soma node along x-axis.\n");
    printf("\t#y <yshift> :  shift soma node along y-axis.\n");
    printf("\t#z <zshift> :  shift soma node along z-axis.\n");
    printf("\t#c <center> :  whether to move soma to the origion; default is 1.\n");
    printf("\t#t <rotation> :  whether to perform PCA projection; default is 0.\n");
    printf("\t#a <report_single_tree> :  whether to report a single tree; default is 1.\n");
    printf("Usage: vaa3d -x preprocess -f crop_swc -p \"#i input.swc #o result.swc #r 100 #j 2\"\n");
}
