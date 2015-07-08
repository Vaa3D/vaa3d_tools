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
#include <algorithm>
#include "Composite.h"

/**
 * Below are classes associated directly with the Composite
 */

/* Class CompositeBranchContainer */
CompositeBranchContainer::CompositeBranchContainer(){
    cbc_parent = nullptr;
    dp1 = nullptr;
    dp2 = nullptr;
};
CompositeBranchContainer::CompositeBranchContainer(NeuronSegment * segment){
    bc_segment = segment;
    cbc_parent = nullptr;
    dp1 = nullptr;
    dp2 = nullptr;
};
CompositeBranchContainer::CompositeBranchContainer(CompositeBranchContainer * branch){
    NeuronSegment * segment_copy = copy_segment_markers(branch->get_segment());
    set_segment(segment_copy);
    branch_matches = branch->get_branch_matches();
    bc_confidence = branch->get_confidence();
    confidence_denominator = branch->get_confidence_denominator();
    cbc_parent = nullptr;
    dp1 = nullptr;
    dp2 = nullptr;
};
void CompositeBranchContainer::add_branch_match(BranchContainer * match){
    branch_matches.push_back(match);
    bc_confidence += match->get_confidence();
    confidence_denominator += match->get_confidence();
};

void CompositeBranchContainer::add_branch_miss(double confidence){
    confidence_denominator += confidence;
}

void CompositeBranchContainer::set_parent(CompositeBranchContainer * parent){
    cbc_parent = parent;
    if (parent){
        cbc_parent->add_child(this);
        if (!parent->get_decision_point2()){
            DecisionPoint * dp = new DecisionPoint();
            parent->set_decision_point2(dp);
            dp1 = dp;
        }
    }
};
void CompositeBranchContainer::add_child(CompositeBranchContainer * child){
    if (child){
        cbc_children.insert(child);
        std::vector<NeuronSegment *>::iterator position = std::find(bc_segment->child_list.begin(), bc_segment->child_list.end(), child->get_segment());
        if (position == bc_segment->child_list.end()){
            bc_segment->child_list.push_back(child->get_segment());
        }
        if (child->get_parent() != this){
            child->set_parent(this);
        }
    }
};
void CompositeBranchContainer::remove_child(CompositeBranchContainer * child){
    if (child){
        if (child->get_parent() == this){
            child->set_parent(nullptr);
        }
        bc_children.erase(child);
        std::vector<NeuronSegment *> *vect = &bc_segment->child_list;
        std::vector<NeuronSegment *>::iterator position = std::find(vect->begin(), vect->end(), child->BranchContainer::get_segment());
        if (position != vect->end()) // == vector.end() means the element was not found
            vect->erase(position);
    }
};
void CompositeBranchContainer::remove_children(){
    for (CompositeBranchContainer * child : cbc_children){
        remove_child(child);
    }
}

double CompositeBranchContainer::get_confidence(){
    return bc_confidence;
};

CompositeBranchContainer* CompositeBranchContainer::get_parent(){
    return cbc_parent;
}
std::set<CompositeBranchContainer*> CompositeBranchContainer::get_children(){
    return std::set<CompositeBranchContainer*>(cbc_children);
}

/* To be implemented this will need to handle decision points and connections
void CompositeBranchContainer::set_children(std::vector<CompositeBranchContainer *> children){
    cbc_children = children;
};
*/
void CompositeBranchContainer::set_decision_point1(DecisionPoint * decision_point){
    dp1 = decision_point;
};
void CompositeBranchContainer::set_decision_point2(DecisionPoint * decision_point){
    dp2 = decision_point;
};
DecisionPoint * CompositeBranchContainer::get_decision_point1(){
    return dp1;
};
DecisionPoint * CompositeBranchContainer::get_decision_point2(){
    return dp2;
};

/* Class Connection */
Connection::Connection(){};
Connection::Connection(CompositeBranchContainer * parent, CompositeBranchContainer * child, Reconstruction * reconstruction, double confidence){
    c_parent = parent;
    c_child = child;
    c_reconstructions.insert(reconstruction);
    if (confidence == 0){
        confidence = reconstruction->get_confidence();
    }
    c_confidence = confidence;
};
void Connection::set_confidence(double confidence){
    c_confidence = confidence;
};
void Connection::add_reconstruction(Reconstruction *reconstruction){
    c_reconstructions.insert(reconstruction);
}

