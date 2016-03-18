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


void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,int dialate_radius,
                    double origin_x, double origin_y,double origin_z,
                    int imageCount, V3DPluginCallback2 & callback);



bool  remove_outliers(vector<NeuronTree> & nt_list, QString SelectedNeuronsAnoFileName);

//void build_tree_from_adj_matrix(QList<NeuronSWC> node_list, double * adjMatrix, QList<NeuronSWC> merge_result);

//votemap MST
double  computeTotalLength(const NeuronTree & nt);
bool vote_map(vector<NeuronTree> & nt_list, int dialate_radius, QString outfileName,V3DPluginCallback2 & callback);
bool consensus_skeleton_votemap(vector<NeuronTree>  nt_list, QList<NeuronSWC> & merge_result,
                                int max_vote_threshold,int cluster_distance_threshold, V3DPluginCallback2 &callback);


bool generate_vote_map(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);
void label_image(unsigned char * imgid, V3DLONG xs, V3DLONG ys, V3DLONG zs, double rs,
                 unsigned char * VOTED, V3DLONG sx, V3DLONG sy,V3DLONG sz);
bool generate_vote_map_resample(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);
bool soma_sort(double search_distance_th, QList<NeuronSWC> consensus_nt_list, double soma_x, double soma_y, double soma_z,
               QList<NeuronSWC> &out_sorted_consensus_nt_list, double bridge_size);



// match and center
double dist_pt_to_line_seg( XYZ p0, XYZ p1,  XYZ p2, XYZ & closestPt);
double correspondingPointFromNeuron(XYZ cur,NeuronTree * nt, XYZ & closest_p);
bool match_and_center(vector<NeuronTree> nt_list, int input_neuron_id,  double distance_threshold, NeuronTree & adjusted_neuron);
XYZ mean_XYZ(vector<XYZ> points);
bool consensus_skeleton_match_center(vector<NeuronTree> nt_list, QList<NeuronSWC> & final_consensus, int max_vote_threshold,
int cluster_distance_threshold, V3DPluginCallback2 &callback);
bool merge_and_vote(vector<NeuronTree> & nt_list_resampled,
                    int vote_threshold,QList<NeuronSWC> &merge_result);
double correspondingNodeFromNeuron(XYZ pt, NeuronTree * p_nt, int &closestNodeId,int TYPE_MERGED);
bool build_adj_matrix( vector<NeuronTree>  nt_list, QList<NeuronSWC>  merge_result, double * adjMatrix);
bool build_tree_from_adj_matrix(double * adjMatrix, QList<NeuronSWC> &merge_result,double vote_threshold);
#endif
