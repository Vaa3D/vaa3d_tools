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
#include "tree_matching/swc_utils.h"

struct Match{
    NeuronSegment * seg1;
    NeuronSegment * seg2;
    double score;
    vector<pair<int,int> > alignment;
//    int seg1_start, seg1_end, seg2_start, seg2_end;
    bool forward;
    
    // #Consider: Should the score be normalized by some other length?
    double norm_score(int max_position_score){
        int length1 = alignment.back().first - alignment.front().first;
        int length2 = alignment.back().second - alignment.front().second;
        return score/min(length1,length2)/max_position_score;
    }
    
    int seg1_start(){
        return alignment.front().first;
    }
    int seg1_end(){
        return alignment.back().first;
    }
    int seg2_start(){
        return forward ? alignment.front().second : alignment.back().second;
    }
    int seg2_end(){
        return forward ? alignment.back().second : alignment.front().second;
    }
};

float const default_average_alignment_dist = 5; // microns; this will need to be adjusted
float const default_gap_cost = -0.25; // adjust later
int const min_alignment_size = 4;

/** Heuristic parameters **/
// Score at which forward direction is accepted and reverse alignment is not attempted [optimal value TBD]
float const good_enough_score = 5 * default_average_alignment_dist/2;
// ? Score difference at which best match is treated as only match [optimal value TBD]
float const best_branch_margin = 0.3;
// Number of segments that can be overlapping between two alignments and still both be applied as matches
int const allowable_alignment_overlap = 2;

/*  */
double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res, float const gap_cost=default_gap_cost);

bool is_consistent_alignment(Match * match, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_consistent_alignment(Match * match, bool check_segment1, std::map<NeuronSegment *, std::set<Match *> > match_map);
bool is_overlapping_range(int start1, int end1, int start2, int end2);
int get_overlap_size(Match * match1, Match * match2, bool check_segment1);
int get_overlap_size(int start1, int end1, int start2, int end2);

double simple_seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res);

#endif /* defined(__ConsensusBuilder__align__) */
