#ifndef NEW_RAYSHOOTING_H
#define NEW_RAYSHOOTING_H
#include <v3d_interface.h>
#include <vector>
using namespace std;
float square(float x);
void ray_shooting(int m, int n,vector<vector<float> > ray_x,vector<vector<float> > ray_y);
int rayinten_2D(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1 );
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
#endif // NEW_RAYSHOOTING_H
