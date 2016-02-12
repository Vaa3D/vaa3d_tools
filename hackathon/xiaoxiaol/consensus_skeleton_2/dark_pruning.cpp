#include "dark_pruning.h"
#include "v_neuronswc.h"
#include <iostream>




//prune leave nodes iteratively, according to the image intensity value (< threshold are going to be removed)
bool dark_pruning(NeuronTree input_tree,  QList<NeuronSWC> &output_tree, Image4DSimple * image, int visible_threshold)
{

    //input tree are sorted
    V3DLONG num_nodes = input_tree.listNeuron.size();
    QList<NeuronSWC> list_n= input_tree.listNeuron;

    //copy from listneuron, create supernodes structures with "nchild" attribute
    //assuming the input tree is sorted, id starts from 1
    int num_children[num_nodes];
    for (V3DLONG j=0; j<num_nodes; j++)
    {
        num_children[j]=0;
    }

    //count how many children per parent
    for (V3DLONG j=0; j<num_nodes; j++)
    {
        int parent = list_n[j].parent;
        V3DLONG  p = input_tree.hashNeuron[parent];
        num_children[p]++;
    }

    // tag for prune
    int count = 0;
    for (V3DLONG k=0; ; k++)  //iterate many times
    {
        V3DLONG nprune=0;
        for (V3DLONG j=0; j<num_nodes; j++)
        {
            if (num_children[j]==0)
            {//tip nodes
                int parent = list_n[j].parent;
                V3DLONG  p = input_tree.hashNeuron[parent];

                V3DLONG x  = list_n[j].x;
                V3DLONG y  = list_n[j].y;
                V3DLONG z  = list_n[j].z;

                unsigned char va = image->getIntensity(x,y,z,0);

                if (int(va) <= visible_threshold)  //dark pruning) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
                {
                    num_children[p]--;
                    //label to remove this tip node
                    num_children[j] = -1;
                    nprune++;
                    count++;
                }
            }

        }

        if (nprune==0)
            break;
    }

    cout<<"Will prune "<< count<< " nodes"<<endl;
    //prune the tagged nodes, since only leaf nodes are pruned, it's straighforward to remove nodes independently
    for (V3DLONG j=0; j<num_nodes; j++)
    {
        if (num_children[j] > -1) // to keep
        {
            output_tree.push_back(list_n[j]);
        }
    }

    //todo:resort

    return true;


}


