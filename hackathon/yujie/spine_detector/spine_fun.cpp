#include "spine_fun.h"
#include "FL_bwdist.h"


#define FLAG_ISTEST 1

spine_fun::spine_fun(const char * inname_img, const char* inname_skel, const char* inname_out, V3DPluginCallback * cb)
{
    //init names
    callback=cb;
    fname_img = inname_img;
    fname_skel = inname_skel;
    fname_out = inname_out;

    p_img1D = 0;
    ppp_img3D = 0;

    //init parameters
    param.bgthr = 90;//used
    param.intensity_step = 10; //used
    param.width_thr = 35;//used
    param.layer_thr = 5;
    param.max_dis = 40;//used
    param.min_pixel=30;//used
    param.dst_max_pixel = 2000;//used
    param.spongeness = 11;
    param.aspect_thr = 0.1;
}

spine_fun::spine_fun(V3DPluginCallback * cb,parameters set_para,int channel=1)
{
    p_img1D = 0;
    ppp_img3D = 0;

    callback=cb;

    //init parameters
    param.bgthr = set_para.bgthr;
    param.width_thr = set_para.width_thr;
    param.max_dis = set_para.max_dis;
    param.min_pixel=set_para.min_pixel;
    param.intensity_max_pixel = set_para.intensity_max_pixel;
    param.layer_thr = 5;
    param.spongeness = 11;
    param.aspect_thr = 0.1;
    param.intensity_step = 10;
    param.dst_max_pixel=2000;
    sel_channel=channel;

    qDebug()<<"para reset: param bg_thr:"<<param.bgthr<<" channel:"<<sel_channel;
}

int spine_fun::loadData()
{
    qDebug()<<"~~~~~Spine Detector: Loading datas";
    if(!simple_loadimage_wrapper(*callback, fname_img.toStdString().c_str(), p_img1D, sz_img, type_img)){
        qDebug()<<"load wrong";
        return 11;
    }
    if(type_img!=1){
        return 12;
    }
    if(sz_img[3]>1){
        qDebug()<<"WARNING: plugin can only take 1 channel, the first channel will be used.";
        sz_img[3]=1;
    }

    nt=readSWC_file(fname_skel);
    if(nt.listNeuron.size()<1){
        return 2;
    }

    //arrange the image into 3D
    ppp_img3D=new unsigned char ** [sz_img[2]];
    for(V3DLONG z=0; z<sz_img[2]; z++){
        ppp_img3D[z]=new unsigned char * [sz_img[1]];
        for(V3DLONG y=0; y<sz_img[1]; y++){
            ppp_img3D[z][y]=p_img1D+y*sz_img[0]+z*sz_img[0]*sz_img[1];
        }
    }

    return 0;
}

bool spine_fun::init()
{
    qDebug()<<"~~~~~Spine Detector: init voxels of interest along neuron skeleton";

    V3DLONG sz_page=sz_img[0]*sz_img[1]*sz_img[2];
    map<V3DLONG, VOI *> voxels_map;

    float * dst = 0;
    V3DLONG * label = 0;

    try{
        dst=new float[sz_page];
        label=new V3DLONG[sz_page];
    }
    catch(...){
        qDebug()<<"ERROR: failed to allocate memory for distance transform";
        if(dst!=0) delete[] dst;
        if(label!=0) delete[] label;
        return false;
    }

    for(V3DLONG i=0; i<sz_page; i++){
        dst[i]=INF;
    }

    for(V3DLONG nid=0; nid<nt.listNeuron.size(); nid++){
        V3DLONG nx = nt.listNeuron.at(nid).x;
        V3DLONG ny = nt.listNeuron.at(nid).y;
        V3DLONG nz = nt.listNeuron.at(nid).z;
        int disthr_2 = nt.listNeuron.at(nid).radius*nt.listNeuron.at(nid).radius;
        for(V3DLONG x=MAX(nx-nt.listNeuron.at(nid).radius,0); x<=MIN(nx+nt.listNeuron.at(nid).radius,sz_img[0]-1); x++){
            for(V3DLONG y=MAX(ny-nt.listNeuron.at(nid).radius,0); y<=MIN(ny+nt.listNeuron.at(nid).radius,sz_img[1]-1); y++){
                for(V3DLONG z=MAX(nz-nt.listNeuron.at(nid).radius,0); z<=MIN(nz+nt.listNeuron.at(nid).radius,sz_img[2]-1); z++){
                    if(ppp_img3D[z][y][x]<param.bgthr){ //too dark, skip
                        continue;
                    }
                    int dis=(x-nx)*(x-nx)+(y-ny)*(y-ny)+(z-nz)*(z-nz);
                    if(dis>disthr_2){ //too far, skip
                        continue;
                    }
                    dst[x+y*sz_img[0]+z*sz_img[0]*sz_img[1]]=0;
                }
            }
        }
    }
    sz_img[3]=1;
    qDebug()<<"~~~~~Spine Dectector: distance transform";
    dt3d(dst, label, sz_img);
    if (label!=0) {delete[] label; label=0;}

//    for(V3DLONG i=0; i<sz_page; i++){
//        dst[i]=sqrt(dst[i]);
//    }

//    QString fn="dst.v3draw";
//    simple_saveimage_wrapper(*callback,fn.toStdString().c_str(),(unsigned char *)dst,
//                             sz_img,V3D_FLOAT32);

    qDebug()<<"~~~~~Spine Dectector: init voxels";
    for(V3DLONG i=0; i<sz_page; i++){
        if(dst[i]>param.max_dis*param.max_dis){ //too far away
            continue;
        }
        if(p_img1D[i]<param.bgthr){ //too dark
            continue;
        }
        if(dst[i]<1e-10){ //skeleton node
            continue;
        }
        V3DLONG x,y,z;
        ind2sub(x,y,z,i);
        VOI * tmp_voxel = new VOI(i,x,y,z);
        tmp_voxel->intensity=p_img1D[i];
        tmp_voxel->dst=(int)sqrt(dst[i]);
        tmp_voxel->skel_idx=0;
        tmp_voxel->dst_label=tmp_voxel->dst_layer=-1;
        tmp_voxel->intensity_label=tmp_voxel->intensity_layer=-1;
        voxels_map[i]=tmp_voxel;
    }

    delete [] dst;
    dst=0;
    qDebug()<<"voxels map size:"<<voxels_map.size();
    qDebug()<<"~~~~~Spine Dectector: init neighbors";
    for(map<V3DLONG, VOI *>::iterator iter_map=voxels_map.begin(); iter_map!=voxels_map.end(); iter_map++){
        if(iter_map->second->dst>=0) //only consider selected skel points
        {
            VOI * tmp_voxel =iter_map->second;
            voxels.push_back(tmp_voxel);
            //construct neighbors
            for(V3DLONG x=MAX(tmp_voxel->x-1,0); x<=MIN(tmp_voxel->x+1,sz_img[0]-1); x++)
                for(V3DLONG y=MAX(tmp_voxel->y-1,0); y<=MIN(tmp_voxel->y+1,sz_img[0]-1); y++)
                    for(V3DLONG z=MAX(tmp_voxel->z-1,0); z<=MIN(tmp_voxel->z+1,sz_img[0]-1); z++){
                        if(x==tmp_voxel->x && y==tmp_voxel->y && z==tmp_voxel->z) //self
                            continue;
                        V3DLONG pos = sub2ind(x,y,z);
                        if(voxels_map.find(pos) == voxels_map.end()){ //background
                            continue;
                        }
                        if(voxels_map[pos]->dst<=0){ //skeleton voxels
                            continue;
                        }
                        tmp_voxel->neighbors_26.push_back(voxels_map[pos]);
                        if( (x==tmp_voxel->x && y==tmp_voxel->y) || (x==tmp_voxel->x && z==tmp_voxel->z) ||(z==tmp_voxel->z && y==tmp_voxel->y)){
                            tmp_voxel->neighbors_6.push_back(voxels_map[pos]);
                        }
                    }
        }
    }

    qDebug()<<"~~~~~Spine Detector: found "<<voxels.size()<<" voxels of interest.";
    return true;
}

