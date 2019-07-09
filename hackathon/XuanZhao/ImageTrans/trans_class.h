#ifndef  TRANS_CLASS_H
#define  TRANS_CLASS_H

#include <v3d_interface.h>
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <basic_surf_objs.h>

#define G 255
#define A1 sqrt(2.0)/2
#define A2 sqrt(3.0)/3

using namespace std;

void GaussElimination(vector<vector<double>> &A);

void DecodeMatrix(vector<vector<double>> &A, vector<double> &AN);

class ImageCtrl{
private:
    unsigned char* data1d;
    V3DLONG sz0,sz1,sz2,sz3;
public:
    ImageCtrl():sz0(0),sz1(0),sz2(0),sz3(0)
    {
        data1d=0;
    }
    ImageCtrl(unsigned char* pdata,V3DLONG* sz)
    {
        sz0=sz[0];
        sz1=sz[1];
        sz2=sz[2];
        sz3=sz[3];
        cout<<"in construct..."<<endl;
        //if(data1d){delete[] data1d;}
        cout<<"delete..."<<endl;
        V3DLONG sz_num=sz[0]*sz[1]*sz[2];
        data1d=new unsigned char[sz_num];
        for(V3DLONG i=0;i<sz_num;++i)
        {
            data1d[i]=pdata[i];
        }
        cout<<"end construct..."<<endl;
    }
    ~ImageCtrl()
    {
        if(data1d) delete[] data1d;
    }

    bool Cor_to_Index(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG &index);

    bool Index_to_Cor(V3DLONG &x,V3DLONG &y,V3DLONG &z,V3DLONG index);

    bool Data1d_to_3d(vector<vector<vector<unsigned char>>> &image);

    bool Data3d_to_1d(vector<vector<vector<unsigned char>>> &image);

    double B_cernel(double z);

    bool trans(vector<unsigned char> hdzs,vector<vector<double>> &A);

    bool Draw_z(vector<vector<vector<unsigned char> > > &image_new, int times);

    bool Draw_z_PureB(vector<vector<vector<unsigned char> > > &image_new, int times);

    bool Draw_z_F(vector<vector<vector<unsigned char> > > &image_new, int times);

    bool Draw_z_S(vector<vector<vector<unsigned char> > > &image_new, int times);

    bool Draw_S(vector<vector<vector<unsigned char> > > &image_new, int times,int mode);

    bool SaveImage(QString filename,V3DPluginCallback2 &callback);

    void display(vector<vector<vector<unsigned char>>> &image);
};


struct direction
{
    double x,y,z;
    direction():x(0),y(0),z(0){}
    direction(double _x,double _y,double _z)
    {
        x=_x;
        y=_y;
        z=_z;
    }
    direction operator =(direction &d)
    {
        x=d.x;
        y=d.y;
        z=d.z;
        return *this;
    }
};


struct superpoint{
    double x;
    double y;
    double z;
    int rx, ry, rz;
    double intensity;
    //enum{UD=0,LR=1,FB=2,UD_1=3,UD_2=4,LR_1=5,LR_2=6,FB_1=7,FB_2=8,UK=9};
    direction direc;
    superpoint():x(0),y(0),z(0),rx(0),ry(0),rz(0),direc(),intensity(0)
    {}
    superpoint(double x,double y,double z,int rx=0,int ry=0,int rz=0,direction direc=direction(),double intensity=0)
    {
        this->x=x;
        this->y=y;
        this->z=z;
        this->rx=rx;
        this->ry=ry;
        this->rz=rz;
        this->direc=direc;
        this->intensity=intensity;
    }

    inline V3DLONG getIndex(V3DLONG* sz)
    {
        return z*sz[0]*sz[1]+y*sz[0]+x;
    }

    bool operator <(const superpoint &other) const
    {
        if(x!=other.x)
            return (x<other.x);
        if(this->y!=other.y)
            return (y<other.y);

        return (z<other.z);

    }

    bool getIndexs(vector<V3DLONG> &indexs, V3DLONG* sz);

    double get_Intensity(unsigned char* pdata,V3DLONG* sz);

    double get_Intensity(vector<vector<vector<unsigned char>>> &image);

    bool get_Radius(unsigned char* pdata,V3DLONG* sz,double thres);

    bool get_Radius(vector<vector<vector<unsigned char>>> &image,V3DLONG* sz,double thres);

    bool get_Radius_2(unsigned char* pdata,V3DLONG* sz,double thres);

    bool get_Radius_2(vector<vector<vector<unsigned char>>> &image,V3DLONG* sz, double thres);

    bool get_Direction(unsigned pdata,V3DLONG* sz);

    bool get_Direction(vector<vector<vector<unsigned char>>> &image,V3DLONG* sz);

    bool get_nb_points_index(vector<superpoint> &v_points,vector<int> &n_points_index);

    double get_distance(superpoint &other);

};

class sv_tracer{

public:
    bool init_superpoints(vector<superpoint> &superpoints,unsigned char* pdata,V3DLONG* sz,double thres);

    bool init_superpoints(vector<superpoint> &superpoints, vector<vector<vector<unsigned char>>> &image, long long *sz, double thres);

    bool init_real_points(vector<superpoint> &old,vector<superpoint> &realpoints,unsigned char* pdata,V3DLONG* sz,double thres);

    bool init_real_points(vector<superpoint> &old, vector<superpoint> &realpoints, vector<vector<vector<unsigned char>>> &image, long long *sz, double thres);

    bool writeSuperpoints(QString markerfile,vector<superpoint> &superpoints);

    bool trace(vector<superpoint> &realpoints, vector<int> &plist, vector<vector<vector<unsigned char>>> &image, V3DLONG *sz);

    bool sv_trace(vector<superpoint> &realpoints,vector<vector<vector<unsigned char>>> &image,NeuronTree &nt,V3DLONG *sz);


};



























#endif // unsigned charRANS_CLASS_H
