#ifndef GET_SAMPLE_AREA_H
#define GET_SAMPLE_AREA_H
#include<v3d_interface.h>
#include"basic_surf_objs.h"
#include "my_surf_objs.h"
#include "graph.h"
#include "node.h"

bool get_feature(V3DPluginCallback2 & callback,unsigned char * inimg1d,V3DLONG sz[4],QVector<QVector<int> > &hist_vec,vector<double> &entrople);
bool get_subimg(unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
bool get_sample_area(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output,QWidget *parent);
bool choose_cube(unsigned char* img1d,double thre,V3DLONG in_sz[4],unsigned char bresh, QMap<V3DLONG,Graph<Node*>*> &nodeMap,double m=0,double n1=0);
#endif // GET_SAMPLE_AREA_H
