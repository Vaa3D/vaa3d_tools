#ifndef BRANCH_ANGLE_H
#define BRANCH_ANGLE_H
#include "ImageProcessing_plugin.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

struct Average
{
    int x;
    int y;
    int z;

    bool operator == (const Average &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};


struct PointCoordinate
{
    float x;
    float y;
    float z;

    bool operator == (const PointCoordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};


bool branch_angle1(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool calculate_subtree(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool branch_angle2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool branch_angle(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
QList<ImageMarker> swc_to_marker(QList<NeuronSWC> swc_list);
void choosePoint(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG &par11,V3DLONG &par21,QList<NeuronSWC> &choose_swclist1,QList<NeuronSWC> &choose_swclist2,int m);
void choosePoint2(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG &par11,QList<NeuronSWC> &choose_swclist1,int m);
V3DLONG chooseonechildline(V3DLONG &par_child1,V3DLONG &par_child2,V3DLONG &branchpoint,NeuronTree &nt,vector<vector<V3DLONG> > &childs);
void getFitLine(QList<NeuronSWC> &swclist1,QList<NeuronSWC> &swclist2,Vec6f &line_para1,Vec6f &line_para2);
void GetProjPoint(QList<NeuronSWC> &swclist,Vec6f &line_para,PointCoordinate &ProjPoint1,PointCoordinate &ProjPoint2);
PointCoordinate GetFootOfPerpendicular(PointCoordinate &pt,PointCoordinate &begin,PointCoordinate &end);
void getAllChild(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG par, QList<NeuronSWC> &swclist,int m);
void getAllParent(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG par, QList<NeuronSWC> &swclist,int n);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);


#endif // BRANCH_ANGLE_H
