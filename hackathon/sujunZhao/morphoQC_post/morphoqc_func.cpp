#include "morphoqc_func.h"
#include "sort_swc.h"

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
using namespace std;

double computeDist(const NeuronSWC & p1, const NeuronSWC & p2){
    double xx = p1.x - p2.x;
    double yy = p1.y - p2.y;
    double zz = p1.z - p2.z;
    return sqrt(xx*xx+yy*yy+zz*zz);
}

NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        S.level = node.level;
        S.fea_val = node.fea_val;
        S.seg_id = node.seg_id;
        S.nodeinseg_id = node.nodeinseg_id;
        S.creatmode = node.creatmode;
        S.timestamp = node.timestamp;
        S.tfresindex = node.tfresindex;

        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

void Find_tip(V3DPluginCallback2 & callback, const V3DPluginArgList & input){
    vector<char*> infiles, inparas;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    QString swc_file = infiles.at(0);
    QString swc_name = swc_file.split("/").last().split(".").first();
    NeuronTree nt = readSWC_file(swc_file);
    const int N = nt.listNeuron.size();

    QList<int> pList;
    QList<int> nList;
    for(int i=0; i<N; i++){
        pList.append(nt.listNeuron.at(i).pn);
        nList.append(nt.listNeuron.at(i).n);
    }

    QVector< QVector <V3DLONG > > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0; i<N; i++)
    {
        int pid = nList.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    QList<int> tiplist;
    QList<int> startlist;
    QList<int> checklist;
    for(int i=0; i<N; i++){
        if(children[i].size()==0){
            tiplist.push_back(i);
            checklist.push_back(i);
        }
        if (pList.at(i) == -1 && nt.listNeuron.at(i).type !=1){
            startlist.push_back(i);
            checklist.push_back(i);
        }
    }

    int n_check = checklist.size();
    QList<int> close_points;
    for(int i=0; i<n_check; i++){
        for(int j=i+1; j<n_check; j++){
            double d = dist(nt.listNeuron.at(checklist.at(i)),nt.listNeuron.at(checklist.at(j)));
            if( d < 20 ){
                close_points.push_back(i);
                close_points.push_back(j);
            }
        }
    }

    //output APO & ANO
    QList<CellAPO> apo;
    for( int i=0; i<n_check; i++){
        CellAPO m;
        int check_id = checklist.at(i);
        m.x = nt.listNeuron.at(check_id).x;
        m.y = nt.listNeuron.at(check_id).y;
        m.z = nt.listNeuron.at(check_id).z;
        if( close_points.contains(check_id) ){
            m.color.r = 255;
            m.color.g = 255;
            m.color.b = 0;
        }
        else{
            m.color.r=0;
            m.color.g=255;
            m.color.b=0;
        }
        m.volsize = 50;
        apo.push_back(m);
    }

    QString apo_file = swc_file.split(".").first() + ".apo";
    QString apo_name = apo_file.split("/").last();
    writeAPO_file(apo_file, apo);
    QString ano_file = swc_file.split(".").first() + ".ano";
    QString swc_file_name = swc_file.split("/").last();
    QFile ano_content(ano_file);
    if(ano_content.open(QIODevice::WriteOnly | QIODevice::Text)){
        QStringList content;
        content.append("APOFILE="+apo_name+"\n");
        content.append("SWCFILE="+swc_file_name+"\n");
        for(int i=0; i<content.size(); i++){
            ano_content.write(content.at(i).toAscii());
        }
        ano_content.close();
    }
}

