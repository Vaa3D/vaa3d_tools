/*
 *  ConsensusBuilder.cpp
 *  BigNeuron Consensus Builder
 *
 *  Created by Todd Gillette on 5/26/15. Uses code from BlastNeuron.
 *  Copyright (c) 2015 Todd Gillette. All rights reserved.
 *
 */

#include <iostream>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string>
#include <set>
#include <unordered_set>
#include "tree_matching/swc_utils.h"
#include "tree_matching/neuron_tree_align.h"
#include "Reconstruction.h"
#include "Composite.h"
#include "logger.h"

#include "ConsensusBuilder.h"

// [HOOK] is where a before, after, or replace function can be called from the action map

/* ConsensusBuilder constructors load reconstructions via files or provided Reconstruction objects */

// Initialize empty ConsenusBuilder (requires adding of reconstructions)
ConsensusBuilder::ConsensusBuilder(int log_level){
    default_parameters();
    
    logger->set_write_level(true);
    logger->set_level(log_level);
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
ConsensusBuilder::ConsensusBuilder(string &directory, int log_level){
    default_parameters();
    logger->set_write_level(true);
    logger->set_level(log_level);
    Composite::set_logger(logger);

    struct dirent *ep;
    //logger->debug("opening directory");
    DIR * dir = opendir(directory.c_str());
    if (dir){
       // logger->debug("directory exists, going through");
        unsigned char isFile =0x8;
        while ((ep = readdir(dir)) != NULL){
            if (ep->d_type == DT_REG && ep->d_name[0] != '.'){
                //logger->debug("Processing file");
                string filename = string(ep->d_name);

                // Check file extension
                pair<string,string> name_ext = split_final(filename);
                string name = name_ext.first, extension = name_ext.second;

                if (name.length() > 0 && (extension.compare("swc") == 0 || extension.compare("SWC") == 0)){
                    printf("%s\n",ep->d_name);
                    //logger->debug2("extension %s",extension.c_str());
                    string filepath = directory;
//                    filepath.append("/");
                    filepath.append(filename);

                    //logger->debug2("Adding reconstruction from file %s to list", filepath.c_str());
                    // Read neuron points (in proper order)
                    vector<MyMarker *> neuronPts = readSWC_file(directory+string("/")+filename);
                    
                    if (neuronPts.size() > 0){
                        //logger->debug2("%i neuron points",neuronPts.size());
                        NeuronSegment * neuron;
                        vector<NeuronSegment*> segments;

                        // Convert points into segments (reverses segment direction, that is, markers go from end to beginning)
                        swc_to_segments(neuronPts, segments);
                        
                        // Re-Reverse segments back to forward direction
                        for (NeuronSegment * seg : segments){
                            std::reverse(seg->markers.begin(),seg->markers.end());
                        }

                        neuron = segments[segments.size()-1]; // #CHECK: Get root, which should be last segment
                        //logger->debug3("num segments %i, root length %i",segments.size(), neuron->markers.size());
                        //logger->debug3("root segment markers %f %f %f",neuron->markers[0]->x,neuron->markers[0]->y,neuron->markers[0]->z);
                        // Create reconstruction
                        Reconstruction * reconstruction = new Reconstruction(name, neuron);

                        cb_reconstructions[name] = reconstruction;
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
        builder_state = RECONSTRUCTIONS_ADDED;
    }else{
        logger->error("Not enough reconstructions");
        // #TODO: warning message saying that there are only n swc files in the directory, not enough to run the ConsensusBuilder
    }
};

// #TODO: Load reconstructions in anoFile
ConsensusBuilder::ConsensusBuilder(FILE * anoFile, int log_level){
    default_parameters();
    logger->set_level(log_level);

    
    
    //ready_to_run = true;
};

ConsensusBuilder::~ConsensusBuilder(){
    segments_by_recon_cube.clear();
    segments_by_composite_cube.clear();
    search_cubes_by_segment.clear();

    composite->delete_all();
    delete composite;

    for (pair<string, Reconstruction *> recon_pair : cb_reconstructions){
        Reconstruction * recon = recon_pair.second;
        delete recon;
    }
}
void ConsensusBuilder::default_parameters(){
    if (!logger){
        logger = new Logger();
    }
    gap_cost = default_gap_cost;
    average_alignment_dist = default_average_alignment_dist;
    ready_to_run = false;
    builder_state = INITIALIZED;
}

void ConsensusBuilder::clear(){
    cb_reconstructions.clear();
    composite = new Composite();
}
void ConsensusBuilder::set_logger(Logger * logger){
    if (this->logger){
        delete this->logger;
    }
    this->logger = logger;
};
void ConsensusBuilder::set_log_level(LogLevel level){
    if (this->logger){
        this->logger->set_level(level);
    }
};
void ConsensusBuilder::set_log_level(int level){
    if (this->logger){
        this->logger->set_level(level);
    }
};
void ConsensusBuilder::set_reconstructions(vector<NeuronSegment *> reconstruction_root_segments){
    for (NeuronSegment * segment : reconstruction_root_segments){
        Reconstruction * reconstruction = new Reconstruction(segment);
        cb_reconstructions.insert(ReconstructionPair(reconstruction->get_name(),reconstruction));
    }
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
        builder_state = RECONSTRUCTIONS_ADDED;
    }
};
void ConsensusBuilder::set_reconstructions(vector<Reconstruction *> reconstructions){
    for (Reconstruction * reconstruction : reconstructions){
        cb_reconstructions[reconstruction->get_name()] = reconstruction;
    }
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
        builder_state = RECONSTRUCTIONS_ADDED;
    }
};
void ConsensusBuilder::set_reconstructions(std::map<string, Reconstruction *> reconstructions){
    cb_reconstructions = reconstructions;
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
        builder_state = RECONSTRUCTIONS_ADDED;
    }
};
void ConsensusBuilder::add_reconstruction(NeuronSegment * reconstruction_root_segment, string name, double confidence){
    Reconstruction * reconstruction = new Reconstruction(name, reconstruction_root_segment, confidence);
    cb_reconstructions[reconstruction->get_name()] = reconstruction;
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
        builder_state = RECONSTRUCTIONS_ADDED;
    }
};
void ConsensusBuilder::add_reconstruction(Reconstruction * reconstruction){
    cb_reconstructions[reconstruction->get_name()] = reconstruction;
    if (cb_reconstructions.size() > 1){
        ready_to_run = true;
        builder_state = RECONSTRUCTIONS_ADDED;
    }
};

std::vector<double> ConsensusBuilder::calculate_weights(std::vector<NeuronSegment *> tree1_segs, std::vector<NeuronSegment *> tree2_segs, Reconstruction * reconstruction){
/*
    std::set<NeuronSegment *> reconstruction_segments = reconstruction->get_segments();
    std::set<NeuronSegment *> composite_segments = composite->get_segments();
  */
    int nrows = tree1_segs.size();
    int ncols = tree2_segs.size();
    
    // Generate rough distances between segments to allow aligner to get close without aligning every pair (afYW)
    /* CONSIDER: Use spatial cube assignments to determine which segments to calculate distance between;
     - if not in the same cube, no pair of nodes could possibly be near enough (give initial weight of -1, which will convert to final weight of 0) */
    std::vector<double> weights(nrows * ncols, 0.0);
    double max_weight = 0;
    logger->debug("Calculating weights %d by %d",nrows,ncols);
    //for(int i = 0; i < nrows; i++)
    int i = 0;
    for (NeuronSegment * seg1 : tree1_segs)
    {
        int j = 0;
        std::set<NeuronSegment *> nearby_segments = get_nearby_segments(seg1, reconstruction);
        //for(int j = 0; j < ncols; j++)
        for (NeuronSegment * seg2 : tree2_segs)
        {
            //weights[i*ncols + j] = seg_dist(reconstruction_segments[i]->markers, composite_segments[j]->markers);
            
            // Find out whether seg2 is anywhere near seg1 using registration bins
            if (nearby_segments.find(seg2) != nearby_segments.end()){
                weights[i*ncols + j] = seg_dist(seg1->markers, seg2->markers);
                if (weights[i*ncols + j] >  max_weight)
                    max_weight = weights[i*ncols + j];
            }else{
                // Not nearby, so put in sentinel so the final weight can be set to 0
                //logger->debug4("segment too far away");
                weights[i*ncols + j] = -1;
            }
            j++;
        }
        i++;
    }
    
    for (int k=0; k < nrows*ncols; k++)
        weights[k] = (weights[k] == -1) ? 0 : max_weight - weights[k];
    
    logger->debug("Calculated weights");
    return weights;
};

/***
 * Bin branches in the space to reduce potential branch alignments and assignments
 * Methods below determine which bin a marker is in, determine which bins a segment should
 *   be compared against, and pull the segments from a given reconstruction that a given
 *   segment should be compared against.
 **/
void ConsensusBuilder::bin_branches(){
    logger->debug1("Entered ConsensusBuilder::bin_branches()");
    
    pair<int,int> x_range(0,0); // use PI as a sentinel value
    pair<int,int> y_range(0,0);
    pair<int,int> z_range(0,0);
    
    Reconstruction * reconstruction;
    
    // Find range for each axis - defines region boundaries
    bool first = true;
    MyMarker * marker;
    logger->debug2("Finding ranges for registration bins");
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        string name = map_pairs.first;
//        logger->debug3("Reconstruction %s",name.c_str());
        reconstruction = map_pairs.second;
        logger->debug4("%i segments",reconstruction->get_segments().size());
        for (NeuronSegment * segment : reconstruction->get_segments()){
            logger->debug4("%i markers",segment->markers.size());
            for (int i = 0; i < segment->markers.size(); i += 3){
//            for (MyMarker * marker : segment->markers){
                marker = segment->markers[i];
                //printf("%f %f %f\n",marker->x,marker->y,marker->z);
                if (first){
                    x_range.first = (int)floor(marker->x);
                    x_range.second = (int)ceil(marker->x);
                    y_range.first = (int)floor(marker->y);
                    y_range.second = (int)ceil(marker->y);
                    z_range.first = (int)floor(marker->z);
                    z_range.second = (int)ceil(marker->z);
                    first = false;
                }else{
                    if (marker->x < x_range.first) x_range.first = (int)floor(marker->x);
                    if (marker->x > x_range.second) x_range.second = (int)ceil(marker->x);
                    if (marker->y < y_range.first) y_range.first = (int)floor(marker->y);
                    if (marker->y > y_range.second) y_range.second = (int)ceil(marker->y);
                    if (marker->z < z_range.first) z_range.first = (int)floor(marker->z);
                    if (marker->z > z_range.second) z_range.second = (int)ceil(marker->z);
                }
            }
        }
    }

    logger->debug4("Generating position ranges along each axis");
    
    // Create arrays denoting starting location of each cube along a given axis
    logger->debug3("Ranges: %i %i; %i %i; %i %i", x_range.first, x_range.second, y_range.first, y_range.second, z_range.first, z_range.second);
    int x_bins = 1 + (x_range.second - x_range.first) / register_cube_size;
    int y_bins = 1 + (y_range.second - y_range.first) / register_cube_size;
    int z_bins = 1 + (z_range.second - z_range.first) / register_cube_size;
    total_bins = x_bins * y_bins * z_bins;
    logger->info("Bins %i x %i x %i = %i",x_bins,y_bins,z_bins,total_bins);
    
    x_bin_positions.clear();
    y_bin_positions.clear();
    z_bin_positions.clear();
    int pos = x_range.first;
    for (int index = 0; index < x_bins; index++){
        x_bin_positions.push_back(pos);
        pos += register_cube_size;
    }
    pos = y_range.first;
    for (int index = 0; index < y_bins; index++){
        y_bin_positions.push_back(pos);
        pos += register_cube_size;
    }
    pos = z_range.first;
    for (int index = 0; index < z_bins; index++){
        z_bin_positions.push_back(pos);
        pos += register_cube_size;
    }
    
    // Set cube bins for each branch
    segments_by_recon_cube.clear();
    logger->debug2("binning branches for each reconstruction");
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        reconstruction = map_pairs.second;
        bin_branches(reconstruction);
    }
    logger->debug1("Exiting ConsensusBuilder::bin_branches()");
};

void ConsensusBuilder::bin_branches(Reconstruction * reconstruction){
//    if (reconstruction->get_name()
//    logger->debug2("Entering bin_branches for %s",reconstruction->get_name().c_str());
    segments_by_recon_cube[reconstruction] = std::vector<SegmentPtrSet>(total_bins);
    std::vector<SegmentPtrSet> * segments_by_cube = &(segments_by_recon_cube[reconstruction]);
    logger->debug4("generated segments_by_cube, %i total bins",total_bins);

    for (NeuronSegment * segment : reconstruction->get_segments()){
        std::set<int> * search_cubes = &(search_cubes_by_segment[segment]);
        int marker_num = 0;
        for (MyMarker * marker : segment->markers){
            // Sample markers rather than running each one (based on initial marker resample rate and cube size)
//            if (marker_num++ % marker_sample_rate == 0){
                // Determine the cube this marker is in
                int center_cube_index = get_bin(marker);
                logger->debug4("Determined cube index %i",center_cube_index);
                // Convert to x, y, z indices
                std::vector<int> xyz = indexToXyz(center_cube_index);
                // Get all adjacent cubes
                for (int x = std::max(0, xyz[0] - 1); x < std::min((int)x_bin_positions.size(), xyz[0] + 1); x++){
                    for (int y = std::max(0, xyz[1] - 1); y < std::min((int)y_bin_positions.size(), xyz[1] + 1); y++){
                        for (int z = std::max(0, xyz[2] - 1); z < std::min((int)z_bin_positions.size(), xyz[2] + 1); z++){
                            // Store cubes to search for this segment
                            //logger->debug4("Into bin %i %i %i",x, y, z);
                            //logger->debug4("Inserting cube index %i for recon segment %p",xyzToIndex(x,y,z),segment);
                            search_cubes->insert(xyzToIndex(x,y,z));
                        }
                    }
//                }

                // Add this segment to the cube it is in
                segments_by_cube->at(center_cube_index).insert(segment);
                //logger->debug4("inserted segment %p into cube's set", segment);
            }
        }
    }
};

void ConsensusBuilder::bin_branches(Composite * composite){
    segments_by_composite_cube[composite] = std::vector<SegmentPtrSet>(total_bins);
    std::vector<SegmentPtrSet> * segments_by_cube = &(segments_by_composite_cube[composite]);
    logger->debug3("bin_branches(Composite *)");
    
    for (NeuronSegment * segment : composite->get_segments()){
//        logger->debug4("Running segment");
        std::set<int> * search_cubes = &(search_cubes_by_segment[segment]);
//        logger->debug4("Got search cubes; segment %p",segment);
        int marker_num = 0;
        for (MyMarker * marker : segment->markers){
            // Sample markers rather than running each one (based on initial marker resample rate and cube size)
//            if (marker_num++ % marker_sample_rate == 0){
                // Determine the cube this marker is in
                logger->debug4("Running get_bin(marker) on %p",marker);
                int center_cube_index = get_bin(marker);
                if (center_cube_index < 0 || center_cube_index > total_bins){
                    logger->warn("Segment %p has center_cube_index of %i",segment,center_cube_index);
                    logger->warn("Marker position %f %f %f",marker->x,marker->y,marker->z);
                }
                // Convert to x, y, z indices
                std::vector<int> xyz = indexToXyz(center_cube_index);
                logger->debug4("center cube index %i",center_cube_index);
                // Get all adjacent cubes
                for (int x = std::max(0, xyz[0] - 1); x < std::min((int)x_bin_positions.size(), xyz[0] + 1); x++){
                    for (int y = std::max(0, xyz[1] - 1); y < std::min((int)y_bin_positions.size(), xyz[1] + 1); y++){
                        for (int z = std::max(0, xyz[2] - 1); z < std::min((int)z_bin_positions.size(), xyz[2] + 1); z++){
                            // Store cubes to search for this segment
                            //logger->debug4("Inserting cube index %i for composite segment %p",xyzToIndex(x,y,z),segment);
                            search_cubes->insert(xyzToIndex(x,y,z));
                        }
                    }
//                }
                // Add this segment to the cube it is in
                segments_by_cube->at(center_cube_index).insert(segment);
                //logger->debug4("Added %p to center cube; set size %i",segment,segments_by_cube->at(center_cube_index).size());
            }
        }
    }
};

