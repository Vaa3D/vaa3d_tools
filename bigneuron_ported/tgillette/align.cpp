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
            start1 = match->seg1_start();
            end1 = match->seg1_end();
            start2 = existing_match->seg1_start();
            end2 = existing_match->seg1_end();
        }else{
            start1 = match->seg2_start();
            end1 = match->seg2_end();
            start2 = existing_match->seg2_start();
            end2 = existing_match->seg2_end();
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
bool is_range_inside(Match * match1, Match * match2, bool check_segment1){
    int start1, start2, end1, end2;
    //    printf("get_overlap_size\n");
    if (match1->alignment_size() == 0 || match2->alignment_size() == 0) return false;
    if (check_segment1){
        start1 = match1->seg1_start();
        end1 = match1->seg1_end();
        start2 = match2->seg1_start();
        end2 = match2->seg1_end();
    }else{
        start1 = match1->seg2_start();
        end1 = match1->seg2_end();
        start2 = match2->seg2_start();
        end2 = match2->seg2_end();
    }
    return is_range_inside(start1, end1, start2, end2);
}
bool is_range_inside(int start1, int end1, int start2, int end2){
    return ((start1 > start2 && end1 < end2) || (start2 > start2 && end2 < end1));
}
int get_overlap_size(Match * match1, Match * match2, bool check_segment1){
    int start1, start2, end1, end2;
//    printf("get_overlap_size\n");
    if (match1->alignment_size() == 0 || match2->alignment_size() == 0) return 0;
    if (check_segment1){
        start1 = match1->seg1_start();
        end1 = match1->seg1_end();
        start2 = match2->seg1_start();
        end2 = match2->seg1_end();
    }else{
        start1 = match1->seg2_start();
        end1 = match1->seg2_end();
        start2 = match2->seg2_start();
        end2 = match2->seg2_end();
    }
    return get_overlap_size(start1, end1, start2, end2);
}
int get_overlap_size(int start1, int end1, int start2, int end2){
//    printf("get_overlap_size(start1, end1, start2, end2)\n");
    // No overlap
    if (end1 < start2 || end2 < start1) return 0;

    int len1 = end1 - start1 + 1;
    int len2 = end2 - start2 + 1;
    
    // Full overlap, one alignment between the ends of the other
    if (len1 >= len2){
        if (end1 >= end2 && start1 <= start2) return len2;
    }else if (len2 > len1){
        if (end2 > end1 && start2 < start1) return len1;
    }
    
    // Partial overlaps
    // alignment1 ends within alignment2, get overlap while checking whether alignment1 is entirely within alignment2
    return end1 >= start2 && end1 < end2 ? min(end1-start1+1, end1-start2+1) :
    // alignment1 starts within alignment2, otherwise they don't overlap at all
        start1 > start2 && start1 <= end2 ? end2 - start1 + 1 : -1; // -1 is an error, should NEVER happen
}

//double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res, float const gap_cost)
double local_align(float average_dist, vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, pair<std::vector<int>,std::vector<int> > & matching_res, float const gap_cost)
{
    if (seg1.size() < 1 || seg2.size() < 1) //single-point branch can map to any branch without constrains
        return 0;
    int k=0, l=0;
    // Dynamic programming matrix
    vector<vector<double> > matrix(seg1.size()+1, vector<double>(seg2.size()+1, 0));
    // Path through matrix to produce point alignment
    vector<vector<pair<int, int> > > last_point(seg1.size()+1, vector<pair<int, int> >(seg2.size()+1, pair<int, int>(-1, -1)));
    
    // Consider tracking additional local alignments?
    double best_score = 0, new_score;
    pair<int,int> best_position(-1,-1);
 //   printf("starting local align seg1 size %i seg2 size %i\n",seg1.size(),seg2.size());
//    matrix[0][0] = euc_dist(seg1, seg2, 0, 1, 0, 1); // Should this just be 0?
    for (int i=1;i<seg1.size()+1;i++)
    {
        for (int j=1;j<seg2.size()+1;j++)
        {
            //printf("%i %i\n",i,j);
            // Gap/skip seg 1
            new_score = matrix[i][j-1] + gap_cost;
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
    //printf("Done aligning, now on to backtracing, best score %f at %i %i\n",best_score,best_position.first,best_position.second);
    
    if (best_score == 0){
        return 0;
    }
    
    // From global alignment
    //matching_res.push_back(pair<int, int>(seg1.size()-1, seg2.size()-1));
    
    // Find optimal local alignment
    //matching_res.push_back(pair<int,int>(best_position.first-1,best_position.second-1));
    matching_res.first.push_back(best_position.first-1);
    matching_res.second.push_back(best_position.second-1);
    
    int lastp1 = best_position.first;
    int lastp2 = best_position.second;
    int p1, p2;
    do
    {
        //printf("backtrace pos %i %i\n",lastp1,lastp2);
        p1 = last_point[lastp1][lastp2].first;
        p2 = last_point[lastp1][lastp2].second;
        //printf("next pos %i %i\n",p1,p2);
        if (p1<=0 || p2<=0)
            break;
//        matching_res.push_back(pair<int,int>(p1-1, p2-1)); // Segment indices
        matching_res.first.push_back(p1-1); // Segment indices
        matching_res.second.push_back(p2-1);
        lastp1 = p1;
        lastp2 = p2;
    }
    while (true);
    //printf("done backtracing\n");
    // Reverse alignment so it starts at the beginning of the segment
    //std::reverse(matching_res.begin(),matching_res.end());
    std::reverse(matching_res.first.begin(),matching_res.first.end());
    std::reverse(matching_res.second.begin(),matching_res.second.end());

    //return matrix.back().back();
    return best_score;
};

double simple_seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res)
{
    if (seg1.size()<=1 || seg2.size()<=1) //single-point branch can map to any branch without constrains
        return 0;
    int k=0, l=0;
    vector<vector<double> > matrix(seg1.size(), vector<double>(seg2.size(), MAX_DOUBLE));
    vector<vector<pair<int, int> > > last_point(seg1.size(), vector<pair<int, int> >(seg2.size(), pair<int, int>(-1, -1)));
//    vector<vector<double> > matrix(seg1.size()-1, vector<double>(seg2.size()-1, MAX_DOUBLE));
//    vector<vector<pair<int, int> > > last_point(seg1.size(), vector<pair<int, int> >(seg2.size(), pair<int, int>(-1, -1)));
    double distance, last_dist;
    //matrix[0][0] = euc_dist(seg1, seg2, 0, 1, 0, 1);
    matrix[0][0] = dist(*(seg1[0]), *(seg2[0]));

    for (int i=0;i<seg1.size();i++)
//    for (int i=0;i<seg1.size()-1;i++)
    {
        for (int j=0;j<seg2.size();j++)
//        for (int j=0;j<seg2.size()-1;j++)
        {
           // printf("%i %i\n",i,j);
            k = 1; l = 0;
            if (i-k>=0 && j-l>=0){
                last_dist = matrix[i-k][j-l];
                distance = dist(*(seg1[i]), *(seg2[j]));
//                distance = euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
                if (last_dist + distance <= matrix[i][j])
                {
                    matrix[i][j]  = last_dist + distance;
                    last_point[i][j].first = i-k;
                    last_point[i][j].second = j-l;
                }
            }
            
            k = 0; l = 1;
            if (i-k>=0 && j-l>=0){
                last_dist = matrix[i-k][j-l];
                distance = dist(*(seg1[i]), *(seg2[j]));
                //                distance = euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
                if (last_dist + distance <= matrix[i][j])
                {
                    matrix[i][j]  = last_dist + distance;
                    last_point[i][j].first = i-k;
                    last_point[i][j].second = j-l;
                }
            }
            
            k = 1; l = 1;
            if (i-k>=0 && j-l>=0){
                last_dist = matrix[i-k][j-l];
                distance = dist(*(seg1[i]), *(seg2[j]));
                //                distance = euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
                if (last_dist + distance <= matrix[i][j])
                {
                    matrix[i][j]  = last_dist + distance;
                    last_point[i][j].first = i-k;
                    last_point[i][j].second = j-l;
                }
            }
        }
    }
    matching_res.push_back(pair<int, int>(seg1.size()-1, seg2.size()-1));
    
    
    int lastp1 = seg1.size()-1;
    int lastp2 = seg2.size()-1;
    int p1, p2;
    do
    {
        p1 = last_point[lastp1][lastp2].first;
        p2 = last_point[lastp1][lastp2].second;
        if (p1<0 || p2<0){
            //matching_res.push_back(pair<int,int>(0, 0));
            break;
        }
        matching_res.push_back(pair<int,int>(p1, p2));
        lastp1 = p1;
        lastp2 = p2;
    }
    while (true);
    
    return matrix.back().back();
};

