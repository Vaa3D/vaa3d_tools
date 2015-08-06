#include "populate_neurons.h"
#include <stdlib.h>
#include "../affine_transform_swc/apply_transform_func.h"
#include <newmat.h>
#include <iostream>
#include <numeric>
#include <algorithm>

V3DLONG randomPosition1D(V3DLONG range){
     return (rand() %range);
}

double mean(vector<double> x)
{
     double x_mean  = accumulate( x.begin(), x.end(), 0.0 )/ x.size();
     return x_mean;
}


double range(vector<double> x)
{
    double max_x =  *( max_element(x.begin(), x.end()));
    double min_x =  *( min_element(x.begin(), x.end()));
    return fabs(max_x - min_x);

}



NeuronTree populate_transform (NeuronTree sampleNeuron,
                               V3DLONG x, V3DLONG y, V3DLONG z, float rotation_z )
{

    vector<double>  x_values;
    vector<double>  y_values;
    vector<double>  z_values;

    for (int i =0; i < sampleNeuron.listNeuron.size(); i++)
    {
        NeuronSWC a = sampleNeuron.listNeuron[i];
        x_values.push_back( a.x);
        y_values.push_back( a.y);
        z_values.push_back( a.z);
    }

    int center_x = mean(x_values);
    int center_y = mean(y_values);
    int center_z = mean(z_values);

    cout << "size: x="<<range(x_values)<<", y="<<range(y_values) <<", z="<<range(z_values)<<endl;
    cout << "center: x="<<center_x<<", y="<<center_y <<", z="<<center_z<<endl;

    Matrix translateMatrix3by4 = translate_matrix(-center_x, -center_y, -center_z);
    NeuronTree sampleNeuronOffset = apply_transform(&sampleNeuron,translateMatrix3by4);


    unit_vector y_axis;  //default to be y_axis, todo: allow input axis
    y_axis.x = 0;
    y_axis.y = 1;
    y_axis.z = 0;
    Matrix affineMatrix3by4 = affine_matrix(y_axis,rotation_z, x, y, z);
    NeuronTree new_neuron = apply_transform(&sampleNeuronOffset,affineMatrix3by4);

    if (new_neuron.listNeuron.size() == 0 )
    {
        std::cout<<"list num = "<<new_neuron.listNeuron.size()<<" somethign is wrong: empty neuron tree"<<std::endl;
    }

    return new_neuron;
}




QList<NeuronTree> populate_neurons(NeuronTree sampleNeuron, int num_neurons,
                                   float maxRotation, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z)
{
    QList<NeuronTree> neurons;

    for (int i =0; i< num_neurons; i++) {
        V3DLONG x = randomPosition1D(siz_x);
        V3DLONG y = randomPosition1D(siz_y);
        V3DLONG z = randomPosition1D(siz_z);
        int  rotation_angle = rand() % int(maxRotation); // in degrees

        std::cout<<"random trasnform:" <<"x="<<x <<",y="<<y<<",z="<<z<<", angle=" <<rotation_angle;
        NeuronTree  new_neuron = populate_transform (sampleNeuron, x,y,z,rotation_angle );

        neurons.push_back( new_neuron);

    }

    cout << "populated nuerons size = " << neurons.size()<<endl;




    return neurons;
}






QList<ImageMarker> detect_contacts(QList<NeuronTree*> neuronTreeList, int type1, int type2)
{
    QList<ImageMarker> contacts;

    return contacts;
}