bool spine_fun::init_old()
{
    qDebug()<<"~~~~~Spine Detector: init voxels of interest along neuron skeleton";
    //find the eligible voxels and their parameters
    voxels.clear();
    map<V3DLONG, VOI *> voxels_map;
    vector<V3DLONG> cur_layer, pre_layer;
    V3DLONG sz_page=sz_img[0]*sz_img[1]*sz_img[2];
    unsigned short * mask = new unsigned short[sz_page];
    memset(mask,0,sz_page*sizeof(unsigned short));
    for(V3DLONG nid=0; nid<nt.listNeuron.size(); nid++){
        V3DLONG nx = nt.listNeuron.at(nid).x;
        V3DLONG ny = nt.listNeuron.at(nid).y;
        V3DLONG nz = nt.listNeuron.at(nid).z;
        int disthr_2 = nt.listNeuron.at(nid).radius*nt.listNeuron.at(nid).radius;
        for(V3DLONG x=MAX(nx-nt.listNeuron.at(nid).radius,0); x<=MIN(nx+nt.listNeuron.at(nid).radius,sz_img[0]-1); x++){
            for(V3DLONG y=MAX(ny-nt.listNeuron.at(nid).radius,0); y<=MIN(ny+nt.listNeuron.at(nid).radius,sz_img[1]-1); y++){
                for(V3DLONG z=MAX(nz-nt.listNeuron.at(nid).radius,0); z<=MIN(nz+nt.listNeuron.at(nid).radius,sz_img[2]-1); z++){
                    if(ppp_img3D[z][y][x]<param.bgthr){ //too dark, skip
                        continue;
                    }
                    int dis=(x-nx)*(x-nx)+(y-ny)*(y-ny)+(z-nz)*(z-nz);
                    if(dis>disthr_2){ //too far, skip
                        continue;
                    }
                    dis=sqrt((double)dis)-nt.listNeuron.at(nid).radius;
                    V3DLONG pos=sub2ind(x,y,z);
//                    qDebug()<<x<<":"<<y<<":"<<z;
//                    qDebug()<<pos<<":"<<p_img1D[pos]<<voxels_map[pos];
                    if(voxels_map.find(pos) == voxels_map.end()){//voxel not included yet
                        VOI * tmp_voxel = new VOI(pos,x,y,z);
                        tmp_voxel->intensity=p_img1D[pos];
                        tmp_voxel->dst=dis;
                        tmp_voxel->skel_idx=nid;
                        tmp_voxel->dst_label=tmp_voxel->dst_layer=-1;
                        tmp_voxel->intensity_label=tmp_voxel->intensity_layer=-1;
                        voxels_map[pos]=tmp_voxel;
                        cur_layer.push_back(pos);
                        mask[pos]=nid+1;
                    }else{//voxel already found by others
                        VOI* tmp_voxel=voxels_map[pos];
                        if(tmp_voxel->dst > dis){//distance is shorter
                            tmp_voxel->dst = dis;
                            tmp_voxel->skel_idx = nid;
                            mask[pos]=nid+1;
                        }
                    }
                }
            }
//            cout<<"\r"<<nid<<":"<<voxels_map.size();
        }
    }
    qDebug()<<"\rFound "<<cur_layer.size()<<" initial seeds, start martching";
    for(int idst=0; idst<param.max_dis; idst++){
        pre_layer=cur_layer;
        cur_layer.clear();
        for(V3DLONG sid=0; sid<pre_layer.size(); sid++){
            V3DLONG nx, ny, nz;
            ind2sub(nx,ny,nz,pre_layer[sid]);
            //search its neighbor to find not visited node
            for(V3DLONG x=MAX(nx-1,0); x<=MIN(nx+1,sz_img[0]-1); x++){
                for(V3DLONG y=MAX(ny-1,0); y<=MIN(ny+1,sz_img[1]-1); y++){
                    for(V3DLONG z=MAX(nz-1,0); z<=MIN(nz+1,sz_img[2]-1); z++){
                        V3DLONG pos=sub2ind(x,y,z);
                        if(mask[pos]==0){//not visited yet
                            set<V3DLONG> nids;
                            //search its neighbors to find the candidate neuron node
                            for(V3DLONG cx=MAX(x-1,0); cx<=MIN(x+1,sz_img[0]-1); cx++){
                                for(V3DLONG cy=MAX(y-1,0); cy<=MIN(y+1,sz_img[1]-1); cy++){
                                    for(V3DLONG cz=MAX(z-1,0); cz<=MIN(z+1,sz_img[2]-1); cz++){
                                        V3DLONG cpos=sub2ind(cx,cy,cz);
                                        if(mask[cpos]!=0){
                                            nids.insert(mask[cpos]-1);
                                        }
                                    }
                                }
                            }
                            if(nids.size()==0){//should not happen?
                                qDebug()<<"spine_fun.cpp, init(): unexpected things happend, check the code";
                                continue;
                            }
                            //search for the closest one
                            int dis = param.max_dis+1;
                            V3DLONG nid=*(nids.begin());
                            for(set<V3DLONG>::iterator iter=nids.begin(); iter!=nids.end(); iter++){
                                int tmp_dis=(nt.listNeuron.at(*iter).x - x)*(nt.listNeuron.at(*iter).x - x);
                                tmp_dis+=(nt.listNeuron.at(*iter).y - y)*(nt.listNeuron.at(*iter).y - y);
                                tmp_dis+=(nt.listNeuron.at(*iter).z - z)*(nt.listNeuron.at(*iter).z - z);
                                tmp_dis=sqrt(double(tmp_dis))-nt.listNeuron.at(*iter).radius;
                                if(dis>tmp_dis){
                                    dis=tmp_dis;
                                    nid=*iter;
                                }
                            }
                            mask[pos]=nid+1;
                            cur_layer.push_back(pos);
                            //init a new node if is foreground
                            if(p_img1D[pos]>param.bgthr){
                                VOI * tmp_voxel = new VOI(pos,x,y,z);
                                tmp_voxel->intensity=p_img1D[pos];
                                tmp_voxel->dst=dis;
                                tmp_voxel->skel_idx=nid;
                                tmp_voxel->dst_label=tmp_voxel->dst_layer=-1;
                                tmp_voxel->intensity_label=tmp_voxel->intensity_layer=-1;
                                voxels_map[pos]=tmp_voxel;
                            }
                        }
                    }
                }
            }
        }
        if (cur_layer.size()==0) return false;
        qDebug()<<"round: "<<idst<<"; new seed:"<<cur_layer.size()<<"; voxels:"<<voxels_map.size();
        pre_layer.clear();
    }

    qDebug()<<"~~~~~Spine Dectector: init neighbors";
    for(map<V3DLONG, VOI *>::iterator iter_map=voxels_map.begin(); iter_map!=voxels_map.end(); iter_map++){
        if(iter_map->second->dst>=0) //only consider selected skel points
        {
            VOI * tmp_voxel =iter_map->second;
            voxels.push_back(tmp_voxel);
            //construct neighbors
            for(V3DLONG x=MAX(tmp_voxel->x-1,0); x<=MIN(tmp_voxel->x+1,sz_img[0]-1); x++)
                for(V3DLONG y=MAX(tmp_voxel->y-1,0); y<=MIN(tmp_voxel->y+1,sz_img[0]-1); y++)
                    for(V3DLONG z=MAX(tmp_voxel->z-1,0); z<=MIN(tmp_voxel->z+1,sz_img[0]-1); z++){
                        if(x==tmp_voxel->x && y==tmp_voxel->y && z==tmp_voxel->z) //self
                            continue;
                        V3DLONG pos = sub2ind(x,y,z);
                        if(voxels_map.find(pos) == voxels_map.end()){ //background
                            continue;
                        }
                        if(voxels_map[pos]->dst<=0){ //skeleton voxels
                            continue;
                        }
                        tmp_voxel->neighbors_26.push_back(voxels_map[pos]);
                        if( (x==tmp_voxel->x && y==tmp_voxel->y) || (x==tmp_voxel->x && z==tmp_voxel->z) ||(z==tmp_voxel->z && y==tmp_voxel->y)){
                            tmp_voxel->neighbors_6.push_back(voxels_map[pos]);
                        }
                    }
        }
    }

    qDebug()<<"\n~~~~~Spine Detector: found "<<voxels.size()<<" voxels of interest.";
    delete [] mask; mask=0;
    return true;
}


