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
#include "cmath"  /* for std::abs(double) */



NeuronTree color_lost_branch(NeuronTree new_tree){
    QList <int> components = get_components(new_tree);
//    v3d_msg("components labeled");
    int soma = get_soma(new_tree);
//    v3d_msg("Soma found");
    int soma_component = components.at(soma);

    // Put the soma_connected component as 1st.
    int ncomponents = components.toSet().size();
    QList<NeuronSWC> listneuron = get_ith_component(new_tree, components, soma_component).listNeuron;

    // Color unconnected branches
    for(int i=0; i<ncomponents; i++){
        if(i==soma_component){continue;}
        QList<NeuronSWC> ith_list = get_ith_component(new_tree, components, i).listNeuron;
        for(int j=0; j<ith_list.size(); j++){
            ith_list[j].type = 7;
        }
        listneuron.append(ith_list);
    }

    // Sort
    new_tree.deepCopy(neuronlist_2_neurontree(listneuron));
    soma = get_soma(new_tree);
    int soma_id = new_tree.listNeuron.at(soma).n;
    new_tree.deepCopy(my_SortSWC(new_tree, soma_id, 0));
    return new_tree;
}
//QList<NeuronSWC> get_soma_from_APO(QList<CellAPO> markers){
//    QList<NeuronSWC> S_list;
//    NeuronSWC S;
//    int soma_rootid=1;
//    if(markers.size()>0)
//    {
//        if(markers.size()==1)
//        {
//            //markers.at(0).operator XYZ;
//            S.x = markers.at(0).x;
//            S.y = markers.at(0).y;
//            S.z = markers.at(0).z;
//            S.r = 100;
//            S.type = 1;
//            S.n = soma_rootid;
//            S.pn = -1;
//            S_list.append(S);
//            qDebug()<< "Marker found. Using it as root."; //<< neuron.size()<< neuron.at(neuron.size()-1).pn;
//        }
//        else{
//            for(int i=0;i<markers.size();i++)
//            {
//                if(markers.at(i).comment == "soma")//markers.at(i).color.r == 0 && markers.at(i).color.g == 0 && markers.at(i).color.b == 255)
//                {
//                    //markers.at(0).operator XYZ;
//                    S.x = markers.at(i).x;
//                    S.y = markers.at(i).y;
//                    S.z = markers.at(i).z;
//                    S.type = 1;
//                    S.r = 100;
//                    S.n = soma_rootid;
//                    S.pn = -1;
//                    S_list.append(S);
//                    break;
//                }
//            }
//            qDebug()<< "Warning: more than one marker in the file, taking one commented as 'soma'";
//        }
//    }
//    return S_list;
//}
NeuronTree connect_soma(NeuronTree nt, QList<CellAPO> markers, double dThres, QString outfileLabel,
                        double drop_thres=1e6, bool colorful=true, bool return_maintree=false)  // Adapted from /home/penglab/Desktop/vaa3d/vaa3d_tools/released_plugins/v3d_plugins/connect_neuron_fragments_extractor/neuron_extractor_plugin.cpp
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
    QList<int> components = get_components(nt); // revise this!
    QList<int> ucomponents = components.toSet().toList();
    if(ucomponents.lastIndexOf(-1)>=0){
        return empty_tree;
    }
    int ncomponents=components.toSet().size();

    // Add color to components
    if(colorful){
        nt = color_components(nt, components);
    }
    printf("\tTotal # components:\t%d\n", ncomponents);

    // Connect components to soma
    printf("\tConnecting components to soma\n");
    //check if marker exists and use it as root
    QList<NeuronSWC> S_list = get_soma_from_APO(markers);
    if(S_list.size()==0){
        printf("\tNo soma marker found!\n");
        return empty_tree;
    }
