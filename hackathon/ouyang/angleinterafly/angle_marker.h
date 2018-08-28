#ifndef ANGLE_MARKER_H
#define ANGLE_MARKER_H

#include <QtGlobal>
#include "basic_surf_objs.h"
using namespace std;
struct angles angle_calculate(const NeuronTree & nt);
vector<int> deletetipbranch(const NeuronTree & nt);
QVector<V3DLONG> getRemoteChild(int t);
struct angles{
  vector<double>a;
  vector<double>b;
  vector<int> c;
};



#endif // ANGLE_MARKER_H
