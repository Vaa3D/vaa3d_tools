#include "is_analysis_fun.h"
#include "common.h"

#define MAINWINNAME "ISquantifier_Proofread"
#define TRUNC_VIEW "ISquantifier_trunc"

is_analysis_fun::is_analysis_fun(V3DPluginCallback2 *cb, QStringList name_list,QVector<int> para_list)
{
    callback=cb;
    p_img1D=0;
    mask=0;
    image_trun=0;

    param.spine_bgthr=para_list[0];
    param.is_bgthr=para_list[1];
    param.min_voxel=para_list[2];
    param.max_dis=para_list[3];
    param.max_voxel=8000;  //Apr 6th 2017 by Yujie Li

    param.halfwindowsize=30;
    basedir=name_list[2];
    swc_name=name_list[1];
    image_name=name_list[0];
}

void is_analysis_fun::run()
{
    //backupNeuron(nt,nt_copy);
    //qDebug()<<"nt copy size:"<<nt_copy.listNeuron.size();
    obtain_mask();
    connected_components();
    stat_generate();
    visualize_image();
}


void is_analysis_fun::obtain_mask()
{
    qDebug()<<"in obtain mask"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
//    V3DLONG count=0;
    V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];

    mask=new unsigned char [page_size*3];
    memset(mask,0,page_size*3);

    for(V3DLONG nid=0; nid<nt.listNeuron.size(); nid++){
        V3DLONG nx = nt.listNeuron.at(nid).x;
        V3DLONG ny = nt.listNeuron.at(nid).y;
        V3DLONG nz = nt.listNeuron.at(nid).z;
        if (nx<0 || nx>sz_img[0]-1 || ny<0 ||ny>sz_img[1]-1||nz<0 ||nz>sz_img[2]-1) //swc not in image
            continue;
        int disthr = nt.listNeuron.at(nid).radius+param.max_dis;
        for(V3DLONG x=MAX(nx-disthr,0); x<=MIN(nx+disthr,sz_img[0]-1); x++){
            for(V3DLONG y=MAX(ny-disthr,0); y<=MIN(ny+disthr,sz_img[1]-1); y++){
                for(V3DLONG z=MAX(nz-disthr,0); z<=MIN(nz+disthr,sz_img[2]-1); z++){
                    V3DLONG pos=xyz2pos(x,y,z,sz_img[0],sz_img[0]*sz_img[1]);
                    if (voxels_map.find(pos) != voxels_map.end())  //marked
                        continue;
                    mask[pos]=p_img1D[pos];
                    mask[pos+page_size]=p_img1D[pos+page_size];
                    if(p_img1D[pos]<param.spine_bgthr){ //too dark, skip
                        continue;
                    }
                    if (p_img1D[pos+page_size]<param.is_bgthr)
                        continue;
                    voxels_map[pos]=true;

                }
            }
        }
    }

    mask_sz[0]=sz_img[0];
    mask_sz[1]=sz_img[1];
    mask_sz[2]=sz_img[2];
    mask_sz[3]=3;
    qDebug()<<"count:"<<voxels_map.size();

//    QString filename="mask1.v3draw";
//    simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),mask,mask_sz,V3D_UINT8);
    if (p_img1D!=0)
    {
        delete[] p_img1D;
        p_img1D=0;
    }
}

