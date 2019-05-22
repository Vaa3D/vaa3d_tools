//functions for axon analysis
//by Peng Xie
//2018-08-16

#ifdef DISABLE_V3D_MSG
#define DISABLE_V3D_MSG
#endif

#define DEFAULT 1000000000

#include "axon_func.h"
#include "crop_swc_main.h"
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <fstream>

#define MAXSIZE 1000000

//QList<int> get_subtree(NeuronTree nt, int id){
//        QVector<QVector<V3DLONG> > children;
//        QList<int> names;
//        QList<int> treelist;
//        for(int m=0; m<nt.listNeuron.size(); m++){
//            names.push_back(nt.listNeuron.at(m).n);
//        }
//        int N=nt.listNeuron.size();
//        children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
//        for (V3DLONG i=0;i<N;i++)
//        {
//            int pid = names.lastIndexOf(nt.listNeuron.at(i).pn);
//            if (pid<0) continue;
//            children[pid].push_back(i);
//        }
//      // Initialization
//                int pid=id;
//                QList<int> visited;
//                for(int j=0;j<nt.listNeuron.size(); j++){visited.append(0);}
//                QStack<int> pstack;
//                pstack.push(pid);
//                bool is_push = false;
//                visited[pid]=1;
//                //branch_ind.append(pid);

//                // DFS using stack
//                while(!pstack.isEmpty()){
//                    is_push = false;
//                    pid = pstack.top();
//                    // whether exist unvisited children of pid
//                    // if yes, push child to stack;
//    //                for(int j=0; j<axon.listNeuron.size();j++){  // This loop can be more efficient, improve it later!
//                    for(V3DLONG *j=children[pid].begin(); j!=children[pid].end(); ++j)
//                    {
//                        if((nt.listNeuron.at(*j).pn)==names.at(pid) && visited.at(*j)==0){
//                            pstack.push(*j);
//                            visited[*j]=1;
//                            //branch_ind.append(*j);
//                            is_push=true;
//                            break;
//                        }
//                    }
//                    // else, pop pid
//                    if(!is_push){
//                        pstack.pop();
//                    }
//                }

//}



