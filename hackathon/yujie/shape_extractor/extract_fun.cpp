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
                             vector<V3DLONG> &masscenter,V3DLONG seed_ind, int convolute_iter,
                              int neighbor_size, int bg_thr)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr);
    qDebug()<<"dir size"<<dir.size();
    if(dir.size()<=0){
        return 0;
    }

    //start region grow
    vector<V3DLONG> seeds;
    V3DLONG delta=neighbor_size/2;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    masscenter.clear();
    page_size=sz_image[0]*sz_image[1]*sz_image[2];
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                mask1D[pos]=1;

                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos+cid*page_size];
                }
                project=getProjection(color, dir, convolute_iter);
                if(project<bg_thr)
                    continue;

                x_all.push_back(dx);
                y_all.push_back(dy);
                z_all.push_back(dz);

                seeds.push_back(pos);
            }
        }
    }


    //seed grow
    qDebug()<<"Seed starts growing";
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];

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

                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);

                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }
    qDebug()<<"Before masscenter calc";
    masscenter=get_mass_center(x_all,y_all,z_all);
    qDebug()<<"After masscenter calc";
    return seeds.size();
}

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
        qDebug()<<"In get_mass_center now";
        V3DLONG total_x,total_y,total_z,sum_v;
        vector<V3DLONG> center(3,0);
        total_x=total_y=total_z=sum_v=0;

        qDebug()<<"1";
        for (int i=0;i<x_all.size();i++)
        {

            V3DLONG pos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_image[0],sz_image[0]*sz_image[1]);
            float v=data1Dc_float[pos];
            for (int j=0;j<sz_image[3];j++)
                v=MAX(v,data1Dc_float[pos+page_size*j]);
            total_x=v*x_all[i]+total_x;
            total_y=v*y_all[i]+total_y;
            total_z=v*z_all[i]+total_z;
            sum_v=sum_v+v;
        }
        qDebug()<<"2";
        qDebug()<<"3"<<total_x<<total_y<<total_z;
        qDebug()<<x_all.size();
        if (x_all.size()>0) {
            qDebug()<<"3";
            qDebug()<<"3"<<center[0]<<center[1]<<center[2];
            qDebug()<<"3"<<total_x<<total_y<<total_z;
            center[0]=total_x/sum_v;
            center[1]=total_y/sum_v;
            center[2]=total_z/sum_v;
        }

        qDebug()<<"At the end of mass center calc";
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
