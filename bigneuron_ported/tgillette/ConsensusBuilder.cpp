/*
 *  ConsensusBuilder.cpp
 *  BigNeuron Consensus Builder
 *
 *  Created by Todd Gillette on 5/26/15. Uses code from BlastNeuron.
 *  Copyright (c) 2015 Todd Gillette. All rights reserved.
 *
 */

#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <cstdio>
#include <string>
#include <set>
#include <unordered_set>
#include "Reconstruction.h"
#include "composite.h"
#include "tree_matching/swc_utils.h"
#include "tree_matching/neuron_tree_align.h"
#include "logger.h"

#include "ConsensusBuilder.h"

// [HOOK] is where a before, after, or replace function can be called from the action map

/* ConsensusBuilder constructors load reconstructions via files or provided Reconstruction objects */

Logger * logger = new Logger(logDEBUG2);

// Initialize empty ConsenusBuilder (requires adding of reconstructions)
ConsensusBuilder::ConsensusBuilder(){
    ready_to_run = false;
    logger->set_write_level(true);
    Composite::set_logger(logger);
};

pair<string,string> split_final(std::string &filename, const char &delim='.') {
    size_t pos = filename.find_last_of(delim);
    if (pos == 0){
        return std::pair<string,string>(filename,"");
    }else{
        return std::pair<string,string>(filename.substr(0,pos),filename.substr(pos+1));
    }
}

//ConsensusBuilder::ConsensusBuilder(DIR *directory){
ConsensusBuilder::ConsensusBuilder(string &directory){
    ready_to_run = false;
    logger->set_write_level(true);
    Composite::set_logger(logger);

    struct dirent *ep;
    logger->debug("opening directory");
    DIR * dir = opendir(directory.c_str());
    if (dir){
        logger->debug("directory exists, going through");
        unsigned char isFile =0x8;
        while ((ep = readdir(dir)) != NULL){
            logger->debug("File char 0 %c",ep->d_name[0]);
            if (ep->d_type == DT_REG && ep->d_name[0] != '.'){
                logger->debug("Processing file");
                string filename = string(ep->d_name);

                // Check file extension
                pair<string,string> name_ext = split_final(filename);
                string name = name_ext.first, extension = name_ext.second;

                if (name.length() > 0 && (extension.compare("swc") == 0 || extension.compare("SWC") == 0)){
                    printf("%s\n",ep->d_name);
                    logger->debug2("extension %s",extension.c_str());
                    string filepath = directory;
//                    filepath.append("/");
                    filepath.append(filename);

                    //logger->debug2("Adding reconstruction from file %s to list", filepath.c_str());
                    // Read neuron points
                    vector<MyMarker *> neuronPts = readSWC_file(directory+string("/")+filename);
                    
                    if (neuronPts.size() > 0){
                        logger->debug2("%d neuron points",neuronPts.size());
                        NeuronSegment * neuron;
                        vector<NeuronSegment*> segments;

                        // Convert points into segments
                        swc_to_segments(neuronPts, segments);

                        neuron = segments[segments.size()-1]; // #CHECK: Get root, which should be last segments

                        // Create reconstruction
                        Reconstruction * reconstruction = new Reconstruction(name, neuron);

                        cb_reconstructions.insert(std::pair<string, Reconstruction *>(name, reconstruction));
                    }
                }
            }
            logger->debug("End of loop");
        }
        logger->debug("Done loop");
        
        (void) closedir (dir);
    }else{
        logger->error("Null directory provided");
    }

    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }else{
        logger->error("Not enough reconstructions");
        // #TODO: warning message saying that there are only n swc files in the directory, not enough to run the ConsensusBuilder
    }
};

// #TODO: Load reconstructions in anoFile
ConsensusBuilder::ConsensusBuilder(FILE * anoFile){
    
    
    
    //ready_to_run = true;
};

