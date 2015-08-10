#include "populate_neurons.h"
#include <stdlib.h>
#include "../affine_transform_swc/apply_transform_func.h"
#include <newmat.h>
#include <iostream>
#include <numeric>
#include <algorithm>
#include "QMutableListIterator"


V3DLONG randomPosition1D(V3DLONG range){
     return (rand() %range);
}

float mean(vector<float> x)
{
     float x_mean  = accumulate( x.begin(), x.end(), 0.0 )/ x.size();
     return x_mean;
}


float range(vector<float> x)
{
    float max_x =  *( max_element(x.begin(), x.end()));
    float min_x =  *( min_element(x.begin(), x.end()));
    return fabs(max_x - min_x);

}

struct Point3D{
  float x;
  float y;
  float z;
};

struct NeuronSize{
  float x;
  float y;
  float z;
};

struct MyBoundingBox{
  float min_x;
  float min_y;
  float min_z;
  float max_x;
  float max_y;
  float max_z;
};


MyBoundingBox neuron_tree_bb(NeuronTree nt){
    vector<float>  x_values;
    vector<float>  y_values;
    vector<float>  z_values;

    for (int i =0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC a = nt.listNeuron.at(i);
        x_values.push_back( a.x);
        y_values.push_back( a.y);
        z_values.push_back( a.z);
    }

    MyBoundingBox bb ={0,0,0,0,0,0};
    bb.min_x = *( min_element(x_values.begin(), x_values.end()));
    bb.min_y = *( min_element(y_values.begin(), y_values.end()));
    bb.min_z = *( min_element(z_values.begin(), z_values.end()));
    bb.max_x = *( max_element(x_values.begin(), x_values.end()));
    bb.max_y = *( max_element(y_values.begin(), y_values.end()));
    bb.max_z = *( max_element(z_values.begin(), z_values.end()));

    return bb;
}


bool IsInside(Point3D p, MyBoundingBox bb){

       if ( p.x >= bb.min_x  &&  p.x <= bb.max_x
       &&   p.y >= bb.min_y  &&  p.y <= bb.max_y
       &&   p.z >= bb.min_z  &&  p.z <= bb.max_z )
            return true;
       else
            return false;
}


NeuronSize neuron_tree_size(NeuronTree nt){
    vector<float>  x_values;
    vector<float>  y_values;
    vector<float>  z_values;

    for (int i =0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC a = nt.listNeuron[i];
        x_values.push_back( a.x);
        y_values.push_back( a.y);
        z_values.push_back( a.z);
    }

    NeuronSize siz;
    siz.x=range(x_values);
    siz.y=range(x_values);
    siz.z=range(x_values);
    return siz;
}



Point3D neuron_tree_center(NeuronTree nt){
    vector<float>  x_values;
    vector<float>  y_values;
    vector<float>  z_values;

    for (int i =0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC a = nt.listNeuron[i];
        x_values.push_back( a.x);
        y_values.push_back( a.y);
        z_values.push_back( a.z);
    }

    Point3D center;
    center.x= mean(x_values);
    center.y= mean(x_values);
    center.z= mean(x_values);
    return center;
}



NeuronTree populate_transform (NeuronTree sampleNeuron,
                               V3DLONG x, V3DLONG y, V3DLONG z, float rotation_z )
{

    Point3D center = neuron_tree_center(sampleNeuron);
    //NeuronSize size = neuron_tree_size(sampleNeuron);

    Matrix translateMatrix3by4 = translate_matrix(-center.x, -center.y, -center.z);
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

/*

void removeSubTree( QMutableListIterator<NeuronSWC> iter, int &currentIdx )
{
        while( iter.hasNext())
    {
        int pid = currentIdx;
        NeuronSWC node = iter.next(); // recusive remove the child tree
        currentIdx ++;
        if (node.parent == pid) {
            removeSubtree(iter, currentIdx);
        }
        else{
            iter.previous(); //remove the current node
            iter.remove();
        }
    }

}
*/

void prune_by_boundingbox (NeuronTree &nt, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z)
{ // prune segments that exceed the bounding box specified by siz_x/y/z

    MyBoundingBox bb = neuron_tree_bb(nt);

    if (bb.min_x > 0 && bb.max_x < siz_x  &&  bb.min_y >0  &&  bb.max_y < siz_y && bb.min_z>0 && bb.max_z < siz_z)
    {  // if it is entirely within the big bb
        // skip prunning
        cout <<"yes"<<endl;
    }
    // check each node and remove the
    // chidren branch when found outside of the bb;
    // Soma are assumed to be always in the bounding box in the pupulating process,
    // so we can search from the soma node,
    bb = {0,0,0,float(siz_x),float(siz_y),float(siz_z)};


    //method
    /*
    QMutableListIterator<NeuronSWC> iter(nt.listNeuron);
    int currentId = -1 ;
    while (iter.hasNext()) {
        NeuronSWC node = iter.next();
        currentId ++;
        Point3D p ={node.x, node.y,node.z};
        if (! IsInside(p,bb))
            removeSubTree(iter,currentId);
    }*/


    // tag the subtree  that are outside of bounding box with type =-1
    for (int i  = 0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC * node = &(nt.listNeuron[i]);

        if (node->type >= 0){
            Point3D p ={node->x, node->y,node->z};
            if (! IsInside(p,bb)){ // outside of bounding box
                // assumption: the tree is pre-sorted
                node->type = -1 ;//tag remove
                for (int j = i +1 ; j < nt.listNeuron.size(); j++)
                {
                    int parent_id =  nt.listNeuron[j].parent;
                    if (parent_id == -1 )
                       { cout <<"\n warning!  soma is outside of the bundong box, the whole tree is deleted."<<endl;}
                    if ( nt.listNeuron.at(parent_id-1).type == -1) //parent_id starts from 1
                    {
                        (&nt.listNeuron[j])->type = -1 ;
                    }
                }
            }
        }

    }

    writeSWC_file("./test_type.swc",nt);

    // removed tagged
    QMutableListIterator<NeuronSWC> iter(nt.listNeuron);
    while (iter.hasNext())
    {
        NeuronSWC node = iter.next();
        if (node.type == -1){
            iter.remove();
        }
     }
}



QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2)
{
    QList<ImageMarker> contacts;




    return contacts;
}
