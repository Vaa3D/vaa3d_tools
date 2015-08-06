#include "populate_neurons.h"
#include <stdlib.h>


V3DLONG randomPosition1D(V3DLONG range){
    return range;

}


NeuronTree populate_transform (NeuronTree sampleNeuron,V3DLONG x, V3DLONG y, V3DLONG z, float rotation_z ){

    NeuronTree new_neuron;
    return new_neuron;
}




QList<NeuronTree> populate_neurons(NeuronTree sampleNeuron, int num_neurons, float maxRotation, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z)
{
    QList<NeuronTree> neurons;

    for (int i =0; i< num_neurons; i++) {
        V3DLONG x = randomPosition1D(siz_x);
        V3DLONG y = randomPosition1D(siz_y);
        V3DLONG z = randomPosition1D(siz_z);
        float rotation_angle = (rand() %100)  * 0.01 * maxRotation;
        NeuronTree new_neuron = populate_transform (sampleNeuron, x,y,z,rotation_angle );
        neurons.push_back(new_neuron);

    }

    return neurons;
}






QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2)
{
    QList<ImageMarker> contacts;

    return contacts;
}
