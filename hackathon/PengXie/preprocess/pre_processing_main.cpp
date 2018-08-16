//pre_processing_main.cpp
//main function for pre processing (blastneron)
//by Yinan Wan
//2012-06-02
//modified by Peng Xie
//2018-07-27

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "QtGui"
#include "pre_processing_main.h"
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

NeuronTree connect_soma(NeuronTree nt, QList<CellAPO> markers, double dThres, QString outfileLabel, double drop_thres=1e6, bool colorful=true, bool return_maintree=false)  // Adapted from /home/penglab/Desktop/vaa3d/vaa3d_tools/released_plugins/v3d_plugins/connect_neuron_fragments_extractor/neuron_extractor_plugin.cpp
{
    QList<int> components;
    QList<int> pList;
    int ncomponents=0;
    const int N=nt.listNeuron.size();

    //connected component
    for(V3DLONG i=0; i<N; i++){
        if(nt.listNeuron.at(i).pn<0){
            components.append(ncomponents); ncomponents++;
            pList.append(-1);
        }
        else{
//            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            int pid = nt.listNeuron.at(i).pn-1;
            components.append(-1);
            pList.append(pid);
        }
    }

    //assign nodes to components
    for(int cid=0; cid<ncomponents; cid++){
//    for(int cid=0; cid<1; cid++){
        QStack<int> pstack;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        int pid=components.indexOf(cid);
        pstack.push(pid);

        // DFS to mark the component of currunt root (cid);
        QList<int> visited;
        for(int i=0;i<N; i++){visited.append(0);}
        bool is_push = false;
        visited[pid]=1;
        while(!pstack.isEmpty()){
            is_push = false;
            pid = pstack.top();
            // whether exist unvisited children of pid
            // if yes, push child to stack;
            for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
                if((nt.listNeuron.at(i).pn-1)==pid && visited.at(i)==0){
                    pstack.push(i);
                    visited[i]=1;
                    is_push=true;
                    components[i]=cid;
                    break;
                }
            }
            // else, pop pid
            if(!is_push){
                pstack.pop();
            }
        }
    }
    // Add color to components
    if(colorful){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].type=components.at(i)+1;
        }
    }

    //check if marker exists and use it as root
    int somarootid = 0;
    if(markers.size()>0)
    {
        QList<NeuronSWC> S_list;
        NeuronSWC S;
        if(markers.size()==1)
        {
            //markers.at(0).operator XYZ;
            S.x = markers.at(0).x;
            S.y = markers.at(0).y;
            S.z = markers.at(0).z;
//            S.r = markers.at(0).volsize;
            S.r = 100;
            S.type = 1;
            S.n = 0;
            S.pn = -1;
            S_list.append(S);

            qDebug()<< "Marker found. Using it as root."; //<< neuron.size()<< neuron.at(neuron.size()-1).pn;
        }
        else{
            QList <CellAPO> new_markers;
            for(int i=0;i<markers.size();i++)
            {
                if(markers.at(i).comment == "soma")//markers.at(i).color.r == 0 && markers.at(i).color.g == 0 && markers.at(i).color.b == 255)
                {
                    //markers.at(0).operator XYZ;
                    S.x = markers.at(i).x;
                    S.y = markers.at(i).y;
                    S.z = markers.at(i).z;
                    S.type = 1;
                    S.r = 100;
                    S.n = 0;
                    S.pn = -1;
                    S_list.append(S);
                    new_markers.append(markers.at(i));
                    markers = new_markers;
                    break;
                }
            }
            qDebug()<< "Warning: more than one marker in the file, taking one commented as 'soma'";// << neuron.size();
        }
        for(int i=0;i<N;i++){S_list.append(nt.listNeuron.at(i));}
        nt.listNeuron = S_list;
        markers[0].color.r=0;
        markers[0].color.g=0;
        markers[0].color.b=0;
        somarootid = 0;
    }

    // whether a node is a tip;
    QList<bool> istip;
    for(int i=0; i<N; i++){
        if(pList.count(i)==0){istip.append(true);}
        else{istip.append(false);}
    }


    // For each subtree, connect the closest end (tip or root) to soma, if within certain distance.
    cout << "Total components:\t" <<ncomponents << endl;
    QList<int> drop_list;
    int bad_nodes=0;
    for(int cid=0; cid<ncomponents; cid++){

        // Find closet ends
        cout << "\nComponent\t" << cid <<endl;
        QList<double> edist; // distance from end to soma
        QList<int> eid; // end id
        int ct = 0;
        for(int i=0; i<N; i++){
            if((istip.at(i) || pList.at(i)==-1) && (components.at(i)==cid)){  //  i is tip or root
                edist.append(computeDist2(nt.listNeuron.at(i+1), nt.listNeuron.at(somarootid)));
                eid.append(i);
            }
            if(components.at(i)==cid){ct++;}
        }
        cout << "# Nodes:\t" << ct <<endl;

        QList<double> dsorted = edist;
        qSort(dsorted);
        // Drop a subtree if it's too far from
        if(dsorted.at(0) > drop_thres){
            cout<<"Operation: Deleted!\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
            for(int i=0; i<N; i++){
                if(components.at(i)==cid){
                    drop_list.append(i+1);
                    bad_nodes++;
                }
            }
            printf("Subtree size:\t%d\n", ct);
            continue;
        }

        // Whether to connect subtrees to soma
        if(dsorted.at(0) < dThres){
            int cend = eid.at(edist.indexOf(dsorted.at(0)));
            cout<<"Operation: Connected to soma.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
            markers.append(get_marker(nt.listNeuron.at(eid.at(edist.indexOf(dsorted.at(0)))+1), 50, 255, 255, 255));
            // If this is a root, connect with soma
            if(pList.at(cend)==-1){
                nt.listNeuron[cend+1].pn = somarootid;
            }
            // If this is a tip, creat a root at the same position and connect to soma.
            else{
                NeuronSWC psuedo_root = nt.listNeuron.at(cend+1);
                psuedo_root.pn = somarootid;
                psuedo_root.n = nt.listNeuron.size()+1;
                nt.listNeuron.append(psuedo_root);
            }
        }
        else{
             // Label un-connected subtrees
            markers.append(get_marker(nt.listNeuron.at(eid.at(edist.indexOf(dsorted.at(0)))+1), 50, 255, 0, 0));
            if(!return_maintree){
                cout<<"Operation: kept but not connected.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
                for(int i=0; i<N;i++){
                    if(components.at(i)==cid){
                        nt.listNeuron[i+1].type = 0;
                        bad_nodes++;
                    }
                }
            }
            else{
                cout<<"Operation: Deleted!\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
                for(int i=0; i<N;i++){
                    if(components.at(i)==cid){
                        drop_list.append(i+1);
                        bad_nodes++;
                    }
                }
            }
        }
    }
    // Drop distal subtrees.
    if(drop_list.size()>0){
        printf("Nodes droped:\t%d\n", drop_list.size());
        nt = rm_nodes(nt, drop_list);
    }
    writeAPO_file(outfileLabel+QString(".apo"), markers);
    cout<<bad_nodes<<endl;

    // QC report
    FILE * fp=0;
    fp = fopen((char *)qPrintable(outfileLabel+QString(".QC.txt")), "wt");
    fprintf(fp, "Percentage_lost\t%f%%\n", bad_nodes*100.0/N);
    fclose(fp);
    return nt;
}

