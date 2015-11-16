//
//  align.cpp
//  ConsensusBuilder
//
//  Created by Todd Gillette on 8/11/15.
//
//

#include "align.h"
#include "tree_matching/seg_weight.h"

/**
 * Determine whether a particular candidate match is consistent with all existing matches given by match_map
 **/
bool is_consistent_alignment(Match * match, std::map<NeuronSegment *, std::set<Match *> > match_map){
    return is_consistent_alignment(match, false, match_map) && is_consistent_alignment(match, true, match_map);
}
bool is_consistent_alignment(Match * match, bool check_segment1, std::map<NeuronSegment *, std::set<Match *> > match_map){
    NeuronSegment * seg = check_segment1 ? match->seg1 : match->seg2;
    std::set<Match *> existing_matches = match_map[seg];
    int start1, start2, end1, end2;
    for (Match * existing_match : existing_matches){
        if (check_segment1){
            start1 = match->alignment->front().first;
            end1 = match->alignment->back().first;
            start2 = existing_match->alignment->front().first;
            end2 = existing_match->alignment->back().first;
        }else{
            start1 = match->alignment->front().second;
            end1 = match->alignment->back().second;
            start2 = existing_match->alignment->front().second;
            end2 = existing_match->alignment->back().second;
        }
        if (is_overlapping_range(start1, end1, start2, end2)){
            return false;
        }
    }
    return true;
}
bool is_overlapping_range(int start1, int end1, int start2, int end2){
    return !(end1 < start2 || end2 < start1);
}
int get_overlap_size(Match * match1, Match * match2, bool check_segment1){
    int start1, start2, end1, end2;
    if (check_segment1){
        start1 = match1->alignment->front().first;
        end1 = match1->alignment->back().first;
        start2 = match2->alignment->front().first;
        end2 = match2->alignment->back().first;
    }else{
        start1 = match1->alignment->front().second;
        end1 = match1->alignment->back().second;
        start2 = match2->alignment->front().second;
        end2 = match2->alignment->back().second;
    }
    return get_overlap_size(start1, end1, start2, end2);
}
int get_overlap_size(int start1, int end1, int start2, int end2){
    // No overlap
    if (end1 < start2 || end2 < start1) return 0;

    int len1 = end1 - start1;
    int len2 = end2 - start2;
    
    // Full overlap, one alignment between the ends of the other
    if (len1 > len2){
        if (end1 > end2 && start1 < start2) return len2;
    }else if (len2 > len1){
        if (end2 > end1 && start2 < start1) return len1;
    }
    
    // Partial overlaps
    // alignment1 ends within alignment2, get overlap while checking whether alignment1 is entirely within alignment2
    return end1 > start2 && end1 < end2 ? min(end1-start1, end1-start2) :
    // alignment1 starts within alignment2, otherwise they don't overlap at all
        start1 > start2 && start1 < end2 ? end2 - start1 : -1; // -1 is an error, should NEVER happen
}

/*
// Determine which other matches conflict with the given match by having an overlapping alignment
std::set<Match *> get_alignment_conflicts(Match * match, bool check_segment1, std::map<NeuronSegment *, std::set<Match *> > match_map){
    NeuronSegment * seg = check_segment1 ? match->seg1 : match->seg2;
    std::set<Match *> existing_matches = match_map[seg];
    std::set<Match *> conflicts;
    for (Match * existing_match : existing_matches){
        int start1, start2, end1, end2;
        if (check_first){
            start1 = match->alignment->front().first;
            end1 = match->alignment->back().first;
            start2 = existing_match->alignment->front().first;
            end2 = existing_match->alignment->back().first;
        }else{
            start1 = match->alignment->front().second;
            end1 = match->alignment->back().second;
            start2 = existing_match->alignment->front().second;
            end2 = existing_match->alignment->back().second;
        }
        if (is_overlapping_range(start1, end1, start2, end2)){
            conflicts.add(existing_match);
        }
    }
    return conflicts;
}*/


