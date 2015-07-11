#include "neuronpicker_func.h"

neuronPickerMain2::neuronPickerMain2()
{
    data1Dc_float=0;
    mask1D=0;
    sz_image[0]=sz_image[1]=sz_image[2]=sz_image[3];
    page_size=0;
    innerScale=1;
}

neuronPickerMain2::~neuronPickerMain2()
{
    if(data1Dc_float!=0)
        neuronPickerMain::memory_free_float1D(data1Dc_float);
    if(mask1D!=0)
        neuronPickerMain::memory_free_uchar1D(mask1D);
}

//image1D_out do not need to get initialized, will be re-initialized
V3DLONG neuronPickerMain2::extractSub_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG& pos_new, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr)
{
    if(image1D_out!=0){
        neuronPickerMain::memory_free_uchar1D(image1D_out);
        image1D_out=0;
    }

    sz_out[0]=sz_out[1]=sz_out[2]=0;
    sz_out[3]=1;

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
    if(dir.size()<=0){
        return 0;
    }

    QString tmp_str="";
    for(V3DLONG cid=0; cid<sz_image[3]; cid++){
        tmp_str=tmp_str+":"+QString::number(dir[cid]*255);
    }
    qDebug()<<"==========NeuronPicker:\tseed:"<<seed_ind<<"\tcolor:"<<tmp_str;

    //start region grow
    vector<V3DLONG> seeds;
    vector<V3DLONG> x_all, y_all, z_all;
    vector<unsigned char> project_all;
    V3DLONG delta=neighbor_size/2;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    //for subspace calculation
    V3DLONG x_max, x_min, y_max, y_min, z_max, z_min;
    x_max=x_min=x;
    y_max=y_min=y;
    z_max=z_min=z;

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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
                project_all.push_back((unsigned char) project);
                seeds.push_back(pos);

                //calculate sub-space
                x_max=MAX(x_max,dx);
                x_min=MIN(x_min,dx);
                y_max=MAX(y_max,dy);
                y_min=MIN(y_min,dy);
                z_max=MAX(z_max,dz);
                z_min=MIN(z_min,dz);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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
                    project_all.push_back((unsigned char) project);
                    seeds.push_back(pos);

                    //calculate sub-space
                    x_max=MAX(x_max,dx);
                    x_min=MIN(x_min,dx);
                    y_max=MAX(y_max,dy);
                    y_min=MIN(y_min,dy);
                    z_max=MAX(z_max,dz);
                    z_min=MIN(z_min,dz);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    //generate output matrix in sub-space
    sz_out[0]=x_max-x_min+2;
    sz_out[1]=y_max-y_min+2;
    sz_out[2]=z_max-z_min+2;
    sz_out[3]=1;

    image1D_out=neuronPickerMain::memory_allocate_uchar1D(sz_out[0]*sz_out[1]*sz_out[2]);
    memset(image1D_out, 0, sz_out[0]*sz_out[1]*sz_out[2]*sizeof(unsigned char));
    for(int i=0; i<project_all.size(); i++){
        //sub-space
        x=x_all[i]-x_min+1;
        y=y_all[i]-y_min+1;
        z=z_all[i]-z_min+1;

        image1D_out[neuronPickerMain::xyz2pos(x,y,z,sz_out[0],sz_out[1]*sz_out[0])]=project_all[i];
    }
    //markers in subspace
    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    pos_new=neuronPickerMain::xyz2pos(coord[0]-x_min+1,coord[1]-y_min+1,coord[2]-z_min+1,sz_out[0],sz_out[0]*sz_out[1]);

    return seeds.size();
}

//image1D_out do not need to get initialized, will be re-initialized
V3DLONG neuronPickerMain2::extract_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr)
{
    if(image1D_out!=0){
        neuronPickerMain::memory_free_uchar1D(image1D_out);
        image1D_out=0;
    }

    sz_out[0]=sz_out[1]=sz_out[2]=0;
    sz_out[3]=1;

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
    if(dir.size()<=0){
        return 0;
    }

    QString tmp_str="";
    for(V3DLONG cid=0; cid<sz_image[3]; cid++){
        tmp_str=tmp_str+":"+QString::number(dir[cid]*255);
    }
    qDebug()<<"==========NeuronPicker:\tseed:"<<seed_ind<<"\tcolor:"<<tmp_str;

    //start region grow
    vector<V3DLONG> seeds;
    vector<V3DLONG> x_all, y_all, z_all;
    vector<float> project_all;
    V3DLONG delta=neighbor_size/2;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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
                project_all.push_back(project);
                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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
                    project_all.push_back((unsigned char) project);
                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";
    //generate output matrix in original space
    sz_out[0]=sz_image[0];
    sz_out[1]=sz_image[1];
    sz_out[2]=sz_image[2];
    sz_out[3]=1;

    image1D_out=neuronPickerMain::memory_allocate_uchar1D(sz_out[0]*sz_out[1]*sz_out[2]);
    memset(image1D_out, 0, sz_out[0]*sz_out[1]*sz_out[2]*sizeof(unsigned char));
    for(int i=0; i<project_all.size(); i++){
        //original space
        x=x_all[i];
        y=y_all[i];
        z=z_all[i];

        image1D_out[neuronPickerMain::xyz2pos(x,y,z,sz_out[0],sz_out[1]*sz_out[0])]=(unsigned char)MIN(project_all[i]*innerScale,255);
    }

    return seeds.size();
}


