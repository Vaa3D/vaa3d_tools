#ifndef DETECT_FUN_H
#define DETECT_FUN_H

#include <QtGui>
#include <vector>
#include <math.h>
#include "spine_extr_template.h"
#include "basic_surf_objs.h"
#include "fastmarching_dt.h"
#include "v3d_basicdatatype.h"

#define pi 3.1416
#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

using namespace std;

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size);
void memory_free_uchar1D(unsigned char *ptr_input);
float * memory_allocate_float1D(const V3DLONG i_size);
void memory_free_float1D(float *ptr_input);
vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z,
                const V3DLONG _offset_Y, const V3DLONG _offset_Z);
void write_swc(vector<MyMarker> vp, QString filename);
void write_csv(vector<MyMarker> vp,QString filename);
V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z);
vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);

vector<vector<float> > get_dir_vec(NeuronSWC p, NeuronSWC p0);
vector<float> normalize_vec(vector<float> m);
void writer_swc(vector<vector<float> > new_dir_vec, QString filename, NeuronSWC p0);
void GetColorRGB(int* rgb, int idx);
vector<V3DLONG> get_mass_center(vector<V3DLONG> x_all, vector<V3DLONG> y_all,
                                             vector<V3DLONG> z_all);

class detect_fun
{

public:
    detect_fun();
    ~detect_fun();
    double markerRadius(V3DLONG ind);
    double markerRadius_nooverlap(V3DLONG ind);
    vector<float> ray_shoot_vector(V3DLONG ind);
    double calc_mean_shift_center(V3DLONG ind, int windowradius,float *data1Dc_float,
                                         V3DLONG sz_image[],int methodcode);
    double skel_node_radius(V3DLONG ind);
    vector<MyMarker> bubbles();
    vector<float> ray_shoot_feature(NeuronSWC p, NeuronSWC p0, LandmarkList &tmplist);
    vector<V3DLONG> distance_to_skel(vector<MyMarker> bubbles, NeuronTree &neuron, vector<int> &neuron_idx, LandmarkList &far_points);
    V3DLONG extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                                 V3DLONG seed_ind,float r_start,float stop_p);
    V3DLONG extract_nonoverlap(unsigned char *skel_mask,long *label,
                           vector<V3DLONG>& x_all,vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                                           V3DLONG seed_ind,float stop_p,int marker);
    V3DLONG extract_nonsphere(unsigned char *all);
    vector<V3DLONG> bubbles_no_gsdt(unsigned char *seperate);
    float calc_spread_width(vector<V3DLONG> array);
    vector<V3DLONG> spine_grow(float *bound_box, unsigned short *label,
         V3DLONG ind, int max_spine_width, int spine_id, int max_pixel, int min_pixel);

private:
    V3DLONG page_size;
    float * data1Dc_float;
    int bg_thr;

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
//        label = memory_allocate_uchar1D(page_size);
//        memset(label,-1, page_size*sizeof(unsigned char));

    }

};

#endif // DETECT_FUN_H