//    markers.clear();
//    markers.append(get_marker(S_list.at(0), 1, 0, 0, 0));

    // For each subtree, connect the closest end (tip or root) to soma, if within certain distance.
    QList<int> drop_list;
    NeuronTree new_tree;
    QList<NeuronSWC> listneuron;
    int soma_rootid=1;
    S_list[0].n = soma_rootid;
    S_list[0].r = 1;
    NeuronSWC S = S_list.at(0);
    listneuron.append(S);
    for(int cid=0; cid<ncomponents; cid++){

        cout << "\t\tComponent\t" << cid <<endl;
        NeuronTree component_i = get_ith_component(nt, components, cid);
        cout << "\t\t# Nodes:\t" << component_i.listNeuron.size() <<endl;
        QList<int> tip_list = get_tips(component_i, true);
        qDebug()<<"Tips found.";
//        v3d_msg("Tips found");

        // Find closet ends
        QList<double> edist; // distance from end to soma
        QList<int> eid; // end id
        for(int i=0; i<tip_list.size();i++){
            edist.append(computeDist2(component_i.listNeuron.at(tip_list.at(i)), S, XSCALE, YSCALE, ZSCALE));
            eid.append(tip_list.at(i));
        }

        QList<double> dsorted = edist;
        qSort(dsorted);
        qDebug()<<"Closest tip found.";
//        v3d_msg("Closet tip found");

        // Whether to connect subtrees to soma
        int cend = eid.at(edist.indexOf(dsorted.at(0))); // end to be connected to soma
        NeuronTree component_i_sorted;
//        export_listNeuron_2swc(component_i.listNeuron, "Z:/Peng/Preprocess_0102/input_of_sortSWC.swc");
//        if(cid==0){
//            export_listNeuron_2swc(component_i.listNeuron, "Z:\Peng\Preprocess_0102\input_of_sortSWC.swc");
//        }
//        component_i_sorted.deepCopy(my_SortSWC(component_i, cend, 0));
//        component_i_sorted.deepCopy(my_SortSWC(component_i, 1, 0));
//        v3d_msg(QString("sort component %1").arg(cid));
        component_i_sorted.deepCopy(my_SortSWC(component_i, component_i.listNeuron.at(cend).n, 0));
        int new_cend = listneuron.size();
        qDebug()<<"Component sorted."<<endl;
//        v3d_msg("Component sorted");

        // Case 1: subtree close to soma
        listneuron = neuronlist_cat(listneuron, component_i_sorted.listNeuron);
        if(dsorted.at(0) < dThres){
            cout<<"\t\tOperation: Connected to soma.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
//            markers.append(get_marker(component_i_sorted.listNeuron.at(0), 50, 255, 255, 255));
            listneuron[new_cend].pn = soma_rootid;
        }
        // Case 2: subtree far from soma
        else{
            cout<<"\t\tOperation: kept but not connected.\t"<<"Distance:\t"<<dsorted.at(0)<<endl;
            listneuron[new_cend].pn = -1;
        }
    }
    new_tree.deepCopy(neuronlist_2_neurontree(listneuron));
//    writeAPO_file(outfileLabel+QString(".apo"), markers);
//    v3d_msg("Done soma connection");
    return new_tree;
}

