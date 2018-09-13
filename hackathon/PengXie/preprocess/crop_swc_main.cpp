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

bool crop_swc(QString qs_input, QString qs_output, double radius, int soma=0, bool center=1, int resample_step=2,
              double xshift=0, double yshift=0, double zshift=0){

    printf("welcome to crop_swc\n");
    // 1. read input
    NeuronTree nt = readSWC_file(qs_input);
    int soma_name = nt.listNeuron.at(soma).n;
    QList <int> plist;
    QList <int> nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].r=1;
        NeuronSWC node = nt.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    // 2. center and crop swc
    // 2.1 center swc and decide which nodes are within certain distance
    NeuronSWC soma_node = nt.listNeuron.at(soma);
    if(center){
        xshift -= soma_node.x;
        yshift -= soma_node.y;
        zshift -= soma_node.z;
    }
    NeuronSWC new_soma_node = nt.listNeuron.at(soma);
    new_soma_node.x += xshift;
    new_soma_node.y += yshift;
    new_soma_node.z += zshift;

    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].x += xshift;
        nt.listNeuron[i].y += yshift;
        nt.listNeuron[i].z += zshift;
        NeuronSWC node = nt.listNeuron.at(i);
        if(computeDist2(node, new_soma_node, 1, 1, 5)<radius){
//            printf("%d distance:\t%f\n", i, computeDist2(node, new_soma_node, 1, 1, 5));
            inside_nlist.append(node.n);
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
    export_list2file(new_tree.listNeuron, "cropped.swc");
    // 2.3 return single tree
    nt = single_tree(new_tree, soma);
    export_list2file(nt.listNeuron, "single_tree.swc");
    // 2.4 sort the single tree
    nt = my_SortSWC(nt, soma_name, 0);

    // 3. resample
    if (resample_step>0){
        printf("\tResampling\n");
        nt = resample(nt, resample_step);
    }else{
        printf("\tSkip resampling\n");
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
    bool center=1;
    int resample_step=2;
    double xshift=0;
    double yshift=0;
    double zshift=0;

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
        static char optstring[]="hi:o:s:r:c:j:x:y:x:";
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
                case 'r':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
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

    return crop_swc(qs_input, qs_output, radius, soma, center, resample_step,
                    xshift, yshift, zshift);
}

void printHelp_crop_swc()
{
    printf("\nVaa3D plugin: \n");
    printf("\t1) crop swc by radius and center it by soma; \n");
    printf("\t2) soma-connected single tree will be reported; \n");
    printf("\t#i <neuron_filename> :   input neuron structure (.swc) name\n");
    printf("\t#o <output_filename> :   output file name; default output name: input.cropped.swc\n");
    printf("\t#s <soma> :  soma index; default is 0;\n");
    printf("\t#r <radius> :  the range for cropping.\n");
    printf("\t#j <reample_step> :  step size for resampling; set to -1 if you'd like to skip resampling.\n");
    printf("\t#x <xshift> :  shift soma node along x-axis.\n");
    printf("\t#x <yshift> :  shift soma node along y-axis.\n");
    printf("\t#x <zshift> :  shift soma node along z-axis.\n");
    printf("Usage: vaa3d -x preprocess -f crop_swc -p \"#i input.swc #o result.swc #r 100 #j 2\"\n");
}
