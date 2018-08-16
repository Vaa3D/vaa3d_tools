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
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
//#include "getopt_tool.h"
#include "io.h"
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

    // Case 1
    if(subtree_ct==1){return nt;}

    // Case 2
    if(subtree_ct==0){
        printf("No root found in the swc.\n");
        return nt;
    }

    // Case 3
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

NeuronTree long_axon_extractor(NeuronTree nt, int soma){
    // extract axon nodes
    NeuronTree new_tree;
    QList<int> name_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(node.type==1 || node.type==2){
            new_tree.listNeuron.append(node);
            name_list.append(node.n);
        }
    }
    nt = new_tree;
    int N = nt.listNeuron.size();
    export_list2file(nt.listNeuron, "test.swc");

    // Case that needs consideration: if more than 1 axon braches connected to soma
    // Case that needs consideration: if no axon connected to soma

    // find longest path from soma
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

    // extract nodes on the longest path
    new_tree.listNeuron.clear();
    int cur_id = endpoint;
    new_tree.listNeuron.prepend(nt.listNeuron.at(cur_id));
    while(nt.listNeuron.at(cur_id).type!=1){
        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
        new_tree.listNeuron.prepend(nt.listNeuron.at(cur_id));
        if(nt.listNeuron.at(cur_id).pn<0){  // Reached root
            break;
        }
    }
    return new_tree;
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
    printf("Usage: vaa3d -x preprocess -f neurite_analysis -p \"#i input.swc #o result.swc #t a\"\n");
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
    string extract_type = "a";

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
        static char optstring[]="hi:o:t:";
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
        char *tag;
        if(extract_type == "a"){tag = strdup(".long_axon");}
        if(extract_type == "c"){tag = strdup(".cluster_axon");}
        if(extract_type == "d"){tag = strdup(".dendrite");}
        if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){outfileName = qs_input.left(qs_input.length()-4)+tag+".swc";}
        if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){outfileName = qs_input.left(qs_input.length()-5)+tag+".swc";}
    }

    // 2. Examine input
    // 2.1 Check soma
    printf("Checking soma\n");
    int soma=get_soma(nt);
    if(soma<0){return 0;}

    // 2.2 Check single tree
    printf("Checking single tree\n");
    double total_size = nt.listNeuron.size();
    nt = single_tree(nt, soma);
    if(nt.listNeuron.size() / total_size < 0.1){
        printf("Warning: only %f%% belongs to the soma-connected single tree!\n", nt.listNeuron.size()/total_size*100);
    }


    // 3. Extract the specified type of neurite
    printf("Extracting neurite\n");
    if(extract_type == "a"){nt=long_axon_extractor(nt, soma);}


    writeSWC_file(outfileName, nt);

    return 1;


}
