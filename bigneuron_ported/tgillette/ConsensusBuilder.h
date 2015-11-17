/*
 *  BigNeuron_Consensus_Builder.h
 *  BigNeuron Consensus Builder
 *
 *  Created by Todd Gillette on 5/26/15.
 *  Copyright (c) 2015 Todd Gillette. All rights reserved.
 *
 *
 */

#include <map>
#include <vector>
#include <dirent.h>
#include "Reconstruction.h"
#include "Composite.h"
#include "ActionHook.h"
#include "align.h"
    
/*
 *  #NOTE: Currently no destructors are defined or used, but realistically they
 *  should be employed at least in the process of generating a consenuss where a
 *  composite copy is made.
 */
typedef void (*ScriptFunction)(void); // function pointer type
typedef std::map<ActionHook, ScriptFunction> script_map;
typedef pair<string,Reconstruction*> ReconstructionPair;
//typedef std::set<NeuronSegment *, std::less<NeuronSegment *> > SegmentPtrSet;
typedef std::set<NeuronSegment *> SegmentPtrSet;

const double default_branch_confidence_threshold = 0.25;
const double register_cube_size = 8; // microns
const int marker_sample_rate = 3; // Value determining every n markers to sample to determine bin of segment

class PositionCube{
    int index;
    short quadrant;
    std::vector<double> position;
    std::vector<NeuronSegment *> segments;
};

enum BuilderState {INITIALIZED, RECONSTRUCTIONS_ADDED, PREPROCESSED, BUILT_COMPOSITE};

/**
 * ConsensusBuilder
 *  - Currently runs by incorporating one reconstruction into a single composite at a time.
 *        Some helper objects are crafted to facilitate the future capability of aligning/merging reconstructions
 *        hierarchically, such that there could be multiple composites simultaneously, eventually to be merged
 *        into the final composite. A hierarchical merge might reduce processing of extraneous branches, though
 *        the implemented bin registration method should eliminate a substantial amount of such comparisons.
 **/
class ConsensusBuilder{
    BuilderState builder_state;
    bool ready_to_run = false;
    std::map<string,Reconstruction*> cb_reconstructions;
    Composite * composite;
    
    // The algorithm needs to know what direction the markers go within a segment [Now handled explicitly in the code]
    //bool markers_forward;
    
    // Parameters
    double match_score_threshold;
    double endpoint_threshold_distance;
    float gap_cost;
    float average_alignment_dist;
    void default_parameters();
    
    Logger * logger;
    
    std::set<NeuronSegment *> unmatched_tree1_segments; // From composite (treated as base for alignment, or primary composite)
    std::set<NeuronSegment *> unmatched_tree2_segments; // From reconstruction (or secondary composite to be merged)
    //std::vector<NeuronSegment *> composite_segments, reconstruction_segments; // For use in build_consensus and all its subroutines
    
