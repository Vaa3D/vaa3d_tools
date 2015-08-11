#ifndef _POPULATE_NEURON_H_
#define _POPULATE_NEURON_H_



#include <v3d_interface.h>


V3DLONG randomPosition1D(V3DLONG range);
NeuronTree populate_transform (NeuronTree sampleNeuron,V3DLONG x, V3DLONG y, V3DLONG z, float rotation_z );

void removeSubtree(NeuronTree &nt, int nodeIndex);
void prune_by_boundingbox (NeuronTree &nt, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z);
//void removeSubTree( QMutableListIterator<NeuronSWC> iter, int &currentIdx );
QList<NeuronTree>  populate_neurons(NeuronTree sampleNeuron,int num_neurons,float maxRotation, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z);

QList<ImageMarker> detect_pairwise_contacts(NeuronTree treeA, NeuronTree treeB, int type1, int type2, double closeness);
QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2,double closeness);



#endif // _POPULATE_NEURON_H_



