#ifndef NEURON_WEIGHTED_DISTANCE_H
#define NEURON_WEIGHTED_DISTANCE_H


#include "basic_surf_objs.h"

struct NeuronDistSimple
{
    double weighted_dist12_allnodes;//feature-weighted distance of all interpolated points, from neuron 1 to 2
    double weighted_dist21_allnodes; //feature-weighted distance of all interpolated points, from neuron 2 to 1
    double weighted_dist_ave_allnodes; //the average weighted
    double dist_apartnodes; //the average distance of interpolated points that have significant distance (>= 2 pixels)
    double percent_apartnodes; //the percentage of interpolated points that have significant distance (>= 2 pixels)
    double dist_max; //the maximal distance between two neurons, which is defined the smaller one of the two one-directional max distances
    NeuronDistSimple() {weighted_dist12_allnodes = weighted_dist21_allnodes = weighted_dist_ave_allnodes = dist_apartnodes = percent_apartnodes = dist_max = -1; }
};

//round all neuronal node coordinates, and compute the average min distance matches for all places the neurons go through
NeuronDistSimple weighted_neuron_score_rounding_nearest_neighbor(const NeuronTree *p1, const NeuronTree *p2, bool menu);
double weighted_dist_directional_swc_1_2( double & w_dist_12big, double & difference_ratio12,  const NeuronTree *p1, const NeuronTree *p2, double &maxdist);
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


#endif // NEURON_WEIGHTED_DISTANCE_H