QList<bool> whether_internal(NeuronTree nt){
    QList<bool> is_internal;
    QList<int> internal_list = get_tips(nt, 1);
    for(int i=0; i<nt.listNeuron.size(); i++){
        is_internal.append(1);
    }
    for(int i=0; i<internal_list.size(); i++){
        is_internal[internal_list.at(i)] = 0;
    }
    return is_internal;
}
NeuronTree label_connections(NeuronTree A, NeuronTree B, QString APO_file){

    printf("welcome to use label cennections\n");
    // Note: A and B should be a one-to-one match;
    // Otherwise the function will exit without any editing
    // Map nodes of A and B
    qDebug()<<"\tchecking one-to-one match";
    QHash<int, int> AtoB;
    for(int i=0; i<A.listNeuron.size(); i++){
        int ct = 0;
        NeuronSWC NA = A.listNeuron.at(i);
        for(int j=0; j<B.listNeuron.size(); j++){
            NeuronSWC NB = B.listNeuron.at(j);
            if(isEqual(NA.x, NB.x) & isEqual(NA.y, NB.y) & isEqual(NA.z, NB.z))
            {
                AtoB.insert(i, j);
                ct++;
                break;
            }
        }
        if(ct == 0){
            qDebug()<<"A"<<"Nodes of neuron trees not one-to-one match!"<<i<<A.listNeuron.at(i).x<<A.listNeuron.at(i).y<<A.listNeuron.at(i).z;
            return A;
        }
    }
    QHash<int, int> BtoA;
    for(int i=0; i<B.listNeuron.size(); i++){
        int ct = 0;
        NeuronSWC NB = B.listNeuron.at(i);
        for(int j=0; j<A.listNeuron.size(); j++){
            NeuronSWC NA = A.listNeuron.at(j);
            if(isEqual(NA.x, NB.x) & isEqual(NA.y, NB.y) & isEqual(NA.z, NB.z))
            {
                BtoA.insert(i, j);
                ct++;
                break;
            }
        }
        if(ct == 0){
            qDebug()<<"B"<<"Nodes of neuron trees not one-to-one match!"<<i<<B.listNeuron.at(i).x<<B.listNeuron.at(i).y<<B.listNeuron.at(i).z;
            return A;
        }
    }
    qDebug()<<"\tchecking one-to-one match done\n";

    // Find out changed nodes;
    qDebug()<<"\tfinding node changes";
    QList<bool> is_internal_A = whether_internal(A);
    QList<bool> is_internal_B = whether_internal(B);
    QList<int> change_type; // 0: no change; 1: tip to internal (lost connection of A); 2: internal to tip (extra connection of A)
    for(int i=0; i<A.listNeuron.size(); i++){
        bool ia = is_internal_A.at(i);
        bool ib = is_internal_B.at(AtoB.value(i));
        if(ia == ib){
            change_type.append(0);
        }
        else if(!ia & ib){
            change_type.append(1);
        }
        else{
            change_type.append(2);
        }
    }
    qDebug()<<"\tfinding node changes done\n";

    // Find out changed connections
    qDebug()<<"\tfinding edge changes";
    QList<CellAPO> markers;
    // Change color of related nodes
    // If no change, label as black
    for(int i=0; i<A.listNeuron.size(); i++){
        A.listNeuron[i].type = 1;
    }
    // Label extra edges of A as red
    for(int i=0; i<A.listNeuron.size(); i++){
        if(A.listNeuron.at(i).pn == -1){continue;}
        int child_A = i;
        int parent_A = A.hashNeuron.value(A.listNeuron.at(i).pn);
        int child_B = AtoB.value(child_A);
        int parent_B = AtoB.value(parent_A);
        if((B.listNeuron.at(child_B).pn == B.listNeuron.at(parent_B).n) || // Continue if there's an edge between the two nodes in B
                (B.listNeuron.at(child_B).n == B.listNeuron.at(parent_B).pn)){
            continue;
        }
        if( (change_type.at(child_A)==0) && (change_type.at(parent_A))==0){ // At least one node of an edge needs to be changed
            continue;
        }
        A.listNeuron[child_A].type = 2;
        A.listNeuron[parent_A].type = 2;
        markers.append(get_marker(A.listNeuron.at(child_A), 10, 255, 0, 0));
        markers.append(get_marker(A.listNeuron.at(parent_A), 10, 255, 0, 0));
    }
    // Label extra edges of B as blue
    for(int i=0; i<B.listNeuron.size(); i++){
        if(B.listNeuron.at(i).pn == -1){continue;}
        int child_B = i;
        int parent_B = B.hashNeuron.value(B.listNeuron.at(i).pn);
        int child_A = AtoB.key(child_B);
        int parent_A = AtoB.key(parent_B);
        if((A.listNeuron.at(child_A).pn == A.listNeuron.at(parent_A).n) || // Continue if there's an edge between the two nodes in B
                (A.listNeuron.at(child_A).n == A.listNeuron.at(parent_A).pn)){
            continue;
        }
        if( (change_type.at(child_A)==0) && (change_type.at(parent_A))==0){ // At least one node of an edge needs to be changed
            continue;
        }
        A.listNeuron[child_A].type = 3;
        A.listNeuron[parent_A].type = 3;
        markers.append(get_marker(A.listNeuron.at(child_A), 10, 0, 0, 255));
        markers.append(get_marker(A.listNeuron.at(parent_A), 10, 0, 0, 255));
    }
    qDebug()<<"\tfinding edge changes done\n";
    writeAPO_file(APO_file, markers);
    return A;
}

QList<NeuronSWC> get_new_edge(NeuronTree nt, NeuronTree cur_nt, QString APO_file, int new_type=2){
    cur_nt = label_connections(nt, cur_nt, APO_file);
    QList<NeuronSWC> new_connection;
    for(int i=0; i<cur_nt.listNeuron.size(); i++){
        NeuronSWC node = cur_nt.listNeuron.at(i);
        if(node.type==2){
            node.type = new_type;
            new_connection.append(node);
        }
    }
    NeuronTree new_tree;
    if(new_connection.size()>0){
        new_tree.deepCopy(neuronlist_2_neurontree(new_connection));
        new_tree = my_SortSWC(new_tree, VOID, 0);
    }
    return new_tree.listNeuron;
}
QList<CellAPO> get_new_marker(QString APO_file, int r, int g, int b){
    QList<CellAPO> markers = readAPO_file(APO_file);
    for(int i=0; i<markers.size(); i++){
        markers[i].color.r=r;
        markers[i].color.g=g;
        markers[i].color.b=b;
    }
    return markers;
}

