#ifndef _CONSENSUS_SKELETON_H_
#define _CONSENSUS_SKELETON_H_

#include <vector>
#include "v3d_interface.h"  // use call back to save image for debugging purpose
#include "basic_surf_objs.h"
#include "point3d_util.h"
using namespace std;

bool  export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
void  non_max_suppresion( unsigned char * img1d,V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,
                          Point3D offset, vector<Point3D>  &node_list, vector<unsigned int> &vote_list, unsigned int win_size);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);

//void AddTMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, int imageCount,V3DPluginCallback2 &callback);
void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,
                    double origin_x, double origin_y,double origin_z,
                    int imageCount, V3DPluginCallback2 & callback);

bool vote_map(vector<NeuronTree> & nt_list,  QString outfileName,V3DPluginCallback2 & callback);
bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, int method_code,
                         V3DPluginCallback2 &callback);


void  remove_outliers(vector<NeuronTree> & nt_list);
#endif
