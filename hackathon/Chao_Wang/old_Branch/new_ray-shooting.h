#ifndef NEW_RAYSHOOTING_H
#define NEW_RAYSHOOTING_H
#include <v3d_interface.h>
#include <vector>
using namespace std;
float square(float x);
void ray_shooting(int m, int n,vector<vector<float> > ray_x,vector<vector<float> > ray_y);
bool rayinten_2D(int point_x,int point_y,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1 );
float interp_2d(float point_x,float point_y,unsigned char * PP,V3DLONG sz0,V3DLONG sz1);
v3d_uint8  get_2D_ValueUINT8(V3DLONG  x,  V3DLONG  y, unsigned char * T, V3DLONG  sz0, V3DLONG  sz1);
double *getOneGuassionArray(int size, double sigma);
void MyGaussianBlur(  float*  & srcimgae , float* & dst, int size, V3DLONG x1,V3DLONG y1) ;
void harrisResponse(float* & Gxx, float* & Gyy, float* & Gxy, float* & Hresult, float k,V3DLONG x1,V3DLONG y1)  ;
#endif // NEW_RAYSHOOTING_H
