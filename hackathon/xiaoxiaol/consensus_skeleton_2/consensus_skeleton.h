#ifndef _CONSENSUS_SKELETON_H_
#define _CONSENSUS_SKELETON_H_

#include <vector>
#include "v3d_interface.h"  // use call back to save image for debugging purpose
#include "basic_surf_objs.h"
#include "point3d_util.h"
#include "ANN/ANN.h"

using namespace std;

bool  export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename);
void  non_max_suppresion( unsigned char * img1d,V3DLONG sz_x, V3DLONG sz_y,V3DLONG sz_z,double threshold_votes,
                          Point3D offset, vector<Point3D>  &node_list, vector<unsigned int> &vote_list, unsigned int win_size);

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);


void AddToMaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz,int dialate_radius,
                    double origin_x, double origin_y,double origin_z,
                    int imageCount, V3DPluginCallback2 & callback);

bool sort_all_inputs(vector<NeuronTree> & nt_list, double bridge_gap);
bool prune_all_inputs(vector<NeuronTree> & nt_list, double prune_length);
bool  prune_branch(NeuronTree nt, NeuronTree & result, double prune_size);
bool  remove_outliers(vector<NeuronTree> & nt_list, QString SelectedNeuronsAnoFileName);
void  isIsolated(vector<int>& isolated,vector<double>& nt_lens,vector<double>& nt_N_bifs, double radius, int count);
void  normalizeVector(vector<double>& original, vector<double>& normalized);
void  buildDistanceMatrix(vector< vector<double> >& dist, vector<double>& x, vector<double>& y);
//void build_tree_from_adj_matrix(QList<NeuronSWC> node_list, unsigned short * adjMatrix, QList<NeuronSWC> merge_result);


double  computeTotalLength(const NeuronTree & nt);
int  computeNumberOfBifurcations(const NeuronTree & nt);
bool vote_map(vector<NeuronTree> & nt_list, int dialate_radius, QString outfileName,V3DPluginCallback2 & callback);
bool consensus_skeleton_votemap(vector<NeuronTree>  nt_list, QList<NeuronSWC> & merge_result,
                                int max_vote_threshold,int cluster_distance_threshold, V3DPluginCallback2 &callback);



//votemap MST
bool generate_vote_map(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);
void label_image(unsigned char * imgid, V3DLONG xs, V3DLONG ys, V3DLONG zs, double rs,
                 unsigned char * VOTED, V3DLONG sx, V3DLONG sy,V3DLONG sz);
bool generate_vote_map_resample(vector<NeuronTree> & nt_list, int dialate_radius, unsigned char * img1d,
                       V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z, Point3D offset);




// match and center

bool soma_sort(double search_distance_th, QList<NeuronSWC> consensus_nt_list, double soma_x, double soma_y, double soma_z,
               QList<NeuronSWC> &out_sorted_consensus_nt_list, double bridge_size);

double match_and_center(vector<NeuronTree> nt_list, int input_neuron_id,  double distance_threshold, NeuronTree & adjusted_neuron);
XYZ mean_XYZ(vector<XYZ> points);
bool tightRange(vector<double> x, double &low, double &high);
void  resample_neurons(vector<NeuronTree> nt_list,  vector<NeuronTree> * nt_list_resampled);

void run_match_center(vector<NeuronTree> & nt_list, int max_num_iters, double cluster_distance_threshold);
bool consensus_skeleton_match_center(vector<NeuronTree> nt_list, QList<NeuronSWC> & final_consensus, int max_vote_threshold,
double cluster_distance_threshold, int resample_flag, V3DPluginCallback2 &callback);
bool merge_and_vote(vector<NeuronTree> & nt_list_resampled,
                    int vote_threshold,QList<NeuronSWC> &merge_result,int TYPE_MERGED);
double correspondingNodeFromNeuron(XYZ pt,QList<NeuronSWC> listNodes, int &closestNodeIdx,int TYPE_MERGED);
int build_adj_matrix( vector<NeuronTree>  nt_list, QList<NeuronSWC>  merge_result, unsigned short * adjMatrix,int TYPE_MERGED);
int postprocessing_neuron_node_list(QList<NeuronSWC>  &merge_result, unsigned short*&  adjMatrix, double vote_threshold);
bool build_tree_from_adj_matrix(unsigned short * adjMatrix, QList<NeuronSWC> &merge_result,double vote_threshold);
bool build_tree_from_adj_matrix_mst(unsigned short * adjMatrix, int n_edges, QList<NeuronSWC> &merge_result, double vote_threshold);
void trim_unconfident_branches(QList<NeuronSWC> &merge_result,int vote_threshold);
void generate_batch_trimmed_results(NeuronTree nt,QString outfileName,double initial_threshold, int steps);

double kd_match_and_center(vector<NeuronTree> nt_list, vector<ANNkd_tree *>kd_list, int input_neuron_id,  double distance_threshold, NeuronTree & adjusted_neuron);
void kd_run_match_center(vector<NeuronTree> & nt_list, int max_num_iters, double cluster_distance_threshold);
bool kd_merge_and_vote(vector<NeuronTree>  & nt_list_resampled,int vote_threshold,  QList<NeuronSWC> &merge_result, int TYPE_MERGED);
#endif