//image1D_out do not need to get initialized, will be re-initialized
V3DLONG neuronPickerMain2::extractMargin_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr, int margin_size)
{
    if(image1D_out!=0){
        neuronPickerMain::memory_free_uchar1D(image1D_out);
        image1D_out=0;
    }

    sz_out[0]=sz_out[1]=sz_out[2]=0;
    sz_out[3]=1;

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
    if(dir.size()<=0){
        return 0;
    }

    QString tmp_str="";
    for(V3DLONG cid=0; cid<sz_image[3]; cid++){
        tmp_str=tmp_str+":"+QString::number(dir[cid]*255);
    }
    qDebug()<<"==========NeuronPicker:\tseed:"<<seed_ind<<"\tcolor:"<<tmp_str;

    //start region grow
    vector<V3DLONG> seeds;
    vector<V3DLONG> x_all, y_all, z_all;
    vector<unsigned char> project_all;
    V3DLONG delta=neighbor_size/2;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord;
    vector<float> color(sz_image[3]);
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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
                project_all.push_back((unsigned char) project);
                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";
    //generate output matrix in original space
    sz_out[0]=sz_image[0];
    sz_out[1]=sz_image[1];
    sz_out[2]=sz_image[2];
    sz_out[3]=1;

    for(int i=0; i<x_all.size(); i++){
        x=x_all[i];
        y=y_all[i];
        z=z_all[i];
        for(V3DLONG dx=MAX(x-margin_size,0); dx<=MIN(sz_image[0]-1,x+margin_size); dx++){
            for(V3DLONG dy=MAX(y-margin_size,0); dy<=MIN(sz_image[1]-1,y+margin_size); dy++){
                for(V3DLONG dz=MAX(z-margin_size,0); dz<=MIN(sz_image[2]-1,z+margin_size); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
                    mask1D[pos]=1;
                }
            }
        }
    }

    image1D_out=neuronPickerMain::memory_allocate_uchar1D(sz_out[0]*sz_out[1]*sz_out[2]);
    memset(image1D_out, 0, sz_out[0]*sz_out[1]*sz_out[2]*sizeof(unsigned char));
    for(int i=0; i<page_size; i++){
        if(mask1D[i]<1)
            continue;
        for(int cid=0; cid<sz_image[3]; cid++){
            color[cid]=data1Dc_float[i+cid*page_size];
        }
        project=getProjection(color, dir, convolute_iter);

        image1D_out[i]=(unsigned char)MIN(project*innerScale,255);
    }

    return seeds.size();
}

V3DLONG neuronPickerMain2::extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all, vector<V3DLONG>& z_all, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
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
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    V3DLONG x_max, x_min, y_max, y_min, z_max, z_min;
    x_max=x_min=x;
    y_max=y_min=y;
    z_max=z_min=z;

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
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

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    return seeds.size();
}

V3DLONG neuronPickerMain2::extract_eng(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all, vector<V3DLONG>& z_all, float* energy, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
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
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    V3DLONG x_max, x_min, y_max, y_min, z_max, z_min;
    x_max=x_min=x;
    y_max=y_min=y;
    z_max=z_min=z;

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
                mask1D[pos]=1;

                float v=0;
                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos+cid*page_size];
                    v=MAX(v,color[cid]);
                }
                project=getProjection(color, dir, convolute_iter);
                project*=(energy[pos]/v);
                if(project<bg_thr)
                    continue;

                x_all.push_back(dx);
                y_all.push_back(dy);
                z_all.push_back(dz);

                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if(mask1D[pos]>0) continue;

                    mask1D[pos]=1;

                    float v=0;
                    for(int cid=0; cid<sz_image[3]; cid++){
                        color[cid]=data1Dc_float[pos+cid*page_size];
                        v=MAX(v,color[cid]);
                    }
                    project=getProjection(color, dir, convolute_iter);
                    project*=(energy[pos]/v);
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

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    return seeds.size();
}

