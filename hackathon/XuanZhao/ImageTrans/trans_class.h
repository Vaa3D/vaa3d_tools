#ifndef  TRANS_CLASS_H
#define  TRANS_CLASS_H

#include <v3d_interface.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <basic_surf_objs.h>

#define G 255

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































#endif // unsigned charRANS_CLASS_H
