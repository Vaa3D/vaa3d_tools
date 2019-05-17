#ifndef THRESHOLD_SELECT_H
#define THRESHOLD_SELECT_H

#include "ImageProcessing_plugin.h"
//#include "FM/fastmarching_dt.h"
//#include "FM/heap.h"

struct Coordinate
{
    int x;
    int y;
    int z;
    int signal;
    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};

bool binary_gsdt(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool binary_gsdt(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev);
bool gsdt(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback,unsigned char * &out1dData,input_PARA &PARA);
bool export_TXT(vector<Coordinate> &vec_coord,QString fileSaveName);
bool writeTXT_file(const QString & filename, const QList <ImageMarker> & listMarker);

#endif // THRESHOLD_SELECT_H


