#include "populate_neurons.h"
#include <stdlib.h>
#include "../affine_transform_swc/apply_transform_func.h"
#include <newmat.h>
#include <iostream>
#include <numeric>
#include <algorithm>
#include "QMutableListIterator"
#include "basic_4dimage.h"

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



V3DLONG boundValue(V3DLONG x, V3DLONG m_min, V3DLONG m_max)
{
    x = MAX(x, m_min);
    x = MIN(x, m_max);
    return x;

}



MyBoundingBox neuron_tree_bb(const NeuronTree nt){
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
    siz.y=range(y_values);
    siz.z=range(z_values);
    return siz;
}



Point3D neuron_tree_center(const NeuronTree nt){
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
    center.y = mean(y_values);
    center.z = mean(z_values);
    return center;
}



NeuronTree populate_transform (NeuronTree sampleNeuron,
                               V3DLONG x, V3DLONG y, V3DLONG z, float rotation_y )
{

    //Point3D center = neuron_tree_center(sampleNeuron);

    //make sure soma point is within the specified space
    Point3D somaPoint;
    somaPoint.x= sampleNeuron.listNeuron[0].x;
    somaPoint.y= sampleNeuron.listNeuron[0].y;
    somaPoint.z= sampleNeuron.listNeuron[0].z;

    Matrix translateMatrix3by4 = translate_matrix(-somaPoint.x, -somaPoint.y, -somaPoint.z);
    NeuronTree sampleNeuronOffset = apply_transform(&sampleNeuron,translateMatrix3by4);


    unit_vector y_axis;  //default to be y_axis, todo: allow input axis
    y_axis.x = 0;
    y_axis.y = 1;
    y_axis.z = 0;
    Matrix affineMatrix3by4 = affine_matrix(y_axis,rotation_y, x, y, z);
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
        cout <<"Completely inside the specified space range."<<endl;
        return;
    }
    // check each node and remove the
    // chidren branch when found outside of the bb;
    // Soma are assumed to be always in the bounding box in the pupulating process,
    // so we can search from the soma node,
    bb.min_x = 0;
    bb.min_y = 0;
    bb.min_z = 0;
    bb.max_x = siz_x-1;
    bb.max_y = siz_y-1;
    bb.max_z = siz_z-1;


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

void label_neighborhood(unsigned char * img1d,V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                        Point3D center, int typeValue, float closeness)

{
    V3DLONG id_x,id_y,id_z;



    for (id_x = MAX(0,center.x - closeness); id_x <=  MIN(sz_x-1,center.x + closeness ); id_x++)
        for (id_y = MAX(0,center.y - closeness); id_y <= MIN(sz_y-1,center.y + closeness) ; id_y++)
            for (id_z = MAX(0, center.z - closeness); id_z <= MIN(sz_z-1,center.z + closeness) ; id_z++)
            {
                V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;

                img1d[idx] = typeValue;
                //tag neighborhood region defined by r= closeness with the same value

            }



}
/*
void label_image_by_type(unsigned char * img1d,V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                         Point3D offset, NeuronTree nt, int type,  int typeValue, float closeness)
{
    if (!img1d )
    {
        cout<<"error in label_image_by_type(): Null 1d image pointer!" <<endl;
        return;
    }

    for (int i = 0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC node = nt.listNeuron.at(i);
        V3DLONG id_x = (node.x-offset.x) +0.5; //round up
        V3DLONG id_y = (node.y-offset.y) +0.5;
        V3DLONG id_z = (node.z-offset.z) +0.5;

        if (node.type == type)
        {
           Point3D center;
           center.x = id_x;
           center.y = id_y;
           center.z = id_z;
           label_neighborhood(img1d,tol_sz,sz_x,sz_y,sz_z,center,typeValue,closeness);
        }

    }

}
*/

