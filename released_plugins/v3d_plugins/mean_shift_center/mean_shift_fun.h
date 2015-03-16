#ifndef MEAN_SHIFT_FUN_H
#define MEAN_SHIFT_FUN_H

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
#include <math.h>
#include "mean_shift_extr_template.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

using namespace std;

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size);
void memory_free_uchar1D(unsigned char *ptr_input);
float * memory_allocate_float1D(const V3DLONG i_size);
void memory_free_float1D(float *ptr_input);
vector<V3DLONG>landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);
V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z);
vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);

void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);

vector<float> calc_mean_shift_center(V3DLONG ind, int windowradius,float *data1Dc_float,V3DLONG sz_image[],int methodcode);

bool load_data(V3DPluginCallback2 *cb,unsigned char * &image1Dc_in,LandmarkList &LList,
               ImagePixelType &pixeltype,V3DLONG sz_img[4],v3dhandle &curwin);

class mean_shift_fun
{
public:
    mean_shift_fun();
    ~mean_shift_fun();
    vector<float> mean_shift_center(V3DLONG ind, int windowradius);
    vector<float> mean_shift_with_constraint(V3DLONG ind, int windowradius);
    vector<float> ray_shoot_center(V3DLONG ind, int bg_thr);
    vector<float> gradient_transform(float *&outimg1d,V3DLONG ind,
                        int bg_thr, int cnn_type,int z_thickness,int halfwindowsize, int search_radius);

private:

    V3DLONG page_size;
    float * data1Dc_float;

public:

    V3DLONG sz_image[4];
    //unsigned char * mask1D;
    template <class T>
    void pushNewData(T * data1Dc_in, V3DLONG sz_img[4])
    {
        if(data1Dc_float!=0){
            memory_free_float1D(data1Dc_float);
        }
        sz_image[0]=sz_img[0];
        sz_image[1]=sz_img[1];
        sz_image[2]=sz_img[2];
        sz_image[3]=sz_img[3];
        page_size=sz_image[0]*sz_image[1]*sz_image[2];

        data1Dc_float = memory_allocate_float1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(V3DLONG i=0; i<page_size*sz_img[3]; i++){
            data1Dc_float[i] = (float)(data1Dc_in[i]);
        }
        normalizeEachChannelTo255<float>(data1Dc_float, sz_img);

    }
};

#endif // MEAN_SHIFT_FUN_H
