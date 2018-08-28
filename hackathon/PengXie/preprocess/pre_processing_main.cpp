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

#include "pre_processing_main.h"

NeuronTree connect_soma(NeuronTree nt, QList<CellAPO> markers, double dThres, QString outfileLabel, double drop_thres=1e6, bool colorful=true, bool return_maintree=false)  // Adapted from /home/penglab/Desktop/vaa3d/vaa3d_tools/released_plugins/v3d_plugins/connect_neuron_fragments_extractor/neuron_extractor_plugin.cpp
{
    printf("Running connect_soma\n");
    QList<int> pList;
    QList<int> nList;
    NeuronTree empty_tree;
    const int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        NeuronSWC node = nt.listNeuron.at(i);
        pList.append(node.pn);
        nList.append(node.n);
    }

    //connected component
    QList<int> components = get_components(nt);
    int ncomponents=components.toSet().size();

    // Add color to components
    if(colorful){
        color_components(nt, components);
    }
    printf("\tTotal # components:\t%d\n", ncomponents);

    // Connect components to soma
    printf("\tConnecting components to soma\n");
    //check if marker exists and use it as root
    QList<NeuronSWC> S_list;
    NeuronSWC S;
    int soma_rootid=0;
    if(markers.size()>0)
    {
        if(markers.size()==1)
        {
            //markers.at(0).operator XYZ;
            S.x = markers.at(0).x;
            S.y = markers.at(0).y;
            S.z = markers.at(0).z;
            S.r = 100;
            S.type = 1;
            S.n = soma_rootid;
            S.pn = -1;
            S_list.append(S);
            qDebug()<< "Marker found. Using it as root."; //<< neuron.size()<< neuron.at(neuron.size()-1).pn;
        }
        else{
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
                    S.n = soma_rootid;
                    S.pn = -1;
                    S_list.append(S);
                    break;
                }
            }
            qDebug()<< "Warning: more than one marker in the file, taking one commented as 'soma'";
        }
        markers[0].color.r=0;
        markers[0].color.g=0;
        markers[0].color.b=0;
        markers[0].comment="soma";
    }
    if(S_list.size()==0){
        printf("\tNo soma marker found!\n");
        return empty_tree;
    }


    // For each subtree, connect the closest end (tip or root) to soma, if within certain distance.
    QList<int> drop_list;
    int bad_nodes=0;
    NeuronTree new_tree;
    new_tree.listNeuron.append(S);
    for(int cid=0; cid<ncomponents; cid++){

        cout << "\t\tComponent\t" << cid <<endl;
        NeuronTree component_i = get_ith_component(nt, components, cid);
        cout << "\t\t# Nodes:\t" << component_i.listNeuron.size() <<endl;
        QList<int> tip_list = get_tips(component_i, true);

        // Find closet ends
        QList<double> edist; // distance from end to soma
        QList<int> eid; // end id
        for(int i=0; i<tip_list.size();i++){
            edist.append(computeDist2(component_i.listNeuron.at(tip_list.at(i)), S));
            eid.append(tip_list.at(i));
        }

        QList<double> dsorted = edist;
        qSort(dsorted);

        // Whether to connect subtrees to soma
        int cend = eid.at(edist.indexOf(dsorted.at(0))); // end to be connected to soma
        NeuronTree component_i_sorted;
        SortSWC(component_i.listNeuron, component_i_sorted.listNeuron, component_i.listNeuron.at(cend).n, 0);
        int new_cend = new_tree.listNeuron.size();

        // Case 1: subtree close to soma
        if(dsorted.at(0) < dThres){
            cout<<"\t\tOperation: Connected to soma.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
            markers.append(get_marker(component_i_sorted.listNeuron.at(0), 50, 255, 255, 255));
            new_tree = neuron_cat(new_tree, component_i_sorted);
            new_tree.listNeuron[new_cend].pn = soma_rootid;
        }
        // Case 2: subtree far from soma
        else{
            bad_nodes += component_i_sorted.listNeuron.size();
             // Label un-connected subtrees
            if(!return_maintree){
                cout<<"\t\tOperation: kept but not connected.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
                markers.append(get_marker(component_i_sorted.listNeuron.at(0), 50, 255, 0, 0));
                // Color this subtree as white
                for(int i=0; i<component_i_sorted.listNeuron.size(); i++){
                    component_i_sorted.listNeuron[i].type=0;
                }
                new_tree = neuron_cat(new_tree, component_i_sorted);
                new_tree.listNeuron[new_cend].pn = -1;
            }
            else{
                cout<<"\t\tOperation: Deleted!\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
            }
        }
    }

    writeAPO_file(outfileLabel+QString(".apo"), markers);
    printf("Percentage_lost\t%f%%\n", bad_nodes*100.0/N);

    return new_tree;
}

