#ifndef MAIN_H
#define MAIN_H
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include <QtGlobal>
using namespace std;
bool detect_type_distance(V3DPluginCallback2 &callback, QWidget *parent);
bool detect_type(V3DPluginCallback2 &callback, QWidget *parent);
struct segsinfo getallchildsofseg(int t);
struct segsinfo{
   vector<int> segpoint; //all the child points of current segment
   int aa; //the childs number of current segment
   int endnd; //the ending point of current segment
};

#endif // MAIN_H
