
#include "Node_type_correction_func.h"

QVector< QVector<V3DLONG> > get_children(NeuronTree nt){
    QVector< QVector<V3DLONG> > children;
    QList<NeuronSWC> n_list = nt.listNeuron;
    for(int i=0; i<n_list.size(); i++){
        QVector<V3DLONG> empty_vector;
        empty_vector.clear();
        children.append(empty_vector);
    }
    for(int i=0; i<n_list.size(); i++){
        int parent = n_list.at(i).pn;
        if(parent>=0){
            children[nt.hashNeuron.value(parent)].append(i);
        }
    }
    return children;
}

int get_root(NeuronTree nt){
    QList<NeuronSWC> n_list = nt.listNeuron;
    int root;
    for(int i=0; i<n_list.size(); i++){
        if(n_list.at(i).pn == -1){
            root = i;
            return root;
        }
    }
    v3d_msg("No root node found!");
    return -1;
}

void node_type_correction(V3DPluginCallback2 &callback, QWidget *parent)
{
    // This plugin does the following:
    // 1. Find swc nodes whose types are invalid (valid types: 1 (soma), 2 (axon), 3 (basal dendrite), 4 (apical dendrite) )
    // 2. Make corrections when possible, otherwise label them
    // 3. Generate a report of node type counts.

    //choose a directory that contain swc files
    QString qs_dir_swc;
    qs_dir_swc=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the directory that contains the input SWC/ESWC files")));
    //choose a directory that save swc files
    QString output_dir_swc;
    output_dir_swc=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the directory that save the output SWC/ESWC files.")));
    QDir dir(qs_dir_swc);
    QStringList qsl_filelist,qsl_filters;
    qsl_filters+="*.swc";
    qsl_filters+="*.eswc";

    foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
    {
        qsl_filelist+=file;
    }

    if(qsl_filelist.size()==0)
    {
        v3d_msg("Cannot find the respective files in the given directory!\nTry another diretory");
        return;
    }

    QString output_swc;
    QVector<NeuronTree> all_tree;
    for(V3DLONG i=0;i<qsl_filelist.size();i++)
    {

        QList<int> valid_types;
        valid_types << 2 << 3 << 4;

        QString input_swc=qs_dir_swc+"/"+qsl_filelist.at(i);
        NeuronTree nt = readSWC_file(input_swc);
        QVector< QVector<V3DLONG> > children = get_children(nt);
        int root = get_root(nt);
        if(root < 0){
            return;
        }

        // DFS
        QStack<int> pstack;
        QList<bool> visited;       
        for(int i=0; i<nt.listNeuron.size(); i++){visited.append(false);}

        // Begin
        pstack.push(root);
        visited[root] = true;
        if(nt.listNeuron.at(root).type!=1){nt.listNeuron[root].type = 0;}

        int cur_type = 0;

        // DFS search
        while(!pstack.empty()){
            bool new_push = false;
            int cur_node = pstack.top();
            //        qDebug()<<"cur_node: "<<cur_node;
            QVector<V3DLONG>::iterator it;
            QVector<V3DLONG> cur_children = children.at(cur_node);
            for(it=cur_children.begin(); it!=cur_children.end(); ++it)
            {
                cur_type = nt.listNeuron.at(cur_node).type;
                if(!visited.at(*it)){
                    pstack.push(*it);
                    new_push = true;
                    visited[*it] = true;
                    if(!valid_types.contains(nt.listNeuron.at(*it).type)){nt.listNeuron[*it].type = cur_type;}
                    else{
                        cur_type = nt.listNeuron.at(*it).type;
                    }
                    break;
                }
            }
            if(!new_push){
                int cur_node = pstack.pop();
                if((!valid_types.contains(nt.listNeuron.at(cur_node).type)) && (cur_node != root)){
                    QVector<V3DLONG>::iterator it;
                    QVector<V3DLONG> cur_children = children.at(cur_node);
                    for(it=cur_children.begin(); it!=cur_children.end(); ++it)
                    {
                        if(valid_types.contains(nt.listNeuron.at(*it).type)){
                            nt.listNeuron[cur_node].type = nt.listNeuron.at(*it).type;
                            break;
                        }
                    }
                }
            }
        }
        all_tree.append(nt);
        //export swc file
        output_swc=output_dir_swc+"/"+qsl_filelist.at(i);
        writeSWC_file(output_swc, nt);
    }
    // To be implemented: Display number of node types, e.g. types: 1(1), 2(500), 3(200), 4(50), others (0)
    QVector< QPair< QString, QVector<int> > > all_swc_name_type;
    for (int ii=0;ii<all_tree.size();ii++)
    {
        QPair<QString, QVector<int> > single_swc_name_type;
        QVector<int> typenum_single_swc;typenum_single_swc.clear();
        int type1=0,type2=0,type3=0,type4=0,type_others=0;
        for(int i=0; i<all_tree.at(ii).listNeuron.size(); i++)
        {
            if(all_tree.at(ii).listNeuron.at(i).type==1) type1++;
            else if(all_tree.at(ii).listNeuron.at(i).type==2) type2++;
            else if(all_tree.at(ii).listNeuron.at(i).type==3) type3++;
            else if(all_tree.at(ii).listNeuron.at(i).type==4) type4++;
            else type_others++;
        }
        typenum_single_swc.append(type1);typenum_single_swc.append(type2);typenum_single_swc.append(type3);
        typenum_single_swc.append(type4);typenum_single_swc.append(type_others);
        single_swc_name_type.first=qsl_filelist.at(ii);
        single_swc_name_type.second=typenum_single_swc;
        all_swc_name_type.append(single_swc_name_type);
    }


    QString info_tree;
    for (int i=0; i<MIN(all_swc_name_type.size(),30);i++)
    {
        info_tree += QString("%1 :1(%2),2(%3),3(%4),4(%5),others(%6)\n").arg(all_swc_name_type.at(i).first.split("_")[0]+"_"+all_swc_name_type.at(i).first.split("_")[1])
                .arg(all_swc_name_type.at(i).second.at(0)).arg(all_swc_name_type.at(i).second.at(1)).arg(all_swc_name_type.at(i).second.at(2))
                .arg(all_swc_name_type.at(i).second.at(3)).arg(all_swc_name_type.at(i).second.at(4));
    }
    if(all_swc_name_type.size()>30)
        info_tree += ". . .\n";

    QMessageBox infoBox;
    infoBox.setText("Neuron type:");
    infoBox.setInformativeText(QString("<pre><font size='4'>"
                "number of neuron-trees : %1<br>"
                "%2<br>")
                .arg(all_swc_name_type.size())
                .arg(info_tree.toStdString().c_str()));
    infoBox.exec();
    //show message box
     if(1)
          v3d_msg(QString("Save the output swc. files to: \n\n%1\n\nComplete!").arg(output_dir_swc));
     else
          v3d_msg(QString("Save the output swc. files to: \n\n%1\n\nComplete!").arg(output_dir_swc), 0);

    return;
}
