/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/

#ifndef NEURON_STITCH_FUNC_H
#define NEURON_STITCH_FUNC_H

#include <basic_surf_objs.h>

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

void getNeuronTreeBound(const NeuronTree& nt, float * bound, int direction);


void getNeuronTreeBound(const NeuronTree& nt, double &minx, double &miny, double &minz,
                        double &maxx, double &maxy, double &maxz,
                        double &mmx, double &mmy, double &mmz);

int highlight_edgepoint(const QList<NeuronTree> *ntList, float dis, int direction);

int highlight_adjpoint(const NeuronTree& nt1, const NeuronTree& nt2, float dis);

void change_neuron_type(const NeuronTree& nt, int type);

void copyType(QList<int> source, const NeuronTree & target);

void copyType(const NeuronTree & source, QList<int> & target);

void backupNeuron(const NeuronTree & source, const NeuronTree & backup);

void copyProperty(const NeuronTree & source, const NeuronTree & target);

float quickMoveNeuron(QList<NeuronTree> * ntTreeList, int ant, int stackdir, int first_nt);

void multiplyAmat(double* front, double* back);
void multiplyAmat_centerRotate(double* rotate, double* tmat, double cx, double cy, double cz);

bool writeAmat(const char* fname, double* amat);
bool readAmat(const char* fname, double* amat);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
#endif // NEURON_STITCH_FUNC_H
