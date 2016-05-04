//
//  Reconstruction.cpp
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/29/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#include <stack>
#include <algorithm>
#include <tgmath.h>
#include "Reconstruction.h"

NeuronSegment * convert_to_neuron_segment(TreeSegment *root, bool destroy_tree){
    std::stack<TreeSegment *> seg_stack;
    seg_stack.push(root);
    NeuronSegment *neuron_root = NULL;
    while (!seg_stack.empty()){
        TreeSegment *seg = seg_stack.top();
        NeuronSegment *neuron_seg = new NeuronSegment();
        if (neuron_root == NULL){
            neuron_root = neuron_seg;
        }
        for (int i =0; i<seg->child_list.size(); i++){
            TreeSegment * child = seg->child_list[i];
            seg_stack.push(child);
        }
        if (destroy_tree){
           for (int i =0; i< seg->markers.size(); i++){ 
                MyMarker *marker = seg->markers[i];
                delete marker;
            }
            delete seg;
        }
    }
    return neuron_root;
};
TreeSegment * convert_to_tree_segment(NeuronSegment *root, bool destroy_neuron){
    std::stack<NeuronSegment *> seg_stack;
    seg_stack.push(root);
    TreeSegment *tree_root;
    std::map<NeuronSegment *,TreeSegment *> parent_connector;
    while (!seg_stack.empty()){
        NeuronSegment *seg = seg_stack.top();
        TreeSegment *tree_seg = new TreeSegment();
        if (parent_connector[seg]){
            tree_seg->parent = parent_connector[seg];
        }else{
            tree_root = tree_seg;
        }
        tree_seg->markers = seg->markers;
        for (int i =0; i<seg->child_list.size(); i++){
            NeuronSegment *child = seg->child_list[i];
            parent_connector[child] = tree_seg;
            seg_stack.push(child);
        }
        if (destroy_neuron) delete seg;
    }
    return tree_root;
};
void delete_tree(TreeSegment *root, bool delete_markers){
    std::stack<TreeSegment *> seg_stack;
    seg_stack.push(root);
    while (!seg_stack.empty()){
        TreeSegment *seg = seg_stack.top();
        for (int i =0; i<seg->child_list.size(); i++){
            TreeSegment * child = seg->child_list[i];
            seg_stack.push(child);
        }
        if (delete_markers){
            for (int i =0;i<seg->markers.size(); i++){
                MyMarker *marker =seg->markers[i];
                delete marker;
            }
        }

        delete seg;
    }
};
void delete_neuron(NeuronSegment *root, bool delete_markers){
    std::stack<NeuronSegment *> seg_stack;
    seg_stack.push(root);
    while (!seg_stack.empty()){
        NeuronSegment *seg = seg_stack.top();
        for (int i =0; i<seg->child_list.size(); i++){
            NeuronSegment *child = seg->child_list[i];
            seg_stack.push(child);
        }
        if (delete_markers){
            for (int i =0;i<seg->markers.size(); i++){
                MyMarker *marker =seg->markers[i];
                delete marker;
            }
        }
        delete seg;
    }
};

NeuronSegment * copy_segment_markers(NeuronSegment *segment){
    NeuronSegment *new_segment = new NeuronSegment();
    new_segment->markers = std::vector<MyMarker*>(segment->markers.size());
    MyMarker *new_marker;
    std::map<MyMarker *, MyMarker *> marker_map;
    int ind = 0;
    for (int i =0; i< segment->markers.size(); i++){
        MyMarker* marker =segment->markers[i];
        new_marker = new MyMarker(*marker);
        if (marker_map.find(marker->parent) != marker_map.end()){
            new_marker->parent = marker_map[marker->parent];
        }
        marker_map[marker] = new_marker;
        new_segment->markers[ind++] = new_marker;
    }
    return new_segment;
};

NeuronSegment * copy_segment_tree(NeuronSegment *root){
    NeuronSegment *segment, *copy, *child_copy, *root_copy;
    typedef pair<NeuronSegment *, NeuronSegment *> SegmentPair;
    std::stack<SegmentPair> seg_stack;
    root_copy = copy_segment_markers(root);
    seg_stack.push(SegmentPair(root, root_copy));
    while (!seg_stack.empty()){
        pair<NeuronSegment *, NeuronSegment *> pair = seg_stack.top();
        segment = pair.first;
        copy = pair.second;
        seg_stack.pop();
        for (int i =0; i<segment->child_list.size(); i++){
            NeuronSegment *child = segment->child_list[i];
            child_copy = copy_segment_markers(child);
            copy->child_list.push_back(child_copy);
            if (child->child_list.size() > 0){
                seg_stack.push(SegmentPair(child, child_copy));
            }
        }
    }
    return root_copy;
};


