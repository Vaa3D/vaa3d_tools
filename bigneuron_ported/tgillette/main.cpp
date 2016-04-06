//
//  main.cpp
//  ConsensusBuilder
//
//  Created by Todd Gillette on 6/5/15.
//
//

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stack>
#include <vector>
#include <map>
//#include <thread>
//#include <csignal>
#include "Reconstruction.h"
#include "Composite.h"
#include "ConsensusBuilder.h"
#include "tree_matching/my_surf_objs.h"

void sigint_handler(int sig)
{
    std::cout << "handling signal no. " << sig << "\n";
}

/** Main function **/
int main(int argc, char* argv[]){
/*
    // Get directory from arguments
    if (argc < 3){
      //        std::cout << "Usage is: ConsensusBuilder <indir> <outfile>\n"; // Inform the user of how to use the program
      printf("Usage is: ConsensusBuilder <indir> <outfile> [Debug Level]\n"); // Inform the user of how to use the program
      //        std::cin.get();
        exit(0);
        //return -1;
    }
  */
    string directory = "", outfile_orig = "";
    int loglevel = 0;
    double confidence_threshold = -1, rescue_threshold = 0,
        scale = 1,
        zscale = 1,
        euclidean_threshold = -1,
        angle_threshold = -1;
    bool single_tree = false;
    
    for (int i = 1; i < argc; i++) {
        // Check parameters
        if (i + 1 != argc){ // Check that we haven't finished parsing already
            std::cout << argv[i] << endl;
            if (strcmp(argv[i],"-d") == 0 || strcmp(argv[i],"--directory") == 0) {
                directory = string(argv[++i]);
                std::cout << "-d " << directory << endl;
            } else if (strcmp(argv[i],"-o") == 0 || strcmp(argv[i],"--output") == 0) {
                outfile_orig = string(argv[++i]);
                std::cout << "-o " << outfile_orig << endl;
            } else if (strcmp(argv[i],"-s") == 0 || strcmp(argv[i],"--scale") == 0) {
                scale = stod(argv[++i]);
                std::cout << "-s " << scale << endl;
            } else if (strcmp(argv[i],"-z") == 0 || strcmp(argv[i],"--z-scale") == 0) {
                zscale = stod(argv[++i]);
                std::cout << "-z " << zscale << endl;
            } else if (strcmp(argv[i],"-l") == 0 || strcmp(argv[i],"--log-level") == 0) {
                loglevel = stoi(argv[++i]);
                std::cout << "-l " << loglevel << endl;
            } else if (strcmp(argv[i],"-e") == 0 || strcmp(argv[i],"--euclidean") == 0) {
                euclidean_threshold = stod(argv[++i]);
                std::cout << "-e " << euclidean_threshold << endl;
            } else if (strcmp(argv[i],"-a") == 0 || strcmp(argv[i],"--angle") == 0) {
                angle_threshold = stod(argv[++i]);
                std::cout << "-a " << angle_threshold << endl;
            } else if (strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--confidence") == 0) {
                confidence_threshold = stod(argv[++i]);
                std::cout << "-c " << confidence_threshold << endl;
            } else if (strcmp(argv[i],"-r") == 0 || strcmp(argv[i],"--rescue")) {
                rescue_threshold = stod(argv[++i]);
                std::cout << "-r " << rescue_threshold << endl;
            } else if (strcmp(argv[i],"--single-tree") == 0) {
                single_tree = true;
                std::cout << "--single-tree" << endl;
            }
        }
    }
    
    bool arg_error = false;
    if (directory.empty()){
        arg_error = true;
        printf("Argument -d <swc_directory> required\n");
    }
    if (outfile_orig.empty()){
        arg_error = true;
        printf("Argument -o <outfile> required\n");
    }
    if (arg_error)
        exit(0);
    
    bool eswc = outfile_orig.substr(outfile_orig.length() - 4) == "eswc";
    printf("Opening directory\n");
    
    try{
        ConsensusBuilder builder(directory, loglevel);
        printf("Created ConsensusBuilder\n");
        builder.set_match_score_threshold(1);
        builder.set_scale(scale);
        builder.set_z_scale(zscale);
        if (euclidean_threshold > 0){
            builder.set_euclidean_dist_threshold(euclidean_threshold);
        }
        if (angle_threshold > 0){
            builder.set_angle_threshold(angle_threshold);
        }
        
        // Start thread to handle keyboard input
        bool finished = false;
        
        printf("Building consensus\n");
        
        string outfile;
        
        if (confidence_threshold == -1){

            /*
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.eswc");
             builder.write_consensus_to_eswc(outfile, 0, 0, true);
             
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.4.eswc");
             builder.write_consensus_to_eswc(outfile, 0.2, 0, true);
             
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0_TypeByBranch_Proportion.swc");
             builder.write_consensus_to_swc(outfile, 0, BRANCH_CONFIDENCE, PROPORTION, 0, true);
             */
            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.2.eswc");
            builder.write_consensus_to_eswc(outfile, 0.2, 0, true);

            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.2_OneTree.eswc");
            builder.write_consensus_to_eswc(outfile, 0.2, 0, false);

            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.25.eswc");
            builder.write_consensus_to_eswc(outfile, 0.25, 0, true);

            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.2_TypeByBranch_Proportion.swc");
            builder.write_consensus_to_swc(outfile, 0.2, BRANCH_CONFIDENCE, PROPORTION, 0, true);
            
            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.2_OneTree_TypeByBranch_Proportion.swc");
            builder.write_consensus_to_swc(outfile, 0.2, BRANCH_CONFIDENCE, PROPORTION, 0, false);
            
            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.25_TypeByBranch_Proportion.swc");
            builder.write_consensus_to_swc(outfile, 0.25, BRANCH_CONFIDENCE, PROPORTION, 0, true);
            /*
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.2_2_TypeByBranch_Proportion.swc");
             builder.write_consensus_to_swc(outfile, 0.2, BRANCH_CONFIDENCE, PROPORTION, 2, true);
             
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.4_TypeByBranch_Proportion.swc");
             builder.write_consensus_to_swc(outfile, 0.4, BRANCH_CONFIDENCE, PROPORTION, 0, true);
             */
            /*
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0_TypeByConn_Proportion.swc");
             builder.write_consensus_to_swc(outfile, 0, CONNECTION_CONFIDENCE, PROPORTION, 0, true);
             
             outfile = outfile_orig;
             outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0.25_TypeByConn_Proportion.swc");
             builder.write_consensus_to_swc(outfile, 0.25, CONNECTION_CONFIDENCE, PROPORTION, 0, true);
             */
        /*
            //        outfile = outfile_orig;
            //        outfile.replace(outfile.end()-3,outfile.end(),"_ConfThresh0_TypeByBranch_Proportion.swc");
            //        builder.write_consensus_to_swc(outfile, 0, BRANCH_CONFIDENCE, PROPORTION);
            
            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh1_TypeByBranch_Proportion.swc");
            builder.write_consensus_to_swc(outfile, 1, BRANCH_CONFIDENCE, PROPORTION);
            
            //        outfile = outfile_orig;
            //        outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh0_TypeByConn_Proportion.swc");
            //        builder.write_consensus_to_swc(outfile, 0, CONNECTION_CONFIDENCE, PROPORTION);
            
            outfile = outfile_orig;
            outfile.replace(outfile.end()-3,outfile.end(),"ConfThresh1_TypeByConn_Proportion.swc");
            builder.write_consensus_to_swc(outfile, 1, CONNECTION_CONFIDENCE, PROPORTION);
         */
        }else{
            string conf_thresh_str = confidence_threshold > 1 ? to_string((int)confidence_threshold) : to_string(confidence_threshold);
            outfile = outfile_orig;
            if (eswc){
                outfile.replace(outfile.end()-4,outfile.end(),"BranchConfThresh"+conf_thresh_str+".eswc");
                builder.write_consensus_to_eswc(outfile, confidence_threshold, rescue_threshold, !single_tree);
            }else{
                outfile.replace(outfile.end()-3,outfile.end(),"BranchConfThresh"+conf_thresh_str+"_TypeByBranch_Proportion.swc");
                builder.write_consensus_to_swc(outfile, confidence_threshold, BRANCH_CONFIDENCE, PROPORTION, rescue_threshold, !single_tree);
            }
        }
        // Print out the names of the reconstructions that were included
        printf("Reconstructions included in the consensus\n");
        for (string name : builder.get_reconstruction_names()){
            printf("%s\n",name.c_str());
            printf("%i trees\n",builder.get_reconstruction(name)->get_trees().size());
        }
        
    } catch (std::string e){
        printf(e.c_str());
    }
    return 0;
}
