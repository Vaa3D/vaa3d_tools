#ifndef NEURONPICKER_FUNC_H
#define NEURONPICKER_FUNC_H

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
#include "neuronpicker_templates.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

using namespace std;

class neuronPickerMain
{
public:
    neuronPickerMain() {}
    ~neuronPickerMain() {}

    static unsigned char * memory_allocate_uchar1D(const V3DLONG i_size);
    static void memory_free_uchar1D(unsigned char *ptr_input);
    static int * memory_allocate_int1D(const V3DLONG i_size);
    static void memory_free_int1D(int *ptr_input);
    static float * memory_allocate_float1D(const V3DLONG i_size);
    static void memory_free_float1D(float *ptr_input);

    static vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);

    static void initChannels_rgb(unsigned char *image1Dc, int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, V3DLONG sz_img[4], const int bg_thr);
    static V3DLONG extract_mono(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, unsigned char *image1D_out, V3DLONG seed, int cubSize, int colorSpan, V3DLONG sz_img[4]);
    static V3DLONG extract(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, unsigned char *image1D_out, V3DLONG seed, int cubSize, int colorSpan, V3DLONG sz_img[4]);
    static V3DLONG extract_color(int *image1D_h, unsigned char *image1Dc_in, unsigned char *image1D_s, unsigned char *image1Dc_out, V3DLONG seed, int cubSize, int colorSpan, V3DLONG sz_img[4]);
    static void autoSeeds(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_s, vector<V3DLONG>& seeds, int cubSize, int colorSpan, V3DLONG sz_img[4], int fgthr, int sizethr);
    static void saveSingleMarker(V3DLONG pos_landmark, QString fname, V3DLONG sz_img[4]);

//private:
    static int huedis(int a, int b);
    static unsigned char saturationdis(unsigned char a, unsigned char b);
    static void findMaxVal(unsigned char *image1D_v, V3DLONG len, V3DLONG & maxIdx, unsigned char & maxVal);
    static V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z);
    static vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
    static V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
    static void rgb2hsv(const unsigned char R, const unsigned char G, const unsigned char B, int & h, unsigned char & s, unsigned char & v);
    static int rgb2hue(const unsigned char R, const unsigned char G, const unsigned char B);

};

class neuronPickerMain2
{
public:
    neuronPickerMain2();
    ~neuronPickerMain2();

    V3DLONG extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all, vector<V3DLONG>& z_all, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr);
    V3DLONG extractMore(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all, vector<V3DLONG>& z_all, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr);
    V3DLONG extract_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr);
    V3DLONG extractMargin_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr, int margin_size);
    V3DLONG extractSub_uchar(unsigned char*& image1D_out, V3DLONG sz_out[4], V3DLONG& pos_new, V3DLONG seed_ind, int convolute_iter, int neighbor_size, int bg_thr);
    V3DLONG autoSeeds(vector<V3DLONG>& seeds, int cubSize, int conviter, int fgthr, int bgthr, int sizethr);
    //find, extract and save
    V3DLONG autoAll(QString fname_output, V3DPluginCallback2 * callback, int cubSize, int conviter, int fgthr, int bgthr, int sizethr, int margin_size);

private:
    vector<float> getProjectionDirection(V3DLONG seed_ind, int neighbor_size, int bg_thr, int convolute_iter);
    float getProjection(vector<float> vec, vector<float> dir, int convolute_iter);

private:
    float * data1Dc_float;
    unsigned char * mask1D;
    V3DLONG sz_image[4];
    V3DLONG page_size;

public:
    template <class T>
    void pushNewData(T * data1Dc_in, V3DLONG sz_img[4])
    {
        if(data1Dc_float!=0){
            neuronPickerMain::memory_free_float1D(data1Dc_float);
        }
        if(mask1D!=0){
            neuronPickerMain::memory_free_uchar1D(mask1D);
        }

        sz_image[0]=sz_img[0];
        sz_image[1]=sz_img[1];
        sz_image[2]=sz_img[2];
        sz_image[3]=sz_img[3];
        page_size=sz_image[0]*sz_image[1]*sz_image[2];

        data1Dc_float = neuronPickerMain::memory_allocate_float1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(V3DLONG i=0; i<page_size*sz_img[3]; i++){
            data1Dc_float[i]=(float) (data1Dc_in[i]);
        }
        normalizeEachChannelTo255<float>(data1Dc_float, sz_img);
        mask1D = neuronPickerMain::memory_allocate_uchar1D(page_size);
    }
};



#endif // NEURONPICKER_FUNC_H
