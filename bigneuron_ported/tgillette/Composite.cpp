//
//  Composite.cpp
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/29/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#include <stdio.h>
#include <stack>
#include <set>
#include <math.h>
#include <tgmath.h>
#include <algorithm>
#include "Composite.h"

/**
 * Below are classes associated directly with the Composite
 */

/******************************
 ** CompositeBranchContainer **
 ******************************/
CompositeBranchContainer::CompositeBranchContainer(){
    cbc_composite = nullptr;
    cbc_parent = nullptr;
    combined_connection_weight = 0;
    best_connection = nullptr;
    segment_reversed = false;
    bc_confidence = 0;
    confidence_denominator = 0;
};
CompositeBranchContainer::CompositeBranchContainer(NeuronSegment * segment, Composite * composite){
    cbc_composite = composite;
    bc_segment = segment;
    if (composite) composite->add_branch(this);
    cbc_parent = nullptr;
    combined_connection_weight = 0;
    best_connection = nullptr;
    segment_reversed = false;
    bc_confidence = 0;
    confidence_denominator = 0;
};
/**
 * Creates a CompositeBranchContainer that is a copy of the input branch
 *   - Deep copy of NeuronSegment
 **/
CompositeBranchContainer::CompositeBranchContainer(CompositeBranchContainer * branch, bool copy_segment){
    copy_from(branch, branch->get_composite(), copy_segment);
};
CompositeBranchContainer::CompositeBranchContainer(CompositeBranchContainer * branch, Composite * composite, bool copy_segment){
    copy_from(branch, composite, copy_segment);
};
CompositeBranchContainer::CompositeBranchContainer(CompositeBranchContainer * branch){
    copy_from(branch, branch->get_composite(), true);
};
CompositeBranchContainer::CompositeBranchContainer(CompositeBranchContainer * branch, Composite * composite){
    copy_from(branch, composite, true);
};
void CompositeBranchContainer::copy_from(CompositeBranchContainer * branch, Composite * composite, bool copy_segment){
//    printf("CBC:copy_from Enter, copy from branch %p\n",branch);
    if (copy_segment){
        NeuronSegment * segment_copy = copy_segment_markers(branch->get_segment());
        set_segment(segment_copy);
    }else{
        set_segment(branch->get_segment());
        //        parent_connections = branch->get_connections();
        //        best_connection = branch->get_best_connection_weight().first;
    }
//    printf("CBC:copy_from 0l best_connection %p\n",get_best_connection_weight().first);
    best_connection = nullptr;
    parent_connections = branch->copy_connections();
    calculate_best_connection();

//    printf("CBC:copy_from 1\n");
    cbc_composite = composite;
    if (composite) composite->add_branch(this);
    
//    printf("CBC:copy_from 2\n");
    segment_reversed = branch->is_segment_reversed();
    branch_matches = branch->get_branch_matches();
//    printf("CBC:copy_from 3\n");
    bc_confidence = branch->get_summed_confidence();
//    printf("CBC:copy_from 4\n");
    confidence_denominator = branch->get_confidence_denominator();
    cbc_parent = nullptr;
    
    combined_connection_weight = branch->get_combined_connection_weight();
};
std::set<Connection *> CompositeBranchContainer::copy_connections() const{
    std::set<Connection *> new_connections;
//    printf("CBC:copy_connections 1\n");

    for (Connection * orig_con : parent_connections){
//        printf("CBC:copy_from connection %p, has %i recons\n",orig_con, orig_con->get_reconstructions().size());

        new_connections.insert(new Connection(orig_con));
    }
    return new_connections;
};




CompositeBranchContainer::~CompositeBranchContainer(){
    if (cbc_composite){
        cbc_composite->remove_branch(this);
    }
    /*
    set_parent(nullptr);
    remove_children();
    
    printf("Removing connections\n");

    for (Connection * connection : parent_connections){
        printf("Removing connections, parent %p\n",connection->get_parent());
        if (connection->get_parent()){
            connection->get_parent()->remove_child_connection(connection);
            printf("Removed parent's child connection\n");
        }
        delete connection;
    }
     */
};

void CompositeBranchContainer::prepare_for_deletion(){
    set_parent(nullptr);
    remove_children();
    
    for (Connection * connection : parent_connections){
        if (connection->get_parent()){
            connection->get_parent()->remove_child_connection(connection);
        }
    }
}
void CompositeBranchContainer::delete_connections(){
    for (Connection * connection : parent_connections){
        delete connection;
    }
}


void CompositeBranchContainer::add_branch_match(BranchContainer * match){
    add_branch_match(match, true);
};
void CompositeBranchContainer::add_branch_match(BranchContainer * match, bool match_forward){
    branch_matches.push_back(match);
    match_forward_map[match] = match_forward;
    printf("add_branch_match, current summed conf %f, to be added %f\n",bc_confidence,match->get_confidence());
//    printf("add_branch_match conf before %f\n",bc_confidence);
    bc_confidence += match->get_confidence();
//    printf("add_branch_match conf after %f\n",bc_confidence);
    confidence_denominator += match->get_confidence();
//    printf("add_branch_match confidence_denominator %f\n",confidence_denominator);
};
bool CompositeBranchContainer::is_match_forward(BranchContainer * match){
    return match_forward_map[match];
};
void CompositeBranchContainer::reset_confidence(){
    bc_confidence = 0;
    confidence_denominator = 0;
}
void CompositeBranchContainer::add_branch_miss(double confidence){
    confidence_denominator += confidence;
};
void CompositeBranchContainer::set_segment_reversed(bool segment_reversed){
    this->segment_reversed = segment_reversed;
};
void CompositeBranchContainer::reverse_segment(){
    segment_reversed = !segment_reversed;
    std::reverse(bc_segment->markers.begin(),bc_segment->markers.end());
};
bool CompositeBranchContainer::is_segment_reversed(){
    return segment_reversed;
};

