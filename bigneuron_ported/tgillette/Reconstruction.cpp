//
//  Reconstruction.cpp
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/29/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#include <stack>
#include "Reconstruction.h"

/* Class Reconstruction */
Reconstruction::Reconstruction(){};
Reconstruction::Reconstruction(NeuronSegment * tree){
    r_tree = tree;
    BranchContainer container(this, tree);
};
Reconstruction::Reconstruction(string name, NeuronSegment * tree, double confidence){
    r_name = name;
    r_tree = tree;

    BranchContainer * container = new BranchContainer(this, tree);
    r_root = container;

    create_branch_container_trees(container);

    segment_container_map = produce_segment_container_map(container);

    r_segments = produce_segment_vector(r_tree);

    r_confidence = confidence;
};

void Reconstruction::update_constituents(){
    create_branch_container_trees(r_root);
    segment_container_map = produce_segment_container_map(r_root);
    r_segments = produce_segment_vector(r_tree);
};

void Reconstruction::create_branch_container_trees(BranchContainer * root_branch){
    stack<pair<NeuronSegment *,BranchContainer *> > stack;
    typedef pair<NeuronSegment *,BranchContainer *> SCPair;
    SCPair sc_pair;
    NeuronSegment * segment;
    BranchContainer * branch, * child_branch;
    stack.push(SCPair(root_branch->get_segment(), root_branch));
    while (!stack.empty()){
        sc_pair = stack.top();
        stack.pop();
        segment = sc_pair.first;
        branch = sc_pair.second;
        vector<NeuronSegment*> children = segment->child_list;
        for (NeuronSegment * child : children){
            // Creates a BC for the child segment as well as pointer to its BC parent, and for its parent back to it
            child_branch = new BranchContainer(this,child,branch,nullptr,r_confidence);
            if (child->child_list.size() > 0){
                stack.push(SCPair(child,child_branch));
            }
        }
    }
}
/*
Reconstruction::Reconstruction(string name, BranchContainer * branchRoot, double confidence){
    rName = name;
    rRoot = branchRoot;
    if (branchRoot->getSegment()){
        rTree = branchRoot->getSegment();
    }
    rSegments = produce_segment_vector(*rTree);
    rConfidence = confidence;
};*/
BranchContainer * Reconstruction::get_root_branch(){
    return get_branch_by_segment(r_tree);
}
void Reconstruction::set_name(string name){
    r_name = name;
};
string Reconstruction::get_name(){
    return r_name;
};
void Reconstruction::set_tree(NeuronSegment * tree){
    r_tree = tree;
};
NeuronSegment * Reconstruction::get_tree(){
    return r_tree;
};
std::vector<NeuronSegment *> Reconstruction::get_segments(){
    return r_segments;
};
typedef map<NeuronSegment *,BranchContainer *> SegmentContainerMap;
typedef pair<NeuronSegment *,BranchContainer *> SegmentContainerPair;
void Reconstruction::update_branch_by_segment(NeuronSegment * segment, BranchContainer * branch){
    segment_container_map[segment] = branch;
}

BranchContainer * Reconstruction::get_branch_by_segment(NeuronSegment * segment){
    return segment_container_map[segment];
};
void Reconstruction::set_confidence(double confidence){
    r_confidence = confidence;
};
double Reconstruction::get_confidence(){
    return r_confidence;
};


/**
 * Class BranchContainer
 * Contains the confidence value of the branch, a pointer to the branch data, and pointers to either end's deicision point
 */
