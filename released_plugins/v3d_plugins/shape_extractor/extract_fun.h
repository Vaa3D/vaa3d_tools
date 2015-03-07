#ifndef EXTRACT_FUN_H
#define EXTRACT_FUN_H

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
#include <math.h>
#include "shape_extr_template.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
//#define pi 3.14

using namespace std;

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size);
void memory_free_uchar1D(unsigned char *ptr_input);
float * memory_allocate_float1D(const V3DLONG i_size);
void memory_free_float1D(float *ptr_input);
vector<V3DLONG>landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);
V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z);
vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);

class extract_fun
{
public:
    extract_fun();
    ~extract_fun();

    vector<float> getProjectionDirection(V3DLONG seed_ind, int neighbor_size, int bg_thr);
    float getProjection(vector<float> vec, vector<float> dir, int convolute_iter);
    V3DLONG extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,unsigned char * label,V3DLONG seed_ind,
                     int convolute_iter,int bg_thr,int marker);
    vector<V3DLONG> get_mass_center(vector<V3DLONG> x_all, vector<V3DLONG> y_all, vector<V3DLONG> z_all);


private:

    V3DLONG page_size;
    float * data1Dc_float;

public:

    V3DLONG sz_image[4];
    unsigned char * mask1D;
    template <class T>
    void pushNewData(T * data1Dc_in, V3DLONG sz_img[4])
    {
        if(data1Dc_float!=0){
            memory_free_float1D(data1Dc_float);
        }
        if(mask1D!=0){
            memory_free_uchar1D(mask1D);
        }

        sz_image[0]=sz_img[0];
        sz_image[1]=sz_img[1];
        sz_image[2]=sz_img[2];
        sz_image[3]=sz_img[3];
        page_size=sz_image[0]*sz_image[1]*sz_image[2];

        data1Dc_float = memory_allocate_float1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(V3DLONG i=0; i<page_size*sz_img[3]; i++){
            data1Dc_float[i]=(float) (data1Dc_in[i]);
        }
        normalizeEachChannelTo255<float>(data1Dc_float, sz_img);
        mask1D = memory_allocate_uchar1D(page_size);
        memset(mask1D, 0, page_size*sizeof(unsigned char));

    }
};


#endif // EXTRACT_FUN_H