void CompositeBranchContainer::set_parent(CompositeBranchContainer * parent, bool follow, BranchEnd branch_end, BranchEnd parent_end){
    if (cbc_parent && follow){
        cbc_parent->remove_child(this, false);
    }
    cbc_parent = parent;
    if (parent)
        cbc_parent->add_child(this);
};
void CompositeBranchContainer::add_child(CompositeBranchContainer * child){
    if (child){
        cbc_children.insert(child);
        std::vector<NeuronSegment *>::iterator position = std::find(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment());
        if (position == bc_segment->child_list.end()){
//            printf("pushing segment starting at %f %f into list of children of segment starting at %f %f\n",child->get_segment()->markers[0]->x,child->get_segment()->markers[0]->y,get_segment()->markers[0]->x,get_segment()->markers[0]->y);
            bc_segment->child_list.push_back(child->get_segment());
        }
        if (child->get_parent() != this){
            child->set_parent(this);
        }
    }
};
void CompositeBranchContainer::remove_child(CompositeBranchContainer * child, bool follow){
    if (child && child->get_parent() == this){
        if (follow) child->set_parent(nullptr, false);
        bc_children.erase(child);
        NeuronSegment * child_seg = child->get_segment();
        if (child_seg)
            bc_segment->child_list.erase(std::remove(bc_segment->child_list.begin(), bc_segment->child_list.end(), child_seg),bc_segment->child_list.end());
        /*
        std::vector<NeuronSegment *>::iterator position = std::find(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment());
        printf("found child's segment in segment child_list? %i \n",position != bc_segment->child_list.end());
        if (position != bc_segment->child_list.end()) // == vector.end() means the element was not found
            bc_segment->child_list.erase(position);
         */
    }
};
void CompositeBranchContainer::remove_children(){
    std::set<CompositeBranchContainer *> tmp_children = cbc_children;
    for (CompositeBranchContainer * child : tmp_children){
        remove_child(child);
    }
};

// Returns new branch above the split; Transfers all connections on the TOP end to the new branch
CompositeBranchContainer * CompositeBranchContainer::split_branch(std::size_t const split_point){
    // Get original segment
    NeuronSegment * orig_seg = bc_segment;
    
    if (split_point <= 0 || split_point > orig_seg->markers.size()-1){
        printf("ERROR! split_point is %i, segment length is %i\n",split_point,orig_seg->markers.size());
    }
    
    // Create new segment above split
    NeuronSegment * new_seg_before = new NeuronSegment();
    std::size_t const split_point_t = split_point;
    new_seg_before->markers = std::vector<MyMarker *>(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point_t);
    //new_seg_below(orig_seg->markers.begin() + split_point_t, orig_seg->markers.end());
    orig_seg->markers.erase(orig_seg->markers.begin(), orig_seg->markers.begin() + split_point_t);

    // Create new branch above split
    CompositeBranchContainer * branch_before = new CompositeBranchContainer(new_seg_before, cbc_composite);

//    printf("CBC::split_branch 0\n");

    // Transfer TOP or BOTTOM connections to new branch_before (depending on whether branch is reversed at the moment)
    // Update connections, and add contributing reconstructions to new connection
    std::set<Connection *> tmp_connections = parent_connections;
    for (Connection * connection : tmp_connections){
        // Connections at the TOP of the segment need to be transfered to the new segment above the split
        // OR if the segment is reversed then segments at the BOTTOM need to be transfered
        if (segment_reversed ^ connection->get_child_end() == TOP){ //
//            printf("CBC::split_branch 0.2 %i %i\n",segment_reversed, connection->get_child_end() == TOP);
            this->remove_connection(connection);
//            printf("CBC::split_branch 0.3\n");
            connection->set_child(branch_before); // This will also take care of parent's child connection (top_child_connections & bottom_child_connections)
//            printf("CBC::split_branch 0.4\n");
            branch_before->import_connection(connection);
        }
    }
    
    // Transfer child connections that are at the TOP
    for (Connection * connection : top_child_connections){
        connection->set_parent(branch_before);
        branch_before->add_child_connection(connection);
    }
    top_child_connections.clear();

    // Create connection between branches
    BranchEnd child_end, parent_end;
    Connection * new_connection;
    for (BranchContainer * r_branch : branch_matches){
        if (segment_reversed){
            if (match_forward_map[r_branch])
                branch_before->add_connection(TOP, this, BOTTOM, r_branch->get_reconstruction(), r_branch->get_confidence());
            else
                this->add_connection(BOTTOM, branch_before, TOP, r_branch->get_reconstruction(), r_branch->get_confidence());
        }else{
            if (match_forward_map[r_branch])
                this->add_connection(TOP, branch_before, BOTTOM, r_branch->get_reconstruction(), r_branch->get_confidence());
            else
                branch_before->add_connection(BOTTOM, this, TOP, r_branch->get_reconstruction(), r_branch->get_confidence());
        }
    }

    // Split matches [#Consider updating registration bins wherever split_branch is called, though probably unnecessary since these branches have already been matched and won't be compared again]
    branch_before->reset_confidence();
    for (BranchContainer * match : branch_matches){
        // Insert new branch between current and parent branch to match the new composite branch

        BranchContainer * parent = match->get_parent();
        parent->remove_child(match);
        BranchContainer * new_match = new BranchContainer(match->get_reconstruction(), match->get_segment(), parent, branch_before, match->get_confidence());
        match->set_parent(new_match);
        branch_before->add_branch_match(new_match, match_forward_map[match]);
        
        // #CONSIDER: Is there any need to also split up NeuronSegments of each Reconstruction?
        // For now we won't as it would require pointers/maps between reconstruction and composite markers, or at least vectors of gaps
        // (which should be rare given the resampling that must occur at the start)
    }

    printf("CBC::split_branch complete\n");

    return branch_before;
};


Composite * CompositeBranchContainer::get_composite(){
    return cbc_composite;
};
double CompositeBranchContainer::get_summed_confidence() const{
    return bc_confidence;
};
double CompositeBranchContainer::get_confidence() {
    return (double)bc_confidence / (double)confidence_denominator;
};
double CompositeBranchContainer::get_connection_confidence() const{
    get_connection_confidence(best_connection);
}
double CompositeBranchContainer::get_connection_confidence(Connection * connection) const{
    if (parent_connections.find(connection) == parent_connections.end())
        return 0;
    return connection->get_confidence() / confidence_denominator;
};