V3DLONG neuronPickerMain2::extractMore(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all, vector<V3DLONG>& z_all, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr)
{
    x_all.clear();
    y_all.clear();
    z_all.clear();

    //get the projection direction from the seed
    vector<float> dir = getProjectionDirection(seed_ind, neighbor_size, bg_thr, convolute_iter);
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
    memset(mask1D, 0, sz_image[0]*sz_image[1]*sz_image[2]*sizeof(unsigned char));
    float project;

    coord=neuronPickerMain::pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];

    V3DLONG x_max, x_min, y_max, y_min, z_max, z_min;
    x_max=x_min=x;
    y_max=y_min=y;
    z_max=z_min=z;

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
                mask1D[pos]=1;
                x_all.push_back(dx);
                y_all.push_back(dy);
                z_all.push_back(dz);

                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos+cid*page_size];
                }
                project=getProjection(color, dir, convolute_iter);
                if(project<bg_thr)
                    continue;

                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=neuronPickerMain::pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if(mask1D[pos]>0) continue;
                    mask1D[pos]=1;
                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);

                    for(int cid=0; cid<sz_image[3]; cid++){
                        color[cid]=data1Dc_float[pos+cid*page_size];
                    }
                    project=getProjection(color, dir, convolute_iter);
                    if(project<bg_thr) continue;

                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    return seeds.size();
}

V3DLONG neuronPickerMain2::autoSeeds(vector<V3DLONG>& seeds, int cubSize, int conviter, int fgthr, int bgthr, int sizethr)
{
    seeds.clear();
    multimap<V3DLONG, V3DLONG, std::greater<V3DLONG> > seedMap; //first:

    float v=0;
    unsigned char * mask1D_v=neuronPickerMain::memory_allocate_uchar1D(page_size);
    for(int i=0; i<page_size; i++){
        v=data1Dc_float[i];
        for(int cid=1; cid<sz_image[3]; cid++){
            v=MAX(v,data1Dc_float[i+page_size*cid]);
        }
        mask1D_v[i]=(unsigned char)v;
    }
    unsigned char max_val, min_val;
    V3DLONG max_ind, min_ind;
    vector<V3DLONG> x_all, y_all, z_all;
    V3DLONG rsize;
    findMaxMinVal<unsigned char>(mask1D_v, page_size, max_ind, max_val, min_ind, min_val);
    while((int)max_val>fgthr){
        rsize=extractMore(x_all, y_all, z_all, max_ind, conviter, cubSize, bgthr);
        for(int i=0; i<x_all.size(); i++){
            mask1D_v[x_all[i]+y_all[i]*sz_image[0]+z_all[i]*sz_image[0]*sz_image[1]]=0;
        }
        if(rsize>sizethr)
            seedMap.insert(std::pair<V3DLONG, V3DLONG>(rsize, max_ind) );

//        qDebug()<<"NeuronPicker: "<<max_val<<":"<<max_ind<<":"<<x_all.size();

        findMaxMinVal<unsigned char>(mask1D_v, page_size, max_ind, max_val, min_ind, min_val);
    }

    for(multimap<V3DLONG, V3DLONG, std::greater<V3DLONG> >::iterator iter=seedMap.begin();
        iter!=seedMap.end(); iter++){
        seeds.push_back(iter->second);
    }
    return seeds.size();
}

