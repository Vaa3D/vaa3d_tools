#ifndef IMAGEGRAYVALUE_H
#define IMAGEGRAYVALUE_H
#include "ImageProcessing_plugin.h"
#include "branch_angle.h"
#include "binary_gsdt.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_center_plugin.h"

struct ImageSignal
{
    int x;
    int y;
    int z;
    int signal;
    bool operator == (const ImageSignal &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};

struct Point1;
struct Point1
{
    double x,y,z,r;
    V3DLONG type;
    Point1* p;
    V3DLONG childNum;
};
typedef vector<Point1*> Segment;
typedef vector<Point1*> Tree;


bool ImageGrayValue(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool ImageGrayValue(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
bool export_TXT(vector<ImageSignal> &vec,QString fileSaveName);
void mean_shift_center(unsigned char* data1d,V3DLONG sz_img[4], LandmarkList &LList,QList<NeuronSWC> &nt_list,int methodcode,V3DLONG n,V3DLONG pn);
//LandmarkList mean_shift_center(unsigned char* data1d,V3DLONG sz_img[4], LandmarkList &LList,QList<NeuronSWC> &nt_list,int methodcode);
#endif // IMAGEGRAYVALUE_H