void ConsensusBuilder::clear(){
    cb_reconstructions.clear();
    composite = new Composite();
}
void ConsensusBuilder::set_reconstructions(vector<NeuronSegment *> reconstruction_root_segments){
    for (NeuronSegment * segment : reconstruction_root_segments){
        Reconstruction * reconstruction = new Reconstruction(segment);
        cb_reconstructions.insert(ReconstructionPair(reconstruction->get_name(),reconstruction));
    }
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }
};
void ConsensusBuilder::set_reconstructions(vector<Reconstruction *> reconstructions){
    for (Reconstruction * reconstruction : reconstructions){
        cb_reconstructions.insert(ReconstructionPair(reconstruction->get_name(),reconstruction));
    }
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }
};
void ConsensusBuilder::set_reconstructions(std::map<string, Reconstruction *> reconstructions){
    cb_reconstructions = reconstructions;
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }
};
void ConsensusBuilder::add_reconstruction(NeuronSegment * reconstruction_root_segment, string name, double confidence){
    Reconstruction * reconstruction = new Reconstruction(name, reconstruction_root_segment, confidence);
    cb_reconstructions.insert(ReconstructionPair(reconstruction->get_name(),reconstruction));
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }
};
void ConsensusBuilder::add_reconstruction(Reconstruction * reconstruction){
    cb_reconstructions.insert(ReconstructionPair(reconstruction->get_name(),reconstruction));
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
    }
};
/*
script_map hook_map;
void ConsensusBuilder::register_action(HOOK_NAME hook_name, HOOK_POSITION position, ScriptFunction func){
    ActionHook hook(hook_name, position);
    hook_map[hook] = func;
}*/
NeuronSegment * ConsensusBuilder::build_consensus()
{
    logger->info("Ready to run? %d ",ready_to_run);
    if (!ready_to_run){
        // #TODO: Provide some message saying that the ConsensusBuilder needs at least 2 reconstructions to run
        return NULL;
    }
    
    // Detect potential overlaps and break into subsegments (for each reconstruction) [HOOK to better detect cases for specific cell types?]
    /*
    for (pair<string, Reconstruction> mapPairs : reconstructions){

    }*/
    
    // Create branch composite, looping through reconstructions
    logger->debug2("About to loop through reconstructions");
    int reconCount = 0;
    Reconstruction * reconstruction;
    string name;
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        logger->info("Running reconstruction %d",reconCount);
        name = map_pairs.first;
        reconstruction = map_pairs.second;
        //logger->info("Reconstruction name %s",reconstruction->get_name().c_str());
        printf("Reconstruction name %s\n",reconstruction->get_name().c_str());
        std::vector<NeuronSegment *> reconstruction_segments = reconstruction->get_segments();
        std::set<NeuronSegment *> reconstruction_segments_set(reconstruction_segments.begin(), reconstruction_segments.end());
        logger->debug2("Got recon segments");
        
        reconCount++;
        // Initialize composite as first reconstruction
        if (reconCount == 1){
            composite = new Composite(reconstruction);
            logger->info("Initialized composite using reconstruction with %d segments, composite has %d",reconstruction->get_segments().size(), composite->get_segments().size());
        }else{
            // First run BlastNeuron to align reconstruction with composite and get confident alignment
            std::vector<pair<int, int> > result;
            
            //printf("composite reconstruction %p",composite->get_composite_reconstruction());
            //std::vector<NeuronSegment *> composite_segments = composite->get_composite_reconstruction()->get_segments();
            std::vector<NeuronSegment *> composite_segments = composite->get_segments();
            logger->debug2("Got composite segments");

            std::set<NeuronSegment *> composite_segments_set(composite_segments.begin(), composite_segments.end());
            
            // #TODO: Create new alignment version with a threshold to reject alignment of branches that aren't sufficiently similar spatially

            int nrows = reconstruction_segments.size();
            int ncols = composite_segments.size();

            // Generate rough distances between segments to allow aligner to get close without aligning every pair (afYW)
            std::vector<double> weights(nrows * ncols, 0.0);
            double max_weight = 0;
            logger->debug("Calculating weights %d by %d",nrows,ncols);
            for(int i = 0; i < nrows; i++)
            {
                for(int j = 0; j < ncols; j++)
                {
                    weights[i*ncols + j] = seg_dist(reconstruction_segments[i]->markers, composite_segments[j]->markers);
                    if (weights[i*ncols + j] >  max_weight)
                        max_weight = weights[i*ncols + j];
                }
            }

            for (int i=0;i<nrows*ncols; i++)
                weights[i] = max_weight - weights[i];

            logger->debug("Calculated weights");
            /**
             * Align composite and reconstruction (in neuron_tree_align.h) 
             **/
            logger->debug("Running alignment");
            neuron_tree_align(reconstruction_segments, composite_segments, weights, result);
            // 'result' is a vector of segment index pairs from reconstructionSegments and compositeSegments respectively
            
            logger->debug("Ran alignment, num branches: %d and %d",reconstruction_segments.size(),composite_segments.size());
            
            // Put all branches into set of unmatched branches, then remove those that are matched from the list
            pair<std::set<BranchContainer *>,std::set<CompositeBranchContainer *> > unmatched_branches;
            for (NeuronSegment * segment : reconstruction_segments){
                BranchContainer * branch = reconstruction->get_branch_by_segment(segment);
                unmatched_branches.first.insert(branch);
            }
            for (NeuronSegment * segment : composite_segments){
                CompositeBranchContainer * branch = composite->get_branch_by_segment(segment);
                unmatched_branches.second.insert(branch);
            }
            logger->info("num unmatched branches: %d and %d",unmatched_branches.first.size(),unmatched_branches.second.size());
            
            // Combine separate segment matches when a segment in one reconstruction matches more than one segment in the other (afYW)
            std::vector<vector<int> > seg_clusters1, seg_clusters2; // We'll use these to split segments matched to multiple others later
            merge_multi_match(result, seg_clusters1, seg_clusters2);
            
            // Merge multiple matches (afYW); they'll be separated again after curve alignment
            std::vector<pair<std::vector<MyMarker*>, std::vector<MyMarker*> > > pairs_merged;
            logger->debug("cluster num: %d",seg_clusters1.size());
            for (int i=0; i < seg_clusters1.size(); i++)
            {
                std::vector<MyMarker*> new_seg1, new_seg2;
                std::vector<MyMarker*> tmp_seg;
                //the clustered segments should be in increasing order, so just concatinate them
                for (int j=0;j<seg_clusters1[i].size();j++)
                {
                    tmp_seg.clear();
                    NeuronSegment * segment = reconstruction_segments[seg_clusters1[i][j]];
                    tmp_seg = segment->markers;
                    for (int k=0;k<tmp_seg.size();k++){
                        new_seg1.push_back(tmp_seg[k]);
                    }
                    // This branch is matched (tentatively), so remove it from unmatched_branches
                    BranchContainer * branch = reconstruction->get_branch_by_segment(segment);
                    unmatched_branches.first.erase(branch);
                }
                for (int j=0;j<seg_clusters2[i].size();j++)
                {
                    tmp_seg.clear();
                    NeuronSegment * segment = composite_segments[seg_clusters2[i][j]];
                    tmp_seg = segment->markers;
                    //tmp_seg = composite_segments[seg_clusters2[i][j]]->markers;
                    for (int k=0;k<tmp_seg.size();k++){
                        new_seg2.push_back(tmp_seg[k]);
                    }
                    // This branch is matched (tentatively), so remove it from unmatched_branches
                    CompositeBranchContainer * branch = composite->get_branch_by_segment(segment);
                    unmatched_branches.second.erase(branch);
                }
                if (new_seg2.size() == 0){
                    logger->error("0 length new_seg2; seg_clusters2 size %d",seg_clusters2[i].size());
                    if (seg_clusters2[i].size() > 0){
                        NeuronSegment * segment = composite_segments[seg_clusters2[i][0]];
                        logger->error("seg_clusters2[i][0]: %d",seg_clusters2[i][0]);
                        logger->error("composite_segments size: %d",composite_segments.size());
                        logger->error("tmp_seg size %d",segment->markers.size());
                    }
                }
                pairs_merged.push_back(pair<std::vector<MyMarker*>, std::vector<MyMarker*> >(new_seg1, new_seg2));
            }
            
            /**
             *  Get curve alignments and segment match scores (afYW)
             *  Eliminate any matches that aren't close enough (TG)
             *  Create weighted average (TG)
             *  Find locations to split on either tree for a given merged segment (when seg_clusters[i].size() > 1)
             **/
            logger->info("Loop through pairs to align, average, and split if necessary");
            std::vector<std::map<MyMarker *,MyMarker *> > segment_maps;
            for (int i=0;i<pairs_merged.size();i++) // Loop through aligned segment pairs
            {
                std::vector<pair<int, int> > seg_res;
                std::vector<MyMarker*> seg1 = pairs_merged[i].first;
                std::vector<MyMarker*> seg2 = pairs_merged[i].second;
                std::vector<MyMarker*> average;
                
                // #CONSIDER: return alignment distances and take score as median, max, or 95th% distance
                logger->debug1("seg1 and 2 size: %d %d",seg1.size(),seg2.size());
                logger->debug1("Aligning seg1 %d %d %d; seg2 %d %d %d",seg1[0]->x,seg1[0]->y,seg1[0]->z,seg2[0]->x,seg2[0]->y,seg2[0]->z);
                /* Run curve alignment */
                double normed_align_score = seg_weight(seg1, seg2, seg_res) / seg_res.size();
                logger->debug1("seg_res size %d",seg_res.size());
                logger->debug1("detect below threshold, score %f; threshold %f",normed_align_score,match_score_threshold);
                
                /* Eliminate matched pair if the normed_align_score falls below threshold */
                if (normed_align_score > match_score_threshold){
                    // Add segments to "unmatched" list
                    logger->debug3("test1");
                    for (int j = 0; j < seg_clusters1.size(); j++){
                        BranchContainer * branch = reconstruction->get_branch_by_segment(reconstruction_segments[seg_clusters1[i][j]]);
                        unmatched_branches.first.insert(branch);
                    }
                    logger->debug3("test2");
                    for (int j = 0; j < seg_clusters2.size(); j++){
                        CompositeBranchContainer * composite_branch = composite->get_branch_by_segment(composite_segments[seg_clusters2[i][j]]);
                        unmatched_branches.second.insert(composite_branch);
                    }
                    logger->debug3("test3");
                    // If this pair has an above-threshold score, skip the rest of the procedure
                    continue;
                }

                // For determining where (and if) to split merged segments and its aligned segment:
                // Create MyMarker sets, each containing markers from a given single segment - needed because alignment may not point to marker at the end or beginning of either segment in the merge
                logger->debug1("Determine split");
                bool recon_multi_seg = false;
                std::vector<std::set<MyMarker *> > r_segment_sets;
                std::set<MyMarker *> * marker_set;
                //std::vector<std::unordered_set<MyMarker *> > r_segment_sets;
                //std::unordered_set<MyMarker *> * marker_set;
                if (seg_clusters1[i].size() > 1){
                    recon_multi_seg = true;
                    for (int j=0; j<seg_clusters1[i].size(); j++){
                        //r_segment_sets.push_back(std::unordered_set<MyMarker *>());
                        r_segment_sets.push_back(std::set<MyMarker *>());
                        marker_set = &r_segment_sets.back();
                        for (MyMarker * mark : reconstruction_segments[seg_clusters1[i][j]]->markers){
                            marker_set->insert(mark);
                        }
                    }
                }
                bool composite_multi_seg = false;
//                std::vector<std::unordered_set<MyMarker *> > c_segment_sets;
                std::vector<std::set<MyMarker *> > c_segment_sets;
                if (seg_clusters2[i].size() > 1){
                    composite_multi_seg = true;
                    for (int j=0; j<seg_clusters2[i].size(); j++){
                        //c_segment_sets.push_back(std::unordered_set<MyMarker *>());
                        c_segment_sets.push_back(std::set<MyMarker *>());
                        marker_set = &c_segment_sets.back();
                        for (MyMarker * mark : composite_segments[seg_clusters2[i][j]]->markers){
                            marker_set->insert(mark);
                        }
                    }
                }
                bool final_process_recon = composite_multi_seg;
                
                // Set up weighting between reconstruction and composite weights based on first (or only) branch container
                logger->debug2("Set up weighting");
                int r_seg_num = 0, c_seg_num = 0;
                NeuronSegment * current_r_seg = reconstruction_segments[seg_clusters1[i][r_seg_num]];
                NeuronSegment * current_c_seg = composite_segments[seg_clusters2[i][c_seg_num]];
                BranchContainer * recon_branch = reconstruction->get_branch_by_segment(current_r_seg);
                CompositeBranchContainer * composite_branch = composite->get_branch_by_segment(current_c_seg);
                double reconstruction_weight = recon_branch->get_confidence();
                double composite_weight = composite_branch->get_confidence();
                double combined_weight = reconstruction_weight + composite_weight;

                // Create averaged segment to replace current composite, and determine segment split points
                map<MyMarker*, MyMarker*> map_segment;
                NeuronSegment * new_composite_segment = new NeuronSegment(), * new_recon_segment = new NeuronSegment();
                MyMarker *new_marker, *r_marker, *c_marker;
                vector<NeuronSegment *> children;
                double x, y, z;
                BranchContainer * new_recon_branch = recon_branch;
                CompositeBranchContainer * new_composite_branch = composite_branch;
                std::unordered_set<MyMarker *>::iterator marker_it;
                logger->debug1("Loop through aligned point pairs");
                for (int j=0;j<seg_res.size();j++) // Loop through aligned point pairs
                {
                    r_marker = seg1[seg_res[j].first];
                    c_marker = seg2[seg_res[j].second];

                    // Checking whether either composite or reconstruction are at a new branch - since they are of different data types we can't run the same code twice with the parameter order flipped
                    
                    // Check whether we are in a new segment in either branch and recalculate weights
                    if (recon_multi_seg && r_segment_sets[r_seg_num].find(r_marker) == r_segment_sets[r_seg_num].end()){
                        // Next reconstruction segment: Split composite branch and segment and replace composite segment
                        
                        // Insert composite branch above current composite branch, markers based on what was being traversed (last branch only needs to have markers replaced)

                        // Split all reconstruction branches that match the current composite branch
                        CompositeBranchContainer * new_above_branch = split_composite_branch(recon_branch, composite_branch, new_composite_segment);
                        composite->update_branch_by_segment(new_composite_segment, new_above_branch);
                        composite->add_branch(new_above_branch);
                        new_composite_segment = new NeuronSegment();                            // Reinitialize next segment markers
                        
                        // Update reconstruction branch with new match and segment markers
                        recon_branch->set_composite_match(new_above_branch);               // Composite branch match
                        recon_branch->get_segment()->markers = new_recon_segment->markers;   // Segment markers #CAUTION: Should we be copying the markers vector or the marker pointers?
                        
                        // Start up new reconstruction branches and segments
                        r_seg_num++;
                        new_recon_segment = new NeuronSegment();                                // Reinitialize next segment markers
                        current_r_seg = composite_segments[seg_clusters2[i][r_seg_num]];        // Next composite segment
                        recon_branch = reconstruction->get_branch_by_segment(current_r_seg);     // CompositeBranch of segment
                        reconstruction_weight = recon_branch->get_confidence();
                        combined_weight = reconstruction_weight + composite_weight;             // Update weight
                        
                        // Whether or not a final processing step is needed depends on whether the recon segment was the last to force a split
                        final_process_recon = false;
                    }
                    if (composite_multi_seg && c_segment_sets[c_seg_num].find(c_marker) == c_segment_sets[c_seg_num].end()){
                        // Next composite segment: Split reconstruction branch and segment and replace composite segment

                        // Insert reconstruction branch above current reconstruction branch, markers based on what was being traversed (last branch only needs to have markers replaced)
                        // Create and insert new reconstruction branch container with new segment
                        BranchContainer * branch_parent = recon_branch->get_parent();
                        branch_parent->remove_child(recon_branch); // Remove branch which will go below newly inserted branch (includes segment child_list update)
                        new_recon_branch = new BranchContainer(reconstruction, new_recon_segment, branch_parent, composite_branch, recon_branch->get_confidence());
                        reconstruction->update_branch_by_segment(new_recon_segment, new_recon_branch);
                        // Reconnect recon_branch to newly inserted branch
                        recon_branch->set_parent(new_recon_branch);
                        // Reinitialize next segment markers
                        new_recon_segment = new NeuronSegment();
                        
                        // Update composite branch with new match and segment markers
                        composite_branch->add_branch_match(new_recon_branch);                     // Reconstruction branch match
                        composite_branch->get_segment()->markers = new_composite_segment->markers;  // Segment markers #CAUTION: Should we be copying the markers vector or the marker pointers?
                        
                        // Start up new composite branches and segments
                        c_seg_num++;
                        new_composite_segment = new NeuronSegment();                  // Reinitialize next segment markers (#DEBUG: make sure this doesn't clear prior assignment to composite_branch->get_segment()->markers)
                        current_c_seg = composite_segments[seg_clusters2[i][c_seg_num]];        // Next composite segment
                        composite_branch = composite->get_branch_by_segment(current_c_seg);   // CompositeBranch of segment
                        composite_weight = composite_branch->get_confidence();
                        combined_weight = reconstruction_weight + composite_weight;             // Update weight
                        
                        // Whether or not a final processing step is needed depends on whether the recon segment was the last to force a split
                        final_process_recon = true;
                    }

                    // Update the current new reconstruction segment with the current marker
                    new_recon_segment->markers.push_back(r_marker);

                    // Weighted average of seg1 and seg2
                    x = (r_marker->x * reconstruction_weight + c_marker->x * composite_weight)/combined_weight;
                    y = (r_marker->y * reconstruction_weight + c_marker->y * composite_weight)/combined_weight;
                    z = (r_marker->z * reconstruction_weight + c_marker->z * composite_weight)/combined_weight;
                    new_marker = new MyMarker(x,y,z);
                    new_marker->radius = (r_marker->radius * reconstruction_weight + c_marker->radius * composite_weight)/combined_weight;
                    
                    // Add the updated consensus
                    new_composite_segment->markers.push_back(new_marker);

                    
                    // Mapping from marker in first to marker in second
                    // #CONSIDER: I don't think I need this for anything, was used in BlastNeuron to generate swc link from consensus points to points from either reconstruction
                    map_segment[seg1[seg_res[j].first]] = seg2[seg_res[j].second];
                }
                
                // Put marker->marker map for this match into segment_maps
                segment_maps.push_back(map_segment);
                
                // Update final segment markers if the composite segment was composed of multiple merged segments
                if (final_process_recon){
                    recon_branch->get_segment()->markers = new_recon_segment->markers;
                }
                // Composite gets processed no matter what since it has to update its path;
                // #CAUTION: Do the marker pointers themselves need to be copied over, or the new segment?
                composite_branch->get_segment()->markers = new_composite_segment->markers;
            }
            
            /* Add branches to composite and update composite branch confidence_denominators */
            std::set<BranchContainer *> handled_branches;
            // First calculate summed confidence of reconstructions that have already gone
            logger->debug2("calculated summed confidence of reconstructions");
            double prev_summed_confidence = 0;
            for (Reconstruction * prev_recon : composite->get_reconstructions()){
                prev_summed_confidence += prev_recon->get_confidence();
            }
            // Go through all branches in the list
            logger->debug2("Go through all branches");
            for (BranchContainer * branch : unmatched_branches.first){
                // Some branches will be handled as children of previously handled branches, so we can skip them
                logger->debug3("branch hasn't been dealt with? %d",(handled_branches.find(branch) != handled_branches.end()));
                if (handled_branches.find(branch) != handled_branches.end()){
                    BranchContainer * prev_branch;
                    logger->debug3("init test branch and composite match %p %p",branch,branch->get_composite_match());
                    while (branch && !branch->get_composite_match()){
                        prev_branch = branch;
                        branch = branch->get_parent();
                        logger->debug3("test branch and composite match %p %p",branch,branch->get_composite_match());
                    }
                    // Hook last_branch and its descendents onto branch's composite match
                    logger->debug3("Hook last_branch and its descendents onto branch's composite match");
                    std::stack<BranchContainer *> branch_stack;
                    branch_stack.push(prev_branch);
                    while (!branch_stack.empty()){
                        BranchContainer * orphan = branch_stack.top();
                        branch_stack.pop();
                        handled_branches.insert(orphan);
                        
                        // Get parent of current reconstruction branch
                        BranchContainer * branch_parent = branch->get_parent();
                        CompositeBranchContainer * composite_parent = branch_parent->get_composite_match();
                        
                        // Copy segment and create composite branch out of it
                        NeuronSegment * composite_segment = copy_segment_markers(orphan->get_segment());
                        CompositeBranchContainer * composite_branch = new CompositeBranchContainer(composite_segment);
                        composite->add_branch(composite_branch);
                        composite_branch->add_branch_miss(prev_summed_confidence);
                        
                        // Create links between branch and composite branch
                        composite_branch->add_branch_match(orphan);
                        orphan->set_composite_match(composite_branch);
                        
                        // Set composite parent and create a Decision Point
                        composite_branch->set_parent(composite_parent);
                        composite_branch->set_decision_point1(composite_parent->get_decision_point2());
                        composite_branch->get_decision_point1()->add_connection(composite_branch, composite_parent, reconstruction);
                            
                        // Create decision point (which creates the connection)
                        DecisionPoint decision_point;
                        decision_point.add_connection(composite_branch, composite_parent, reconstruction, reconstruction->get_confidence());
                    }
                }
            }
            logger->debug2("Get recon confidence");
            double recon_conf = reconstruction->get_confidence();
            logger->debug3("Add misses to increase denominator");
            logger->debug1("Number of misses: %d",unmatched_branches.second.size());
            for (CompositeBranchContainer * branch : unmatched_branches.second){
                branch->add_branch_miss(recon_conf);
            }
            
            /* Remaining Plans (TODO) */
            // Take remaining subtrees and branches and try to match without hierarchy
                // Find places for possible branch splits
                // Find candidate branches using clustering on some similarity metric - or use node clustering?
                // Curve alignment on branches and combinations of branches (BlastNeuron code)
            
            // Load matches into composite and update composite branch curves
                // If no match, new branch, add to composite [composite->addBranch() - HOOK]
            
                // If new Join Decision Point, add to composite
            
                // If one branch in new reconstruction matches multiple branches in composite, break up new branch
            
                // If one branch in composite matches multiple branches in new reconstruction
                    // Break up composite branch and propogate to all matching reconstructions

                // Update composite branch(es) component positions based on new branch
                    // Weighted average: (New branch position x New branch confidence + Composite branch position x Cumulative confidence) / (Cumulative confidence + New branch confidence)

            // Generate consensus, copy composite to consensus
            logger->debug("Generating intermediary consensus");
            composite->generate_consensus(0); // #TODO: plug new consensus in as base composite structure
        }
 
        // Generate confidence for each branch and eliminate those below a confidence threshold
    
        // Determine connections
    
            // Iterate until no more decision points: Sort decision points by confidence and determine highest confidence connection
    
                // Connect highest probability
    
                // If two possibilities have similar score, copy consensus to run both
    
                // Update possible branches and connections based on new connection and its impact on topology

    }

    return composite->get_root_segment();
}

