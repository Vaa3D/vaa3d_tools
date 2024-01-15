#include "soma_confirmation.h"

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
using namespace std;

XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}

QList< CellAPO> read_table(const QString& filename){
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
    QList <CellAPO> soma_list;
    for (int i=0; i<tempOption.count(); i++){
        QStringList temp = tempOption.at(i).split(sep);
        CellAPO line;
        line.orderinfo=temp.at(0);
        line.x=temp.at(1).toDouble();
        line.y=temp.at(2).toDouble();
        line.z=temp.at(3).toDouble();
        if(temp.size()>4){
            line.mass=temp.at(4).toDouble();
        }
        soma_list.append(line);
    }
    return soma_list;
}

NeuronTree remove_nodes_soma(NeuronTree nt, double radius_threshold, NeuronSWC soma){
    //remove nodes in the volume of soma
    QList<int> del_list;
    int N=nt.listNeuron.size();
    int tmp_soma;

    for (int i=0; i<N; i++){
        double d = computeDist(nt.listNeuron.at(i),soma);
        if ( d < radius_threshold) {
            del_list.append(i);
            if(nt.listNeuron.at(i).pn == -1){
                tmp_soma=i;
            }
        }
    }

    if (del_list.size() == 0){
        return nt;
    }

    QList <int> plist,nlist;
    for (int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        nlist.append(nt.listNeuron.at(i).n);
    }

    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(N, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<N;i++)
    {
        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    // remove & replace
    NeuronTree nt_new;
    // parent=-1 or max children size
    int max_degree=0, max_id=0;
    for(int i=0; i<del_list.size(); i++){
        int num = children.at(del_list.at(i)).size();
        if(num>max_degree){
            max_id = del_list.at(i);
            max_degree = num;
        }
    }
    if(max_degree == 2){
        max_id = tmp_soma;
    }

    QList<int> removed_nodes;
    removed_nodes.append(max_id);
    for (int i=0; i<children[max_id].size(); i++){
        int start_id = children[max_id].at(i);
        while((children[start_id].size()<2)&&(del_list.contains(start_id))){
            removed_nodes.append(start_id);
            start_id = children[start_id].at(0);
        }
        nt.listNeuron[start_id].parent = nt.listNeuron[max_id].n;
    }

    nt_new.listNeuron.append(soma);
    for(int i=0; i<N; i++){
        if(!removed_nodes.contains(i)){
            nt_new.listNeuron.append(nt.listNeuron.at(i));
        }
    }
    return nt_new;
}

QList<int> soma_extract(QString swc_file, bool flag_check){
    // 1. whether there exists a soma
    NeuronTree nt = readSWC_file(swc_file);
    int N = nt.listNeuron.size();
    QList<int> soma_ids;
    for (int i=0; i<N; i++){
        if ((nt.listNeuron.at(i).parent==-1) && (nt.listNeuron.at(i).type==1)){
            soma_ids.append(i);
        }
    }
    if (flag_check){
        if(soma_ids.size()==0){
            cout<<QString("Soma not defined.Please check %1").arg(swc_file).toStdString()<<endl;;
        }
        else if(soma_ids.size() > 1){
            cout<<QString("More than 1 soma found.Please check %1").arg(swc_file).toStdString()<<endl;
        }
    }
    return soma_ids;
}

void list_to_apo(QList<int> point_list, QString swc_file, QList<int> color_mark){
    QList<CellAPO> apo_list;
    NeuronTree swc = readSWC_file(swc_file);
    for(int i=0; i<point_list.size(); i++){
        CellAPO m;
        m.x = swc.listNeuron.at(point_list.at(i)).x;
        m.y = swc.listNeuron.at(point_list.at(i)).y;
        m.z = swc.listNeuron.at(point_list.at(i)).z;
        m.color.r = color_mark[0];
        m.color.g = color_mark[1];
        m.color.b = color_mark[2];
        m.volsize = 50;
        apo_list.push_back(m);
    }

    QFileInfo fileinfo(swc_file);
    QString apo_file = fileinfo.absolutePath()+"/"+fileinfo.completeBaseName()+".apo";
    writeAPO_file(apo_file,apo_list);
}

unsigned char * crop_soma_block(NeuronSWC soma, QString image_file, V3DLONG mysz[], V3DPluginCallback2 & callback){
    V3DLONG *in_zz;
    if(!callback.getDimTeraFly(image_file.toStdString(), in_zz))
    {
//        v3d_msg("Cannot load terafly images.",0);
        cout<<"can not load terafly images"<<endl;
    }
    XYZ block_size=XYZ(100,100,50);
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    block crop_block = offset_block(zcenter_block, XYZ(soma.x, soma.y, soma.z));
    XYZ small=XYZ(crop_block.small);
    XYZ large=XYZ(crop_block.large);
    small.x = floor(small.x);
    small.y = floor(small.y);
    small.z = floor(small.z);
    large.x = ceil(large.x)+1;
    large.y = ceil(large.y)+1;
    large.z = ceil(large.z)+1;
    unsigned char * cropped_image = 0;
    cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                               small.x, large.x,
                                                 small.y, large.y,
                                                 small.z, large.z);
    mysz[0] = large.x-small.x;
    mysz[1] = large.y-small.y;
    mysz[2] = large.z-small.z;
    mysz[3] = in_zz[3];
    return cropped_image;
}