    /* Cubes determine which segments are near enough to be compared [To be implemented]
        - Use 'register_cube_size' for cube dimensions
        - Cube structure options: 
            A. Set segment in own cube and all (26) surrounding cubes - PREFERRED
            B. Build two sets of cubes which are offset relative to each other
        - Two options for creating cubes:
            A. Expand as needed
                1. Go through each segment in all reconstructions
                2. the first point defining the center of the first cube
                3. if a point is not in the same cube as the last was, check whether need to create new cube by quadrant and location
            B. Process all points to determine number of cubes
                1. Process all points to get smallest and largest x, y, and z values
                2. Create array for each dimension containing the position at the start of the cube (in the given dimension)
                3. Use binary search for a given branch position on each dimension array to find cube
                4. Skip certain number of markers in segment?
     
     Better to refer to actual cubes or to their ids? Can assign vector of PositionCubes
*/
    /** Members for binning segments **/
    std::vector<int> x_bin_positions;
    std::vector<int> y_bin_positions;
    std::vector<int> z_bin_positions;
    int total_bins;
    // For each reconstruction, the segments that are found within each cube
    std::map<Reconstruction *, std::vector<std::set<NeuronSegment *> > > segments_by_recon_cube;
    // Composite segments found within each cube (Mapping on Composite * for if processing goes hierarchical - there would then be multiple composites at the same time, to eventually be merged together)
    std::map<Composite *, std::vector<std::set<NeuronSegment *> > > segments_by_composite_cube;
    // For each segment, the cubes to search for candidate segments for comparison
    std::map<NeuronSegment *, std::set<int> > search_cubes_by_segment;
    /*
    // For each reconstruction, the segments that are found within each cube
    std::map<Reconstruction *, std::vector<SegmentPtrSet * > * > segments_by_recon_cube;
    // Composite segments found within each cube (Mapping on Composite * for if processing goes hierarchical - there would then be multiple composites at the same time, to eventually be merged together)
    std::map<Composite *, std::vector<SegmentPtrSet * > * > segments_by_composite_cube;
    // For each segment, the cubes to search for candidate segments for comparison
    std::map<NeuronSegment *, std::set<int> * > search_cubes_by_segment;
     */

    /** Methods for binning segments **/
    std::vector<int> indexToXyz(int index);
    int xyzToIndex(std::vector<int> xyz);
    int xyzToIndex(int x_ind, int y_ind, int z_ind);

    void bin_branches();
    //std::vector<NeuronSegment *> * bin_branches(Reconstruction * reconstruction);
    void bin_branches(Reconstruction * reconstruction);
    void bin_branches(Composite * composite);
    void bin_branch(CompositeBranchContainer * branch);
    void unbin_branch(CompositeBranchContainer * branch);
    void unbin_branch(BranchContainer * branch);
    int get_bin(MyMarker * marker);
    SegmentPtrSet get_nearby_segments(NeuronSegment * segment, Composite * in_composite);
    SegmentPtrSet get_nearby_segments(NeuronSegment * segment, Reconstruction * in_reconstruction);
    SegmentPtrSet get_nearby_segments(MyMarker * marker, Composite * in_composite);
    SegmentPtrSet get_nearby_segments(MyMarker * marker, Reconstruction * in_reconstruction);
    
    NeuronSegment * build_consensus_sorted();
    NeuronSegment * build_consensus_hierarchically();
    
    /*** Subroutines of build_composite ***/
    
    void preprocess_reconstructions();
    void split_proximal_branches(Reconstruction * reconstruction, float distance_threshold = 2);
    void split_curved_branches(Reconstruction * reconstruction, float curve_distance_threshold = 3); // Default threshold is 3 microns
    
    /**
     *  Create weighted average for matches
     *  Find locations to split on either merged segment (when segments.size() > 1)
     **/
    void average_and_split_alignments(Reconstruction * reconstruction, std::vector<std::map<MyMarker *,MyMarker *> > &segment_maps, std::vector<pair<int, int> > marker_index_alignments, std::vector<MyMarker*> merged_seg1, std::vector<MyMarker*> merged_seg2, std::vector<NeuronSegment *> segments1, std::vector<NeuronSegment *> segments2);

    /* Assign/match branches that were not matched by BlastNeuron */
    //std::set<Match *> match_remaining_branches(std::set<NeuronSegment *> remaining_segments);
    void match_remaining_branches(Reconstruction * reconstruction);
    
    /* Add missed branches to composite and update composite branch confidence_denominators */
    void incorporate_unassigned_branches(Reconstruction * reconstruction);

    /* Create connections between composite branches based on reconstruction branches, their parents, and their composite matches */
    void create_connections(Reconstruction * reconstruction);

    std::vector<double> calculate_weights(std::vector<NeuronSegment *> tree1_segs, std::vector<NeuronSegment *> tree2_segs, Reconstruction * reconstruction);

    std::vector<Match *> find_matches(NeuronSegment * tree1_root, vector<NeuronSegment *> tree1_segments, NeuronSegment * tree2_root, vector<NeuronSegment *> tree2_segments);
    