bool getSomaFromProcessedSWC(const NeuronTree& inputTree, CellAPO& somaMarker)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.constBegin(); it != inputTree.listNeuron.constEnd(); ++it)
	{
		if (it->type == 1)
		{
			RGBA8 somaColor;
			somaColor.r = 0;
			somaColor.g = 0;
			somaColor.b = 0;
			somaMarker.color = somaColor;
			somaMarker.x = it->x;
			somaMarker.y = it->y;
			somaMarker.z = it->z;
			somaMarker.volsize = 500;
			return true;
		}
	}
	return false;
}

bool getSomaFromProcessedAPO(const QList<CellAPO>& inputAPOs, CellAPO& somaMarker)
{
	set<vector<float>> somaConnPoints;
	for (QList<CellAPO>::const_iterator it = inputAPOs.constBegin(); it != inputAPOs.constEnd(); ++it)
	{
		if (it->color.r == 0 || it->color.g == 255 || it->color.b == 0)
		{
			vector<float> connPoint;
			connPoint.push_back(it->x);
			connPoint.push_back(it->y);
			connPoint.push_back(it->z);
			somaConnPoints.insert(connPoint);
		}
	}

	float somaCoord[3];
	integratedDataStructures::ChebyshevCenter(somaConnPoints, somaCoord); 
	RGBA8 somaColor;
	somaColor.r = 0;
	somaColor.g = 0;
	somaColor.b = 0;
	somaMarker.color = somaColor;
	somaMarker.x = somaCoord[0];
	somaMarker.y = somaCoord[1];
	somaMarker.z = somaCoord[2];
	somaMarker.volsize = 500;
}

bool pre_processing(QString qs_input, QString qs_output, double prune_size, double thres, double thres_long,
                    double step_size, double connect_soma_dist, bool rotation,
                    bool colorful, bool return_maintree, bool return_temp)
{
    printf("welcome to use pre_processing!\n");

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
    QList<NeuronSWC> new_connection;
    QList<NeuronSWC> tp_new_connection;
    NeuronTree tp_new_tree;
    QList<CellAPO> markers;
    QList<CellAPO> tp_markers;
    if (qs_input.endsWith("swc") || qs_input.endsWith("SWC"))
    {
        nt = readSWC_file(qs_input);
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].r = 1;
        }
    }

    //2. start processing
    NeuronTree cur_nt;

