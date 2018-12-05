#ifndef ANGLE_CALCULATE_H
#define ANGLE_CALCULATE_H
#include <QtGlobal>
#include "basic_surf_objs.h"
using namespace std;
struct angles angle_calculate(const NeuronTree & nt);
QVector<V3DLONG> getRemoteChild(int t);
struct angles{
  vector<double>a;
  vector<double>b;
  vector<int> c;
};


#endif


