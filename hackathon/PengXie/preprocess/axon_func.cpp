//functions for axon analysis
//by Peng Xie
//2018-08-16

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "axon_func.h"
#include "crop_swc_main.h"

#define MAXSIZE 1000000

QList<int> match_axon_and_lpa(NeuronTree axon, NeuronTree lpa){

    QList<int> lpa_id;

    // 1.1 Load whole axon data
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }

    // 2 Check whether match
    bool matched = 1; // Whether the id's of axon and lpa matches
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node=lpa.listNeuron.at(i);
        int i_in_axon = nlist.indexOf(node.n);
        if(i_in_axon == -1){  // lpa id doesn't exsit in axon: break the loop.
            matched = 0;
            break;
        }
        NeuronSWC node_axon = axon.listNeuron.at(i_in_axon);  // The matching node in axon
        if(node.x != node_axon.x || node.y != node_axon.y || node.z != node_axon.z){
            matched = 0;
            break;
        }
        lpa_id.append(node.n);
    }

    if(matched){
        return lpa_id;
    }
    else{
        printf("ID's of axon and lpa do not match!\n");
        int soma = get_soma(axon);
        lpa_id.clear();
        QList<int> lpa = find_long_axon(axon, soma);
        for(int i=0; i<lpa.size(); i++){
            lpa_id.append(axon.listNeuron.at(lpa.at(i)).n);
        }
        return lpa_id;
    }
}
QList <int> find_long_axon(NeuronTree nt, int soma){

    printf("welcome to use find_long_axon\n");
    const int N = nt.listNeuron.size();
    QList<int> name_list;
    for(int i=0; i<N; i++){
        name_list.append(nt.listNeuron.at(i).n);
    }

//    list<int> children[MAXSIZE];
//    for(int i=0; i<N; i++){
//        NeuronSWC node = nt.listNeuron.at(i);
//        int pid = name_list.lastIndexOf(node.pn);
//        if(pid<0){
//            continue;
//        }
//        children[pid].push_back(i);
//    }
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = name_list.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }


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
//        for(int i=0; i<nt.listNeuron.size();i++){  // This loop can be more efficient, improve it later!
        for(V3DLONG *i=children[pid].begin(); i!=children[pid].end(); ++i)
        {
            if((nt.listNeuron.at(*i).pn)==nt.listNeuron.at(pid).n && distance.at(*i)==(-1.0)){
                pstack.push(*i);
                distance[*i]=distance.at(pid)+computeDist2(nt.listNeuron.at(pid), nt.listNeuron.at(*i), 0.2,0.2,1);
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
    // Peng Xie 2019-03-20
    while(nt.listNeuron.at(cur_id).pn!=(-1)){
        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
        lpa.prepend(cur_id);
    }
//    while(nt.listNeuron.at(cur_id).type!=1){
//        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
//        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
//        lpa.prepend(cur_id);
//        if(nt.listNeuron.at(cur_id).pn<0){  // Reached root
//            break;
//        }
//    }
    return lpa;
}
QList<double> arbor_distribution(QString whole_axon_swc, QString lpa_swc){

    printf("Welcome to use arbor_distribution\n");

    // 1. Load data

    // 1.1 Load whole axon data
    NeuronTree axon = readSWC_file(whole_axon_swc);
    QList<int> plist;
    QList<int> nlist;
    const int N = axon.listNeuron.size();
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    // 1.2 Load lpa
    NeuronTree lpa = readSWC_file(lpa_swc);
    // 1.3 Match axon and lpa
    QList<int> lpa_id = match_axon_and_lpa(axon, lpa);
    lpa = get_subtree_by_name(axon, lpa_id);

    // 2. Branch size at every node of lpa
    QList<double> lpa_size;
    for(int i=0; i<lpa.listNeuron.size();i++){lpa_size.append(0);}
    double branch_size;
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = nlist.lastIndexOf(axon.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    for(int i=0; i<axon.listNeuron.size(); i++){
        branch_size=0;
        NeuronSWC node = axon.listNeuron.at(i);
        // find the start of a non-long-projection branch
        if(lpa_id.indexOf(node.n) == (-1) && lpa_id.indexOf(node.pn) != (-1)){
            // Initialization
            int pid=i;
            QList<int> visited;
            for(int j=0;j<axon.listNeuron.size(); j++){visited.append(0);}
            QStack<int> pstack;
            pstack.push(pid);
            bool is_push = false;
            visited[pid]=1;

            // DFS using stack
            while(!pstack.isEmpty()){
                is_push = false;
                pid = pstack.top();
                // whether exist unvisited children of pid
                // if yes, push child to stack;
//                for(int j=0; j<axon.listNeuron.size();j++){  // This loop can be more efficient, improve it later!
                for(V3DLONG *j=children[pid].begin(); j!=children[pid].end(); ++j){
                    if((axon.listNeuron.at(*j).pn)==nlist.at(pid) && visited.at(*j)==0){
                        pstack.push(*j);
                        visited[*j]=1;
//                        branch_size+=1;
                        
                        is_push=true;
                        break;
                    }
                }
                // else, pop pid
                if(!is_push){
                    pstack.pop();
                }
            }

            // Add to the parent node on lpa
            int pid_lpa = lpa_id.indexOf(node.pn);
            if(pid_lpa==-1){cout<<"warning\n";break;}
            lpa_size[pid_lpa] += branch_size;
        }
    }

    cout<<3<<endl;
    // 3. Branch density at every node of lpa
    QList<double> lpa_density;
    for(int i=0; i<lpa.listNeuron.size();i++){
        lpa_density.append(lpa_size.at(i));
//        lpa_density.append(0);
//        for(int j=0; j<lpa.listNeuron.size();i++){
//            lpa_density[i] += lpa_size.at(j) * exp(-pow(i-j, 2));
//        }
    }

    return lpa_density;
}
bool export_branch_distribution(QList<double> lpa_density, QString output_fileName){
    FILE * fp=0;
    fp = fopen((char *)qPrintable(output_fileName), "wt");
    for(int i=0; i<lpa_density.size(); i++){
        fprintf(fp, "%d\t%f\n", i, lpa_density.at(i));
    }
    fclose(fp);
    return 1;
}
bool axon_retype(QString whole_axon_swc, QString lpa_swc, QString output_swc, bool proximal_distal){

    printf("Welcome to use axon_retype\n");

    // 0. File check
    if(!fexists(whole_axon_swc)){
        return 0;
    }
    if (!(whole_axon_swc.endsWith(".swc") || whole_axon_swc.endsWith(".SWC")
          || whole_axon_swc.endsWith(".eswc") || whole_axon_swc.endsWith(".ESWC"))){
        printf("Error: Input file is not SWC.\n");
        return 0;
    }
    if(output_swc.size()==0){
        if (whole_axon_swc.endsWith(".swc") || whole_axon_swc.endsWith(".SWC")){
            output_swc = whole_axon_swc.left(whole_axon_swc.length()-4)+".retype.swc";
        }
        if (whole_axon_swc.endsWith(".eswc") || whole_axon_swc.endsWith(".ESWC")){
            output_swc = whole_axon_swc.left(whole_axon_swc.length()-5)+".retype.swc";
        }
    }
    QString proximal_swc;
    QString distal_swc;
    if (output_swc.endsWith(".swc") || output_swc.endsWith(".SWC")){
        proximal_swc = output_swc.left(output_swc.length()-11)+".proximal_axon.swc";
        distal_swc = output_swc.left(output_swc.length()-11)+".distal_axon.swc";
    }
    else if (whole_axon_swc.endsWith(".eswc") || whole_axon_swc.endsWith(".ESWC")){
        proximal_swc = output_swc.left(output_swc.length()-12)+".proximal_axon.eswc";
        distal_swc = output_swc.left(output_swc.length()-12)+".distal_axon.eswc";
    }

    // 1. Load data
    // 1.1 Load whole axon data
    NeuronTree axon = readSWC_file(whole_axon_swc);
    QList<int> plist;
    QList<int> nlist;
    for(int i=0; i<axon.listNeuron.size();i++){
        NeuronSWC node=axon.listNeuron.at(i);
        plist.append(node.pn);
        nlist.append(node.n);
    }
    crop_swc(whole_axon_swc, proximal_swc, 200, 0, 0, 0, 0,0,0,0,0);

    // 1.2 Load lpa
    NeuronTree lpa = readSWC_file(lpa_swc);
    // 1.3 Match axon and lpa
    QList<int> lpa_id = match_axon_and_lpa(axon, lpa);
    lpa = get_subtree_by_name(axon, lpa_id);

    QList<double> path_distance;
    path_distance.append(0);
    double cur_distance = 0;
    for(int i=1; i<lpa.listNeuron.size(); i++){
        cur_distance += computeDist2(lpa.listNeuron.at(i-1), lpa.listNeuron.at(i), 0.2,0.2,1);
        path_distance.append(cur_distance);
    }
    double total_path_distance = cur_distance;
    qDebug()<<"Long projection axon path distance:\t"<<total_path_distance;

    QList<bool> is_proximal;
    QList<bool> is_distal;
    NeuronTree proximal_axon;
    NeuronTree distal_axon;
    for(int i=0; i<lpa.listNeuron.size();i++){
        NeuronSWC node=lpa.listNeuron.at(i);
        if((path_distance.at(i)/total_path_distance)<0.2){
            is_proximal.append(1);
//            lpa.listNeuron[i].type=0;
            proximal_axon.listNeuron.append(node);
        }
        else{
            is_proximal.append(0);
        }
        if((path_distance.at(i)/total_path_distance)>0.7){
            is_distal.append(1);
//            lpa.listNeuron[i].type=1;
            distal_axon.listNeuron.append(node);
        }
        else{
            is_distal.append(0);
        }
    }
//    export_listNeuron_2swc(lpa.listNeuron, "proximal_distal.swc");

    // 2. label non-long-projection axon braches
    // 2.1 default mode: retype axon clusters as proximal or distal
    int proximal_retype = 5;
    int distal_retype = 6;
    // 2.2 optional model: label branch with a different type if passes size_thres.
    int cur_retype = 2;
    int size_thres = 10;
    QList<int> branch_ind;
    const int N=axon.listNeuron.size();
//    list<int> children[MAXSIZE];
//    for(int i=0; i<N; i++){
//        NeuronSWC node = axon.listNeuron.at(i);
//        int pid = nlist.lastIndexOf(node.pn);
//        if(pid<0){continue;}
//        children[pid].push_back(i);
//    }
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = nlist.lastIndexOf(axon.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    for(int i=1; i<axon.listNeuron.size(); i++){
        branch_ind.clear();
        NeuronSWC node = axon.listNeuron.at(i);
        // find the start of a non-long-projection branch
        if(lpa_id.indexOf(node.n) == (-1) && lpa_id.indexOf(node.pn) != (-1)){

            // Initialization
            int pid=i;
            QList<int> visited;
            for(int j=0;j<axon.listNeuron.size(); j++){visited.append(0);}
            QStack<int> pstack;
            pstack.push(pid);
            bool is_push = false;
            visited[pid]=1;
            branch_ind.append(pid);

            // DFS using stack
            while(!pstack.isEmpty()){
                is_push = false;
                pid = pstack.top();
                // whether exist unvisited children of pid
                // if yes, push child to stack;
//                for(int j=0; j<axon.listNeuron.size();j++){  // This loop can be more efficient, improve it later!
                for(V3DLONG *j=children[pid].begin(); j!=children[pid].end(); ++j)
                {
                    if((axon.listNeuron.at(*j).pn)==nlist.at(pid) && visited.at(*j)==0){
                        pstack.push(*j);
                        visited[*j]=1;
                        branch_ind.append(*j);
                        is_push=true;
                        break;
                    }
                }
                // else, pop pid
                if(!is_push){
                    pstack.pop();
                }
            }

        }
        // retype the branch
        if(branch_ind.size()>0){
            // Mode 1
            if(proximal_distal){
                int branch_head = branch_ind.at(0);
                int branch_at_lpa = lpa_id.indexOf(plist.at(branch_head));
                // Proximal cluster
                if(is_proximal.at(branch_at_lpa)){
                    cur_retype = proximal_retype;
                }
                // Distal cluster
                else{
                    cur_retype = distal_retype;
                }
                for(int j=0; j<branch_ind.size(); j++){
                    axon.listNeuron[branch_ind.at(j)].type=cur_retype;
                }
            }
            // Mode 2
            else{
                if(branch_ind.size()>size_thres){
                    cur_retype++;
                    for(int j=0; j<branch_ind.size(); j++){
                        axon.listNeuron[branch_ind.at(j)].type=cur_retype;
                    }
                }
            }
        }
        // proximal and distal axon
        if(branch_ind.size()>0){
            int branch_head = branch_ind.at(0);
            int branch_at_lpa = lpa_id.indexOf(plist.at(branch_head));
            // Proximal cluster
            if(is_proximal.at(branch_at_lpa)){
                for(int j=0; j<branch_ind.size(); j++){
                    NeuronSWC node = axon.listNeuron.at(branch_ind.at(j));
                    proximal_axon.listNeuron.append(node);
                }
            }
            // Distal cluster
            if(is_distal.at(branch_at_lpa)){
                for(int j=0; j<branch_ind.size(); j++){
                    NeuronSWC node = axon.listNeuron.at(branch_ind.at(j));
                    distal_axon.listNeuron.append(node);
                }
            }
        }
    }
//    proximal_axon = missing_parent(proximal_axon);
//    proximal_axon = my_SortSWC(proximal_axon, proximal_axon.listNeuron.at(0).n, 0);
//    writeSWC_file(proximal_swc, proximal_axon);
//    crop_swc(whole_axon_swc, proximal_swc, 400, 0, 0, 0);
    distal_axon = missing_parent(distal_axon);
    distal_axon = my_SortSWC(distal_axon, distal_axon.listNeuron.at(0).n, 0);
    writeSWC_file(output_swc, axon);
    writeSWC_file(distal_swc, distal_axon);

    return 1;
}