CompositeBranchContainer * Connection::get_parent(){
    return c_parent;
};
CompositeBranchContainer * Connection::get_child(){
    return c_child;
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


/* Class DecisionPoint */
DecisionPoint::DecisionPoint(){};
DecisionPoint::DecisionPoint(std::set<CompositeBranchContainer*> branches, Connection * connection){
    dp_branches = branches;
    dp_connections.increment(connection, connection->get_confidence());
};
DecisionPoint::DecisionPoint(DecisionPoint * decision_point){
    for (Connection * connection : decision_point->get_connections()){
        add_connection(connection);
    }
};
/*
void DecisionPoint::add_branch(CompositeBranchContainer * branch){
    dp_branches.insert(branch);
};*/
void DecisionPoint::add_connection(Connection * connection){
    dp_connections.increment(connection, connection->get_confidence());
    if (!connections_by_parent[connection->get_parent()]){
        connections_by_parent[connection->get_parent()] = new std::set<Connection *>();
    }
    connections_by_parent[connection->get_parent()]->insert(connection);
    connection_map[CompositeBranchPair(connection->get_parent(),connection->get_child())] = connection;

    // Insert branches
    dp_branches.insert(connection->get_parent());
    dp_branches.insert(connection->get_child());

    dp_conflict = -1; // set to sentinel
};
void DecisionPoint::remove_connection(Connection * connection){
    dp_connections.remove(connection);
    connections_by_parent[connection->get_parent()]->erase(connection);
    connection_map.erase(CompositeBranchPair(connection->get_parent(),connection->get_child()));
    dp_conflict = -1;
}
Connection * DecisionPoint::add_connection(CompositeBranchContainer * parent, CompositeBranchContainer * child, Reconstruction *reconstruction, double confidence){
    Connection * connection = new Connection(parent, child, reconstruction, confidence);
    add_connection(connection);
    return connection;
};
std::set<CompositeBranchContainer *> DecisionPoint::get_parents(){
    std::set<CompositeBranchContainer *> potential_parents;
    for(map<CompositeBranchContainer*,std::set<Connection*> *>::iterator it1 = connections_by_parent.begin(); it1 != connections_by_parent.end(); ++it1){
        potential_parents.insert(it1->first);
    }
    return potential_parents;
}

std::set<CompositeBranchContainer *> DecisionPoint::get_branches(){
    return std::set<CompositeBranchContainer *>(dp_branches);
};
std::set<Connection *> DecisionPoint::get_connections(){
    return std::set<Connection *>(dp_connections.get_objects());
};
std::set<Connection *> DecisionPoint::get_connections_from_parent(CompositeBranchContainer * branch){
    return *connections_by_parent[branch];
};

/* Calculate confidence from connections */
// #CONSIDER: What about dealing with branches that could be parents OR children!? Maybe start with connections with the greatest confidence.
//      The conflict calculation could probably be improved in other ways, maybe also streamlined.
double DecisionPoint::get_conflict(){
    if (dp_conflict == -1){
        dp_conflict = 0;
        std::set<CompositeBranchContainer*> potential_parents;
        std::set<Connection *> * connections1, * connections2;
        // Go through parent branches and determine where they conflict in terms of their children
        for(map<CompositeBranchContainer*,std::set<Connection*> * >::iterator it1 = connections_by_parent.begin(); it1 != connections_by_parent.end(); ++it1){
            potential_parents.insert(it1->first);
            connections1 = it1->second;
            for(map<CompositeBranchContainer*,std::set<Connection*> * >::iterator it2 = it1; it1 != connections_by_parent.end(); ++it1){
                if (it1 != it2){
                    connections2 = it2->second;
                    for (Connection * connection1 : *connections1){
                        for (Connection * connection2 : *connections2){
                            if (connection1->get_child() == connection2->get_child()){
                                dp_conflict += 1 - fabs(connection1->get_confidence() - connection2->get_confidence());
                            }
                        }
                    }
                }
            }
        }
//        dp_conflict = dp_connections.get_confidence(); // Not using the weighted poll as it doesn't capture what we want - two connections could be consistent with each other, as branch1->branch2 and branch1->branch3
    }
    return dp_conflict;
};

bool operator<(DecisionPoint& dp1, DecisionPoint& dp2){
    return (dp1.get_conflict() < dp2.get_conflict());
};

bool compare_decision_points(DecisionPoint &first, DecisionPoint &second){
  return first.get_conflict() < second.get_conflict();
};

/* Class Composite */
NeuronSegment* copy_segment_markers(NeuronSegment* segment){
    NeuronSegment* new_segment = new NeuronSegment();
    MyMarker * new_marker;
    for (MyMarker* marker : segment->markers){
//        printf("marker %p\n",marker);
        new_marker = new MyMarker(*marker);
//        printf("marker test point 2\n");
        new_segment->markers.push_back(new_marker);
//        printf("marker test point 3\n");
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
}

Composite::Composite(){
    
};

void Composite::set_root(CompositeBranchContainer * root){
    c_root = root;
    c_root_segment = root->get_segment();
}
// For initializing composite to the first reconstruction
Composite::Composite(Reconstruction * reconstruction){
    // Add reconstruction to vector
    reconstructions.push_back(reconstruction);
    //composite_reconstruction = reconstruction;
    //c_root_segment = copy_segment_tree(reconstruction->get_tree());
    add_first_reconstruction(reconstruction);
};

void Composite::add_first_reconstruction(Reconstruction * reconstruction){
    // Traverse reconstruction: copy branches and create decision points
    NeuronSegment *segment, *composite_segment;
    BranchContainer * branch, *branch_parent;
    CompositeBranchContainer *composite_parent;
    
    std::set<pair<CompositeBranchContainer *, CompositeBranchContainer *> > connection_pair;
    Connection * connection;
    DecisionPoint * decision_point;
    std::stack<pair<NeuronSegment *, BranchContainer *> > segment_stack;
    
    // Push root of reconstruction, and null pointer as there is no parent
    segment_stack.push(pair<NeuronSegment *, BranchContainer *>(reconstruction->get_tree(),nullptr));
    pair<NeuronSegment *, BranchContainer *> segment_pair;

    // Process each branch (create CompositeBranchContainers and DecisionPoints), handling via a stack
    int count = 0;
    while (!segment_stack.empty()){
        count++;
        segment_pair = segment_stack.top();
        segment_stack.pop();
        segment = segment_pair.first;
        
        // Get parent of current reconstruction branch
        branch_parent = segment_pair.second;
        
        // Create branch container for this segment
        //branch = new BranchContainer(reconstruction,segment,branch_parent);
        branch = reconstruction->get_branch_by_segment(segment);
        
        // Copy segment and create composite branch out of it
        composite_segment = copy_segment_markers(segment);
        CompositeBranchContainer * composite_branch = new CompositeBranchContainer(composite_segment);
        add_branch(composite_branch);

        // Create links between branch and composite branch
        composite_branch->add_branch_match(branch);
        branch->set_composite_match(composite_branch);
        
        // Create a Decision Point if the branch has a parent
        if (!branch_parent){
            // If this is the root, create new composite reconstruction pointing to the root composite segment
            c_root = composite_branch;
            c_root_segment = composite_branch->get_segment();
        }else{
            // Get composite parent of new composite branch
            composite_parent = branch_parent->get_composite_match();
            composite_branch->set_parent(composite_parent);
            
            // Create decision point (which creates the connection)
            if (composite_parent->get_decision_point2()){
                composite_branch->set_decision_point1(composite_parent->get_decision_point2());
            }else{
                decision_point = new DecisionPoint();
                decision_point->add_connection(composite_branch, composite_parent, reconstruction, reconstruction->get_confidence());
                composite_branch->set_decision_point1(decision_point);
                composite_parent->set_decision_point2(decision_point);
            }
        }
        
        // Add children of the current segment along with the newly created composite segment for connecting
        for (NeuronSegment *child : segment->child_list){
            segment_stack.push(pair<NeuronSegment *,BranchContainer *>(child, branch));
        }
    }
    printf("Processed %d segments in creating first composite\n",count);
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
}
vector<NeuronSegment *> Composite::get_segments(){
    vector<NeuronSegment *> segments;
    std::stack<NeuronSegment *> stack;
    stack.push(c_root_segment);
    NeuronSegment * segment;
    logger->debug("getting composite segments");
    int count = 0;
    while (!stack.empty()){
        count++;
        segment = stack.top();
        stack.pop();
        segments.push_back(segment);
        for (NeuronSegment * child : segment->child_list){
            stack.push(child);
        }
    }
    logger->debug2("Last segment size %d",segment->child_list.size());
    logger->debug("got %d segments",count);
    return segments;
}

void Composite::add_reconstruction(Reconstruction * reconstruction){
    if (reconstructions.size() == 0){
        //composite_reconstruction = &reconstruction;
        add_first_reconstruction(reconstruction);
    }
    reconstructions.push_back(reconstruction);
};

void Composite::add_branch(CompositeBranchContainer * branch){
    branches.push_back(branch);
    segment_container_map[branch->get_segment()] = branch;
};

void Composite::add_decision_point(DecisionPoint * decision_point){
    decision_points.push_back(decision_point);
};


std::vector<Reconstruction*> Composite::get_reconstructions(){
    return reconstructions;
};

double Composite::get_summary_confidence(){
    return summary_confidence;
};

std::vector<CompositeBranchContainer*> Composite::get_branches(){
    //return branches;
    std::vector<CompositeBranchContainer*> compiled_branches;
    std::stack<CompositeBranchContainer*> cb_stack;
    cb_stack.push(c_root);
    CompositeBranchContainer * branch;
    while (!cb_stack.empty()){
        branch = cb_stack.top();
        cb_stack.pop();
        compiled_branches.push_back(branch);
        for (CompositeBranchContainer * child : branch->get_children()){
            cb_stack.push(child);
        }
    }
    return compiled_branches;
};
CompositeBranchContainer * Composite::get_branch_by_segment(NeuronSegment * segment){
    return segment_container_map[segment];
};
typedef map<NeuronSegment *,CompositeBranchContainer *> SegmentCBContainerMap;
typedef pair<NeuronSegment *,CompositeBranchContainer *> SegmentCBContainerPair;
void Composite::update_branch_by_segment(NeuronSegment * segment, CompositeBranchContainer * branch){
    segment_container_map[segment] = branch;
}

std::vector<DecisionPoint*> Composite::get_decision_points(){
    return decision_points;
};

/** Currently not used. Need to check whether all data should be copied or just pointers. 
 Current code only copies the pointers **/
Composite * Composite::copy(){
    Composite * copy = new Composite();
    
    printf("Composite::copy 1\n");
    std::map<CompositeBranchContainer *,CompositeBranchContainer *> new_to_old_map;
    std::map<CompositeBranchContainer *,CompositeBranchContainer *> old_to_new_map;
    
    printf("Composite::copy  creating new branches\n");
    for (CompositeBranchContainer * branch : get_branches()){
        CompositeBranchContainer * branch_copy = new CompositeBranchContainer(branch);
        new_to_old_map[branch_copy] = branch;
        old_to_new_map[branch] = branch_copy;
        copy->add_branch(branch_copy);
    }
    //for (CompositeBranchContainer * new_branch : branches){
    // Create parent/child pointers
    printf("Composite::copy connecting branches\n");
    int count = 0;
    for (std::map<CompositeBranchContainer *,CompositeBranchContainer *>::iterator it = new_to_old_map.begin(); it != new_to_old_map.end(); ++it){
        CompositeBranchContainer * new_branch = it->first;
        CompositeBranchContainer * orig_branch = it->second;
        //CompositeBranchContainer * orig_branch = new_to_old_map[new_branch];
        CompositeBranchContainer * orig_parent = orig_branch->get_parent();
        
        count++;
        //printf("Branch connecting %d; orig_parent %p\n",count,orig_parent);
        
        if (orig_parent){
            CompositeBranchContainer * new_parent = old_to_new_map[orig_parent];
            // Make child->parent connection
            new_branch->set_parent(new_parent);

            // Copy parent-side decision points
            if (new_parent->get_decision_point2()){
                // When the copied decision point already exists
                new_branch->set_decision_point1(new_parent->get_decision_point2());
            }else{
                DecisionPoint * dp_copy = new DecisionPoint(orig_branch->get_decision_point1());
                new_branch->set_decision_point1(dp_copy);
                decision_points.push_back(dp_copy);
            }
        }else{
            // No parent, so this branch must be the root
            printf("setting root\n");
            copy->set_root(new_branch);
        }
        // Copy child-side decision point (if it isn't a certain terminal branch)
        if (orig_branch->get_decision_point2()){
            //printf("copying dp2 %p\n",orig_branch->get_decision_point2());
            DecisionPoint * dp_copy = new DecisionPoint(orig_branch->get_decision_point2());
            new_branch->set_decision_point2(dp_copy);
            decision_points.push_back(dp_copy);
        }
    }
    printf("Done making copy\n");
    
    return copy;
}

Reconstruction * Composite::generate_consensus(double branch_confidence_threshold){
    Reconstruction * consensus;

    // Copy CompositeBranch and DecisionPoint structure to allow for removal of branches and connections below threshold
    printf("Making composite copy\n");
    Composite * comp_copy = copy();

    // First remove all parent/child relationships of all branches
    printf("Clearing parent/child relationships\n");
    CompositeBranchContainer * root_branch = comp_copy->get_root(); // Not sure if this is needed
    for (CompositeBranchContainer * branch : comp_copy->get_branches()){
        branch->set_parent(nullptr);
        branch->remove_children();
    }
        

    // #TODO: Loop while any decision points remain, taking the most confident (least conflicting) decision point each loop
    //  Currently just going through with the initial confidence order
    
    
    // Make decision on parentage and connectivity, setting directionality on each branch that gets connected (those that are not connected at all must have a parent via the other decision points)
    
    // #CONSIDER: If a branch can only be a termination at one end, do we have to connect it to a parent at the other end, or can it be eliminated from the consensus even though it surpassed threshold?
    //   For now, we'll enforce connectivity in order to keep it in the composite

    // #TODO:fix - Sort decision points by minimization of conflict
    //std::sort(comp_copy.decision_points.begin(),comp_copy.decision_points.end(),&compare_decision_points);
    int dp_num = 0;
    for (DecisionPoint * dp : comp_copy->decision_points){
        printf("Decision point %d\n",++dp_num);

        /** Resolve decision point
         *   1. Determine best set of parent->child connections
         *   2. Update decision points at the other end of each branch to ensure the directionality of the branches set by this decision
         **/
        
        std::set<CompositeBranchContainer*> potential_parents = dp->get_parents();
        std::set<Connection *> connections1, connections2;
        
        // Start with those with the greatest confidence
        std::set<Connection*> connections = dp->get_connections();
        //std::sort(connections.begin(),connections.end(),&compare_connections);
        
        // #TODO: Change method to go through all possible combinations and get most optimal set of connections in terms of joint confidence
        
        // For now, we're just taking the single most confident connection, then the next, creating each connection so long as it is still possible
        //  - A single branch cannot be the child of two branches
        //  - A single branch can only be a child or a parent
        for(Connection * connection : connections){
            connection->get_child()->set_parent(connection->get_parent());
            // Handle other side decision point of each connection
            DecisionPoint * other_dp = connection->get_parent()->get_decision_point1();
            if (other_dp == dp){
                other_dp = connection->get_parent()->get_decision_point2();
            }
            if (other_dp){ // Other side might be a termination
                // Remove all connections in which this parent branch is a parent on the other side
                for (Connection * bad_con : other_dp->get_connections_from_parent(connection->get_parent())){
                    other_dp->remove_connection(bad_con);
                }
            }else{
                // #TODO: Throw an error because the other side of a parent should certainly have a connection point if its parent-connection here still exists
            }
            other_dp = connection->get_child()->get_decision_point1();
            if (other_dp == dp){
                other_dp = connection->get_child()->get_decision_point2();
            }
            if (other_dp){ // Other side might be a termination, which would be fine
                // Remove all connections in which this child branch is a child on the other side
                for (Connection * con_check : other_dp->get_connections()){
                    if (con_check->get_child() == connection->get_child()){
                        other_dp->remove_connection(con_check);
                    }
                }
            }
            
            // #TODO: Update conflict estimation of other decision points
        }

    }
    
    /** Remove branches below threshold AND update NeuronSegment connections
     *  Remove below threshold
     *   1. Go through terminal branches
     *   2. Remove branch if it is below threshold and has no children
     *   3. Go to the parent branch of the removed branch
     **/
    if (branch_confidence_threshold > 0){
        std::set<CompositeBranchContainer*> terminal_branches;
        for (CompositeBranchContainer* branch : comp_copy->get_branches()){
            if (branch->get_children().size() == 0){
                terminal_branches.insert(branch);
                branch->get_segment()->child_list.clear();
            }else{
                branch->get_segment()->child_list.clear();
                for (CompositeBranchContainer * child : branch->get_children()){
                    branch->get_segment()->child_list.push_back(child->get_segment());
                }
            }
        }
        for (CompositeBranchContainer* branch : terminal_branches){
            while (branch->get_confidence() < 0 && branch->get_children().size() == 0){
                // remove and get parent
                CompositeBranchContainer * parent = branch->get_parent();
                parent->remove_child(branch);
                vector<NeuronSegment*> child_list = parent->get_segment()->child_list;
                vector<NeuronSegment*>::iterator it = std::find(child_list.begin(),child_list.end(),branch->get_segment());
                if (it != child_list.end()){
                    child_list.erase(it);
                }
                branch = parent;
            }
        }
    }
    
    // #TODO: Delete comp_copy, its branches, connections, and decision points, without deleting the segments
    
    // Return consensus reconstruction object containing NeuronSegment* root
    return consensus;
};