CompositeBranchContainer* CompositeBranchContainer::get_parent(){
    return cbc_parent;
};
std::set<CompositeBranchContainer*> CompositeBranchContainer::get_children(){
    return cbc_children;
};
std::set<Reconstruction *> CompositeBranchContainer::get_reconstructions(){
    std::set<Reconstruction *> reconstructions;
    for (BranchContainer * branch : branch_matches){
        reconstructions.insert(branch->get_reconstruction());
    }
    return reconstructions;
};

/*
void CompositeBranchContainer::add_connection(BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, double confidence){
    Connection * connection = new Connection(this, child_end, parent, parent_end);
    connection->set_confidence(confidence);
    add_connection(connection);
};
 */
void CompositeBranchContainer::add_connection(BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, Reconstruction * reconstruction, double confidence){

    if (confidence == 0 && reconstruction) confidence = reconstruction->get_confidence();
    Connection * connection = new Connection(this, child_end, parent, parent_end, reconstruction, confidence);
    add_connection(connection);
};

// Adds confidence for given connection, or creates new connection if it does not already exist
void CompositeBranchContainer::add_connection(Connection * connection){
    if (this == connection->get_child()){
        combined_connection_weight += connection->get_confidence();

//        std::set<Connection *>::iterator it = parent_connections.find(connection);
        Connection * existing_conn = nullptr;

        for (Connection * test_conn : parent_connections){
            if (test_conn->get_parent() == connection->get_parent() && test_conn->get_parent_end() == connection->get_parent_end() && test_conn->get_child_end() == connection->get_child_end()){
                existing_conn = test_conn;
            }
        }

        if (existing_conn){
            // If the connection exists for this branch, update existing connection
            existing_conn->increment_confidence(connection->get_confidence());
            existing_conn->add_reconstructions(connection->get_reconstructions());
            delete connection;
            connection = existing_conn;
        }else{
            // Otherwise insert the new connection
            parent_connections.insert(connection);
        }

        if (connection->get_parent()){
            connection->get_parent()->add_child_connection(connection);
        }

        if (!best_connection || connection->get_confidence() > best_connection->get_confidence()){
            best_connection = connection;
        }
    }else{
        //printf("NO! the child != this\n");
    }
};

void CompositeBranchContainer::import_connection(Connection * connection){
    if (this == connection->get_child())
        parent_connections.insert(connection);
};

// Might need to be tested and revisited, but I think this will work
void CompositeBranchContainer::remove_connection(Connection * connection){
    if (this == connection->get_child()){
        parent_connections.erase(connection);
        if (best_connection == connection){
            // Determine next best connection
            double highest_conf = 0;
            for (Connection * candidate : parent_connections){
                if (candidate->get_confidence() > highest_conf){
                    highest_conf = candidate->get_confidence();
                    best_connection = candidate;
                }
            }
        }
    }else{
        //logger->warn("Child of connection set to remove is not this branch");
    }
};

std::set<Connection *> CompositeBranchContainer::get_connections() const{
    return parent_connections;
};
double CompositeBranchContainer::get_combined_connection_weight() const{
    return combined_connection_weight;
}

void CompositeBranchContainer::add_child_connection(Connection * connection){
    if (connection->get_parent_end() == TOP){
        top_child_connections.insert(connection);
    }else{
        bottom_child_connections.insert(connection);
    }
};

void CompositeBranchContainer::remove_child_connection(Connection * connection){
    if (connection->get_parent_end() == TOP){
        top_child_connections.erase(connection);
    }
    else{
        bottom_child_connections.erase(connection);
    }
}
void CompositeBranchContainer::remove_child_connections(BranchEnd branch_end){
    std::set<Connection *> * child_connections;
    if (branch_end == TOP){
        for (Connection * connection : top_child_connections){
            connection->get_child()->remove_connection(connection);
        }
        top_child_connections.erase(top_child_connections.begin(), top_child_connections.end());
    }else{
        //child_connections = &bottom_child_connections;
        for (Connection * connection : bottom_child_connections){
            connection->get_child()->remove_connection(connection);
        }
        bottom_child_connections.erase(bottom_child_connections.begin(), bottom_child_connections.end());

    }
};


// Returns true if end1 is the top, false if end2 is the top (end1 is tail)
// double gives certainty of the decision (0 means 50/50, 1 means definitely the given end is top)
pair<bool,double> CompositeBranchContainer::get_direction_certainty(){
    float dir1_conf = 0, dir2_conf = 0;
    for(Connection * connection : parent_connections) {
        if (connection->get_child_end() == TOP){
            dir1_conf += connection->get_confidence();
        }else{
            dir2_conf += connection->get_confidence();
        }
    }
    /*
     typedef std::map<CompositeBranchContainer, float>::iterator it_type;
    for(it_type iterator = end_parents1.begin(); iterator != m.end(); iterator++) {
        dir1_conf += iterator->second;
    }
    for(it_type iterator = end_parents2.begin(); iterator != m.end(); iterator++) {
        dir2_conf += iterator->second;
    }
     */
     
    pair<bool,double> return_val;
    return_val.first = dir1_conf > dir2_conf;
    return_val.second = abs(dir1_conf-dir2_conf)/(dir1_conf+dir2_conf);
    return return_val;
};

// Returns pair of most likely connection and the entropy of the possibilities
pair<Connection*,double> CompositeBranchContainer::get_connection_entropy(){
    double entropy = 0, total_confidence = 0;
    typedef pair<Connection*, double> conn_entropy_pair;

    Connection * most_probable = nullptr;
    double highest_conf = 0;
    for(Connection * connection : parent_connections) {
        total_confidence += connection->get_confidence();
        if (most_probable == nullptr || connection->get_confidence() > most_probable->get_confidence()){
            most_probable = connection;
        }
    }
    for(Connection * connection : parent_connections) {
        double connection_prob = connection->get_confidence() / total_confidence;
        entropy += connection_prob * log(connection_prob);
    }
    
    return conn_entropy_pair(most_probable,entropy);
};