NeuronSWC calc_mean_shift_center(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre,int windowradius)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    V3DLONG pos;
//    vector<V3DLONG> coord;
    //cout<<"image"<<sz_image[0]<<","<<sz_image[1]<<","<<sz_image[2]<<","<<endl;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    //float center_dis=1;
    vector<float> center_float(3,0);

    x=snode.x;y=snode.y;z=snode.z;
    //cout<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<endl;
    int windowradius1 = 2;
    int thresHold = bkg_thre;//最佳二值化阈值
    int v_color_mean=0;
    int c=0;
    //cout<<"*****"<<MAX(x+0.5-windowradius1,0)<<endl;
    //cout<<"*****"<<MIN(sz_image[0]-1,x+0.5+windowradius1)<<endl;
    for(V3DLONG dx=MAX(x+0.5-windowradius1,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius1); dx++){
        //cout<<"in1"<<endl;
        for(V3DLONG dy=MAX(y+0.5-windowradius1,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius1); dy++){
            //cout<<"in2"<<endl;
            for(V3DLONG dz=MAX(z+0.5-windowradius1,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius1); dz++){
                //cout<<"in3"<<endl;
                     pos=dz*z_offset+dy*y_offset+dx;
                     v_color=inimg1d[pos];
                        //hist[v_color]+=1;
                        v_color_mean+=v_color;
                        //cout<<"Mean:"<<v_color<<endl;
                        c+=1;
            }
        }
     }

    //v_color_mean=bkg_thre;
    v_color_mean= v_color_mean/c;
    cout<<"mean:"<<v_color_mean<<endl;
    if(v_color_mean<14){
        //cout<<"return1"<<endl;
        return snode;
    }
//    thresHold=MAX(40,2*(v_color_mean-31)+40);
//    thresHold=MIN(thresHold,60);
    //find out the channel with the maximum intensity for the marker
    v_prev=inimg1d[long(z)*z_offset+long(y)*y_offset+long(x)];
    int testCount=0;
    int testCount1=0;
    //cout<<"step1"<<endl;
    while (testCount<1) //center_dis>=0.5 &&
    {
        total_x=total_y=total_z=sum_v=0;
        testCount++;
        testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=dz*z_offset+dy*y_offset+dx;
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=inimg1d[pos];
                    if(v_color<thresHold)
                        v_color=0;
                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    //cout<<v_color<<"%%%%%"<<endl;
                    //testCount++;
                    if(v_color>100)
                        testCount1++;
                 }
             }
         }

        center_float[0]=total_x/sum_v;
        center_float[1]=total_y/sum_v;
        center_float[2]=total_z/sum_v;
        //cout<<"result:"<<center_float[0]<<","<<center_float[1]<<","<<center_float[2]<<","<<endl;

        if (total_x<1e-5||total_y<1e-5||total_z<1e-5) //a very dark marker.
        {

//            v3d_msg("Sphere surrounding the marker is zero. Mean-shift cannot happen. Marker location will not move",0);
            center_float[0]=x;
            center_float[1]=y;
            center_float[2]=z;
            return snode;
        }

