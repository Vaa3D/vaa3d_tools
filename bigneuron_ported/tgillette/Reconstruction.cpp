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

NeuronSegment * convert_to_neuron_segment(TreeSegment * root, bool destroy_tree){
    std::stack<TreeSegment *> seg_stack;
    seg_stack.push(root);
    NeuronSegment * neuron_root = nullptr;
    while (!seg_stack.empty()){
        TreeSegment * seg = seg_stack.top();
        NeuronSegment * neuron_seg = new NeuronSegment();
        if (neuron_root == nullptr){
            neuron_root = neuron_seg;
        }
        for (TreeSegment * child : seg->child_list){
            seg_stack.push(child);
        }
        if (destroy_tree){
            for (MyMarker * marker : seg->markers){
                delete marker;
            }
            delete seg;
        }
    }
    return neuron_root;
};
TreeSegment * convert_to_tree_segment(NeuronSegment * root, bool destroy_neuron){
    std::stack<NeuronSegment *> seg_stack;
    seg_stack.push(root);
    TreeSegment * tree_root;
    std::map<NeuronSegment *,TreeSegment *> parent_connector;
    while (!seg_stack.empty()){
        NeuronSegment * seg = seg_stack.top();
        TreeSegment * tree_seg = new TreeSegment();
        if (parent_connector[seg]){
            tree_seg->parent = parent_connector[seg];
        }else{
            tree_root = tree_seg;
        }
        tree_seg->markers = seg->markers;
        for (NeuronSegment * child : seg->child_list){
            parent_connector[child] = tree_seg;
            seg_stack.push(child);
        }
        if (destroy_neuron) delete seg;
    }
    return tree_root;
};
void delete_tree(TreeSegment * root, bool delete_markers){
    std::stack<TreeSegment *> seg_stack;
    seg_stack.push(root);
    while (!seg_stack.empty()){
        TreeSegment * seg = seg_stack.top();
        for (TreeSegment * child : seg->child_list){
            seg_stack.push(child);
        }
        if (delete_markers){
            for (MyMarker * marker : seg->markers){
                delete marker;
            }
        }
        delete seg;
    }
};
void delete_neuron(NeuronSegment * root, bool delete_markers){
    std::stack<NeuronSegment *> seg_stack;
    seg_stack.push(root);
    while (!seg_stack.empty()){
        NeuronSegment * seg = seg_stack.top();
        for (NeuronSegment * child : seg->child_list){
            seg_stack.push(child);
        }
        if (delete_markers){
            for (MyMarker * marker : seg->markers){
                delete marker;
            }
        }
        delete seg;
    }
};


