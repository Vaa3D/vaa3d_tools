#ifndef _CONSENSUS_SKELETON_H_
#define _CONSENSUS_SKELETON_H_

#include <vector>
#include "v3d_interface.h"  // use call back to save image for debugging purpose
#include "basic_surf_objs.h"
using namespace std;
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


bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
void  non_max_suppresion( unsigned char * img1d,V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                          Point3D offset, vector<Point3D>  &node_list, vector<unsigned char> &vote_list, unsigned int win_size);
bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, int method_code,V3DPluginCallback2 &callback);


void remove_outliers(vector<NeuronTree> & nt_list);
#endif


