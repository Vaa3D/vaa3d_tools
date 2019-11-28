#ifndef __GIT_NEURON_FUNC_H__
#define __GIT_NEURON_FUNC_H__

#include <QtGui>
#include <QDialog>
#include <v3d_interface.h>

void neuron_compare(NeuronTree* nt,NeuronTree* nt2,QString fileSaveName,QString fileSaveName2);
void neuron_merge(NeuronTree* nt,NeuronTree* nt2,QString fileSaveName);
void neuron_merge_test(NeuronTree* nt,NeuronTree* nt2,NeuronTree* nt3,QString fileSaveName);
void conflict_detection(NeuronTree* nt,NeuronTree* nt2,NeuronTree* ancestor);
#endif // 