/* Class Reconstruction */
Reconstruction::Reconstruction(){
    r_confidence = 1;
};
Reconstruction::Reconstruction(NeuronSegment * tree){
    r_tree = tree;
    r_confidence = 1;
    BranchContainer * container = new BranchContainer(this, tree);
    r_root = container;
    
    create_branch_container_trees(container);
    segment_container_map = produce_segment_container_map(container);
    r_segments = produce_segment_set(r_tree);
};
Reconstruction::Reconstruction(string name, NeuronSegment * tree, double confidence){
    r_name = name;
    r_tree = tree;
    r_confidence = confidence;

    BranchContainer * container = new BranchContainer(this, tree);
    r_root = container;

    create_branch_container_trees(container);

    segment_container_map = produce_segment_container_map(container);

    r_segments = produce_segment_set(r_tree);
};
Reconstruction::~Reconstruction(){
    for (NeuronSegment * seg : r_segments){
//        printf("~Reconstruction 1\n");
        BranchContainer * branch = get_branch_by_segment(seg);
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
    create_branch_container_trees(r_root);
    segment_container_map = produce_segment_container_map(r_root);
    r_segments = produce_segment_set(r_tree);
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
string Reconstruction::get_name() const{
    return r_name;
};
void Reconstruction::set_tree(NeuronSegment * tree){
    r_tree = tree;
};
NeuronSegment * Reconstruction::get_tree() const{
    return r_tree;
};
SegmentPtrSet Reconstruction::get_segments() const{
    return r_segments;
};
std::vector<NeuronSegment *> Reconstruction::get_segments_ordered(){
    return produce_segment_vector(r_tree);
};
typedef map<NeuronSegment *,BranchContainer *> SegmentContainerMap;
typedef pair<NeuronSegment *,BranchContainer *> SegmentContainerPair;
void Reconstruction::update_branch_by_segment(NeuronSegment * segment, BranchContainer * branch){
    segment_container_map[segment] = branch;
}
void Reconstruction::add_branch(BranchContainer * branch){
    printf("add_branch before: r_segments size %i, segment_container_map size %i\n",r_segments.size(),segment_container_map.size());
    segment_container_map[branch->get_segment()] = branch;
    r_segments.insert(branch->get_segment());
    printf("add_branch after: r_segments size %i, segment_container_map size %i\n",r_segments.size(),segment_container_map.size());
}

BranchContainer * Reconstruction::get_branch_by_segment(NeuronSegment * segment) {
    return segment_container_map[segment];
};
void Reconstruction::set_confidence(double confidence){
    r_confidence = confidence;
};
double Reconstruction::get_confidence() const{
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
    bc_composite_match = nullptr;
    bc_confidence = 1;
};
BranchContainer::BranchContainer(Reconstruction * reconstruction){
    bc_reconstruction = reconstruction;
    reconstruction->add_branch(this);
    bc_segment = nullptr;
    bc_parent = nullptr;
    bc_composite_match = nullptr;
    bc_confidence = reconstruction->get_confidence();
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
    reconstruction->add_branch(this);

    bc_composite_match = composite_match;
    bc_confidence = confidence;
    if (confidence == -1){
        bc_confidence = reconstruction->get_confidence();
    }
};
BranchContainer::~BranchContainer(){
    // #TODO - delete segment and markers?
//    printf("~BranchContainer\n");
}

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
            BranchContainer *prev_parent = bc_parent;
            bc_parent = nullptr; // Avoid endless loop by breaking connection from child to parent
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
void BranchContainer::remove_child(BranchContainer * child){
    bc_children.erase(child);
    if (child->get_parent() == this){
        child->set_parent(nullptr);
    }
    // Remove segment child
    bc_segment->child_list.erase(std::remove(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment()), bc_segment->child_list.end());
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


// Returns new branch above the split
BranchContainer * BranchContainer::split_branch(){
    // Get original segment
    NeuronSegment * orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment * new_seg_before = copy_segment_markers(orig_seg);

    // Since there is no split_point given, just copy the markers and leave the original segment alone
    //new_seg_before->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.end());
    //orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point_t);
    
    // Create new branch above split
    BranchContainer * branch_before = new BranchContainer(bc_reconstruction, new_seg_before, nullptr, nullptr, bc_confidence);
    
    BranchContainer * parent = bc_parent;
    set_parent(branch_before);
    branch_before->set_parent(parent);
    
    return branch_before;
}
BranchContainer * BranchContainer::split_branch(std::size_t const split_point){
    // Get original segment
    NeuronSegment * orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment * new_seg_before = new NeuronSegment();
    
    if (split_point <= 0 || split_point > orig_seg->markers.size()-1){
        printf("ERROR! split_point is %i, segment length is %i\n",split_point,orig_seg->markers.size());
    }

    new_seg_before->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    
    // Create new branch above split
    BranchContainer * branch_before = new BranchContainer(bc_reconstruction, new_seg_before, nullptr, nullptr, bc_confidence);
    
    BranchContainer * parent = bc_parent;
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

// Creates vector in post order
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
    std::reverse(segments.begin(), segments.end());
    return segments;
}

SegmentPtrSet produce_segment_set(NeuronSegment * root){
    std::stack<NeuronSegment *> segment_stack;
    segment_stack.push(root);
    SegmentPtrSet segments;
    NeuronSegment * segment;
    while (!segment_stack.empty()){
        segment = segment_stack.top();
        segment_stack.pop();
        segments.insert(segment);
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


NeuronSegment * copy_segment_markers(NeuronSegment* segment){
    NeuronSegment * new_segment = new NeuronSegment();
    new_segment->markers = std::vector<MyMarker*>(segment->markers.size());
    MyMarker * new_marker;
    std::map<MyMarker *, MyMarker *> marker_map;
    int ind = 0;
    for (MyMarker* marker : segment->markers){
        new_marker = new MyMarker(*marker);
        if (marker_map.find(marker->parent) != marker_map.end()){
            new_marker->parent = marker_map[marker->parent];
        }
        marker_map[marker] = new_marker;
        new_segment->markers[ind++] = new_marker;
    }
    return new_segment;
};

NeuronSegment * copy_segment_tree(NeuronSegment * root){
    NeuronSegment * segment, * copy, * child_copy, * root_copy;
    typedef pair<NeuronSegment *, NeuronSegment *> SegmentPair;
    std::stack<SegmentPair> seg_stack;
    root_copy = copy_segment_markers(root);
    seg_stack.push(SegmentPair(root, root_copy));
    while (!seg_stack.empty()){
        pair<NeuronSegment *, NeuronSegment *> pair = seg_stack.top();
        segment = pair.first;
        copy = pair.second;
        seg_stack.pop();
        for (NeuronSegment * child : segment->child_list){
            child_copy = copy_segment_markers(child);
            copy->child_list.push_back(child_copy);
            if (child->child_list.size() > 0){
                seg_stack.push(SegmentPair(child, child_copy));
            }
        }
    }
    return root_copy;
};