//        float tmp_1=(center_float[0]-x)*(center_float[0]-x)+(center_float[1]-y)*(center_float[1]-y)
//                    +(center_float[2]-z)*(center_float[2]-z);
//        center_dis=sqrt(tmp_1);
        x=center_float[0]; y=center_float[1]; z=center_float[2];
    }

    NeuronSWC out_center=snode;
    out_center.x=center_float[0]; out_center.y=center_float[1]; out_center.z=center_float[2];
    out_center.level=inimg1d[long(out_center.z)*z_offset+long(out_center.y)*y_offset+long(out_center.x)];
//    if(out_center.level>v_prev)
//        cout<<"shift intensity improve= "<<out_center.level-v_prev<<endl;
    return out_center;
}

NeuronTree shift_soma(NeuronTree nt, QString image_file,V3DPluginCallback2 & callback){
    int soma_id;
    int N = nt.listNeuron.size();
    for(int i=0; i<N; i++){
        if((nt.listNeuron.at(i).type==1)&&(nt.listNeuron.at(i).pn==-1)){
            soma_id = i;
            LocationSimple soma_coord;
            soma_coord.x = nt.listNeuron.at(i).x;
            soma_coord.y = nt.listNeuron.at(i).y;
            soma_coord.z = nt.listNeuron.at(i).z;
            break;
        }
    }
    NeuronSWC soma = nt.listNeuron.at(soma_id);
    cout<<"org node"<<soma.x<<" "<<soma.y<<" "<<soma.z<<endl;

    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(image_file.toStdString(),in_zz)){cout<<"can't load terafly img"<<endl;}
    long start_x,start_y,start_z,end_x,end_y,end_z,half_block_size;
    half_block_size = 50;
    start_x = soma.x - half_block_size; if(start_x<0) start_x = 0;
    end_x = soma.x + half_block_size; if(end_x >= in_zz[0]) end_x = in_zz[0]-1;
    start_y =soma.y - half_block_size;if(start_y<0) start_y = 0;
    end_y = soma.y + half_block_size;if(end_y >= in_zz[1]) end_y = in_zz[1]-1;
    start_z = soma.z - half_block_size;if(start_z<0) start_z = 0;
    end_z = soma.z + half_block_size;if(end_z >= in_zz[2]) end_z = in_zz[2]-1;
    unsigned char * cropped_img_soma = 0;
    cropped_img_soma = callback.getSubVolumeTeraFly(image_file.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
    V3DLONG mysz[4];
    mysz[0] = end_x - start_x+1;
    mysz[1] = end_y - start_y+1;
    mysz[2] = end_z - start_z+1;
    mysz[3]=in_zz[3];

    int ms_windowradius = 20;
    NeuronSWC new_soma,shifted_soma;
    shifted_soma.x = soma.x-start_x;shifted_soma.y = soma.y-start_y; shifted_soma.z = soma.z-start_z;
    double bkg_thre = 30;
    new_soma = calc_mean_shift_center(cropped_img_soma,shifted_soma,mysz, bkg_thre, ms_windowradius);
    cout<<"new node"<<new_soma.x<<" "<<new_soma.y<<" "<<new_soma.z<<endl;

//    QList<CellAPO> apo;
//    CellAPO m;
//    m.x = new_soma.x+float(start_x);
//    m.y = new_soma.y+float(start_y);
//    m.z = new_soma.z+float(start_z);
//    m.color.r = 255;
//    m.color.g = 255;
//    m.color.b = 0;
//    m.volsize = 50;
//    apo.push_back(m);
//    QString apo_file = "/home/penglab/Desktop/MyFiles/Projects/BBP/116/BBP_116_manual_final_defined_soma/1.apo";
//    writeAPO_file(apo_file,apo);
    nt.listNeuron[soma_id].x = new_soma.x+float(start_x);
    nt.listNeuron[soma_id].y = new_soma.y+float(start_y);
    nt.listNeuron[soma_id].z = new_soma.z+float(start_z);
    return nt;
}

void Soma_pos_correct(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    QString in_name = infiles.at(0);
    QFileInfo fileinfo(in_name);

    QList<int> soma_list;
    if (fileinfo.isFile()){
        QString swc_file = in_name;
        QString image_file = infiles.at(1);
        NeuronTree nt = readSWC_file(swc_file);
        NeuronTree new_nt;
        new_nt = shift_soma(nt,image_file,callback);
        writeSWC_file(swc_file,new_nt);
    }
}