// Returns pair of most likely connection and the entropy of the possibilities given one end of the branch
pair<Connection*,double> CompositeBranchContainer::get_connection_entropy(BranchEnd branch_end){
    double entropy = 0, total_confidence = 0;
    typedef pair<Connection*, double> conn_entropy_pair;
    
    Connection * most_probable = nullptr;
    double highest_conf = 0;
    for(Connection * connection : parent_connections) {
        if (branch_end == connection->get_child_end()){
            total_confidence += connection->get_confidence();
            if (most_probable == nullptr || connection->get_confidence() > most_probable->get_confidence()){
                most_probable = connection;
            }
        }
    }
    for(Connection * connection : parent_connections) {
        if (branch_end == connection->get_child_end()){
            double connection_prob = connection->get_confidence() / total_confidence;
            entropy += connection_prob * log(connection_prob);
        }
    }
    
    return conn_entropy_pair(most_probable,entropy);
};

pair<Connection*,double> CompositeBranchContainer::get_best_connection_probability() const{
    if (best_connection){
//        printf("best_connection %p, combined_conn_weight %f\n",best_connection,combined_connection_weight);
        return pair<Connection*,double>(best_connection, best_connection->get_confidence() / combined_connection_weight);
    }
    return pair<Connection*,double>(nullptr,0);
};
void CompositeBranchContainer::calculate_best_connection(){
    best_connection = nullptr;
    for (Connection * connection : parent_connections){
        if (!best_connection || connection->get_confidence() > best_connection->get_confidence()){
            best_connection = connection;
        }
    }
};
Connection* CompositeBranchContainer::get_best_connection() const{
    return best_connection;
};
pair<Connection*,double> CompositeBranchContainer::get_best_connection_weight() const{
//    printf("best_connection %p\n",best_connection);
    if (best_connection){
  //      printf("combined_conn_weight %f\n",best_connection,combined_connection_weight);
        return pair<Connection*,double>(best_connection, best_connection->get_confidence());
    }
    return pair<Connection*,double>(nullptr,0);
};

// Better to use a different measure than best_connection_probability? Maybe entropy or some other alternative?
bool compare_branch_pointers_by_confidence(const CompositeBranchContainer * first, const CompositeBranchContainer * second){
    return first->get_best_connection_probability().second > second->get_best_connection_probability().second;
};
bool compare_branch_pointers_by_weight(const CompositeBranchContainer * first, const CompositeBranchContainer * second){
    return first->get_best_connection_weight().second > second->get_best_connection_weight().second;
};
bool compare_branches_by_confidence(const CompositeBranchContainer &first, const CompositeBranchContainer &second){
    return first.get_best_connection_probability().second > second.get_best_connection_probability().second;
};

/*******************
 **  Connection   **
 *******************/
/*
Connection::Connection(){};
Connection::Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, double confidence){
    c_child = child;
    c_child_end = child_end;
    c_parent = parent;
    c_parent_end = parent_end;
    c_confidence = confidence;
}
 */
Connection::Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, Reconstruction * reconstruction, double confidence){
    c_child = child;
    c_child_end = child_end;
    c_parent = parent;
    c_parent_end = parent_end;
    c_reconstructions.insert(reconstruction);
    if (confidence == 0){
        confidence = reconstruction->get_confidence();
    }
    c_confidence = confidence;
};
Connection::Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, std::set<Reconstruction *> reconstructions, double confidence){
    c_child = child;
    c_child_end = child_end;
    c_parent = parent;
    c_parent_end = parent_end;
    c_reconstructions.insert(reconstructions.begin(), reconstructions.end());
    c_confidence = confidence;
};
Connection::Connection(Connection * connection){
//    printf("Connection::Connection(Connection *) Enter\n");
    c_child = connection->get_child();
  //  printf("Connection::Connection(Connection *) 1\n");
    c_child_end = connection->get_child_end();
    //printf("Connection::Connection(Connection *) 2\n");
    c_parent = connection->get_parent();
    //printf("Connection::Connection(Connection *) 3\n");
    c_parent_end = connection->get_parent_end();
    add_reconstructions(connection->get_reconstructions());
    //printf("Connection::Connection(Connection *) 5\n");
    c_confidence = connection->get_confidence();
    //printf("Connection::Connection(Connection *) Exit\n");
}
void Connection::set_confidence(double confidence){
    c_confidence = confidence;
};
void Connection::increment_confidence(double confidence){
    c_confidence += confidence;
};
void Connection::add_reconstruction(Reconstruction *reconstruction){
    c_reconstructions.insert(reconstruction);
};
void Connection::add_reconstructions(std::set<Reconstruction *> reconstructions){
    c_reconstructions.insert(reconstructions.begin(), reconstructions.end());
};

CompositeBranchContainer * Connection::get_parent(){
    return c_parent;
};
CompositeBranchContainer * Connection::get_child(){
    return c_child;
};
BranchEnd Connection::get_parent_end(){
    return c_parent_end;
};
void Connection::set_parent(CompositeBranchContainer * parent){
    c_parent = parent;
};
BranchEnd Connection::get_child_end(){
    return c_child_end;
};
void Connection::set_child(CompositeBranchContainer * child){
    c_child = child;
};
double Connection::get_confidence(){
    return c_confidence;
};
std::set<Reconstruction *> Connection::get_reconstructions(){
    return c_reconstructions;
};

bool compare_connections(Connection &first, Connection &second){
    return first.get_confidence() > second.get_confidence();
};
bool compare_connection_ptrs(Connection *first, Connection *second){
    return first->get_confidence() > second->get_confidence();
};


/*******************
 **   Composite   **
 *******************/

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

Composite::Composite(){
    
};

// For initializing composite to the first reconstruction
Composite::Composite(Reconstruction * reconstruction){
    // Add reconstruction to vector
    reconstructions.push_back(reconstruction);
    //composite_reconstruction = reconstruction;
    //c_root_segment = copy_segment_tree(reconstruction->get_tree());
    add_first_reconstruction(reconstruction);
};