//group voxels by dst
//marching from voxels with larger dst
//component with larger size will have higher priority to grow
bool spine_fun::run_dstGroup()
{
    sort(voxels.begin(), voxels.end(), sortfunc_dst); //descending sort by dst

    qDebug()<<"~~~~~Spine Detector: dst sorting done, start to grow regions based on dst";

    dst_groups.clear();
    vector<VOI*> cur_layer, pre_layer, seeds;
    int dst_floor = voxels.front()->dst;
    int vid_begin,vid_end;
    vid_begin=vid_end=0;

    while(dst_floor>0){
        while(vid_end<voxels.size() && voxels[vid_end]->dst>=dst_floor){
            vid_end++;
            continue;
        }
        pre_layer=cur_layer;
        cur_layer.clear();
        reverse(pre_layer.begin(),pre_layer.end()); //switch the order to balance the grow
        if(pre_layer.size()>0){ //grow the connected part to the previous round first
            seeds=pre_layer;
            for(V3DLONG sid=0; sid<seeds.size(); sid++){
                VOI * tmp_voxel = seeds[sid];
                for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                    if(tmp_voxel->neighbors_26[neid]->dst>=dst_floor &&tmp_voxel->neighbors_26.size()>=8 )
                    {
                        if(tmp_voxel->neighbors_26[neid]->dst_label<0) //if not visited and within layer
                        {
                            VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                            //tmp_seed->dst_layer=tmp_voxel->dst_layer;
                            tmp_seed->dst_label=tmp_voxel->dst_label;
                            dst_groups[tmp_seed->dst_label].push_back(tmp_seed);
                            cur_layer.push_back(tmp_seed);
                            seeds.push_back(tmp_seed);
                        }
                        else if (tmp_voxel->neighbors_26[neid]->dst_label!=tmp_voxel->dst_label)
                        //need to check if merge is possible
                        {
                            VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                            //qDebug()<<"might be a merge:"<<dst_groups[tmp_seed->dst_label].size();
                            if (dst_groups[tmp_seed->dst_label].size()<20 || dst_groups[tmp_voxel->dst_label].size()<20) //merge to this one
                            {
                                int merged_label = MAX(tmp_seed->dst_label,tmp_voxel->dst_label);
                                int kept_label = MIN (tmp_seed->dst_label,tmp_voxel->dst_label);
                                for (int i=0;i<dst_groups[merged_label].size();i++)
                                {
                                    dst_groups[merged_label][i]->dst_label=kept_label;
                                    dst_groups[kept_label].push_back(dst_groups[merged_label][i]);
                                }
                                dst_groups[merged_label].clear();

                            }


                        }

                    }
                }
            }
        }

        for(V3DLONG vid=vid_begin; vid<vid_end; vid++){ //check the rest nodes in this layer, grow the rest if there is any left
            if(voxels[vid]->dst_label<0){
                GOV tmp_group; //a new group of voxels
                tmp_group.push_back(voxels[vid]);
                voxels[vid]->dst_label=dst_groups.size();
                //voxels[vid]->dst_layer=0;
                cur_layer.push_back(voxels[vid]);
                //region grow
                seeds.clear();
                seeds.push_back(voxels[vid]);
                for(V3DLONG sid=0; sid<seeds.size(); sid++){
                    VOI * tmp_voxel = seeds[sid];
                    for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                        if(tmp_voxel->neighbors_26[neid]->dst_label<0 &&
                                tmp_voxel->neighbors_26[neid]->dst>=dst_floor &&
                                tmp_voxel->neighbors_26.size()>=8){ //if not visited and within layer
                            VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                            //tmp_seed->dst_layer=tmp_voxel->dst_layer;
                            tmp_seed->dst_label=tmp_voxel->dst_label;
                            tmp_group.push_back(tmp_seed);
                            cur_layer.push_back(tmp_seed);
                            seeds.push_back(tmp_seed);
                        }
                    }
                }
                dst_groups.push_back(tmp_group);
            }
        }

        for(V3DLONG vid=0; vid<cur_layer.size(); vid++){
            cur_layer[vid]->dst_layer++;
        }

        if(FLAG_ISTEST)
            qDebug()<<"dst:"<<dst_floor<<"; group"<<dst_groups.size()<<"; voxels"<<cur_layer.size()<<"; span:"<<vid_begin<<"-"<<vid_end<<"="<<vid_end-vid_begin;

        vid_begin=vid_end;
        if(vid_begin>=voxels.size())
            dst_floor=-1;
        else
            dst_floor = voxels[vid_begin]->dst;
    }

    return true;
}

bool spine_fun::reverse_dst_grow()
{
    sort(voxels.begin(), voxels.end(), sortfunc_dst_ascend);
    vector<VOI*> cur_layer,seeds,seeds_next;
    int dst_floor = voxels.front()->dst;
    int vid_begin,vid_end;
    vid_begin=vid_end=0;
    float sum_x,sum_y,sum_z,center_x,center_y,center_z;
    //qDebug()<<"dst_floor:"<<dst_floor;
    while(vid_end<voxels.size() && voxels[vid_end]->dst<=dst_floor){
        vid_end++;
        continue;
    }
    //qDebug()<<"vid end:"<<vid_end;
    seeds.clear();
    for(V3DLONG vid=vid_begin; vid<vid_end; vid++)
    {
        if (voxels[vid]->dst_label<0)
        {
            seeds.push_back(voxels[vid]);
            voxels[vid]->dst_label=0;
        }
    }
    //dst_label=-1 not touched
    //dst_label=1 candidate for curr layer
    //dst_label=0 discard
    //dst_label=2 keep

    while (seeds.size()>0)
    {
        //qDebug()<<"dst_floor:"<<dst_floor<<" seeds size:"<<seeds.size();
        dst_floor++;
        cur_layer.clear();
        for(V3DLONG sid=0; sid<seeds.size(); sid++){
            VOI * tmp_voxel = seeds[sid];
            for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){

                if(tmp_voxel->neighbors_26[neid]->dst==dst_floor &&
                        tmp_voxel->neighbors_26[neid]->dst_label<0)
                {
                    cur_layer.push_back(tmp_voxel->neighbors_26[neid]);
                    seeds.push_back(tmp_voxel->neighbors_26[neid]);
                    tmp_voxel->neighbors_26[neid]->dst_label=1;
                }
            }
        }
        //qDebug()<<"cur_layer set"<<cur_layer.size();

        for (int i=0;i<cur_layer.size();i++) //find connected components
        {
            if (cur_layer[i]->dst_label!=1) continue;
            VOI * tmp_voxel = cur_layer[i];
            seeds.clear();
            seeds.push_back(tmp_voxel);
            tmp_voxel->dst_label=2;
            GOV tmp_group;
            tmp_group.push_back(tmp_voxel);
            for(int sid=0; sid<seeds.size();sid++)
            {
                VOI * tmp_voxel = seeds[sid];
                for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                    if (tmp_voxel->neighbors_26[neid]->dst_label==1)
                    {
                        VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                        tmp_group.push_back(tmp_seed);
                        tmp_seed->dst_label=2;
                        seeds.push_back(tmp_seed);
                    }
                }
            }
            if (tmp_group.size()>param.dst_max_pixel)
            {
                //qDebug()<<"this group exeeds the maximum pixels"<<tmp_group.size();
                for (int j=0;j<tmp_group.size();j++)
                {
                    tmp_group[j]->dst_label=0;
                    seeds_next.push_back(tmp_group[j]);
                }
                //qDebug()<<"seeds_next size:"<<seeds_next.size();
                continue;
            }

            //calc width
            sum_x=sum_y=sum_z=0;
            for (int j=0;j<tmp_group.size();j++)
            {
                sum_x+=tmp_group[j]->x;
                sum_y+=tmp_group[j]->y;
                sum_z+=tmp_group[j]->z;
            }
            center_x=sum_x/tmp_group.size();
            center_y=sum_y/tmp_group.size();
            center_z=sum_z/tmp_group.size();
            float max_dis=0;
            for (int k=0;k<tmp_group.size();k++)
            {
                float dis=(tmp_group[k]->x-center_x)*(tmp_group[k]->x-center_x)+
                        (tmp_group[k]->y-center_y)*(tmp_group[k]->y-center_y)+
                        (tmp_group[k]->z-center_z)*(tmp_group[k]->z-center_z);
                if (dis>max_dis)
                    max_dis=dis;
            }
            max_dis=2*sqrt(max_dis);
            if (max_dis>param.width_thr)
            {
                //qDebug()<<"this group exeeds maximum width"<<max_dis;

                for (int j=0;j<tmp_group.size();j++)
                {
                    seeds_next.push_back(tmp_group[j]);
                    tmp_group[j]->dst_label=0;
                }
                //qDebug()<<"seeds_next size:"<<seeds_next.size();
                continue;
            }
        }
        seeds=seeds_next;
        //qDebug()<<"seeds next size:"<<seeds_next.size();
        seeds_next.clear();
    }
    //qDebug()<<"~~~~~dst grow complete, check connected component";
    //find connected components among points whose dst_label!=0
    //1)get rid of groups that are too small
    //2)get ready for intesity grow
    dst_groups.clear();
    GOV tmp_group;
    int spine_id=3;
    for (V3DLONG i=vid_end;i<voxels.size();i++)
    {
        if (voxels[i]->dst_label==0 || voxels[i]->dst_label>=3) continue;
        //qDebug()<<"i:"<<i;
        if (voxels[i]->dst_label==1) qDebug()<<"~~~~!!!!!! this equals 1"<<voxels[i]->pos;
        VOI * tmp_voxel = voxels[i];
        voxels[i]->dst_label=spine_id;
        seeds.clear();
        tmp_group.clear();
        seeds.push_back(tmp_voxel);
        tmp_group.push_back(tmp_voxel);
        for(int sid=0; sid<seeds.size();sid++)
        {
            VOI * tmp_voxel = seeds[sid];
            for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                if (tmp_voxel->neighbors_26[neid]->dst_label==-1||
                       tmp_voxel->neighbors_26[neid]->dst_label==2 )
                {
                    VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                    tmp_group.push_back(tmp_seed);
                    seeds.push_back(tmp_seed);
                    tmp_seed->dst_label=spine_id;
                }
            }
            //qDebug()<<"seeds size:"<<seeds.size();
        }
        //qDebug()<<"tmp_group size:"<<tmp_group.size();
        if (tmp_group.size()<param.min_pixel) //discard this group
        {
            for (int j=0;j<tmp_group.size();j++)
            {
                tmp_group[j]->dst_label=0;
            }
            //qDebug()<<"not enough pixels:"<<tmp_group.size();
            continue;
        }
        sort(tmp_group.begin(),tmp_group.end(),sortfunc_dst); //descending
        if (tmp_group.front()->dst<=2)
        {
            for (int j=0;j<tmp_group.size();j++)
            {
                tmp_group[j]->dst_label=0;
            }
            //qDebug()<<"too flat:"<<tmp_group.front()->dst;
            continue;
        }
        dst_groups.push_back(tmp_group);
        spine_id++;
    }
    if (dst_groups.size()<=0)
    {
        return false;
    }
    return true;
    qDebug()<<"dst_groups grow finished~~~~~~  dst_group size:"<<dst_groups.size()<<"spine id:"<<spine_id;
}

