//
//  align.h
//  ConsensusBuilder
//
//  Created by Todd Gillette on 8/11/15.
//
//

#ifndef __ConsensusBuilder__align__
#define __ConsensusBuilder__align__

#include <stdio.h>
#include <set>
#include <vector>
#include <array>
#include <math.h>
#include "CBUtils.h"
#include "tree_matching/swc_utils.h"

/** Heuristic parameters **/

/** Alignment parameters **/
double const default_gap_cost = -0.25; // adjust later
double const default_euclidean_dist_threshold = 3; // microns; this will need to be adjusted
double const default_angle_threshold = M_PI/4;
double const default_combined_dist_threshold = default_euclidean_dist_threshold * (1 - cos(default_angle_threshold));
std::array<float,3> const alignment_kernel = {0.25, 0.5, 0.25};

// Number of segments that can be overlapping between two alignments and still both be applied as matches
int const allowable_alignment_overlap = 2;
// Score at which forward direction is accepted and reverse alignment is not attempted [optimal value TBD]
//float const default_good_enough_norm_score = default_alignment_dist_threshold/2 - default_alignment_dist_threshold/12;
// ? Score difference at which best match is treated as only match [optimal value TBD]
float const default_best_branch_margin = 3 * default_euclidean_dist_threshold;


bool apply_kernel(vector<MyMarker *> segment, int index, float kernel[], MyMarker &avg_vect);
MyMarker * calculate_vector(MyMarker *marker, MyMarker *parent=nullptr);
std::vector<MyMarker *> calculate_vectors(vector<MyMarker *> const segment, MyMarker *parent=nullptr);
double angle_dist(MyMarker *m1, MyMarker *m2);
double angle_cos(MyMarker *m1, MyMarker *m2);
double get_combined_distance_normalization(float euclidean_distance_threshold, float angle_threshold);

//double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res, float const gap_cost=default_gap_cost);
double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, pair<std::vector<int>, std::vector<int> > & matching_res, float const gap_cost=default_gap_cost);

// Score given distance between markers
double position_score(MyMarker *marker1, MyMarker *marker2, float euclidean_distance_normalization);
// Score given distance between markers and their direction vectors
double position_and_angle_score(MyMarker *marker1, MyMarker *marker2, MyMarker *vector1, MyMarker *vector2, float combined_distance_normalization=default_combined_dist_threshold, float euclidean_distance_threshold=-1);

//double local_align_position_and_angle(float const average_dist, vector<MyMarker*> const seg1, vector<MyMarker*> const seg2, MyMarker *seg1_parent, MyMarker *seg2_parent, pair<std::vector<int>,std::vector<int> > &matching_res, float const gap_cost);
double local_align_position_and_angle(vector<MyMarker*> const seg1, vector<MyMarker*> const seg2, std::vector<MyMarker*> const vectors1, std::vector<MyMarker*> const vectors2, pair<std::vector<int>,std::vector<int> > &matching_res, float const combined_dist_threshold, float const euclidean_dist_threshold, float const gap_cost);
double local_align_position_and_angle(vector<MyMarker*> const seg1, vector<MyMarker*> const seg2, std::vector<MyMarker*> const vectors1, std::vector<MyMarker*> const vectors2, pair<std::vector<int>,std::vector<int> > &matching_res, std::vector<double> &position_scores, float const combined_dist_threshold, float const euclidean_dist_threshold, float const gap_cost);

bool is_consistent_alignment(Match * match, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_consistent_alignment(Match * match, bool check_segment1, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_overlapping_range(int start1, int end1, int start2, int end2);
bool is_range_inside(Match * match1, Match * match2, bool check_segment1);
bool is_range_inside(int start1, int end1, int start2, int end2);
int get_overlap_size(Match * match1, Match * match2, bool check_segment1);
int get_overlap_size(int start1, int end1, int start2, int end2);

double simple_seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res);

#endif /* defined(__ConsensusBuilder__align__) */