/**
 * Class BranchContainer
 * Contains the confidence value of the branch, a pointer to the branch data, and pointers to either end's deicision point
 */
unsigned long BranchContainer::master_uid = 0;

BranchContainer::BranchContainer(){
    // Uncertain whether it is necessary to set these, but I want to be certain #CONSIDER: removing the NULL assignments if they are indeed unncessary
    bc_reconstruction = NULL;
    bc_segment = NULL;
    bc_parent = NULL;
    bc_composite_match = NULL;
    bc_confidence = 1;
    uid = BranchContainer::master_uid++;
};
BranchContainer::BranchContainer(Reconstruction *reconstruction){
    bc_reconstruction = reconstruction;
    reconstruction->add_branch(this);
    bc_segment = NULL;
    bc_parent = NULL;
    bc_composite_match = NULL;
    bc_confidence = reconstruction->get_confidence();
    uid = BranchContainer::master_uid++;
};
BranchContainer::BranchContainer(Reconstruction *reconstruction, NeuronSegment *segment, BranchContainer *parent, CompositeBranchContainer *composite_match, double confidence){
    bc_reconstruction = reconstruction;
    bc_segment = segment;
    bc_parent = parent;
    if (parent){
        parent->add_child(this);
    }
    reconstruction->add_branch(this);

    bc_composite_match = composite_match;
    bc_confidence = confidence;
    if (confidence == -1){
        bc_confidence = reconstruction->get_confidence();
    }
    subtree_markers = segment->markers.size();
    uid = BranchContainer::master_uid++;
};
BranchContainer::~BranchContainer(){
    // #TODO - delete segment and markers?
//    printf("~BranchContainer\n");
}

void BranchContainer::set_segment(NeuronSegment *segment){
    if (bc_segment)
        subtree_markers -= bc_segment->markers.size();
    if (segment)
        subtree_markers += segment->markers.size();
    
    bc_segment = segment;
};
void BranchContainer::set_reconstruction(Reconstruction *reconstruction){
    bc_reconstruction = reconstruction;
};
void BranchContainer::set_parent(BranchContainer *parent){
    if (bc_parent != parent){ // Don't bother doing anything if 'parent' is already the parent
    // Clear the existing parent relationship if it exists, and if
        if (bc_parent){
            BranchContainer *prev_parent = bc_parent;
            bc_parent = NULL; // Avoid endless loop by breaking connection from child to parent
            prev_parent->remove_child(this);
        }
        bc_parent = parent;
//        segment->parent = parent->get_segment();
        // If the parent isn't null, add this as its child
        if (parent){
            std::set<BranchContainer *> parent_children = parent->get_children();
            if (parent_children.find(this) == parent_children.end())
                parent->add_child(this);
        }
    }
};
void BranchContainer::add_child(BranchContainer *child){
    if (child != NULL){
        bc_children.insert(child);
        // Add segment child, if it isn't already there (BC uses set, but NeuronSegment uses vector)
        std::vector<NeuronSegment *>::iterator position = std::find(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment());
        if (position == bc_segment->child_list.end()){
            bc_segment->child_list.push_back(child->get_segment());
        }
        if (child->get_parent() != this){
            child->set_parent(this);
        }
        subtree_markers += child->get_subtree_markers();
    }
};
void BranchContainer::remove_child(BranchContainer *child){
    int num_children_before = bc_children.size();
    bc_children.erase(child);
    if (bc_children.size() < num_children_before) subtree_markers -= child->get_subtree_markers();
    
    if (child->get_parent() == this){
        child->set_parent(NULL);
    }
    
    // Remove segment child
    bc_segment->child_list.erase(std::remove(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment()), bc_segment->child_list.end());
};

void BranchContainer::add_children(std::set<BranchContainer *> children){
	std::set<BranchContainer *>::iterator it;
	for (it = children.begin(); it != children.end(); ++it)
	{
		BranchContainer *child =  *it;
                add_child(child);
        }
}
/*
void BranchContainer::set_children(std::set<BranchContainer *> children){
    bc_children = children;
};*/
void BranchContainer::remove_children(){
	std::set<BranchContainer *>::iterator it;
	for (it = bc_children.begin(); it != bc_children.end(); ++it)
	{
		BranchContainer *child =  *it;
		remove_child(child);
	}
}
void BranchContainer::set_composite_match(CompositeBranchContainer *composite_match){
	bc_composite_match = composite_match;
};
void BranchContainer::set_confidence(double confidence){
    bc_confidence = confidence;
};