void ConsensusBuilder::bin_branch(CompositeBranchContainer * branch){
    if (segments_by_composite_cube.find(branch->get_composite()) == segments_by_composite_cube.end()){
        segments_by_composite_cube[branch->get_composite()] = std::vector<SegmentPtrSet>(total_bins);
    }
    std::vector<SegmentPtrSet> * composite_segments_by_cube = &(segments_by_composite_cube[branch->get_composite()]);
    NeuronSegment * segment = branch->get_segment();
    std::set<int> * search_cubes = &(search_cubes_by_segment[segment]);
    int marker_num = 0;
    for (MyMarker * marker : segment->markers){
        // Sample markers rather than running each one (based on initial marker resample rate and cube size)
//        if (marker_num++ % marker_sample_rate == 0){
            // Determine the cube this marker is in
            int center_cube_index = get_bin(marker);
            // Convert to x, y, z indices
            std::vector<int> xyz = indexToXyz(center_cube_index);
            // Get all adjacent cubes
            for (int x = std::max(0, xyz[0] - 1); x < std::min((int)x_bin_positions.size(), xyz[0] + 1); x++){
                for (int y = std::max(0, xyz[1] - 1); y < std::min((int)y_bin_positions.size(), xyz[1] + 1); y++){
                    for (int z = std::max(0, xyz[2] - 1); z < std::min((int)z_bin_positions.size(), xyz[2] + 1); z++){
                        // Store cubes to search for this segment
          //              logger->debug4("Inserting cube index %i for segment %p",xyzToIndex(x,y,z),segment);
                        search_cubes->insert(xyzToIndex(x,y,z));
                    }
                }
//            }
            // Add this segment to the cube it is in
         //   logger->debug4("Inserting segment %p into composite cube index %i",segment,center_cube_index);
            composite_segments_by_cube->at(center_cube_index).insert(segment);
        }
    }
};

void ConsensusBuilder::unbin_branch(CompositeBranchContainer * branch){
    std::vector<SegmentPtrSet> composite_segments_by_cube = segments_by_composite_cube[branch->get_composite()];
    NeuronSegment * segment = branch->get_segment();
    std::set<int> cube_inds = search_cubes_by_segment[segment];
    for (int cube_ind : cube_inds){
        composite_segments_by_cube[cube_ind].erase(segment);
    }
    search_cubes_by_segment.erase(segment);
}

void ConsensusBuilder::unbin_branch(BranchContainer * branch){
    std::vector<SegmentPtrSet> segments_by_cube = segments_by_recon_cube[branch->get_reconstruction()];
    NeuronSegment * segment = branch->get_segment();
    std::set<int> cube_inds = search_cubes_by_segment[segment];
    for (int cube_ind : cube_inds){
        segments_by_cube[cube_ind].erase(segment);
    }
    search_cubes_by_segment.erase(segment);
}

int ConsensusBuilder::get_bin(MyMarker * marker){
    std::vector<int>::iterator ind_it;
    ind_it = std::lower_bound(x_bin_positions.begin(), x_bin_positions.end(), marker->x);
    int x_ind = ind_it - x_bin_positions.begin() - 1;
    if (x_ind < 0){
        logger->warn("x_ind = %i",x_ind);
        x_ind = 0;
    }
    ind_it = std::lower_bound(y_bin_positions.begin(), y_bin_positions.end(), marker->y);
    int y_ind = ind_it - y_bin_positions.begin() - 1;
    if (y_ind < 0){
        logger->warn("y_ind = %i",y_ind);
        y_ind = 0;
    }
    ind_it = std::lower_bound(z_bin_positions.begin(), z_bin_positions.end(), marker->z);
    int z_ind = ind_it - z_bin_positions.begin() - 1;
    if (z_ind < 0){
        logger->warn("z_ind = %i",z_ind);
        z_ind = 0;
    }
    
    return x_ind * y_bin_positions.size() * z_bin_positions.size() + y_ind * z_bin_positions.size() + z_ind;
};

std::vector<int> ConsensusBuilder::indexToXyz(int index){
    int per_x = y_bin_positions.size() * z_bin_positions.size();
    int x_ind = index / per_x;
    int remain = index % per_x;
    int y_ind = remain / z_bin_positions.size();
    int z_ind = remain % z_bin_positions.size();
    std::vector<int> xyz;
    xyz.push_back(x_ind);
    xyz.push_back(y_ind);
    xyz.push_back(z_ind);
    return xyz;
};
int ConsensusBuilder::xyzToIndex(std::vector<int> xyz){
    return xyz[0] * y_bin_positions.size() * z_bin_positions.size() + xyz[1] * z_bin_positions.size() + xyz[2];
};
int ConsensusBuilder::xyzToIndex(int x_ind, int y_ind, int z_ind){
    return x_ind * y_bin_positions.size() * z_bin_positions.size() + y_ind * z_bin_positions.size() + z_ind;
};