void Soma_check(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    cout<<"infile size="<<infiles.size()<<endl;
    QString in_name = infiles.at(0);
    cout<<in_name.toStdString()<<endl;

    QFileInfo fileinfo(in_name);

    QList<int> soma_list;
    if (fileinfo.isFile()){
        cout<<"read file:"<<in_name.toStdString()<<endl;
        QString swc_file = in_name;
        NeuronTree nt = readSWC_file(swc_file);
        QString image_file = infiles.at(1);
        //shift_soma(nt, image_file,callback);
        soma_list = soma_extract(swc_file,TRUE);
        if (soma_list.size()>1){
            QList<int> marker_color;
            marker_color<<255<<255<<0;
            list_to_apo(soma_list, swc_file,marker_color);
        }

    }
    else{
        cout<<"read folder:"<<in_name.toStdString()<<endl;
        QDir dir(in_name);
        QStringList nameFilters;
        nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
        QStringList swclist = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        // save to csv
        QString csv_out = fileinfo.absolutePath()+"/soma_info.csv";
        for (int i=0; i<swclist.size(); i++){
            QString swc_file = in_name+"/"+swclist.at(i);
            soma_list = soma_extract(swc_file,FALSE);
            //cout<<"neuron:"<<swclist.at(i).split(".v3d")[0].toStdString()<<endl;
            if (soma_list.size()>1){
                QList<int> marker_color;
                marker_color<<255<<255<<0;
                list_to_apo(soma_list, swc_file,marker_color);

                QFileInfo swcInfo(swc_file);
                QString neuron_name = swcInfo.completeBaseName();
                QFile check_csv(csv_out);
                if(!check_csv.exists()){
                    QString head = "neuron_name,#soma\n";
                    check_csv.open(QIODevice::WriteOnly|QIODevice::Text);
                    check_csv.write(head.toAscii());
                }
                QString line = neuron_name+","+QString::number(soma_list.size())+"\n";
                check_csv.write(line.toAscii());
                check_csv.close();
            }
        }
    }
}

int search_for_soma(NeuronTree nt, QList<int> soma_candidates){

    QList<int> nlist,recordlist;
    QVector<QVector<V3DLONG> > children;
    children = QVector< QVector<V3DLONG> >(nt.listNeuron.size(), QVector<V3DLONG>() );
    for (V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        nlist.append(nt.listNeuron.at(i).n);
    }
    for (V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        int pid = nlist.lastIndexOf(nt.listNeuron.at(i).pn);
        if (pid<0) continue;
        children[pid].push_back(i);
    }

    int max_degree=0;
    for (int i=0; i<soma_candidates.size(); i++){
        if(children[soma_candidates[i]].size()>max_degree){
            recordlist.clear();
            recordlist.append(soma_candidates.at(i));
            max_degree=children[soma_candidates[i]].size();
        }
        else if(children[soma_candidates[i]].size()==max_degree){
            recordlist.append(soma_candidates[i]);
        }
    }
    if(recordlist.size()==1){
        return recordlist.at(0);
    }
    else{
        double d = 0;
        for (int i=0; i<recordlist.size()-1; i++){
            for (int j=i+1; j<recordlist.size(); j++){
                double current_d = computeDist(nt.listNeuron.at(recordlist[i]),nt.listNeuron.at(recordlist[j]));
                if(current_d > d){
                    d = current_d;
                }
            }
        }
        if (d<30){
            return recordlist.at(0);
        }
        else{
            return -1;
        }
    }
}

