#include "type_check.h"
#include "morphoqc_func.h"
#include "soma_confirmation.h"
using namespace std;

QList<int> mark_type_error(QString in_name, int apical_soma_threshold,vector<char*> outfiles){
    NeuronTree nt = readSWC_file(in_name);
    int N = nt.listNeuron.size();
    cout<<"file size:"<<N<<endl;
    QList<int> nlist;
    bool *visited = new bool [nt.listNeuron.size()];
    for(int m=0; m<N; m++){
        visited[m]=false;
        nlist.push_back(nt.listNeuron.at(m).n);
    }
    QVector< QList<int> > children;
    children = QVector< QList<int> >(N, QList<int>() );
    for (V3DLONG k=0;k<N;k++)
    {
        int pid = nlist.indexOf(nt.listNeuron.at(k).pn);
        if (pid<0) continue;
        children[pid].push_back(k);
    }

    // soma check & confirmation
    QList<int> soma_id;
    int degree_soma=0;
    int soma_degree_max=-1;
    for(int i=0; i<N; i++){
        if((nt.listNeuron.at(i).pn==-1)&(nt.listNeuron.at(i).type==1)){
            soma_id.append(i);
        }
        if(nt.listNeuron.at(i).type==-1){
            if(degree_soma<children[i].size()){
                degree_soma = children[i].size();
                soma_degree_max = i;
             }
        }
    }
    int soma_final;
    if(soma_id.size()>1){
        v3d_msg(QString("More than 1 soma found.\n"
                        "Please check %1").arg(in_name));
        soma_final = soma_degree_max;
    }
    else if(soma_id.size()==0){
        v3d_msg(QString("No soma found.\n"
                        "Please check %1").arg(in_name));
        soma_final = soma_degree_max;
    }
    else{
        soma_final = soma_id.at(0);
    }
    cout<<"soma id:"<<soma_final<<endl;

    // apical confirm
    bool flag_apical_found = false;
    bool flag_apical = false;
    int count_apical_B = 0;
    for(int i=0; i<N; i++){
        if(nt.listNeuron.at(i).type==4){
            double d = computeDist(nt.listNeuron.at(soma_final),nt.listNeuron.at(i));
            if(d<apical_soma_threshold){
                flag_apical_found=true;
            }
            if(children[i].size()>1){
                count_apical_B++;
            }
        }
    }
    if((count_apical_B>1)&&(flag_apical_found)){
        flag_apical=true;
    }
    cout<<"apical:"<<flag_apical_found<<" "<<count_apical_B<<endl;
    cout<<"exist apical:"<<flag_apical<<endl;

    int axon_start_id = -1;
    int apical_start_id = -1;
    QList<int> queue, wrong_type_list;
    int count_axon = 0;
    QList<int> color_range;
    color_range<<1<<2<<3<<4;
    NeuronTree nt_new;
    bool flag_change = false;
    nt_new.deepCopy(nt);
    for(int i=0; i<soma_id.size();i++){
        int root_id = soma_id[i];
        visited[root_id]=true;
        queue.push_back(root_id);

        int current_id;
        while(!queue.empty()){
            current_id = queue.front();
            queue.pop_front();
            int current_type = nt_new.listNeuron.at(current_id).type;

            //soma
            if(current_id == soma_final){
                QList<int> apical_start;
                for(int j=0; j<children[current_id].size(); j++){
                    int id = children[current_id][j];
                    if(!visited[id]){
                        visited[id] = true;
                        queue.push_back(id);

                        if(nt_new.listNeuron.at(id).type==4){
                            apical_start.push_back(id);
                        }
                        else if(nt_new.listNeuron[id].type==2){
                            if(axon_start_id == -1){
                                axon_start_id = id;
                                count_axon++;
                            }
                        }
                        else if(nt_new.listNeuron.at(id).type!=3){
                            wrong_type_list.append(id);
                        }
                    }
                }
                if(flag_apical){
                    if(apical_start.size()>1){
                        v3d_msg(QString("Apical starting point marked wrong.\n"
                                        "Please check %1").arg(in_name));
                    }
                else if (apical_start.size()<1){
                    v3d_msg(QString("Apical needs recheck.\n"
                                    "Please check %1").arg(in_name));
                }
                else{
                    apical_start_id = apical_start.at(0);
                }
            }
            }
            // basal
            else if(current_type == 3){
                for(int j=0; j<children[current_id].size();j++){
                    int id = children[current_id][j];
                    int child_type = nt_new.listNeuron.at(id).type;
                    if(!visited[id]){
                        visited[id] = true;
                        queue.push_back(id);
                        if(child_type == 2){
                            if(axon_start_id!=-1){
                                wrong_type_list.append(id);
                                count_axon++;
                            }
                            else{
                                axon_start_id = id;
                            }
                        }
                        else if(child_type!=3){
                            //wrong_type_list.append(id);
                            nt_new.listNeuron[id].type = 3;
                            flag_change=true;
                        }
                    }
                }
            }
            // axon
            else if(current_type == 2){
                for(int j=0; j<children[current_id].size(); j++){
                    int id = children[current_id][j];
                    int child_type = nt_new.listNeuron.at(id).type;
                    if(!visited[id]){
                        visited[id] = true;
                        queue.push_back(id);
                        if(child_type !=2){
                            if( !color_range.contains(child_type) ){
                                nt_new.listNeuron[id].type = 2;
                                flag_change=true;
                            }
                            else{
                                wrong_type_list.append(id);
                            }
                        }
                    }
                }
            }
            //apical
            else if(current_type == 4){
                for(int j=0; j<children[current_id].size();j++){
                    int id = children[current_id][j];
                    int child_type = nt_new.listNeuron.at(id).type;
                    if(!visited[id]){
                        visited[id] = true;
                        queue.push_back(id);
                        if(flag_apical){
                            if(child_type !=4){
                                wrong_type_list.append(id);
                            }
                        }
                        else{
                            wrong_type_list.append(id);
                        }
                    }
                }
            }
            else {
                int pre_color = nt_new.listNeuron[nlist.lastIndexOf(nt_new.listNeuron.at(current_id).pn)].type;
                for(int j=0; j<children[current_id].size();j++){
                    int id = children[current_id][j];
                    int child_type = nt_new.listNeuron.at(id).type;
                    if(!visited[id]){
                        visited[id] = true;
                        queue.push_back(id);
                        QList<int> colorlist;
                        colorlist<<2<<3;
                        if(child_type==4){
                            if(!flag_apical){
                                wrong_type_list.append(id);
                            }
                        }
                        else if(!colorlist.contains(child_type)){
                            //wrong_type_list.append(id);
                            nt_new.listNeuron[id].type=pre_color;
                            flag_change=true;
                        }
                    }
                }
            }
        }
    }
//    if(flag_change){
//        QString out_file;
//        if(outfiles.size() == 0){
//            out_file = in_name+"_retype.swc";
//        }
//        else{
//            out_file = outfiles.at(0);
//        }

//        writeSWC_file(out_file,nt_new);
//    }
    return wrong_type_list;
}