V3DLONG neuronPickerMain2::autoAll(QString fname_outbase, V3DPluginCallback2 * callback , int cubSize, int conviter, int fgthr, int bgthr, int sizethr, int margin_size, float sparsthr, float touchthr)
{
    float v=0;
    float * mask1D_v=neuronPickerMain::memory_allocate_float1D(page_size);
    float * mask1D_eng=neuronPickerMain::memory_allocate_float1D(page_size);
    unsigned char * data1D_out=neuronPickerMain::memory_allocate_uchar1D(page_size);
    for(int i=0; i<page_size; i++){
        v=data1Dc_float[i];
        for(int cid=1; cid<sz_image[3]; cid++){
            v=MAX(v,data1Dc_float[i+page_size*cid]);
        }
        mask1D_eng[i]=v;
    }
    V3DLONG x, y, z;
    int seedwin=2;
    for(x=0; x<sz_image[0]; x++){
        for(y=0; y<sz_image[1]; y++){
            for(z=0; z<sz_image[2]; z++){
                V3DLONG pos=neuronPickerMain::xyz2pos(x,y,z,sz_image[0],sz_image[0]*sz_image[1]);
                v=0;
                int accum=0;
                for(V3DLONG dx=MAX(x-seedwin,0); dx<MIN(x+seedwin+1,sz_image[0]); dx++){
                    for(V3DLONG dy=MAX(y-seedwin,0); dy<MIN(y+seedwin+1,sz_image[1]); dy++){
                        for(V3DLONG dz=MAX(z-seedwin,0); dz<MIN(z+seedwin+1,sz_image[2]); dz++){
                            v+=mask1D_eng[neuronPickerMain::xyz2pos(dx,dy,dz,sz_image[0],sz_image[0]*sz_image[1])];
                            accum++;
                        }
                    }
                }
                mask1D_v[pos]=MIN(v,mask1D_eng[pos]);
            }
        }
    }
    float max_val, min_val;
    V3DLONG max_ind, min_ind;
    vector<V3DLONG> x_all, y_all, z_all;
    V3DLONG rsize;
    int delta=cubSize/2;
    vector<float> color(sz_image[3]);
    float project;
    int neuronNum = 0;
    V3DLONG sz_out[4];
    sz_out[0]=sz_image[0];
    sz_out[1]=sz_image[1];
    sz_out[2]=sz_image[2];
    sz_out[3]=1;
    bool eligible;
    findMaxMinVal<float>(mask1D_v, page_size, max_ind, max_val, min_ind, min_val);
    while((int)max_val>fgthr){
//        rsize=extract(x_all, y_all, z_all, max_ind, conviter, cubSize, bgthr);
        rsize=extract_eng(x_all, y_all, z_all, mask1D_eng, max_ind, conviter, cubSize, bgthr);
        if(rsize>sizethr)
            eligible=checkEligibility(x_all, y_all, z_all, sparsthr, touchthr, cubSize);
        else
            eligible=false;
        vector<V3DLONG> coord=neuronPickerMain::pos2xyz(max_ind,sz_image[0],sz_image[0]*sz_image[1]);
        for(V3DLONG dx=MAX(coord[0]-delta,0); dx<=MIN(sz_image[0]-1,coord[0]+delta); dx++){
            for(V3DLONG dy=MAX(coord[1]-delta,0); dy<=MIN(sz_image[1]-1,coord[1]+delta); dy++){
                for(V3DLONG dz=MAX(coord[2]-delta,0); dz<=MIN(sz_image[2]-1,coord[2]+delta); dz++){
                    mask1D_v[neuronPickerMain::xyz2pos(dx,dy,dz,sz_image[0],sz_image[0]*sz_image[1])]=0;
                }
            }
        }
//        mask1D_v[max_ind]=0;
        if(eligible){ //save if it is an eligible neuron
            //mask seed region
            for(int i=0; i<x_all.size(); i++){
                x=x_all[i];
                y=y_all[i];
                z=z_all[i];
                vector<float> dir = getProjectionDirection(max_ind, cubSize, bgthr, conviter);
                V3DLONG i=x+y*sz_image[0]+z*sz_image[0]*sz_image[1];
                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[i+cid*page_size];
                }
                project=getProjection(color, dir, conviter);
                mask1D_v[i]-=project;
            }

            //set output
            memset(mask1D,0,page_size*sizeof(unsigned char));
            memset(data1D_out,0,page_size*sizeof(unsigned char));
            vector<float> dir = getProjectionDirection(max_ind, cubSize, bgthr, conviter);
            //find the region
            for(int i=0; i<x_all.size(); i++){
                x=x_all[i];
                y=y_all[i];
                z=z_all[i];
                for(V3DLONG dx=MAX(x-margin_size,0); dx<=MIN(sz_image[0]-1,x+margin_size); dx++){
                    for(V3DLONG dy=MAX(y-margin_size,0); dy<=MIN(sz_image[1]-1,y+margin_size); dy++){
                        for(V3DLONG dz=MAX(z-margin_size,0); dz<=MIN(sz_image[2]-1,z+margin_size); dz++){
                            mask1D[dx+dy*sz_image[0]+dz*sz_image[0]*sz_image[1]]=1;
                        }
                    }
                }
            }
            //calculate projection
            for(int i=0; i<page_size; i++){
                if(mask1D[i]<1)
                    continue;
                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[i+cid*page_size];
                }
                project=getProjection(color, dir, conviter);
                mask1D_eng[i]-=project;

                data1D_out[i]=(unsigned char)MIN(project*innerScale,255);
            }
            //save file
            qDebug()<<"NeuronPicker: outputing neuron #"<<neuronNum<<", seed intensity: "<<max_val<<", voxels:"<<x_all.size();
            QString fname_output=fname_outbase + "_" + QString::number(neuronNum) + ".v3dpbd";
            if(!simple_saveimage_wrapper(*callback, qPrintable(fname_output),data1D_out,sz_out,1)){
                v3d_msg("Error: failed to save file to " + fname_output);
                continue;
            }else{
                qDebug()<<"NeuronPicker: "<<fname_output;
            }
            fname_output=fname_outbase + "_" + QString::number(neuronNum) + ".marker";
            neuronPickerMain::saveSingleMarker(max_ind,fname_output,sz_out);

            //record
            neuronNum++;
        }

        findMaxMinVal<float>(mask1D_v, page_size, max_ind, max_val, min_ind, min_val);
    }

    return neuronNum;
}

