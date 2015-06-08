/*
 *  BigNeuron_Consensus_Builder.h
 *  BigNeuron Consensus Builder
 *
 *  Created by Todd Gillette on 5/26/15.
 *  Copyright (c) 2015 Todd Gillette. All rights reserved.
 *
 */

//extern "C" {
#include <map>
#include <vector>
#include <dirent.h>
//#include <CoreFoundation/CoreFoundation.h>
#include "Reconstruction.h"
#include "Composite.h"
#include "ActionHook.h"
    
    
    typedef void (*ScriptFunction)(void); // function pointer type
    typedef std::map<ActionHook, ScriptFunction> script_map;
    
    typedef pair<string,Reconstruction*> ReconstructionPair;
    
    class ConsensusBuilder{
        bool readyToRun = false;
        std::map<string,Reconstruction*> cb_reconstructions;
        Composite composite;
        double match_score_threshold;
    public:
        ConsensusBuilder();
        ConsensusBuilder(DIR * directory); // TODO Finds all swc files
        ConsensusBuilder(FILE * anoFile); // TODO Load specified ano file
        ConsensusBuilder(vector<NeuronSegment*> reconstruction_root_segments);
        void clear(); // Clear reconstructions and composite, allowing for a restart
        void set_reconstructions(vector<NeuronSegment*> reconstruction_root_segments);
        void set_reconstructions(vector<Reconstruction*> reconstructions);
        void set_reconstructions(std::map<string,Reconstruction*> reconstructions);
        void add_reconstruction(NeuronSegment* reconstruction_root_segment, string name, double confidence=1);
        void add_reconstruction(Reconstruction* reconstruction);
       // void register_action(HOOK_NAME hook_name, HOOK_POSITION position, ScriptFunction func);
        void set_match_score_threshold(double threhsold);

        /** Primary function - builds consensus **/
        Reconstruction build_consensus();

        std::vector<Reconstruction*> get_reconstructions();
        Composite get_composite();
        CompositeBranchContainer split_composite_branch(BranchContainer * splitting_branch, CompositeBranchContainer * branch, NeuronSegment * top_segment);
        void split_branch_matches(BranchContainer * splitting_branch, CompositeBranchContainer * branch, CompositeBranchContainer * child);
    };

//}