    //vector<int> get_cubes(NeuronSegment * segment);
    
    /**
     * Takes segments from tree 1 and 2 that have not already been assigned
     * Returns a set of Match objects
     **/
    //std::set<Match *> find_matches_local_alignment(vector<NeuronSegment *> tree1_segments, Reconstruction * reconstruction, vector<NeuronSegment *> tree2_segments);
    std::set<Match *> find_matches_local_alignment(); // Using member objects unmatched_tree1_segments (and tree2), and for registration bins 'composite'
    
    std::map<NeuronSegment *,std::vector<Match *> > generate_candidate_matches_via_local_align(std::set<NeuronSegment *> tree2_segments);
    
    void find_conflicts(std::set<NeuronSegment *> tree_segments,
                        std::map<NeuronSegment *, vector<Match *> > matches_by_segment,
                        std::map<Match *, std::set<Match *> * > &match_conflicts,
                        std::map<Match *, pair<Match *, int> > &small_overlaps);
    
    
public:
    ConsensusBuilder(int log_level=1);
    //ConsensusBuilder(DIR * directory);
    ConsensusBuilder(std::string &directory, int log_level=1);
    ConsensusBuilder(FILE * anoFile, int log_level=1); // TODO Load specified ano file
    ConsensusBuilder(vector<NeuronSegment*> reconstruction_root_segments, int log_level=1);
    ~ConsensusBuilder();
    
    void clear(); // Clear reconstructions and composite, allowing for a restart
    
    void set_logger(Logger * logger);
    void set_log_level(LogLevel level);
    void set_log_level(int level);

    // Get/set parameters
    double get_match_score_threshold() { return match_score_threshold; };
    double get_endpoint_threshold_distance() { return endpoint_threshold_distance; };
    float get_gap_cost() { return gap_cost; };
    float get_average_alignment_dist() { return average_alignment_dist; };
    void set_match_score_threshold(double match_score_threshold) { this->match_score_threshold = match_score_threshold; };
    void set_endpoint_threshold_distance(double endpoint_threshold_distance) { this->endpoint_threshold_distance = endpoint_threshold_distance; };
    void set_gap_cost(float gap_cost) { this->gap_cost = gap_cost; };
    void set_average_alignment_dist(float average_alignment_dist) { this->average_alignment_dist = average_alignment_dist; };

    // Manage reconstructions
    void set_reconstructions(vector<NeuronSegment*> reconstruction_root_segments);
    void set_reconstructions(vector<Reconstruction*> reconstructions);
    void set_reconstructions(std::map<string,Reconstruction*> reconstructions);
    void add_reconstruction(NeuronSegment* reconstruction_root_segment, string name, double confidence=1);
    void add_reconstruction(Reconstruction* reconstruction);
    // void register_action(HOOK_NAME hook_name, HOOK_POSITION position, ScriptFunction func);
    bool is_ready_to_run();

    /** Primary functions - builds composite and consensus **/
    void build_composite();
    NeuronSegment * build_consensus(); // Calls build_composite()
    NeuronSegment * build_consensus(int branch_vote_threshold);
    NeuronSegment * build_consensus(double branch_confidence_threshold);

    std::vector<Reconstruction*> get_reconstructions();
    Composite * get_composite();
    /*
    CompositeBranchContainer * split_composite_branch(BranchContainer * splitting_branch, CompositeBranchContainer * branch, NeuronSegment * top_segment);
     */
    CompositeBranchContainer * split_branch(CompositeBranchContainer * branch, Match * match, std::vector<CompositeBranchContainer *> &resulting_branches);
    BranchContainer * split_branch(BranchContainer * branch, Match * match, std::vector<BranchContainer *> &resulting_branches);
    void split_branch_matches(BranchContainer * splitting_branch, CompositeBranchContainer * branch, CompositeBranchContainer * child);
};