bool neuronPickerMain2::checkEligibility(vector<V3DLONG> x_all, vector<V3DLONG> y_all, vector<V3DLONG> z_all, float thr_sparse, float thr_touching, int cubeSize)
{
    memset(mask1D, 0, page_size*sizeof(unsigned char));
    V3DLONG nbox[6];
    nbox[0]=sz_image[0];
    nbox[1]=sz_image[1];
    nbox[2]=sz_image[2];
    nbox[3]=0;
    nbox[4]=0;
    nbox[5]=0;

    for(int i=0; i<x_all.size(); i++){
        mask1D[x_all[i]+y_all[i]*sz_image[0]+z_all[i]*sz_image[0]*sz_image[1]]=1;
        nbox[0]=MIN(nbox[0],x_all[i]);
        nbox[1]=MIN(nbox[1],y_all[i]);
        nbox[2]=MIN(nbox[2],z_all[i]);
        nbox[3]=MAX(nbox[3],x_all[i]);
        nbox[4]=MAX(nbox[4],y_all[i]);
        nbox[5]=MAX(nbox[5],z_all[i]);
    }

    //check boundary eligibility
    qDebug()<<"=======NeuronPicker: Check edge touching: ";
    if((nbox[0]<=1) && (nbox[3]<sz_image[0]*thr_touching)) return false;
    if((nbox[1]<=1) && (nbox[4]<sz_image[1]*thr_touching)) return false;
    if((nbox[2]<=1) && (nbox[5]<sz_image[2]*thr_touching)) return false;
    if((nbox[3]>sz_image[0]-2) && (sz_image[0]-nbox[0]<sz_image[0]*thr_touching)) return false;
    if((nbox[4]>sz_image[1]-2) && (sz_image[1]-nbox[1]<sz_image[1]*thr_touching)) return false;
    if((nbox[5]>sz_image[2]-2) && (sz_image[2]-nbox[2]<sz_image[2]*thr_touching)) return false;

    //calculate sparsness
    V3DLONG x,y,z;
    int delta=cubeSize/2;
    double sparsity=0;
    double neighborSize=(delta*2+1)*(delta*2+1)*(delta*2+1);
    double fgcount=0;
    for(int i=0; i<x_all.size(); i++){
        x=x_all[i];
        y=y_all[i];
        z=z_all[i];
        fgcount=0;
        //mask all regions founded
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_image[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_image[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_image[2]-1,z+delta); dz++){
                    if(mask1D[dx+dy*sz_image[0]+dz*sz_image[0]*sz_image[1]]==1)
                        fgcount++;
                }
            }
        }
        sparsity+=fgcount/neighborSize;
    }
    sparsity/=(double)x_all.size();
    qDebug()<<"=======NeuronPicker: Sparsity: "<<sparsity;
    if(sparsity>thr_sparse)
        return false;
    return true;
}

float neuronPickerMain2::getProjection(vector<float> vec, vector<float> dir, int convolute_iter)
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