// Returns new branch above the split
BranchContainer * BranchContainer::split_branch(){
    // Get original segment
    NeuronSegment *orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment *new_seg_before = copy_segment_markers(orig_seg);

    // Since there is no split_point given, just copy the markers and leave the original segment alone
    //new_seg_before->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.end());
    //orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point_t);
    
    // Create new branch above split
    BranchContainer *branch_before = new BranchContainer(bc_reconstruction, new_seg_before, NULL, NULL, bc_confidence);
    
    BranchContainer *parent = bc_parent;
    set_parent(branch_before);
    branch_before->set_parent(parent);
    
    return branch_before;
}
BranchContainer * BranchContainer::split_branch(std::size_t const split_point){
    // Get original segment
    NeuronSegment *orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment *new_seg_before = new NeuronSegment();
    
    if (split_point <= 0 || split_point > orig_seg->markers.size()-1){
        printf("ERROR! split_point is %i, segment length is %i\n",split_point,orig_seg->markers.size());
    }

    new_seg_before->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    subtree_markers -= split_point;
    
    // Create new branch above split
    BranchContainer *branch_before = new BranchContainer(bc_reconstruction, new_seg_before, NULL, NULL, bc_confidence);
    
    BranchContainer *parent = bc_parent;
    set_parent(branch_before);
    branch_before->set_parent(parent);

    return branch_before;
};


std::set<BranchContainer *> BranchContainer::get_children() const{
    return bc_children;
}
BranchContainer * BranchContainer::get_parent() const{
    return bc_parent;
}
NeuronSegment * BranchContainer::get_segment() const{
    return bc_segment;
}
CompositeBranchContainer * BranchContainer::get_composite_match() const{
    return bc_composite_match;
};
double BranchContainer::get_confidence() const{
    return bc_confidence;
};
Reconstruction * BranchContainer::get_reconstruction() const{
    return bc_reconstruction;
}

/* Class Reconstruction */
Reconstruction::Reconstruction(){
    r_confidence = 1;
};
Reconstruction::Reconstruction(NeuronSegment *tree){
    r_tree = tree;
    r_trees.push_back(tree);
    r_confidence = 1;
    
    create_branch_container_trees(r_trees);
    r_segments = produce_segment_set(r_trees);
};
Reconstruction::Reconstruction(std::vector<NeuronSegment *> trees){
    r_tree = trees[0];
    r_trees = trees;
    r_confidence = 1;
    
    create_branch_container_trees(r_trees);
    r_segments = produce_segment_set(r_trees);
};
Reconstruction::Reconstruction(string name, NeuronSegment *tree, double confidence){
    r_name = name;
    r_tree = tree;
    r_trees.push_back(tree);
    r_confidence = confidence;
    
    create_branch_container_trees(r_trees);
    r_segments = produce_segment_set(r_trees);
};
Reconstruction::Reconstruction(string name, std::vector<NeuronSegment *> trees, double confidence){
    r_name = name;
    r_tree = trees[0];
    r_trees = trees;
    r_confidence = confidence;
    
    create_branch_container_trees(r_trees);
    r_segments = produce_segment_set(r_trees);
};

Reconstruction::~Reconstruction(){
     
    std::set<NeuronSegment *> ::iterator it;
    for (it = r_segments.begin() ; it != r_segments.end(); it++){
        NeuronSegment *seg  = *it;
        //        printf("~Reconstruction 1\n");
        BranchContainer *branch = get_branch_by_segment(seg);
        //        printf("~Reconstruction 2\n");
        if (branch){
            //            printf("~Reconstruction 3 branch %p\n",branch);
            delete branch;
            //            printf("~Reconstruction 4\n");
        }
        //delete seg;
    }
}

void Reconstruction::update_constituents(){
    create_branch_container_trees(r_trees);
    r_segments = produce_segment_set(r_trees);
};
/*
 void Reconstruction::create_branch_container_trees(BranchContainer *root_branch){
 stack<pair<NeuronSegment *,BranchContainer *> > stack;
 typedef pair<NeuronSegment *,BranchContainer *> SCPair;
 SCPair sc_pair;
 NeuronSegment *segment;
 BranchContainer *branch, *child_branch;
 stack.push(SCPair(root_branch->get_segment(), root_branch));
 while (!stack.empty()){
 sc_pair = stack.top();
 stack.pop();
 segment = sc_pair.first;
 branch = sc_pair.second;
 vector<NeuronSegment*> children = segment->child_list;
 for (NeuronSegment *child : children){
 // Creates a BC for the child segment as well as pointer to its BC parent, and for its parent back to it
 child_branch = new BranchContainer(this,child,branch,NULL,r_confidence);
 if (child->child_list.size() > 0){
 stack.push(SCPair(child,child_branch));
 }
 }
 }
 }
 */
