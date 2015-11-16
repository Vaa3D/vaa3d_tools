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

enum BranchEnd {TOP, BOTTOM};

/******************************
 ** CompositeBranchContainer **
 ******************************/
class CompositeBranchContainer : public BranchContainer{
    // ? Pointer to composite ? If so, constructor MUST contain composite as parameter
    Composite * cbc_composite;
    bool segment_reversed; // Value relative to the initial order of segment markers
    
    std::vector<BranchContainer *> branch_matches; // Should this be a set?
    std::map<BranchContainer *,bool> match_forward_map;
    double confidence_denominator = 0;

    // Current dominant connectivity
    CompositeBranchContainer * cbc_parent;
    std::set<CompositeBranchContainer *> cbc_children;
    void add_child(CompositeBranchContainer * branch); // Called reciprocally when parent is set

    // Possible connectivity members
    std::set<Connection *> parent_connections; // Possible connections to branch's parent, each tallying confidence from contributing reconstructions
    Connection * best_connection;
    double combined_connection_probability;

    std::set<Connection *> top_child_connections;
    std::set<Connection *> bottom_child_connections;
public:
    CompositeBranchContainer();
    CompositeBranchContainer(NeuronSegment * segment, Composite * composite);
    CompositeBranchContainer(CompositeBranchContainer * branch, bool copy_segment); // Copies branch_matches, and confidence values, and segment as specified
    CompositeBranchContainer(CompositeBranchContainer * branch, Composite * composite, bool copy_segment);
    CompositeBranchContainer(CompositeBranchContainer * branch); // Copies branch's segments, branch_matches, and confidence values
    CompositeBranchContainer(CompositeBranchContainer * branch, Composite * composite); // Copies branch's segments, branch_matches, and confidence values
    ~CompositeBranchContainer();
    
    void copy_from(CompositeBranchContainer * branch, Composite * composite, bool copy_segment);
    
    void set_parent(CompositeBranchContainer * parent, bool follow=true, BranchEnd branch_end=TOP, BranchEnd parent_end=BOTTOM);
    void remove_child(CompositeBranchContainer * child, bool follow=true);
    void remove_children();

    //void set_children(std::vector<CompositeBranchContainer *> children);
    void add_branch_match(BranchContainer * match);
    void add_branch_match(BranchContainer * match, bool match_forward);
    bool is_match_forward(BranchContainer * match);
    void add_branch_miss(double confidence);
    void set_segment_reversed(bool segment_reversed);
    void reverse_segment();
    bool is_segment_reversed();
    
    
    // Returns new branch above the split; Transfers all connections on the TOP end to the new branch
    CompositeBranchContainer * split_branch(std::size_t const split_point);

    Composite * get_composite();
    NeuronSegment * get_segment() const { return BranchContainer::bc_segment; };
    CompositeBranchContainer * get_parent();
    std::vector<BranchContainer *> get_branch_matches() { return branch_matches; };
    std::set<CompositeBranchContainer *> get_children();
    std::set<Reconstruction *> get_reconstructions();

    double get_confidence() const;
    double get_summed_confidence() const;
    double get_connection_confidence() const;
    double get_connection_confidence(Connection * connection) const;
    double get_confidence_denominator() const{ return confidence_denominator; };
    
    /* Possible connectivity methods */
    // Adds vote for connection as described by params, or creates new connection if it does not already exist
    //void add_connection(BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, double confidence=1);
    void add_connection(BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, Reconstruction * reconstruction, double confidence=0);
    void add_connection(Connection * connection);
    // Adds connection in full to parent_connections (assuming this CBC is child)
    void import_connection(Connection * connection);
    void remove_connection(Connection * connection);

    void add_child_connection(Connection * connection);
    
    void remove_child_connections(BranchEnd branch_end);

    Connection * get_best_connection();
    std::set<Connection *> get_connections() const;
    
    pair<Connection*,double> get_best_connection_probability() const;
    pair<bool,double> get_direction_certainty();
    pair<Connection*,double> get_connection_entropy();
    pair<Connection*,double> get_connection_entropy(BranchEnd branch_end);

    bool operator < (const CompositeBranchContainer& branch) const
    {
        return (this->get_confidence() < branch.get_confidence());
    }
    bool operator == (CompositeBranchContainer& rhs) const
    {
        return (this->get_segment() == rhs.get_segment());
    }

};

bool compare_branch_pointers_by_confidence(const CompositeBranchContainer * first, const CompositeBranchContainer * second);
bool compare_branches_by_confidence(const CompositeBranchContainer &first, const CompositeBranchContainer &second);

/*******************
 **   Connection  **
 *******************/