vector<float> neuronPickerMain2::getProjectionDirection(V3DLONG seed_ind, int neighbor_size, int bg_thr, int convolute_iter)
{
    vector<V3DLONG> seed_coord = neuronPickerMain::pos2xyz(seed_ind, sz_image[0], sz_image[1]*sz_image[0]);
    int delta=neighbor_size/2;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    V3DLONG pos;

    int v_count=0;
    vector<float> dir(sz_image[3],0);
    vector<float> color(sz_image[3]);
    vector<float> color0(sz_image[3]);
    float norm=0;
    for(int cid=0; cid<sz_image[3]; cid++){
        color0[cid]=data1Dc_float[seed_ind+cid*page_size];
        norm+=color0[cid]*color0[cid];
    }
    norm=sqrt(norm);
    for(int cid=0; cid<sz_image[3]; cid++){
        color0[cid]/=norm;
    }
    for(V3DLONG dx=MAX(seed_coord[0]-delta,0); dx<=MIN(sz_image[0]-1,seed_coord[0]+delta); dx++){
        for(V3DLONG dy=MAX(seed_coord[1]-delta,0); dy<=MIN(sz_image[1]-1,seed_coord[1]+delta); dy++){
            for(V3DLONG dz=MAX(seed_coord[2]-delta,0); dz<=MIN(sz_image[2]-1,seed_coord[2]+delta); dz++){
                pos=neuronPickerMain::xyz2pos(dx,dy,dz,y_offset,z_offset);

                for(int cid=0; cid<sz_image[3]; cid++){
                    color[cid]=data1Dc_float[pos+cid*page_size];
                }

                float v=getProjection(color, color0, convolute_iter);

                if(v<bg_thr)    continue;

                for(V3DLONG cid=0; cid<sz_image[3]; cid++)
                    dir[cid]+=data1Dc_float[pos+page_size*cid]*v/255;
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
    norm=0;
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


unsigned char * neuronPickerMain::memory_allocate_uchar1D(const V3DLONG i_size)
{
    unsigned char *ptr_result;
    ptr_result=(unsigned char *) calloc(i_size, sizeof(unsigned char));
    return(ptr_result);
}
void neuronPickerMain::memory_free_uchar1D(unsigned char *ptr_input)
{
    free(ptr_input);
}
int * neuronPickerMain::memory_allocate_int1D(const V3DLONG i_size)
{
    int *ptr_result;
    ptr_result=(int *) calloc(i_size, sizeof(int));
    return(ptr_result);
}
void neuronPickerMain::memory_free_int1D(int *ptr_input)
{
    free(ptr_input);
}
float * neuronPickerMain::memory_allocate_float1D(const V3DLONG i_size)
{
    float *ptr_result;
    ptr_result=(float *) calloc(i_size, sizeof(float));
    return(ptr_result);
}
void neuronPickerMain::memory_free_float1D(float *ptr_input)
{
    free(ptr_input);
}
vector<V3DLONG> neuronPickerMain::landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
{
    vector<V3DLONG> poss_result;
    V3DLONG count_landmark=LandmarkList_input.count();
    for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
    {
        poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
    }
    return poss_result;
}
V3DLONG neuronPickerMain::landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z)
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
vector<V3DLONG> neuronPickerMain::pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    vector<V3DLONG> pos3_result (3, -1);
    pos3_result[2]=floor(_pos_input/(double)_offset_Z);
    pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
    pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
    return pos3_result;
}
V3DLONG neuronPickerMain::xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    return _z*_offset_Z+_y*_offset_Y+_x;
}

void neuronPickerMain::initChannels_rgb(unsigned char *image1Dc, int *image1D_h,unsigned char *image1D_v, unsigned char *image1D_s, V3DLONG sz_img[4], const int bg_thr)
{
    memset(image1D_h, -1, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(int));
    memset(image1D_s, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
    V3DLONG page=sz_img[0]*sz_img[1]*sz_img[2];

    //rgb 2 hsv, obtain foreground and histogram
    for(V3DLONG idx=0; idx<page; idx++){
        unsigned char V=MAX(image1Dc[idx],image1Dc[idx+page]);
        V=MAX(V,image1Dc[idx+page*2]);
        image1D_v[idx]=V;
        //filter by value
        if(V<(unsigned char)bg_thr){
            continue;
        }
        //get hue
        image1D_h[idx] = rgb2hue(image1Dc[idx], image1Dc[idx+page], image1Dc[idx+page*2]);
        //rgb2hsv(image1Dc[idx], image1Dc[idx+page], image1Dc[idx+page*2], image1D_h[idx], image1D_s[idx], image1D_v[idx]);
    }
}

void neuronPickerMain::saveSingleMarker(V3DLONG pos_landmark, QString fname, V3DLONG sz_img[4])
{
    vector<V3DLONG> coord;
    coord=pos2xyz(pos_landmark, sz_img[0], sz_img[0]*(sz_img[1]));
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    myfile<<coord[0]<<", "<<coord[1]<<", "<<coord[2]<<",0, 1, , , 255, 0"<<endl;
    file.close();
}

int neuronPickerMain::huedis(int a, int b)
{
    int dis=MAX(a,b)-MIN(a,b);
    if(dis>180){
        dis=360-180;
    }
    return dis;
}

unsigned char neuronPickerMain::saturationdis(unsigned char a, unsigned char b)
{
    return MAX(a,b)-MIN(a,b);
}

void neuronPickerMain::findMaxVal(unsigned char *image1D_v, V3DLONG len, V3DLONG & maxIdx, unsigned char & maxVal)
{
    maxVal=0;
    maxIdx=0;
    for(V3DLONG i=0; i<len; i++){
        if(image1D_v[i]>maxVal){
            maxIdx=i;
            maxVal=image1D_v[i];
        }
    }
}

void neuronPickerMain::autoSeeds(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, vector<V3DLONG>& seeds, int cubSize, int colorSpan, V3DLONG sz_img[4], int fgthr, int sizethr)
{
    seeds.clear();
    multimap<V3DLONG, V3DLONG, std::greater<V3DLONG> > seedMap; //first:

    //mask will be used to track the progress (volexes > 0 will be processed)
    unsigned char* image1D_mask=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]);
    memset(image1D_mask, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
//    int* image1D_hbk=memory_allocate_int1D(sz_img[0]*sz_img[1]*sz_img[2]);

    for(V3DLONG i=0; i<sz_img[0]*sz_img[1]*sz_img[2]; i++){
        if(image1D_h[i]>=0){
            image1D_mask[i]=image1D_v[i];
        }
//        image1D_hbk[i]=image1D_h[i];
    }

    //cur will be used to obtain current growed region
    unsigned char* image1D_cur=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]);

    V3DLONG maxIdx;
    unsigned char maxVal;
    findMaxVal(image1D_mask, sz_img[0]*sz_img[1]*sz_img[2], maxIdx, maxVal);
    while(maxVal>fgthr){
        //region grow
        memset(image1D_cur, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
        V3DLONG curSize = extract(image1D_h, image1D_mask, image1D_s, image1D_cur, maxIdx, cubSize, colorSpan, sz_img);
        //get the size of the region
//        image1D_hbk[maxIdx]=-1;
        image1D_mask[maxIdx]=0;
        for(V3DLONG i=0; i<sz_img[0]*sz_img[1]*sz_img[2]; i++){
            if(image1D_cur[i]>0){
                image1D_mask[i]=0;
//                image1D_hbk[i]=-1;
            }
        }
        if(curSize>sizethr)
            seedMap.insert(std::pair<V3DLONG, V3DLONG>(curSize, maxIdx) );

        findMaxVal(image1D_mask, sz_img[0]*sz_img[1]*sz_img[2], maxIdx, maxVal);
        qDebug()<<"======NeuronPicker: max val:"<<maxVal<<"\tpos:"<<maxIdx;
    }

    for(multimap<V3DLONG, V3DLONG, std::greater<V3DLONG> >::iterator iter=seedMap.begin();
        iter!=seedMap.end(); iter++){
        seeds.push_back(iter->second);
    }

    qDebug()<<"NeuronPicker: identified "<<seeds.size()<<" regions";

    memory_free_uchar1D(image1D_mask);
    memory_free_uchar1D(image1D_cur);
//    memory_free_int1D(image1D_hbk);
}

