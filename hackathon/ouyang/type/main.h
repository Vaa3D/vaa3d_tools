#ifndef MAIN_H
#define MAIN_H
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include <QtGlobal>
using namespace std;
//bool detect_type_distance(V3DPluginCallback2 &callback, QWidget *parent);
bool detect_type(V3DPluginCallback2 &callback, QWidget *parent);
struct segsinfo getallchildsofseg(int t);
struct segsinfo{
   vector<int> segpoint; //all the child points of current segment
   int aa; //the childs number of current segment
   int endnd; //the ending point of current segment
};
vector<int> deletesus(QList<NeuronSWC> &neurons,vector<int> &points,int ofone);
QList<NeuronSWC> change_type_of_seg(QList<NeuronSWC> &neurons,vector<int> &childs,int resulttype);
QList<NeuronTree> find_wrong_type_ending(QList<NeuronSWC> &neurons,int root,int resulttype,vector<int> & sametyperoot,
                                         vector<int> & samecoornotroot,vector<int> & suspoint,vector<int> & allduplnodes,vector<int> & totalduplnodes);
void detect_type_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

#endif // MAIN_H
