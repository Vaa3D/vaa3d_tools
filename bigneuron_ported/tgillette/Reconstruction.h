//
//  Reconstruction.h
//  BigNeuron Consensus Builder
//
//  Contains class definitions for the Composite and related objects, including wrappers for reconstruction elements
//
//  Created by Todd Gillette on 5/27/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#ifndef BigNeuron_Consensus_Builder_Reconstruction_h
#define BigNeuron_Consensus_Builder_Reconstruction_h

#include <vector>
#include <set>
#include <map>
#include <stack>
#include "Poll.h"
//#include "Composite.h"
#include "tree_matching/swc_utils.h"

class Reconstruction;
class BranchTip;
class BranchContainer;
class Connection;
class DecisionPoint;
class Composite;

class CompositeBranchContainer;

class Reconstruction {
    string r_name;
    NeuronSegment * r_tree;
    std::vector<NeuronSegment *> r_segments;
    std::map<NeuronSegment *,BranchContainer *> segment_container_map;
    BranchContainer * r_root; // Should point to r_tree
    double r_confidence;

    void create_branch_container_trees(BranchContainer * root_branch);
public:
    Reconstruction();
    Reconstruction(NeuronSegment * tree);
    Reconstruction(string name, NeuronSegment * tree, double confidence=1);
  //  Reconstruction(string name, BranchContainer * branchRoot, double confidence=1);
    
    void update_branch_by_segment(NeuronSegment * segment, BranchContainer * branch);
    void update_constituents();
    
    void set_name(string name);
    void set_tree(NeuronSegment * tree);
    void set_confidence(double confidence);

    string get_name();
    NeuronSegment * get_tree();
    std::vector<NeuronSegment *> get_segments();
    BranchContainer * get_root_branch();
    BranchContainer * get_branch_by_segment(NeuronSegment * segment);
    double get_confidence();
};

class BranchTip {
    BranchContainer * branch;
    bool is_parent;
    std::vector<BranchContainer *> connected_branches;
};
/*
class Connection {
    BranchContainer * con_parent;
    BranchContainer * con_child;
    
public:
    Connection(BranchContainer * parent, BranchContainer * child){
        con_parent = parent;
        con_child = child;
    }
    BranchContainer * get_parent(){
        return con_parent;
    };
    BranchContainer * get_child(){
        return con_child;
    };
};*/

/**
 * Contains the confidence value of the branch, a pointer to the branch data, and pointers to either end's deicision point
 */
class BranchContainer {
protected:
    Reconstruction * bc_reconstruction;
    NeuronSegment * bc_segment;
    BranchContainer * bc_parent;
    std::set<BranchContainer *> bc_children;
    CompositeBranchContainer * bc_composite_match;
    double bc_confidence;
    /*
    Connection * parent_connection;
    std::set<Connection *> child_connections;
*/
public:
    // Contructors
    BranchContainer();
    BranchContainer(Reconstruction * reconstruction);
    BranchContainer(Reconstruction * reconstruction, NeuronSegment * segment, BranchContainer * parent=nullptr, CompositeBranchContainer * composite_match=nullptr, double confidence=-1);

    // Setters
    void set_segment(NeuronSegment * segment);
    void set_reconstruction(Reconstruction * reconstruction);
    void set_parent(BranchContainer * parent);
    void add_child(BranchContainer * branch);
    void set_children(std::set<BranchContainer *> children);
    void set_composite_match(CompositeBranchContainer * composite_match);
    void set_confidence(double confidence);
    void remove_child(BranchContainer * child);
    
    // Getters
    NeuronSegment * get_segment();
    BranchContainer * get_parent();
    std::set<BranchContainer *> get_children();
    CompositeBranchContainer * get_composite_match();
    double get_confidence();
    Connection * get_parent_connection();
    std::set<Connection *> get_child_connection();
    Reconstruction * get_reconstruction();
    
    bool operator < (const BranchContainer& branch) const
    {
        return (bc_confidence < branch.bc_confidence);
    }
};

std::vector<NeuronSegment *> produce_segment_vector(NeuronSegment &root){
    std::stack<NeuronSegment *> segment_stack;
    segment_stack.push(&root);
    std::vector<NeuronSegment *> segments;
    NeuronSegment * segment;
    while (!segment_stack.empty()){
        segment = segment_stack.top();
        segment_stack.pop();
        segments.push_back(segment);
    }
    return segments;
}

std::map<NeuronSegment *, BranchContainer *> produce_segment_container_map(BranchContainer &container_root){
    std::map<NeuronSegment *, BranchContainer *> sb_map;
    std::stack<BranchContainer *> branch_stack;
    branch_stack.push(&container_root);
    BranchContainer * branch;
    while (!branch_stack.empty()){
        branch = branch_stack.top();
        branch_stack.pop();
        sb_map.insert(pair<NeuronSegment *,BranchContainer *>(branch->get_segment(), branch));
    }
    return sb_map;
}

#endif
