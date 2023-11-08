#include "morphoqc_func.h"

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
using namespace std;

double computeDist(const NeuronSWC & p1, const NeuronSWC & p2){
    double xx = p1.x - p2.x;
    double yy = p1.y - p2.y;
    double zz = p1.z - p2.z;
    return sqrt(xx*xx+yy*yy+zz*zz);
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
    QFile ano_content(ano_filsoma_radius_givene);
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

QList< point_info> read_table(const QString& filename){
    QFile file(filename);
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
    }
    QTextStream *out = new QTextStream(&file);
    QStringList tempOption = out->readAll().split("\n");
    QString sep;
    if (filename.endsWith("txt")){
        sep = " ";
    }
    else if (filename.endsWith("csv")){
        sep = ",";
    }
    QList <point_info> soma_list;
    for (int i=0; i<tempOption.count(); i++){
        QStringList temp = tempOption.at(i).split(sep);
        point_info line;
        line.name=temp.at(0);
        line.x=temp.at(1).toDouble();
        line.y=temp.at(2).toDouble();
        line.z=temp.at(3).toDouble();
        line.r=temp.at(4).toDouble();
        soma_list.append(line);
    }
    return soma_list;
}

NeuronTree remove_nodes(NeuronTree nt, QList<int> list){
    if (list.size() == 0){
        return nt;
    }
    QList <int> nlist;
    int n = nt.listNeuron.size();
    for (int i=0; i<n; i++){
        nlist.append(nt.listNeuron.at(i).n);
    }
    int N = list.size();
    QList <int> replaced_child_list;
    for (int i=0; i<N; i++){
        int parent_id = nt.listNeuron.at(list.at(i)).n;
        int child_id =
    }
}

bool Soma_correction(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    QString swc_file = infiles.at(0);
    NeuronTree nt = readSWC_file(swc_file);
    QString soma_file = inparas.at(0);
    QString neuron_id = QFileInfo(swc_file).completeBaseName();
    bool soma_radius_given = FALSE;

    // 1. check original defined soma points
    int N = nt.listNeuron.size();
    int soma_id = -1;
    for (int i=0; i<N; i++){
        if ((nt.listNeuron.at(i).parent==-1) && (nt.listNeuron.at(i).type==1)){
            soma_id = i;
            break;
        }
    }
    if (soma_id == -1){
        v3d_msg(QString("Soma not defined.\n"
                        "Please check %1").arg(swc_file));
    }

    // -------------------Case1. Assigned soma information is given-------------------------

    // 2. find matched soma position in soma list file
    NeuronSWC org_soma;
    if (soma_file.endsWith(".apo")){
        QList<CellAPO> soma_markers = readAPO_file(soma_file);
        if (soma_markers.isEmpty()){
            v3d_msg(QString("Soma file is empty.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(soma_file));
            return 0;
        }
        int N_soma=soma_markers.size();
        int matched_soma_id = -1;
        for (int i=0; i<N_soma; i++){
            bool match = neuron_id.contains(soma_markers.at(i).name);
            if (match == true){
                matched_soma_id = i;
                break;
            }
        }
        if (matched_soma_id == -1){
            v3d_msg(QString("Soma info is not in the APO file.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(soma_file));
            return 0;
        }
        org_soma.x = soma_markers.at(matched_soma_id).x;
        org_soma.y = soma_markers.at(matched_soma_id).y;
        org_soma.z = soma_markers.at(matched_soma_id).z;
        org_soma.r = soma_markers.at(matched_soma_id).mass;
        if( org_soma.r != 0 ){
            soma_radius_given = TRUE;
        }
    }
    else {
        QList< point_info> soma_markers = read_table(soma_file);
        if (soma_markers.isEmpty()){
            v3d_msg(QString("Soma file is empty.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(soma_file));
            return 0;
        }
        int N_soma = soma_markers.size();
        int matched_soma_id = -1;
        for (int i=0; i<N_soma; i++){
            bool match = neuron_id.contains(soma_markers.at(i).name);
            if (match == true){
                matched_soma_id = i;
            }
        }
        if (matched_soma_id == -1){
            v3d_msg(QString("Soma info is not in the given file.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(soma_file));
            return 0;
        }
        org_soma.x = soma_markers.at(matched_soma_id).x;
        org_soma.y = soma_markers.at(matched_soma_id).y;
        org_soma.z = soma_markers.at(matched_soma_id).z;
        org_soma.r = soma_markers.at(matched_soma_id).r;
        if( org_soma.r != 0 ){
            soma_radius_given = TRUE;
        }
    }

    // 3. remove nodes in the volume of soma
    double radius_threshold = 25;
    double soma_radius;
    QList<int> del_list;
    if (soma_radius_given == TRUE){
        soma_radius = org_soma.r;
    }
    else{
        soma_radius = radius_threshold;
    }
    for (int i=0; i<N; i++){
        if (i == soma_id)
            continue;
        double d = computeDist(nt.listNeuron.at(i),org_soma);
        if ( d < soma_radius) {
            del_list.append(i);
        }
    }


}
