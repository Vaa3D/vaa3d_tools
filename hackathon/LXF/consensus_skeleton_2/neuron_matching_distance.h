#ifndef NEURON_MATCHING_TOTAL_DISTANCE_H
#define NEURON_MATCHING_TOTAL_DISTANCE_H



#include "basic_surf_objs.h"

struct NeuronDist
{
    double matching_total_dist12;
    double matching_total_dist21;
    double matching_total_dist_ave;
    double weighted_dist12_allnodes;//feature-weighted distance of all interpolated points, from neuron 1 to 2
    double weighted_dist21_allnodes; //feature-weighted distance of all interpolated points, from neuron 2 to 1
    double weighted_dist_ave_allnodes; //the average weighted
    double dist_apartnodes; //the average distance of interpolated points that have significant distance (>= 2 pixels)
    double percent_apartnodes; //the percentage of interpolated points that have significant distance (>= 2 pixels)
    double dist_max; //the maximal distance between two neurons, which is defined the smaller one of the two one-directional max distances
    NeuronDist() {matching_total_dist12=matching_total_dist21=matching_total_dist_ave=weighted_dist12_allnodes = weighted_dist21_allnodes = weighted_dist_ave_allnodes = dist_apartnodes = percent_apartnodes = dist_max = -1; }
};

NeuronDist resampled_neuron_matching_distance(const NeuronTree *p1, const NeuronTree *p2, bool menu);
double resampled_dist_directional_swc_1_2(double &sum_dist, double & w_dist_12big, double & difference_ratio12,  const NeuronTree *p1,
                                         const NeuronTree *p2, double &maxdist, bool USE_WEIGHT);
double dist_pt_to_swc(const XYZ & pt, const NeuronTree * p2);
double dist_pt_to_line(const XYZ & p0, const XYZ &  p1, const XYZ &  p2); //p1 and p2 define a straight line, and p0 the point
double dist_pt_to_line_seg(const XYZ & p0, const XYZ &  p1, const XYZ &  p2); //p1 and p2 are the two ends of the line segment, and p0 the point

QHash<int, int> generate_neuron_swc_hash(const NeuronTree * p_tree); //generate a hash lookup table from a neuron swc graph


struct NeuronMorphoInfo
{
    bool b_valid;
    double total_length;
    V3DLONG n_node;
    V3DLONG n_segment; //091009 RZC
    V3DLONG n_branch;
    V3DLONG n_tip;  //091027 RZC
    double bbox_xmin, bbox_xmax, bbox_ymin, bbox_ymax, bbox_zmin, bbox_zmax;
    double moments[13];
    NeuronMorphoInfo()	{b_valid=false;}
    bool isValid() {return b_valid;}
};

NeuronMorphoInfo neuron_morpho_features(const NeuronTree *p); //collect the morphological features of a neuron
QString get_neuron_morpho_features_str(const NeuronTree *p);


#endif // NEURON_MATCHING_TOTAL_DISTANCE_H
