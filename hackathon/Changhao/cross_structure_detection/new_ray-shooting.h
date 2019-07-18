#ifndef NEW_RAYSHOOTING_H
#define NEW_RAYSHOOTING_H
#include <v3d_interface.h>
#include <vector>
#include"../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

#define PI 3.1415926

using namespace std;
float square(float x);
void ray_shooting(int m, int n,vector<vector<float> > ray_x,vector<vector<float> > ray_y);
int rayshooting_modle_length(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1,int thres ,double max_radiu);
float project_interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1,int old_x,int old_y);
v3d_uint8  get_2D_ValueUINT8(V3DLONG  x,  V3DLONG  y, unsigned char * T, V3DLONG  sz0, V3DLONG  sz1);
double *getOneGuassionArray(int size, double sigma);
void MyGaussianBlur(  float*  & srcimgae , float* & dst, int size, V3DLONG x1,V3DLONG y1) ;
void harrisResponse(float* & Gxx, float* & Gyy, float* & Gxy, float*  & Hresult, float k,V3DLONG x1,V3DLONG y1,float &max_hresult) ;
void harrisResponse(float* & Gxx, float* & Gyy, float* & Gxy, float* & Hresult, float k,V3DLONG x1,V3DLONG y1)  ;
void mul(unsigned char * & G, float * &GG,V3DLONG x1,V3DLONG y1);
void mul_xy(unsigned char * & Gx, unsigned char * &Gy, float *& Gxy, V3DLONG x1,V3DLONG y1);
bool mip_z_slices(Image4DSimple * subject, Image4DSimple & outImage,
             V3DLONG startnum, V3DLONG increment, V3DLONG endnum);
bool parseFormatString(QString t, V3DLONG & startnum, V3DLONG & increment, V3DLONG & endnum, V3DLONG sz2);
void Z_mip(V3DLONG ix,V3DLONG iy,V3DLONG iz,unsigned char * datald,unsigned char * &image_mip);
void thres_segment(V3DLONG size_image, unsigned char * old_image, unsigned char * &binary_image,unsigned char thres);
void mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip,int layer);
void mip_layer_xy(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip,int layer);
void mip_layer_yz(V3DLONG nx,V3DLONG ny,V3DLONG nz,V3DLONG x_sliceloc,unsigned char * datald,unsigned char * &image_mip,int layer);
void mip_layer_xz(V3DLONG nx,V3DLONG ny,V3DLONG nz,V3DLONG y_sliceloc,unsigned char * datald,unsigned char * &image_mip,int layer);
void mip_yz(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip);
void mip_xz(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip);
int rayinten_2D(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y,
                vector<vector<float> > ray_x_left, vector<vector<float> > ray_y_left, vector<vector<float> > ray_x_right,
                vector<vector<float> > ray_y_right, unsigned char * P,V3DLONG sz0,V3DLONG sz1 );
void skeletonization(V3DLONG nx, V3DLONG ny, unsigned char * &image_binary );
int candidate_points(V3DLONG nx, unsigned char * image_binary,V3DLONG point_y,V3DLONG point_x);
vector<MyMarker> readMarker_file(string marker_file);
vector<V3DLONG> bubbleSort(vector<V3DLONG> array);
bool save_groundtruth_marker(string marker_file, vector<vector<int>> & out_marker, list<string> & infostring );
bool find_neighborhood_maximum(V3DLONG source_point_x,V3DLONG source_point_y, float * phi, int window_size, int &changed_x_location, int &changed_y_location,V3DLONG nx,V3DLONG ny);
template<class T> double markerRadius_hanchuan_XY(T* &inimg1d, V3DLONG nx, V3DLONG ny, V3DLONG x,V3DLONG y, double thresh);
bool find_neighborhood_maximum_radius(V3DLONG source_point_x,V3DLONG source_point_y, unsigned char * datald_2D, int window_size, V3DLONG &changed_x_location, V3DLONG &changed_y_location,double &max_radius,V3DLONG nx,V3DLONG ny ,int thres);
void rorate_method(int point_y,int point_x,int model_size,double modle_length,  vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG nx,V3DLONG ny,vector<float> & x_loc, vector<float> & y_loc);
void seek_2D_candidate_points(V3DLONG nx,V3DLONG ny, unsigned char *image_binary,vector<V3DLONG> &x_loc,vector<V3DLONG> &y_loc,int &count,int cnp_val);
void seek_2D_candidate_points(V3DLONG nx,V3DLONG ny, unsigned char *image_binary,vector<V3DLONG> &x_loc,vector<V3DLONG> &y_loc,int &count);
void find_valid_layer(unsigned char * data1d, V3DLONG nx, V3DLONG ny, V3DLONG nz, int thresh, vector<int> &valid_layer);

#endif // NEW_RAYSHOOTING_H