//    NeuronTree test_nt;
//    QList<int> test_newroot;
//    test_newroot << 1 << 1000 << 3000 << 3005 << 10000 << 30000;
//    for(int i=0; i<test_newroot.size(); i++){
//        int newroot_name = nt.listNeuron.at(test_newroot.at(i)).n;
//        qDebug()<<"Trial newroot name:"<<newroot_name;
//        test_nt.deepCopy(my_SortSWC(nt, newroot_name, 0));
//        writeSWC_file("C:/Users/pengx/Desktop/test/tmp_"+QString::number(newroot_name)+".swc",test_nt);
//    }
//    for(int i=0; i<nt.listNeuron.size(); i++){
//        int newroot_name = nt.listNeuron.at(i).n;
//        qDebug()<<"Trial newroot name:"<<newroot_name;
//        test_nt.deepCopy(my_SortSWC(nt, newroot_name, 0));
//    }
//    return 1;

    //2.0 Remove duplicates
    printf("\tRemoving duplicates\n");
    nt.deepCopy(my_SortSWC(nt, VOID, 0));
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".dedup.swc"));
    }

    //2.1 Resample
    printf("\tResampling\n");
    if (step_size>0){
        printf("Resampling along segments\n");
        nt.deepCopy(resample(nt, step_size));
    }else{
        printf("Skip Resampling\n");
    }
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".resample.swc"));
    }

    //2.2 Prune
    printf("\tPruning short branches\n");
    if (!prune_branch(nt, cur_nt, prune_size))
    {
        fprintf(stderr,"Error in prune_short_branch.\n");
        return 1;
    }
    nt.deepCopy(cur_nt);
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".prune.swc"));
    }

    //2.3 Short distance connection
    cur_nt.deepCopy(nt);
    printf("\tShort distance connection\n");

    nt.deepCopy(my_connectall(nt, XSCALE, YSCALE, ZSCALE, 60, thres, 0, false, 1));
    // Keep track of new_edges
    new_connection = neuronlist_cat(new_connection, get_new_edge(nt, cur_nt, infileLabel+".short_connection.apo", 5));
    // Keep track of nodes
    markers.append(get_new_marker(infileLabel+".short_connection.apo", 255,0,0));
    qDebug()<<count_root(cur_nt)<<get_new_marker(infileLabel+".short_connection.apo", 0,0,0).size()/2<<count_root(nt);
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".short_connection.swc"));
    }

    //2.4 Connect to soma
    bool connect_soma_performed = 1;
    if ((connect_soma_dist>0) && (fexists(infileLabel + QString(".apo")))){
        printf("\tConnecting to soma\n");
        QList<CellAPO> soma_markers = readAPO_file(infileLabel + QString(".apo"));
        QList<NeuronSWC> S_list = get_soma_from_APO(soma_markers);
        if(S_list.isEmpty()){
            v3d_msg(QString("APO file is given but don't know which is soma.\n"
                            "Soma connection is skiped.\n"
                            "Please double check %1").arg(infileLabel + QString(".apo")));
            connect_soma_performed = 0;
        }
        else{
            S_list[0].r = 1;
            cur_nt.deepCopy(neuronlist_2_neurontree(neuronlist_cat(S_list, nt.listNeuron)));
            nt.deepCopy(connect_soma(nt, soma_markers, connect_soma_dist, infileLabel+".soma_connection", 1e6, colorful, false));  // 10-10-2018: return_maintree has been moved to after long_connection
            // Keep track of new_edges
            new_connection = neuronlist_cat(new_connection, get_new_edge(nt, cur_nt, infileLabel+".soma_connection.apo", 6));
            // Keep track of nodes
            markers.append(get_new_marker(infileLabel+".soma_connection.apo", 0,255,0));
            qDebug()<<count_root(cur_nt)<<get_new_marker(infileLabel+".soma_connection.apo", 0,0,0).size()/2<<count_root(nt);
        }
    }
    else{
        // If the input swc is already connected to soma, then the apo file is not required
        // and one can skip the soma connecting step.
        printf("\tSkip connecting to soma\n");
        connect_soma_performed=0;
    }
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".soma_connection.swc"));
    }


    // 2.5 Long connection
    printf("\tLong distance connection\n");
    cur_nt.deepCopy(nt);
    nt.deepCopy(my_connectall(nt, XSCALE, YSCALE, ZSCALE, 60, thres_long, 1, false, 1));
    // Keep track of new_edges
    new_connection = neuronlist_cat(new_connection, get_new_edge(nt, cur_nt, infileLabel+".long_connection.apo", 7));
    // Keep track of nodes
    markers.append(get_new_marker(infileLabel+".long_connection.apo", 0,0,255));
    qDebug()<<count_root(cur_nt)<<get_new_marker(infileLabel+".long_connection.apo", 0,0,0).size()/2<<count_root(nt);
//    v3d_msg("color lost branch");
    if(connect_soma_performed){
        nt.deepCopy(color_lost_branch(nt));
    }
//    v3d_msg("lost branch colored");

    if(return_maintree){
        int soma = 0;
        nt.deepCopy(single_tree(nt, soma));
    }
    if(return_temp)
    {
        export_listNeuron_2swc(nt.listNeuron, qPrintable(outfileLabel+".long_connection.swc"));
    }

    //2.6 Align axis
    cur_nt.listNeuron = nt.listNeuron;
    if (rotation)
    {
        printf("\tAligning PCA axis\n");
        nt = align_axis(cur_nt);
    }
    else{
        printf("\tSkip PCA alignment\n");
    }

    if (export_listNeuron_2swc(nt.listNeuron,qPrintable(qs_output))){
        printf("\t %s has been generated successfully.\n",qPrintable(qs_output));
    }

    for(int i=0;i<markers.size(); i++){
        markers[i].volsize=500;
    }

#ifndef _YUN_FINAL_RELEASE_
    writeAPO_file(outfileLabel+".apo", markers);
    my_saveANO(outfileLabel, true, true, qs_output);