void Composite::delete_all(){
    std::set<CompositeBranchContainer *> branch_set = get_branches();
    for (CompositeBranchContainer * c_branch : branch_set){
        c_branch->prepare_for_deletion();
    }
    for (CompositeBranchContainer * c_branch : branch_set){
        for (MyMarker * marker : c_branch->get_segment()->markers){
            delete marker;
        }
        delete c_branch->get_segment();

        c_branch->delete_connections();
        delete c_branch;
    }
}

void Composite::set_root(CompositeBranchContainer * root){
    c_root = root;
    if (root)
        c_root_segment = root->get_segment();
    else
        c_root_segment = nullptr;
};

void Composite::add_first_reconstruction(Reconstruction * reconstruction){
    // Traverse reconstruction: copy branches and create decision points
    logger->debug1("Entered add_first_reconstruction");
    NeuronSegment *segment, *composite_segment;
    BranchContainer * branch, *branch_parent;
    CompositeBranchContainer *composite_parent;
    
    std::set<pair<CompositeBranchContainer *, CompositeBranchContainer *> > connection_pair;
    Connection * connection;

    // Determine whether segment markers go first to last or last to first
    bool markers_forward = false;
    for (NeuronSegment * segment : reconstruction->get_segments()){
        if (segment->markers.size() > 1){
            markers_forward = segment->markers[0]->parent != segment->markers[1];
            break;
        }
    }

    // Push root of reconstruction, and null pointer as there is no parent
    std::stack<pair<NeuronSegment *, BranchContainer *> > segment_stack;
    segment_stack.push(pair<NeuronSegment *, BranchContainer *>(reconstruction->get_tree(),nullptr));
    pair<NeuronSegment *, BranchContainer *> segment_pair;

    // Process each branch (create CompositeBranchContainers and DecisionPoints), handling via a stack
    int count = 0;
    while (!segment_stack.empty()){
        count++;
        segment_pair = segment_stack.top();
        segment_stack.pop();
        segment = segment_pair.first;
        logger->debug4("Going through segment stack");
        
        // Get parent of current reconstruction branch
        branch_parent = segment_pair.second;

        logger->debug4("Going through segment stack 2");

        // Create branch container for this segment
        //branch = new BranchContainer(reconstruction,segment,branch_parent);
        branch = reconstruction->get_branch_by_segment(segment);

        logger->debug4("Going through segment stack 3");

        // Copy segment and create composite branch out of it
        composite_segment = copy_segment_markers(segment);
        
        CompositeBranchContainer * composite_branch = new CompositeBranchContainer(composite_segment, this);
        
        logger->debug4("Going through segment stack 4");

        // Create links between branch and composite branch
        composite_branch->add_branch_match(branch, true);
        branch->set_composite_match(composite_branch);

        logger->debug4("Going through segment stack 5");

        // Create a Connection if the branch has a parent
        if (!branch_parent){
            logger->debug4("No branch parent");
            // If this is the root, create new composite reconstruction pointing to the root composite segment
            c_root = composite_branch;
            c_root_segment = composite_branch->get_segment();
            c_root->add_connection(TOP, nullptr, BOTTOM, reconstruction, reconstruction->get_confidence());
        }else{
            // Get composite parent of new composite branch
            logger->debug4("Yes branch parent");
            composite_parent = branch_parent->get_composite_match();
            logger->debug4("Setting branch parent");
            composite_branch->set_parent(composite_parent);
            
            // Add Connection
            logger->debug4("Adding connection");
            logger->debug4("Confidence: %f",reconstruction->get_confidence());
            composite_branch->add_connection(TOP, composite_parent, BOTTOM, reconstruction, reconstruction->get_confidence());

            NeuronSegment * parent_seg = composite_parent->get_segment();
            // Link marker at top of segment to it's parent marker
            if (markers_forward){
                // The first marker is the beginning of the segment
                segment->markers[0]->parent = parent_seg->markers.back();
            }else{
                // The last marker is the beginning of the segment
                segment->markers.back()->parent = parent_seg->markers[0];
            }
        }
        
        logger->debug4("Going through segment stack 6");

        // Add children of the current segment along with the newly created composite segment for connecting
        for (NeuronSegment *child : segment->child_list){
            segment_stack.push(pair<NeuronSegment *,BranchContainer *>(child, branch));
        }
        logger->debug4("Going through segment stack 7");
    }
    /*
    // Update marker head parent connection
    logger->debug4("add_first_reconstruction: Test update marker parents");
    for (CompositeBranchContainer * branch : get_branches()){
        if (branch->get_parent()){
        }
    }
    logger->debug4("add_first_reconstruction: End test update marker parents");
*/
    
    logger->debug("Processed %d segments in creating first composite",count);
};

/* #TODO: Takes in new branch and ???
void Composite::process_branch(BranchContainer * branch){
    
};
*/
//Composite::logger = new Logger(0);
Logger * Composite::logger = new Logger(0);
void Composite::set_logger(Logger * logger) { Composite::logger = logger; };

CompositeBranchContainer * Composite::get_root(){
    return c_root;
};
/*
Reconstruction * Composite::get_composite_reconstruction(){
    return composite_reconstruction;
}*/
NeuronSegment * Composite::get_root_segment(){
    return c_root_segment;
};
std::set<NeuronSegment *> Composite::get_segments(){
    return c_segments;
}
std::vector<NeuronSegment *> Composite::get_segments_ordered(){
    return produce_segment_vector(c_root_segment);
};

void Composite::add_reconstruction(Reconstruction * reconstruction){
    if (reconstructions.size() == 0){
        //composite_reconstruction = &reconstruction;
        add_first_reconstruction(reconstruction);
    }
    reconstructions.push_back(reconstruction);
};

void Composite::add_branch(CompositeBranchContainer * branch){
    c_branches.insert(branch);
//    logger->debug3("add_branch with segment pointer %p, branch pointer %p",branch->get_segment(), branch);
    c_segments.insert(branch->get_segment());
    segment_container_map[branch->get_segment()] = branch;
};
void Composite::remove_branch(CompositeBranchContainer * branch){
    c_branches.erase(branch);
    segment_container_map.erase(branch->get_segment());
    c_segments.erase(branch->get_segment());
};

