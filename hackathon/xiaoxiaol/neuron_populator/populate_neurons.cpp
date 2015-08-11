#include "populate_neurons.h"
#include <stdlib.h>
#include "../affine_transform_swc/apply_transform_func.h"
#include <newmat.h>
#include <iostream>
#include <numeric>
#include <algorithm>
#include "QMutableListIterator"

#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif

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
    center.x = mean(x_values);
    center.y = mean(x_values);
    center.z = mean(x_values);
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
                    int parent_id =  (nt.listNeuron[j]).parent;
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

    //debug
    //writeSWC_file("./test_type.swc",nt);

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


void label_image_by_type(unsigned char * img1d,V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                         Point3D offset, NeuronTree nt, int type,  int typeValue)
{

    for (int i = 0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC node = nt.listNeuron.at(i);
        V3DLONG id_x = round(node.x-offset.x) ;
        V3DLONG id_y = round(node.y-offset.y);
        V3DLONG id_z = round(node.x-offset.z);
        if (node.type == type){
            V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;
            img1d[idx] = typeValue;
        }

    }
}



QList<ImageMarker> detect_pairwise_contacts(NeuronTree treeA, NeuronTree treeB, int type1, int type2, double closeness)
{
    QList<ImageMarker> pair_contacts;
    int A1 = 1;
    int A2 = 2;
    int B1 = 3;
    int B2 = 4;
    //A1B2 = 5;
    //A2B1 = 5
    MyBoundingBox bbA = neuron_tree_bb(treeA);
    MyBoundingBox bbB = neuron_tree_bb(treeB);

    MyBoundingBox bbUnion;
    bbUnion.min_x = MIN(bbA.min_x, bbB.min_x);
    bbUnion.min_y = MIN(bbA.min_y, bbB.min_y);
    bbUnion.min_z = MIN(bbA.min_z, bbB.min_z);
    bbUnion.max_x = MAX(bbA.max_x, bbB.max_x);
    bbUnion.max_y = MAX(bbA.max_y, bbB.max_y);
    bbUnion.max_z = MAX(bbA.max_z, bbB.max_z);

    //if bbA does not corss bbB , return
    Point3D offset = {bbUnion.min_x ,bbUnion.min_y ,bbUnion.min_z };
    V3DLONG  sz_x = bbUnion.max_x - bbUnion.min_x;
    V3DLONG  sz_y = bbUnion.max_y - bbUnion.min_y;
    V3DLONG  sz_z = bbUnion.max_x - bbUnion.min_z;
    V3DLONG  tol_sz = sz_x *sz_y*sz_z;
    //else

    cout << "image size = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;
    //mask A
    //Image4DSimple * imageA = new Image4DSimple();
    unsigned char *  img1d_A = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) img1d_A[i] = 0;
    label_image_by_type (img1d_A, tol_sz, sz_x, sz_y,sz_z,offset,treeA,type1,A1);
    label_image_by_type (img1d_A, tol_sz, sz_x, sz_y,sz_z,offset,treeA,type2,A2);

    //mask B
    //Image4DSimple * imageB = new Image4DSimple();
    unsigned char *  img1d_B = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) img1d_B[i] = 0;
    label_image_by_type (img1d_B, tol_sz, sz_x, sz_y,sz_z,offset,treeB,type1,B1);
    label_image_by_type (img1d_B, tol_sz, sz_x, sz_y,sz_z,offset,treeB,type2,B2);

    //add
    unsigned char *  img1d = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) {
        img1d[i] = img1d_A[i] + img1d_B[i];
        if (img1d[i] == 5){
            ImageMarker mark;
            mark.z = i / (sz_x*sz_y)                       + offset.z;
            mark.y = (i - mark.z *(sz_x*sz_y) ) / sz_x     + offset.y;
            mark.x = i - mark.z *(sz_x*sz_y) - mark.y*sz_x + offset.x;
            cout << "!Find contact" <<endl;
            pair_contacts.push_back(mark);
        }
    }


    // image->setData(img1d, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    delete img1d_A;
    delete img1d_B;
    delete img1d;

    return pair_contacts;

}


QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2 , double closeness)
{
    QList<ImageMarker> contacts;
    for (int i = 0; i < neuronTreeList.size()-1 ; i++)
    {
        for (int j = i+1; j <neuronTreeList.size() ; j++)
        {
            NeuronTree treeA = neuronTreeList.at(i);
            NeuronTree treeB = neuronTreeList.at(j);
            QList<ImageMarker>  pair_contacts = detect_pairwise_contacts(treeA, treeB, type1, type2, closeness);
            if (!pair_contacts.isEmpty())
            {
                contacts.append(pair_contacts);

                cout << "number of contacts between tree 1 and tree2 :"<<pair_contacts.size()<<endl;
            }
            else{
                cout << "no contacts" <<endl;
            }
        }
    }
    return contacts;
}