bool pre_processing(QString qs_input, QString qs_output, double prune_size = 2, double thres = 0,
                    double step_size = 0, double connect_soma_dist = 2000, bool rotation = false,
                    bool colorful = false, bool return_maintree = false)
{

    // 1. Load data
    printf("Loading swc\n");

    NeuronTree nt;
    QList<CellAPO> markers;

    if (qs_input.endsWith("swc") || qs_input.endsWith("SWC"))
    {
        nt = readSWC_file(qs_input);
    }

    //2. start processing

    //2.1 Prune
    printf("Pruning short branches\n");
    NeuronTree pruned;

    if (!prune_branch(nt, pruned, prune_size))
    {
        fprintf(stderr,"Error in prune_short_branch.\n");
        return 1;
    }

    //2.2 Remove duplicates
    printf("Removing duplicates\n");
    QList<NeuronSWC> newNeuron;
    NeuronTree deduped;
    // Maybe not the best solution. Use it for now.
    SortSWC(pruned.listNeuron, deduped.listNeuron, VOID, thres);

    //2.3 Resample
    printf("Resampling\n");
    NeuronTree resampled = deduped;
    if (step_size>0){
        printf("Resampling along segments\n");
        resampled = resample(pruned, step_size);
    }else{
        printf("Skip Resampling\n");
        resampled=pruned;
    }

    //2.4 Connect to soma
    printf("Connecting to soma\n");
    NeuronTree connected = deduped;
    if (connect_soma_dist>0){
        printf(qPrintable(qs_input),"\n");
        if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){
            markers = readAPO_file(qs_input.left(qs_input.length() - 4) + QString(".apo"));
        }
        if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){
            markers = readAPO_file(qs_input.left(qs_input.length() - 5) + QString(".apo"));
        }
        if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
            connected = connect_soma(deduped, markers, connect_soma_dist, qs_output.left(qs_output.length() - 4), 1e6, colorful, return_maintree);
        }
        if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
            connected = connect_soma(deduped, markers, connect_soma_dist, qs_output.left(qs_output.length() - 5), 1e6, colorful, return_maintree);
        }
    }
    cout << connected.listNeuron.size()<<endl;

    //2.5 Sort the tree.
    printf("Sorting\n");
    NeuronTree sorted;
    SortSWC(connected.listNeuron, sorted.listNeuron, 0, thres);
    // To be implemented: remove duplicates

    //2.6 Align axis
    NeuronTree result;
    if (rotation)
    {
        printf("Aligning PCA axis\n");
        result = align_axis(sorted);
    }
    else{
        printf("Skip PCA alignment\n");
        result = sorted;
    }

    if (export_listNeuron_2swc(result.listNeuron,qPrintable(qs_output)))
        printf("\t %s has been generated successfully.\n",qPrintable(qs_output));
    bool print_apo = connect_soma_dist>0;
    my_saveANO(qs_output.left(qs_output.length() - 4), true, print_apo);

    return 1;
}