void Tip_pruning(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas,outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString swc_file = infiles.at(0);
    QString swc_name = swc_file.split("/").last().split(".").first();
    NeuronTree nt = readSWC_file(swc_file);
    //cout<<"***************"<<outfiles.size()<<endl;
    int N = nt.listNeuron.size();
    QList<int> record;

    QList<int> pList,nList;
    for(int i=0; i<N; i++){
        pList.append(nt.listNeuron.at(i).pn);
        nList.append(nt.listNeuron.at(i).n);
        record.append(1);
    }

    QVector<QList<int> > children;
    children = QVector<QList<int> >(N, QList<int>() );
    for (V3DLONG k=0;k<N;k++)
    {
        int pid = nList.lastIndexOf(nt.listNeuron.at(k).pn);
        if (pid<0) continue;
        children[pid].push_back(k);
    }
    QList<int> tipList,branchList;
    for (int i=0; i<N; i++){
        if(children[i].size()==0){
            tipList.append(i);
        }
        else if(children[i].size()>1){
            branchList.append(i);
        }
    }
    int n_tip = tipList.size();
    bool exist_del=false;
    cout<<"tip num:"<<n_tip<<endl;
    for(int i=0; i<n_tip; i++){
        //find parent branch
        int start_id = tipList.at(i);
        int current_id = start_id;
        QList<int> path;
        double d = 0.0;
        while(!branchList.contains(current_id)){
            path.append(current_id);
            int p_id = nList.lastIndexOf(pList[current_id]);
            d = d + computeDist(nt.listNeuron.at(p_id),nt.listNeuron.at(current_id));
            current_id = p_id;
            if(d>=6){break;}
        }
        if (d<6){
            for(int j=0; j<path.size(); j++){
                record[path.at(j)]=0;
                //cout<<"del id:"<<path[j]<<endl;
                exist_del = true;
            }
        }
        path.clear();
    }

    QString out_file;
    if(outfiles.size() == 0){
        out_file = swc_name+"_pruned.swc";
    }
    else{
        out_file = outfiles.at(0);
    }
    cout<<"need_delete:"<<exist_del<<endl;

    if(exist_del){
        QList<NeuronSWC> new_nt, new_nt_sorted;
        int count=0;
        for(int i=0; i<N; i++){
            if(record[i]==1){
                NeuronSWC line = nt.listNeuron.at(i);
                new_nt.append(line);
                count++;
            }
        }
        //sort
        SortSWC(new_nt,new_nt_sorted,VOID,VOID);
        export_list2file(new_nt_sorted, out_file, swc_file);
        cout<<"del:"<<count<<endl;
    }
    else{
        writeSWC_file(out_file,nt);
    }
}


pair<QList<int>,int> get_subtree(NeuronTree nt, int id){
    bool *visited = new bool [nt.listNeuron.size()];
    int count_basal = 0;
    QList<int> ids;
    for(int m=0; m<nt.listNeuron.size(); m++){
        visited[m]=false;
        ids.push_back(nt.listNeuron.at(m).n);
    }

    QVector<QList<int> > children;
    children = QVector<QList<int> >(nt.listNeuron.size(), QList<int>() );
    for (V3DLONG k=0;k<nt.listNeuron.size();k++)
    {
        int pid = ids.indexOf(nt.listNeuron.at(k).pn);
        if (pid<0) continue;
        children[pid].push_back(k);
    }

    QList<int> queue,treelist;
    visited[id]=true;
    queue.push_back(id);
    treelist.push_back(id);

    int id_queue;
    while(!queue.empty()){
        id_queue = queue.front();
        queue.pop_front();

        for(QList<int>::iterator i = children[id_queue].begin();i!=children[id_queue].end();++i){
            if(!visited[*i]){
                visited[*i] = true;
                queue.push_back(*i);
                treelist.push_back(*i);
                if(nt.listNeuron.at(*i).type==3){
                    count_basal++;
                }
            }
        }
    }
    cout<<"finish"<<endl;
    return make_pair(treelist,count_basal);
}

int degree_to_soma(QList<int> parent_ids,QVector<QVector<V3DLONG> > children, int soma_id,int p){
    int parent = parent_ids[p];
    int n=0;
    while(parent!=soma_id){
        if(children[parent].size()>1){
            n=n+1;
        }
        parent = parent_ids[parent];
    }
    return n;
}

