#include "topo_validation.h"
#include "soma_confirmation.h"
using namespace std;

void apo_generate(QList<QList<int> > points_list, QString swc_file, QList<QList<int> > color_marks){
    QList<CellAPO> apo_list;
    NeuronTree swc = readSWC_file(swc_file);
    for(int i=0; i<points_list.size(); i++){
        QList<int> pts = points_list[i];
        QList<int> Mcolor = color_marks[i];
        if(pts.size()==0){
            continue;
        }
        for(int j=0; j<pts.size(); j++){
            CellAPO m;
            m.x = swc.listNeuron.at(pts.at(j)).x;
            m.y = swc.listNeuron.at(pts.at(j)).y;
            m.z = swc.listNeuron.at(pts.at(j)).z;
            m.color.r = Mcolor[0];
            m.color.g = Mcolor[1];
            m.color.b = Mcolor[2];
            m.volsize = 50;
            apo_list.push_back(m);
        }
    }
    cout<<"apo num:"<<apo_list.size()<<endl;

    QFileInfo fileinfo(swc_file);
    QString apo_file = fileinfo.absolutePath()+"/"+fileinfo.completeBaseName()+".apo";
    cout<<"apo name"<<apo_file.toStdString()<<endl;
    writeAPO_file(apo_file,apo_list);
}

QList< QList<int> > node_count(NeuronTree nt){
    int N = nt.listNeuron.size();
    int soma_id;
    QList<int> nlist,plist,root_list,multifurcation_list;
    for(int i=0; i<N; i++){
        nlist.append(nt.listNeuron.at(i).n);
        plist.append(nt.listNeuron.at(i).pn);
        if((nt.listNeuron.at(i).type==1)&&(nt.listNeuron.at(i).pn==-1)){
            soma_id = i;
        }
    }

    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    for(int i=0; i<nt.listNeuron.size();i++){
        if(nt.listNeuron.at(i).pn==-1){
            root_list.append(i);
        }
        else{
            int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
            if(pid<0){
                root_list.append(i);
            }
        }
        if((children[i].size()>2)&&(i!=soma_id)){
            multifurcation_list.append(i);
        }
    }

    QList< QList<int> > result;
    result<<root_list<<multifurcation_list;
    return result;
}

QHash< QString, QList<int> > duplicated_pts_check(NeuronTree nt){
    // check coordinates & connected relationships
    QHash< QString, QList<int> > coord_id_pairs, duplicated_coords;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        QString coords;
        coords=QString("%1").arg(nt.listNeuron.at(i).x)+"_"+QString("%1").arg(nt.listNeuron.at(i).y)+"_"+QString("%1").arg(nt.listNeuron.at(i).z);
        if(!coord_id_pairs.contains(coords)){
            QList<int> tmp;
            tmp<<i;
            coord_id_pairs.insert(coords,tmp);
        }
        else{
            coord_id_pairs[coords].append(i);
        }
    }

    QHash<QString, QList<int> >::iterator item;
    for (item = coord_id_pairs.begin(); item!= coord_id_pairs.end(); ++item){
        if(item.value().size()>1){
            duplicated_coords[item.key()]=item.value();
        }
    }
    return duplicated_coords;
}

//QList<int> loop_detection(const NeuronTree & nt){
//    int siz = nt.listNeuron.size();

//    QList<int> nlist,plist,trifur,root_pts;
//    QHash< QString, QList<int> > dup_pts;

//    nlist.clear();
//    plist.clear();
//    QVector< QVector<int> > neighbor;
//    neighbor = QVector< QVector<int> >(siz, QVector<int>() );
//    bool *visited = new bool [nt.listNeuron.size()];

//    //count xyz pair, record corresponding ids
//    dup_pts = duplicated_pts_check(nt);

//    for(int i=0; i<siz; i++){
//        nlist.append(nt.listNeuron[i].n);
//        plist.append(nt.listNeuron[i].pn);
//        if(nt.listNeuron.at(i).pn==-1){
//            root_pts.append(i);
//        }
//        visited[i] = false;
//    }

//    //neighbor matrix
//    for(int i=0; i<siz; i++){
//        int child_id = plist.indexOf(nt.listNeuron.at(i).n);
//        int parent_id = nlist.indexOf(nt.listNeuron.at(i).pn);
//        if(child_id != -1){
//            neighbor[i].push_back(child_id);
//            neighbor[child_id].push_back(i);
//        }
//        if(plist.at(i) != -1 && parent_id != -1){
//            neighbor[i].push_back(parent_id);
//            neighbor[parent_id].push_back(i);
//        }
//    }