void spine_fun::dst_group_check()
{
    //construct profile
    vector<group_profile*> all_group_info;
    qDebug()<<"dst_group size:"<<dst_groups.size();
    QString fname="spine_fun.marker";
    FILE *fp1=fopen(fname.toAscii(),"wt");
    fprintf(fp1,"##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");

    for (int i=0;i<dst_groups.size();i++)
    {
        qDebug()<<"I:"<<i;
        if(dst_groups[i].size()<=0) continue;
        GOV tmp_group=dst_groups[i];
//        if (tmp_group.size()<param.min_pixel){
//            for (int j=0;j<tmp_group.size();j++)
//                tmp_group[j]->dst_label=-1;
//            dst_groups[i].clear();
//            continue;
//        }
        sort (tmp_group.begin(),tmp_group.end(),sortfunc_dst);
//        if (tmp_group.front()->dst<=2)
//        {
//            qDebug()<<"----------------------this group too flat";
//            continue;
//        }

        fprintf(fp1,"%d,%d,%d,1,1,"","",255,255,255\n",tmp_group.front()->x+1,
                tmp_group.front()->y+1,tmp_group.front()->z+1);
        int number_of_layer=tmp_group.front()->dst-tmp_group.back()->dst+1;
        qDebug()<<"------group: "<<i<<" number of layer:"<<number_of_layer;
        float sum_x,sum_y,sum_z,sum_nb,center_x,center_y,center_z,ave_nb;
        int tmp_dis_floor=tmp_group.front()->dst;
        GOV cur_layer;

        group_profile *cur_group_info=new group_profile;
        cur_group_info->number_of_layer=number_of_layer;
        cur_group_info->group_id=i;
        //cur_group_info->distance=tmp_group.front()->dst;
        while (tmp_dis_floor>=tmp_group.back()->dst)
        {
            qDebug()<<"tmp_dis_floor:"<<tmp_dis_floor;
            sum_x=sum_y=sum_z=sum_nb=0;
            cur_layer.clear();
            for (int j=0;j<tmp_group.size();j++)
            {
                if (tmp_group[j]->dst==tmp_dis_floor)
                {
                    sum_x+=tmp_group[j]->x;
                    sum_y+=tmp_group[j]->y;
                    sum_z+=tmp_group[j]->z;
                    sum_nb+=tmp_group[j]->neighbors_26.size();
                    cur_layer.push_back(tmp_group[j]);
                }
            }
            center_x=sum_x/cur_layer.size();
            center_y=sum_y/cur_layer.size();
            center_z=sum_z/cur_layer.size();
            ave_nb=sum_nb/cur_layer.size();
            float max_dis=0;
            for (int k=0;k<cur_layer.size();k++)
            {
                float dis=(cur_layer[k]->x-center_x)*(cur_layer[k]->x-center_x)+
                        (cur_layer[k]->y-center_y)*(cur_layer[k]->y-center_y)+
                        (cur_layer[k]->z-center_z)*(cur_layer[k]->z-center_z);
                if (dis>max_dis)
                    max_dis=dis;
            }
            max_dis=2*sqrt(max_dis);
            cur_group_info->layer_length.push_back(max_dis);
            cur_group_info->layer_spongeness.push_back(ave_nb);
            cur_group_info->layer_voxel_num.push_back(cur_layer.size());
            tmp_dis_floor--;
        }
        qDebug()<<"cur_group_info layer_length size:"<<cur_group_info->layer_length.size()
               <<"cur group info layer number:"<<cur_group_info->number_of_layer;
        all_group_info.push_back(cur_group_info);

    }

    qDebug()<<"all group info size:"<<all_group_info.size();
    //start trimming layers

    QString outfile="spine_analysis2.csv";
    FILE *fp2=fopen(outfile.toAscii(),"wt");
    fprintf(fp2,"##id,layer,length,layer_pixels,spongeness,distance\n");
    for (int i=0;i<all_group_info.size();i++)
    {
        group_profile *cur_group_info=all_group_info[i];
        for (int j=0;j<cur_group_info->number_of_layer;j++)
        {
//            if(cur_group_info->layer_spongeness<3 ||
//                    cur_group_info->layer_length>param.width_thr)
            qDebug()<<"layer:"<<j<<" length:"<<cur_group_info->layer_length[j]
                      <<" spongeness:"<<cur_group_info->layer_spongeness[j]
                        <<" pixels:"<<cur_group_info->layer_voxel_num[j];
            fprintf(fp2,"%d,%d,%.1f,%d,%.1f,%d\n",i,j,cur_group_info->layer_length[j],
                    cur_group_info->layer_voxel_num[j],cur_group_info->layer_spongeness[j],
                    cur_group_info->distance);
        }

    }
    fclose(fp1);
    fclose(fp2);
}

//LandmarkList spine_fun::construct_group_profile()
//{
//    qDebug()<<"~~~Conducting final group check";
//    //construct profile

//    int spine_id=1;
//    QString fname="spine_fun.marker";
//    FILE *fp1=fopen(fname.toAscii(),"wt");
//    fprintf(fp1,"##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");
//    LandmarkList LList;

//    for (int i=0;i<intensity_groups.size();i++)
//    {
//        if(intensity_groups[i].size()<=0) continue;
//        //qDebug()<<"I:"<<i<<spine_id;
//        int vid_begin,vid_end;
//        vid_begin=vid_end=0;

//        GOV tmp_group=intensity_groups[i];

//        //check the minumum distance
//        sort (tmp_group.begin(),tmp_group.end(),sortfunc_dst); //descending
//        if (tmp_group.front()->dst<=2)
//        {
//            for (int j=0;j<tmp_group.size();j++)
//                tmp_group[j]->intensity_label=-1;
//            intensity_groups[i].clear();
//            continue;
//        }