void is_analysis_fun::connected_components()
{
    qDebug()<<"in connected components";
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    int label_id=1;

    map<V3DLONG, int> lookup;

    for(map<V3DLONG, bool>::iterator iter_map=voxels_map.begin(); iter_map!=voxels_map.end(); iter_map++)
    {
        if (voxels_map[iter_map->first]<=0)
            continue;
        //qDebug()<<iter_map->first;
        if (lookup[iter_map->first]>0)
            continue;

        vector<V3DLONG> seeds;
        seeds.clear();
        seeds.push_back(iter_map->first);
        lookup[iter_map->first]=label_id;
        int sid=0;
        while (sid<seeds.size())
        {
            //qDebug()<<"seeds size:"<<seeds.size();
            if (seeds[sid]-1>=0 && lookup[seeds[sid]-1]<=0 && voxels_map[seeds[sid]-1]>0)
            {
                seeds.push_back(seeds[sid]-1);
                lookup[seeds[sid]-1]=label_id;
            }
            if (seeds[sid]+1<size_page && lookup[seeds[sid]+1]<=0 && voxels_map[seeds[sid]+1]>0)
            {
                seeds.push_back(seeds[sid]+1);
                lookup[seeds[sid]+1]=label_id;
            }
            if (seeds[sid]-sz_img[0]>=0 && lookup[seeds[sid]-sz_img[0]]<=0 && voxels_map[seeds[sid]-sz_img[0]]>0)
            {
                seeds.push_back(seeds[sid]-sz_img[0]);
                lookup[seeds[sid]-sz_img[0]]=label_id;
            }
            if (seeds[sid]+sz_img[0]<size_page&& lookup[seeds[sid]+sz_img[0]]<=0 && voxels_map[seeds[sid]+sz_img[0]]>0)
            {
                seeds.push_back(seeds[sid]+sz_img[0]);
                lookup[seeds[sid]+sz_img[0]]=label_id;
            }
            if (seeds[sid]-z_offset>=0 && lookup[seeds[sid]-z_offset]<=0 && voxels_map[seeds[sid]-z_offset]>0)
            {
                seeds.push_back(seeds[sid]-z_offset);
                lookup[seeds[sid]-z_offset]=label_id;
            }
            if (seeds[sid]+z_offset<size_page && lookup[seeds[sid]+z_offset]<=0 && voxels_map[seeds[sid]+z_offset]>0)
            {
                seeds.push_back(seeds[sid]+z_offset);
                lookup[seeds[sid]+z_offset]=label_id;
            }
            sid++;
        }
        if (seeds.size()<param.min_voxel)
        {
            //need to make mask all zero??
            //qDebug()<<"small group:"<<seeds.size();
            continue;
        }
        if (seeds.size()>param.max_voxel)
            continue;

        GOI one_group = GOI(seeds,label_id);
        voxel_groups.push_back(one_group);
        label_id++;
        //qDebug()<<"i:"<<iter_map->first<<" seeds size:"<<seeds.size()<<label_id;
    }

    //qDebug()<<"final groups size:"<<voxel_groups.size();
}

//LList comments status:
//1 not reviewed_dendrite
//2 not reviewed_spine
//3 accept_dendrite
//4 accept_spine
//5 reject_dendrite
//6 reject_spine
void is_analysis_fun::stat_generate()
{
    //qDebug()<<"in stat_Generate";
    float sum_x,sum_y,sum_z;
    vector<V3DLONG> coord(3,0);
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    for (int i=0;i<voxel_groups.size();i++)
    {
        sum_x=sum_y=sum_z=0;
        //qDebug()<<"i:"<<i<<" size:"<<voxel_groups[i].voxel.size();
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            coord=pos2xyz(voxel_groups[i].voxel[j],sz_img[0],z_offset);
            sum_x+=coord[0];
            sum_y+=coord[1];
            sum_z+=coord[2];
        }
        LocationSimple tmp_lm;
        tmp_lm.x=sum_x/(float)voxel_groups[i].voxel.size()+1;
        tmp_lm.y=sum_y/(float)voxel_groups[i].voxel.size()+1;
        tmp_lm.z=sum_z/(float)voxel_groups[i].voxel.size()+1;
        //
        tmp_lm.name = QString::number(i+1).toStdString(); //starts from 1
        int nearest_node=calc_nearest_node(nt,tmp_lm.x,tmp_lm.y,tmp_lm.z);
        voxel_groups[i].nearest_node=nearest_node;

        float min_dis=10e6;
        float dis;
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            coord=pos2xyz(voxel_groups[i].voxel[j],sz_img[0],z_offset);
            dis=(coord[0]-tmp_lm.x-1)*(coord[0]-tmp_lm.x-1)+(coord[1]-tmp_lm.y-1)*(coord[1]-tmp_lm.y-1)
                +(coord[2]-tmp_lm.z-1)*(coord[2]-tmp_lm.z-1);
            if (dis<min_dis)
                min_dis=dis;
        }
        if (min_dis<nt.listNeuron[nearest_node].radius) //if on dendrite blue
        {
            voxel_groups[i].on_dendrite=true;
            tmp_lm.color.r=85;
            tmp_lm.color.g=170;
            tmp_lm.color.b=255;
            tmp_lm.comments=QString::number(1).toStdString();
        }
        else //if on spine purple
        {
            tmp_lm.color.r=255;
            tmp_lm.color.g=0;
            tmp_lm.color.b=255;
            tmp_lm.comments=QString::number(2).toStdString();
        }
        LList_out.append(tmp_lm);
    }
}

