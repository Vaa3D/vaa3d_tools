//crop_swc_cuboid_main.cpp
//crop swc by a given cuboid, instead of a sphere
//by Peng Xie
//2018-11-20

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "crop_swc_cuboid_main.h"

#if !defined(Q_OS_WIN32)
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include "io.h"
#endif

bool in_cuboid(NeuronSWC node, XYZ small, XYZ large){
    if((node.x>=small.x) & (node.x<=large.x) &
            (node.y>=small.y) & (node.y<=large.y) &
            (node.z>=small.z) & (node.z<=large.z)
            )
    {
        return 1;
    }
    return 0;
}

bool crop_swc_cuboid(QString qs_input, QString qs_output,
              double xs, double ys, double zs,
              double xe, double ye, double ze,
              double xshift, double yshift, double zshift
              )
{
    printf("welcome to use crop_swc_cuboid\n");
    // 1. read input
    XYZ small(xs, ys, zs);
    XYZ large(xe, ye, ze);
    qDebug()<<small.x<<small.y<<small.z;
    qDebug()<<large.x<<large.y<<large.z;
    NeuronTree nt = readSWC_file(qs_input);
    // 2. Decide which nodes are within the cuboid
    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(in_cuboid(node, small, large)){inside_nlist.append(node.n);}
    }
    qDebug()<<inside_nlist.size();
    //3. create a cropped tree
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
    nt.deepCopy(my_SortSWC(new_tree, VOID, 0));
    //4. shift if needed
    if((xshift!=0) || (yshift!=0) || (zshift!=0)){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x -= xshift;
            nt.listNeuron[i].y -= yshift;
            nt.listNeuron[i].z -= zshift;
        }
    }
    // 5. save output
    export_list2file(nt.listNeuron, qs_output);
    return 1;
}

bool crop_swc_cuboid_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

    //0. read arguments
    char * dfile_input = ((vector<char*> *)(input.at(0).p))->at(0);
    char * dfile_output = ((vector<char*> *)(output.at(0).p))->at(0);
    vector<char*>* paralist = NULL;

    double xs=0;
    double ys=0;
    double zs=0;
    double xe=0;
    double ye=0;
    double ze=0;
    double xshift=0;
    double yshift=0;
    double zshift=0;
    if(1==1){
        if(input.size() != 2)
        {
            printf("Please specify parameter set.\n");
            printHelp_crop_swc_cuboid();
            return false;
        }
        paralist = (vector<char*>*)(input.at(1).p);

        if (paralist->size()!=1)
        {
            printf("Please specify all paramters in one text string.\n");
            printHelp_crop_swc_cuboid();
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
            printHelp_crop_swc_cuboid();

        int c;
        static char optstring[]="ha:b:c:d:e:f:r:s:t:";
        extern char * optarg;
        extern int optind, opterr;
        optind = 1;
        while ((c = getopt(argc, argv, optstring))!=-1)
        {
            switch (c)
            {
                case 'h':
                    printHelp_crop_swc_cuboid();
                    return 0;
                    break;
                case 'a':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -a.\n");
                        return 1;
                    }
                    xs = atof(optarg);
                    break;
            case 'b':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -b.\n");
                    return 1;
                }
                ys = atof(optarg);
                break;
            case 'c':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -c.\n");
                    return 1;
                }
                zs = atof(optarg);
                break;
            case 'd':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -d.\n");
                    return 1;
                }
                xe = atof(optarg);
                break;
            case 'e':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -e.\n");
                    return 1;
                }
                ye = atof(optarg);
                break;
            case 'f':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -f. \n");
                    return 1;
                }
                ze = atof(optarg);
                break;
            case 'r':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                    return 1;
                }
                xshift = atof(optarg);
                break;
            case 's':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
                    return 1;
                }
                yshift = atof(optarg);
                break;
            case 't':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
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

    return crop_swc_cuboid(qs_input, qs_output, xs,ys,zs, xe,ye,ze, xshift,yshift,zshift);
}

void printHelp_crop_swc_cuboid()
{
    printf("\nVaa3D plugin: \n");
    printf("\t1) crop swc by a given cuboid; \n");
    printf("\t-i <neuron_filename> :   input neuron structure (.swc) name\n");
    printf("\t-o <output_filename> :   output file name; default output name: input.cropped.swc\n");
    printf("\t#a <xsmall> :  smaller end of the cuboid along x-axis.\n");
    printf("\t#b <ysmall> :  smaller end of the cuboid along y-axis.\n");
    printf("\t#c <zsmall> :  smaller end of the cuboid along z-axis.\n");
    printf("\t#d <xlarge> :  larger end of the cuboid along x-axis.\n");
    printf("\t#e <ylarge> :  larger end of the cuboid along y-axis.\n");
    printf("\t#f <zlarge> :  larger end of the cuboid along z-axis.\n");
    printf("\t#r <xshift> :  shift soma node along x-axis.\n");
    printf("\t#s <yshift> :  shift soma node along y-axis.\n");
    printf("\t#t <zshift> :  shift soma node along z-axis.\n");
    printf("Usage: vaa3d -x preprocess -f crop_swc_cuboid -i input.swc -o result.swc -p \"#a 1000 #b 1000 #c 1000 #d 2000 #e 2000 #c 2000 #r 500 #s 500 #t 500\"\n");
}
