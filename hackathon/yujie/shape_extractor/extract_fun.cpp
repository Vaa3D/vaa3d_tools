#include "extract_fun.h"

extract_fun::extract_fun()
{
    data1Dc_float=0;
    mask1D=0;
    sz_image[0]=sz_image[1]=sz_image[2]=sz_image[3]=0;
    page_size=0;
}

extract_fun::~extract_fun()
{
    if(data1Dc_float!=0)
        memory_free_float1D(data1Dc_float);
    if(mask1D!=0)
        memory_free_uchar1D(mask1D);
}


V3DLONG extract_fun::extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                             V3DLONG seed_ind, int convolute_iter,
                              int neighbor_size, int bg_thr,double percent_thr)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //1) Collect user specified point info
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr);
    if(dir.size()<=0){
        return 0;
    }

    //vector<V3DLONG> x_tmp,y_tmp,z_tmp;
    vector<V3DLONG> seeds;
   // V3DLONG delta=1; //neighbor_size/2;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    vector<V3DLONG> new_mass_center;
    page_size=sz_image[0]*sz_image[1]*sz_image[2];
    memset(mask1D, 0, page_size*sizeof(unsigned char));
    unsigned char * seed_mask;
    seed_mask=memory_allocate_uchar1D(page_size*sizeof(unsigned char));
    memset(seed_mask,0,page_size*sizeof(unsigned char));
    float project;

    coord=pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    V3DLONG r_grow=1;
    float total_count=1.0;
    float fore_count=1.0;

    while(fore_count/total_count>percent_thr)
    {
        for(V3DLONG dx=MAX(x-r_grow,0); dx<=MIN(sz_image[0]-1,x+r_grow); dx++){
            for(V3DLONG dy=MAX(y-r_grow,0); dy<=MIN(sz_image[1]-1,y+r_grow); dy++){
                for(V3DLONG dz=MAX(z-r_grow,0); dz<=MIN(sz_image[2]-1,z+r_grow); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (mask1D[pos]>0) continue;

                    //distance to user specified point
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
                    double distance=pow(tmp,1./2.);
                    if (distance>r_grow) continue;

                    for(int cid=0; cid<sz_image[3]; cid++){
                        color[cid]=data1Dc_float[pos+cid*page_size];
                    }
                    project=getProjection(color, dir, convolute_iter);
                    if(project<bg_thr)  continue;
                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);
                    seeds.push_back(pos);
                    mask1D[pos]=1;
                }
            }
        }

        //Calculate the new mass center,new radius,new seeds
        new_mass_center=get_mass_center(x_all,y_all,z_all);
        x=new_mass_center[0];
        y=new_mass_center[1];
        z=new_mass_center[2];

        double tmp_num=(double)seeds.size()*3/(4*pi);
        double new_r=pow(tmp_num,1./3.);

        fore_count=0;

        for(V3DLONG dx=MAX(x-new_r,0); dx<=MIN(sz_image[0]-1,x+new_r); dx++){
            for(V3DLONG dy=MAX(y-new_r,0); dy<=MIN(sz_image[1]-1,y+new_r); dy++){
                for(V3DLONG dz=MAX(z-new_r,0); dz<=MIN(sz_image[2]-1,z+new_r); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    //distance to new center
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
                    double distance=pow(tmp,1./2.);
                    if (distance>new_r) continue;
                    for(int cid=0; cid<sz_image[3]; cid++){
                     color[cid]=data1Dc_float[pos+cid*page_size];
                    }
                    project=getProjection(color, dir, convolute_iter);
                    if(project<bg_thr)  continue;
                    if(seed_mask[pos]==0) seed_mask[pos]=1;
                    fore_count=fore_count+1;
                }
            }
        }
        total_count=4*pi*pow(new_r,3)/3;
        r_grow=r_grow+1;
        qDebug()<<"fore_count/total_count:"<<fore_count/total_count;
        qDebug()<<"New r:"<<new_r;
        qDebug()<<"r_grow:"<<r_grow;
    }
   return seeds.size();
}





//    if (x_tmp.size()<=0){
//        v3d_msg("No seeds were found");
//        return 0;
//    }

//    //3)Calculate the new mass center,new radius,new seeds
//    float total_count=0;
//    float fore_count=0;
//    new_mass_center=get_mass_center(x_tmp,y_tmp,z_tmp);
//    double tmp_num=(double)seeds_tmp.size()*3/(4*pi);
//    double new_r=pow(tmp_num,1./3.);
//    qDebug()<<"seeds_tmp size and x_tmp size:"<<seeds_tmp.size()<<x_tmp.size();

//    x=new_mass_center[0];
//    y=new_mass_center[1];
//    z=new_mass_center[2];

