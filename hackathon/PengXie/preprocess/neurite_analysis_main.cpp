//neurite_analysis_main.cpp
//extract different types of neurite from a single tree: dendrite; long projection axon; axon clusters
//by Peng Xie
//2018-08-13

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "neurite_analysis_main.h"

#if !defined(Q_OS_WIN32)
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include "io.h"
#endif


QList <int> find_long_axon(NeuronTree nt, int soma){

    int N = nt.listNeuron.size();
    QList<int> name_list;
    for(int i=0; i<N; i++){name_list.append(nt.listNeuron.at(i).n);}
    // 1. find longest path from soma
    // DFS
    // Initialization
    int pid=soma;
    QList<double> distance;
    for(int i=0;i<N; i++){distance.append(-1);}
    QStack<int> pstack;
    pstack.push(pid);
    bool is_push = false;
    distance[pid]=0;

    // DFS search
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited children of pid
        // if yes, push child to stack;
        for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
            if((nt.listNeuron.at(i).pn)==nt.listNeuron.at(pid).n && distance.at(i)==(-1.0)){
                pstack.push(i);
                distance[i]=distance.at(pid)+1;
                is_push=true;
                break;
            }
        }
        // else, pop pid
        if(!is_push){
            pstack.pop();
        }
    }
    QList<double> sort_distance = distance;
    qSort(sort_distance);
    double longest_distance = sort_distance.last();
    int endpoint = distance.lastIndexOf(longest_distance);
    cout<<"Longest distance:\t"<<distance.at(endpoint)<<"\t"<<"node "<<nt.listNeuron.at(endpoint).n <<endl;

    // 2. Return a list of node ids of the long projection axon
    QList <int> lpa;
    int cur_id = endpoint;
    lpa.prepend(cur_id);
    while(nt.listNeuron.at(cur_id).type!=1){
        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
        lpa.prepend(cur_id);
        if(nt.listNeuron.at(cur_id).pn<0){  // Reached root, should't happen
            break;
        }
    }
    return lpa;
}
NeuronTree return_long_axon(NeuronTree nt, int soma, bool color_tree){
    QList<int> idlist = find_long_axon(nt, soma);
    if(color_tree){
        nt = color_subtree_by_id(nt, idlist);
    }
    else{
        nt = get_subtree_by_id(nt, idlist);
    }
    return nt;
}
NeuronTree return_axon(NeuronTree nt, int soma){

    printf("Welcome to use return_axon.\n");
    // 1. Get axon subtrees starting from soma
    // 1.1 extract axon nodes
    NeuronTree new_tree = get_subtree_by_type(nt, 2);
    QList<int> name_list;
    for(int i=0; i<nt.listNeuron.size();i++){
        NeuronSWC node = nt.listNeuron.at(i);
        name_list.append(node.n);
    }
    // 1.2 Add axon-connected dendrite segments to axon subtrees
    // Find out whether any axon node connected to dendrite
    QList<int> a2d_ind;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        int pn_ind = name_list.indexOf(node.pn);
        if(pn_ind<0){continue;}
        if((nt.listNeuron.at(pn_ind).type==3) && (node.type==2)){
            a2d_ind.append(i);
            cout << i <<endl;
        }
    }
    // If any, trace back to soma;
    if(a2d_ind.size()>0){
        if(a2d_ind.size()>1){printf("Warning: %d axon nodes connected to dendrite!\n", a2d_ind.size());}
        for(int i=0; i<a2d_ind.size(); i++){
            bool soma_reached=0;
            NeuronSWC node = nt.listNeuron.at(a2d_ind.at(i));
            while(node.pn != -1){
                int pn_ind = name_list.indexOf(node.pn);
                node = nt.listNeuron.at(pn_ind);
                if(node.type==3){new_tree.listNeuron.append(node);}
                if(node.type==1){soma_reached==1;}
            }
            if(!soma_reached){
                printf("Warning: axon nodes connected to dendrite but cannot trace back to soma!\n");
            }
        }
    }

    nt = single_tree(new_tree, soma);
    return nt;

}
NeuronTree return_dendrite(NeuronTree nt, int soma){
    nt = get_subtree_by_type(nt, 3);
    nt = single_tree(nt, soma);
    return nt;
}

bool neurite_analysis(QString qs_input, QString qs_output, string extract_type){

    printf("welcome to neurite_analysis\n");

    // 1. Load data
    NeuronTree nt;

    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC") || qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC"))
    {
        nt = readSWC_file(qs_input);
    }

    if (qs_output.size()==0)
    {
        char *tag;
        if(extract_type == "a"){tag = strdup(".long_axon");}
        if(extract_type == "c"){tag = strdup(".cluster_axon");}
        if(extract_type == "d"){tag = strdup(".dendrite");}
        if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){qs_output = qs_input.left(qs_input.length()-4)+tag+".swc";}
        if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){qs_output = qs_input.left(qs_input.length()-5)+tag+".swc";}
    }

    // 2. Examine input
    // 2.1 Check soma
    int soma=get_soma(nt);
    if(soma<0){return 0;}
    if(nt.listNeuron.at(soma).pn != (-1)){
        printf("Exit: soma is not root!\n");
        return 0;
    }

    // 3. Extract the specified type of neurite
    printf("Extracting neurite\n");
    if(extract_type == "a" || extract_type == "l"){
        nt=return_axon(nt, soma);
        if(extract_type == "l"){
            nt = return_long_axon(nt, soma, false);
        }
    }
    if(extract_type == "d"){nt=return_dendrite(nt, soma);}

    writeSWC_file(qs_output, nt);
    return 1;
}

bool neurite_analysis_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

    //0. read arguments
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    char *dfile_input = NULL;
    char *dfile_output = NULL;
    string extract_type = "a";
    bool color_tree = 0;

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
        static char optstring[]="hi:o:t:c:";
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
                case 'c':
                    if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                    {
                        fprintf(stderr, "Found illegal or NULL parameter for the option -c.\n");
                        return 1;
                    }
                    color_tree = (atoi(optarg)==1);
                    break;
            }
        }
    }

    return neurite_analysis(QString(dfile_input), QString(dfile_output), extract_type);
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
    printf("\t                         if not specified, it is \"inputName.extract_type.swc\"\n");
    printf("\t#t <extract_type> :  which neurite type to extract.\n");
    printf("\t                         if not specified, long projection axon will be extracted\n");
    printf("\t#c <color_tree> :  whether to label the specified type by color.\n");
    printf("\t                         if not specified, only return the subtree\n");
    printf("\t                         if specified as 1, whole neuron will be returned, with the specified part showing white coloor\n");
    printf("Usage: vaa3d -x preprocess -f neurite_analysis -p \"#i input.swc #o result.swc #t a #c 1\"\n");
}