void label_image_by_type(unsigned char * img1d,V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                         Point3D offset,   NeuronTree nt, int type,  int typeValue, float closeness)
{
    if (!img1d )
    {
        cout<<"error in label_image_by_type(): Null 1d image pointer!" <<endl;
        return;
    }

    for (int i = 0; i < nt.listNeuron.size(); i++)
    {
        NeuronSWC node = nt.listNeuron.at(i);
        V3DLONG id_x = (node.x-offset.x)/closeness -1 +0.5; //round up
        V3DLONG id_y = (node.y-offset.y)/closeness -1 +0.5;
        V3DLONG id_z = (node.z-offset.z)/closeness -1 +0.5;

        if (node.type == type)
        {
            V3DLONG idx = id_z * (sz_x*sz_y) + id_y * sz_x + id_x;
            if (idx < tol_sz)
            {
                img1d[idx] = typeValue;
            }
            else
            {
                cout<<"error in indexing:[" <<id_x <<" "<<id_y<<" "<<id_z<<"]"<<endl;
                cout <<"size:[" <<sz_x  <<" " << sz_y<<" "<< sz_z <<"]"<<endl;

            }
        }

    }

}


QList<ImageMarker> detect_pairwise_contacts(const NeuronTree treeA, const NeuronTree treeB, int type1, int type2,
                                            float closeness,V3DPluginCallback2 &callback)
{
    QList<ImageMarker> pair_contacts;
    if (treeA.listNeuron.isEmpty() || treeB.listNeuron.isEmpty() ){
        cout << "Error: Empty Tree detected." << endl;
        return pair_contacts;
    }

    for (int i = 0; i < treeA.listNeuron.size(); i++)
    {
        NeuronSWC nodeA = treeA.listNeuron.at(i);

        double min_dis = 1000000.00; // a big enough number
        NeuronSWC min_node;
        for (int j = 0; j < treeB.listNeuron.size(); j++)
        {
            NeuronSWC nodeB = treeB.listNeuron.at(j);
            if ( (nodeA.type == type1 && nodeB.type == type2)  ||  (nodeA.type == type2 && nodeB.type == type1) )
            {
                double dis_square = pow((nodeA.x - nodeB.x), 2.0) + pow((nodeA.y-nodeB.y), 2.0) + pow((nodeA.z-nodeB.z),2.0);

                if(dis_square <= pow(closeness,2.0) )
                {
                    if (dis_square < min_dis)
                    {
                        min_node = nodeB;
                        min_dis = MIN(min_dis, dis_square);
                    }
                }
            }
        }

        if (min_dis != 1000000.00){
            ImageMarker marker;
            //A
            /*
    marker.x = nodeA.x+1;
    marker.y = nodeA.y+1;
    marker.z = nodeA.z+1;
    pair_contacts.push_back(marker);
    */
            marker.x = (nodeA.x+min_node.x)/2+1;  // +1 due to vaa3d convention: landmarkers are 1-based
            marker.y = (nodeA.y+min_node.y)/2+1;
            marker.z = (nodeA.z+min_node.z)/2+1;
            pair_contacts.push_back(marker);
            //B
            /*
    marker.x = min_node.x+1;
    marker.y = min_node.y+1;
    marker.z = min_node.z+1;
    pair_contacts.push_back(marker);
    */
        i = i + closeness; // to space the nodes around to avoid too many contacts within a small region defined by closeness, assuming the step size of the swc nodes  is 1
        }
    }
    return pair_contacts;


}

