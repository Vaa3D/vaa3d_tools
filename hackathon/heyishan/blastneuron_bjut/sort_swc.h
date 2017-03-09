#ifndef SORT_SWC_H
#define SORT_SWC_H
#include "basic_surf_objs.h"
QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT) ;
QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons);
void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined);
bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);

#endif // SORT_SWC_H