V3DLONG neuronPickerMain::extract(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, unsigned char *image1D_out, V3DLONG _pos_input, int cubSize, int colorSpan, V3DLONG sz_img[4])
{
    vector<V3DLONG> seeds;
    V3DLONG delta=cubSize/2;
    V3DLONG h_mean=0, v_count=0, s_mean=0;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    vector<V3DLONG> coord;
    coord=pos2xyz(_pos_input, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];
    qDebug()<<"==========NeuronPicker: seed:"<<x<<":"<<y<<":"<<z<<":"<<delta;
    //find the average hue in seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos];
                if(image1D_h[pos]<0)    continue;
                h_mean+=image1D_h[pos];
                s_mean+=(V3DLONG)image1D_s[pos];
                v_count++;
            }
        }
    }
    if(v_count<=0){
        qDebug()<<"==========NeuronPicker: an empty region!";
        return 0;
    }
    h_mean/=v_count;
    s_mean/=v_count;

    unsigned char* image1D_mask=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]);
    memset(image1D_mask, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos]<<":"<<v_mean;
                if(image1D_h[pos]<0)    continue;
                if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;
                image1D_out[pos]=image1D_v[pos];
                image1D_mask[pos]=1;
                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    image1D_out[pos]=image1D_v[pos];
                    if(image1D_h[pos]<0)    continue;
                    if(image1D_mask[pos]>0) continue;
                    if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                    if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;
                    image1D_mask[pos]=1;
                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    memory_free_uchar1D(image1D_mask);
    return seeds.size();
}

//input is color matrix
V3DLONG neuronPickerMain::extract_color(int *image1D_h, unsigned char *image1Dc_in, unsigned char *image1D_s, unsigned char *image1Dc_out, V3DLONG _pos_input, int cubSize, int colorSpan, V3DLONG sz_img[4])
{
    vector<V3DLONG> seeds;
    V3DLONG delta=cubSize/2;
    V3DLONG h_mean=0, v_count=0, s_mean=0;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    V3DLONG page=sz_img[0]*sz_img[1]*sz_img[2];
    vector<V3DLONG> coord;
    coord=pos2xyz(_pos_input, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];
    qDebug()<<"==========NeuronPicker: seed:"<<x<<":"<<y<<":"<<z<<":"<<delta;
    //find the average hue in seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos];
                if(image1D_h[pos]<0)    continue;
                h_mean+=image1D_h[pos];
                s_mean+=(V3DLONG)image1D_s[pos];
                v_count++;
            }
        }
    }
    if(v_count<=0){
        qDebug()<<"==========NeuronPicker: an empty region!";
        return 0;
    }
    h_mean/=v_count;
    s_mean/=v_count;

    unsigned char* image1D_mask=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]);
    memset(image1D_mask, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos]<<":"<<v_mean;
                if(image1D_h[pos]<0)    continue;
                if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;
                image1Dc_out[pos]=image1Dc_in[pos];
                image1Dc_out[pos+page]=image1Dc_in[pos+page];
                image1Dc_out[pos+page*2]=image1Dc_in[pos+page*2];
                image1D_mask[pos]=1;
                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";
    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    image1Dc_out[pos]=image1Dc_in[pos];
                    image1Dc_out[pos+page]=image1Dc_in[pos+page];
                    image1Dc_out[pos+page*2]=image1Dc_in[pos+page*2];
                    if(image1D_h[pos]<0)    continue;
                    if(image1D_mask[pos]>0) continue;
                    if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                    if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;
                    image1D_mask[pos]=1;
                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    memory_free_uchar1D(image1D_mask);
    return seeds.size();
}

