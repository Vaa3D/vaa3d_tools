//
//  CBUtils.h
//  ConsensusBuilder
//
//  Created by Todd Gillette on 11/20/15.
//
//

#ifndef CBUtils_h
#define CBUtils_h

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

class match_ptr_less{
public:
    bool operator()(const Match * lhs, const Match * rhs) const{
        return lhs < rhs;
//        return !lhs ? (rhs ? true : false) : (!rhs ? false : lhs->score < rhs->score ? true : lhs < rhs);
    }
};

class segment_ptr_less{
public:
    segment_ptr_less(){};
    bool operator()(const NeuronSegment * lhs, const NeuronSegment * rhs) const{
        return lhs < rhs;
/*        return !lhs ? (rhs ? true : false) : (!rhs ? false :
                 lhs->child_list.size() < rhs->child_list.size() ? true :
                 lhs->child_list.size() > rhs->child_list.size() ? false :
                 lhs->markers.size() < rhs->markers.size() ? true :
                 lhs->markers.size() > rhs->markers.size() ? false :
                 lhs->markers.size() > 0 && lhs->markers[0]->x < rhs->markers[0]->x ? true :
                 lhs < rhs);*/
    }
};

typedef std::set<NeuronSegment *,segment_ptr_less> SegmentPtrSet;
//typedef std::set<NeuronSegment *> SegmentPtrSet;
typedef std::set<Match *,match_ptr_less> MatchPtrSet;


#endif /* CBUtils_h */
