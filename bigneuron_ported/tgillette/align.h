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
#include "CBUtils.h"
#include "tree_matching/swc_utils.h"

/** Heuristic parameters **/
float const default_alignment_dist_threshold = 3; // microns; this will need to be adjusted
float const default_gap_cost = -0.5; // adjust later

// Number of segments that can be overlapping between two alignments and still both be applied as matches
int const allowable_alignment_overlap = 2;
// Score at which forward direction is accepted and reverse alignment is not attempted [optimal value TBD]
float const default_good_enough_norm_score = default_alignment_dist_threshold/2;
// ? Score difference at which best match is treated as only match [optimal value TBD]
float const default_best_branch_margin = 3 * default_alignment_dist_threshold;


//double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res, float const gap_cost=default_gap_cost);
double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, pair<std::vector<int>, std::vector<int> > & matching_res, float const gap_cost=default_gap_cost);

bool is_consistent_alignment(Match * match, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_consistent_alignment(Match * match, bool check_segment1, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_overlapping_range(int start1, int end1, int start2, int end2);
bool is_range_inside(Match * match1, Match * match2, bool check_segment1);
bool is_range_inside(int start1, int end1, int start2, int end2);
int get_overlap_size(Match * match1, Match * match2, bool check_segment1);
int get_overlap_size(int start1, int end1, int start2, int end2);

double simple_seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res);

#endif /* defined(__ConsensusBuilder__align__) */