std::vector<Reconstruction*> Composite::get_reconstructions(){
    return reconstructions;
};

double Composite::get_summary_confidence(){
    return summary_confidence;
};

std::set<CompositeBranchContainer*> Composite::get_branches(){
    return c_branches;
    /*
    std::set<CompositeBranchContainer*> compiled_branches;
    std::stack<CompositeBranchContainer*> cb_stack;
    cb_stack.push(c_root);
    CompositeBranchContainer * branch;
    while (!cb_stack.empty()){
        branch = cb_stack.top();
        cb_stack.pop();
        compiled_branches.insert(branch);
        for (CompositeBranchContainer * child : branch->get_children()){
            cb_stack.push(child);
        }
    }
    return compiled_branches;
     */
};
CompositeBranchContainer * Composite::get_branch_by_segment(NeuronSegment * segment){
    return segment_container_map[segment];
};
typedef map<NeuronSegment *,CompositeBranchContainer *> SegmentCBContainerMap;
typedef pair<NeuronSegment *,CompositeBranchContainer *> SegmentCBContainerPair;
void Composite::update_branch_by_segment(NeuronSegment * segment, CompositeBranchContainer * branch){
    segment_container_map[segment] = branch;
};

/** Need to check whether all data should be copied or just pointers.
 Current code only copies the pointers **/
Composite * Composite::copy(){
    return copy(true);
}
Composite * Composite::copy(bool copy_segments){
    Composite * copy = new Composite();
    
    logger->new_line();
    logger->debug("Composite::copy(%i)",copy_segments);
    std::map<CompositeBranchContainer *,CompositeBranchContainer *> new_to_old_map;
    std::map<CompositeBranchContainer *,CompositeBranchContainer *> old_to_new_map;
    
    logger->debug1("Composite::copy  creating new branches");
    std::set<CompositeBranchContainer*> tmp = c_root->get_children();
    logger->debug1("Composite::copy another earlier attempt at get_children on c_root - has %i children",tmp.size());
    std::vector<NeuronSegment*> tmp2 = c_root_segment->child_list;
    logger->debug1("Composite::copy another earlier attempt at get_children on c_root_segment - has %i children",tmp2.size());
    for (CompositeBranchContainer* child : tmp){
        logger->debug4("before ->get_segment, child %p", child);
        if (child){
            NeuronSegment * sg = child->get_segment();
            logger->debug4("got child segment %p",sg);
            logger->debug4("%i markers, first: %f %f %f",sg->markers.size(),sg->markers[0]->x,sg->markers[0]->y,sg->markers[0]->z);
            for (int i = 0; i < sg->markers.size(); i++){
                MyMarker * marker = sg->markers[i];
            }
        }
    }
    
    logger->debug3("number of branches in this composite %i",c_branches.size());
    for (CompositeBranchContainer * branch : c_branches){
        logger->debug4("Made it 1, branch ptr %p",branch);
        logger->debug4("segment ptr %p",branch->get_segment());
        CompositeBranchContainer * branch_copy = new CompositeBranchContainer(branch, copy, copy_segments);
        logger->debug4("Made it 2");
        new_to_old_map[branch_copy] = branch;
        old_to_new_map[branch] = branch_copy;
        //copy->add_branch(branch_copy); // Now done when creating new CBC
    }

    // Create parent/child pointers
    logger->debug1("Composite::copy connecting branches");
    int count = 0;
    for (std::map<CompositeBranchContainer *,CompositeBranchContainer *>::iterator it = new_to_old_map.begin(); it != new_to_old_map.end(); ++it){
        CompositeBranchContainer * new_branch = it->first;
        CompositeBranchContainer * orig_branch = it->second;
        //CompositeBranchContainer * orig_branch = new_to_old_map[new_branch];
        CompositeBranchContainer * orig_parent = orig_branch->get_parent();
        
        logger->debug4("new_branch %p, Num connections %i",new_branch,new_branch->get_connections().size());
        // Connections have been copied, now their references must be updated!
        for (Connection * connection : new_branch->get_connections()){
            connection->set_child(new_branch);
            if (connection->get_parent()){
                CompositeBranchContainer * new_parent = old_to_new_map[connection->get_parent()];
                connection->set_parent(new_parent);
                new_parent->add_child_connection(connection);
            }
        }

        count++;
        //printf("Branch connecting %d; orig_parent %p\n",count,orig_parent);
        
        if (orig_parent){
            CompositeBranchContainer * new_parent = old_to_new_map[orig_parent];
            // Make child->parent connection
            new_branch->set_parent(new_parent);
        }else {
            // No parent
            logger->debug4("no parent");
        }
    }
    copy->set_root(old_to_new_map[c_root]);
    logger->debug("Done making copy");
    
    return copy;
};

void Composite::update_tree(){
    logger->debug("Start Composite::update_tree()");
    Composite * running_consensus = generate_consensus(0, false);
    // Clear connections of the current tree
    for (NeuronSegment * segment : running_consensus->get_segments()){
        CompositeBranchContainer * my_branch = get_branch_by_segment(segment);
        my_branch->set_parent(nullptr);
    }
    
    logger->debug4("Beginning of update_tree; check root_branch %p %i",c_root,c_root->get_children().size());
    logger->debug4("Beginning of update_tree; check root_segment %p %i",c_root_segment,c_root_segment->child_list.size());

    
    // Update connections based on running_consensus
    for (NeuronSegment * segment : running_consensus->get_segments()){
        logger->debug4("processing segment %p",segment);
        CompositeBranchContainer * my_branch = get_branch_by_segment(segment);
        CompositeBranchContainer * consensus_branch = running_consensus->get_branch_by_segment(segment);
        CompositeBranchContainer * consensus_parent = consensus_branch->get_parent();
        if (consensus_parent){
            NeuronSegment * parent_segment = consensus_parent->get_segment();
            CompositeBranchContainer * my_parent = get_branch_by_segment(parent_segment);
            my_branch->set_parent(my_parent);
        }else{
            logger->debug("segment has no parent");
        }
        /* Consensus has chosen the root, deal with after loop
        else{
            my_branch->set_parent(nullptr);
            c_root = my_branch;
            c_root_segment = my_branch->get_segment();
            logger->debug("Setting composite root branch %p segment %p",c_root,c_root_segment);
        }
         */
    }
    // Determine and set root
    NeuronSegment *root_segment = running_consensus->get_root()->get_segment();
    set_root(get_branch_by_segment(root_segment));

    logger->debug4("End of update_tree; check root_branch %p %i",c_root,c_root->get_children().size());
    logger->debug4("End of update_tree; check root_segment %p %i",c_root_segment,c_root_segment->child_list.size());
    

    // Destroy running_consensus
    for (CompositeBranchContainer * c_branch : running_consensus->get_branches()){
        delete c_branch;
    }
    delete running_consensus;
    logger->debug2("End of update_tree");
}