// Tracks the connection between a child and parent given a particular end (TOP or BOTTOM) for each.
// Also tracks which reconstructions voted for this connection, and the summed confidence of those contributions.
class Connection{
    CompositeBranchContainer * c_child;
    BranchEnd c_child_end;
    CompositeBranchContainer * c_parent;
    BranchEnd c_parent_end;
    double c_confidence;
    std::set<Reconstruction *> c_reconstructions; // Point to reconstructions so we know where this Connection comes from
public:
    /*
    Connection();
    Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, double confidence=1);
     */
    Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, Reconstruction * reconstruction, double confidence=0);
    Connection(CompositeBranchContainer * child, BranchEnd child_end, CompositeBranchContainer * parent, BranchEnd parent_end, std::set<Reconstruction *> reconstructions, double confidence);

    CompositeBranchContainer * get_child();
    BranchEnd get_child_end();
    CompositeBranchContainer * get_parent();
    BranchEnd get_parent_end();
    double get_confidence();
    std::set<Reconstruction *> get_reconstructions();

    // This is kind of dangerous, but it does make some things easier - consider safer way of achieving this (CompositeBranchContainer::split_branch)
    void set_child(CompositeBranchContainer * child);
/*
    void set_parent(CompositeBranchContainer * parent);
    void set_confidence(double confidence);
 */
    void set_confidence(double confidence);
    void increment_confidence(double confidence);
    void add_reconstruction(Reconstruction * reconstruction);
    void add_reconstructions(std::set<Reconstruction *> reconstructions);

    bool operator==(const Connection& rhs) const
    {
        return (c_parent == rhs.c_parent)
        && (c_parent_end == rhs.c_parent_end)
        && (c_child == rhs.c_child)
        && (c_child_end == rhs.c_child_end);
    }
    bool operator!=(const Connection& rhs) const
    {
        return !operator==(rhs);
    }
    bool operator<(const Connection& rhs) const
    {
        return c_parent < rhs.c_parent ? true :
                c_parent > rhs.c_parent ? false :
                c_parent_end < rhs.c_parent_end ? true :
                c_parent_end > rhs.c_parent_end ? false :
                c_child < rhs.c_child ? true :
                c_child > rhs.c_child ? false :
                c_child_end < rhs.c_child_end ? true : false;
    }
};

bool compare_connections(Connection &first, Connection &second);
bool compare_connection_ptrs(Connection * first, Connection * second);

typedef pair<CompositeBranchContainer*,CompositeBranchContainer*> CompositeBranchPair;
typedef pair<CompositeBranchContainer*,Connection*> ParentConnectionPair;

/*******************
 **   Composite   **
 *******************/
class Composite{
    CompositeBranchContainer * c_root;                  // Root composite branch of tree being used as running consensus
    std::set<CompositeBranchContainer *> branches;   // Set of all composite branches
// #TODO: Need to handle composite_reconstruction - when first is loaded, and with running consensus
    //Reconstruction * composite_reconstruction;          // Reconstruction object used for running consensus
    NeuronSegment * c_root_segment;                     // Root segment of composite tree
    std::vector<Reconstruction *> reconstructions;      // Vector of contributing reconstructions that have been processed
    double summary_confidence;                          // Total confidence (calculated when asked for)
    std::set<NeuronSegment *> c_segments;
    std::map<NeuronSegment *, CompositeBranchContainer *> segment_container_map;    // Map from a segment to its composite branch container
    //std::set<Connection *> connections;       // Vector of all Connections
    
    void process_branch(BranchContainer * branch);        // UNUSED - might use as alternate to certain code in ConsensusBuilder
    void add_first_reconstruction(Reconstruction * reconstruction);  // Makes composite out of the first individual reconstruction
    static Logger * logger;
public:
    Composite();
    Composite(Reconstruction * reconstruction); // For initializing composite to the first reconstruction
    //~Composite(); // #CONSIDER: using a destructor would allow for the destruction of all CompositeBranchContainers, DecisionPoints, and Connections, but what about NeuronSegments and matched BranchContainers. I would assume that they should be handled separately, where appropriate, before destroying the Composite
    
    void delete_all();
    

    Composite * copy();
    Composite * copy(bool copy_segments);
    
    void set_root(CompositeBranchContainer * composite_branch);
    void add_reconstruction(Reconstruction * reconstruction);
    void add_branch(CompositeBranchContainer * branch);
    void update_branch_by_segment(NeuronSegment * segment, CompositeBranchContainer * branch);
//    void set_onfidence(double confidence);

    CompositeBranchContainer * get_root();
    //Reconstruction * get_composite_reconstruction();
    NeuronSegment * get_root_segment();
    std::set<NeuronSegment *> get_segments();
    std::vector<NeuronSegment *> get_segments_ordered();
    std::vector<Reconstruction *> get_reconstructions();
    double get_summary_confidence();
    CompositeBranchContainer * get_branch_by_segment(NeuronSegment * segment);
    std::set<CompositeBranchContainer *> get_branches();
    //std::set<CompositeBranchContainer> get_branches();
    
    // Updates the tree using generate_consensus and a threshold of 0; maintains all connection data
    void update_tree();
    // Returns a new composite with branches below threshold removed and connections chosen
    Composite * generate_consensus(int branch_vote_threshold);
    Composite * generate_consensus(int branch_vote_threshold, bool copy_segments);
    Composite * generate_consensus(double branch_confidence_threshold);
    Composite * generate_consensus(double branch_confidence_threshold, bool copy_segments);
    
    static void set_logger(Logger * logger);
};

NeuronSegment * copy_segment_tree(NeuronSegment * root);
NeuronSegment * copy_segment_markers(NeuronSegment* segment);

#endif
