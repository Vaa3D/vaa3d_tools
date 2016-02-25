#ifndef _CONSENSUS_SKELETON_H_
#define _CONSENSUS_SKELETON_H_

#include <vector>
#include "v3d_interface.h"  // use call back to save image for debugging purpose
#include "basic_surf_objs.h"
#include "point3d_util.h"
using namespace std;

bool  export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
void  non_max_suppresion( unsigned char * img1d,V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,double threshold_votes,
                          Point3D offset, vector<Point3D>  &node_list, vector<unsigned int> &vote_list, unsigned int win_size);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);

//void AddTMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, int imageCount,V3DPluginCallback2 &callback);
void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,int dialate_radius,
                    double origin_x, double origin_y,double origin_z,
                    int imageCount, V3DPluginCallback2 & callback);
void AddToMaskImage_old(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,
                    double origin_x, double origin_y,double origin_z,
                    int imageCount, V3DPluginCallback2 & callback);

double  computeTotalLength(const NeuronTree & nt);
bool vote_map(vector<NeuronTree> & nt_list, int dialate_radius, QString outfileName,V3DPluginCallback2 & callback);
bool consensus_skeleton(vector<NeuronTree> & nt_list, QList<NeuronSWC> & merge_result, int method_code,
                         int cluster_distance_threshold, V3DPluginCallback2 &callback);
bool generate_vote_map(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);

void label_image(unsigned char * imgid, V3DLONG xs, V3DLONG ys, V3DLONG zs, double rs,
                 unsigned char * VOTED, V3DLONG sx, V3DLONG sy,V3DLONG sz);
bool generate_vote_map_resample(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);


bool soma_sort(double search_distance_th, QList<NeuronSWC> consensus_nt_list, double soma_x, double soma_y, double soma_z,
               QList<NeuronSWC> &out_sorted_consensus_nt_list, double bridge_size);

void  remove_outliers(vector<NeuronTree> & nt_list,double &median_root_x, double &median_root_y, double  &median_root_z);
#endif
