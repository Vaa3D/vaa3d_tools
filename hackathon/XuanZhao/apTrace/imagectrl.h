#ifndef IMAGECTRL_H
#define IMAGECTRL_H

#include <vector>
#include "v3d_interface.h"
#include "mysurface.h"

using namespace std;


class ImageCtrl
{
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
        sz0 = sz[0];
        sz1 = sz[1];
        sz2 = sz[2];
        sz3 = sz[3];
        V3DLONG sz_num = sz[0]*sz[1]*sz[2];
        data1d = new unsigned char[sz_num];
        for(V3DLONG i=0;i<sz_num;++i)
        {
            data1d[i] = pdata[i];
        }
    }
    ~ImageCtrl()
    {
        if(data1d) delete[] data1d;
    }

    bool Cor_to_Index(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG &index);

    bool Index_to_Cor(V3DLONG &x,V3DLONG &y,V3DLONG &z,V3DLONG index);

    bool Data1d_to_3d(vector<vector<vector<unsigned char> > > &image);

    bool Data3d_to_1d(vector<vector<vector<unsigned char> > > &image);

    bool corrode(vector<vector<vector<unsigned char> > > &image, int mode);

    double getMeanIntensity();

    bool SaveImage(QString filename,V3DPluginCallback2 &callback);

    unsigned char* getdata();

    unsigned char*** get3ddata();

    bool getSegImage(vector<assemblePoint> &assemblePoints);

    bool histogram();

    double getMode();

    bool segment0();
};

#endif // IMAGECTRL_H
