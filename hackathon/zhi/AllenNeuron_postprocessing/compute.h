#ifndef COMPUTE_H
#define COMPUTE_H

#include <QtGlobal>
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h" 

void computeFeature(const NeuronTree & nt, double * features);
int getParent(int n, QList<NeuronSWC> & list, QHash<int,int> & LUT);
QList<int> getChild(int n, QList <NeuronSWC> & list, QHash<int,int> & LUT);
void computeLinear(QList<NeuronSWC> & list, QHash<int,int> & LUT);
void computeTree(QList<NeuronSWC> &list, QHash<int,int> & LUT);
double computeHausdorff(QList <NeuronSWC> & list, QHash<int,int> & LUT);
int fillArray(QList<NeuronSWC> & list, QHash<int,int> & LUT, short** r1, short** r2);
short **matrix(int n,int m);
void free_matrix(short **mat,int n,int m);
int mark(int m, short r[3], short ** c);
double dist(const NeuronSWC & s1, const NeuronSWC & s2);
double angle(const NeuronSWC & ori, const NeuronSWC & s1, const NeuronSWC & s2);

#endif