Composite * Composite::generate_consensus(int branch_vote_threshold){
    double confidence_threshold = (double)branch_vote_threshold / reconstructions.size();
    return generate_consensus(confidence_threshold, true);
}
Composite * Composite::generate_consensus(int branch_vote_threshold, bool copy_segments){
    double confidence_threshold = (double)branch_vote_threshold / reconstructions.size();
    return generate_consensus(confidence_threshold, copy_segments);
}
Composite * Composite::generate_consensus(double branch_confidence_threshold){
    return generate_consensus(branch_confidence_threshold, true);
}
Composite * Composite::generate_consensus(double branch_confidence_threshold, bool copy_segments){
    // Copy CompositeBranch to allow for removal of branches and connections below threshold
    logger->debug("Making composite copy");

    logger->debug4("Before composite copy; check root_branch %p %i",c_root,c_root->get_children().size());
    logger->debug4("Before composite copy; check root_segment %p %i",c_root_segment,c_root_segment->child_list.size());

    Composite * consensus = copy(copy_segments); // Segment pointers transfered, not copied
    consensus->convert_to_consensus(branch_confidence_threshold);

    // Return consensus reconstruction object containing NeuronSegment* root
    return consensus;
};

std::map<NeuronSegment *, double> Composite::get_segment_confidences(){
    std::map<NeuronSegment *, double> confidence_map;
    logger->debug4("Composite::get_segment_confidences");
    for (NeuronSegment * segment : c_segments){
        CompositeBranchContainer * branch = segment_container_map[segment];
        confidence_map[segment] = branch->get_summed_confidence();
        printf("%f %f %f\n",branch->get_summed_confidence(),branch->get_confidence_denominator(),branch->get_confidence());
    }
    return confidence_map;
}