void is_analysis_fun::visualize_image()
{
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    for (int i=0;i<voxel_groups.size();i++)
    {
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            mask[voxel_groups[i].voxel[j]+2*size_page]=255;
        }
    }
    main_win =callback->newImageWindow(MAINWINNAME);
    Image4DSimple image4d;
    unsigned char *visual_copy=new unsigned char[size_page*3];
    memcpy(visual_copy,mask,size_page*3);
    image4d.setData(visual_copy,mask_sz[0],mask_sz[1],mask_sz[2],3,V3D_UINT8);
    callback->setImage(main_win,&image4d);
    callback->setLandmark(main_win,LList_out);
    callback->open3DWindow(main_win);
    callback->pushObjectIn3DWindow(main_win);
//    QString filename1="list.marker";
//    write_marker_file(filename1,LList_new);
//    QString filename="test1.v3draw";
//    simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),mask,mask_sz,V3D_UINT8);
//    qDebug()<<"finished:"<<LList_new.size();
}


//bool is_analysis_fun::eswc_generate()
//{
//    //only get the swc name
//    qDebug()<<"in eswc generate";
//    QString baseName = swc_name.section('/', -1);
//    QString fileDefaultName=baseName+"_IS.eswc";
//    QString completName=QDir(basedir).filePath(fileDefaultName);
//    eswc_out_name=QFileDialog::getSaveFileName(this,tr("Save eswc"),completName,tr("Supported file: (*.eswc)"),0,0);
//    if (eswc_out_name.isEmpty())
//        return false;

//    for (int kk=0;kk<voxel_groups.size();kk++)
//    {
//        QString tmp=QString::fromStdString(LList_out[kk].comments);
//        if (tmp.contains("2"))  //delete
//            continue;
//        int node_idx=voxel_groups[kk].nearest_node;
//        if (nt_copy.listNeuron[node_idx].fea_val[3]==-1)
//        {
//            nt_copy.listNeuron[node_idx].fea_val[3]=voxel_groups[kk].voxel.size();
//            nt_copy.listNeuron[node_idx].fea_val[4]=voxel_groups[kk].on_dendrite;
//        }
//        else
//        {
//            NeuronSWC S;
//            S.n = nt_copy.listNeuron.length()+1;
//            qDebug()<<"~~~~~~~~~~~~~New node add:"<<S.n<<"ori node: "<<nt_copy.listNeuron[node_idx].n
//                   <<" ori node parent:"<<nt_copy.listNeuron[node_idx].parent;
//            S.type = nt_copy.listNeuron[node_idx].type;
//            S.x = nt_copy.listNeuron[node_idx].x;
//            S.y = nt_copy.listNeuron[node_idx].y;
//            S.z = nt_copy.listNeuron[node_idx].z;
//            S.r = nt_copy.listNeuron[node_idx].r;
//            S.parent = nt_copy.listNeuron[node_idx].parent;
//            S.fea_val.clear();
//            for (int jj=0;jj<3;jj++)
//                S.fea_val.push_back(-1);
//            S.fea_val.push_back(voxel_groups[kk].voxel.size());
//            S.fea_val.push_back(voxel_groups[kk].on_dendrite);
//            nt_copy.listNeuron[node_idx].parent=S.n;
//            nt_copy.listNeuron.append(S);
//        }
//    }

//    writeESWC_file(eswc_out_name,nt_copy);
////    out<<"# "<<image_name<<endl;
////    out<<"# "<<swc_name<<endl;
////    out<<"# for location: on_dendrite=1; on_spine=0"<<endl;
////    file.close();
//    qDebug()<<"voxel_group:"<<voxel_groups.size()<<"finish writing out"<<endl;
//    return true;
//}