BranchContainer::BranchContainer(){
    // Uncertain whether it is necessary to set these, but I want to be certain #CONSIDER: removing the nullptr assignments if they are indeed unncessary
    bc_reconstruction = nullptr;
    bc_segment = nullptr;
    bc_parent = nullptr;
};
BranchContainer::BranchContainer(Reconstruction * reconstruction){
    bc_reconstruction = reconstruction;
    bc_segment = nullptr;
    bc_parent = nullptr;
};
/*
BranchContainer::BranchContainer(NeuronSegment * segment, BranchContainer * parent){
    bc_segment = segment;
    bc_parent = parent;
};*/
BranchContainer::BranchContainer(Reconstruction * reconstruction, NeuronSegment * segment, BranchContainer * parent, CompositeBranchContainer * composite_match, double confidence){
    bc_reconstruction = reconstruction;
    bc_segment = segment;
    bc_parent = parent;
    if (parent){
        parent->add_child(this);
    }

    bc_composite_match = composite_match;
    bc_confidence = confidence;
    
};
void BranchContainer::set_segment(NeuronSegment * segment){
    bc_segment = segment;
};
void BranchContainer::set_reconstruction(Reconstruction * reconstruction){
    bc_reconstruction = reconstruction;
};
void BranchContainer::set_parent(BranchContainer * parent){
    if (bc_parent != parent){ // Don't bother doing anything if 'parent' is already the parent
    // Clear the existing parent relationship if it exists, and if
        if (bc_parent){
            BranchContainer *parent = bc_parent;
            bc_parent = nullptr; // Avoid endless loop by breaking connection from child to parent
            parent->remove_child(this);
        }
        bc_parent = parent;
        // If the parent isn't null, add this as its child
        if (parent){
            parent->add_child(this);
        }
    }
};
void BranchContainer::add_child(BranchContainer * child){
    if (child != nullptr){
        bc_children.insert(child);
        // Add segment child, if it isn't already there (BC uses set, but NeuronSegment uses vector)
        std::vector<NeuronSegment *>::iterator position = std::find(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment());
        if (position == bc_segment->child_list.end()){
            bc_segment->child_list.push_back(child->get_segment());
        }
        if (child->get_parent() != this){
            child->set_parent(this);
        }
    }
};
void BranchContainer::add_children(std::set<BranchContainer *> children){
    for (BranchContainer * child : children){
        add_child(child);
    }
}
/*
void BranchContainer::set_children(std::set<BranchContainer *> children){
    bc_children = children;
};*/
void BranchContainer::remove_children(){
    for (BranchContainer * child : bc_children){
        remove_child(child);
    }
}
void BranchContainer::set_composite_match(CompositeBranchContainer * composite_match){
    bc_composite_match = composite_match;
};
void BranchContainer::set_confidence(double confidence){
    bc_confidence = confidence;
};
void BranchContainer::remove_child(BranchContainer * child){
    bc_children.erase(bc_children.find(child));
    if (child->get_parent() == this){
        child->set_parent(nullptr);
    }
    // Remove segment child
    std::vector<NeuronSegment *> *vect = &(bc_segment->child_list);
    std::vector<NeuronSegment *>::iterator position = std::find(vect->begin(), vect->end(), child->get_segment());
    if (position != vect->end()) // == vector.end() means the element was not found
        vect->erase(position);
};
std::set<BranchContainer *> BranchContainer::get_children(){
    return std::set<BranchContainer*>(bc_children);
}
BranchContainer * BranchContainer::get_parent(){
    return bc_parent;
}
NeuronSegment * BranchContainer::get_segment(){
    return bc_segment;
}
CompositeBranchContainer * BranchContainer::get_composite_match(){
    return bc_composite_match;
};
double BranchContainer::get_confidence(){
    return bc_confidence;
};
Reconstruction * BranchContainer::get_reconstruction(){
    return bc_reconstruction;
}

std::vector<NeuronSegment *> produce_segment_vector(NeuronSegment * root){
    std::stack<NeuronSegment *> segment_stack;
    segment_stack.push(root);
    std::vector<NeuronSegment *> segments;
    NeuronSegment * segment;
    while (!segment_stack.empty()){
        segment = segment_stack.top();
        segment_stack.pop();
        segments.push_back(segment);
        for (NeuronSegment * child : segment->child_list){
            segment_stack.push(child);
        }
    }
    return segments;
}

std::map<NeuronSegment *, BranchContainer *> produce_segment_container_map(BranchContainer * container_root){
    std::map<NeuronSegment *, BranchContainer *> sb_map;
    std::stack<BranchContainer *> branch_stack;
    branch_stack.push(container_root);
    BranchContainer * branch;
    while (!branch_stack.empty()){
        branch = branch_stack.top();
        branch_stack.pop();
        sb_map[branch->get_segment()] = branch;
        for (BranchContainer * child : branch->get_children()){
            branch_stack.push(child);
        }
    }
    return sb_map;
}