NeuronSWC soma_correction_with_somaInfo(QString swc_file,QString soma_file){
    QString neuron_id = QFileInfo(swc_file).completeBaseName();

    NeuronSWC org_soma;
    QList< CellAPO> soma_markers;
    // APO file
    if (soma_file.endsWith(".apo")){
        soma_markers = readAPO_file(soma_file);
    }
    else{
        soma_markers = read_table(soma_file);
    }
    if (soma_markers.isEmpty()){
        v3d_msg(QString("Soma file is empty.\n"
                        "Process terminated.\n"
                        "Please double check %1").arg(soma_file));
    }
    else{
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
            v3d_msg(QString("Soma info is not in the given soma file.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(soma_file));
        }
        else{
            org_soma.x = soma_markers.at(matched_soma_id).x;
            org_soma.y = soma_markers.at(matched_soma_id).y;
            org_soma.z = soma_markers.at(matched_soma_id).z;
            org_soma.r = soma_markers.at(matched_soma_id).mass;
            org_soma.comment = "found";
        }
    }
    return org_soma;
}

bool Soma_correction(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output){
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString in_name = infiles.at(0);
    QFileInfo fileinfo(in_name);

    if (fileinfo.isFile()){
    // -------------------Case1. Assigned soma information is given-------------------------
        NeuronTree nt = readSWC_file(in_name);
        QList<int> plist;
        for(int j=0; j<nt.listNeuron.size(); j++){
            plist.append(j);
        }
        QString para_file = inparas.at(0);
        QString out_file;
        if(outfiles.size() == 0){
            out_file = in_name + "_somaCorrected.swc";
        }
        else{
            out_file = outfiles.at(0);
        }

        QDir dir(para_file);
        if (!dir.exists()){
            QString soma_file = para_file;
            NeuronSWC soma_extracted = soma_correction_with_somaInfo(in_name,soma_file);
            if(soma_extracted.comment=="found"){
                NeuronTree new_swc_nt = remove_nodes_soma(nt, 10.0,soma_extracted);
                writeSWC_file(out_file,new_swc_nt);
            }
        }
        // -------------------Case2. Search for soma -------------------------
        else {
            QList<int> soma_list = soma_extract(in_name,FALSE);
            if(soma_list.size()!=1){
                // find a new one around the point
                int soma_id_found = search_for_soma(nt, soma_list);
                if(soma_id_found == -1){
                    v3d_msg(QString("Multiple soma found.\n"
                                    "Correction failed.\n"
                                    "Please manually check %1").arg(in_name));
                }
                else{
                    QString image_file = para_file;
                    NeuronSWC soma = nt.listNeuron.at(soma_id_found);
                    V3DLONG *in_zz = 0;
                    if(!callback.getDimTeraFly(image_file.toStdString(),in_zz)){cout<<"can't load terafly img"<<endl;}
                    long start_x,start_y,start_z,end_x,end_y,end_z,half_block_size;
                    half_block_size = 50;
                    start_x = soma.x - half_block_size; if(start_x<0) start_x = 0;
                    end_x = soma.x + half_block_size; if(end_x >= in_zz[0]) end_x = in_zz[0]-1;
                    start_y =soma.y - half_block_size;if(start_y<0) start_y = 0;
                    end_y = soma.y + half_block_size;if(end_y >= in_zz[1]) end_y = in_zz[1]-1;
                    start_z = soma.z - half_block_size;if(start_z<0) start_z = 0;
                    end_z = soma.z + half_block_size;if(end_z >= in_zz[2]) end_z = in_zz[2]-1;
                    unsigned char * cropped_img_soma = 0;
                    cropped_img_soma = callback.getSubVolumeTeraFly(image_file.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);

                    V3DLONG mysz[4];
                    mysz[0] = end_x - start_x+1;
                    mysz[1] = end_y - start_y+1;
                    mysz[2] = end_z - start_z+1;
                    mysz[3]=in_zz[3];

                    int ms_windowradius = 20;
                    NeuronSWC new_soma,shifted_soma;
                    shifted_soma.x = soma.x-start_x;shifted_soma.y = soma.y-start_y; shifted_soma.z = soma.z-start_z;
                    double bkg_thre = 30;
                    new_soma = calc_mean_shift_center(cropped_img_soma,shifted_soma,mysz, bkg_thre, ms_windowradius);
                    QList<NeuronSWC> new_nt, new_nt_sorted;
                    for(int i=0; i<nt.listNeuron.size(); i++){
                        if(soma_list.contains(i)){
                            if (i == soma_id_found){
                                new_nt.append(new_soma);
                            }
                            else{
                                NeuronSWC tmp = nt.listNeuron[i];
                                int tmp_type = nt.listNeuron[i].type;
                                int current_id = i;
                                while(tmp_type!=1){
                                    current_id = plist.lastIndexOf(nt.listNeuron.at(current_id).pn);
                                    tmp_type = nt.listNeuron[current_id].type;
                                }
                                tmp.type = tmp_type;
                                new_nt.append(tmp);
                            }
                        }
                        else{
                            new_nt.append(nt.listNeuron.at(i));
                        }
                    }
                    SortSWC(new_nt,new_nt_sorted,VOID,VOID);
                    export_list2file(new_nt_sorted, out_file, in_name);
                 }
            }
        }

    }
//    else{
//        QDir dir(in_name);
//        QStringList nameFilters;
//        nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
//        QStringList swclist = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);
//    }
}

