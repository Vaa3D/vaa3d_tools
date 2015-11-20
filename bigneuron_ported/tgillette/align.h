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
    //vector<pair<int,int> > alignment;
    pair<vector<int>,vector<int> > alignment;
//    int seg1_start, seg1_end, seg2_start, seg2_end;
    bool forward;
    
    // #Consider: Should the score be normalized by some other length?
    double norm_score(int max_position_score){
        //int length1 = alignment.back().first - alignment.front().first;
        //int length2 = alignment.back().second - alignment.front().second;
        int length1 = alignment.first.back() - alignment.first.front();
        int length2 = alignment.second.back() - alignment.second.front();
        return score/min(length1,length2)/max_position_score;
    }
    
    unsigned long alignment_size(){
        return alignment.first.size();
    }
    void erase_from_alignment(int from, int to){
        alignment.first.erase(alignment.first.begin() + from, alignment.first.begin() + to);
        alignment.second.erase(alignment.second.begin() + from, alignment.second.begin() + to);
    }
    int seg1_start(){
        return alignment.first.front();
    }
    int seg1_end(){
        return alignment.first.back();
    }
    int seg2_start(){
        return forward ? alignment.second.front() : alignment.second.back();
    }
    int seg2_end(){
        return forward ? alignment.second.back() : alignment.second.front();
    }
};

float const default_average_alignment_dist = 4; // microns; this will need to be adjusted
float const default_gap_cost = -0.5; // adjust later
int const min_alignment_size = 3;

/** Heuristic parameters **/
// Number of segments that can be overlapping between two alignments and still both be applied as matches
int const allowable_alignment_overlap = 2;
// Score at which forward direction is accepted and reverse alignment is not attempted [optimal value TBD]
float const default_good_enough_score = default_average_alignment_dist*2;
// ? Score difference at which best match is treated as only match [optimal value TBD]
float const default_best_branch_margin = 3 * default_average_alignment_dist;

/*  */
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
