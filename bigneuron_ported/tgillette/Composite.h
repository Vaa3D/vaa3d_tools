//
//  Composite.h
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/28/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#ifndef BigNeuron_Consensus_Builder_Composite_h
#define BigNeuron_Consensus_Builder_Composite_h

#include <vector>
#include <iterator>
#include "logger.h"
#include "tree_matching/swc_utils.h"
#include "Reconstruction.h"
#include "Poll.h"

class BranchContainer;
class DecisionPoint;
class Connection;
class NeuronSegment;
class Reconstruction;

class CompositeBranchContainer : public BranchContainer{
    std::vector<BranchContainer *> branch_matches; // Should this be a set?
    double confidence_denominator = 0;
    // It is assumed that dp1 contains parent connections and dp2 contains child connections.
    // In the care of a reconstruction branch matching in reverse, the proper ends will be handled in the matching procedure in ConsensusBuilder
    DecisionPoint * dp1;
    DecisionPoint * dp2;
    CompositeBranchContainer * cbc_parent;
    std::set<CompositeBranchContainer *> cbc_children;
    
    void add_child(CompositeBranchContainer * branch); // Called reciprocally when parent is set
public:
    CompositeBranchContainer();
    CompositeBranchContainer(NeuronSegment * segment);
    CompositeBranchContainer(CompositeBranchContainer * branch); // Copies branch's segments, branch_matches, and confidence values
    
    void set_parent(CompositeBranchContainer * parent);
    //void set_children(std::vector<CompositeBranchContainer *> children);
    void add_branch_match(BranchContainer * match);
    void add_branch_miss(double confidence);
    void remove_child(CompositeBranchContainer * child);
    void remove_children();
    void add_parent_connection(Connection * connection);
    void set_decision_point1(DecisionPoint * decision_point);
    void set_decision_point2(DecisionPoint * decision_point);

    NeuronSegment * get_segment(){ return BranchContainer::bc_segment; };
    CompositeBranchContainer * get_parent();
    std::vector<BranchContainer *> get_branch_matches() { return branch_matches; };
    std::set<CompositeBranchContainer *> get_children();
    double get_confidence();
    double get_confidence_denominator(){ return confidence_denominator; };
    DecisionPoint * get_decision_point1();
    DecisionPoint * get_decision_point2();

};

class Connection{
    CompositeBranchContainer * c_parent;
    CompositeBranchContainer * c_child;
    double c_confidence;
    std::set<Reconstruction *> c_reconstructions; // Point to reconstruction so we know where this comes from
public:
    Connection();
    Connection(CompositeBranchContainer * parent, CompositeBranchContainer * child, Reconstruction * reconstruction, double confidence=0);
    void set_confidence(double confidence);
    
    void add_reconstruction(Reconstruction * reconstruction);
    CompositeBranchContainer * get_parent();
    CompositeBranchContainer * get_child();
    void set_child(CompositeBranchContainer * child);
    double get_confidence();
    std::set<Reconstruction *> get_reconstructions();

    bool operator==(const Connection& rhs) const
    {
        return (c_parent == rhs.c_parent)
        && (c_child == rhs.c_child);
    }
    bool operator!=(const Connection& rhs) const
    {
        return !operator==(rhs);
    }
};

bool compare_connections(Connection &first, Connection &second);


typedef pair<CompositeBranchContainer*,CompositeBranchContainer*> CompositeBranchPair;
typedef pair<CompositeBranchContainer*,Connection*> ParentConnectionPair;

class DecisionPoint{
    std::set<CompositeBranchContainer *> dp_branches;
    std::map<CompositeBranchPair, Connection *> connection_map;
    std::map<CompositeBranchContainer *, std::set<Connection *> *> connections_by_parent;
    //WeightedPoll<Connection> dp_connections;
    WeightedPoll dp_connections;
    double dp_conflict; // To be set based on relative confidence of the available connections
public:
    DecisionPoint();
    DecisionPoint(std::set<CompositeBranchContainer *> branches, Connection * connection);
    DecisionPoint(DecisionPoint * decision_point);
    // DecisionPoint(); // #CONSIDER: See note in Composite with regards to destructors
    
    //void add_branch(CompositeBranchContainer * branch);
    void add_connection(Connection * connection);
    Connection * add_connection(CompositeBranchContainer * child, CompositeBranchContainer * parent, Reconstruction * reconstruction, double confidence=1);
    void remove_connection(Connection * connection);
    
    std::set<CompositeBranchContainer *> get_branches();
    std::set<Connection *> get_connections();
    std::set<CompositeBranchContainer *> get_parents();
    
    double get_conflict();
    CompositeBranchContainer * get_best_parent();
    std::set<Connection *> get_connections_from_parent(CompositeBranchContainer *);
    std::set<Connection *> get_acceptable_connections(double threshold);
  /*
    bool operator<( const DecisionPoint& lhs , const DecisionPoint& rhs )
    {
        return lhs.get_conflict(); < rhs.get_conflict();
    }*/
};

bool compare_decision_points(DecisionPoint &first, DecisionPoint &second);

class Composite{
    CompositeBranchContainer * c_root;                  // Root composite branch of tree being used as running consensus
    std::vector<CompositeBranchContainer *> branches;   // Vector of all composite branches
// #TODO: Need to handle composite_reconstruction - when first is loaded, and with running consensus
    //Reconstruction * composite_reconstruction;          // Reconstruction object used for running consensus
    NeuronSegment * c_root_segment;                     // Root segment of composite tree
    std::vector<Reconstruction *> reconstructions;      // Vector of contributing reconstructions that have been processed
    double summary_confidence;                          // Total confidence (calculated when asked for)
    std::map<NeuronSegment *, CompositeBranchContainer *> segment_container_map;    // Map from a segment to its composite branch container
    std::vector<DecisionPoint *> decision_points;       // Vector of all decision_points
    
    void process_branch(BranchContainer * branch);        // UNUSED - might use as alternate to certain code in ConsensusBuilder
    void add_first_reconstruction(Reconstruction * reconstruction);  // Makes composite out of the first individual reconstruction
    static Logger * logger;
public:
    Composite();
    Composite(Reconstruction * reconstruction); // For initializing composite to the first reconstruction
    //~Composite(); // #CONSIDER: using a destructor would allow for the destruction of all CompositeBranchContainers, DecisionPoints, and Connections, but what about NeuronSegments and matched BranchContainers. I would assume that they should be handled separately, where appropriate, before destroying the Composite
    Composite * copy();
    
    void set_root(CompositeBranchContainer * composite_branch);
    void add_reconstruction(Reconstruction * reconstruction);
    void add_branch(CompositeBranchContainer * branch);
    void add_decision_point(DecisionPoint * decision_point);
    void update_branch_by_segment(NeuronSegment * segment, CompositeBranchContainer * branch);
//    void set_onfidence(double confidence);

    CompositeBranchContainer * get_root();
    //Reconstruction * get_composite_reconstruction();
    NeuronSegment * get_root_segment();
    std::vector<NeuronSegment *> get_segments();
    std::vector<Reconstruction *> get_reconstructions();
    double get_summary_confidence();
    CompositeBranchContainer * get_branch_by_segment(NeuronSegment * segment);
    std::vector<CompositeBranchContainer *> get_branches();
    std::vector<DecisionPoint *> get_decision_points();
    Reconstruction * generate_consensus(double branch_confidence_threshold);
    
    static void set_logger(Logger * logger);
};

NeuronSegment * copy_segment_tree(NeuronSegment * root);
NeuronSegment * copy_segment_markers(NeuronSegment* segment);

#endif
