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

/** Main function **/
int main(int argc, char* argv[]){
    // Get directory from arguments
    if (argc < 3){
        std::cout << "Usage is: ConsensusBuilder <indir> <outfile>\n"; // Inform the user of how to use the program
        std::cin.get();
        exit(0);
        //return -1;
    }
    
    DIR * directory = opendir(argv[1]);
    ConsensusBuilder builder(directory);
    Reconstruction consensus = builder.build_consensus();
    
    // Make vector of MyMarker*, each pointing to its parent (#TODO: move into ConsensusBuilder, take care of MyMarker pointers inside as well)
    std::stack<BranchContainer*> branch_stack;
    std::vector<MyMarker*> map_swc;
    branch_stack.push(consensus.get_root_branch());
    MyMarker * prev_marker = nullptr;
    while (!branch_stack.empty()){
        BranchContainer * branch = branch_stack.top();
        branch_stack.pop();
        if (branch->get_parent()){
            prev_marker = branch->get_parent()->get_segment()->markers.back();
        }
        for (MyMarker * marker : branch->get_segment()->markers){
            marker->parent = prev_marker;
            prev_marker = marker;
            map_swc.push_back(marker);
        }
        for (BranchContainer * child : branch->get_children()){
            branch_stack.push(child);
        }
    }
    
    // Write consensus to swc file
    saveSWC_file(argv[2], map_swc);
    string message = "Wrote file to "+string(argv[2])+"\n";
    std::cout << message;
    return 0;
}