//    //Get the new seeds
//    for(V3DLONG dx=MAX(x-new_r,0); dx<=MIN(sz_image[0]-1,x+new_r); dx++){
//        for(V3DLONG dy=MAX(y-new_r,0); dy<=MIN(sz_image[1]-1,y+new_r); dy++){
//            for(V3DLONG dz=MAX(z-new_r,0); dz<=MIN(sz_image[2]-1,z+new_r); dz++){
//                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);



//                total_count=total_count+1;
//                for(int cid=0; cid<sz_image[3]; cid++){
//                    color[cid]=data1Dc_float[pos1+cid*page_size];
//                }
//                project=getProjection(color, dir, convolute_iter);
//                if(project>bg_thr&&distance<r_grow)
//                {
//                    fore_count=fore_count+1;
//                    x_all.push_back(dx);
//                    y_all.push_back(dy);
//                    z_all.push_back(dz);
//                    seeds.push_back(pos1);
//                    seed_mask[pos1]=1;
//                }
//            }
//        }
//     }

//     if (seeds.size()<=0) v3d_msg("No seeds were found");
//     if (fore_count/total_count<percent_thr){
//         v3d_msg("Cell is not found.");
//         return 0;}

//     while(fore_count/total_count>=percent_thr){
//        //repeat 2): grow one round
//        x_tmp.clear();y_tmp.clear();z_tmp.clear();
//        new_mass_center=get_mass_center(x_all,y_all,z_all);
//        double tmp_num=(double)x_all.size()*3/(4*pi);
//        double new_r=pow(tmp_num,1./3.);
//        x=new_mass_center[0]; y=new_mass_center[1]; z=new_mass_center[2];

////        V3DLONG sid=0;
////        while(sid<seeds.size()){
////            coord=pos2xyz(seeds[sid], y_offset, z_offset);
////            x=coord[0];y=coord[1];z=coord[2];
////            for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
////                for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
////                    for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
////                        pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
////                        if(mask1D[pos]>0) continue;
////                        mask1D[pos]=1;
////                        for(int cid=0; cid<sz_image[3]; cid++){
////                            color[cid]=data1Dc_float[pos+cid*page_size];
////                        }
////                        project=getProjection(color, dir, convolute_iter);
////                        if(project<bg_thr) continue;
////                        x_tmp.push_back(dx);
////                        y_tmp.push_back(dy);
////                        z_tmp.push_back(dz);
////                        seeds_tmp.push_back(pos);
////                    }
////                }
////            }
////            sid++;
////        }
////        if (x_tmp.size()<=0)
////        {
////            qDebug()<<"No seeds found in seeds_tmp";
////            return 0;
////        }
//        //repeat 3) new center,radius,seeds

//        //3)Calculate the new mass center,new radius,new seeds
//       total_count=0;
//       fore_count=0;
//       new_mass_center=get_mass_center(x_tmp,y_tmp,z_tmp);
//       tmp_num=(double)seeds_tmp.size()*3/(4*pi);
//       new_r=pow(tmp_num,1./3.);
//       x=new_mass_center[0]; y=new_mass_center[1]; z=new_mass_center[2];

//        //Get the new seeds

//        for(V3DLONG dx=MAX(x-new_r,0); dx<=MIN(sz_image[0]-1,x+new_r); dx++){
//            for(V3DLONG dy=MAX(y-new_r,0); dy<=MIN(sz_image[1]-1,y+new_r); dy++){
//                for(V3DLONG dz=MAX(z-new_r,0); dz<=MIN(sz_image[2]-1,z+new_r); dz++){
//                    pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                    total_count=total_count+1;

//                    for(int cid=0; cid<sz_image[3]; cid++){
//                        color[cid]=data1Dc_float[pos1+cid*page_size];
//                    }
//                    project=getProjection(color, dir, convolute_iter);
//                    if(project>bg_thr)
//                    {
//                        fore_count=fore_count+1;
//                        if (seed_mask[pos1]>0) continue;
//                        seed_mask[pos1]=1;
//                        x_all.push_back(dx);
//                        y_all.push_back(dy);
//                        z_all.push_back(dz);
//                        seeds_next.push_back(pos1);
//                    }
//                }
//            }
//        }
//        if (seeds_next.size()<=0)
//        {
//         qDebug()<<"No seeds in seeds_next";
//         return 0;
//        }
//        seeds=seeds_next;
//        qDebug()<<"fore_count/total_count:"<<fore_count/total_count;
//        qDebug()<<"Current size of r:"<<new_r;
//    }

//    return seeds.size();
//}

float extract_fun::getProjection(vector<float> vec, vector<float> dir, int convolute_iter)
{
    float dp=0, norm=0;
    for(int i=0; i<vec.size(); i++){
        dp+=vec[i]*dir[i];
        norm+=vec[i]*vec[i];
    }
    norm=sqrt(norm);
    float scale;
    float proj=dp;
    if(norm>1e-16)
        scale=dp/norm;
    else
        scale=1;
    for(int i=0; i<convolute_iter; i++){
        proj*=scale;
    }
    return proj;
}

