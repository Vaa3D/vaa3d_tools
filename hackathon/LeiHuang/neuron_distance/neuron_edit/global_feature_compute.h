#ifndef COMPUTE_H
#define COMPUTE_H

#include <QtGlobal>
#include "basic_surf_objs.h" 

void computeFeature(const NeuronTree & nt, double * features);
QVector<V3DLONG> getRemoteChild(int t);
void computeLinear(const NeuronTree & nt);
void computeTree(const NeuronTree & nt);
double computeHausdorff(const NeuronTree & nt);
int fillArray(const NeuronTree & nt, short** r1, short** r2);
short **matrix(int n,int m);
void free_matrix(short **mat,int n,int m);
int mark(int m, short r[3], short ** c);

#endif