//        if (check_spongeness(tmp_group)<0) continue;

//        int tmp_dis_floor=tmp_group.front()->dst;
//        bool wide_layer=false;
//        bool complete_delete=false;
//        float max_dis;

//        while (tmp_dis_floor>=tmp_group.back()->dst)
//        {
//            vid_begin=vid_end;
//            while(vid_end<tmp_group.size() && tmp_group[vid_end]->dst==tmp_dis_floor){
//                vid_end++;
//                continue;
//            }
//            if (vid_begin==vid_end) //for one layer no voxels found
//            {
//                tmp_dis_floor--;
//                continue;
//            }

//            GOV cur_layer;
//            for (int j=vid_begin;j<vid_end;j++)
//            {
//                cur_layer.push_back(tmp_group[j]);
//            }

//            max_dis=calc_spread_width(cur_layer);

//            if (max_dis>param.width_thr) //delete the whole group
//            {
//                int count=vid_begin-1;
//                if (count<=param.min_pixel)
//                {
//                    complete_delete=true;
//                    for (int j=0;j<tmp_group.size();j++)
//                        tmp_group[j]->intensity_label=-1;
//                    intensity_groups[i].clear();
//                    qDebug()<<"wide layer_delete whole group:"<<"number left:"<<count <<"group size:"
//                                          <<tmp_group.size()<<"which group:"<<spine_id;
//                    break;
//                }
//                else //only delete the pixels in the latter layer
//                {
//                    for (int j=vid_begin;j<tmp_group.size();j++)
//                        tmp_group[j]->intensity_label=-1;
//                    wide_layer=true;

//                    //check connected components
//                    int id=1;
//                    GOV seeds;
//                    map<V3DLONG, int> lookup;
//                    for (int j=0;j<vid_begin;j++)
//                    {
//                        VOI * tmp_voi=tmp_group[j];
//                        if (lookup[tmp_voi->pos]>0) continue;
//                        seeds.clear();
//                        lookup[tmp_voi->pos]=id;
//                        seeds.push_back(tmp_voi);
//                        int sid=0;
//                        while (seeds.size()>0)
//                        {
//                            VOI * single_seed = seeds[sid];
//                            for (int neid=0; neid<tmp_voi->neighbors_26.size();neid++)
//                            {
//                                if (lookup[single_seed->neighbors_26[neid]->pos]==0)
//                                {
//                                    seeds.push_back(tmp_voi->neighbors_26[neid]);
//                                    lookup[single_seed->neighbors_26[neid]->pos]=id;
//                                }
//                            }
//                            sid++;
//                        }
//                        id++;
//                        if (seeds.size()<param.min_pixel|| check_spongeness(seeds)<0)
//                        {
//                            for (int k=0;k<seeds.size();k++)
//                                seeds[k]->intensity_label=-1;
//                        }
//                        //check aspect ratio
//                        sort(seeds.begin(),seeds.end(),sortfunc_dst);
//                        if (seeds.front()->dst-seeds.back()->dst<=1) break;
//                        GOV tmp;
//                        for (int k=0;k<seeds.size();k++)
//                        {
//                            if (seeds[k]->dst==seeds.back()->dst)
//                                tmp.push_back(seeds[k]);
//                        }
//                        float width;
//                        width=calc_spread_width(tmp);
//                        float aspect_ratio = (seeds.front()->dst-seeds.back()->dst)/width;
//                        if (aspect_ratio< param.aspect_thr)
//                        {
//                            for (int k=0;k<seeds.size();k++)
//                                seeds[k]->intensity_label=-1;
//                        }
//                        //possible to have several separate groups?
//                    }
//                    break;
//                 }
//            }
//            tmp_dis_floor--;
//        }
//        if (complete_delete) continue;

//        if (wide_layer)
//        {
//            GOV new_group;

//            for (int j=0;j<tmp_group.size();j++)
//            {
//                if (tmp_group[j]->intensity_label!=-1)
//                {
//                    new_group.push_back(tmp_group[j]);
//                }
//            }
//            sort (new_group.begin(),new_group.end(),sortfunc_dst);
//            //check

//        }
//        else
//        //aspect ratio check
//        float aspect_ratio=(cur_group_info->distance.front()-cur_group_info->distance.back())/
//                cur_group_info->layer_length.back();
//        if (cur_group_info->layer_length.size()>1 && aspect_ratio<param.aspect_thr)
//        {
//            qDebug()<<"spine id:"<<spine_id<<" aspect ratio:"<<aspect_ratio;
//            for (int j=0;j<tmp_group.size();j++)
//                tmp_group[j]->intensity_label=-1;
//            intensity_groups[i].clear();
////            qDebug()<<"wide layer_delete whole group:"<<"number left:"<<count <<"group size:"
////                                  <<tmp_group.size()<<"which group:"<<spine_id;
//            delete cur_group_info;
//            cur_group_info=0;
//            continue;
////            QString comment = "aspect";
////            QString name = QString::number(spine_id);
////            fprintf(fp1,"%.1f,%.1f,%.1f,1,1,%s,%s,255,255,255\n",sum_sum_x/sum_pixel,sum_sum_y/sum_pixel
////            ,sum_sum_z/sum_pixel,name.toStdString().c_str(),comment.toStdString().c_str());
//        }


//        for (int j=0;j<tmp_group.size();j++)
//        {
//            if (tmp_group[j]->intensity_label!=-1)
//            {
//                tmp_group[j]->intensity_label=spine_id;
//                new_group.push_back(tmp_group[j]);
//            }
//        }
//        final_groups.push_back(new_group);  //the group_number is 1 smaller than the label
//        //qDebug()<<"final groups size:"<<final_groups.size()<<" spine id:"<<spine_id;
//        LocationSimple tmp_marker;
//        tmp_marker.x=sum_sum_x/sum_pixel;
//        tmp_marker.y=sum_sum_y/sum_pixel;
//        tmp_marker.z=sum_sum_z/sum_pixel;
//        tmp_marker.name = QString::number(spine_id).toStdString().c_str();
//        LList.append(tmp_marker);
//        fprintf(fp1,"%.1f,%.1f,%.1f,1,1,,,255,255,255\n",sum_sum_x/sum_pixel,sum_sum_y/sum_pixel
//            ,sum_sum_z/sum_pixel);
//        all_group_info.push_back(cur_group_info);
//        spine_id++;

//    }
//    spine_id--;
//    qDebug()<<"all group info size:"<<all_group_info.size()<<" spine_id:"<<spine_id<<" final_group size:"
//           << final_groups.size();

//    QString outfile="spine_analysis2.csv";
//    FILE *fp2=fopen(outfile.toAscii(),"wt");
//    fprintf(fp2,"##id,layer,length,layer_pixels,spongeness,distance,total_sponge\n");
//    qDebug()<<"fp2 opened";

//    for (int i=0;i<all_group_info.size();i++)
//    {
//        group_profile *cur_group_info=all_group_info[i];
//        //qDebug()<<"i:"<<i<<"  this group has layer: "<<cur_group_info->layer_length.size()
////               <<":"<<cur_group_info->number_of_layer;
//        for (int j=0;j<cur_group_info->layer_length.size();j++)
//        {

//            fprintf(fp2,"%d,%d,%.1f,%d,%.1f,%d,%.1f\n",cur_group_info->group_id,j,cur_group_info->layer_length[j],
//                    cur_group_info->layer_voxel_num[j],cur_group_info->layer_spongeness[j],
//                    cur_group_info->distance[j],cur_group_info->total_spongeness);
//        }

//    }
//    fclose(fp1);
//    fclose(fp2);
//    return LList;
//}

float spine_fun::check_spongeness(GOV group)
{
    float sum_nb=0;
    for (int i=0;i<group.size();i++)
    {
        sum_nb = sum_nb+group[i]->neighbors_26.size();
    }
    float spongeness=sum_nb/group.size();
    if (spongeness<=param.spongeness)
    {
//        for (int j=0;j<group.size();j++)
//            group[j]->intensity_label=-1;
        //intensity_groups[i].clear();
        spongeness=-1;
        qDebug()<<"group spongy:"<< group.size();
    }
    return spongeness;
}