//    //DFS
//    QList<int> loop_mark_candidate;
////    QHash <QString, int> cnt;
//    for(int j=0;j<root_pts.size();j++){
//        QList<int> queue;
//        queue.push_back(root_pts.at(j));
//        visited[root_pts.at(j)]=true;
//        //cout<<"***************"<<soma_candidate.at(j)<<endl;
//        int id_queue;
//        while(!queue.empty()){
//            id_queue = queue.front();
//            queue.pop_front();
//            QList<float> coord;
//            coord<<nt.listNeuron.at(id_queue).x<<nt.listNeuron.at(id_queue).y<<nt.listNeuron.at(id_queue).z;
//            if(dup_pts.contains(coord)){
//                QList<int> duplicated_pts_tmp = dup_pts[coord];
//                for(int i=0; i<duplicated_pts_tmp.size(); i++){
//                    if(!visited[duplicated_pts_tmp[i]]){
//                        visited[duplicated_pts_tmp[i]]=true;
//                        queue.push_back(duplicated_pts_tmp[i]);
//                    }
//                }
//            }
//                for(QVector<int>::iterator iter = neighbor[id_queue].begin(); iter!=neighbor[id_queue].end(); ++iter){
//                    if(!visited[*iter]){
//                        visited[*iter]=true;
//                        queue.push_back(*iter);
//                    }
//                    else{
//                        if(!loop_mark_candidate.contains(*iter) && !loop_mark_candidate.contains(id_queue)){
//                            int k=0;
//                            for(int m=0; m<same_coord[*iter].size(); m++){
//                                if(visited[same_coord[*iter].at(m)]!=true){
//                                    k=k+1;
//                                }
//                            }
//                            if(k == 0 && same_coord[*iter].size()>0){
//                               loop_mark_candidate.append(id_queue);
//                               loop_mark_candidate.append(*iter);
//                               break;
//                            }
//                            //cout<<"**********"<<id_queue<<endl;
//                        }
////                        if(!cnt.count(indicator)){
////                            cnt[indicator]=1;
//                        }
////                        else{
////                            cnt[indicator]++;
////                            cout<<cnt[indicator]<<"***************"<<endl;
////                        }
////                    }
//                }
//    }
//    }
//    cout<<"check6"<<endl;
//    return loop_mark_candidate;
//}


void Topo_check(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    //check parent=-1
    QString in_name = infiles.at(0);
    QFileInfo fileinfo(in_name);
    if(fileinfo.isFile()){
        NeuronTree nt = readSWC_file(in_name);

        // single tree; multifurcation
        QList<QList<int> > node_markdown = node_count(nt);
        QList<int> rootlist, multiF_list;
        QHash< QString, QList<int> > dup_nodes;
        rootlist = node_markdown[0];
        multiF_list = node_markdown[1];
        cout<<"root:"<<rootlist.size()<<endl;
        cout<<"multiF:"<<multiF_list.size()<<endl;

        dup_nodes = duplicated_pts_check(nt);
        cout<<"dup:"<<dup_nodes.size()<<endl;

        if((rootlist.size()>1)||(multiF_list.size()>0)||(dup_nodes.size()>0)){
            QList<int> dup_node_list;
            if(dup_nodes.size()>0){
                QHash< QString, QList<int> >::const_iterator k;
                for(k=dup_nodes.constBegin();k!=dup_nodes.constEnd();++k){
                    dup_node_list.append(k.value()[0]);
                }
            }
            cout<<"dup2:"<<dup_node_list.size()<<endl;
            if(rootlist.size()<2){
                rootlist.clear();
            }

            QList<int> root_color,furcation_color,dup_color;
            root_color<<255<<255<<0;
            furcation_color<<0<<255<<255;
            dup_color<<255<<0<<255;
            QList<QList<int> > point_marked, color_marked;
            point_marked<<rootlist<<multiF_list<<dup_node_list;
            color_marked<<root_color<<furcation_color<<dup_color;
            apo_generate(point_marked, in_name, color_marked);
        }
    }
    else{
        cout<<"folder:"<<in_name.toStdString()<<endl;
        QDir dir(in_name);
        QStringList nameFilters;
        nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
        QStringList swclist = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        QString csv_out = fileinfo.absolutePath()+"/treeNum_info.csv";
        QFile check_csv(csv_out);
        for(int i=0; i<swclist.size(); i++){
            QString swc_file = in_name+"/"+swclist.at(i);
            NeuronTree nt = readSWC_file(swc_file);
            QList<QList<int> > node_markdown = node_count(nt);
            QList<int> rootlist, multiF_list;
            rootlist = node_markdown[0];
            multiF_list = node_markdown[1];

//            QHash< QString, QList<int> > dup_nodes;
//            dup_nodes = duplicated_pts_check(nt);

            //if((rootlist.size()>1)||(dup_nodes.size()>0)||(multiF_list.size()>0)){
            if((rootlist.size()>1)||(multiF_list.size()>0))
//                QList<int> dup_node_list;
//                if(dup_nodes.size()>0){
//                    QHash< QString, QList<int> >::const_iterator k;
//                    for(k=dup_nodes.constBegin();k!=dup_nodes.constEnd();++k){
//                        dup_node_list.append(k.value()[0]);
//                    }
//                }
                if(rootlist.size()<2){
                    rootlist.clear();
                }

                QList<int> root_color,furcation_color,dup_color;
                root_color<<255<<255<<0;
                furcation_color<<0<<255<<255;
                dup_color<<255<<0<<255;
                QList<QList<int> > point_marked, color_marked;
                //point_marked<<rootlist<<multiF_list<<dup_node_list;
                point_marked<<rootlist<<multiF_list;
                //color_marked<<root_color<<furcation_color<<dup_color;
                color_marked<<root_color<<furcation_color;
                apo_generate(point_marked, swc_file, color_marked);

                QFileInfo swcInfo(swc_file);
                QString neuron_name = swcInfo.completeBaseName();
                cout<<"neuron:"<<neuron_name.toStdString()<<endl;
                if(!check_csv.exists()){
                    QString head = "neuron_name,#root,#multifurcation,#duplicated_node\n";
                    check_csv.open(QIODevice::WriteOnly|QIODevice::Text);
                    check_csv.write(head.toAscii());
                }
                else{
                    check_csv.open(QIODevice::Text|QIODevice::Append);
                }
                QString line = neuron_name+","+QString::number(rootlist.size())+","+\
                        QString::number(multiF_list.size())+","+QString::number(dup_node_list.size())+"\n";
                check_csv.write(line.toAscii());
                check_csv.close();
            }
        }
    }
}