bool pre_processing_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

	printf("welcome to pre_processing\n");
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
    double prune_size = 2; //default case
    double step_size = 0;
    double connect_soma_dist = 1000;
    double thres = 0;
    bool rotation = false;
    bool colorful = false;
    bool return_maintree = false;
	
	int c;
    static char optstring[]="hi:o:l:s:m:t:r:d:f:";
	extern char * optarg;
	extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
        {
            case 'h':
				printHelp_pre_processing();
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
				dfile_result = optarg;
				break;

             case 'l':
                 if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -l.\n");
                    return 1;
                }
                prune_size = atof(optarg);
                if (prune_size<-1)
                {
                    fprintf(stderr, "Illegal prune size. Special case: 0 -- no prunning; -1 -- default prunning size.\n");
                }
                  break;
            case 's':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				step_size = atof(optarg);
                if (step_size<0)
                {
                    fprintf(stderr, "Illegal step_size.\n");
                }
                break;
            case 'm':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -m.\n");
                    return 1;
                }
                connect_soma_dist = atof(optarg);
            case 't':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
                    return 1;
                }
                thres = atof(optarg);
                if (thres<-1)
                {
                    fprintf(stderr, "Illegal thres.\n");
                }
                break;
           case 'r':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -r.\n");
                    return 1;
                }
                rotation = (atoi(optarg)==1);
                break;
            case 'd':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -d.\n");
                    return 1;
                }
                if(atoi(optarg)==1){
                    colorful=true;
                    cout<<"Compartments will be labeled\n";
                }
                else{
                    colorful=false;
                    cout<<"Compartments will not be labeled\n";
                }
                break;
            case 'f':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -f.\n");
                    return 1;
                }
                else{
                    if(atoi(optarg)==1){
                        return_maintree=true;
                        cout<<"Only return main tree\n";
                    }
                    else{return_maintree=false;cout<<"Report main tree and subtrees\n";}
                    break;
                }

            case '?':
                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                return 1;
                break;
		}
	}

    pre_processing(QString(dfile_input), QString(dfile_result), prune_size, thres, step_size, connect_soma_dist, rotation, colorful, return_maintree);
	return 1;
}

bool pre_processing_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    double prune_size = 2; //default case
    double step_size = 0;
    double connect_soma_dist = 1000;
    double thres = 0;
    bool rotation = false;
    bool colorful = false;
    bool return_maintree = false;

    //choose a directory that contain swc files
    QString qs_input;
    qs_input=QFileDialog::getOpenFileName(
                parent,
                "Please select input file (*.swc)\n",
                QDir::currentPath(),
                "All files (*.*) ;; swc files (*.swc *.eswc)"
                );
    QString qs_output(qPrintable(qs_input));
    if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
        qs_output = qs_output.left(qs_output.length() - 4) + QString(".processed.swc");
    }
    if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
        qs_output = qs_output.left(qs_output.length() - 5) + QString(".processed.eswc");
    }

    pre_processing(qs_input, qs_output, prune_size, thres, step_size, connect_soma_dist, rotation, colorful, return_maintree);
    return 1;
}

void printHelp_pre_processing()
{
    printf("\nVaa3D plugin: modifiled pre-processing step analysis/QC, including: \n");
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
    printf("\t                        if not specified, use 0.\n");
    printf("\t#r <rotation = 0>   :   whether to perform PCA alignment.\n");
    printf("\t                         if not specified, rotation is not performed\n");
    printf("\t#d <label_subtree = 0>   :   whether to give each subtree a different color.\n");
    printf("\t                         if not specified, labeling is not performed\n");
    printf("\t#f <return_maintree = 0>   :   whether to return only the maintree (a single tree connected to soma).\n");
    printf("\t                         if not specified, all trees will be reported\n");
    printf("Usage: vaa3d -x preprocess -f preprocess -p \"#i input.swc #o result.swc #l 2 #s 0 #m 2000 #t 0.25 #r 0 #d 0 #f 1\"\n");
}
