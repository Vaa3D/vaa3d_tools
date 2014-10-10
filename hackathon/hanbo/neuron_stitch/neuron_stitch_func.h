/*
 * 2014.10.07 by: Hanbo Chen cojoc(at)hotmail.com
*/

#ifndef NEURON_STITCH_FUNC_H
#define NEURON_STITCH_FUNC_H

#include <basic_surf_objs.h>

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

void getNeuronTreeBound(const NeuronTree& nt, float * bound, int direction);

int highlight_edgepoint(const QList<NeuronTree> *ntList, float dis, int direction);

int highlight_adjpoint(const NeuronTree& nt1, const NeuronTree& nt2, float dis);

void backupNeuron(const NeuronTree & source, const NeuronTree & backup);

void copyProperty(const NeuronTree & source, const NeuronTree & target);

float quickMoveNeuron(QList<NeuronTree> * ntTreeList, int ant, int stackdir, int first_nt);
#endif // NEURON_STITCH_FUNC_H