float spine_fun::calc_spread_width(GOV cur_layer)
{
    float sum_x,sum_y,sum_z,center_x,center_y,center_z;

    sum_x=sum_y=sum_z=0;
    for (int j=0;j<cur_layer.size();j++)
    {
        sum_x+=cur_layer[j]->x;
        sum_y+=cur_layer[j]->y;
        sum_z+=cur_layer[j]->z;
    }

    center_x=sum_x/cur_layer.size();
    center_y=sum_y/cur_layer.size();
    center_z=sum_z/cur_layer.size();

    float max_dis=0;
    for (int k=0;k<cur_layer.size();k++)
    {
        float dis=(cur_layer[k]->x-center_x)*(cur_layer[k]->x-center_x)+
                (cur_layer[k]->y-center_y)*(cur_layer[k]->y-center_y)+
                (cur_layer[k]->z-center_z)*(cur_layer[k]->z-center_z);
        if (dis>max_dis)
            max_dis=dis;
    }
    max_dis=2*sqrt(max_dis);
    return max_dis;
}

int spine_fun::construct_layer_info(GOV tmp_group, group_profile *cur_group_info)
{
//    float sum_x,sum_y,sum_z,center_x,center_y,center_z,ave_nb;
//    int tmp_dis_floor=tmp_group.front()->dst;
//    GOV cur_layer;

//    bool wide_layer=false;
//    while (tmp_dis_floor>=tmp_group.back()->dst)
//    {
//        vid_begin=vid_end;
//        while(vid_end<tmp_group.size() && tmp_group[vid_end]->dst==tmp_dis_floor){
//            vid_end++;
//            continue;
//        }
//        if (vid_begin==vid_end) //for one layer no voxels found
//        {
//            tmp_dis_floor--;
//            continue;
//        }
//        sum_x=sum_y=sum_z=sum_nb=0;
//        cur_layer.clear();
//        for (int j=vid_begin;j<vid_end;j++)
//        {
//            sum_x+=tmp_group[j]->x;
//            sum_y+=tmp_group[j]->y;
//            sum_z+=tmp_group[j]->z;
//            sum_nb+=tmp_group[j]->neighbors_26.size();
//            cur_layer.push_back(tmp_group[j]);
//        }

//        center_x=sum_x/cur_layer.size();
//        center_y=sum_y/cur_layer.size();
//        center_z=sum_z/cur_layer.size();
//        ave_nb=sum_nb/cur_layer.size();

//        cur_group_info->distance.push_back(tmp_dis_floor);
//        float max_dis=0;
//        for (int k=0;k<cur_layer.size();k++)
//        {
//            float dis=(cur_layer[k]->x-center_x)*(cur_layer[k]->x-center_x)+
//                    (cur_layer[k]->y-center_y)*(cur_layer[k]->y-center_y)+
//                    (cur_layer[k]->z-center_z)*(cur_layer[k]->z-center_z);
//            if (dis>max_dis)
//                max_dis=dis;
//        }
//        max_dis=2*sqrt(max_dis);

//        if (max_dis>param.width_thr) //delete the whole group
//        {
//            int count=vid_begin-1;
//            if (count<=param.min_pixel)
//            {
//                wide_layer=true;
//                for (int j=0;j<tmp_group.size();j++)
//                    tmp_group[j]->intensity_label=-1;
//                intensity_groups[i].clear();
//                qDebug()<<"wide layer_delete whole group:"<<"number left:"<<count <<"group size:"
//                                      <<tmp_group.size()<<"which group:"<<spine_id;
//                return 0;

//            }
//            else //only delete the pixels in the latter layer
//            {
//                for (int j=vid_begin;j<tmp_group.size();j++)
//                    tmp_group[j]->intensity_label=-1;
//                map<V3DLONG, int> lookup;

//                //check connected components
//                int id=1;
//                GOV seeds;
//                for (int j=0;j<vid_begin;j++)
//                {
//                    VOI * tmp_voi=tmp_group[j];
//                    if (lookup[tmp_voi->pos]>0) continue;
//                    seeds.clear();
//                    lookup[tmp_voi->pos]=id;
//                    seeds.push_back(tmp_voi);
//                    int sid=0;
//                    while (seeds.size()>0)
//                    {
//                        VOI * single_seed = seeds[sid];
//                        for (int neid=0; neid<tmp_voi->neighbors_26.size();neid++)
//                        {
//                            if (lookup[single_seed->neighbors_26[neid]->pos]==0)
//                            {
//                                seeds.push_back(tmp_voi->neighbors_26[neid]);
//                                lookup[single_seed->neighbors_26[neid]->pos]=id;
//                            }
//                        }
//                        sid++;
//                    }
//                    id++;
//                    if (seeds.size()<param.min_pixel)
//                    {
//                        for (int k=0;k<seeds.size();k++)
//                            seeds[k]->intensity_label=-1;
//                    }
//                }

//                //update the group info
//                for (int gid=0;gid<vid_begin;gid++)
//                {
//                    if (tmp_group[gid]->intensity_label!=-1)
//                }

//                cur_group_info->number_of_layer=cur_group_info->layer_length.size();
//                        //tmp_group.front()->dst-tmp_group[vid_end-1]->dst+1;
////                    qDebug()<<"wide layer:"<<"number left:"<<count <<"group size:"
////                                          <<tmp_group.size()<<"which group:"<<spine_id
////                                         <<"layers left"<<":"<<cur_group_info->layer_length.size();
//                cur_group_info->total_spongeness=sum_sum_nb/sum_pixel;
//                if (cur_group_info->total_spongeness<=param.spongeness)
//                {
//                    qDebug()<<"spongeness not satisfied and wide layer";
//                    for (int j=0;j<vid_begin;j++)
//                        tmp_group[j]->intensity_label=-1;
//                    intensity_groups[i].clear();
//                    delete cur_group_info;
//                    cur_group_info=0;
//                    wide_layer=true;
//                    break;
//                }

//            }
//        }

//        cur_group_info->layer_length.push_back(max_dis);
//        cur_group_info->layer_spongeness.push_back(ave_nb);
//        cur_group_info->layer_voxel_num.push_back(cur_layer.size());
//        tmp_dis_floor--;
//    }
    return 0;
}

