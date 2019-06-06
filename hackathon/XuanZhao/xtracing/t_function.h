#ifndef T_FUNCTION_H
#define T_FUNCTION_H

#include "t_class.h"
#include <basic_surf_objs.h>
#include <vector>
using namespace std;

V3DLONG threeToOne(V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2);

V3DLONG nextPoint(T_Point p,vector<T_Point> &points,double angle0,double angle1,V3DLONG d,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2);

vector<V3DLONG> nextPoints(T_Point &p, vector<T_Point> &points, double angle0_min, double angle1_min, double angle0_max, double angle1_max, V3DLONG d_min, V3DLONG d_max, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2/*,int &mode*/);

bool construct_tree(T_Point root,vector<T_Point> &points,vector<T_Point> &outtree,double angle0_min,double angle1_min,double angle0_max,double angle1_max,V3DLONG d_min,V3DLONG d_max,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2);

























#endif // T_FUNCTION_H