bool Type_correction(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString swc_file = infiles.at(0);
    NeuronTree nt = readSWC_file(swc_file);
    QString out_file = outfiles.at(0);
    int N = nt.listNeuron.size();

    int soma_id = -1;
    bool flag_exist_apical = FALSE;
    QList <int> parent_ids,nlist;
    int count_apical_branch=0;
    for (int i=0; i<N; i++){
        nlist.append(nt.listNeuron.at(i).n);
        if ((nt.listNeuron.at(i).parent == -1) && (nt.listNeuron.at(i).type == 1)){
            soma_id = i;
        }
    }

    if(soma_id == -1){
        v3d_msg(QString("Soma info is not in the given file.\n"
                        "Process terminated.\n"
                        "Please double check!"));
        return 0;
    }

    //cout<<"soma:"<<soma_id<<endl;

    QList<int> visited,recheck_point,basal_pts,axon_pts;
    QList<NeuronSWC> crop_tree;
    int soma_circle_threshold = 300;
    int apical_soma_threshold = 50;
    int count_apical_soma=0;
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );\
    for (V3DLONG i=0;i<N;i++)
    {
        visited.append(0);
        double d = computeDist(nt.listNeuron.at(soma_id),nt.listNeuron.at(i));
        if (d<soma_circle_threshold){
            recheck_point.push_back(i);
            crop_tree.append(nt.listNeuron.at(i));
            if(d<apical_soma_threshold){
                count_apical_soma++;
            }
        }
        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
        parent_ids.append(pid);
        if (pid<0) continue;
        children[pid].push_back(i);
    }
    //cout<<"children finished"<<endl;
    QList<int> branchlist,apical_pts;
    for(int i=0; i<N;i++){
        if(children[i].size()>1){
            branchlist.append(i);
            if(nt.listNeuron.at(i).type==4){
                count_apical_branch++;
            }
        }
    }

    for(int i=0; i<children[soma_id].size();i++){
        int tmp = nt.listNeuron.at(children[soma_id].at(i)).type;
        if (tmp==2){
            axon_pts.append(children[soma_id].at(i));
        }
        else if(tmp == 3){
            basal_pts.append(children[soma_id].at(i));
        }
        else if(tmp ==4){
            apical_pts.append(children[soma_id].at(i));
        }
    }

    NeuronTree nt_crop = neuronlist_2_neurontree(crop_tree);
    QList<int> crop_tiplist;
    int count_apical_tip=0;
    for(int i=0; i<crop_tree.size();i++){
        QVector<V3DLONG> child_point = children[recheck_point[i]];
        if(child_point.size()==0){
            crop_tiplist.append(recheck_point[i]);
            if(nt.listNeuron.at(recheck_point[i]).type==4){
                count_apical_tip++;
            }
        }
        else{
            bool single=FALSE;
            for(int j=0; j<child_point.size(); j++){
                if (!recheck_point.contains(child_point.at(j))){
                    single=TRUE;
                    break;
                }
            }
            if(single==TRUE){
                crop_tiplist.append(recheck_point[i]);
                if(nt.listNeuron.at(recheck_point[i]).type==4){
                    count_apical_tip++;
                }
            }
        }
    }

    //cout<<"finish crop"<<endl;
    QList<int> common_axon_branch;
    int nct = crop_tiplist.size();
    int apical_start=-1,axon_start=-1;
    if(axon_pts.size()==1){
        axon_start=axon_pts[0];
    }
    if(apical_pts.size()==1){
        apical_start=apical_pts[0];
    }
    cout<<"axon pts size"<<axon_pts.size()<<endl;
    cout<<"axon start"<<axon_start<<endl;
    cout<<"apical start"<<apical_start<<endl;
    cout<<"parameter:"<<count_apical_branch<<" "<<count_apical_tip<<endl;
    if((count_apical_branch>1)&&(count_apical_tip>1)){
        if(apical_start!=-1){
            if(count_apical_soma>10){
                flag_exist_apical=TRUE;
            }
        }
        else{
            flag_exist_apical=TRUE;
        }
    }

    QList<int> axon_start_check;
    QHash<int,int> map_axon_child;
    //cout<<"tip:"<<nct<<endl;
    //cout<<crop_tiplist.size()<<endl;

    cout<<"input axon start"<<axon_start<<endl;
    cout<<"initial apical exist:"<<flag_exist_apical<<endl;
    cout<<"check point:"<<nct<<endl;
    for(int i=0; i<nct; i++){
        int current_id = crop_tiplist[i];
        cout<<"type"<<current_id<<endl;
        int p = parent_ids[current_id];
        int t = nt.listNeuron.at(current_id).type;
        if ((t==4)&&(flag_exist_apical)){
            //cout<<"check apical"<<endl;
            QList<int> recheck_apical;
            while(p!=soma_id){
                if((nt.listNeuron.at(current_id).type==4)&&(nt.listNeuron.at(p).type!=4)){
                    recheck_apical.append(current_id);
                }
                current_id = p;
                p=parent_ids[p];
            }
            cout<<"check1:"<<p<<endl;
            if(apical_start==-1){
                apical_start = current_id;
            }
            if(recheck_apical.size()>0){
                int apical_to_soma = computeDist(nt.listNeuron.at(soma_id),nt.listNeuron.at(recheck_apical.last()));
                if(apical_to_soma>50){
                    apical_start=-1;
                }
                recheck_apical.clear();
            }
            cout<<"apical:"<<current_id<<endl;
        }
//        else if((t==2)&&(!axon_flag)){
        else if(t==2){
            QList<int> recheck_axon;
            int current_type = nt.listNeuron.at(current_id).type;
            int type_p = nt.listNeuron.at(p).type;
            map_axon_child[current_id]=0;
            while(p!=soma_id){
                if((type_p !=2)&&(current_type==2)){
                    recheck_axon.append(current_id);
                }
                if(current_type==2){
                    map_axon_child[current_id]++;
                }
                current_id = p;
                current_type = nt.listNeuron.at(current_id).type;
                p=parent_ids[p];
                type_p = nt.listNeuron.at(p).type;
            }

            //cout<<"current axon:"<<current_id<<endl;
            if((axon_start != -1)&&(!axon_pts.contains(current_id))){
                cout<<"step1"<<endl;
                cout<<current_id<<endl;

                int tmp_id = recheck_axon.last();
                if((tmp_id!=axon_start)&&(parent_ids[tmp_id]!=soma_id)){
//                        int org_n = degree_to_soma(parent_ids,children,soma_id,axon_start);
//                        int c_n = degree_to_soma(parent_ids,children,soma_id,tmp_id);
                    int org_n = map_axon_child[axon_start];
                    int c_n = map_axon_child[tmp_id];
                    if(c_n>org_n){
                        axon_start = tmp_id;
                        common_axon_branch.append(axon_start);
                        cout<<"change axon1:"<<axon_start<<endl;
                    }
                }
            }
            else if (axon_start == -1){
                cout<<"step2"<<endl;
                cout<<"size:"<<recheck_axon.size()<<endl;
                QList<int> new_recheck = recheck_axon;
                new_recheck.append(current_id);
                if(new_recheck.size()>0){
                    int d_n=-1;
                    int axon_start_mark=-1;
                    for(int k=0;k<new_recheck.size();k++){
                        cout<<"**************"<<new_recheck[k];
                        int tmp_n = map_axon_child[new_recheck[k]];
                        cout<<"test:"<<new_recheck[k]<<" "<<tmp_n<<endl;
                        if (tmp_n>d_n){
                            axon_start_mark=new_recheck.at(k);
                            d_n=tmp_n;
                        }
                    }
                    cout<<"mark:"<<axon_start_mark<<endl;
                    if(axon_start_mark>0){
                        axon_start=axon_start_mark;
                        common_axon_branch.append(axon_start);
                        cout<<"change axon2:"<<axon_start<<endl;
                    }
                }
            }
            recheck_axon.clear();
        }
        else{
            continue;
        }
    }
    //cout<<"finish check"<<endl;
    cout<<"apical_exist:"<<flag_exist_apical<<endl;
    QList<int> axon_subtree, apical_subtree;
    if((flag_exist_apical)&&(apical_start!=-1)){
        pair <QList<int>,int> apical_test = get_subtree(nt,apical_start);
        apical_subtree = apical_test.first;
    }
    cout<<"axon final"<<axon_start<<endl;
    pair <QList<int>,int> axon_test = get_subtree(nt,axon_start);
    axon_subtree = axon_test.first;
    int axon_count_basal = axon_test.second;
    cout<<"axon_basal_count:"<<axon_count_basal<<endl;
    cout<<"apical:"<<apical_start<<" "<<apical_subtree.size()<<"\n";
    cout<<"axon:"<<axon_start<<" "<<axon_subtree.size()<<"\n";
    if(axon_count_basal > 10){
        QVector<V3DLONG> tmp_start = children[axon_start];
        int start_tmp;
        if(branchlist.contains(axon_start)){
            int c1 = tmp_start[0];
            int c2 = tmp_start[1];
            pair <QList<int>,int> c_test1 = get_subtree(nt,c1);
            int count_c1 = c_test1.second;
            pair <QList<int>,int> c_test2 = get_subtree(nt,c2);
            int count_c2 = c_test2.second;
            if(count_c1<count_c2){
                start_tmp = c1;
            }
            else{
                start_tmp = c2;
            }
        }
        else{
            start_tmp = tmp_start[0];
        }
        while((nt.listNeuron.at(start_tmp).type!=2)&(!branchlist.contains(parent_ids[start_tmp]))){
            start_tmp = children[start_tmp][0];
        }
        axon_start = start_tmp;
    }
    axon_subtree.clear();
    pair <QList<int>,int> axon_test2 = get_subtree(nt,axon_start);
    axon_subtree = axon_test2.first;
    cout<<"axon2:"<<axon_start<<" "<<axon_subtree.size()<<"\n";

    QList<NeuronSWC> results;
    for(int i=0; i<N; i++){
        NeuronSWC line;
        if(i==soma_id){
            results.append(nt.listNeuron.at(i));
        }
        else if(apical_subtree.contains(i)){
            line = nt.listNeuron.at(i);
            line.type =4;
            results.append(line);
        }
        else if(axon_subtree.contains(i)){
            line = nt.listNeuron.at(i);
            line.type=2;
            results.append(line);
        }
        else{
            line = nt.listNeuron.at(i);
            line.type = 3;
            results.append(line);
        }
    }

    NeuronTree final_reults = neuronlist_2_neurontree(results);
    writeSWC_file(out_file,final_reults);
}
