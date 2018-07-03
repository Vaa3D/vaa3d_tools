#ifndef RAYSHOOTING_FUNC_H
#define RAYSHOOTING_FUNC_H
#include <v3d_interface.h>
#include <vector>
using namespace std;


float square(float x);
int rayinten_2D_multiscale(int point_x, int point_y, int m, int n, int T1, vector<vector<float> > ray_x, vector<vector<float> > ray_y, unsigned char * P, int sz0, int sz1 );
float interp_2d(float point_x,float point_y,unsigned char * PP,int sz0,int sz1);
v3d_uint8  get_2D_ValueUINT8(int x,int y,unsigned char * T,int sz0,int sz1);
#endif // RAYSHOOTING_FUNC_H
