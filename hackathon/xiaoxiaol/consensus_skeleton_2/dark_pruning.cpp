#include "dark_pruning.h"
#include "v_neuronswc.h"
#include <iostream>


#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif


double get_nearestneighbor_ave_int(Image4DSimple* image, V3DLONG x, V3DLONG y, V3DLONG z){
    double val = 0;
    V3DLONG x_start = MAX(0,x-1);
    V3DLONG x_end = MIN(x+1, image->getXDim()-1);
    V3DLONG y_start = MAX(0,y-1);
    V3DLONG y_end = MIN(y+1, image->getYDim()-1);
    V3DLONG z_start = MAX(0,z-1);
    V3DLONG z_end = MIN(z+1, image->getZDim()-1);

    for (V3DLONG i = x_start; i <= x_end; i++)
        for (V3DLONG j = y_start; j <= y_end; j++)
            for (V3DLONG k = z_start; k <= z_end; k++)
            {
              val += int(image->getValueUINT8(i,j,k,0));
            }
    return val/27;
};


//prune leave nodes iteratively, according to the image intensity value (< threshold are going to be removed)
bool dark_pruning(NeuronTree input_tree,  QList<NeuronSWC> &output_tree, Image4DSimple * image, int visible_threshold)
{
    //scale to 0-255 range
    image->convert_to_UINT8();

    //input tree are sorted
    V3DLONG num_nodes = input_tree.listNeuron.size();
    QList<NeuronSWC> list_n= input_tree.listNeuron;

    //keep track of children number to identify tip nodes
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
                if (parent == -1)
                { //isolated roots
                    num_children[j] = -1;
                    nprune++;
                    count++;
                    continue;
                }
                V3DLONG  p = input_tree.hashNeuron[parent];

                V3DLONG x  = list_n[j].x;
                V3DLONG y  = list_n[j].y;
                V3DLONG z  = list_n[j].z;

                //unsigned char va = image->getIntensityUnit8(x,y,z,0);
                double va =  get_nearestneighbor_ave_int (image, x,y,z);
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