/*
QList<ImageMarker> detect_pairwise_contacts(const NeuronTree treeA, const NeuronTree treeB, int type1, int type2,
    float closeness,V3DPluginCallback2 &callback)
{
    QList<ImageMarker> pair_contacts;
    if (treeA.listNeuron.isEmpty() || treeB.listNeuron.isEmpty() ){
    cout << "Error: Empty Tree detected." << endl;
    return pair_contacts;
    }


    int A1 = 30*1;
    int A2 = 30*2;
    int B1 = 30*3;
    int B2 = 30*4;
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
    V3DLONG  sz_x = ceil((bbUnion.max_x - bbUnion.min_x ) / closeness); //+0.5 to round up from float to V3DLONG
    V3DLONG  sz_y = ceil((bbUnion.max_y - bbUnion.min_y ) / closeness);
    V3DLONG  sz_z = ceil((bbUnion.max_z - bbUnion.min_z ) / closeness);
    V3DLONG  tol_sz = sz_x * sz_y * sz_z;
    //else

    // cout << "image size = " << tol_sz<<": " <<sz_x<<"x "<<sz_y<<" x"<<sz_z<< endl;


    //mask A
    unsigned char * img1d_A = new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) img1d_A[i] = 0;
    label_image_by_type (img1d_A, tol_sz, sz_x, sz_y,sz_z, offset, treeA,type1,A1,closeness);
    label_image_by_type (img1d_A, tol_sz, sz_x, sz_y,sz_z, offset, treeA,type2,A2,closeness);


    // Image4DSimple *image = new Image4DSimple();
    // image->setData(img1d_A, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    // callback.saveImage(image, "./test_maskA.v3draw");

    //mask B
    unsigned char * img1d_B= new unsigned char[tol_sz];
    for(V3DLONG i = 0; i < tol_sz; i++) img1d_B[i] = 0;
    label_image_by_type (img1d_B,tol_sz,sz_x, sz_y,sz_z,offset,treeB,type1,B1,closeness);
    label_image_by_type (img1d_B,tol_sz,sz_x, sz_y,sz_z,offset,treeB,type2,B2,closeness);



    //cout << "Done tagging." <<endl;
    //add
    for(V3DLONG i = 0; i < tol_sz; i++) {
    img1d_A[i] = img1d_A[i] + img1d_B[i];
    if (img1d_A[i] == (A1+B2)){
    ImageMarker mark;
    V3DLONG z = i / (sz_x*sz_y)            ;
    V3DLONG y = (i -z *(sz_x*sz_y) ) / sz_x   ;
    V3DLONG x = i - z *(sz_x*sz_y) - y*sz_x   ;
    //cout << "Find contact at:" << x <<" " << y <<" "<< z<<endl;
    mark.x = (x +0.5) * closeness   + offset.x ;
    mark.y = (y +0.5) * closeness  + offset.y ;
    mark.z = (z +0.5) * closeness  + offset.z ;  // +1 landmarks are 1-based in vaa3d
    //  cout << "converted to :" << mark.x <<" " <<mark.y <<" "<< mark.z<<endl;

    pair_contacts.push_back(mark);
    }
    }

    //image->setData(img1d_A, sz_x, sz_y, sz_z, 1, V3D_UINT8);
    //callback.saveImage(image, "./test_Add.v3draw");

    delete[] img1d_A;
    delete[] img1d_B;
    return pair_contacts;

}
*/

QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2 , float closeness,
                                   V3DPluginCallback2 &callback)
{
    QList<ImageMarker> contacts;

    cout <<"neuron_A,neuron_B, num_contacts"<<endl;
    for (int i = 0; i < neuronTreeList.size()-1 ; i++)
    {
        for (int j = i+1; j < neuronTreeList.size() ; j++)
        {

            QList<ImageMarker>  pair_contacts = detect_pairwise_contacts(neuronTreeList.at(i), neuronTreeList.at(j),
                                                                         type1, type2, closeness, callback);
            if (!pair_contacts.isEmpty())
            {
                contacts += pair_contacts;
                //cout << "Number of contacts between tree ["<<i<<"] and tree ["<<j<<"]: "<<pair_contacts.size()<<endl;
                cout <<i<<","<<j<<","<<pair_contacts.size()<<endl;
            }
            else{
                //cout << "Number of contacts between tree ["<<i<<"] and tree ["<<j<<"]: 0" <<endl;
                cout <<i<<","<<j<<","<< 0<<endl;
            }
        }
    }
    cout <<"total contacts number:"<<contacts.size()<<endl;
    return contacts;
}