void Reconstruction::create_branch_container_trees(std::vector<NeuronSegment *> trees){
    stack<pair<NeuronSegment *,BranchContainer *> > stack;
    typedef pair<NeuronSegment *,BranchContainer *> SCPair;
    SCPair sc_pair;
    NeuronSegment *segment;
    BranchContainer *branch, *child_branch;
    for (int i =0; i< trees.size();i++){ 
        NeuronSegment *segment = trees[i];
        child_branch = new BranchContainer(this,segment,NULL,NULL,r_confidence);
        segment_container_map[segment] = child_branch;
        if (segment->child_list.size() > 0)
            stack.push(SCPair(segment,child_branch));
    }
    
    //stack.push(SCPair(root_branch->get_segment(), root_branch));
    while (!stack.empty()){
        sc_pair = stack.top();
        stack.pop();
        segment = sc_pair.first;
        branch = sc_pair.second;
        vector<NeuronSegment*> children = segment->child_list;
        for (int i =0; i< children.size(); i++){
            NeuronSegment *child = children[i];
            // Creates a BC for the child segment as well as pointer to its BC parent, and for its parent back to it
            child_branch = new BranchContainer(this,child,branch,NULL,r_confidence);
            segment_container_map[child] = child_branch;
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
string Reconstruction::get_name() const{
    return r_name;
};
/*
 void Reconstruction::set_tree(NeuronSegment *tree){
 r_tree = tree;
 };
 */
NeuronSegment * Reconstruction::get_tree() const{
    return r_tree;
};
std::vector<NeuronSegment *> Reconstruction::get_trees() const{
    return r_trees;
};
SegmentPtrSet Reconstruction::get_segments() const{
    return r_segments;
};
std::vector<NeuronSegment *> Reconstruction::get_segments_ordered(){
    return produce_segment_vector(r_tree);
};
typedef map<NeuronSegment *,BranchContainer *> SegmentContainerMap;
typedef pair<NeuronSegment *,BranchContainer *> SegmentContainerPair;
void Reconstruction::update_branch_by_segment(NeuronSegment *segment, BranchContainer *branch){
    segment_container_map[segment] = branch;
}
void Reconstruction::add_branch(BranchContainer * branch){
    //    printf("add_branch before: r_segments size %i, segment_container_map size %i\n",r_segments.size(),segment_container_map.size());
    segment_container_map[branch->get_segment()] = branch;
    r_segments.insert(branch->get_segment());
    //    printf("add_branch after: r_segments size %i, segment_container_map size %i\n",r_segments.size(),segment_container_map.size());
}

BranchContainer * Reconstruction::get_branch_by_segment(NeuronSegment *segment) {
    return segment_container_map[segment];
};
void Reconstruction::set_confidence(double confidence){
    r_confidence = confidence;
};
double Reconstruction::get_confidence() const{
    return r_confidence;
};

long Reconstruction::get_node_count(){
    long node_count = 0;
    std::set<NeuronSegment *> ::iterator it;
    for (it = r_segments.begin() ; it != r_segments.end(); it++){
        NeuronSegment *seg  = *it;
        node_count += seg->markers.size();
    }
    return node_count;
};

/* Extraneous methods */

// Creates vector in post order
std::vector<NeuronSegment *> produce_segment_vector(NeuronSegment *root){
    std::stack<NeuronSegment *> segment_stack;
    segment_stack.push(root);
    std::vector<NeuronSegment *> segments;
    NeuronSegment * segment;
    while (!segment_stack.empty()){
        segment = segment_stack.top();
        segment_stack.pop();
        segments.push_back(segment);
        for (int i =0; i< segment->child_list.size(); i++){
            NeuronSegment *child = segment->child_list[i];
            segment_stack.push(child);
        }
    }
    std::reverse(segments.begin(), segments.end());
    return segments;
}

SegmentPtrSet produce_segment_set(std::vector<NeuronSegment *> roots){
    std::stack<NeuronSegment *> segment_stack;
    for (int i =0; i<roots.size(); i++){
        NeuronSegment *segment = roots[i];
        segment_stack.push(segment);
    }
    SegmentPtrSet segments;
    NeuronSegment *segment;
    while (!segment_stack.empty()){
        segment = segment_stack.top();
        segment_stack.pop();
        segments.insert(segment);

        for (int i =0; i< segment->child_list.size(); i++){
            NeuronSegment *child = segment->child_list[i];
            segment_stack.push(child);
        }
    }
    return segments;
}

std::map<NeuronSegment *, BranchContainer *> produce_segment_container_map(BranchContainer *container_root){
    std::map<NeuronSegment *, BranchContainer *> sb_map;
    std::stack<BranchContainer *> branch_stack;
    branch_stack.push(container_root);
    BranchContainer *branch;
    while (!branch_stack.empty()){
        branch = branch_stack.top();
        branch_stack.pop();
        sb_map[branch->get_segment()] = branch;

        std::set<BranchContainer * >::iterator it;
	for (it = branch->get_children().begin(); it!= branch->get_children().end(); it++){
		BranchContainer *child = *it;
		branch_stack.push(child);
        }
    }
    return sb_map;
};