bool spine_fun::run_intensityGroup()
{
    qDebug()<<"~~~~~start to grow regions based on intensity"<<"dst group size:"<<dst_groups.size();

    intensity_groups.clear();
    for (int gid=0;gid<dst_groups.size();gid++)
    {
        //qDebug()<<"gid:"<<gid<<" group size:"<<dst_groups[gid].size();
        if (dst_groups[gid].size()<=300)
        {
            GOV tmp_group=dst_groups[gid];

            for (int i=0;i<tmp_group.size();i++)
            {
                tmp_group[i]->intensity_label=intensity_groups.size();
            }
            intensity_groups.push_back(tmp_group);
            continue;
        }
        GOV tmp_group;
        tmp_group=dst_groups[gid];

        sort(tmp_group.begin(),tmp_group.end(),sortfunc_intensity);//decending
        vector<VOI*> cur_layer, pre_layer, seeds;
        int intensity_floor = MIN(200,tmp_group.front()->intensity);
        int vid_begin,vid_end;
        vid_begin=vid_end=0;
        //qDebug()<<"starting intensity floor:"<<intensity_floor;
        while(intensity_floor>0)
        {
            //qDebug()<<"intensity floor:"<<intensity_floor;
            while(vid_end<tmp_group.size() && tmp_group[vid_end]->intensity>=intensity_floor){
                vid_end++;
                continue;
            }
            pre_layer=cur_layer;
            cur_layer.clear();
            reverse(pre_layer.begin(),pre_layer.end()); //switch the order to balance the grow
            if(pre_layer.size()>0){ //grow the connected part to the previous round first
                seeds=pre_layer;
                for(V3DLONG sid=0; sid<seeds.size(); sid++){
                    VOI * tmp_voxel = seeds[sid];
                    for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                        if(tmp_voxel->neighbors_26[neid]->intensity>=intensity_floor
                                && tmp_voxel->neighbors_26[neid]->dst_label==tmp_voxel->dst_label)
                        {
                            if(tmp_voxel->neighbors_26[neid]->intensity_label<0) //if not visited and within layer
                            {

                                VOI * tmp_seed = tmp_voxel->neighbors_26[neid];

                                tmp_seed->intensity_label=tmp_voxel->intensity_label;
                                intensity_groups[tmp_seed->intensity_label].push_back(tmp_seed);
                                cur_layer.push_back(tmp_seed);
                                seeds.push_back(tmp_seed);

                            }
                            else if (tmp_voxel->neighbors_26[neid]->intensity_label!=tmp_voxel->intensity_label)
                            //need to check if merge is possible
                            {
                                VOI * tmp_seed = tmp_voxel->neighbors_26[neid];

                                if (intensity_groups[tmp_seed->intensity_label].size()<20 ||intensity_groups[tmp_voxel->intensity_label].size()<20) //merge to this one
                                {
                                    int merged_label = MAX(tmp_seed->intensity_label,tmp_voxel->intensity_label);
                                    int kept_label = MIN (tmp_seed->intensity_label,tmp_voxel->intensity_label);
                                    for (int i=0;i<intensity_groups[merged_label].size();i++)
                                    {
                                        intensity_groups[merged_label][i]->intensity_label=kept_label;
                                        intensity_groups[kept_label].push_back(intensity_groups[merged_label][i]);
                                    }
                                    intensity_groups[merged_label].clear();

                                }
                            }

                        }
                    }
                }
            }

        for(V3DLONG vid=vid_begin; vid<vid_end; vid++){ //check the rest nodes in this layer, grow the rest if there is any left
            if(tmp_group[vid]->intensity_label<0){
                GOV new_group; //a new group of voxels
                new_group.push_back(tmp_group[vid]);
                tmp_group[vid]->intensity_label=intensity_groups.size();
                cur_layer.push_back(tmp_group[vid]);
                //region grow
                seeds.clear();
                seeds.push_back(tmp_group[vid]);
                for(V3DLONG sid=0; sid<seeds.size(); sid++){
                    VOI * tmp_voxel = seeds[sid];
                    for(int neid=0; neid<tmp_voxel->neighbors_26.size(); neid++){
                        if(tmp_voxel->neighbors_26[neid]->intensity_label<0 &&
                                tmp_voxel->neighbors_26[neid]->intensity>=intensity_floor &&
                                tmp_voxel->neighbors_26[neid]->dst_label==tmp_voxel->dst_label)
                        { //if not visited and within layer
                            VOI * tmp_seed = tmp_voxel->neighbors_26[neid];
                            tmp_seed->intensity_label=tmp_voxel->intensity_label;
                            new_group.push_back(tmp_seed);
                            cur_layer.push_back(tmp_seed);
                            seeds.push_back(tmp_seed);

                        }
                    }
                }

                intensity_groups.push_back(new_group);
            }
        }

        vid_begin=vid_end;
        intensity_floor = intensity_floor-param.intensity_step;//voxels[vid_begin]->intensity;
        }
    }
    int count=0;
    for (int i=0;i<intensity_groups.size();i++)
    {
        if (intensity_groups[i].size()<=param.min_pixel)
        {
            GOV tmp=intensity_groups[i];
            for (int j=0;j<tmp.size();j++)
            {
                tmp[j]->intensity_label=-1;
            }
            intensity_groups[i].clear();
        }
        else
            count++;

    }
    qDebug()<<"intensity_group size:"<<count;
    return true;
}


void spine_fun::saveResult()
{
    qDebug()<<"~~~save Results";
    if(FLAG_ISTEST){
        V3DLONG sz_page=sz_img[0]*sz_img[1]*sz_img[2];
        V3DLONG sz_tmp[4]; sz_tmp[0]=sz_img[0]; sz_tmp[1]=sz_img[1]; sz_tmp[2]=sz_img[2]; sz_tmp[3]=3;
        tmp_out = new unsigned char[sz_page*3];
        memset(tmp_out,0,sz_page*3);
        memcpy(tmp_out,p_img1D,sz_page);
        for(int i=0; i<voxels.size(); i++){
            if (voxels.at(i)->dst_label>=3)
                tmp_out[voxels.at(i)->pos+2*sz_page]=255;
            if (voxels.at(i)->intensity_label>0)
                tmp_out[voxels.at(i)->pos+sz_page]=255;

        }
        QString fname_output = fname_out + "_tmp.v3draw";
        simple_saveimage_wrapper(*callback, fname_output.toStdString().c_str(), tmp_out, sz_tmp, 1);

        tmp_label = new unsigned short[sz_page];
        memset(tmp_label,0,sz_page*sizeof(unsigned short));
        label_sz[0]=sz_img[0];
        label_sz[1]=sz_img[1];
        label_sz[2]=sz_img[2];
        label_sz[3]=1;
//        QString fname="spine_fun.marker";
//        FILE *fp1=fopen(fname.toAscii(),"wt");
//        fprintf(fp1,"##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");
        for (V3DLONG i=0;i<voxels.size();i++)
        {
            if (voxels[i]->intensity_label>0)
            {
                tmp_label[voxels.at(i)->pos]=voxels[i]->intensity_label;
//                fprintf(fp1,"%d,%d,%d,1,1,"","",255,255,255\n",tmp_group[tmp_group.size()/2]->x+1,
//                        tmp_group[tmp_group.size()/2]->y+1,tmp_group[tmp_group.size()/2]->z+1);
            }

        }
//        fclose(fp1);
        fname_output = fname_out + "_tmp_label.v3draw";
        simple_saveimage_wrapper(*callback, fname_output.toStdString().c_str(), (unsigned char *)tmp_label, sz_img, 2);
    }
}


void spine_fun::ind2sub(V3DLONG &x, V3DLONG &y, V3DLONG &z, V3DLONG ind)
{
    z=ind/(sz_img[0]*sz_img[1]);
    y=(ind-z*sz_img[0]*sz_img[1])/sz_img[0];
    x=ind-z*sz_img[0]*sz_img[1]-y*sz_img[0];
}

long spine_fun::sub2ind(V3DLONG x, V3DLONG y, V3DLONG z)
{
    return(z*sz_img[0]*sz_img[1]+y*sz_img[0]+x);
}

bool sortfunc_dst(VOI * a, VOI * b){ return (a->dst > b->dst); }
bool sortfunc_intensity(VOI * a, VOI * b){ return (a->intensity > b->intensity); }


void spine_fun::conn_comp_nb6()
{
    final_groups=conn_comp_nb6_imp(intensity_groups);
//    int id=1;
//    for (int i=0;i<intensity_groups.size();i++)
//    {
//        if (intensity_groups[i].size()==0) continue;
//        if (intensity_groups[i].size()>param.intensity_max_pixel) continue;

//        GOV cur_group = intensity_groups[i];
//        //check connected components
//        //qDebug()<<"i:"<<i<<" Id:"<<id<<"group size:"<<cur_group.size();

//        GOV seeds;
//        map<V3DLONG, int> lookup;
//        for (int j=0;j<cur_group.size();j++)
//        {
//            VOI * tmp_voi=cur_group[j];
//            if (lookup[tmp_voi->pos]>0) continue;
//            seeds.clear();
//            lookup[tmp_voi->pos]=id;
//            seeds.push_back(tmp_voi);
//            int sid=0;
//            while (sid<seeds.size())
//            {
//                VOI * single_seed = seeds[sid];
//                for (int neid=0; neid<single_seed->neighbors_6.size();neid++)
//                {
//                    if (lookup[single_seed->neighbors_6[neid]->pos]==0 &&
//                            single_seed->neighbors_6[neid]->intensity_label==
//                            tmp_voi->intensity_label)
//                    {
//                        seeds.push_back(single_seed->neighbors_6[neid]);
//                        lookup[single_seed->neighbors_6[neid]->pos]=id;
//                        single_seed->neighbors_6[neid]->intensity_label=id;
//                    }
//                }
//                sid++;
//            }
//            //qDebug()<<"seeds size:"<<seeds.size();
//            if (seeds.size()<param.min_pixel)
//            {
//                for (int k=0;k<seeds.size();k++)
//                    seeds[k]->intensity_label=-1;
//                continue;
//            }

//            tmp_voi->intensity_label=id;
//            GOV new_seeds;
//            closing(seeds,id,new_seeds);
//            final_groups.push_back(new_seeds);
//            id++;
//        }
//    }
    qDebug()<<"final groups size:"<<final_groups.size();
}