void Composite::convert_to_consensus(double branch_confidence_threshold){
    // Create set of segments that are below the confidence threshold
    // (they can be added back in in order to attach a branch/subtree that is otherwise unconnected from the rest of the tree)
    
    // Remove all parent/child relationships of all branches
    logger->debug("Clearing parent/child relationships");

    std::set<CompositeBranchContainer *> copied_branch_pointers = c_branches;
    std::vector<CompositeBranchContainer *> copied_branches;
    for (CompositeBranchContainer * branch : copied_branch_pointers){
        copied_branches.push_back(branch);
    }
    logger->debug("test3");
    
    logger->debug4("generate consensus 1, branch_confidence_threshold %f",branch_confidence_threshold);
    std::set<CompositeBranchContainer *> removed_segments;
    for (CompositeBranchContainer * branch : copied_branches){
        // Checking whether has high confidence
        logger->debug3("Branch starting at %f %f %f has confidence %f, being removed?",branch->get_segment()->markers[0]->x,branch->get_segment()->markers[0]->y,branch->get_segment()->markers[0]->z,branch->get_confidence());
        logger->debug3("Summed conf %f, conf denominator %f",branch->get_summed_confidence(),branch->get_confidence_denominator());
        if ((double)branch->get_confidence() < (double)branch_confidence_threshold){
            printf("Branch is below threshold and so will be removed %f < %f",branch->get_confidence(),branch_confidence_threshold);
            removed_segments.insert(branch);
        }
        
        // Removing connections
        branch->set_parent(nullptr);
        branch->remove_children();
    }
    logger->debug4("generate consensus 2, num removed segments %i",removed_segments.size());
    
    // #TODO: Loop while any connection_sets remain, taking the most confident (least conflicting) connection_set each loop
    //  Currently just going through with the initial confidence order
    
    /* Make decision on parentage and connectivity, setting directionality on each branch that gets connected (those that are not connected at all  must have a parent via the other decision points) */
    
    // Sort connection_sets by minimization of conflict; Start with branch with highest weight (combined confidence votes)
    std::sort(copied_branches.begin(), copied_branches.end(), &compare_branch_pointers_by_weight);
    logger->debug4("generate consensus 2.5; num copied branches %i",copied_branches.size());
    
    pair<NeuronSegment *,double> root_seg_conf = pair<NeuronSegment *,double>(nullptr,0);
    //std::map<CompositeBranchContainer *,int> branch_tree_size;
    //std::set<CompositeBranchContainer*> roots;

    c_root = nullptr;
    c_root_segment = nullptr;
    
    while (!copied_branches.empty()){
        CompositeBranchContainer * branch = *(copied_branches.begin());
        //copied_branches.erase(branch);
        // #TODO: Replace this with a sorted set, or something that would be faster
        copied_branches.erase(std::remove(copied_branches.begin(),copied_branches.end(),branch),copied_branches.end());
        
        std::set<Connection *> connections_set = branch->get_connections();
        logger->debug4("Number of available connections %i",connections_set.size());
        logger->debug4("Segment %p, num_markers %i",branch->get_segment(),branch->get_segment()->markers.size());
        logger->debug4("Segment first marker %f %f %f",branch->get_segment()->markers[0]->x,branch->get_segment()->markers[0]->y,branch->get_segment()->markers[0]->z);
        std::vector<Connection *> connections(connections_set.begin(),connections_set.end());
        if (connections.size() == 0){
            if (removed_segments.find(branch) != removed_segments.end())
                logger->warn("Probable error, this branch is above threshold but has no remaining potential connections");
            continue;
        }
        std::sort(connections.begin(), connections.end(), &compare_connection_ptrs);
        
        /** Resolve connection_set: 1. Determine best parent **/
        
        // In order of the connection with the greatest confidence, choose the first connection to a branch that has sufficient confidence
        Connection * chosen_connection = nullptr;
        for (std::vector<Connection *>::iterator conn_it = connections.begin(); conn_it != connections.end() && !chosen_connection; ++conn_it){
            Connection * connection = *conn_it;
            logger->debug4("LOOK HERE processing connection %p with parent %p",connection,connection->get_parent());
            if (connection->get_parent())
                logger->debug4("parent segment %p",connection->get_parent()->get_segment());
            if (removed_segments.find(connection->get_parent()) == removed_segments.end()){ // Branch has sufficient confidence not to have been removed
                logger->debug4("connection chosen");
                chosen_connection = connection;
                break;
            }
        }
        
        // If none of the parents are above threshold confidence
        if (!chosen_connection){
            logger->debug2("No connection found, see if one can be rescued of %i connections",connections.size());
            // One of the parents must be chosen using a combination of their branch confidence and the connection confidence
            double best_confidence = 0;
            for (std::vector<Connection *>::iterator conn_it = connections.begin(); conn_it != connections.end(); ++conn_it){
                Connection * connection = *conn_it;
                logger->debug4("connection %p",connection);
                double joint_conf = connection->get_parent()->get_confidence() * connection->get_confidence();
                if (joint_conf > best_confidence){
                    best_confidence = joint_conf;
                    chosen_connection = connection;
                }
            }
        }
        
        /** Create connections in consensus **/
        // Get parent CBC from within composite copy
        CompositeBranchContainer * chosen_parent = chosen_connection->get_parent();
        if (!chosen_parent){
            // Check whether this branch is the root branch
            logger->debug2("No parent, is this the root branch");
            if (!c_root){
                logger->debug2("This was the first parentless branch with the highest confidence, so it will be the root");
                c_root = branch;
                c_root_segment = branch->get_segment();
            }else{
                // Choose the next best connection
                for (std::vector<Connection *>::iterator conn_it = connections.begin(); conn_it != connections.end(); ++conn_it){
                    Connection * connection = *conn_it;
                    if (connection != chosen_connection && removed_segments.find(connection->get_parent()) == removed_segments.end()){
                        chosen_connection = connection;
                        chosen_parent = chosen_connection->get_parent();
                        break;
                    }
                }
                // If no parent is found? Currently do nothing
            }
        }
        
        logger->debug4("branch %p, parent %p",branch,chosen_parent);
        logger->debug4("branch markers %i",branch->get_segment()->markers.size());

        if (chosen_parent)
            logger->debug4("branch's parent segment has %i children",chosen_parent->get_segment()->child_list.size());

        BranchEnd parent_end = chosen_connection->get_parent_end();
        branch->set_parent(chosen_parent, true, chosen_connection->get_child_end(), parent_end);
        
        if (chosen_parent)
            logger->debug4("branch's parent segment has %i children, starts at %f %f %f",chosen_parent->get_segment()->child_list.size(),chosen_parent->get_segment()->markers[0]->x,chosen_parent->get_segment()->markers[0]->y,chosen_parent->get_segment()->markers[0]->z);
        
        if (c_root){
            logger->debug4("Middle4 of generate_consensus; check root_branch %p %i",c_root,c_root->get_children().size());
            logger->debug4("Middle4 of generate_consensus; check root_segment %p %i",c_root_segment,c_root_segment->child_list.size());
        }else{
            logger->debug4("Middle4 of generate_consensus; No root branch or segment yet");
        }
        
        // Reverse direction of branch and segment based on current direction and side of new connection to parent
        // True if the segment is not reversed and the connection is at the bottom, OR if the segment is reversed and the connection is at the top
        if (branch->is_segment_reversed() ^ chosen_connection->get_child_end() == BOTTOM){
            logger->debug2("reverse segment");
            branch->reverse_segment();
        }
        
        /** Resolve connection_set: 2. Update connections at the other end of each branch to ensure the directionality of the branches set by this decision **/
        // Remove all connections from the parent to its potential parents at the end connected to its child
        if (chosen_parent){
            if (parent_end == TOP)
                chosen_parent->remove_child_connections(BOTTOM);
            else
                chosen_parent->remove_child_connections(TOP);
        }
        
        // Remove all connections from the child to its potential children at the end connected to its parent
        CompositeBranchContainer * child = chosen_connection->get_child();
        child->remove_child_connections(chosen_connection->get_child_end());
        
        // Update order since modifying connections
        std::sort(copied_branches.begin(), copied_branches.end(), &compare_branch_pointers_by_weight);
    }
    logger->debug4("generate consensus 3; root segment start point %f %f %f",c_root_segment->markers[0]->x,c_root_segment->markers[0]->y,c_root_segment->markers[0]->z);
    /*
     // Determine whether segment markers go first to last or last to first
     bool markers_forward = false;
     for (NeuronSegment * segment : reconstructions[0]->get_segments()){
     if (segment->markers.size() > 1){
     markers_forward = segment->markers[0]->parent != segment->markers[1];
     break;
     }
     }
     */
    // Update marker head parent connection
    logger->debug4("Test update marker parents");
    for (CompositeBranchContainer * branch : get_branches()){
        if (branch->get_parent()){
            NeuronSegment *segment = branch->get_segment(), *parent_seg = branch->get_parent()->get_segment();
            // Link marker at top of segment to it's parent marker
            segment->markers[0]->parent = parent_seg->markers.back();
        }
    }
    logger->debug4("End test update marker parents");
};


void update_branch_tree_sizes(CompositeBranchContainer * branch, std::map<CompositeBranchContainer *,int> &branch_tree_size){
    int size = branch_tree_size[branch];
    branch = branch->get_parent();
    while(branch){
        branch_tree_size[branch] += size;
        branch = branch->get_parent();
    }
};