bool pre_processing(QString qs_input, QString qs_output, double prune_size = 2, double thres = 0,
                    double step_size = 0, double connect_soma_dist = 2000, bool rotation = false,
                    bool colorful = false, bool return_maintree = false)
{

    printf("welcome to use pre_processiing!\n");

    QString outfileLabel;
    QString infileLabel;
    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){
        infileLabel = qs_input.left(qs_input.length() - 4);
    }
    if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){
        infileLabel = qs_input.left(qs_input.length() - 5);
    }
    if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
        outfileLabel = qs_output.left(qs_output.length() - 4);
    }
    if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
        outfileLabel = qs_output.left(qs_output.length() - 5);
    }

    // 1. Load data
    printf("\tLoading swc\n");

    NeuronTree nt;
    QList<CellAPO> markers;

    if (qs_input.endsWith("swc") || qs_input.endsWith("SWC"))
    {
        nt = readSWC_file(qs_input);
    }

    //2. start processing

    //2.1 Prune
    printf("\tPruning short branches\n");
    NeuronTree pruned;

    if (!prune_branch(nt, pruned, prune_size))
    {
        fprintf(stderr,"Error in prune_short_branch.\n");
        return 1;
    }

    //2.2 Remove duplicates
    printf("\tRemoving duplicates\n");
    // Maybe not the best solution. Use it for now.
    NeuronTree deduped = my_SortSWC(pruned, VOID, 0);;
//    NeuronTree deduped = readSWC_file("temp.swc");

    //2.3 Connect as a break points
    printf("\tConnecting breaks\n");
    NeuronTree connected;
    double angthr=60;
    angthr = cos((180-angthr)/180*M_PI);
    connectall(&deduped, connected.listNeuron, 1, 1, 5, angthr, thres, 1, true, -1);

    //2.3 Resample
    printf("\tResampling\n");
    NeuronTree resampled = connected;
    if (step_size>0){
        printf("Resampling along segments\n");
        resampled = resample(connected, step_size);
    }else{
        printf("Skip Resampling\n");
        resampled=connected;
    }

    //2.4 Connect to soma
    printf("\tConnecting to soma\n");
    NeuronTree soma_connected;
    if (connect_soma_dist>0){
        markers = readAPO_file(infileLabel + QString(".apo"));
        soma_connected = connect_soma(resampled, markers, connect_soma_dist, outfileLabel, 1e6, colorful, return_maintree);
    }
    double percentage_lost = 100 - soma_connected.listNeuron.size()*100.0 / resampled.listNeuron.size();
    if(percentage_lost<0){
        percentage_lost = 0;
    }

    //2.5 Sort the tree and remove duplicates.
    printf("\tSorting\n");
    NeuronTree sorted;
    SortSWC(soma_connected.listNeuron, sorted.listNeuron, 0, 0);

    //2.6 Align axis
    NeuronTree result;
    if (rotation)
    {
        printf("\tAligning PCA axis\n");
        result = align_axis(sorted);
    }
    else{
        printf("\tSkip PCA alignment\n");
        result = sorted;
    }

    if (export_listNeuron_2swc(result.listNeuron,qPrintable(qs_output))){
        printf("\t %s has been generated successfully.\n",qPrintable(qs_output));
    }
    bool print_apo = connect_soma_dist>0;
    my_saveANO(outfileLabel, true, print_apo);


    // QC report
    FILE * fp=0;
    fp = fopen((char *)qPrintable(outfileLabel+QString(".QC.txt")), "wt");
    // QC.1
    fprintf(fp, "Percentage_lost(%%)\t%f\n", percentage_lost);
    // QC.2
    QList<int> components = get_components(sorted);
    int ncomponents=components.toSet().size();
    fprintf(fp, "Number_single_trees\t%d\n", ncomponents);
    // QC.3
    if(check_duplicate(result)){
        fprintf(fp, "Exist_duplicate_nodes\t%d\n", 0);
    }
    else{
        fprintf(fp, "Exist_duplicate_nodes\t%d\n", 1);
    }

    fclose(fp);

    return 1;
}

bool split_neuron(QString qs_input){
    // Split neurons into different components
    QString qs_tag;
    if (qs_input.endsWith(".swc") || qs_input.endsWith(".SWC")){qs_tag = qs_input.left(qs_input.length()-4);}
    if (qs_input.endsWith(".eswc") || qs_input.endsWith(".ESWC")){qs_tag = qs_input.left(qs_input.length()-5);}

    // Report 1: long axon only.
    neurite_analysis(qs_input, qs_tag+".long_axon.swc", "l");
    // Report 2: other axons retyped
    neurite_analysis(qs_input, qs_tag+".axon.swc", "a");
    axon_retype(qs_tag+".axon.swc", qs_tag+".long_axon.swc", qs_tag+".axon.retype.swc");
//    QList<double> lpa_density = branch_distribution(qs_tag+".axon.swc", qs_tag+".long_axon.swc");
//    export_branch_distribution(lpa_density, qs_tag+".axon_density.txt");
    // Report 3: dendrite
    neurite_analysis(qs_input, qs_tag+".dendrite.swc", "d");

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
    double connect_soma_dist = 2000;
    double thres = 10;
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

    QString qs_input = QString(dfile_input);
    QString qs_output = QString(dfile_result);
    // Pre-process
    pre_processing(qs_input, qs_output, prune_size, thres, step_size, connect_soma_dist, rotation, colorful, return_maintree);
    // Split neuron
    qs_input = QString(qPrintable(qs_output));
    split_neuron(qs_input);
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


    // Pre-process
    pre_processing(qs_input, qs_output, prune_size, thres, step_size, connect_soma_dist, rotation, colorful, return_maintree);
    // Split neuron
    qs_input = QString(qPrintable(qs_output));
    split_neuron(qs_input);

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
    printf("\t                         if not specified, use 2\"\n");
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