vector<GOV> spine_fun::conn_comp_nb6_imp(vector<GOV> old_groups)
{
    int id=1;
    vector<GOV> new_groups;
    for (int i=0;i<old_groups.size();i++)
    {
        if (old_groups[i].size()==0) continue;
        if (old_groups[i].size()>param.intensity_max_pixel) continue;

        GOV cur_group = old_groups[i];

        GOV seeds;
        map<V3DLONG, int> lookup;
        for (int j=0;j<cur_group.size();j++)
        {
            VOI * tmp_voi=cur_group[j];
            if (lookup[tmp_voi->pos]>0) continue;
            seeds.clear();
            lookup[tmp_voi->pos]=id;
            seeds.push_back(tmp_voi);
            int sid=0;
            while (sid<seeds.size())
            {
                VOI * single_seed = seeds[sid];
                for (int neid=0; neid<single_seed->neighbors_6.size();neid++)
                {
                    if (lookup[single_seed->neighbors_6[neid]->pos]==0 &&
                            single_seed->neighbors_6[neid]->intensity_label==
                            tmp_voi->intensity_label)
                    {
                        seeds.push_back(single_seed->neighbors_6[neid]);
                        lookup[single_seed->neighbors_6[neid]->pos]=id;
                        single_seed->neighbors_6[neid]->intensity_label=id;
                    }
                }
                sid++;
            }
            //qDebug()<<"seeds size:"<<seeds.size();
            if (seeds.size()<param.min_pixel)
            {
                for (int k=0;k<seeds.size();k++)
                    seeds[k]->intensity_label=-1;
                continue;
            }

            tmp_voi->intensity_label=id;
            GOV new_seeds;
            closing(seeds,id,new_seeds);
            new_groups.push_back(new_seeds);
            id++;
        }
    }
    qDebug()<<"final groups size:"<<final_groups.size();
    return new_groups;
}

//vector<GOV> conn_comp_nb6_implement(vector<GOV> old_groups,int intensity_max_pixel,
//                                    int min_pixel,int close_code)
//{
//    int id=1;
//    vector<GOV> new_groups;
//    for (int i=0;i<old_groups.size();i++)
//    {
//        if (old_groups[i].size()==0) continue;
//        if (old_groups[i].size()>intensity_max_pixel) continue;

//        GOV cur_group = old_groups[i];

//        GOV seeds;
//        map<V3DLONG, int> lookup;
//        for (int j=0;j<cur_group.size();j++)
//        {
//            VOI * tmp_voi=cur_group[j];
//            if (lookup[tmp_voi->pos]>0) continue;
//            seeds.clear();
//            lookup[tmp_voi->pos]=id;
//            seeds.push_back(tmp_voi);
//            int sid=0;
//            while (sid<seeds.size())
//            {
//                VOI * single_seed = seeds[sid];
//                for (int neid=0; neid<single_seed->neighbors_6.size();neid++)
//                {
//                    if (lookup[single_seed->neighbors_6[neid]->pos]==0 &&
//                            single_seed->neighbors_6[neid]->intensity_label==
//                            tmp_voi->intensity_label)
//                    {
//                        seeds.push_back(single_seed->neighbors_6[neid]);
//                        lookup[single_seed->neighbors_6[neid]->pos]=id;
//                        single_seed->neighbors_6[neid]->intensity_label=id;
//                    }
//                }
//                sid++;
//            }
//            //qDebug()<<"seeds size:"<<seeds.size();
//            if (seeds.size()<min_pixel)
//            {
//                for (int k=0;k<seeds.size();k++)
//                    seeds[k]->intensity_label=-1;
//                continue;
//            }

//            tmp_voi->intensity_label=id;

//            GOV new_seeds;
//            closing(seeds,id,new_seeds);
//            new_groups.push_back(new_seeds);
//            id++;
//        }
//    }
//    qDebug()<<"final groups size:"<<final_groups.size();
//    return new_groups;
//}

void spine_fun::closing(GOV seeds, int id, GOV &new_seeds)
{
    GOV seeds_next;
    for (int sid=0;sid<seeds.size();sid++)
    {
        VOI * single_seed = seeds[sid];
        for (int neid=0; neid<single_seed->neighbors_6.size();neid++)
        {
            if (p_img1D[single_seed->neighbors_6[neid]->pos]>param.bgthr &&
                    single_seed->neighbors_6[neid]->intensity_label<1)
            {
                single_seed->neighbors_6[neid]->intensity_label=id;
                seeds_next.push_back(single_seed->neighbors_6[neid]);
            }
        }
    }
    seeds.insert(seeds.end(),seeds_next.begin(),seeds_next.end());
    //then erode
    for (int sid=0;sid<seeds.size();sid++)
    {
        VOI * single_seed = seeds[sid];
        bool flag=false;
        for (int neid=0; neid<single_seed->neighbors_6.size();neid++)
        {
            if (p_img1D[single_seed->neighbors_6[neid]->pos]<param.bgthr)
            {
                single_seed->neighbors_6[neid]->intensity_label=-1;
                flag=true;
                break;
            }
        }
        if (!flag) new_seeds.push_back(single_seed);
    }
//    qDebug()<<"before closing"<<seeds.size()-seeds_next.size()<<" after closing:"
//           <<new_seeds.size();
}

void spine_fun::write_spine_center_profile()
{
    qDebug()<<"in write spine center profile";

    QString fname="spine_fun.marker";
    FILE *fp1=fopen(fname.toAscii(),"wt");
    fprintf(fp1,"##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");

    QString outfile="spine_analysis2.csv";
    FILE *fp2=fopen(outfile.toAscii(),"wt");
    fprintf(fp2,"##id,volume,max_dis,min_dis,center_dis,center_x,center_y,center_z,skel_node,skel_type,skel_node_seg,skel_node_branch,dis_to_root\n");
    for (int i=0;i<final_groups.size();i++)
    {
        GOV tmp=final_groups[i];
        //qDebug()<<"I:"<<i<<" size:"<<tmp.size();
        sort(tmp.begin(),tmp.end(),sortfunc_dst);
        int group_id=tmp.front()->intensity_label;
        int max_dis=tmp.front()->dst;
        int min_dis=tmp.back()->dst;
        int volume=tmp.size();
        V3DLONG sum_x,sum_y,sum_z,sum_dis;
        sum_x=sum_y=sum_z=sum_dis=0;
        for (int j=0;j<tmp.size();j++)
        {
            sum_x+=tmp[j]->x;
            sum_y+=tmp[j]->y;
            sum_z+=tmp[j]->z;
            sum_dis+=tmp[j]->dst;
        }
        int center_x=sum_x/tmp.size();
        int center_y=sum_y/tmp.size();
        int center_z=sum_z/tmp.size();
        int center_dis=sum_dis/tmp.size();
        int skel_id=0;
        for (int j=0;j<tmp.size();j++)
        {
            VOI *tmp_voi=tmp[j];
            if ((tmp_voi->x==center_x) && (tmp_voi->y==center_y) && (tmp_voi->z==center_z))
            {
                skel_id=tmp_voi->skel_idx;
                break;
            }
        }

        fprintf(fp1,"%d,%d,%d,1,1,"","",255,255,255\n",center_x+1,center_y+1,center_z+1);

        fprintf(fp2,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f\n",group_id,volume,max_dis,
                min_dis,center_dis,center_x,center_y,center_z,skel_id,nt.listNeuron.at(skel_id).type,nt.listNeuron.at(skel_id).seg_id,
                nt.listNeuron.at(skel_id).level, nt.listNeuron.at(skel_id).fea_val[1]);

    }
    fclose(fp1);
    fclose(fp2);
}

LandmarkList spine_fun::get_center_landmarks()
{
    LandmarkList LList;
    for (int i=0;i<final_groups.size();i++)
    {
        GOV tmp=final_groups[i];
        V3DLONG sum_x,sum_y,sum_z;
        sum_x=sum_y=sum_z=0;
        for (int i=0;i<tmp.size();i++)
        {
            sum_x+=tmp[i]->x;
            sum_y+=tmp[i]->y;
            sum_z+=tmp[i]->z;
        }

        LocationSimple tmp_marker;
        tmp_marker.x=sum_x/tmp.size()+1;
        tmp_marker.y=sum_y/tmp.size()+1;
        tmp_marker.z=sum_z/tmp.size()+1;
        tmp_marker.color.r=0;
        tmp_marker.color.g=0;
        tmp_marker.color.b=0;
        tmp_marker.name = QString::number(tmp[0]->intensity_label).toStdString().c_str();
        tmp_marker.comments = QString::number(0).toStdString().c_str();
        LList.append(tmp_marker);
    }
    return LList;
}