void Type_check(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >=1) outfiles = *((vector<char*> *)output.at(0).p);

    QString in_name = infiles.at(0);
    QFileInfo fileinfo(in_name);
    int apical_soma_threshold = 50;

    if(fileinfo.isFile()){
        QList<int> wrong_type_list;
        wrong_type_list = mark_type_error(in_name, apical_soma_threshold,outfiles);
        cout<<"Total error:"<<wrong_type_list.size()<<endl;
        if(wrong_type_list.size()>0){
            QList<int> color;
            color<<255<<0<<0;
            list_to_apo(wrong_type_list,in_name,color);
        }
    }
    else{
        cout<<"folder:"<<in_name.toStdString()<<endl;
        QDir dir(in_name);
        QStringList nameFilters;
        nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
        QStringList swclist = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        QString csv_out = fileinfo.absolutePath()+"/type_error.csv";
        QFile check_csv(csv_out);
        for (int i=0; i<swclist.size(); i++){
            QString swc_file = in_name+"/"+swclist.at(i);
            QList<int> wrong_type_list;
            wrong_type_list = mark_type_error(swc_file, apical_soma_threshold,outfiles);
            cout<<"Total error:"<<wrong_type_list.size()<<endl;
            if(wrong_type_list.size()>0){
                QList<int> color;
                color<<255<<0<<0;
                list_to_apo(wrong_type_list,swc_file,color);

                QFileInfo swcInfo(swc_file);
                QString neuron_name = swcInfo.completeBaseName();

                if(!check_csv.exists()){
                    QString head = "neuron_name,#type_error\n";
                    check_csv.open(QIODevice::WriteOnly|QIODevice::Text);
                    check_csv.write(head.toAscii());
                }
                else{
                    check_csv.open(QIODevice::Text|QIODevice::Append);
                }
                QString line = neuron_name+","+QString::number(wrong_type_list.size())+"\n";
                check_csv.write(line.toAscii());
                check_csv.close();
            }
        }
    }
}