QList<int> get_subtree(NeuronTree nt, int id){
    bool *visited = new bool [nt.listNeuron.size()];
    QList<int> names;
    for(int m=0; m<nt.listNeuron.size(); m++){
        visited[m]=false;
        names.push_back(nt.listNeuron.at(m).n);
    }

    QVector<QList<int> > children;
    children = QVector<QList<int> >(nt.listNeuron.size(), QList<int>() );
    for (V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        int pid = names.indexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    QList<int> queue,treelist;
    visited[id]=true;
    queue.push_back(id);
    treelist.push_back(id);

    int id_queue;
    //cout<<"----------------------------------------------------"<<endl;
    while(!queue.empty()){
        id_queue = queue.front();
        queue.pop_front();

        for(QList<int>::iterator i = children[id_queue].begin();i!=children[id_queue].end();++i){
            if(!visited[*i]){
                visited[*i] = true;
                queue.push_back(*i);
                treelist.push_back(*i);
//                cout<<nt.listNeuron.at(*i).n<<" "<<nt.listNeuron.at(*i).type<<" "<<nt.listNeuron.at(*i).x<<" "
//                   <<nt.listNeuron.at(*i).y<<" "<<nt.listNeuron.at(*i).z<<" "<<
//                     nt.listNeuron.at(*i).r<<" "<<nt.listNeuron.at(*i).pn<<" "<<endl;
            }
        }
    }
    return treelist;
}


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

QList<int> match_axon(NeuronTree axon, NeuronTree lpa){

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
        lpa_id.append(node_axon.n);
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
    QList<int> parent_list;
    for(int i=0; i<N; i++){
        name_list.append(nt.listNeuron.at(i).n);
        parent_list.append(nt.listNeuron.at(i).pn);
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
    map<int,int > ch2;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = name_list.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
        int count_num=count(parent_list.begin(),parent_list.end(),nt.listNeuron.at(i).pn);
        if(count_num==2){
            int another_id=parent_list.lastIndexOf(nt.listNeuron.at(i).pn);
            ch2.insert(pair<int,int>(i,N-another_id));
            ch2.insert(pair<int,int>(N-another_id,i));
        }
    }

    cout<<ch2.size()<<endl;


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
                distance[*i]=distance.at(pid)+computeDist2(nt.listNeuron.at(pid), nt.listNeuron.at(*i), XSCALE, YSCALE, ZSCALE);
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
//        if(!(ch2.find(cur_id)==ch2.end())){
//             cout<<ch2.at(cur_id)<<"*********find branch*************"<<cur_id<<endl;
//             QList<int> subT = get_subtree(nt,ch2.at(cur_id));
//             for(int k=0; k<subT.size(); k++){
//                 lpa.prepend(subT.at(k));
//             }
//        }
        cur_id = name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn); // Move to the parent node
        if(cur_id==name_list.lastIndexOf(nt.listNeuron.at(cur_id).pn)){break;} // check self loop; should not exist;
        //cout<<cur_id<<"llllllllllllllllllllllllllll"<<endl;
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
    cout<<"finished"<<endl;
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
//bool axon_retype(QString whole_axon_swc, QString lpa_swc, QString output_swc, bool proximal_distal){
bool axon_retype(QString whole_axon_swc, QString lpa_swc, QString output_swc){
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
    QString den_swc;
    QString long_swc;
    if (output_swc.endsWith(".swc") || output_swc.endsWith(".SWC")){
        proximal_swc = output_swc.left(output_swc.length()-11)+".proximal_axon.swc";
        distal_swc = output_swc.left(output_swc.length()-11)+".distal_axon.swc";
        den_swc = output_swc.left(output_swc.length()-16)+".dendrite.swc";
        long_swc = output_swc.left(output_swc.length()-11)+".long_projection.swc";
    }
    else if (whole_axon_swc.endsWith(".eswc") || whole_axon_swc.endsWith(".ESWC")){
        proximal_swc = output_swc.left(output_swc.length()-12)+".proximal_axon.eswc";
        distal_swc = output_swc.left(output_swc.length()-12)+".distal_axon.eswc";
        den_swc = output_swc.left(output_swc.length()-17)+".dendrite.eswc";
        long_swc = output_swc.left(output_swc.length()-12)+".long_projection.eswc";
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
    //crop_swc(whole_axon_swc, proximal_swc, 200, 0, 0, 0, 0,0,0,0,0);

    // 1.2 Load lpa
    NeuronTree lpa = readSWC_file(lpa_swc);
    // 1.3 Match axon and lpa
    QList<int> l = match_axon_and_lpa(axon, lpa);
    QList<int> lpa_id = match_axon(axon,lpa);
    NeuronTree lpa2 = get_subtree_by_name(axon, l);
//    writeSWC_file("/home/penglab/Desktop/test2/1.swc", lpa2);
//    ofstream lid("/home/penglab/Desktop/test2/lpa_id.txt");
//    for (int i=0; i<lpa_id.size();i++){
//        lid << lpa_id.at(i) << endl;
//    }
//    lid.close();

    QList<double> path_distance;
    path_distance.append(0);
    double cur_distance = 0;
    double max_dis=0;
    QList<int> lpa_plist;
    QList<int> lpa_nlist;
    for(int i=0;i<lpa2.listNeuron.size();i++){
        lpa_plist.append(lpa.listNeuron.at(i).pn);
        lpa_nlist.append(lpa.listNeuron.at(i).n);
    }

    for(int i=1; i<lpa2.listNeuron.size(); i++){
        cur_distance += computeDist2(lpa2.listNeuron.at(i-1), lpa2.listNeuron.at(i), XSCALE, YSCALE, ZSCALE);
        path_distance.append(cur_distance);
        double d=computeDist2(lpa2.listNeuron.at(i),lpa2.listNeuron.at(0),1,1,1);
        max_dis = max(d,max_dis);
    }
    double total_path_distance = cur_distance;
    qDebug()<<"Long projection axon path distance:\t"<<total_path_distance;
    //printf("%s\n",den_swc.toStdString().data());

    //dendrite radius
    //1.load dendrite data
    NeuronTree dendrite = readSWC_file(den_swc);
    double den_r=0;
    for(int i=0; i<dendrite.listNeuron.size();i++){
        double dis = computeDist2(dendrite.listNeuron.at(i),dendrite.listNeuron.at(0),1,1,1);
        if(dis>den_r){
            den_r=dis;
        }
    }
    //cout<<"**************dendrite*********"<<endl;
    QList<int> b_proximal, b_distal;
    b_proximal.append(1);
    b_distal.append(0);
    QList<int> branch_list;
    map<int,int > child;
    for(int i=0; i<lpa2.listNeuron.size();i++){
        if(count(plist.begin(),plist.end(),lpa_id.at(i))>1){
            //lpa ids
            branch_list.push_back(i);
            //axon ids
            int next_id = lpa_plist.indexOf(lpa2.listNeuron.at(i).n);
            int idc1 = plist.indexOf(lpa_id.at(i));
            int idc2 = axon.listNeuron.size()-plist.lastIndexOf(lpa_id.at(i));
            if(nlist.at(idc1)==lpa_id.at(next_id)){
                child.insert(pair<int,int>(next_id,idc2));
            }
            else{
                child.insert(pair<int,int>(next_id,idc1));
            }
        }
    }
    ofstream b("/home/penglab/Desktop/test2/b_id.txt");
    for (int i=0; i<branch_list.size();i++){
        b << branch_list.at(i) << endl;
    }
    b.close();
//    ofstream cid("/home/penglab/Desktop/test2/c_id.txt");
//    for (map<int,int>::iterator iter = child.begin(); iter != child.end() ; iter++){
//        cid << iter->first << " "<<iter->second << endl;
//    }
//    lid.close();



    //cout<<"check1"<<endl;
    for(int i=0;i<branch_list.size();i++){
        double bd = computeDist2(dendrite.listNeuron.at(0),lpa2.listNeuron.at(branch_list.at(i)),1,1,1);
        //proximal
        if(bd<2*den_r){
            b_proximal.append(1);
        }
        else{b_proximal.append(0);}
        //distal
        if((path_distance.at(branch_list.at(i)-1)/total_path_distance) >0.99){
            b_distal.append(1);
        }
        else{
            b_distal.append(0);
        }
    }
    b_distal.append(1);
    b_proximal.append(0);
    cout<<"check2"<<endl;
    NeuronTree proximal_axon;
    NeuronTree distal_axon;
    int cb=0;
    int end_bid;
    int start_bid;
    int start_long;
    for(int i=0; i<lpa2.listNeuron.size();i++){
        NeuronSWC node=lpa2.listNeuron.at(i);
        //proximal
        if(b_proximal.at(cb)==1 && b_proximal.at(cb+1)==1){
            proximal_axon.listNeuron.append(node);
            if(i==branch_list.at(cb)){
                //cout<<"proximal"<<endl;
                cb=cb+1;
                QList<int> ch_list;
                int ch_id= lpa_plist.indexOf(lpa2.listNeuron.at(i).n);
                ch_list=get_subtree(axon,child.at(ch_id));
                //cout<<child.at(ch_id)<<"!!!!!!!!!!!!!!!!proximal"<<endl;
                for(int j=0; j<ch_list.size();j++){
                    NeuronSWC b_node =axon.listNeuron.at(ch_list.at(j));
                    proximal_axon.listNeuron.append(b_node);
                }
            }
        }
        else if(b_proximal.at(cb)==1 && b_proximal.at(cb+1)==0){
            end_bid=i;
            start_long = cb;
            break;
        }
    }
    cout<<path_distance.at(branch_list.at(7))<<endl;
    cout<<"proximal finish"<<endl;
    int s=b_distal.indexOf(1);
    cout<<s<<"****************"<<endl;
    int db = s;
    for(int i=branch_list.at(s-1); i<lpa2.listNeuron.size();i++){
        NeuronSWC node=lpa2.listNeuron.at(i);
        cout<<i<<endl;
//        //distal
//        if(b_distal.at(db)==0 && b_distal.at(db+1)==0){
//            if(i==branch_list.at(db)){
//                db=db+1;
//            }
//        }
//        else if(b_distal.at(db)==0 && b_distal.at(db+1)==1){
//            if(i==branch_list.at(db)){
//            start_bid=branch_list.at(db-1);
//            db=db+1;
//            }
//        }
//        else if(b_distal.at(db)==1 && b_distal.at(db+1)==1 && db<(b_distal.size()-1)){
        if(db<(b_distal.size()-1)){
            distal_axon.listNeuron.append(node);
            if(i==branch_list.at(db)){
                db=db+1;
                QList<int> ch_list;
                int ch_id= lpa_plist.indexOf(lpa2.listNeuron.at(i).n);
                ch_list=get_subtree(axon,child.at(ch_id));
                //cout<<child.at(ch_id)<<"!!!!!!!!!!!!!!!!proximal"<<endl;
                for(int j=0; j<ch_list.size();j++){
                    NeuronSWC d_node =axon.listNeuron.at(ch_list.at(j));
                    distal_axon.listNeuron.append(d_node);
                }
            }
        }
    }
    cout<<"check3"<<endl;
    QList<int>::iterator iter;
    for(iter=b_distal.begin();iter!=b_distal.end();iter++)
         cout << *iter << " ";
    cout << endl;
    cout<<end_bid<<"**************"<<start_bid<<endl;
    for(int j=0;j<5;j++){
        NeuronSWC node = lpa2.listNeuron.at(end_bid+j);
        proximal_axon.listNeuron.append(node);
    }
    cout<<"proximal"<<endl;
//    for(int j=5;j>0;j--){
//        NeuronSWC node = lpa2.listNeuron.at(start_bid-j);
//        distal_axon.listNeuron.prepend(node);
//    }
//    cout<<"distal"<<endl;
    //long projection
    NeuronTree long_projection;
    int sl=start_long;



    for(int k=end_bid+5;k<branch_list.at(s-1);k++){
        NeuronSWC node = lpa2.listNeuron.at(k);
        long_projection.listNeuron.append(node);
        if(k==branch_list.at(sl)){
            sl=sl+1;
            if(b_distal.at(sl)==1){break;}
            if(b_proximal.at(sl)==1){break;}
            QList<int> ch_list;
            int ch_id= lpa_plist.indexOf(lpa2.listNeuron.at(k).n);
            cout<<"long projection branch"<<endl;
            ch_list=get_subtree(axon,child.at(ch_id));
            cout<<child.at(ch_id)<<endl;
            for(int j=0; j<ch_list.size();j++){
                NeuronSWC l_node =axon.listNeuron.at(ch_list.at(j));
                long_projection.listNeuron.append(l_node);
            }
        }
    }
    cout<<"check4"<<endl;

//    for(int i=0; i<lpa.listNeuron.size();i++){
//        NeuronSWC node=lpa.listNeuron.at(i);
//        if((path_distance.at(i)/total_path_distance)<0.2){
//            is_proximal.append(1);
////            lpa.listNeuron[i].type=0;
//            proximal_axon.listNeuron.append(node);
//        }
//        else{
//            is_proximal.append(0);
//        }
//        if((path_distance.at(i)/total_path_distance)>0.7){
//            is_distal.append(1);
////            lpa.listNeuron[i].type=1;
//            distal_axon.listNeuron.append(node);
//        }
//        else{
//            is_distal.append(0);
//        }
//    }
//    export_listNeuron_2swc(lpa.listNeuron, "proximal_distal.swc");

    // 2. label non-long-projection axon braches
    // 2.1 default mode: retype axon clusters as proximal or distal
//    int proximal_retype = 5;
//    int distal_retype = 6;
//    // 2.2 optional model: label branch with a different type if passes size_thres.
//    int cur_retype = 2;
//    int size_thres = 10;
//    QList<int> branch_ind;
//    const int N=axon.listNeuron.size();
////    list<int> children[MAXSIZE];
////    for(int i=0; i<N; i++){
////        NeuronSWC node = axon.listNeuron.at(i);
////        int pid = nlist.lastIndexOf(node.pn);
////        if(pid<0){continue;}
////        children[pid].push_back(i);
////    }
//    QVector<QVector<V3DLONG> > children;
//    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
//    for (V3DLONG i=0;i<N;i++)
//    {
//        int pid = nlist.lastIndexOf(axon.listNeuron.at(i).pn);
//        if (pid<0) continue;
//        children[pid].push_back(i);
//    }

//    for(int i=1; i<axon.listNeuron.size(); i++){
//        branch_ind.clear();
//        NeuronSWC node = axon.listNeuron.at(i);
//        // find the start of a non-long-projection branch
//        if(lpa_id.indexOf(node.n) == (-1) && lpa_id.indexOf(node.pn) != (-1)){

//            // Initialization
//            int pid=i;
//            QList<int> visited;
//            for(int j=0;j<axon.listNeuron.size(); j++){visited.append(0);}
//            QStack<int> pstack;
//            pstack.push(pid);
//            bool is_push = false;
//            visited[pid]=1;
//            branch_ind.append(pid);

//            // DFS using stack
//            while(!pstack.isEmpty()){
//                is_push = false;
//                pid = pstack.top();
//                // whether exist unvisited children of pid
//                // if yes, push child to stack;
////                for(int j=0; j<axon.listNeuron.size();j++){  // This loop can be more efficient, improve it later!
//                for(V3DLONG *j=children[pid].begin(); j!=children[pid].end(); ++j)
//                {
//                    if((axon.listNeuron.at(*j).pn)==nlist.at(pid) && visited.at(*j)==0){
//                        pstack.push(*j);
//                        visited[*j]=1;
//                        branch_ind.append(*j);
//                        is_push=true;
//                        break;
//                    }
//                }
//                // else, pop pid
//                if(!is_push){
//                    pstack.pop();
//                }
//            }

//        }
//        // retype the branch
//        if(branch_ind.size()>0){
//            // Mode 1
//            if(proximal_distal){
//                int branch_head = branch_ind.at(0);
//                int branch_at_lpa = lpa_id.indexOf(plist.at(branch_head));
//                // Proximal cluster
//                if(is_proximal.at(branch_at_lpa)){
//                    cur_retype = proximal_retype;
//                }
//                // Distal cluster
//                else{
//                    cur_retype = distal_retype;
//                }
//                for(int j=0; j<branch_ind.size(); j++){
//                    axon.listNeuron[branch_ind.at(j)].type=cur_retype;
//                }
//            }
//            // Mode 2
//            else{
//                if(branch_ind.size()>size_thres){
//                    cur_retype++;
//                    for(int j=0; j<branch_ind.size(); j++){
//                        axon.listNeuron[branch_ind.at(j)].type=cur_retype;
//                    }
//                }
//            }
//        }
//        // proximal and distal axon
//        if(branch_ind.size()>0){
//            int branch_head = branch_ind.at(0);
//            int branch_at_lpa = lpa_id.indexOf(plist.at(branch_head));
//            // Proximal cluster
//            if(is_proximal.at(branch_at_lpa)){
//                for(int j=0; j<branch_ind.size(); j++){
//                    NeuronSWC node = axon.listNeuron.at(branch_ind.at(j));
//                    proximal_axon.listNeuron.append(node);
//                }
//            }
//            // Distal cluster
//            if(is_distal.at(branch_at_lpa)){
//                for(int j=0; j<branch_ind.size(); j++){
//                    NeuronSWC node = axon.listNeuron.at(branch_ind.at(j));
//                    distal_axon.listNeuron.append(node);
//                }
//            }
//        }
//    }
    proximal_axon = missing_parent(proximal_axon);
    proximal_axon = my_SortSWC(proximal_axon, proximal_axon.listNeuron.at(0).n, 0);
    writeSWC_file(proximal_swc, proximal_axon);
    cout<<"check save proximal"<<endl;
    long_projection = missing_parent(long_projection);
    cout<<"check1"<<endl;
    long_projection = my_SortSWC(long_projection, long_projection.listNeuron.at(0).n, 0);
    cout<<"check2"<<endl;
    writeSWC_file(long_swc, long_projection);
    cout<<"check save long"<<endl;
//    crop_swc(whole_axon_swc, proximal_swc, 400, 0, 0, 0);
    distal_axon = missing_parent(distal_axon);
    distal_axon = my_SortSWC(distal_axon, distal_axon.listNeuron.at(0).n, 0);
//    writeSWC_file(output_swc, axon);
    writeSWC_file(distal_swc, distal_axon);
    cout<<"check save distal"<<endl;

    return 1;
}