#else
	CellAPO somaMarker;
	if (!getSomaFromProcessedSWC(nt, somaMarker))
		getSomaFromProcessedAPO(markers, somaMarker);
	QList<CellAPO> finalAPOs;
	finalAPOs.push_back(somaMarker);
	writeAPO_file(outfileLabel + ".apo", finalAPOs);
#endif

    if(return_temp)
    {
        if (export_listNeuron_2swc(new_connection,qPrintable(outfileLabel+".new_connection.swc"))){
            printf("\t %s has been generated successfully.\n",qPrintable(outfileLabel+".new_connection.swc"));
        }
    }

    if(!return_temp){
        remove(qPrintable(infileLabel+".short_connection.apo"));
        remove(qPrintable(infileLabel+".soma_connection.apo"));
        remove(qPrintable(infileLabel+".long_connection.apo"));
    }


    return 1;
}

bool pre_processing_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output)
{

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
    double connect_soma_dist = 20;
    double thres = 0.5;
    double thres_long = 10;
    bool rotation = false;
    bool colorful = false;
    bool return_maintree = false;
	
	int c;
    static char optstring[]="hi:o:l:s:m:t:r:d:f:z:";
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
                break;
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
        case 'z':
            if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
            {
                fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
                return 1;
            }
            thres_long = atof(optarg);
            if (thres_long<-1)
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
    QString qs_output;
    if(dfile_result != NULL){
        qs_output = QString(dfile_result);
    }
    else{
        qs_output = QString((qPrintable(qs_input)));
        if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
            qs_output = qs_output.left(qs_output.length() - 4) + QString(".processed.swc");
        }
        if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
            qs_output = qs_output.left(qs_output.length() - 5) + QString(".processed.eswc");
        }
    }

    // Pre-process
    pre_processing(qs_input, qs_output, prune_size, thres, thres_long, step_size, connect_soma_dist, rotation, colorful, return_maintree);
    return 1;
}

bool pre_processing_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{

    double prune_size = 2; //default case
    double step_size = 0;
    double connect_soma_dist = 20;
    double thres = 0.5;
    double thres_long = 10;
    bool rotation = false;
    bool colorful = false;
    bool return_maintree = false;
    bool return_temp = false;


    //choose a directory that contain swc files
    QString qs_input;
    qs_input=QFileDialog::getOpenFileName(
                parent,
                "Please select input file (*.swc)\n",
                QDir::currentPath(),
                "All files (*.*)" ";; swc files (*.swc *.eswc)"
                );
    QString qs_output(qPrintable(qs_input));
    if (qs_output.endsWith(".swc") || qs_output.endsWith(".SWC")){
        qs_output = qs_output.left(qs_output.length() - 4) + QString(".processed.swc");
    }
    else if (qs_output.endsWith(".eswc") || qs_output.endsWith(".ESWC")){
        qs_output = qs_output.left(qs_output.length() - 5) + QString(".processed.eswc");
    }
    else{return 0;}

    // Pre-process
    pre_processing(qs_input, qs_output, prune_size, thres, thres_long, step_size, connect_soma_dist, rotation, colorful, return_maintree);

    return 1;
}

void printHelp_pre_processing()
{
    printf("\nVaa3D plugin: modifiled pre-processing step analysis/QC, including: \n");
    printf("\t1) delete the branches in a neuron which have a length smaller the prune size.\n");
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
    printf("\t#t <thres>       :    gap threshold before soma connection.\n");
    printf("\t                        if not specified, use 2.\n");
    printf("\t#z <thres_long>       :   gap threshold after soma connection.\n");
    printf("\t                        if not specified, use 10.\n");
    printf("\t#r <rotation = 0>   :   whether to perform PCA alignment.\n");
    printf("\t                         if not specified, rotation is not performed\n");
    printf("\t#d <label_subtree = 0>   :   whether to give each subtree a different color.\n");
    printf("\t                         if not specified, labeling is not performed\n");
    printf("\t#f <return_maintree = 0>   :   whether to return only the maintree (a single tree connected to soma).\n");
    printf("\t                         if not specified, all trees will be reported\n");
    printf("Usage: vaa3d -x preprocess -f preprocess -p \"#i input.swc #o result.swc #l 2 #s 0 #m 70 #t 5 #r 0 #d 0 #f 0\"\n");
}