double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res, float const gap_cost)
{
    if (seg1.size()<=1 || seg2.size()<=1) //single-point branch can map to any branch without constrains
        return 0;
    int k=0, l=0;
    // Dynamic programming matrix
    vector<vector<double> > matrix(seg1.size()+1, vector<double>(seg2.size()+1, 0));
    // Path through matrix to produce point alignment
    vector<vector<pair<int, int> > > last_point(seg1.size(), vector<pair<int, int> >(seg2.size(), pair<int, int>(-1, -1)));
    
    // Consider tracking additional local alignments?
    double best_score = 0, new_score;
    pair<int,int> best_position(-1,-1);
    printf("starting local align seg1 size %i seg2 size %i\n",seg1.size(),seg2.size());
//    matrix[0][0] = euc_dist(seg1, seg2, 0, 1, 0, 1); // Should this just be 0?
    for (int i=1;i<seg1.size()+1;i++)
    {
        for (int j=1;j<seg2.size()+1;j++)
        {
            //printf("%i %i\n",i,j);
            // Gap/skip seg 1
            new_score = matrix[i-1][j] + gap_cost;
            if (new_score > matrix[i][j]){ // new_score could be less than 0
                last_point[i][j].first = i;
                last_point[i][j].second = j-1;
            }
            
            // Gap/skip seg 2
            new_score = matrix[i-1][j] + gap_cost;
            if (new_score > matrix[i][j]){
                matrix[i][j] = new_score;
                last_point[i][j].first = i-1;
                last_point[i][j].second = j;
            }
            
            // Match
            //new_score = matrix[i-1][j-1] + average_dist - euc_dist(seg1, seg2, i-1, i, j-1, j); // For
            //printf("markers %p %p\n",seg1[i-1],seg2[j-1]);
            new_score = matrix[i-1][j-1] + (average_dist - dist(*(seg1[i-1]), *(seg2[j-1])));
            //printf("after dist\n");
            if (new_score > matrix[i][j]){
                matrix[i][j] = new_score;
                last_point[i][j].first = i-1;
                last_point[i][j].second = j-1;
            }
            
            // Keep track of best local alignment
            if (matrix[i][j] > best_score){
                best_score = matrix[i][j];
                best_position.first = i;
                best_position.second = j;
            }
        }
    }
    printf("Done aligning, now on to backtracing\n");
    // Find optimal local alignment
    matching_res.push_back(best_position);
    
    // From global alignment
    //matching_res.push_back(pair<int, int>(seg1.size()-1, seg2.size()-1));
    
    
    int lastp1 = last_point[best_position.first][best_position.second].first;
    int lastp2 = last_point[best_position.first][best_position.second].second;
    int p1, p2;
    do
    {
        p1 = last_point[lastp1][lastp2].first;
        p2 = last_point[lastp1][lastp2].second;
        if (p1==0 || p2==0)
            break;
        matching_res.push_back(pair<int,int>(p1, p2));
        lastp1 = p1;
        lastp2 = p2;
    }
    while (true);
    printf("done backtracing\n");

    //return matrix.back().back();
    return best_score;
};

void split_by_alignment(NeuronSegment * seg1, NeuronSegment * seg2, vector<pair<int,int> > alignment){
    if (alignment[0].first > 0){
        // Split segment 1
        split_segment(seg1, alignment[0].first);
    }
    if (alignment[0].second > 0){
        // Split segment 2
        split_segment(seg2, alignment[0].second);
    }
    
    if (alignment.back().first < seg1->markers.size()-1){
        split_segment(seg1, alignment.back().second);
    }
    if (alignment.back().second < seg2->markers.size()-1){
        split_segment(seg2, alignment.back().second);
    }
    /*
    for (pair<int,int> aligned_pair : alignment){
        
    }
     */
};

void split_segment(NeuronSegment * seg, int pos){
    NeuronSegment * new_seg = new NeuronSegment();
    
}
