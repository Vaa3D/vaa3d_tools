#ifndef SV_TRACE_FUNC_H
#define SV_TRACE_FUNC_H

#include <v3d_interface.h>
#include "basic_surf_objs.h"

#include<iostream>

#include<vector>
#include<map>
using namespace std;

struct superpoint{
    double x;
    double y;
    double z;
    int rx, ry, rz;
    double intensity;
    enum{UD=0,LR=1,FB=2,UD_1=3,UD_2=4,LR_1=5,LR_2=6,FB_1=7,FB_2=8,UK=9};
    int direction;
    superpoint():x(0),y(0),z(0),rx(0),ry(0),rz(0),direction(UK),intensity(0)
    {}
    superpoint(double x,double y,double z,int rx=0,int ry=0,int rz=0,int direction=UK,double intensity=0)
    {
        this->x=x;
        this->y=y;
        this->z=z;
        this->rx=rx;
        this->ry=ry;
        this->rz=rz;
        this->direction=direction;
        this->intensity=intensity;
    }

    inline V3DLONG getIndex(V3DLONG* sz)
    {
        return z*sz[0]*sz[1]+y*sz[0]+x;
    }

    bool getIndexs(vector<long> &indexs, V3DLONG* sz);

    double get_Intensity(unsigned char* pdata,V3DLONG* sz);

    bool get_Radius(unsigned char* pdata,V3DLONG* sz,double thres);

    bool get_Radius_2(unsigned char* pdata,V3DLONG* sz,double thres);

    bool get_Direction(unsigned pdata,V3DLONG* sz);

};

class sv_tracer{

public:
    bool init_superpoints(vector<superpoint> &superpoints,unsigned char* pdata,V3DLONG* sz,double thres);

    bool init_real_points(vector<superpoint> &old,vector<superpoint> &realpoints,unsigned char* pdata,V3DLONG* sz,double thres);

    bool writeSuperpoints(QString markerfile,vector<superpoint> &superpoints);


};













#endif // SV_TRACE_FUNC_H
