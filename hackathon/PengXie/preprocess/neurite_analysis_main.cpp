//neurite_analysis_main.cpp
//extract different types of neurite from a single tree: dendrite; long projection axon; axon clusters
//by Peng Xie
//2018-08-13

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "neurite_analysis_main.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neuron_connector_func.h"
#include "utilities.h"
#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include <io.h>
#endif

int get_soma(NeuronTree nt){
    const int N=nt.listNeuron.size();
    int soma;
    int soma_ct=0;
    // check whether unique soma
    for(int i=0; i<N; i++){
        // soma check
        if(nt.listNeuron.at(i).type==1){
            soma=i;
            soma_ct++;
            if(soma_ct>1){return -1;}
        }
    }
    return soma;
}

NeuronTree single_tree(NeuronTree nt, int soma){
    QList<int> pList;
    const int N=nt.listNeuron.size();
    int subtree_ct=0;
    // check whether this is a single tree
    for(int i=0; i<N; i++){
        // root check
        if(nt.listNeuron.at(i).pn<0){
            pList.append(-1);
            subtree_ct++;
        }
        else{
            int pid = nt.listNeuron.at(i).pn-1;
            pList.append(pid);
        }
    }
    if(subtree_ct==1){return nt;}
    if(subtree_ct==0){
        printf("No root found in the swc.\n");
        return nt;
    }
    if(subtree_ct>1){
        printf("More than one single trees in the input swc. Only the first connected with soma will be reported.\n");
        NeuronTree new_tree;
        // DFS to extract the first soma-connected tree;

        // Initialization
        int pid=soma;
        QList<int> visited;
        for(int i=0;i<N; i++){visited.append(0);}
        QStack<int> pstack;
        pstack.push(pid);
        bool is_push = false;
        visited[pid]=1;
        new_tree.listNeuron.append(nt.listNeuron.at(pid));

        // DFS using stack
        while(!pstack.isEmpty()){
            is_push = false;
            pid = pstack.top();
            // whether exist unvisited children of pid
            // if yes, push child to stack;
            for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
                if((nt.listNeuron.at(i).pn-1)==pid && visited.at(i)==0){
                    pstack.push(i);
                    visited[i]=1;
                    new_tree.listNeuron.append(nt.listNeuron.at(i));
                    is_push=true;
                    break;
                }
            }
            // else, pop pid
            if(!is_push){
                pstack.pop();
            }
        }
        return new_tree;
    }
}


void printHelp_neurite_analysis()
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
    printf("\t                         use 0 to skip, if not specified, use 0.\n");
    printf("\t#m <thres_connect_soma>       :   maximun distance to connect a node to soma.\n");
    printf("\t                         use 0 to skip, if not specified, use 1000.\n");
    printf("\t#t <thres>       :    gap threshold for connecting during the sorting procedure.\n");
    printf("\t                         use 0 to skip, if not specified, use 2.\n");
    printf("\t#r <rotation = 0>   :   whether to perform PCA alignment.\n");
    printf("\t                         if not specified, rotation is not performed\n");
    printf("\t#d <label_subtree = 0>   :   whether to give each subtree a different color.\n");
    printf("\t                         if not specified, labeling is not performed\n");
    printf("\t#f <return_maintree = 0>   :   whether to return only the maintree (a single tree connected to soma).\n");
    printf("\t                         if not specified, all trees will be reported\n");
    printf("Usage: vaa3d -x preprocess -f preprocess -p \"#i input.swc #o result.swc #l 2 #s 0 #m 2000 #t 0.25 #r 0 #d 0 #f 1\"\n");
}

bool neurite_analysis_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    printf("welcome to neurite_analysis\n");

    //0. read arguments
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    char *dfile_input = NULL;
    char *dfile_output = NULL;
    char *extract_type = NULL;

    if(1==1){
        if(input.size() != 2)
        {
            printf("Please specify parameter set.\n");
            printHelp_neurite_analysis();
            return false;
        }
        paralist = (vector<char*>*)(input.at(1).p);


        if (paralist->size()!=1)
        {
            printf("Please specify all paramters in one text string.\n");
            printHelp_neurite_analysis();
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
            printHelp_neurite_analysis();



        int c;
        static char optstring[]="ht:";
        extern char * optarg;
        extern int optind, opterr;
        optind = 1;
        while ((c = getopt(argc, argv, optstring))!=-1)
        {
            switch (c)
            {
                case 'h':
                    printHelp_neurite_analysis();
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
                case 't':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
                        return 1;
                    }
                    extract_type = optarg;
                    break;
            }
        }
    }



    // 1. Load data
    QString qs_input(dfile_input);
    NeuronTree nt;

    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC"))
    {
        nt = readSWC_file(qs_input);
    }

    QString outfileName = QString(dfile_output);
    if (dfile_output==NULL)
    {
        outfileName = qs_input+"_preprocessed.swc";
    }

    // 2. Examine input
    // 2.1 Check soma
    int soma=get_soma(nt);
    if(soma<0){return 0;}

    // 2.2 Check single tree
    nt = single_tree(nt, soma);
    writeSWC_file(outfileName, nt);

    return 0;


}
