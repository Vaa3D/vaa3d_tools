#ifndef _POPULATE_NEURON_H_
#define _POPULATE_NEURON_H_



#include <v3d_interface.h>


QList<NeuronTree>  populate_neurons(NeuronTree sampleNeuron, double maxRotation, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z);
QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2);



#endif // _POPULATE_NEURON_H_