vector<float> extract_fun::getProjectionDirection(V3DLONG seed_ind, int neighbor_size, int bg_thr)
{
    vector<V3DLONG> seed_coord = pos2xyz(seed_ind, sz_image[0], sz_image[1]*sz_image[0]);
    int delta=neighbor_size/2;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    V3DLONG pos;

    int v_count=0;
    vector<float> dir(sz_image[3],0);
    for(V3DLONG dx=MAX(seed_coord[0]-delta,0); dx<=MIN(sz_image[0]-1,seed_coord[0]+delta); dx++){
        for(V3DLONG dy=MAX(seed_coord[1]-delta,0); dy<=MIN(sz_image[1]-1,seed_coord[1]+delta); dy++){
            for(V3DLONG dz=MAX(seed_coord[2]-delta,0); dz<=MIN(sz_image[2]-1,seed_coord[2]+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                float v=data1Dc_float[pos];
                for(V3DLONG pid=1; pid<sz_image[3]; pid++)
                    v=MAX(v,data1Dc_float[pos+page_size*pid]);

                if(v<bg_thr)    continue;

                for(V3DLONG pid=0; pid<sz_image[3]; pid++)
                    dir[pid]+=data1Dc_float[pos+page_size*pid];
                v_count++;

            }
        }
    }
    if(v_count==0){
        qDebug()<<"Warning: NeuronPicker found empty seed area, please decrease the threshold or reselect an area!";
        dir.clear();
        return dir;
    }
    //normalize it
    float norm=0;
    for(V3DLONG pid=0; pid<sz_image[3]; pid++){
        dir[pid]/=v_count;
        norm+=dir[pid]*dir[pid];
    }
    norm=sqrt(norm);
    for(V3DLONG pid=0; pid<sz_image[3]; pid++){
        dir[pid]/=norm;
    }
    return dir;
}


vector<V3DLONG> extract_fun::get_mass_center(vector<V3DLONG> x_all, vector<V3DLONG> y_all,
                                             vector<V3DLONG> z_all)
{
        float total_x,total_y,total_z;
        float v,sum_v;
        vector<V3DLONG> center(3,0);
        total_x=total_y=total_z=sum_v=0;

        for (int i=0;i<x_all.size();i++)
        {

            V3DLONG pos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_image[0],sz_image[0]*sz_image[1]);
//            float v=data1Dc_float[pos];
//            for (int j=0;j<sz_image[3];j++)
//            v=MAX(v,data1Dc_float[pos+page_size*j]);
//            total_x=v*x_all[i]+total_x;
//            total_y=v*y_all[i]+total_y;
//            total_z=v*z_all[i]+total_z;
//            sum_v=sum_v+v;
            total_x=x_all[i]+total_x;
            total_y=y_all[i]+total_y;
            total_z=z_all[i]+total_z;

        }
            sum_v=x_all.size();
        if (x_all.size()>0) {

            center[0]=total_x/sum_v;
            center[1]=total_y/sum_v;
            center[2]=total_z/sum_v;
        }
        //qDebug()<<"center:"<<center[0]<<","<<center[1]<<","<<center[2]<<","<<center[3];
        return center;
}

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size)
{
    unsigned char *ptr_result;
    ptr_result=(unsigned char *) calloc(i_size, sizeof(unsigned char));
    return(ptr_result);
}

void memory_free_uchar1D(unsigned char *ptr_input)
{
    free(ptr_input);
}

float * memory_allocate_float1D(const V3DLONG i_size)
{
    float *ptr_result;
    ptr_result=(float *) calloc(i_size, sizeof(float));
    return(ptr_result);
}

void memory_free_float1D(float *ptr_input)
{
    free(ptr_input);
}



vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
{
    vector<V3DLONG> poss_result;
    V3DLONG count_landmark=LandmarkList_input.count();
    for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
    {
        poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
    }
    return poss_result;
}

V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z)
{
    float x=0;
    float y=0;
    float z=0;
    Landmark_input.getCoord(x, y, z);
    x=MAX(x,0);
    y=MAX(y,0);
    z=MAX(z,0);
    return (xyz2pos(x-1, y-1, z-1, _offset_Y, _offset_Z));
}

vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    vector<V3DLONG> pos3_result (3, -1);
    pos3_result[2]=floor(_pos_input/(double)_offset_Z);
    pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
    pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
    return pos3_result;
}

V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    return _z*_offset_Z+_y*_offset_Y+_x;
}


