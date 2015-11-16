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
        BranchContainer * branch = get_branch_by_segment(seg);
        delete branch;
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
string Reconstruction::get_name(){
    return r_name;
};
void Reconstruction::set_tree(NeuronSegment * tree){
    r_tree = tree;
};
NeuronSegment * Reconstruction::get_tree(){
    return r_tree;
};
std::set<NeuronSegment *> Reconstruction::get_segments(){
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
    segment_container_map[branch->get_segment()] = branch;
    r_segments.insert(branch->get_segment());
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

// Any two branches that come into close proximity to each other will each be split at their nearest point
void Reconstruction::split_proximal_branches(float distance_threshold){
    // Generate branch stack
    std::stack<BranchContainer *> branch_stack;
    std::vector<BranchContainer *> branch_vector;
    branch_stack.push(r_root);
    printf("Test1\n");
    while (!branch_stack.empty()){
        BranchContainer * branch = branch_stack.top();
        branch_stack.pop();
        branch_vector.push_back(branch);
        printf("Test1.1\n");
        for (BranchContainer * child : branch->get_children()){
            printf("Test1.2\n");
            branch_stack.push(child);
        }
    }
    printf("Test2\n");
    
    for (int i = 0; i < branch_vector.size(); i++){
        BranchContainer * branch1 = branch_vector[i];
        printf("Test2.1\n");
        NeuronSegment * segment1 = branch1->get_segment();
        std::set<BranchContainer *> b1_children = branch1->get_children();
        printf("Test2.2\n");
        for (int j = i+1; j < branch_vector.size(); j++){
            BranchContainer * branch2 = branch_vector[j];
            printf("Test3\n");
            NeuronSegment * segment2 = branch2->get_segment();
            std::set<BranchContainer *> b2_children = branch2->get_children();

            // Don't search for crossover if each branch is parent of the other
            if(std::find(b1_children.begin(), b1_children.end(), branch2) == b1_children.end() &&
               std::find(b2_children.begin(), b2_children.end(), branch1) == b2_children.end()) {
                printf("Test3.1\n");

                MyMarker closest1, closest2;
                double segments_distance = segments_perpendicular_distance(
                    *(segment1->markers.front()), *(segment1->markers.back()),
                    *(segment2->markers.front()), *(segment2->markers.back()),
                    closest1, closest2);
                printf("Test3.2\n");

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
                            segment_container_map[new_segment] = new_branch;
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
                            segment_container_map[new_segment] = new_branch;
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
                printf("Test3.3\n");

            }
        }
        
    }
};

// Any branch that curves sufficiently (min distance from swc node to line segment between end points > threshold)
// will be split at the local maximum.
void Reconstruction::split_curved_branches(float curve_distance_threshold){
    std::stack<BranchContainer *> branch_stack;
    branch_stack.push(r_root);
    printf("SCB Test1\n");
    while (!branch_stack.empty()){
        BranchContainer * branch = branch_stack.top();
        NeuronSegment * branch_seg = branch->get_segment();
        branch_stack.pop();
        
        printf("SCB Test2 %p %p\n",branch,branch->get_segment());
        printf("SCB Test2.0 %i\n",branch->get_segment()->markers.size());
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
            printf("SCB Test2.1\n");
            int split_index = -1;
            for (int index = 0; index < markers.size() && split_index == -1; index++){
                MyMarker * marker = markers[index];
                // Calculate shortest distance from curve point to line segment between branch ends
                printf("SCB Test2.2\n");
                MyMarker closest_on_line;
                double curr_dist = point_segment_distance(*tip, *tail, *marker, closest_on_line);
                printf("SCB Test2.3\n");

                // If the previous distance was the local maximum and was greater than threshold
                if (last_dist > curve_distance_threshold && last_dist > curr_dist){
                    split_index = index;
                }
                
            }
            printf("SCB Test3\n");

            if (split_index != -1){
                // Split segment at the current marker
                NeuronSegment * new_segment = new NeuronSegment();
                BranchContainer * new_branch = new BranchContainer(branch->get_reconstruction(), new_segment, branch, nullptr, branch->get_confidence());
                std::set<BranchContainer *> children = branch->get_children();
                for (BranchContainer * child : children){
                    new_branch->add_child(child); // Removes child from its former parent
                }
                segment_container_map[new_segment] = new_branch;
                
                // Add the new branch back to the stack
                branch_stack.push(new_branch);

                // Move markers from old to new segment
                new_segment->markers = std::vector<MyMarker *>(branch_seg->markers.begin() + split_index, branch_seg->markers.end());
                branch_seg->markers.erase(branch_seg->markers.begin() + split_index, branch_seg->markers.end());
            }
        }
        printf("SCB Test4\n");

        if (split == -1){
            // Add children to be split in subsequent iterations
            for (BranchContainer * child : branch->get_children()){
                branch_stack.push(child);
            }
        }
    }
}

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
}

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
}

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
    reconstruction->add_branch(this);
    bc_segment = segment;
    bc_parent = parent;
    if (parent){
        parent->add_child(this);
    }

    bc_composite_match = composite_match;
    bc_confidence = confidence;
    if (confidence == -1){
        bc_confidence = reconstruction->get_confidence();
    }
};
BranchContainer::~BranchContainer(){
    // #TODO - delete segment and markers?
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


// Returns new branch above the split
BranchContainer * BranchContainer::split_branch(){
    // Get original segment
    NeuronSegment * orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment * new_seg_above = new NeuronSegment();

    // Since there is no split_point given, just copy the markers and leave the original segment alone
    new_seg_above->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.end());
    //orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point_t);
    
    // Create new branch above split
    BranchContainer * branch_above = new BranchContainer(bc_reconstruction, new_seg_above);
    
    return branch_above;
}
BranchContainer * BranchContainer::split_branch(std::size_t const split_point){
    // Get original segment
    NeuronSegment * orig_seg = bc_segment;
    
    // Create new segment above split
    NeuronSegment * new_seg_above = new NeuronSegment();
    
    new_seg_above->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point);
    
    // Create new branch above split
    BranchContainer * branch_above = new BranchContainer(bc_reconstruction, new_seg_above);
    
    return branch_above;

};


void BranchContainer::remove_child(BranchContainer * child){
    bc_children.erase(child);
    if (child->get_parent() == this){
        child->set_parent(nullptr);
    }
    // Remove segment child
    bc_segment->child_list.erase(std::remove(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment()), bc_segment->child_list.end());
};
std::set<BranchContainer *> BranchContainer::get_children(){
    printf("BranchContainer::get_children()\n");
    return bc_children;
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

std::set<NeuronSegment *> produce_segment_set(NeuronSegment * root){
    std::stack<NeuronSegment *> segment_stack;
    segment_stack.push(root);
    std::set<NeuronSegment *> segments;
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