std::set<NeuronSegment *> ConsensusBuilder::get_nearby_segments(NeuronSegment * segment, std::vector<SegmentPtrSet> segments_by_cube){
    SegmentPtrSet nearby_segments;
    std::set<int> cubes;
    
    // Get all the cubes this segment passes near
    std::set<int> search_cubes = search_cubes_by_segment[segment];
    logger->debug4("%i search cubes for segment %p",search_cubes.size(), segment);
    for (int cube : search_cubes){
//        logger->debug4("Searching cube %i",cube);
        nearby_segments.insert(segments_by_cube[cube].begin(), segments_by_cube[cube].end());
    }
    logger->debug3("Got nearby segments %i",nearby_segments.size());
    return nearby_segments;
};
std::set<NeuronSegment *> ConsensusBuilder::get_nearby_segments(NeuronSegment * segment, Composite * in_composite){
    SegmentPtrSet nearby_segments;
    std::set<int> cubes;
    if (segments_by_composite_cube.find(in_composite) == segments_by_composite_cube.end()){
        logger->warn("A composite did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_composite);
    }
    logger->debug4("continuing to get nearby segments");
    std::vector<SegmentPtrSet> segments_by_cube = segments_by_composite_cube[in_composite];
    
    // Get all the cubes this segment passes near
    std::set<int> search_cubes = search_cubes_by_segment[segment];
    logger->debug4("%i search cubes for segment %p",search_cubes.size(), segment);
    for (int cube : search_cubes){
//        logger->debug4("Searching cube %i",cube);
        nearby_segments.insert(segments_by_cube[cube].begin(), segments_by_cube[cube].end());
    }
    logger->debug3("Got nearby segments %i",nearby_segments.size());
    return nearby_segments;
};
std::set<NeuronSegment *> ConsensusBuilder::get_nearby_segments(NeuronSegment * segment, Reconstruction * in_reconstruction){
    SegmentPtrSet nearby_segments;
    std::set<int> cubes;
    if (segments_by_recon_cube.find(in_reconstruction) == segments_by_recon_cube.end()){
        logger->warn("A reconstruction did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_reconstruction);
    }
    std::vector<SegmentPtrSet> segments_by_cube = segments_by_recon_cube[in_reconstruction];
    
    // Get all the cubes this segment passes near
    std::set<int> search_cubes = search_cubes_by_segment[segment];
    for (int cube : search_cubes){
        nearby_segments.insert(segments_by_cube[cube].begin(), segments_by_cube[cube].end());
    }
    
    return nearby_segments;
};

SegmentPtrSet ConsensusBuilder::get_nearby_segments(MyMarker * marker, Composite * in_composite){
    if (segments_by_composite_cube.find(in_composite) == segments_by_composite_cube.end()){
        logger->warn("A composite did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_composite);
    }
    std::vector<SegmentPtrSet> segments_by_cube = segments_by_composite_cube[in_composite];

    int center_cube = get_bin(marker);
    std::vector<int> xyz = indexToXyz(center_cube);
    
    std::set<int> search_cubes;
    for (int x = std::min(0, xyz[0] - 1); x < std::max((int)x_bin_positions.size(), xyz[0] + 1); x++){
        for (int y = std::min(0, xyz[1] - 1); y < std::max((int)y_bin_positions.size(), xyz[1] + 1); y++){
            for (int z = std::min(0, xyz[2] - 1); z < std::max((int)z_bin_positions.size(), xyz[2] + 1); z++){
                search_cubes.insert(xyzToIndex(x,y,z));
            }
        }
    }
    
    SegmentPtrSet nearby_segments;
    for (int cube : search_cubes){
        nearby_segments.insert(segments_by_cube[cube].begin(), segments_by_cube[cube].end());
    }
    
    return nearby_segments;
};
SegmentPtrSet ConsensusBuilder::get_nearby_segments(MyMarker * marker, Reconstruction * in_reconstruction){
    if (segments_by_recon_cube.find(in_reconstruction) == segments_by_recon_cube.end()){
        logger->warn("A reconstruction did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_reconstruction);
    }

    int center_cube = get_bin(marker);
    std::vector<int> xyz = indexToXyz(center_cube);
    
    std::set<int> search_cubes;
    for (int x = std::min(0, xyz[0] - 1); x < std::max((int)x_bin_positions.size(), xyz[0] + 1); x++){
        for (int y = std::min(0, xyz[1] - 1); y < std::max((int)y_bin_positions.size(), xyz[1] + 1); y++){
            for (int z = std::min(0, xyz[2] - 1); z < std::max((int)z_bin_positions.size(), xyz[2] + 1); z++){
                search_cubes.insert(xyzToIndex(x,y,z));
            }
        }
    }
    
    std::vector<SegmentPtrSet> segments_by_cube = segments_by_recon_cube[in_reconstruction];
    SegmentPtrSet nearby_segments;
    for (int cube : search_cubes){
        nearby_segments.insert(segments_by_cube[cube].begin(), segments_by_cube[cube].end());
    }
    
    return nearby_segments;
};

/*
std::set<NeuronSegment *> ConsensusBuilder::get_nearby_segments(NeuronSegment * segment, Composite * in_composite){
    std::set<NeuronSegment *> nearby_segments;
    std::set<int> cubes;
    std::vector<SegmentPtrSet * > * segments_by_cube = segments_by_composite_cube[in_composite];
    if (!segments_by_cube){
        logger->warn("A composite did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_composite);
        segments_by_cube = segments_by_composite_cube[in_composite];
    }
    
    // Get all the cubes this segment passes near
    std::set<int> * search_cubes = search_cubes_by_segment[segment];
    for (int cube : *search_cubes){
        nearby_segments.insert(segments_by_cube->at(cube)->begin(), segments_by_cube->at(cube)->end());
    }
    
    return nearby_segments;
};
std::set<NeuronSegment *> ConsensusBuilder::get_nearby_segments(NeuronSegment * segment, Reconstruction * in_reconstruction){
    std::set<NeuronSegment *> nearby_segments;
    std::set<int> cubes;
    std::vector<SegmentPtrSet * > * segments_by_cube = segments_by_recon_cube[in_reconstruction];
    if (!segments_by_cube){
        logger->warn("A reconstruction did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_reconstruction);
        segments_by_cube = segments_by_recon_cube[in_reconstruction];
    }
    
    // Get all the cubes this segment passes near
    std::set<int> * search_cubes = search_cubes_by_segment[segment];
    for (int cube : *search_cubes){
        nearby_segments.insert(segments_by_cube->at(cube)->begin(), segments_by_cube->at(cube)->end());
    }
    
    return nearby_segments;
};
SegmentPtrSet ConsensusBuilder::get_nearby_segments(MyMarker * marker, Composite * in_composite){
    SegmentPtrSet nearby_segments;
    
    std::vector<SegmentPtrSet * > * segments_by_cube = segments_by_composite_cube[in_composite];
    if (!segments_by_cube){
        logger->warn("A reconstruction did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_composite);
        segments_by_cube = segments_by_composite_cube[in_composite];
    }
    int center_cube = get_bin(marker);
    std::vector<int> xyz = indexToXyz(center_cube);
    
    std::set<int> search_cubes;
    for (int x = std::min(0, xyz[0] - 1); x < std::max((int)x_bin_positions.size(), xyz[0] + 1); x++){
        for (int y = std::min(0, xyz[1] - 1); y < std::max((int)y_bin_positions.size(), xyz[1] + 1); y++){
            for (int z = std::min(0, xyz[2] - 1); z < std::max((int)z_bin_positions.size(), xyz[2] + 1); z++){
                search_cubes.insert(xyzToIndex(x,y,z));
            }
        }
    }
    
    for (int cube : search_cubes){
        nearby_segments.insert(segments_by_cube->at(cube)->begin(), segments_by_cube->at(cube)->end());
    }
    
    return nearby_segments;
};
SegmentPtrSet ConsensusBuilder::get_nearby_segments(MyMarker * marker, Reconstruction * in_reconstruction){
    SegmentPtrSet nearby_segments;
    
    std::vector<SegmentPtrSet * > * segments_by_cube = segments_by_recon_cube[in_reconstruction];
    if (!segments_by_cube){
        logger->warn("A reconstruction did not have its branches binned prior to accessing nearby_segments()");
        bin_branches(in_reconstruction);
        segments_by_cube = segments_by_recon_cube[in_reconstruction];
    }
    int center_cube = get_bin(marker);
    std::vector<int> xyz = indexToXyz(center_cube);
    
    std::set<int> search_cubes;
    for (int x = std::min(0, xyz[0] - 1); x < std::max((int)x_bin_positions.size(), xyz[0] + 1); x++){
        for (int y = std::min(0, xyz[1] - 1); y < std::max((int)y_bin_positions.size(), xyz[1] + 1); y++){
            for (int z = std::min(0, xyz[2] - 1); z < std::max((int)z_bin_positions.size(), xyz[2] + 1); z++){
                search_cubes.insert(xyzToIndex(x,y,z));
            }
        }
    }
    
    for (int cube : search_cubes){
        nearby_segments.insert(segments_by_cube->at(cube)->begin(), segments_by_cube->at(cube)->end());
    }
    
    return nearby_segments;
};
*/
/**
 * An idea to enable plug-ins to modify or supplement existing code. The intention would be for providing intelligence
 * for weighting reconstructions or their branches, either based on prior success generally or in specific cases.
 * Connection confidence might also be modified given some other local or global features, not necessarily based on the
 * reconstruction sources. Not yet implemented.
 
script_map hook_map;
void ConsensusBuilder::register_action(HOOK_NAME hook_name, HOOK_POSITION position, ScriptFunction func){
    ActionHook hook(hook_name, position);
    hook_map[hook] = func;
}
 */

/***
 * Major public methods followed by their (mostly) private subroutines
 ***/
NeuronSegment * ConsensusBuilder::build_consensus(){
    return build_consensus(default_branch_confidence_threshold);
}
NeuronSegment * ConsensusBuilder::build_consensus(int branch_vote_threshold){
    double conf_threshold = (double)branch_vote_threshold / cb_reconstructions.size();
    return build_consensus(conf_threshold);
}
std::vector<NeuronSegment *> ConsensusBuilder::produce_composite(){
    /* Build composite */
    build_composite();
    
    /* Generate consensus */
    Composite * consensus = composite->generate_consensus(branch_confidence_threshold, true);
    
    return consensus->get_root_segment();
};

std::vector<NeuronSegment *> ConsensusBuilder::get_composite_segments(double branch_confidence_threshold){
    std::vector<NeuronSegment *> segments;
    
    
    
    return segments;
}

NeuronSegment * ConsensusBuilder::build_consensus(double branch_confidence_threshold){
    logger->info("Ready to run? %d ",ready_to_run);
    if (!ready_to_run){
        logger->error("At least 2 reconstructions are required to run ConsensusBuilder");
        return NULL;
    }
    
    /** CONSIDER:
     *  1. Optimal order in which to include reconstructions
     *      - probably best to start with median (use Hanchuan and Xiaoxiao's code based on BlastNeuron) and then work out
     *      - generate a new vector of reconstruction names in the preferred order, then loop through names, taking associated reconstruction from cb_reconstructions
     *  2. Consider merging hierarchically to reduce branch comparisons from relatively different reconstructions
     *      a. generation of hierarchy
     *      b. modified code that runs matching on two composites
     *          - all matching effectively the same, but updating composite would need to be modified
     *          - change from 'composite' and 'reconstruction' to tree1 (always primary composite) and tree2 (reconstruction or composite)
     *          - different methods, or same methods that then detect tree2 type to determine which integrating procedures to call
     **/

    /* Build composite */
    build_composite();

    /* Generate consensus */
    Composite * consensus = composite->generate_consensus(branch_confidence_threshold, true);

    return consensus->get_root_segment();
};

void ConsensusBuilder::preprocess_reconstructions(){
    // Assign branches to bins (spatial cubes) in order to minimize set of possible branch-branch assignments
    bin_branches();

    // Branch cutting for simplifying alignments (prior to assignment of cubes)
    int recon_count = 0;
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        string name = map_pairs.first;
        Reconstruction * reconstruction = map_pairs.second;
        logger->debug("Recon segs %i",reconstruction->get_segments().size());
        
        
        // #TODO: Test the following two functions
        // Split highly curved branches at maximal distance from straight line
        /*
         logger->debug("Cutting branches on reconstruction %i",recon_count);
        split_curved_branches(reconstruction);
        logger->debug2("After split_curved_branches");
        // Split branches where they come very close or pass by another branch (this will make aligning and assigning simpler)
        split_proximal_branches(reconstruction);
        logger->debug2("After split_proximal_branches");
         */
        recon_count++;
    }
    
    // Determine similarity of trees to set order of processing, reducing growth rate of composite
    //sort_trees();
    // ALTERNATIVELY: Generate a binary tree indicating which trees to combine, further reducing extraneous alignment attempts
    //build_hierarchy();
    
    builder_state = PREPROCESSED;
}

void ConsensusBuilder::build_composite(){
    switch(builder_state){
        case BUILT_COMPOSITE:
            logger->info("Composite is already built");
            return;
    }
    if (!ready_to_run()){
        logger->error("Not ready to run");
        return;
    }

    /* Preprocess */
    preprocess_reconstructions();
/*
    // Determine the direction of markers in segments (determined when marker vector is converted into connected segments)
    bool determined_marker_dir = false;
    for (pair<string,Reconstruction *> recon_pair : cb_reconstructions){
        for (NeuronSegment * segment : recon_pair.second->get_segments()){
            if (segment->markers.size() > 1){
                markers_forward = segment->markers[0]->parent != segment->markers[1];
                determined_marker_dir = true;
                break;
            }
        }
        if (determined_marker_dir) break;
    }
  */
    // Create branch composite, looping through reconstructions
    logger->debug2("About to loop through reconstructions");
    int recon_count = 0;
    typedef pair<std::vector<MyMarker*>, std::vector<MyMarker*> > MarkerVectPair;

    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        logger->new_line();
        logger->info("Running reconstruction %d",recon_count);
        string name = map_pairs.first;
        Reconstruction * reconstruction = map_pairs.second;
        //logger->info("Reconstruction name %s",reconstruction->get_name().c_str());
        printf("Reconstruction name %s\n",reconstruction->get_name().c_str());

        std::vector<NeuronSegment *> reconstruction_segments = reconstruction->get_segments_ordered();

        std::set<NeuronSegment *> reconstruction_segments_set(reconstruction_segments.begin(),reconstruction_segments.end());

        logger->debug2("Got recon segments");
        /*
        for (NeuronSegment *seg : reconstruction_segments){
            logger->debug4("seg size %i",seg->markers.size());
        }
        */
        
        recon_count++;
        // Initialize composite as first reconstruction
        if (recon_count == 1){
            composite = new Composite(reconstruction);
            bin_branches(composite);
            logger->info("Initialized composite using reconstruction with %d segments, composite has %d",reconstruction->get_segments().size(), composite->get_segments().size());
        }else{
            if (recon_count > 2){
                // ** Generate consensus **
                logger->debug("Generating intermediary consensus");
                //Composite * new_composite = composite->generate_consensus(0); // #TODO: plug new consensus in as base composite structure
                // Update the NeuronSegment tree structure given the information of the new reconstruction
                composite->update_tree();
                
                logger->debug4("After update_tree; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
                logger->debug4("After update_tree; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());
            }
            // First run BlastNeuron to align reconstruction with composite and get confident alignment
            std::vector<pair<int, int> > result;
            logger->new_line();
            logger->debug4("Beginning of build_composite; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("Middle of build_composite; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

            //printf("composite reconstruction %p",composite->get_composite_reconstruction());
            std::vector<NeuronSegment *> composite_segments = composite->get_segments_ordered();
            logger->debug("Got composite segments %i",composite_segments.size());
            for (NeuronSegment *seg : composite_segments){
                //logger->debug4("seg size %i",seg->markers.size());
                for (int d = 0; d < seg->markers.size(); d++){
                    MyMarker *mrk = seg->markers[d];
                    //logger->debug4("marker %f %f %f",mrk->x,mrk->y,mrk->z);
                }
            }

            // Create a set of segments from the composite
            std::set<NeuronSegment *> composite_segments_set(composite_segments.begin(), composite_segments.end());
            
            // Calculate weights for each branch pair, higher weight is better for aligning
            std::vector<double> weights = calculate_weights(composite_segments, reconstruction_segments, reconstruction);
            
            /**
             * Align composite and reconstruction using code from BlastNeuron (in neuron_tree_align.h)
             **/
            logger->debug("Running alignment, num branches: %d and %d",composite_segments.size(),reconstruction_segments.size());
            //neuron_tree_align(composite_segments, reconstruction_segments, weights, result);
            neuron_tree_align(composite_segments, reconstruction_segments, reconstruction, weights, result);
            // 'result' is a vector of segment index pairs from reconstructionSegments and compositeSegments respectively
            
            logger->debug1("Ran alignment, result ");
            
            // Put all branches into set of unmatched branches, then later remove those that are matched from the list
            unmatched_tree1_segments.clear();
            unmatched_tree1_segments.insert(composite_segments.begin(),composite_segments.end());
            //unmatched_tree2_segments.clear();
            unmatched_tree2_segments = reconstruction_segments_set;
            
            // Combine separate segment matches when a segment in one reconstruction matches more than one segment in the other (afYW)
            std::vector<vector<int> > seg_clusters1, seg_clusters2; // We'll use these to split segments matched to multiple others later
            merge_multi_match(result, seg_clusters1, seg_clusters2); // neuron_tree_align.cpp
            
            // Merge multiple segments into a single segment, one for each side of the match (afYW); they'll be separated again after curve alignment
            std::vector<MarkerVectPair> pairs_merged;
            logger->debug("clusters1 size: %i, cluster2 size: %i",seg_clusters1.size(),seg_clusters2.size());
            for (int i=0; i < seg_clusters1.size(); i++)
            {
                std::vector<MyMarker*> new_seg1, new_seg2;
                
                logger->debug1("i %i; cluster1[i].size %i; cluster2[i].size %i",i,seg_clusters1[i].size(),seg_clusters2[i].size());
                
                //the clustered segments should be in increasing order, so just concatinate them
                for (int j = seg_clusters1[i].size()-1; j >= 0;j--)
                //for (int j = 0; j < seg_clusters1[i].size(); j--)
                {
                    logger->debug4("seg_cluster num %i",seg_clusters1[i][j]);
                    NeuronSegment * segment = composite_segments[seg_clusters1[i][j]];
                    new_seg1.insert(new_seg1.end(), segment->markers.begin(), segment->markers.end());

                    // This branch is matched (tentatively), so remove it from unmatched_branches
                    CompositeBranchContainer * branch = composite->get_branch_by_segment(segment);
                }
                logger->debug4("new_seg markers");

                for (int j = seg_clusters2[i].size()-1; j >= 0;j--)
                //for (int j = 0; j < seg_clusters2[i].size(); j--)
                {
                    NeuronSegment * segment = reconstruction_segments[seg_clusters2[i][j]];
                    new_seg2.insert(new_seg2.end(), segment->markers.begin(), segment->markers.end());
                    
                    // This branch is matched (tentatively), so remove it from unmatched_branches
                    BranchContainer * branch = reconstruction->get_branch_by_segment(segment);
                }
                pairs_merged.push_back(MarkerVectPair(new_seg1, new_seg2));
            }
            
            /**
             *  Get curve alignments and segment match scores (afYW)
             *  Eliminate any matches that aren't close enough (TG)
             **/
            logger->info("Loop through pairs to align, average, and split if necessary");

            // For each merged pair, links tree2 marker to tree1 marker for possible splitting later if necessary [not currently used; consider alternative data structure!]
            std::vector<std::map<MyMarker *,MyMarker *> > segment_maps;
            //std::vector< pair<std::vector<MyMarker*>, std::vector<MyMarker*> >
            
            // seg_clusters1, seg_clusters2: vectors of vectors giving indices for segments in reconstruction_segments and composite_segments
            // pairs_merged: vector of pairs of MyMarker vectors representing combined segments that were jointly matched
            logger->debug("Aligned pairs: %i",pairs_merged.size());
            for (int i=0; i < pairs_merged.size(); i++) // Loop through matched segment pairs
            {
                logger->debug("Checking segment matches index %i",i);
                std::vector<pair<int, int> > marker_index_alignments;
                std::vector<MyMarker*> seg1 = pairs_merged[i].first;
                std::vector<MyMarker*> seg2 = pairs_merged[i].second;
                std::vector<MyMarker*> average;
                
                // #CONSIDER: return alignment distances and take score as median, max, or 95th% distance
                logger->debug1("seg1 and 2 size: %i %i",seg1.size(),seg2.size());
                logger->debug1("Aligning seg1 %f %f %f; seg2 %f %f %f",seg1[0]->x,seg1[0]->y,seg1[0]->z,seg2[0]->x,seg2[0]->y,seg2[0]->z);
                /* Run curve alignment */
                double align_dist = 0;
                double align_score = 0;
                if (abs((int)(seg1.size() - seg2.size())) <= 2){
                    logger->debug("Difference in sequence lengths is too large");
                    align_dist = simple_seg_weight(seg1, seg2, marker_index_alignments);
                    align_score = average_alignment_dist - align_dist/marker_index_alignments.size();
                }
                logger->debug1("marker_index_alignments size %i, align_dist %f",marker_index_alignments.size(), align_dist);
                logger->debug1("normalized score %f, threshold %f",align_score,match_score_threshold);
                if (align_score > match_score_threshold){
                    logger->debug2("Score is good enough");
                    //matching_pairs_merged.insert(pairs_merged[i]);
                    
                    // Generate lists of segments (rather than of segment indices)
                    std::vector<NeuronSegment *> segments1, segments2;
                    for (int seg_ind : seg_clusters1[i]){
                        segments1.push_back(composite_segments[seg_ind]);
                        unmatched_tree1_segments.erase(composite_segments[seg_ind]);
                    }
                    logger->debug("unmatched_tree2_segments before match %i",unmatched_tree2_segments.size());
                    for (int seg_ind : seg_clusters2[i]){
                        segments2.push_back(reconstruction_segments[seg_ind]);
                        unmatched_tree2_segments.erase(reconstruction_segments[seg_ind]);
                    }
                    logger->debug("unmatched_tree2_segments after match %i",unmatched_tree2_segments.size());
                    
                    std::reverse(segments1.begin(),segments1.end());
                    std::reverse(segments2.begin(),segments2.end());

                    // Create weighted average for matches, then split merged segments (when segments.size() > 1)
                    average_and_split_alignments(reconstruction, segment_maps, marker_index_alignments, seg1, seg2, segments1, segments2);
                }
            }

            logger->debug4("After average_and; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("After average_and; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

            logger->debug("Looking for zero-length composite segments before handling unmatched branches");
            for (NeuronSegment * seg : composite->get_segments()){
                if (seg->markers.size() == 0){
                    logger->warn("Found zero-length segments");
                }
            }
            
            /**
             *  Match branches that BlastNeuron could not
             *   - Use local sequence alignment
             **/
            
            // Use local sequence alignment
            logger->debug3("calling 'match_remaining_branches' if there are any in unmatched_tree1_segments - %i and tree2 %i",unmatched_tree1_segments.size(),unmatched_tree2_segments.size());
            if (unmatched_tree1_segments.size() > 0 && unmatched_tree2_segments.size() > 0)
                match_remaining_branches(reconstruction);
            logger->debug3("done matching remaining branches");

            // Add missed branches to composite and update composite branch confidence_denominators
            // If no match, new branch, add to composite [composite->addBranch() - HOOK]
            //incorporate_unassigned_branches(unmatched_branches);
            incorporate_unassigned_branches(reconstruction);
            
            logger->debug4("Before create_connections; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("Before create_connections; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

            // Create connections now that all branches have been associated with a composite branch
            create_connections(reconstruction);

            logger->debug4("After create_connections; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("After create_connections; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

            logger->debug("Looking for zero-length composite segments");
            for (NeuronSegment * seg : composite->get_segments()){
                if (seg->markers.size() == 0){
                    logger->warn("Found zero-length segments at end of alignment and processing");
                }
            }
            logger->debug("On to next reconstruction");
        }
    }

    builder_state = BUILT_COMPOSITE;

};

void ConsensusBuilder::match_remaining_branches(Reconstruction * reconstruction){
    logger->debug1("Enter match_remaining_branches");
    std::set<Match *> matches = find_matches_local_alignment();
    
    // Produce sets of matches on the same segment (should not be overlapping)
    std::map<NeuronSegment *, std::set<Match * > > matches_by_segment;
    for (Match * match : matches){
        matches_by_segment[match->seg1].insert(match);
        matches_by_segment[match->seg2].insert(match);
    }
    
    // First, make association between branches and composite branches based on match specifications
    for (Match * match : matches){
        logger->debug2("Next match %p",match);

        CompositeBranchContainer * c_branch = composite->get_branch_by_segment(match->seg1);
        BranchContainer * r_branch = reconstruction->get_branch_by_segment(match->seg2);
        unmatched_tree1_segments.erase(c_branch->get_segment());
        unmatched_tree2_segments.erase(r_branch->get_segment());

        logger->debug3("seg1 %p seg2 %p",match->seg1,match->seg2);
        logger->debug3("c_branch %p r_branch %p",match);

        std::vector<pair<int,int> > alignment = match->alignment;
        logger->debug2("Alignment size: %i",alignment.size());
        logger->debug2("Alignment start: %i %i, end: %i %i", alignment.front().first, alignment.front().second, alignment.back().first, alignment.back().second);
        logger->debug2("Compos branch length %i, recon branch length %i", c_branch->get_segment()->markers.size(),r_branch->get_segment()->markers.size());
/*
        for (pair<int,int> intpr : alignment){
            logger->debug4("%i %i",intpr.first,intpr.second);
        }
  */
        // Merge position
        double r_weight = r_branch->get_confidence();
        double c_weight = c_branch->get_summed_confidence();
        double combined_weight = r_weight + c_weight;
        NeuronSegment * r_seg = r_branch->get_segment();
        NeuronSegment * c_seg = c_branch->get_segment();
        for (int i = 0; i < match->alignment.size(); i++){
            int j = match->alignment[i].first;
            int k = match->alignment[i].second;
            c_seg->markers[i]->x = (c_weight * c_seg->markers[j]->x + r_weight * r_seg->markers[k]->x) / combined_weight;
            c_seg->markers[i]->y = (c_weight * c_seg->markers[j]->y + r_weight * r_seg->markers[k]->y) / combined_weight;
            c_seg->markers[i]->z = (c_weight * c_seg->markers[j]->z + r_weight * r_seg->markers[k]->z) / combined_weight;
            c_seg->markers[i]->radius = (c_weight * c_seg->markers[j]->radius + r_weight * r_seg->markers[k]->radius) / combined_weight;
        }
        
        // Split branches as needed - composite branch
        if (alignment.front().first != 0 || alignment.back().first != c_branch->get_segment()->markers.size()-1){
            logger->debug1("Splitting composite branch of size %i given alignment from %i - %i",c_branch->get_segment()->markers.size(),match->seg1_start(),match->seg1_end());

            logger->debug("c_seg before %p with other matches %i",c_seg,matches_by_segment[c_seg].size());
            for (Match * other_match : matches_by_segment[c_seg]){
                logger->debug("Other c_ceg match %p size %i before: %i - %i",other_match,c_seg->markers.size(),other_match->seg1_start(),other_match->seg1_end());
            }
            

            std::vector<CompositeBranchContainer *> resulting_c_branches;
            c_branch = split_branch(c_branch,match,resulting_c_branches);
            
            // Add new segments to unmatched set
            for (int i = 0; i < resulting_c_branches.size(); i++){
                CompositeBranchContainer * c_of_set = resulting_c_branches[i];
                if (c_of_set != c_branch)
                    unmatched_tree1_segments.insert(c_of_set->get_segment());
            }
            
            logger->debug("c_seg after %p, with %i other matches",c_seg,matches_by_segment[c_seg].size());

            // Update other matches, their segment references and alignment start/end points
            matches_by_segment[c_seg].erase(match);
            std::set<Match *> other_matches = matches_by_segment[c_seg];
            for (Match * other_match : other_matches){
                if (other_match->seg1_start() > match->seg1_end()){
                    for (int i = 0; i < other_match->alignment.size(); i++){
                        other_match->alignment[i].first -= (match->alignment.back().first + 1);
                    }
                }else if (other_match->seg1_end() < match->seg1_start()){
                    other_match->seg1 = resulting_c_branches[0]->get_segment();
                    matches_by_segment[c_seg].erase(other_match);
                    matches_by_segment[other_match->seg1].insert(other_match);
                }else{
                    logger->error("other match that overlaps? match range %i %i; other range %i %i", match->alignment[0].first, match->alignment.back().first, other_match->seg1_start(), other_match->seg1_end());
                }
                logger->debug("Other match %p size %i after: start %i end %i",other_match,other_match->seg1->markers.size(),other_match->seg1_start(),other_match->seg1_end());
                if (other_match->seg1_end() - other_match->seg1_start() + 1 > other_match->seg1->markers.size()){
                    logger->error("Alignment too large for segment size");
                }

            }
        }
        
        // Split reconstruction branch
        int align_start = match->seg2_start();
        int align_end = match->seg2_end();
        if (align_start != 0 || align_end != r_branch->get_segment()->markers.size()-1){
            logger->debug1("Splitting recon branch of size %i given alignment from %i - %i",r_branch->get_segment()->markers.size(),match->seg2_start(),match->seg2_end());
            for (Match * other_match : matches_by_segment[r_seg]){
                logger->debug("Other r_seg match %p size %i start before: %i - %i", other_match,r_seg->markers.size(),other_match->seg2_start(),other_match->seg2_end());
            }

            std::vector<BranchContainer *> resulting_r_branches;
            logger->debug4("segments in recon before split %i",r_branch->get_reconstruction()->get_segments().size());
            r_branch = split_branch(r_branch,match,resulting_r_branches);
            logger->debug4("segments in recon after split %i",r_branch->get_reconstruction()->get_segments().size());
            
            // Add new segments to unmatched set
            for (int i = 0; i < resulting_r_branches.size(); i++){
                BranchContainer * r_of_set = resulting_r_branches[i];
                logger->debug1("r_of_set %p",r_of_set);
                if (r_of_set != r_branch)
                    unmatched_tree2_segments.insert(r_of_set->get_segment());
            }
            
            // Update other matches, their segment references and alignment start/end points
            matches_by_segment[r_seg].erase(match);
            std::set<Match *> other_matches = matches_by_segment[r_seg];
            for (Match * other_match : other_matches){
                if (other_match->seg2_start() > match->seg2_end()){
                    for (int i = 0; i < other_match->alignment.size(); i++){
                        other_match->alignment[i].second -= (match->seg2_end() + 1);
                    }
                }else if (other_match->seg2_end() < match->seg2_start()){
                    other_match->seg2 = resulting_r_branches[0]->get_segment();
                    matches_by_segment[r_seg].erase(other_match);
                    matches_by_segment[other_match->seg2].insert(other_match);
                }else{
                    logger->error("other match that overlaps? match range %i %i; other range %i %i", match->seg2_start(), match->seg2_end(), other_match->seg2_start(), other_match->seg2_end());
                }
                logger->debug("Other match %p size %i after: start %i - %i",other_match,other_match->seg2->markers.size(),other_match->seg2_start(),other_match->seg2_end());
                if (other_match->seg2_end() - other_match->seg2_start() + 1 > other_match->seg2->markers.size()){
                    logger->error("Alignment too large for segment size");
                }
            }

        }
        
        logger->debug4("Number of connections in existing composite %i",c_branch->get_connections().size());

        // Create association
        logger->debug("Creating match between c_branch %p and r_branch %p",c_branch,r_branch);
        c_branch->add_branch_match(r_branch, match->forward);
        r_branch->set_composite_match(c_branch);
    }
    
    logger->debug1("Exit match_remaining_branches");
};

/**
 * Takes segments from tree 1 and 2 that have not already been assigned
 * Returns a set of Match objects
 **/
std::set<Match *> ConsensusBuilder::find_matches_local_alignment(){
    // Set of matches to be returned
    std::set<Match *> final_assignments;

    // Get candidate matches for each segment
    logger->debug2("enter find_matches_local_alignment");
    std::map<NeuronSegment *,std::vector<Match *> * > matches_by_segment =
        generate_candidate_matches_via_local_align(unmatched_tree2_segments);
    logger->debug3("generated candidate matches via local align, num %i",matches_by_segment.size());
    
    if (matches_by_segment.size() == 0){
        return final_assignments;
    }
    /** Create conflict sets to determine best set of matches that are consistent **/
    
    /* Determine all conflicts for each match */
    
    // Map from a given match to all of its immediate conflicts
    std::map<Match *, std::set<Match *> * > match_conflicts;
    // Map of marginal-conflict pairs, those accepted but with small overlap, with the overlap size as the value
    //std::map<pair<Match *, Match *>, int> small_overlaps;
    std::map<Match *, map<Match *, bool> > small_overlaps;
    
    // Put conflicts into match_conflicts
    logger->debug("about to find conflicts");

    find_conflicts(unmatched_tree1_segments, matches_by_segment, match_conflicts, small_overlaps);
    find_conflicts(unmatched_tree2_segments, matches_by_segment, match_conflicts, small_overlaps);
    
    /* Process the conflict sets for possible combinations of assignments, and select optimal assignments for a given conflict set */
    
    // Create a set of ALL matches - will be used as set of matches that have not yet been processed
    logger->debug("create set of ALL matches");
    std::set<Match *> unprocessed;
    for (std::map<NeuronSegment *,std::vector<Match *> * >::iterator it = matches_by_segment.begin();
         it != matches_by_segment.end(); ++it){
        for (Match * match : *(it->second)){
            unprocessed.insert(match);
        }
    }
    
    logger->debug("while unprocessed isn't empty");
    while (!unprocessed.empty()){
        logger->debug("unprocessed size %i",unprocessed.size());

        Match * first_match = *(unprocessed.begin());
        unprocessed.erase(first_match);
        logger->debug3("# match conflicts %i on match %p",match_conflicts[first_match]->size(),first_match);
        if (match_conflicts[first_match]->size() == 0){
            logger->debug3("match %p has no conflicts",first_match);
            final_assignments.insert(first_match);
            continue;
        }

        // Map of matches and whether they are assigned or not (T/F)
        std::map<Match *,bool> *assignment_map_ptr, *assignment_map_copy_ptr;
        std::map<Match *,bool> assignment_map;
        // Initialize a vector of assignment maps for all possible cases
        std::vector<std::map<Match *,bool> * > assignment_options;
        // Cumulative score for each assignments map
        std::map<std::map<Match *,bool> *, double> score_map;
        
        // Stack of matches still remaining to determine
        std::stack<Match *> * next_matches, * next_matches_copy;
        // "Possibilities" stack: Stack of next_matches stacks, as each path of assignments needs its own set of remaining matches to process
        std::stack<std::stack<Match *> * > next_matches_stack;
        // Map that gets the assignment_map for a given next_matches stack
        std::map<std::stack<Match *> *, std::map<Match *,bool> * > next_matches_map;
        
        logger->debug("Processing 1; %i conflicts",match_conflicts[first_match]->size());

        // Initialize objects for the current conflict set
        assignment_map_ptr = new std::map<Match *,bool>();
        assignment_options.push_back(assignment_map_ptr);
        next_matches = new std::stack<Match *>();
        // Put first match in set into the stack
        //next_matches.push(*(conflict_set->begin()));
        next_matches->push(first_match);
        // Put first stack of next_matches into stack of paths (beginning with just one)
        next_matches_map[next_matches] = assignment_map_ptr;
        next_matches_stack.push(next_matches);
        score_map[assignment_map_ptr] = 0;
        
        logger->debug("Processing 2");

        while (!next_matches_stack.empty()){
            logger->debug4("next_matches_stack size %i",next_matches_stack.size());

            // Pull the current next_matches and assignments objects
            next_matches = next_matches_stack.top();
            next_matches_stack.pop();
            assignment_map_ptr = next_matches_map[next_matches];
            assignment_map = *assignment_map_ptr;
            
            // Loop until no stacks remain
            while (!next_matches->empty()){
                Match * candidate = next_matches->top();
                logger->debug4("candidate %p with %i conflicts; next_matches size %i",candidate,match_conflicts[candidate]->size(),next_matches->size());
                logger->debug4("candidate seg1 %i - %i, seg2 %i - %i",candidate->seg1_start(), candidate->seg1_end(), candidate->seg2_start(), candidate->seg2_end());
                logger->debug4("candidate has %i conflicts",match_conflicts[candidate]->size());
                next_matches->pop();
                unprocessed.erase(candidate); // this match will now have been processed
                
                bool must_be_off = false;
                int count_off = 0;
                
                std::set<Match *> to_add;
                // First determine whether the candidate can be on and/or off (if the match has no conflict on either side, turn it ON)
                for (Match * conflict : *(match_conflicts[candidate])){
                    if (conflict->seg1 == candidate->seg1){
                        logger->debug4("Conflict %i - %i",conflict->seg1_start(),conflict->seg1_end());
                    }else{
                        logger->debug4("Conflict %i - %i",conflict->seg2_start(),conflict->seg2_end());
                    }
                    if (assignment_map.find(conflict) != assignment_map.end()){
                        if (assignment_map[conflict]){
                            logger->debug4("Conflict is set to ON");
                            must_be_off = true;
                        }else{
                            logger->debug4("Conflict is set to OFF");
                            count_off++;
                        }
                    }else {
                        logger->debug4("Conflict is not set");
                        // Add this match to those to be processed
                        next_matches->push(conflict);
                    }
                }
                logger->debug4("Count of OFF %i",count_off);

                if (must_be_off){
                    assignment_map[candidate] = false;
                    logger->debug4("Must be OFF");
                }else if (count_off == match_conflicts[candidate]->size()){
                    logger->debug4("count_off equals size of conflict set, so assign this match candidate to ON");
                    // No conflicts remain for this match, so don't bother with the case of it being OFF
                    assignment_map[candidate] = true;
                    score_map[assignment_map_ptr] += candidate->score;
                }else{
                    logger->debug4("No conflict is ON, and some conflicts have not been set, so create version ON and OFF");
                    logger->debug4("next_matches size before %i, next_matches_stack before %i",next_matches->size(),next_matches_stack.size());

                    // Can be on or off, need to duplicate the assignment map for each possibility
                    assignment_map_copy_ptr = new std::map<Match *,bool>();
                    *assignment_map_copy_ptr = *assignment_map_ptr;
                    next_matches_copy = new std::stack<Match *>();
                    *next_matches_copy = *next_matches;
                    
                    // Copy the score
                    score_map[assignment_map_copy_ptr] = score_map[assignment_map_ptr];
                    
                    // Link the new next_matches stack to its assignment map
                    next_matches_map[next_matches_copy] = assignment_map_copy_ptr;
                    // Set the copy to have this candidate OFF
                    (*assignment_map_copy_ptr)[candidate] = false;
                    // Push the new next_matches stack onto the possiblities stack
                    next_matches_stack.push(next_matches_copy);
                    
                    // Set the original to have this candidate ON
                    assignment_map[candidate] = true;
                    score_map[assignment_map_ptr] += candidate->score;

                    logger->debug4("next_matches size after %i, next_matches_stack after %i",next_matches->size(),next_matches_stack.size());
                }
            }
            delete next_matches;
        }
        
        
        // Determine best assignment_map
        std::map<Match *,bool> * best_assignments;
        double best_score = 0;
        for (std::map<Match *, bool> * assignment_map : assignment_options){
            if (score_map[assignment_map] > best_score){
                best_assignments = assignment_map;
                best_score = score_map[assignment_map];
            }
        }
        // Transfer assignments to final_assignments
        logger->info("Transfering matches into final_assignments");
        for (std::map<Match *,bool>::iterator it = best_assignments->begin(); it != best_assignments->end(); ++it){
            if (it->second){
                logger->debug("Adding match %p",it->first);
                final_assignments.insert(it->first);
            }else{
                // Delete rejected candidate matches
                logger->debug("Deleting match %p",it->first);
                delete it->first;
            }
        }
        
        // Cleanup of asignment objects
        for (std::map<Match *,bool> * assignment_map : assignment_options){
            delete assignment_map;
        }
    }
    
    // Delete match_conflicts sets
    typedef std::map<Match *, std::set<Match *> * > mc_map;
    for (mc_map::iterator conflicts_it = match_conflicts.begin(); conflicts_it != match_conflicts.end(); ++conflicts_it){
        delete conflicts_it->second;
    }
    
    // Resolve small overlaps
    logger->debug("Resolve small overlaps");
    std::set<Match *> processed_previously;
    // Process each match with other possible matches
    for (Match * m1 : final_assignments){
//        Match * m1 = final_assignments_vect[i];
        processed_previously.insert(m1);
        // Check whether m1 has any overlaps
        if (small_overlaps.find(m1) != small_overlaps.end()){
            logger->debug("M1 %p has %i small overlaps",m1,small_overlaps[m1].size());

            // Go through overlaps and process those that haven't already been processed
            for (pair<Match*,bool> overlap : small_overlaps[m1]){
                if (processed_previously.find(overlap.first) == processed_previously.end()){
                    Match *m2 = overlap.first;

                    printf("M1 %p M2 %p\n",m1,m2);
                    printf("M1 segments %p %p\n",m1->seg1,m1->seg2);
                    printf("M2 segments %p %p\n",m2->seg1,m2->seg2);
                    // ** For those that overlap, just divide the markers roughly evenly **

                    // Calculate the overlap size using the segment positions on the overlapped segment
                    int m1start, m1end, m2start, m2end;
                    // Overlap is via first segment (denoted in second value in overlap pair) in both matches
                    bool overlap_on_first = overlap.second;
                    if (overlap_on_first){
                        // Work on the first segment
                        m1start = m1->seg1_start();
                        m1end = m1->seg1_end();
                        m2start = m2->seg1_start();
                        m2end = m2->seg1_end();
                    }else{
                        // Work on the second segment
                        m1start = m1->seg2_start();
                        m1end = m1->seg2_end();
                        m2start = m2->seg2_start();
                        m2end = m2->seg2_end();
                    }
                    logger->debug3("Determined which segment overlap is on; first? %i",overlap_on_first);

                    printf("M1 alignment size %i\n",m1->alignment.size());
                    printf("M2 alignment size %i\n",m2->alignment.size());
                    printf("small overlap_on_first %i; m1 %i - %i, m2 %i - %i, m1 forward %i, m2 forward %i\n", overlap_on_first, m1start, m1end,m2start,m2end,m1->forward, m2->forward);
                    // Determine which match aligns from the top (before) and which from the bottom (after)
                    if ((m1start > m2start && m1start <= m2end) || (m1start == m2start && m2end < m1end)){
                        // M2 aligns first, then M1
                        int overlap_size = m2end - m1start + 1;
                        int cut_from1 = (overlap_size+1) / 2;
                        int cut_from2 = overlap_size / 2;
                        
                        printf("M2 aligns first; cut from 1: %i; cut from 2: %i\n",cut_from1,cut_from2);
                        
                        // M1 cuts off the front
                        if (overlap_on_first || m1->forward)
                            m1->alignment.erase(m1->alignment.begin(), m1->alignment.begin() + cut_from1);
                        else
                            m1->alignment.erase(m1->alignment.begin() + (m1->alignment.size() - cut_from1), m1->alignment.end());

                        // M2 cuts off the end
                        if (overlap_on_first || m2->forward)
                            m2->alignment.erase(m2->alignment.begin() + (m2->alignment.size() - cut_from2), m2->alignment.end());
                        else
                            m2->alignment.erase(m2->alignment.begin(), m2->alignment.begin() + cut_from2);
                        
                        if (overlap_on_first)
                            printf("After mods, m1 %i to %i, m2 %i to %i\n", m1->seg1_start(), m1->seg1_end(), m2->seg1_start(), m2->seg1_end());
                        else
                            printf("After mods, m1 %i to %i, m2 %i to %i\n", m1->seg2_start(), m1->seg2_end(), m2->seg2_start(), m2->seg2_end());

                    }else if ((m2start > m1start && m2start <= m1end) || (m2start == m1start && m1end < m2end)){
                        // M1 aligns first, then M2
                        int overlap_size = m1end - m2start + 1;
                        int cut_from1 = (overlap_size+1) / 2;
                        int cut_from2 = overlap_size / 2;
                        
                        printf("M1 aligns first; cut from 1: %i; cut from 2: %i\n",cut_from1,cut_from2);

                        // M1 cuts off the front
                        if (overlap_on_first || m1->forward)
                            m1->alignment.erase(m1->alignment.begin() + (m1->alignment.size() - cut_from1), m1->alignment.end());
                        else
                            m1->alignment.erase(m1->alignment.begin(), m1->alignment.begin() + cut_from1);
                        
                        // M2 cuts off the end
                        if (overlap_on_first || m2->forward)
                            m2->alignment.erase(m2->alignment.begin(), m2->alignment.begin() + cut_from2);
                        else
                            m2->alignment.erase(m2->alignment.begin() + (m2->alignment.size() - cut_from2), m2->alignment.end());

                    }else{
                        logger->warn("Final matches marked as conflicting at end, but no overlap found");
                        continue;
                    }
                    bool favor_end = true;
                    /*
                    for (int ov_step = 0; ov_step < overlap_end - overlap_start; ov_step++){
                        //double dist_end = dist(*(markers1[]), *(segment->markers[overlap_start+ov_step]));
                        //double dist_begin = dist(*(markers2[]), *(segment->markers[overlap_start+ov_step]));
                        if (ov_step){

                        }else{
                            
                        }
                        double mrk1_dist = dist(*(markers1[]),*(markers2[]));
                    }
                     */
                }
            }
        }
    }
    
    return final_assignments;
};

std::map<NeuronSegment *,std::vector<Match *> *> ConsensusBuilder::generate_candidate_matches_via_local_align(std::set<NeuronSegment *> tree2_segments){
    /* Run local alignment on candidates [produce std::map<NeuronSegment *, Match *> matches_by_segment] */
    
    // Map from branch to all possible matches
    // Map<NeuronSegment *,vector<Match *> >; need to initialize for all segments of either side?
    std::map<NeuronSegment *,std::vector<Match *> *> candidate_matches;
    std::map<NeuronSegment *,Match *> best_matches;
    
    // Attempt to find a match for each segment in the new tree (tree 2)
    for (NeuronSegment * seg2 : tree2_segments){
        // Get nearby composite segments for alignment
        std::set<NeuronSegment *> nearby_segments = get_nearby_segments(seg2, composite);
        // Make sure seg2 has a vector of matches created
        //candidate_matches[seg2] = new std::vector<Match *>();
        candidate_matches[seg2] = new std::vector<Match *>();
        
        for (NeuronSegment * seg1 : nearby_segments){
            // Make sure seg1 has a vector of matches created
            if (candidate_matches.find(seg1) == candidate_matches.end())
                candidate_matches[seg1] = new std::vector<Match *>();

            logger->debug3("Nearby segment unmatched ? %i",unmatched_tree1_segments.find(seg1) != unmatched_tree1_segments.end());
            if (unmatched_tree1_segments.find(seg1) == unmatched_tree1_segments.end()) continue;
            logger->debug3("about to local align a branch and a composite branch! lengths %i by %i",seg2->markers.size(),seg1->markers.size());
            Match * match = nullptr;
            // Run local alignment
            vector<pair<int, int> > alignment;// = new vector<pair<int, int> >();
            double score = local_align(average_alignment_dist, seg1->markers, seg2->markers, alignment, gap_cost);
            logger->debug2("Completed local_align with score %f, length %i", score, alignment.size());
            // Store the alignment: If alignment spans entire extent of one segment, or if alignment is long enough, or if per-character alignment score is good enough
            if ((alignment.size()+1 >= seg1->markers.size() && score > 0) || (alignment.size() > min_alignment_size && 2*score/alignment.size() >= average_alignment_dist)){
                logger->debug3("Good enough to create a Match");
                match = new Match();
                match->seg1 = seg1;
                match->seg2 = seg2;
                match->score = score;
                match->alignment = alignment;
                logger->debug3("forward alignment size %i",alignment.size());
                match->forward = true;
            }
            
            // Local align with one branch reversed (if forward align score isn't high enough to rule out reverse - heuristic)
            if (score < good_enough_score){
                logger->debug3("Score not good enough, going to try reverse alignment");
                vector<pair<int, int> > alignment_reverse;// = new vector<pair<int, int> >();
                vector<MyMarker *> seg2_reversed = seg2->markers;
                std::reverse(seg2_reversed.begin(), seg2_reversed.end());
                score = local_align(average_alignment_dist, seg1->markers, seg2_reversed, alignment_reverse, gap_cost);

                // Reverse alignment to reflect actual positions in the segment
                int seg_size = seg2_reversed.size();
                for (int i = 0; i < alignment_reverse.size(); i++){
                    alignment_reverse[i].second = seg_size - alignment_reverse[i].second;
                }
                
                if (match){ // Forward alignment was successful
                    if (score > match->score){ // Reverse alignment is better than forward alignment
                        //delete match->alignment;
                        match->alignment = alignment_reverse;
                        logger->debug3("alignment_reverse size %i",alignment_reverse.size());
                        match->forward = false;
                    }
                }else if ((alignment_reverse.size()+1 >= seg1->markers.size() && score > 0) || alignment_reverse.size() > min_alignment_size && score/alignment_reverse.size() >= average_alignment_dist){
                    // Forward alignment failed, reverse alignment succeeded and produces a match
                    match = new Match();
                    match->seg1 = seg1;
                    match->seg2 = seg2;
                    match->score = score;
                    
                    match->alignment = alignment_reverse;
                    logger->debug3("alignment_reverse size %i",alignment_reverse.size());
                    match->forward = false;
                }
            }
            
            logger->debug3("Generated alignments, checking match");
            
            // If at least one of forward and reverse matches is good enough
            //    and if the segments don't have matches yet or the current match is within range of the best current matches:
            //       put best of forward and reverse matches into vector of matches
            if (match && (!best_matches[seg1] || match->score >= best_matches[seg1]->score - best_branch_margin) &&
                (!best_matches[seg2] || match->score >= best_matches[seg2]->score - best_branch_margin)){
                logger->debug2("match is good enough with score %f and length %i, putting it into candidate_matches",match->score,match->alignment.size());
                if (match->alignment.size() == 0)
                    logger->error("Alignment length 0 for segs %p and %p",match->seg1, match->seg2);
                candidate_matches[seg1]->push_back(match);
                candidate_matches[seg2]->push_back(match);
                
                if (match->seg1_end() - match->seg1_start() + 1 > match->seg1->markers.size()){
                    logger->error("Create match with alignment too big for segment1! %i - %i size %i",match->seg1_end(),match->seg1_start(),match->seg1->markers.size());
                }
                if (match->seg2_end() - match->seg2_start() + 1 > match->seg2->markers.size()){
                    logger->error("Create match with alignment too big for segment2! %i - %i size %i",match->seg2_end(),match->seg2_start(),match->seg2->markers.size());
                }

                /*
                // Stretch alignment on either side if it is 1 segment away from the segment end
                if (match->forward){
                    if (match->alignment[0].first == 1){
                    
                    }
                    if (match->alignment.back().first == seg1->markers.size()-2){
                    
                    }
                    if (match->alignment[0].second == 1){
                        
                    }
                    if (match->alignment.back().second == seg1->markers.size()-2){
                        
                    }
                }else{
                    
                }
                 */
                
                // Update best matches if new match is best
                if (!best_matches[seg1] || match->score >= best_matches[seg1]->score){
                    best_matches[seg1] = match;
                }
                if (!best_matches[seg2] || match->score >= best_matches[seg2]->score){
                    best_matches[seg2] = match;
                }
            }
        }
    }
    return candidate_matches;
};

void ConsensusBuilder::find_conflicts(std::set<NeuronSegment *> tree_segments,
                                      std::map<NeuronSegment *, vector<Match *> * > &matches_by_segment,
                                      std::map<Match *, std::set<Match *> * > &match_conflicts,
                                      std::map<Match *, std::map<Match *, bool> > &small_overlaps){
    int cnt = 0, num_conflicts = 0;
    logger->debug("Enter find_conflicts");
    for (NeuronSegment * segment : tree_segments){
        //logger->debug4("matches_by_segment.find(segment) == matches_by_segment.end() -> %i",matches_by_segment.find(segment) == matches_by_segment.end());
        if (matches_by_segment.find(segment) == matches_by_segment.end()) continue;
        logger->debug3("Segment %p with %i matches",segment,matches_by_segment[segment]->size());
        for (int i = 0; i < matches_by_segment[segment]->size(); i++){
            if (match_conflicts.find(matches_by_segment[segment]->at(i)) == match_conflicts.end())
                match_conflicts[matches_by_segment[segment]->at(i)] = new std::set<Match *>();
        }

        logger->debug3("run_to %i",matches_by_segment[segment]->size()-1);
        // No need to run comparisons if there aren't at least 2 matches on the segment
        if (matches_by_segment[segment]->size() <= 1) continue;
        
        for (int i = 0; i < matches_by_segment[segment]->size()-1; i++){
            logger->debug3("match %i",i);
            Match * match1 = matches_by_segment[segment]->at(i);
            bool first_segment = (segment == match1->seg1);
            // Create sets of conflicts for a given match candidate
            for (int j = i+1; j < matches_by_segment[segment]->size(); j++){
                logger->debug4(" - match j %i",j);
                Match * match2 = matches_by_segment[segment]->at(j);
                if (first_segment && segment != match2->seg1)
                    logger->error("Seg1 of match 2 isn't same as of match 1, is %p",match2->seg1);
                else if (!first_segment && segment != match2->seg2)
                    logger->error("Seg2 of match 2 isn't same as of match 1, is %p",match2->seg2);
                
                if (first_segment)
                    logger->debug4(" - check: from %i - %i vs %i to %i",match1->seg1_start(),match1->seg1_end(),match2->seg1_start(),match2->seg1_end());
                else
                    logger->debug4(" - check: from %i - %i vs %i to %i",match1->seg2_start(),match1->seg2_end(),match2->seg2_start(),match2->seg2_end());
                
                int overlap_size = get_overlap_size(match1, match2, first_segment);
                if (overlap_size == -1)
                    logger->error("Overlap could not be calculated!!!");
                logger->debug4(" - overlap size %i",overlap_size);
                // No conflict
                if (overlap_size <= allowable_alignment_overlap){
                    // Store overlap size (within acceptable range)
                    if (abs(overlap_size) > 0){
                        small_overlaps[match1][match2] = first_segment;
                        small_overlaps[match2][match1] = first_segment;
                        logger->debug3("Small overlap of size %i",overlap_size);
                    }
                }else{
                    // Add the matches to each others' conflict sets
                    match_conflicts[match1]->insert(match2);
                    match_conflicts[match2]->insert(match1);
                    logger->debug3("Added conflict on matches %p (forward %i) and %p (forward %i)",match1,match1->forward,match2,match2->forward);
                    
                    if (first_segment)
                        logger->debug3("Added conflict on seg1 given %i - %i and %i - %i",match1->seg1_start(),match1->seg1_end(),match2->seg1_start(),match2->seg1_end());
                    else
                        logger->debug3("Added conflict on seg2 given %i - %i and %i - %i",match1->seg2_start(),match1->seg2_end(),match2->seg2_start(),match2->seg2_end());

                    logger->debug3("%i conflicts",++num_conflicts);
                }
            }
        }
    }
};


/**
 *  Create weighted average for matches (TG)
 *  Find locations to split on either merged segment (when segments.size() > 1)
 **/
void ConsensusBuilder::average_and_split_alignments(Reconstruction * reconstruction, std::vector<std::map<MyMarker *, MyMarker *> > &segment_maps, std::vector<pair<int,int> > marker_index_alignments, std::vector<MyMarker*> merged_seg1, std::vector<MyMarker*> merged_seg2, std::vector<NeuronSegment *> segments1, std::vector<NeuronSegment *> segments2){
    
    // Create sets of markers for each reconstruction and composite segment in the alignment to allow for later determination of which segment a marker is in
    logger->debug1("Determine split");
    bool composite_multi_seg = false;
    std::vector<std::set<MyMarker *> > c_segment_sets(segments1.size());
    if (segments1.size() > 1){
        composite_multi_seg = true;
        for (int i=0; i < segments1.size(); i++){
            for (MyMarker * mark : segments1[i]->markers){
                c_segment_sets[i].insert(mark);
            }
        }
    }
    //bool final_split_recon = composite_multi_seg; // Probably can be removed
    
    bool recon_multi_seg = false;
    std::vector<std::set<MyMarker *> > r_segment_sets(segments2.size());
    if (segments2.size() > 1){
        recon_multi_seg = true;
        for (int i=0; i < segments2.size(); i++){
            for (MyMarker * mark : segments2[i]->markers){
                r_segment_sets[i].insert(mark);
            }
        }
    }
    
    // Set up weighting between reconstruction and composite weights based on first (or only) branch container
    logger->debug2("Setting up weighting");
    int r_seg_num = 0, c_seg_num = 0;
    NeuronSegment * current_c_seg = segments1[c_seg_num];
    NeuronSegment * current_r_seg = segments2[r_seg_num];
    logger->debug2("Going to get branch by segment on recon seg %p",current_r_seg);
    BranchContainer * recon_branch = reconstruction->get_branch_by_segment(current_r_seg);
    logger->debug2("Going to get branch by segment on composite seg %p",current_c_seg);
    CompositeBranchContainer * composite_branch = composite->get_branch_by_segment(current_c_seg);
    logger->debug2("Calculating weights on %p and %p",recon_branch,composite_branch);
    double reconstruction_weight = recon_branch->get_confidence();
    double composite_weight = composite_branch->get_summed_confidence();
    double combined_weight = reconstruction_weight + composite_weight;
    logger->debug2("Weights calculated, composite %f, recon %f",composite_weight,reconstruction_weight);
    logger->debug2("Reconstruction confidence %f, combined weight %f",reconstruction->get_confidence(), combined_weight);
    
    // Create averaged segment to replace current composite, and determine segment split points
    map<MyMarker*, MyMarker*> map_segment;
    MyMarker *r_marker, *c_marker;
    double x, y, z;
    
    logger->debug1("Loop through aligned point pairs segments1 %i segments2 %i",segments1.size(),segments2.size());
    std::map<NeuronSegment *, std::vector<std::size_t> > tree1_splits, tree2_splits;
    typedef pair<NeuronSegment *,std::size_t> SegmentPointPair;
    //std::vector<SegmentPointPair> tree1_splits, tree2_splits;
    
    logger->debug("number of marker_index_alignments %i",marker_index_alignments.size());
/*
    logger->debug4("Composite segments1[0].markers size %i, segments1[1].markers size %i",segments1[0]->markers.size(),segments1[1]->markers.size());
    logger->debug4("Composite segments1[0].markers[0] %p, segments1[1].markers[0] %p",segments1[0]->markers[0],segments1[1]->markers[0]);
    logger->debug4("Composite segments1[0].markers[last] %p, segments1[1].markers[last] %p",segments1[0]->markers.back(),segments1[1]->markers.back());
    logger->debug4("Recon segments1[0].markers size %i, segments1[1].markers size %i",segments2[0]->markers.size(),segments2[1]->markers.size());
    logger->debug4("Recon segments1[0].markers[0] %p, segments1[1].markers[0] %p",segments2[0]->markers[0],segments2[1]->markers[0]);
    logger->debug4("Recon segments1[0].markers[last] %p, segments1[1].markers[last] %p",segments2[0]->markers.back(),segments2[1]->markers.back());
*/
    logger->debug4("Composite First marker %p, first marker of first segment %p", merged_seg1[marker_index_alignments[0].first], current_c_seg->markers[0]);
    logger->debug4("Reconstruction First marker %p, first marker of first segment %p", merged_seg2[marker_index_alignments[0].second], current_r_seg->markers[0]);
    int j;
//    for (int i=0; i < marker_index_alignments.size(); i++) // Loop through aligned point pairs
    MyMarker *prev_c = nullptr, *prev_r = nullptr;
    std::set<MyMarker *> markers_to_merge;
    double nearest_dist;
    for (int i=0; i < marker_index_alignments.size(); i++) // Loop through aligned point pairs
    {
        j = marker_index_alignments.size() - i - 1;
        
        c_marker = merged_seg1[marker_index_alignments[j].first];
        r_marker = merged_seg2[marker_index_alignments[j].second];
        logger->debug2("i %i",i);
        logger->debug4("looping through aligned pairs 1 %p %p",c_marker,r_marker);
        logger->debug3("c_seg_num %i ; c_marker %p",c_seg_num, c_marker);

        /**
         *  Checking whether either composite or reconstruction are at a new branch - since they are of different data types we can't run the same code
         *  twice with the parameter order flipped
         *      - Check whether we are in a new segment in either branch and recalculate weights
         *          (Assuming that if reconstruction segment ends composite is still going, otherwise the match would have been restricted to the co-
         *           terminating segments)
         **/
        // If this is the beginning of a new reconstruction branch/segment
        if (recon_multi_seg && r_segment_sets[r_seg_num].find(r_marker) == r_segment_sets[r_seg_num].end()){
            if (composite_multi_seg && c_segment_sets[c_seg_num].find(c_marker) == c_segment_sets[c_seg_num].end()){
                // Update composite branch without creating a split
                logger->debug4("Test1");
                current_c_seg = segments1[++c_seg_num];
                composite_branch = composite->get_branch_by_segment(current_c_seg);   // CompositeBranch of segment
                logger->debug4("Test2");
                composite_weight = composite_branch->get_summed_confidence();
            }else{
                // Get the location on the composite segment at which to make the split
                std::size_t split_point = std::find(current_c_seg->markers.begin(), current_c_seg->markers.end(), c_marker) - current_c_seg->markers.begin();
                //tree1_splits.push_back(SegmentPointPair(current_c_seg,split_point));
                tree1_splits[current_c_seg].push_back(split_point);
            }
            logger->debug4("Test3 %i", r_seg_num);

            // At a new reconstruction segment
            current_r_seg = segments2[++r_seg_num];        // Next reconstruction segment
            recon_branch = reconstruction->get_branch_by_segment(current_r_seg);     // Branch of segment
            logger->debug4("Test4 %p %p",current_r_seg,recon_branch);

            // Update weights
            reconstruction_weight = recon_branch->get_confidence();
            combined_weight = reconstruction_weight + composite_weight;

            logger->debug4("looping through aligned pairs 2");
        }

        // If this is the beginning of a new composite branch/segment
        else if (composite_multi_seg && c_segment_sets[c_seg_num].find(c_marker) == c_segment_sets[c_seg_num].end()){
            logger->debug4("new composite segment");

            // Get the location on the reconstruction segment at which to make the split
            std::size_t split_point = std::find(current_r_seg->markers.begin(), current_r_seg->markers.end(), r_marker) - current_r_seg->markers.begin();
            logger->debug("seg length %i, split_point %i",current_r_seg->markers.size(),split_point);
            tree2_splits[current_r_seg].push_back(split_point);
            
            // Start up new composite branches and segments
            current_c_seg = segments1[++c_seg_num];        // Next composite segment
            logger->debug("c_seg_num %i, current_c_seg %p",c_seg_num,current_c_seg);
            composite_branch = composite->get_branch_by_segment(current_c_seg);   // CompositeBranch of segment
            logger->debug("composite branch %p",composite_branch);
            // Update weights
            composite_weight = composite_branch->get_confidence();
            combined_weight = reconstruction_weight + composite_weight;
        }
        logger->debug4("looping through aligned pairs 3, combined_weight %f",combined_weight);

        // Weighted average of seg1 and seg2
        // #IMPORTANT: currently a single marker can be aligned with multiple, and thus the weight effect can be multiplied (especially when the last marker of a short segment is aligned to multuple in the longer). Need to fix this!
        
        if (prev_r == r_marker){
            
        }
        else if (prev_c == c_marker){
            
        }
        
        c_marker->x = (r_marker->x * reconstruction_weight + c_marker->x * composite_weight)/combined_weight;
        c_marker->y = (r_marker->y * reconstruction_weight + c_marker->y * composite_weight)/combined_weight;
        c_marker->z = (r_marker->z * reconstruction_weight + c_marker->z * composite_weight)/combined_weight;
        c_marker->radius = (r_marker->radius * reconstruction_weight + c_marker->radius * composite_weight)/combined_weight;
        
        // Mapping from marker in first to marker in second
        // #CONSIDER: I don't think I need this for anything, was used in BlastNeuron to generate swc link from consensus points to points from either reconstruction
        logger->debug4("looping through aligned pairs 4");
        map_segment[merged_seg2[marker_index_alignments[i].first]] = merged_seg1[marker_index_alignments[i].second];
        
        prev_c = c_marker;
        prev_r = r_marker;
    }
    logger->debug2("Finished looping through aligned pairs, about to split branches");
    
    // Split composite branches and segments, store for assigning matches, and rebin new composite branches
    std::vector<CompositeBranchContainer *> composite_subbranches; // Each branch in this vector will be matched to the corresponding branch in recon_subbranches
    CompositeBranchContainer * c_bottom;
    for (NeuronSegment * seg : segments1){
        c_bottom = composite->get_branch_by_segment(seg);
        int num_orig_markers = seg->markers.size();
        logger->debug4("CURRENT 1");
        if (tree1_splits[seg].size() > 0){
            unbin_branch(c_bottom);
            std::size_t last_split_point = 0;
            logger->debug4("CURRENT 2");
            for (std::size_t split_point : tree1_splits[seg]){
                logger->debug4("CURRENT 2.1");
                // HACK, consider how to handle better
                if (split_point - last_split_point == 1)
                    split_point = last_split_point + 2;
                if (split_point - last_split_point == num_orig_markers - 1)
                    split_point = num_orig_markers - 2;
                CompositeBranchContainer * new_branch = c_bottom->split_branch(split_point - last_split_point);
                last_split_point = split_point;
                logger->debug4("CURRENT 2.2");
                composite_subbranches.push_back(new_branch);
                bin_branch(new_branch);
            }
            bin_branch(c_bottom);
        }
        logger->debug4("CURRENT 3");
        composite_subbranches.push_back(c_bottom);
    }
    logger->debug3("Done splitting composite branches, now on to recon branches");
    // Split reconstruction branches
    std::vector<BranchContainer *> recon_subbranches;
    BranchContainer * r_bottom;
    for (NeuronSegment * seg : segments2){
        int num_orig_markers = seg->markers.size();
        r_bottom = reconstruction->get_branch_by_segment(seg);
        if (tree2_splits[seg].size() > 0){
            unbin_branch(r_bottom);
            std::size_t last_split_point = 0;
            for (std::size_t split_point : tree2_splits[seg]){
                // HACK, consider how to handle better
                if (split_point - last_split_point == 1)
                    split_point = last_split_point + 2;
                if (split_point - last_split_point == num_orig_markers - 1)
                    split_point = num_orig_markers - 2;
                BranchContainer * new_branch = r_bottom->split_branch(split_point - last_split_point);
                last_split_point = split_point;
                recon_subbranches.push_back(new_branch);
                // No need to bin these as they will not be searched again
            }
        }
        recon_subbranches.push_back(r_bottom);
    }
    logger->debug2("Split recon branches");
    
    if (composite_subbranches.size() != recon_subbranches.size()){
        logger->error("Composite and reconstruction branches after splitting do not match!");
    }
    
    // Assign branches to their matched composite branches (after having made splits), adding new connections
    for (int i = 0; i < composite_subbranches.size(); i++){
        CompositeBranchContainer * c_branch = composite_subbranches[i];
        BranchContainer * r_branch = recon_subbranches[i];
        c_branch->add_branch_match(r_branch, true);
        r_branch->set_composite_match(c_branch);

        // Remove each segment from unmatched segment sets
        logger->debug3("Assigning branches and composites 5");
        unmatched_tree1_segments.erase(c_branch->get_segment());
        unmatched_tree2_segments.erase(r_branch->get_segment());
        logger->debug3("Assigning branches and composites 6");
    }
    
    logger->debug3("Assigned branches and composite branches after splitting");
    
    // Put marker->marker map for this match into segment_maps
    segment_maps.push_back(map_segment);
};

// Available results, will require calculation for some
// Final consensus(es)
// Consensus Reconstruction (SWC)
// Overall confidence
// Connection-level confidence
// Branch-level confidence
// Branch or point-level spatial variability
// Level of contribution by each input reconstruction (accuracy?)
// Level of contribution for each branch and connection can be calculated by request?
/**
 * Split composite branch based on where alignment starts and ends
 **/
CompositeBranchContainer * ConsensusBuilder::split_branch(CompositeBranchContainer * branch, Match * match, std::vector<CompositeBranchContainer *> &resulting_branches){
    NeuronSegment * segment = branch->get_segment();

    std::size_t const split_point1 = match->seg1_start();
    std::size_t const split_point2 = match->seg1_end() + 1;

    if (split_point1 == 0 && split_point2 == segment->markers.size()){
        // Match captures all of the branch, so no need to split it
        return branch;
    }
    
    // Remove segment from registration bins
    unbin_branch(branch);
    
    logger->debug("ConsensusBuilder::split_branch(CompositeBranchContainer*) alignment start and end %i %i",split_point1,split_point2-1);
    logger->debug("segment length %i",segment->markers.size());
    CompositeBranchContainer * new_above, * target_branch;
    if (split_point1 > 0){
        CompositeBranchContainer * new_above = branch->split_branch(split_point1);
        resulting_branches.push_back(new_above);
        logger->debug1("Split branch above start of alignment %p %p",new_above,new_above->get_segment());
        // Bin new segment above
        bin_branch(new_above);
    }
    if (split_point2 < segment->markers.size()){
        logger->debug1("About to split branch above end of alignment %i %i",split_point2, segment->markers.size());
        target_branch = branch->split_branch(split_point2);
        resulting_branches.push_back(target_branch);
        logger->debug1("Split branch above end of alignment %p %p",target_branch,target_branch->get_segment());
        
        // Bin new middle segment (or top segment if alignment starts at beginning of segment)
        bin_branch(target_branch);
    }else{
        target_branch = branch;
    }

    // Bin the new bottom-most branch
    logger->debug("Rebinning original branch, now shorter");
    bin_branch(branch);
    resulting_branches.push_back(branch);
    
    return target_branch;
};
BranchContainer * ConsensusBuilder::split_branch(BranchContainer * branch, Match * match, std::vector<BranchContainer *> &resulting_branches){
    NeuronSegment * segment = branch->get_segment();
    std::size_t const split_point1 = match->seg2_start();
    std::size_t const split_point2 = match->seg2_end() + 1;

    if (match->seg2_start() == 0 && match->seg2_end() == segment->markers.size()){
        // Match captures all of the branch, so no need to split it
        return branch;
    }
    
    // Remove segment from registration bins
    unbin_branch(branch);
    
    BranchContainer * new_above, * target_branch;
    if (split_point1 > 0){
        BranchContainer * new_above = branch->split_branch(split_point1);
        resulting_branches.push_back(new_above);

        // Bin new segment above
        //bin_branch(new_above);
    }
    if (split_point2 < segment->markers.size()){
        target_branch = branch->split_branch(split_point2);
        resulting_branches.push_back(target_branch);
        
        // Bin new middle segment (or top segment if alignment starts at beginning of segment)
        //bin_branch(target_branch);
    }else{
        target_branch = branch;
    }
    
    resulting_branches.push_back(branch);
    // Bin the new bottom-most branch
    //bin_branch(branch);
    
    return target_branch;
};

/* Add missed branches to composite and update composite branch confidence_denominators */
void ConsensusBuilder::incorporate_unassigned_branches(Reconstruction * reconstruction){
    // First calculate summed confidence of reconstructions that have already gone
    logger->debug2("calculated summed confidence of reconstructions");
    double prev_summed_confidence = 0;
    for (Reconstruction * prev_recon : composite->get_reconstructions()){
        prev_summed_confidence += prev_recon->get_confidence();
    }
    // Go through all branches in the list
    logger->debug2("Go through all unmatched reconstruction branches %i",unmatched_tree2_segments.size());
    //for (BranchContainer * branch : unmatched_branches.first){
    while(!unmatched_tree2_segments.empty()){
    //for (NeuronSegment * b_seg : unmatched_tree2_segments){
        NeuronSegment * b_seg = *(unmatched_tree2_segments.begin());
        logger->debug2("First segment %p",b_seg);
        BranchContainer * branch = reconstruction->get_branch_by_segment(b_seg);
        logger->debug2("Its branch %p",branch);

        BranchContainer * prev_branch = branch;
        logger->debug3("init test branch and composite match %p %p",branch,branch->get_composite_match());
        // From the current branch, go up until finding where the subtree connects to the composite structure
        while (branch && !branch->get_composite_match()){
            prev_branch = branch;
            branch = branch->get_parent();
            logger->debug3("test branch %p",branch);
            if (branch) logger->debug3("composite match %p",branch->get_composite_match());
        }
        // Hook prev_branch and its descendents onto branch's composite match
        logger->debug3("Hook last_branch and its descendents onto branch's composite match");
        std::stack<BranchContainer *> branch_stack;
        branch_stack.push(prev_branch);
        while (!branch_stack.empty()){
            logger->debug4("TEST 0");
            BranchContainer * orphan = branch_stack.top();
            logger->debug4("TEST 0.1");
            NeuronSegment * orphan_segment = orphan->get_segment();
            branch_stack.pop();
            logger->debug4("TEST 0.2 oprhan %p segment %p",orphan,orphan_segment);
            
            if (unmatched_tree2_segments.find(orphan_segment) == unmatched_tree2_segments.end()){
                logger->debug4("Came to a branch that has been found (not an orphan)");
                continue;
            }
            logger->debug4("TEST 1");
            unmatched_tree2_segments.erase(orphan_segment);
            
            // Copy segment and create composite branch out of it
            NeuronSegment * composite_segment = copy_segment_markers(orphan_segment);
            logger->debug4("composite has %i segments",composite->get_segments().size());
            CompositeBranchContainer * composite_branch = new CompositeBranchContainer(composite_segment, composite);
            logger->debug4("added branch, composite has %i segments",composite->get_segments().size());
            composite_branch->add_branch_miss(prev_summed_confidence);
            logger->debug4("TEST 3");
            
            // Create links between branch and composite branch
            composite_branch->add_branch_match(orphan, true);
            orphan->set_composite_match(composite_branch);
            logger->debug4("TEST 4");
            
            for (BranchContainer *child : orphan->get_children()){
                logger->debug4("TEST 4 child %p segment %p",child);
                logger->debug4("TEST 4 segment %p",child->get_segment());
                if (unmatched_tree2_segments.find(child->get_segment()) != unmatched_tree2_segments.end())
                    branch_stack.push(child);
            }
            logger->debug4("TEST 5");
            
        }
    }
    
    // For unmatched segments from the composite (tree1), mark each with a miss by the current reconstruction (tree2)
    logger->debug2("Get recon confidence");
    double recon_conf = reconstruction->get_confidence();
    logger->debug3("Add misses to increase denominator");
    logger->debug1("Number of misses: %d",unmatched_tree1_segments.size());
    for (NeuronSegment * c_seg : unmatched_tree1_segments){
        CompositeBranchContainer * branch = composite->get_branch_by_segment(c_seg);
        branch->add_branch_miss(recon_conf);
    }
};

void ConsensusBuilder::create_connections(Reconstruction * reconstruction){
    logger->debug("create_connections");
    // Go through all branches in the list
    logger->debug2("Go through all branches, num_segs %i",reconstruction->get_segments().size());
    //for (BranchContainer * branch : unmatched_branches.first){
    BranchContainer *r_branch, *r_parent;
    CompositeBranchContainer *c_branch, *c_parent;
    BranchEnd child_end, parent_end;
    for (NeuronSegment * b_seg : reconstruction->get_segments()){
        logger->debug4("b_seg %p",b_seg);
        r_branch = reconstruction->get_branch_by_segment(b_seg);
        logger->debug4("c_c 1");
        r_parent = r_branch->get_parent();
        
        logger->debug4("c_c 2");
        c_branch = r_branch->get_composite_match();
        c_parent = nullptr;
        parent_end = BOTTOM;
        bool match_forward = c_branch->is_match_forward(r_branch);
        logger->debug3("r_parent %p",r_parent);
        if (r_parent){
            c_parent = r_parent->get_composite_match();
            parent_end = match_forward ^ c_parent->is_segment_reversed() ? BOTTOM : TOP;
        }
        
        //c_branch->set_parent(c_parent);
        child_end = match_forward ^ c_branch->is_segment_reversed() ? TOP : BOTTOM;
        c_branch->add_connection(child_end, c_parent, parent_end, reconstruction, r_branch->get_confidence());
    }
    logger->debug4("End of create_connections; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
    logger->debug4("End of create_connections; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());
};


/** Incomplete! But it should compile **/
NeuronSegment * ConsensusBuilder::build_consensus_sorted(){
    std::vector<std::vector<double> > blastneuron_distances(cb_reconstructions.size(), std::vector<double>(cb_reconstructions.size()));
    // Run BlastNeuron for first reconstruction against all others
    int recon_count = 0;
    pair<string, Reconstruction *> first_recon_pair = *(cb_reconstructions.begin());
    Reconstruction * first_recon = first_recon_pair.second, * reconstruction;
    string first_name = first_recon_pair.first, name;
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        name = map_pairs.first;
        reconstruction = map_pairs.second;
        //blastneuron_distances
    }
    
    // Calculate additional distances
    
    // Determine median and order to process
    
    
    // Begin aligning reconstructions and merging
    std::stack<int> next_merge;
    Composite * composite;

    
    
    return composite->get_root_segment();
};

/** Incomplete! But it should compile so ignore for now **/
NeuronSegment * ConsensusBuilder::build_consensus_hierarchically(){
    std::vector< std::vector<int> > hierarchy(cb_reconstructions.size() * 2 - 1);
    // ^ First n are the original reconstructions, last index will have sentinel of -1 (root or hierarchy)
    
    std::vector<Composite *> composites(cb_reconstructions.size() - 1); // indexes are those of 'hierarchy' - num_recons
    
    std::vector<std::vector<double> > blastneuron_distances(cb_reconstructions.size(), std::vector<double>(cb_reconstructions.size()));
    // Run BlastNeuron for first reconstruction against all others
    int recon_count = 0;
    pair<string, Reconstruction *> first_recon_pair = *(cb_reconstructions.begin());
    Reconstruction * first_recon = first_recon_pair.second, * reconstruction;
    string first_name = first_recon_pair.first, name;
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        name = map_pairs.first;
        reconstruction = map_pairs.second;
        //blastneuron_distances
    }

    // Begin aligning reconstructions and merging
    std::stack<int> next_merge;
    Composite * composite;

    
    return composite->get_root_segment();
};

Composite * ConsensusBuilder::get_composite(){
    return composite;
};
/*
void ConsensusBuilder::set_match_score_threshold(double threshold){
    match_score_threshold = threshold;
};
void ConsensusBuilder::set_endpoint_threshold_distance(double threshold){
    endpoint_threshold_distance = threshold;
};
 */
bool ConsensusBuilder::is_ready_to_run(){
//    return ready_to_run;
    return builder_state >= RECONSTRUCTIONS_ADDED;
};

// Any two branches that come into close proximity to each other will each be split at their nearest point
void ConsensusBuilder::split_proximal_branches(Reconstruction * reconstruction, float distance_threshold){
    // Generate branch stack
    std::stack<BranchContainer *> branch_stack;
    std::vector<BranchContainer *> branch_vector;
    branch_stack.push(reconstruction->get_root_branch());
    while (!branch_stack.empty()){
        BranchContainer * branch = branch_stack.top();
        branch_stack.pop();
        branch_vector.push_back(branch);
        for (BranchContainer * child : branch->get_children()){
            branch_stack.push(child);
        }
    }
    
    for (int i = 0; i < branch_vector.size(); i++){
        BranchContainer * branch1 = branch_vector[i];
        NeuronSegment * segment1 = branch1->get_segment();
        std::set<BranchContainer *> b1_children = branch1->get_children();
        for (int j = i+1; j < branch_vector.size(); j++){
            BranchContainer * branch2 = branch_vector[j];
            NeuronSegment * segment2 = branch2->get_segment();
            std::set<BranchContainer *> b2_children = branch2->get_children();
            
            // Don't search for crossover if each branch is parent of the other
            if(std::find(b1_children.begin(), b1_children.end(), branch2) == b1_children.end() &&
               std::find(b2_children.begin(), b2_children.end(), branch1) == b2_children.end()) {
                
                MyMarker closest1, closest2;
                double segments_distance = segments_perpendicular_distance(
                                                                           *(segment1->markers.front()), *(segment1->markers.back()),
                                                                           *(segment2->markers.front()), *(segment2->markers.back()),
                                                                           closest1, closest2);
                
                if (segments_distance < distance_threshold){
                    MyMarker * split_marker1, * split_marker2;
                    
                    // First check whether either of the closest points are end points
                    bool closest1_is_end = false, closest2_is_end = false;
                    if (closest1.x == segment1->markers.front()->x && closest1.y == segment1->markers.front()->y && closest1.z == segment1->markers.front()->z){
                        closest1_is_end = true;
                        split_marker1 = segment1->markers.front();
                    }else if (closest1.x == segment1->markers.back()->x && closest1.y == segment1->markers.back()->y && closest1.z == segment1->markers.back()->z){
                        closest1_is_end = true;
                        split_marker1 = segment1->markers.back();
                    }
                    
                    if (closest2.x == segment2->markers.front()->x && closest2.y == segment2->markers.front()->y && closest2.z == segment2->markers.front()->z){
                        closest2_is_end = true;
                        split_marker2 = segment2->markers.front();
                    }else if (closest2.x == segment2->markers.back()->x && closest2.y == segment2->markers.back()->y && closest2.z == segment2->markers.back()->z){
                        closest2_is_end = true;
                        split_marker2 = segment2->markers.back();
                    }
                    
                    // If nearest point in either case is not an end point, find the nearest marker and split the branch
                    if (!closest1_is_end){
                        // Find the marker on branch2 closest to closest2
                        float nearest_distance = 10000000;
                        for (MyMarker * marker2 : segment2->markers){
                            float marker_dist = dist(*marker2,closest2);
                            if (marker_dist < nearest_distance){
                                split_marker2 = marker2;
                                nearest_distance = marker_dist;
                            }
                        }
                        // Check to make sure nearest marker wasn't an end point
                        if (split_marker2 != segment2->markers.front() && split_marker2 != segment2->markers.back()){
                            
                            /* Split branch and segment */
                            
                            bool hit_split_point = false;
                            std::vector<MyMarker *> markers = segment2->markers;
                            NeuronSegment * new_segment = new NeuronSegment();
                            BranchContainer * new_branch = new BranchContainer(branch2->get_reconstruction(), new_segment, nullptr, nullptr, branch2->get_confidence());
                            for (BranchContainer * child : branch2->get_children()){
                                new_branch->add_child(child); // Removes child from its former parent
                            }
                            branch2->add_child(new_branch);
                            reconstruction->add_branch(new_branch);
                            // Add new branch to the branch stack
                            branch_vector.push_back(new_branch);
                            
                            int marker_index = 0;
                            MyMarker * marker;
                            do {
                                marker = segment2->markers[marker_index++];
                            } while (marker != split_marker2);
                            new_segment->markers = std::vector<MyMarker *>(segment2->markers.begin() + marker_index, segment2->markers.end());
                            segment2->markers.erase(segment2->markers.begin() + marker_index, segment2->markers.end());
                            /*
                             for (MyMarker * marker : markers){
                             if (!hit_split_point){
                             if (marker == split_marker2){
                             hit_split_point = true;
                             }
                             }else{
                             // Remove this marker from the current segment
                             branch2->get_segment()->markers.erase(marker);
                             
                             // Add to the new segment
                             new_segment->markers.push_back(marker);
                             }
                             }
                             std::reverse(new_segment->markers.begin(),new_segment->markers.end());
                             */
                        }
                    }
                    if (!closest2_is_end){
                        // Just need to find the marker on branch1 closest to closest1
                        float nearest_distance = 10000000;
                        for (MyMarker * marker1 : segment1->markers){
                            float marker_dist = dist(*marker1,closest1);
                            if (marker_dist < nearest_distance){
                                split_marker1 = marker1;
                                nearest_distance = marker_dist;
                            }
                        }
                        // Check to make sure nearest marker wasn't an end point
                        if (split_marker1 != segment1->markers.front() && split_marker1 != segment1->markers.back()){
                            
                            /* Split branch and segment */
                            
                            bool hit_split_point = false;
                            NeuronSegment * new_segment = new NeuronSegment();
                            BranchContainer * new_branch = new BranchContainer(branch1->get_reconstruction(), new_segment, nullptr, nullptr, branch1->get_confidence());
                            for (BranchContainer * child : branch1->get_children()){
                                new_branch->add_child(child); // Removes child from its former parent
                            }
                            branch2->add_child(new_branch);
                            reconstruction->add_branch(new_branch);
                            // Add new branch to the branch stack
                            branch_vector.push_back(new_branch);
                            
                            int marker_index = 0;
                            MyMarker * marker;
                            do {
                                marker = segment1->markers[marker_index++];
                            } while (marker != split_marker1);
                            new_segment->markers = std::vector<MyMarker *>(segment1->markers.begin() + marker_index, segment1->markers.end());
                            segment1->markers.erase(segment1->markers.begin() + marker_index, segment1->markers.end());
                            /*
                             
                             for (MyMarker * marker : markers){
                             if (!hit_split_point){
                             if (marker == split_marker1){
                             hit_split_point = true;
                             }
                             }else{
                             // Remove this marker from the current segment
                             branch1->get_segment()->markers.erase(marker);
                             
                             // Add to the new segment
                             new_segment->markers.push_back(marker);
                             }
                             }
                             std::reverse(new_segment->markers.begin(),new_segment->markers.end());
                             */
                        }
                    }
                }
            }
        }
    }
};

// Any branch that curves sufficiently (min distance from swc node to line segment between end points > threshold)
// will be split at the local maximum.
void ConsensusBuilder::split_curved_branches(Reconstruction * reconstruction, float curve_distance_threshold){
    std::stack<BranchContainer *> branch_stack;
    branch_stack.push(reconstruction->get_root_branch());
    //    printf("SCB Test1\n");
    while (!branch_stack.empty()){
        BranchContainer * branch = branch_stack.top();
        NeuronSegment * branch_seg = branch->get_segment();
        branch_stack.pop();
        
        //        printf("SCB Test2 %p %p\n",branch,branch->get_segment());
        //        printf("SCB Test2.0 %i\n",branch->get_segment()->markers.size());
        bool split = false;
        // Branch must have at least 3 markers
        if (branch->get_segment()->markers.size() > 2){
            // Get ends of segment
            MyMarker * tip = branch_seg->markers.back();
            MyMarker * tail = branch_seg->markers.front();
            
            // Follow curve
            double last_dist = 0;
            std::vector<MyMarker *> markers = branch_seg->markers;
            std::reverse(markers.begin(), markers.end());
            //            printf("SCB Test2.1\n");
            int split_index = -1;
            for (int index = 0; index < markers.size() && split_index == -1; index++){
                MyMarker * marker = markers[index];
                // Calculate shortest distance from curve point to line segment between branch ends
                //                printf("SCB Test2.2\n");
                MyMarker closest_on_line;
                double curr_dist = point_segment_distance(*tip, *tail, *marker, closest_on_line);
                //                printf("SCB Test2.3\n");
                
                // If the previous distance was the local maximum and was greater than threshold
                if (last_dist > curve_distance_threshold && last_dist > curr_dist){
                    split_index = index;
                }
                
            }
            //            printf("SCB Test3\n");
            
            if (split_index != -1){
                // Split segment at the current marker
                NeuronSegment * new_segment = new NeuronSegment();
                BranchContainer * new_branch = new BranchContainer(branch->get_reconstruction(), new_segment, branch, nullptr, branch->get_confidence());
                std::set<BranchContainer *> children = branch->get_children();
                for (BranchContainer * child : children){
                    new_branch->add_child(child); // Removes child from its former parent
                }
                reconstruction->add_branch(new_branch);
                
                // Add the new branch back to the stack
                branch_stack.push(new_branch);
                
                // Move markers from old to new segment
                new_segment->markers = std::vector<MyMarker *>(branch_seg->markers.begin() + split_index, branch_seg->markers.end());
                branch_seg->markers.erase(branch_seg->markers.begin() + split_index, branch_seg->markers.end());
            }
        }
        //        printf("SCB Test4\n");
        
        if (split == -1){
            // Add children to be split in subsequent iterations
            for (BranchContainer * child : branch->get_children()){
                branch_stack.push(child);
            }
        }
    }
};

double point_segment_distance(MyMarker s1, MyMarker s2, MyMarker p, MyMarker &closest_on_line){
    float p_diff_x = p.x - s1.x; //A
    float p_diff_y = p.y - s1.y; //B
    float p_diff_z = p.z - s1.z;
    
    float s_diff_x = s2.x - s1.x; //C
    float s_diff_y = s2.y - s1.y; //D
    float s_diff_z = s2.z - s1.z; //D
    
    float dot = p_diff_x * s_diff_x + p_diff_y * s_diff_y + p_diff_z * s_diff_z;
    float len_sq = s_diff_x * s_diff_x + s_diff_y * s_diff_y + s_diff_z * s_diff_z;
    float param = dot / len_sq;
    
    float xx,yy,zz;
    
    if(param < 0){
        xx = s1.x;
        yy = s1.y;
        zz = s1.z;
    }
    else if(param > 1){
        xx = s2.x;
        yy = s2.y;
        zz = s2.z;
    } else {
        xx = s1.x + param * s_diff_x;
        yy = s1.y + param * s_diff_y;
        zz = s1.z + param * s_diff_z;
    }
    closest_on_line.x = xx;
    closest_on_line.y = yy;
    closest_on_line.z = zz;
    
    //float dist = d(x,y,xx,yy);
    double point_dist = dist(p,closest_on_line);
    return point_dist;
};

double segments_perpendicular_distance(MyMarker l11, MyMarker l12, MyMarker l21, MyMarker l22, MyMarker &closest1, MyMarker &closest2){
    // Calculate direction vectors for each segment
    MyMarker l1_dir(l12.x-l11.x,l12.y-l11.y,l12.z-l11.z);
    MyMarker l2_dir(l22.x-l21.x,l22.y-l21.y,l22.z-l21.z);
    
    // Calculate the cross product of the direction vectors
    MyMarker cross_prod(l1_dir.y * l2_dir.z - l1_dir.z * l2_dir.y,
                        l1_dir.z * l2_dir.x - l1_dir.x * l2_dir.z,
                        l1_dir.x * l2_dir.y - l1_dir.y * l2_dir.x);
    double magnitude = sqrt((double)cross_prod.x*cross_prod.x + cross_prod.y*cross_prod.y + cross_prod.z*cross_prod.z);
    // Calculate the normalized cross product
    MyMarker cp_unit(cross_prod.x/magnitude, cross_prod.y/magnitude, cross_prod.z/magnitude);
    
    // Find points P and Q, the points at which segments 1 and 2 are closest
    // P is given by (l1_dir.x * t + l11.x, l1_dir.y * t + l11.y, l1_dir.z * t + l11.z)
    // Q is given by (l2_dir.x * s + l21.x, l2_dir.y * s + l21.y, l2_dir.z * s + l21.z)
    //  vector Q-P x l1_dir = 0 and vector Q-P x l2_dir = 0
    MyMarker point_vect(l11.x-l21.x,l11.y-l21.y,l11.z-l21.z);
    double s1 = l2_dir.x * l1_dir.x + l2_dir.y * l1_dir.y + l2_dir.z * l1_dir.z;
    double t1 = -l1_dir.x * l1_dir.x - l1_dir.y * l1_dir.y - l1_dir.z * l1_dir.z;
    double const1 = - (point_vect.x * l1_dir.x + point_vect.y * l1_dir.y + point_vect.z * l1_dir.z);
    double s2 = l2_dir.x * l2_dir.x + l2_dir.y * l2_dir.y + l2_dir.z * l2_dir.z;
    double t2 = -l1_dir.x * l2_dir.x - l1_dir.y * l2_dir.y - l1_dir.z * l2_dir.z;
    double const2 = -(point_vect.x * l2_dir.x + point_vect.y * l2_dir.y + point_vect.z * l2_dir.z);
    
    // Systems of equations = matrix [s1, t1], solve for t and s
    //                               [s2, t2]
    double determinant = 1 / (s1*t2 - t1*s2);
    // Inverse matrix is det * [ t2, -t1]
    //                         [-s2,  s1]
    double s = t2*const1 - t1*const2;
    double t = -s2*const1 + s1*const2;
    
    // In order for P and Q to be on the segments, t and s must be between 0 and 1
    if (s > 0 && t > 0 && s < 1 && t < 1){
        closest1.x = l11.x + l1_dir.x * t;
        closest1.y = l11.y + l1_dir.y * t;
        closest1.z = l11.z + l1_dir.z * t;
        closest2.x = l21.x + l2_dir.x * s;
        closest2.y = l21.y + l2_dir.y * s;
        closest2.z = l21.z + l2_dir.z * s;
        return point_vect.x * cp_unit.x + point_vect.y * cp_unit.y + point_vect.z * cp_unit.z;
    } else {
        // Calculate distance from either tip to the other segment
        MyMarker close1, close2, close3, close4;
        double dist1 = point_segment_distance(l11,l12,l21,close1);
        double dist2 = point_segment_distance(l11,l12,l22,close2);
        double dist3 = point_segment_distance(l21,l22,l11,close3);
        double dist4 = point_segment_distance(l21,l22,l12,close4);
        if (dist1 < dist2 && dist1 < dist3 && dist1 < dist4){
            closest1 = close1;
            closest2 = l21;
        }else if (dist2 < dist1 && dist2 < dist3 && dist2 < dist4){
            closest1 = close2;
            closest2 = l22;
        }else if (dist3 < dist1 && dist3 < dist2 && dist3 < dist4){
            closest1 = l11;
            closest2 = close3;
        }else{
            closest1 = l12;
            closest2 = close4;
        }
        return std::min(dist1,std::min(dist2,std::min(dist3,dist4)));
    }
    //    return -1;
    
    // With t and s we can get P and Q and determine if each are on the line segments
    //MyMarker P(l1_dir.x * t + l11.x, l1_dir.y * t + l11.y, l1_dir.z * t + l11.z);
    //MyMarker Q(l2_dir.x * s + l21.x, l2_dir.y * t + l21.y, l2_dir.z * t + l21.z);
};

/**
 * Tree vectors are expected to be ordered such that a branch always comes after all of its children, with the root at the back
 * Weights are positive, neuron_tree_align maximizing the total score
 **/
double ConsensusBuilder::neuron_tree_align(vector<NeuronSegment*> &tree1, vector<NeuronSegment*> &tree2, Reconstruction * reconstruction, vector<double> & w, vector<pair<int, int> > & result)
{
    return neuron_tree_align(tree1, tree2, segments_by_recon_cube[reconstruction], w, result);
}
double ConsensusBuilder::neuron_tree_align(vector<NeuronSegment*> &tree1, vector<NeuronSegment*> &tree2, Composite * composite, vector<double> & w, vector<pair<int, int> > & result)
{
    return neuron_tree_align(tree1, tree2, segments_by_composite_cube[composite], w, result);
}
double ConsensusBuilder::neuron_tree_align(vector<NeuronSegment*> &tree1, vector<NeuronSegment*> &tree2, std::vector<std::set<NeuronSegment *> > segments_by_cube, vector<double> & w, vector<pair<int, int> > & result)
{
    // Indices of branches, sets giving branch merges with a child, sets 1ll<<child_num1 and 1ll<<child_num2 giving max score for pair
    // weights gives scores for a branch pair and each branch's possible merge with one of its children
    map<SetIndexPair, double> weights;
    // results contains a vector of branch index pairs giving all aligned branch pairs
    map<SetIndexPair, vector<pair<int, int> > >  results;
    
    int nrows = tree1.size();
    int ncols = tree2.size();
    
    //    printf("nrows %i, ncols %i\n",nrows,ncols);
    
    map<NeuronSegment*, long> ind_map1, ind_map2;
    for(long ind1 = 0; ind1 < tree1.size(); ind1++) ind_map1[tree1[ind1]] = ind1;
    for(long ind2 = 0; ind2 < tree2.size(); ind2++) ind_map2[tree2[ind2]] = ind2;
    
    for(long ind1 = 0; ind1 < tree1.size(); ind1++)
    {
        NeuronSegment * node1 = tree1[ind1];
        int child_num1 = node1->child_list.size();
        
        SegmentPtrSet nearby_segments  = get_nearby_segments(node1, segments_by_cube);
        logger->debug4("neuron_tree_align tree ind1 %i, nearby_segments: %i",ind1,nearby_segments.size());
        for(long ind2 = 0; ind2 < tree2.size(); ind2++)
        {
            NeuronSegment * node2 = tree2[ind2];
            if (nearby_segments.find(node2) == nearby_segments.end()) continue;

            int child_num2 = node2->child_list.size();
            
            //            printf("Node1 childlist size %i, node2 childlist size %i, weight %f\n",child_num1,child_num2, w[ind1 * ncols + ind2]);
            
            SetIndexPair sip;
            sip.ind1 = ind1;
            sip.ind2 = ind2;
            
            // Compare all the subset align
            // Each set gives a combination of the branch of ind1 with one of its children, position 0 giving the branch alone without its children
            for(long set1 = 0; set1 < (1ll<<child_num1); set1++)
            {
                sip.set1 = set1;
                vector<int> set1_bits; // all bits index of set1 with value 1
                // Determines which children are available for matching for the current set
                // set = 1 -> child 0; set = 2 -> child 1; set = 3 -> children 0 and 1
                for(int b = 0; b < child_num1; b++) if(set1 & (1ll << b)) set1_bits.push_back(b);
                
                for(long set2 = 0; set2 < (1ll<<child_num2); set2++)
                {
                    sip.set2 = set2;
                    vector<int> set2_bits; // all bits index of set2 with value 1
                    for(int b = 0; b < child_num2; b++) if(set2 & (1ll << b)) set2_bits.push_back(b);
                    
                    if(set1 == 0 && set2 == 0)
                    {
                        weights[sip] = 0.0;
                        results[sip] = vector<pair<int, int> >();
                        continue;
                    }
                    
                    double max_val = 0.0;
                    SetIndexPair max_sip1, max_sip2;
                    
                    // When set1 == 0 but set2 > 0 max_sip1 and max_sip2 will be default initialized: ind1=0, ind2=0, set1=0, set2=0
                    //      weights will have a value of 0 (zero-initialized), results will have empty vector of pair<int,int>
                    // When set1 > 0 and set2 == 0, _sip1 gets set, but nothing else happens in terms of max_val, max_sip1, and max_sip2 - same as above
                    //      But then why run this if either set1 or set2 are 0? Seems like wasted operations.
                    
                    // Picking best assignment of children via looking at all possible assignments
                    // first item
                    for(long i = 0; i < set1_bits.size(); i++)
                    {
                        // The bit gives which child to take;
                        // Set=0: no children, Set=1: child 0, Set=2: child 1, Set=3 (if >1 children): children 0 and 1
                        long b1 = set1_bits[i];
                        SetIndexPair _sip1; // Match
                        _sip1.ind1 = ind1;
                        // For set in 1,2 returns 0, if set=3 gets the inverse set number associated with the bit (0:1->2, 1:2->1)
                        // Ex: Set=1: 01 & ~01 = 01&10 = 0;
                        // Ex: Set=3, b1 = 0: 11 & ~01 = 11 & 10 = 2 (b1 assoc set=1, get set=2)
                        // Ex: Set=3, b1 = 1: 11 & ~10 = 11 & 01 = 1 (b1 assoc set=2, get set=1)
                        _sip1.set1 = set1 & (~(1ll<<b1));
                        _sip1.ind2 = ind2;
                        
                        for(long j = 0; j < set2_bits.size(); j++)
                        {
                            long b2 = set2_bits[j];
                            _sip1.set2 = set2 & (~(1ll<<b2));        // clear bit j
                            
                            SetIndexPair _sip2;                   // A - {a_i}, B - {b_j}
                            _sip2.ind1 = ind_map1[node1->child_list[b1]];                   // the index of  C(a_i)
                            _sip2.ind2 = ind_map2[node2->child_list[b2]];                   // the index of  C(a_i)
                            _sip2.set1 = 1ll << tree1[_sip2.ind1]->child_list.size();
                            _sip2.set2 = 1ll << tree2[_sip2.ind2]->child_list.size();
                            // weights[_sip1] is 0 if set is 1 or 2;
                            // If set is 3, weights[_sip1] gives weight of opposite child pair match, weights[_sip2] of previous set iteration
                            double val = weights[_sip1] + weights[_sip2];
                            if(val > max_val)
                            {
                                max_val = val;
                                max_sip1 = _sip1;
                                max_sip2 = _sip2;
                            }
                        }
                    }
                    
                    weights[sip] = max_val;
                    results[sip].insert(results[sip].begin(), results[max_sip1].begin(), results[max_sip1].end());
                    results[sip].insert(results[sip].begin(), results[max_sip2].begin(), results[max_sip2].end());
                }
            }
            /* calcute wTT - Here is where joining of parent and child segments is made possible */
            {
                // For storing the optimal match case with set values above those from previous section
                sip.set1 = 1ll << child_num1;
                sip.set2 = 1ll << child_num2;
                double max_val = -1.0;
                SetIndexPair max_sip;
                // wTT rule 3: initialize max_sip with the last/optimal set from previous child matching section
                max_sip.ind1 = ind1;
                max_sip.ind2 = ind2;
                max_sip.set1 = (1ll << child_num1) - 1;
                max_sip.set2 = (1ll << child_num2) - 1;
                max_val = weights[max_sip];
                // wTT rule 1 - See if best match of node1's children with node 2 is better than child matchings
                for(int ai = 0; ai < child_num1; ai++)
                {
                    SetIndexPair _sip;
                    _sip.ind1 = ind_map1[node1->child_list[ai]];
                    _sip.set1 = 1ll << tree1[_sip.ind1]->child_list.size();
                    _sip.ind2 = ind2;
                    _sip.set2 = 1ll << tree2[_sip.ind2]->child_list.size();
                    double val = weights[_sip];
                    if(val > max_val)
                    {
                        max_val = val;
                        max_sip = _sip;
                    }
                }
                // wTT rule 2 - See if best match of node2's children with node 1 is better than prev matchings
                for(int bj = 0; bj < child_num2; bj++)
                {
                    SetIndexPair _sip;
                    _sip.ind1 = ind1;
                    _sip.set1 = 1ll << tree1[_sip.ind1]->child_list.size();
                    _sip.ind2 = ind_map2[node2->child_list[bj]];
                    _sip.set2 = 1ll << tree2[_sip.ind2]->child_list.size();
                    double val = weights[_sip];
                    if(val > max_val)
                    {
                        max_val = val;
                        max_sip = _sip;
                    }
                }
                
                // Weight of segment 1 and segment 2 + weight of child connections
                weights[sip] = w[ind1 * ncols + ind2] + max_val;
                results[sip] = results[max_sip];
                results[sip].push_back(pair<long, long>(ind1, ind2));
            }
        }
    }
    SetIndexPair root_sip;
    root_sip.ind1 = tree1.size() - 1;
    root_sip.ind2 = tree2.size() - 1;
    root_sip.set1 = 1ll << tree1[root_sip.ind1]->child_list.size();
    root_sip.set2 = 1ll << tree2[root_sip.ind2]->child_list.size();
    result = results[root_sip];
    return weights[root_sip];
};

