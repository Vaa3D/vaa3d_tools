#ifndef SORT_ESWC_H
#define SORT_ESWC_H




#include <QtGlobal>
#include <math.h>
#include "basic_surf_objs.h"
#include <string.h>
#include <vector>
#include <iostream>
using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT);
QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons);
void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined);
bool SortESWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);

#endif // SORT_ESWC_H
