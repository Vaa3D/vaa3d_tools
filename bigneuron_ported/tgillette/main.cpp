//
//  main.cpp
//  ConsensusBuilder
//
//  Created by Todd Gillette on 6/5/15.
//
//

#include <stdio.h>
#include <iostream>
#include <stack>
#include <vector>
#include <map>
#include "Reconstruction.h"
#include "Composite.h"
#include "ConsensusBuilder.h"
#include "tree_matching/my_surf_objs.h"

/** Main function **/
int main(int argc, char* argv[]){
    // Get directory from arguments
    if (argc < 3){
      //        std::cout << "Usage is: ConsensusBuilder <indir> <outfile>\n"; // Inform the user of how to use the program
      printf("Usage is: ConsensusBuilder <indir> <outfile> [Debug Level]\n"); // Inform the user of how to use the program
      //        std::cin.get();
        exit(0);
        //return -1;
    }
    printf("Opening directory\n");

    //    DIR * directory = opendir(argv[1]);
    string directory = argv[1];
    
    printf("Creating ConsenusBuilder\n");
    ConsensusBuilder builder(directory);
    if (argc > 3){
        int loglevel = atoi(argv[3]);
        printf("Setting log level %i\n",loglevel);
        //builder = ConsensusBuilder(directory,loglevel);
        builder.set_log_level(loglevel);
    }else{
        //builder = ConsensusBuilder(directory);
    }
    
    printf("Created ConsensusBuilder\n");
    builder.set_match_score_threshold(1);
    

    printf("Building consensus\n");
    NeuronSegment * consensus_root = builder.build_consensus();
    
    if (consensus_root){
        // Make vector of MyMarker*, each pointing to its parent (#TODO: move into ConsensusBuilder, take care of MyMarker pointers inside as well)
        std::stack<NeuronSegment*> segment_stack;
        std::vector<MyMarker*> map_swc;
        segment_stack.push(consensus_root);
        MyMarker * prev_marker = nullptr;
        std::map<NeuronSegment*,NeuronSegment*> parent_map;
        printf("Generating final consensus for writing to swc\n");
        while (!segment_stack.empty()){
            NeuronSegment * segment = segment_stack.top();
            segment_stack.pop();
            if (parent_map[segment]){
                //prev_marker = parent_map[segment]->markers.back();
                prev_marker = parent_map[segment]->markers[0];
            }
            //for (MyMarker * marker : segment->markers){
            for (std::vector<MyMarker *>::reverse_iterator rit = segment->markers.rbegin();
                 rit != segment->markers.rend(); ++rit){
                MyMarker * marker = *rit;
                marker->parent = prev_marker;
                prev_marker = marker;
                map_swc.push_back(marker);
            }
            for (NeuronSegment * child : segment->child_list){
                segment_stack.push(child);
                parent_map[child] = segment;
            }
        }

        // Write consensus to swc file
        saveSWC_file(argv[2], map_swc);
        string message = "Wrote file to "+string(argv[2])+"\n";
        std::cout << message;
    }else{
        printf("ConsensusBuilder not ready, see previous messages for reason\n");
    }
    return 0;
}