V3DLONG extract_fun::extract_check(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                             V3DLONG seed_ind, int convolute_iter,
                              int neighbor_size, int bg_thr,int rounds)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //1) Collect user specified point info
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr);
    if(dir.size()<=0){
        return 0;
    }

    vector<V3DLONG> x_tmp,y_tmp,z_tmp;
    vector<V3DLONG> seeds_tmp;
    vector<V3DLONG> seeds;
    vector<V3DLONG> seeds_next;
    V3DLONG delta=1; //neighbor_size/2;
    V3DLONG x,y,z,pos,pos1;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    vector<V3DLONG> new_mass_center;
    page_size=sz_image[0]*sz_image[1]*sz_image[2];
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    unsigned char * seed_mask;
    seed_mask=memory_allocate_uchar1D(sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    memset(seed_mask,0,sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    //2) Grow one round (only look at the six neighbors around one point,delta=1)

    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);

                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos+cid*page_size];
                }
                project=getProjection(color, dir, convolute_iter);
                if(project<bg_thr)
                    continue;

                x_tmp.push_back(dx);
                y_tmp.push_back(dy);
                z_tmp.push_back(dz);
                seeds_tmp.push_back(pos);
            }
        }
    }

    if (seeds_tmp.size()<=0){
        v3d_msg("No seeds were found");
        return 0;
    }

    //3)Calculate the new mass center,new radius,new seeds
    float total_count=0;
    float fore_count=0;
    new_mass_center=get_mass_center(x_tmp,y_tmp,z_tmp);
    double tmp_num=(double)seeds.size()*3/(4*pi);
    double new_r=pow(tmp_num,1./3.);

    x=new_mass_center[0];
    y=new_mass_center[1];
    z=new_mass_center[2];

    //Get the new seeds
    for(V3DLONG dx=MAX(x-new_r,0); dx<=MIN(sz_image[0]-1,x+new_r); dx++){
        for(V3DLONG dy=MAX(y-new_r,0); dy<=MIN(sz_image[1]-1,y+new_r); dy++){
            for(V3DLONG dz=MAX(z-new_r,0); dz<=MIN(sz_image[2]-1,z+new_r); dz++){
                pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                total_count=total_count+1;
                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos1+cid*page_size];
                }
                project=getProjection(color, dir, convolute_iter);
                if(project>bg_thr)
                {
                    fore_count=fore_count+1;
                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);
                    seeds.push_back(pos1);
                    seed_mask[pos1]=1;

                }
            }
        }

     }

     if (fore_count/total_count<0.6){
         v3d_msg("Cell is not found.");
         return 0;}

 for (int k=0;k<rounds;k++)
 {
    //repeat 2): grow one round
    x_tmp.clear();y_tmp.clear();z_tmp.clear();
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];y=coord[1];z=coord[2];

        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if(mask1D[pos]>0) continue;
                    mask1D[pos]=1;
                    for(int cid=0; cid<sz_image[3]; cid++){
                        color[cid]=data1Dc_float[pos+cid*page_size];
                    }
                    project=getProjection(color, dir, convolute_iter);
                    if(project<bg_thr) continue;

                    x_tmp.push_back(dx);
                    y_tmp.push_back(dy);
                    z_tmp.push_back(dz);
                    seeds_tmp.push_back(pos);
                }
            }
        }
        sid++;
    }
    if (seeds_tmp.size()<=0) {return 0;}
    //repeat 3) new center,radius,seeds

    //3)Calculate the new mass center,new radius,new seeds
   total_count=0;
   fore_count=0;
   new_mass_center=get_mass_center(x_tmp,y_tmp,z_tmp);
   tmp_num=(double)seeds_tmp.size()*3/(4*pi);
   new_r=pow(tmp_num,1./3.);
   x=new_mass_center[0]; y=new_mass_center[1]; z=new_mass_center[2];

    //Get the new seeds

    for(V3DLONG dx=MAX(x-new_r,0); dx<=MIN(sz_image[0]-1,x+new_r); dx++){
        for(V3DLONG dy=MAX(y-new_r,0); dy<=MIN(sz_image[1]-1,y+new_r); dy++){
            for(V3DLONG dz=MAX(z-new_r,0); dz<=MIN(sz_image[2]-1,z+new_r); dz++){
                pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                total_count=total_count+1;

                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos1+cid*page_size];
                }
                project=getProjection(color, dir, convolute_iter);
                if(project>bg_thr)
                {
                    fore_count=fore_count+1;
                    if (seed_mask[pos1]>0) continue;
                    seed_mask[pos1]=1;
                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);
                    seeds_next.push_back(pos1);
                }
            }
        }
     }
    if (seeds_next.size()<=0) {return 0;}
    seeds=seeds_next;
    qDebug()<<"fore_count/total_count:"<<fore_count/total_count;
    qDebug()<<"Current size of r:"<<new_r;
    if (fore_count/total_count<0.3) {return 0;}
}

    return seeds.size();
}
