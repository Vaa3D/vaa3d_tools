#ifndef _POPULATE_NEURON_H_
#define _POPULATE_NEURON_H_



#include <v3d_interface.h>



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

V3DLONG randomPosition1D(V3DLONG range);
NeuronTree populate_transform (NeuronTree sampleNeuron,V3DLONG x, V3DLONG y, V3DLONG z, float rotation_z );

void removeSubtree(NeuronTree &nt, int nodeIndex);
void prune_by_boundingbox (NeuronTree &nt, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z);
//void removeSubTree( QMutableListIterator<NeuronSWC> iter, int &currentIdx );
QList<NeuronTree>  populate_neurons(NeuronTree sampleNeuron,int num_neurons,float maxRotation, V3DLONG siz_x,  V3DLONG siz_y,  V3DLONG siz_z);

QList<ImageMarker> detect_pairwise_contacts(const NeuronTree treeA, const NeuronTree treeB, int type1, int type2,
                                            float closeness,V3DPluginCallback2 &callback);
QList<ImageMarker> detect_contacts(QList<NeuronTree> neuronTreeList, int type1, int type2,float closeness,
                                   V3DPluginCallback2 &callback);

void label_neighborhood(unsigned char * img1d,V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                         Point3D center, int typeValue, float closeness);
void label_image_by_type(unsigned char * img1d, V3DLONG tol_sz, V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                        Point3D offset, NeuronTree nt, int type,  int typeValue);


#endif // _POPULATE_NEURON_H_



