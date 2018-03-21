#ifndef __RAY_SHOOTING_H__
#define __RAY_SHOOTING_H__
#include <v3d_interface.h>
#include <vector>
using namespace std;

float square(float x);

void ray_shooting(int m, int n,vector<vector<float>> ray_x,vector<vector<float>> ray_y);

void rayinten_2D(int point_x,int point_y,int point_z,int m,int n,int threshold,vector<vector<float>> ray_x, vector<vector<float>> ray_y, Image4DSimple *p4DImage, int &count, float &max_ang);

void ray_shooting_3D(float ray_X[512][8], float ray_Y[512][8], float ray_Z[512][8]);

void rayinten_3D(int point[3], float rayintensity[512][8], float ray_X[512][8], float ray_Y[512][8], float ray_Z[512][8], Image4DSimple *p4DImage, int &count, float &max_ang);

float interp_2d(float point_x,float point_y,int point_z, Image4DSimple *p4DImage);

bool get_slice_flag(int i,int j,int k,int nembers_2d,int length_2d,double angle_2d,double count_2d_thre,int slice_number,int threshold, vector<vector<float>> ray_x,vector<vector<float>> ray_y, Image4DSimple *p4DImage, int &count_2d, float &max_ang_2d);

#endif