// Available results, will require calculation for some
    // Final consensus(es)
        // Consensus Reconstruction (SWC)
        // Overall confidence
        // Connection-level confidence
        // Branch-level confidence
        // Branch or point-level spatial variability
        // Level of contribution by each input reconstruction (accuracy?)
        // Level of contribution for each branch and connection can be calculated by request?

CompositeBranchContainer * ConsensusBuilder::split_composite_branch(BranchContainer * splitting_branch, CompositeBranchContainer * branch, NeuronSegment * top_segment){
    CompositeBranchContainer * new_above = new CompositeBranchContainer(top_segment);
    
    // Create and insert new reconstruction branch container with new segment
    CompositeBranchContainer * branch_parent = branch->get_parent();
    branch_parent->remove_child(branch);
    new_above->set_parent(branch_parent);

    split_branch_matches(splitting_branch, branch, new_above);
    
    return new_above;
}

void ConsensusBuilder::split_branch_matches(BranchContainer * splitting_branch, CompositeBranchContainer * branch, CompositeBranchContainer * new_above){
    vector<BranchContainer *> matches = branch->get_branch_matches();
    BranchContainer *new_match, *parent;

    // Update branch decision points
    DecisionPoint new_dp, *prev_dp = branch->get_decision_point1(); // The new decision point to go between the split (between 'new_above' and 'branch')
    branch->set_decision_point1(&new_dp);
    new_above->set_decision_point1(prev_dp);
    new_above->set_decision_point2(&new_dp);
    
    // Split matches and update decision points accordingly
    for (BranchContainer * match : matches){
        // Insert new branch between current and parent branch to match the new composite branch
        parent = match->get_parent();
        parent->remove_child(match);
        new_match = new BranchContainer(match->get_reconstruction(), match->get_segment(), parent, new_above, match->get_confidence());
        match->set_parent(new_match);
        new_above->add_branch_match(new_match);

        // Decision Points:
        new_dp.add_connection(branch, new_above, match->get_reconstruction(), match->get_confidence());
        for (Connection * connection : prev_dp->get_connections()){
            if (connection->get_child() == branch){
                connection->set_child(new_above);
            }
        }
        new_above->set_decision_point1(branch->get_decision_point1());

        // #CONSIDER: Is there any need to also split up NeuronSegments?
        // For now we won't as it would require pointers/maps between reconstruction and composite markers
    }

}

Composite * ConsensusBuilder::get_composite(){
    return composite;
}
void ConsensusBuilder::set_match_score_threshold(double threshold){
    match_score_threshold = threshold;
}
bool ConsensusBuilder::is_ready_to_run(){
    return ready_to_run;
}