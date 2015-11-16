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
#include <dirent.h>
#include <stdio.h>
#include <cstdio>
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
                    for (MyMarker * marker : neuronPts){
                        printf("%i %f %f %f %f\n",marker->type,marker->x,marker->y,marker->z,marker->radius);
                    }
                    
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
        delete recon_pair.second;
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
    logger->debug2("Finding ranges for registration bins");
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        string name = map_pairs.first;
        logger->debug3("Reconstruction %s",name.c_str());
        reconstruction = map_pairs.second;
        logger->debug4("%i segments",reconstruction->get_segments().size());
        for (NeuronSegment * segment : reconstruction->get_segments()){
            logger->debug4("%i markers",segment->markers.size());
            for (MyMarker * marker : segment->markers){
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
    logger->debug2("Entering bin_branches for %s",reconstruction->get_name().c_str());
    segments_by_recon_cube[reconstruction] = std::vector<SegmentPtrSet>(total_bins);
    std::vector<SegmentPtrSet> * segments_by_cube = &(segments_by_recon_cube[reconstruction]);
    logger->debug4("generated segments_by_cube, %i total bins",total_bins);

    for (NeuronSegment * segment : reconstruction->get_segments()){
        std::set<int> * search_cubes = &(search_cubes_by_segment[segment]);
        int marker_num = 0;
        for (MyMarker * marker : segment->markers){
            // Sample markers rather than running each one (based on initial marker resample rate and cube size)
            if (marker_num++ % marker_sample_rate == 0){
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
                }

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
        logger->debug4("Running segment");
        std::set<int> * search_cubes = &(search_cubes_by_segment[segment]);
        logger->debug4("Got search cubes; segment %p",segment);
        logger->debug4("Num markers %i",segment->markers.size());
        int marker_num = 0;
        for (MyMarker * marker : segment->markers){
            // Sample markers rather than running each one (based on initial marker resample rate and cube size)
            if (marker_num++ % marker_sample_rate == 0){
                // Determine the cube this marker is in
                logger->debug4("Running get_bin(marker) on %p",marker);
                int center_cube_index = get_bin(marker);
                // Convert to x, y, z indices
                std::vector<int> xyz = indexToXyz(center_cube_index);
                logger->debug4("center cube index %i",center_cube_index);
                // Get all adjacent cubes
                for (int x = std::max(0, xyz[0] - 1); x < std::min((int)x_bin_positions.size(), xyz[0] + 1); x++){
                    for (int y = std::max(0, xyz[1] - 1); y < std::min((int)y_bin_positions.size(), xyz[1] + 1); y++){
                        for (int z = std::max(0, xyz[2] - 1); z < std::min((int)z_bin_positions.size(), xyz[2] + 1); z++){
                            // Store cubes to search for this segment
                            logger->debug4("Inserting cube index %i for composite segment %p",xyzToIndex(x,y,z),segment);
                            search_cubes->insert(xyzToIndex(x,y,z));
                        }
                    }
                }
                // Add this segment to the cube it is in
                segments_by_cube->at(center_cube_index).insert(segment);
                logger->debug4("Added %p to center cube; set size %i",segment,segments_by_cube->at(center_cube_index).size());
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
        if (marker_num++ % marker_sample_rate == 0){
            // Determine the cube this marker is in
            int center_cube_index = get_bin(marker);
            // Convert to x, y, z indices
            std::vector<int> xyz = indexToXyz(center_cube_index);
            // Get all adjacent cubes
            for (int x = std::max(0, xyz[0] - 1); x < std::min((int)x_bin_positions.size(), xyz[0] + 1); x++){
                for (int y = std::max(0, xyz[1] - 1); y < std::min((int)y_bin_positions.size(), xyz[1] + 1); y++){
                    for (int z = std::max(0, xyz[2] - 1); z < std::min((int)z_bin_positions.size(), xyz[2] + 1); z++){
                        // Store cubes to search for this segment
                        logger->debug4("Inserting cube index %i for segment %p",xyzToIndex(x,y,z),segment);
                        search_cubes->insert(xyzToIndex(x,y,z));
                    }
                }
            }
            // Add this segment to the cube it is in
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
    ind_it = std::lower_bound(y_bin_positions.begin(), y_bin_positions.end(), marker->y);
    int y_ind = ind_it - y_bin_positions.begin() - 1;
    ind_it = std::lower_bound(z_bin_positions.begin(), z_bin_positions.end(), marker->z);
    int z_ind = ind_it - z_bin_positions.begin() - 1;
    
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
        logger->debug4("Searching cube %i",cube);
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
    // Branch cutting for simplifying alignments (prior to assignment of cubes)
    int recon_count = 0;
    for (pair<string, Reconstruction *> map_pairs : cb_reconstructions){
        logger->info("Cutting branches on reconstruction %i",recon_count);
        string name = map_pairs.first;
        Reconstruction * reconstruction = map_pairs.second;
        //logger->debug4("Reconstruction ptr %p",reconstruction);
        logger->debug4("recon segs %i, recon branches",reconstruction->get_segments().size());
        
        // #TODO: Test the following two functions
        // Split highly curved branches at maximal distance from straight line
        reconstruction->split_curved_branches();
        logger->debug4("After split_curved_branches");
        // Split branches where they come very close or pass by another branch (this will make aligning and assigning simpler)
        reconstruction->split_proximal_branches();
        logger->debug4("After split_proximal_branches");
        recon_count++;
    }
    
    
    // Assign branches to bins (spatial cubes) in order to minimize set of possible branch-branch assignments
    bin_branches();
    
    // Determine similarity of trees to set order of processing, reducing growth rate of composite
    //sort_trees();
    // ALTERNATIVELY: Generate a binary tree indicating which trees to combine, further reducing extraneous alignment attempts
    //build_hierarchy();
    
    builder_state = PREPROCESSED;
}

void ConsensusBuilder::build_composite(){
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
        for (NeuronSegment *seg : reconstruction_segments){
            logger->debug4("seg size %i",seg->markers.size());
        }
        
        recon_count++;
        // Initialize composite as first reconstruction
        if (recon_count == 1){
            composite = new Composite(reconstruction);
            bin_branches(composite);
            logger->info("Initialized composite using reconstruction with %d segments, composite has %d",reconstruction->get_segments().size(), composite->get_segments().size());
        }else{
            // First run BlastNeuron to align reconstruction with composite and get confident alignment
            std::vector<pair<int, int> > result;
            
            logger->debug4("Beginning of build_composite; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("Middle of build_composite; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

            //printf("composite reconstruction %p",composite->get_composite_reconstruction());
            std::vector<NeuronSegment *> composite_segments = composite->get_segments_ordered();
            logger->debug2("Got composite segments %d",composite_segments.size());
            for (NeuronSegment *seg : composite_segments){
                logger->debug4("seg size %i",seg->markers.size());
            }

            // Create a set of segments from the composite
            std::set<NeuronSegment *> composite_segments_set(composite_segments.begin(), composite_segments.end());
            
            // Calculate weights for each branch pair, higher weight is better for aligning
            std::vector<double> weights = calculate_weights(composite_segments, reconstruction_segments, reconstruction);
            for (double weight : weights){
                logger->debug4("weight %f",weight);
            }
            
            /**
             * Align composite and reconstruction using code from BlastNeuron (in neuron_tree_align.h)
             **/
            logger->debug("Running alignment, num branches: %d and %d",composite_segments.size(),reconstruction_segments.size());
            neuron_tree_align(composite_segments, reconstruction_segments, weights, result);
            // 'result' is a vector of segment index pairs from reconstructionSegments and compositeSegments respectively
            
            logger->debug("Ran alignment, result ");
            
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
                std::vector<MyMarker*> tmp_seg;
                
                logger->debug1("i %i; cluster1[i].size %i; cluster2[i].size %i",i,seg_clusters1[i].size(),seg_clusters2[i].size());
                
                //the clustered segments should be in increasing order, so just concatinate them
                for (int j=0;j<seg_clusters1[i].size();j++)
                {
                    tmp_seg.clear();
                    NeuronSegment * segment = composite_segments[seg_clusters1[i][j]];
                    new_seg1.insert(new_seg1.end(), segment->markers.begin(), segment->markers.end());

                    // This branch is matched (tentatively), so remove it from unmatched_branches
                    CompositeBranchContainer * branch = composite->get_branch_by_segment(segment);
                }
                for (int j=0;j<seg_clusters2[i].size();j++)
                {
                    tmp_seg.clear();
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
                std::vector<pair<int, int> > marker_index_alignments;
                std::vector<MyMarker*> seg1 = pairs_merged[i].first;
                std::vector<MyMarker*> seg2 = pairs_merged[i].second;
                std::vector<MyMarker*> average;
                
                // #CONSIDER: return alignment distances and take score as median, max, or 95th% distance
                logger->debug1("seg1 and 2 size: %d %d",seg1.size(),seg2.size());
                logger->debug1("Aligning seg1 %f %f %f; seg2 %f %f %f",seg1[0]->x,seg1[0]->y,seg1[0]->z,seg2[0]->x,seg2[0]->y,seg2[0]->z);
                /* Run curve alignment */
                double align_dist = seg_weight(seg1, seg2, marker_index_alignments);
                double align_score = average_alignment_dist - align_dist/marker_index_alignments.size();
                logger->debug1("marker_index_alignments size %d, align_dist %f",marker_index_alignments.size(), align_dist);
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
                    
                    logger->debug2("Averaging and splitting alignments");
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
            logger->debug3("calling 'match_remaining_branches' if there are any in unmatched_tree2_segments - %i",unmatched_tree2_segments.size());
            if (unmatched_tree2_segments.size() > 0)
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
            
            /** Generate consensus **/
            
            logger->debug("Generating intermediary consensus");
            //Composite * new_composite = composite->generate_consensus(0); // #TODO: plug new consensus in as base composite structure
            // Update the NeuronSegment tree structure given the information of the new reconstruction
            composite->update_tree();
            
            logger->debug4("After update_tree; check root_branch %p %i",composite->get_root(),composite->get_root()->get_children().size());
            logger->debug4("After update_tree; check root_segment %p %i",composite->get_root_segment(),composite->get_root_segment()->markers.size());

        }
    }
};

void ConsensusBuilder::match_remaining_branches(Reconstruction * reconstruction){
    logger->debug1("Enter match_remaining_branches");
    std::set<Match *> matches = find_matches_local_alignment();
    
    // First, make association between branches and composite branches based on match specifications
    for (Match * match : matches){
        CompositeBranchContainer * c_branch = composite->get_branch_by_segment(match->seg1);
        BranchContainer * r_branch = reconstruction->get_branch_by_segment(match->seg2);
        
        c_branch->add_branch_match(r_branch, match->forward);
        r_branch->set_composite_match(c_branch);
    }
    
    // Second, reprocess matches to create connections
    for (Match * match : matches){
        /* Now all connections are being generated after all branches are in place
        CompositeBranchContainer * c_branch = composite->get_branch_by_segment(match->seg1);
        BranchContainer * r_branch = reconstruction->get_branch_by_segment(match->seg2);
        
        if (r_branch->get_parent()){
            CompositeBranchContainer * c_parent = r_branch->get_parent()->get_composite_match();
            if (c_parent){
                // Connection ends depend both on the direction of the current match (relative to the current composite direction), and the current composite direction (for child and parent independently) relative to the original composite direction
                BranchEnd child_end = match->forward ^ c_branch->is_segment_reversed() ? TOP : BOTTOM;
                BranchEnd parent_end = match->forward ^ c_parent->is_segment_reversed() ? BOTTOM : TOP;
                c_branch->add_connection(child_end, c_parent, parent_end, reconstruction, c_branch->get_confidence());
            }
        }
         */
        unmatched_tree1_segments.erase(match->seg1);
        unmatched_tree2_segments.erase(match->seg2);
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
    std::map<NeuronSegment *,std::vector<Match *> > matches_by_segment =
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
    std::map<Match *, pair<Match *, int> > small_overlaps;
    
    // Put conflicts into match_conflicts
    find_conflicts(unmatched_tree1_segments, matches_by_segment, match_conflicts, small_overlaps);
    find_conflicts(unmatched_tree2_segments, matches_by_segment, match_conflicts, small_overlaps);
    
    /* Process the conflict sets for possible combinations of assignments, and select optimal assignments for a given conflict set */
    
    // Create a set of ALL matches - will be used as set of matches that have not yet been processed
    std::set<Match *> unprocessed;
    for (std::map<NeuronSegment *,std::vector<Match *> >::iterator it = matches_by_segment.begin();
         it != matches_by_segment.end(); ++it){
        for (Match * match : it->second){
            unprocessed.insert(match);
        }
    }
    
    while (!unprocessed.empty()){
        
        Match * first_match = *(unprocessed.begin());
        if (match_conflicts[first_match]->size() == 0){
            final_assignments.insert(first_match);
            continue;
        }
        
        // Map of matches and whether they are assigned or not (T/F)
        std::map<Match *,bool> * assignment_map_ptr, * assignment_map_copy_ptr;
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
        
        
        while (!next_matches_stack.empty()){
            // Pull the current next_matches and assignments objects
            next_matches = next_matches_stack.top();
            next_matches_stack.pop();
            assignment_map_ptr = next_matches_map[next_matches];
            assignment_map = *assignment_map_ptr;
            
            // Loop until no stacks remain
            while (!next_matches->empty()){
                Match * candidate = next_matches->top();
                next_matches->pop();
                unprocessed.erase(candidate); // this match will now have been processed
                
                bool must_be_off = false;
                int count_off = 0;
                
                std::set<Match *> to_add;
                // First determine whether the candidate can be on and/or off (if the match has no conflict on either side, turn it ON)
                for (Match * conflict : *(match_conflicts[candidate])){
                    if (assignment_map.count(conflict)){
                        if (assignment_map[conflict]){
                            must_be_off = true;
                        }else{
                            count_off++;
                        }
                    }else {
                        // Add this match to those to be processed
                        next_matches->push(conflict);
                    }
                }
                if (must_be_off){
                    assignment_map[candidate] = false;
                }else if (count_off == match_conflicts[candidate]->size()){
                    // No conflicts remain for this match, so don't bother with the case of it being OFF
                    assignment_map[candidate] = true;
                    score_map[assignment_map_ptr] += candidate->score;
                }else{
                    // Can be on or off, need to duplicate the assignment map for each possibility
                    assignment_map_copy_ptr = new std::map<Match *,bool>();
                    std::map<Match *,bool> assignment_map_copy = *assignment_map_ptr;
                    assignment_map_copy = assignment_map;
                    next_matches_copy = new std::stack<Match *>();
                    *next_matches_copy = *next_matches;
                    
                    // Copy the score
                    score_map[assignment_map_copy_ptr] = score_map[assignment_map_ptr];
                    
                    // Link the new next_matches stack to its assignment map
                    next_matches_map[next_matches_copy] = assignment_map_copy_ptr;
                    // Set the copy to have this candidate OFF
                    assignment_map_copy[candidate] = false;
                    // Push the new next_matches stack onto the possiblities stack
                    next_matches_stack.push(next_matches_copy);
                    
                    // Set the original to have this candidate ON
                    assignment_map[candidate] = true;
                    score_map[assignment_map_ptr] += candidate->score;
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
        for (std::map<Match *,bool>::iterator it = best_assignments->begin(); it != best_assignments->end(); ++it){
            if (it->second){
                final_assignments.insert(it->first);
            }else{
                // Delete rejected candidate matches
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
    
    return final_assignments;
};

std::map<NeuronSegment *,std::vector<Match *> > ConsensusBuilder::generate_candidate_matches_via_local_align(std::set<NeuronSegment *> tree2_segments){
    /* Run local alignment on candidates [produce std::map<NeuronSegment *, Match *> matches_by_segment] */
    
    // Map from branch to all possible matches
    // Map<NeuronSegment *,vector<Match *> >; need to initialize for all segments of either side?
    std::map<NeuronSegment *,std::vector<Match *> > candidate_matches;
    std::map<NeuronSegment *,Match *> best_matches;
    
    // Attempt to find a match for each segment in the new tree (tree 2)
    for (NeuronSegment * seg2 : tree2_segments){
        // Get nearby composite segments for alignment
        std::set<NeuronSegment *> nearby_segments = get_nearby_segments(seg2, composite);
        for (NeuronSegment * seg1 : nearby_segments){
            logger->debug4("about to local align a branch and a composite branch!");
            Match * match = nullptr;
            // Run local alignment
            vector<pair<int, int> > * alignment = new vector<pair<int, int> >();
            double score = local_align(average_alignment_dist, seg1->markers, seg2->markers, *alignment, gap_cost);
            logger->debug4("Completed local_align with score %f", score);
            // Store the alignment: If alignment spans entire extent of one segment, or if alignment is long enough, or if per-character alignment score is good enough
            if (alignment->size()+1 >= seg1->markers.size() || alignment->size() > min_alignment_size && score/alignment->size() >= average_alignment_dist){
                logger->debug3("Good enough to create a Match");
                match = new Match();
                match->seg1 = seg1;
                match->seg2 = seg2;
                match->score = score;
                match->alignment = alignment;
                match->forward = true;
            }else{
                delete alignment;
            }
            
            // Local align with one branch reversed (if forward align score isn't high enough to rule out reverse - heuristic)
            if (score < good_enough_score){
                logger->debug4("Score not good enough, going to try reverse alignment");
                vector<pair<int, int> > * alignment_reverse = new vector<pair<int, int> >();
                vector<MyMarker *> seg2_reversed = seg2->markers;
                std::reverse(seg2_reversed.begin(), seg2_reversed.end());
                score = local_align(average_alignment_dist, seg1->markers, seg2_reversed, *alignment_reverse, gap_cost);
                if (match){ // Forward alignment was successful
                    if (score > match->score){ // Reverse alignment is better than forward alignment
                        delete match->alignment;
                        match->alignment = alignment_reverse;
                        match->forward = false;
                    }else{
                        delete alignment_reverse; // cleanup
                    }
                }else if (alignment_reverse->size()+1 >= seg1->markers.size() || alignment_reverse->size() > min_alignment_size && score/alignment_reverse->size() >= average_alignment_dist){
                    // Forward alignment failed, reverse alignment succeeded and produces a match
                    match = new Match();
                    match->seg1 = seg1;
                    match->seg2 = seg2;
                    match->score = score;
                    match->alignment = alignment_reverse;
                    match->forward = false;
                }else{
                    // Neither forward nor reverse alignment succeed, no match
                    delete alignment_reverse; // cleanup
                }
            }
            
            logger->debug4("Generated alignments, checking match");
            
            // If at least one of forward and reverse matches is good enough
            //    and if the segments don't have matches yet or the current match is within range of the best current matches:
            //       put best of forward and reverse matches into vector of matches
            if (match && (!best_matches[seg1] || match->score >= best_matches[seg1]->score - best_branch_margin) &&
                (!best_matches[seg2] || match->score >= best_matches[seg2]->score - best_branch_margin)){
                logger->debug4("match is good enough, putting it into candidate_matches");
                candidate_matches[seg1].push_back(match);
                candidate_matches[seg2].push_back(match);
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
                                      std::map<NeuronSegment *, vector<Match *> > matches_by_segment,
                                      std::map<Match *, std::set<Match *> * > &match_conflicts,
                                      std::map<Match *, pair<Match *, int> > &small_overlaps){
    for (NeuronSegment * segment : tree_segments){
        for (int i = 0; i < matches_by_segment[segment].size() - 1; i++){
            Match * match1 = matches_by_segment[segment][i];
            if (match_conflicts.count(match1) == 0){
                match_conflicts[match1] = new std::set<Match *>();
            }
            // Create sets of conflicts for a given match candidate
            for (int j = i+1; j < matches_by_segment[segment].size(); j++){
                Match * match2 = matches_by_segment[segment][j];
                int overlap_size = get_overlap_size(match1, match2, true);
                // No conflict
                if (overlap_size <= allowable_alignment_overlap){
                    // Store overlap size (within acceptable range)
                    if (overlap_size > 0){
                        small_overlaps[match1] = pair<Match *, int>(match2, overlap_size);
                        small_overlaps[match2] = pair<Match *, int>(match1, overlap_size);
                    }
                }else{
                    // Add the matches to each others' conflict sets
                    match_conflicts[match1]->insert(match2);
                    if (match_conflicts.count(match2) == 0){
                        match_conflicts[match2] = new std::set<Match *>();
                    }
                    match_conflicts[match2]->insert(match1);
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
    
    logger->debug4("Composite segments1[0].markers size %i, segments1[1].markers size %i",segments1[0]->markers.size(),segments1[1]->markers.size());
    logger->debug4("Composite segments1[0].markers[0] %p, segments1[1].markers[0] %p",segments1[0]->markers[0],segments1[1]->markers[0]);
    logger->debug4("Composite segments1[0].markers[last] %p, segments1[1].markers[last] %p",segments1[0]->markers.back(),segments1[1]->markers.back());
    logger->debug4("Recon segments1[0].markers size %i, segments1[1].markers size %i",segments2[0]->markers.size(),segments2[1]->markers.size());
    logger->debug4("Recon segments1[0].markers[0] %p, segments1[1].markers[0] %p",segments2[0]->markers[0],segments2[1]->markers[0]);
    logger->debug4("Recon segments1[0].markers[last] %p, segments1[1].markers[last] %p",segments2[0]->markers.back(),segments2[1]->markers.back());

    logger->debug4("Composite First marker %p, first marker of first segment %p", merged_seg1[marker_index_alignments[0].first], current_c_seg->markers[0]);
    logger->debug4("Reconstruction First marker %p, first marker of first segment %p", merged_seg2[marker_index_alignments[0].second], current_r_seg->markers[0]);
    //int j;
    for (int i=0; i < marker_index_alignments.size(); i++) // Loop through aligned point pairs
    {
        //j = i;
        
        c_marker = merged_seg1[marker_index_alignments[i].first];
        r_marker = merged_seg2[marker_index_alignments[i].second];
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
        c_marker->x = (r_marker->x * reconstruction_weight + c_marker->x * composite_weight)/combined_weight;
        c_marker->y = (r_marker->y * reconstruction_weight + c_marker->y * composite_weight)/combined_weight;
        c_marker->z = (r_marker->z * reconstruction_weight + c_marker->z * composite_weight)/combined_weight;
        c_marker->radius = (r_marker->radius * reconstruction_weight + c_marker->radius * composite_weight)/combined_weight;
        
        // Mapping from marker in first to marker in second
        // #CONSIDER: I don't think I need this for anything, was used in BlastNeuron to generate swc link from consensus points to points from either reconstruction
        logger->debug4("looping through aligned pairs 4");
        map_segment[merged_seg2[marker_index_alignments[i].first]] = merged_seg1[marker_index_alignments[i].second];
    }
    logger->debug2("Finished looping through aligned pairs, about to split branches");
    
    // Split composite branches and segments, store for assigning matches, and rebin new composite branches
    std::vector<CompositeBranchContainer *> composite_subbranches; // Each branch in this vector will be matched to the corresponding branch in recon_subbranches
    CompositeBranchContainer * c_bottom;
    for (NeuronSegment * seg : segments1){
        c_bottom = composite->get_branch_by_segment(seg);
        if (tree1_splits[seg].size() > 0){
            unbin_branch(c_bottom);
            for (std::size_t split_point : tree1_splits[seg]){
                CompositeBranchContainer * new_branch = c_bottom->split_branch(split_point);
                composite_subbranches.push_back(new_branch);
                bin_branch(new_branch);
            }
            bin_branch(c_bottom);
        }
        composite_subbranches.push_back(c_bottom);
    }
    logger->debug3("Done splitting composite branches, now on to recon branches");
    // Split reconstruction branches
    std::vector<BranchContainer *> recon_subbranches;
    BranchContainer * r_bottom;
    for (NeuronSegment * seg : segments2){
        r_bottom = reconstruction->get_branch_by_segment(seg);
        if (tree2_splits[seg].size() > 0){
            unbin_branch(r_bottom);
            for (std::size_t split_point : tree2_splits[seg]){
                BranchContainer * new_branch = r_bottom->split_branch(split_point);
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
CompositeBranchContainer * ConsensusBuilder::split_composite_branch(CompositeBranchContainer * branch, Match * match){
    NeuronSegment * segment = branch->get_segment();
    vector<pair<int,int> > alignment = *(match->alignment);
    if (alignment[0].first == 0 && alignment.back().first == segment->markers.size()-1){
        // Match captures all of the branch, so no need to split it
        return branch;
    }
    
    // Remove segment from registration bins
    unbin_branch(branch);
    
    std::size_t const split_point1 = alignment[0].first;
    std::size_t const split_point2 = alignment.back().first;
    CompositeBranchContainer * new_above, * target_branch;
    if (alignment[0].first > 0){
        CompositeBranchContainer * new_above = branch->split_branch(split_point1);
        
        // Bin new segment above
        bin_branch(new_above);
    }
    if (alignment.back().first < segment->markers.size()-1){
        target_branch = branch->split_branch(split_point2);
        
        // Bin new middle segment (or top segment if alignment starts at beginning of segment)
        bin_branch(target_branch);
    }else{
        target_branch = branch;
    }
    // Bin the new bottom-most branch
    bin_branch(branch);
    
    return target_branch;
};

/* Add missed branches to composite and update composite branch confidence_denominators */
void ConsensusBuilder::incorporate_unassigned_branches(Reconstruction * reconstruction){
    std::set<BranchContainer *> handled_branches;
    // First calculate summed confidence of reconstructions that have already gone
    logger->debug2("calculated summed confidence of reconstructions");
    double prev_summed_confidence = 0;
    for (Reconstruction * prev_recon : composite->get_reconstructions()){
        prev_summed_confidence += prev_recon->get_confidence();
    }
    // Go through all branches in the list
    logger->debug2("Go through all branches");
    //for (BranchContainer * branch : unmatched_branches.first){
    for (NeuronSegment * b_seg : unmatched_tree2_segments){
        BranchContainer * branch = reconstruction->get_branch_by_segment(b_seg);
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
                CompositeBranchContainer * composite_branch = new CompositeBranchContainer(composite_segment, composite);
                //composite->add_branch(composite_branch); // Done when creating new CBC in ^
                composite_branch->add_branch_miss(prev_summed_confidence);
                
                // Create links between branch and composite branch
                composite_branch->add_branch_match(orphan, true);
                orphan->set_composite_match(composite_branch);
                
                /* Now all connections are being generated after all branches are in place
                // Set composite parent and create a connection
                composite_branch->set_parent(composite_parent);
                BranchEnd parent_end = composite_parent->is_segment_reversed() ? TOP : BOTTOM;
                composite_branch->add_connection(TOP, composite_parent, parent_end, reconstruction, reconstruction->get_confidence());
                 */
            }
        }
    }
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
    logger->debug2("Go through all branches");
    //for (BranchContainer * branch : unmatched_branches.first){
    BranchContainer *r_branch, *r_parent;
    CompositeBranchContainer *c_branch, *c_parent;
    BranchEnd child_end, parent_end;
    for (NeuronSegment * b_seg : reconstruction->get_segments()){
        r_branch = reconstruction->get_branch_by_segment(b_seg);
        r_parent = r_branch->get_parent();
        
        c_branch = r_branch->get_composite_match();
        c_parent = nullptr;
        parent_end = BOTTOM;
        bool match_forward = c_branch->is_match_forward(r_branch);
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
