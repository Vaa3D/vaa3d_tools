#ifndef SPINE_FUN_H
#define SPINE_FUN_H
#include <QtGui>
#include "v3d_interface.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <set>

using namespace std;

struct parameters{
    float bgthr; //background threshold
    int intensity_step; //steps of intensity based grow
    int width_thr; //limitation of the width of layer
    int layer_thr; //minimum number of layer required
    int max_dis; //distance from skel to cover
    int intensity_max_pixel; //maximum pixels allowed per layer
    int min_pixel; //minimum number of pixels in a group
    float spongeness; //minimum ng of each group
    float aspect_thr; //minumum aspect ratio
    int dst_max_pixel; //during reverse dst grow the max pixels allowed for each layer
};

struct VOI{
    V3DLONG pos; //position of the voxel
    V3DLONG x,y,z;
    V3DLONG skel_idx; //the index of the corresponding swc node
    int dst; //dst
    int dst_label; //dst label
    int dst_layer; //dst layer
    unsigned short intensity; //voxel intensity
    int intensity_label; //intensity label
    int intensity_layer; //intensity layer

    vector<VOI*> neighbors_26; //26 neighbor
    vector<VOI*> neighbors_6; //6 neighbor

    VOI(){pos=x=y=z=skel_idx=0.0; dst=dst_label=dst_layer=intensity=intensity_label=intensity_layer=-1;}
    VOI(V3DLONG _pos){pos=_pos; skel_idx=0.0; dst=dst_label=dst_layer=intensity=intensity_label=intensity_layer=-1;}
    VOI(V3DLONG _pos,V3DLONG _x,V3DLONG _y,V3DLONG _z){pos=_pos;x=_x;y=_y;z=_z; skel_idx=0.0; dst=dst_label=dst_layer=intensity=intensity_label=intensity_layer=-1;}
};

struct group_profile{int group_id; int number_of_layer; vector<float> layer_length;
                  vector<int> layer_voxel_num;vector<float> layer_spongeness;
                 vector<int> distance; float total_spongeness; QString comment;};

struct group_profile_simple {int group_id; int volume; int min_dis; int max_dis;float center_x;float center_y;
                            float center_z; int center_dis;};
typedef vector<VOI *> GOV;

bool sortfunc_dst(VOI * a, VOI * b);
bool sortfunc_intensity(VOI * a, VOI * b);
inline bool sortfunc_dst_ascend(VOI *a, VOI *b) {return (a->dst<b->dst);}

class spine_fun
{
public:
    spine_fun(const char * fname_img, const char* fname_skel, const char* fname_out, V3DPluginCallback * cb);
    spine_fun(V3DPluginCallback * cb, parameters set_para, int channel);
    int loadData(); //read images and swc files
    bool init(); //init the voxels of interest (within distance and intesnity is above threshold)
    bool init_old(); //init the voxels of interest (within distance and intesnity is above threshold)
    bool run_dstGroup();
    bool run_intensityGroup();
    void saveResult();
    bool run_dstGroup_individual();
    void dst_group_check();
    bool reverse_dst_grow();
    //LandmarkList construct_group_profile();
    void group_check();
    void conn_comp_nb6();
    void closing(GOV seeds, int id, GOV &new_seeds);

    inline vector<GOV> get_group_label() {return final_groups;}
    float check_spongeness(GOV group);
    int construct_layer_info(GOV tmp_group, group_profile *cur_group_info);
    float calc_spread_width(GOV cur_layer);
    void write_spine_center_profile();
    LandmarkList get_center_landmarks();

private:
    void ind2sub(V3DLONG &x, V3DLONG &y, V3DLONG &z, V3DLONG ind);
    long sub2ind(V3DLONG x, V3DLONG y, V3DLONG z);

public:
    parameters param;

private:
    V3DPluginCallback * callback;
    QString fname_img;
    QString fname_skel;
    QString fname_out;
    unsigned char * p_img1D, *tmp_out;
    unsigned short * tmp_label;
    unsigned char *** ppp_img3D; //ppp_img3d[z][y][x]
    V3DLONG sz_img[4],label_sz[4];
    int sel_channel;
    int type_img;
    NeuronTree nt;
    vector<VOI*> voxels;
    vector<GOV> dst_groups;
    vector<GOV> intensity_groups;
    vector<GOV> final_groups;
    vector<group_profile*> all_group_info;

public:
    bool pushImageData(unsigned char * data1Dc_in, V3DLONG new_sz_img[4])
    {
        qDebug()<<"in push imageData";
        if(ppp_img3D!=0){
            delete [] ppp_img3D; ppp_img3D=0;
        }
        sz_img[0]=new_sz_img[0];
        sz_img[1]=new_sz_img[1];
        sz_img[2]=new_sz_img[2];

        //we know which channel to select
        //need to check this channel whether there are voxels

        int max=0;
        V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];
        p_img1D = new unsigned char [page_size];
        for(V3DLONG i=0; i<page_size; i++){
            p_img1D[i] = data1Dc_in[i+sel_channel*page_size];
            if (p_img1D[i]>max)
                max=p_img1D[i];
        }
        if (max==0)
        {
            v3d_msg("Please check the provided channel. No voxels found");
            delete[] p_img1D;
            p_img1D=0;
            return false;
        }
        //arrange the image into 3D
        ppp_img3D=new unsigned char ** [sz_img[2]];
        for(V3DLONG z=0; z<sz_img[2]; z++){
            ppp_img3D[z]=new unsigned char * [sz_img[1]];
            for(V3DLONG y=0; y<sz_img[1]; y++){
                ppp_img3D[z][y]=p_img1D+y*sz_img[0]+z*sz_img[0]*sz_img[1];
            }
        }
        qDebug()<<"in pushnewdata:"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
        return true;
    }

    void pushSWCData(NeuronTree neuron)
    {
        nt.copy(neuron);
    }
};

#endif // SPINE_FUN_H
