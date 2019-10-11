#ifndef __RAY_SHOOTING_H__
#define __RAY_SHOOTING_H__
#include <v3d_interface.h>
#include <vector>
using namespace std;

float square(float x);

void XY_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip);
void YZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip);
void XZ_mip(int nx,int ny,int nz,unsigned char * datald,unsigned char * &image_mip);
void thres_segment(V3DLONG size_image, unsigned char * old_image, unsigned char * &binary_image,unsigned char thres);
int rayinten_2D(int point_x,int point_y,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y,V3DLONG nx,  V3DLONG ny, unsigned char *PP, int intensity_threshold,float angle_threshold);
v3d_uint8  get_2D_ValueUINT8(V3DLONG  y,  V3DLONG  x, unsigned char * T, V3DLONG  sz0, V3DLONG  sz1);
float project_interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1,int old_x,int old_y);
int get_max_angle(vector<int> ind,int n,int m,int intensity_threshold, float &max_ang);
#endif