//this function will extract less in comparison with extract();
V3DLONG neuronPickerMain::extract_mono(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, unsigned char *image1D_out, V3DLONG _pos_input, int cubSize, int colorSpan, V3DLONG sz_img[4])
{
    vector<V3DLONG> seeds;
    V3DLONG delta=cubSize/2;
    V3DLONG h_mean=0, v_count=0, s_mean=0;
    V3DLONG x,y,z,pos;
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    vector<V3DLONG> coord;
    coord=pos2xyz(_pos_input, y_offset, z_offset);
    x=coord[0];
    y=coord[1];
    z=coord[2];
    qDebug()<<"==========NeuronPicker: seed:"<<x<<":"<<y<<":"<<z<<":"<<delta;
    //find the average hue in seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos];
                if(image1D_h[pos]<0)    continue;
                h_mean+=image1D_h[pos];
                s_mean+=(V3DLONG)image1D_s[pos];
                v_count++;
            }
        }
    }
    if(v_count<=0){
        qDebug()<<"==========NeuronPicker: an empty region!";
        return 0;
    }
    h_mean/=v_count;
    s_mean/=v_count;

    unsigned char* image1D_mask=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]);
    memset(image1D_mask, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));

    //populate the init seed regions
    for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
        for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
            for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                qDebug()<<"==========NeuronPicker: "<<dx<<":"<<dy<<":"<<dz<<":"<<image1D_h[pos]<<":"<<v_mean;
                if(image1D_h[pos]<0)    continue;
                if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;
                image1D_out[pos]=image1D_v[pos];
                image1D_mask[pos]=1;
                seeds.push_back(pos);
            }
        }
    }

    qDebug()<<"==========NeuronPicker: found "<<seeds.size()<<" initial seeds";

    //seed grow
    V3DLONG sid=0;
    while(sid<seeds.size()){
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];
        y=coord[1];
        z=coord[2];
        //find the average hue in seed regions
        for(V3DLONG dx=MAX(x-delta,0); dx<=MIN(sz_img[0]-1,x+delta); dx++){
            for(V3DLONG dy=MAX(y-delta,0); dy<=MIN(sz_img[1]-1,y+delta); dy++){
                for(V3DLONG dz=MAX(z-delta,0); dz<=MIN(sz_img[2]-1,z+delta); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if(image1D_h[pos]<0)    continue;
                    if(image1D_mask[pos]>0) continue;
                    if(huedis(image1D_h[pos],h_mean)>colorSpan) continue;
//                    if(huedis(image1D_h[pos],h_mean)+saturationdis(image1D_s[pos],s_mean)>colorSpan) continue;

                    image1D_out[pos]=image1D_v[pos];
                    image1D_mask[pos]=1;
                    seeds.push_back(pos);
                }
            }
        }
        sid++;
    }

    qDebug()<<"=========NeuronPicker: finally found "<<seeds.size()<<" voxels";

    memory_free_uchar1D(image1D_mask);
    return seeds.size();
}

int neuronPickerMain::rgb2hue(const unsigned char R, const unsigned char G, const unsigned char B)
{
    float r = (float)R/255.0;
    float g = (float)G/255.0;
    float b = (float)B/255.0;
    float maxColor=-1, minColor=1e6;
    int maxChannel=-1;
    int hue=-1;
    if(maxColor<r){
        maxColor=r;
        maxChannel=0;
    }
    if(maxColor<g){
        maxColor=g;
        maxChannel=1;
    }
    if(maxColor<b){
        maxColor=b;
        maxChannel=2;
    }
    if(minColor>r){
        minColor=r;
    }
    if(minColor>g){
        minColor=g;
    }
    if(minColor>b){
        minColor=b;
    }

    if(maxChannel==0){
        float tmp=(g-b)/(maxColor-minColor);
        if(tmp<0)
            tmp=tmp+6;
        hue=60*tmp;
    }
    if(maxChannel==1){
        hue=60.0*(((b-r)/(maxColor-minColor))+2.0);
    }
    if(maxChannel==2){
        hue=60.0*(((r-g)/(maxColor-minColor))+4.0);
    }
    if(maxColor-minColor<1e-5)
        hue=-2;
    return hue;
}

void neuronPickerMain::rgb2hsv(const unsigned char R, const unsigned char G, const unsigned char B, int & h, unsigned char & s, unsigned char & v)
{
    float r = (float)R/255.0;
    float g = (float)G/255.0;
    float b = (float)B/255.0;
    float maxColor=-1, minColor=1e6;
    int maxChannel=-1;
    int hue=-1;
    if(maxColor<r){
        maxColor=r;
        maxChannel=0;
    }
    if(maxColor<g){
        maxColor=g;
        maxChannel=1;
    }
    if(maxColor<b){
        maxColor=b;
        maxChannel=2;
    }
    if(minColor>r){
        minColor=r;
    }
    if(minColor>g){
        minColor=g;
    }
    if(minColor>b){
        minColor=b;
    }

    if(maxChannel==0){
        float tmp=(g-b)/(maxColor-minColor);
        if(tmp<0)
            tmp=tmp+6;
        hue=60*tmp;
    }
    if(maxChannel==1){
        hue=60.0*(((b-r)/(maxColor-minColor))+2.0);
    }
    if(maxChannel==2){
        hue=60.0*(((r-g)/(maxColor-minColor))+4.0);
    }
    if(maxColor-minColor<1e-5)
        hue=-2;
    h=hue;
    v=MAX(MAX(R,G),B);
    if(maxColor<1e-5){
        s=0;
    }else{
        s=(maxColor-minColor)/maxColor*255;
    }
}
