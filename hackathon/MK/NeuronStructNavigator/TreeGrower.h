//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  TreeGrower class intends to handle the functionalities needed to form a NeuronTree from segmented image signals (ImgAnalyzer::connectedComponent).
*  For TreeGrower, NeuronGeoGrapher is an essential class for which many TreeGrower's methods are further development extended from methods in NeuronGeoGrapher class.
*
*  Major functionalities include:
*
*    a. Basic tree operations, i.e., tree trimming, refining
*    b. Tree path tracing for tree identification purposes
*    c. Segment forming / elongating and other operations
*    d. Dendritic tree and axonal tree forming
*
*  This class is composited with NeuronStructExplorer class, as it needs NeuronStructExplorer's capability to manage and process neuron tree and neuron segments.
*  The composition is achieved by using a shred_ptr pointing to the "base" NeuronStructExplorer's memory address. This design is due to the possibility of
*  NeuronStructExplorer serving multiple other classes at the same time.
*  TreeGrower is the main interface in NeuronStructNavigator library for "gorwing" trees out of [NeuronSWC]-based signals.
*
********************************************************************************/

#ifndef TREEGROWER_H
#define TREEGROWER_H

#include <memory>

#include <boost/config.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"

enum edge_lastvoted_t { edge_lastvoted };
namespace boost
{
	BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef boost::property<boost::edge_weight_t, double> weights;
typedef boost::property<edge_lastvoted_t, int, weights> lastVoted;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, lastVoted> undirectedGraph;

class TreeGrower
{
	friend class Tester;

public:
	/************** Constructors and Basic Data/Function Members ****************/	
	TreeGrower() { this->sharedExplorerPtr = nullptr; }
	TreeGrower(NeuronStructExplorer* baseExplorerPtr) { this->sharedExplorerPtr = make_shared<NeuronStructExplorer*>(baseExplorerPtr); }

	// IMPORTANT: Use a shared_ptr here to avoid dangling pointers. 
	shared_ptr<NeuronStructExplorer*> sharedExplorerPtr;
	
	vector<polarNeuronSWC> polarNodeList;
	
	boost::container::flat_map<double, boost::container::flat_set<int>> radiusShellMap_loc;  // radius -> polarNeuronSWC's location on [polarNodeList]
	boost::container::flat_map<double, NeuronTree> radius2shellTreeMap;                      // radius -> all NeuronSWCs on the shell specified by radius (in the form of NeuronTree)
	boost::container::flat_map<double, vector<connectedComponent>> radius2shellConnCompMap;  // radius -> all connected components on the shell specified by radius
	/****************************************************************************/



	/********************** Segment Forming / Elongation ************************/
	// Connects segments that meet the predefined criteria in which their segment ends are in the same cluster with given distance threshold [destThreshold].
	// NOTE: This is the framework under which segment connection/elongation takes place. The algorithmic requirements of connecting shall be integrated here.
	//       Since segments can only be connected 1 at a time, therefore, this operation is completed with a iterative process - TreeGrower::itered_connectSegsWithinClusters.
private:
	profiledTree connectSegsWithinClusters(const profiledTree& inputProfiledTree, float distThreshold = 5) const;

public:
	profiledTree itered_connectSegsWithinClusters(profiledTree& inputProfiledTree, float distThreshold = 5) const;
	/****************************************************************************/



	/******************************* Tree Forming *******************************/
	// ----------------- MST growing methods ----------------- //
	// Connects nodes/segments in an SWC file using MST algorithn provided by boost library.
	static NeuronTree SWC2MSTtree_boost(const NeuronTree& inputTree);

	// Connects nodes/segments in an SWC file using MST algorithn provided by boost library in each predefined tile. (The tile size is predefined.)
	static NeuronTree SWC2MSTtreeTiled_boost(NeuronTree const& inputTree, float tileLength = SEGtileXY_LENGTH, float zDivideNum = 1);	
	// ------------------------------------------------------- //

	// - Dendritic tree forming(polar coord radial approach) - //
	NeuronTree dendriticTree_shellCentroid(double distThre = 1);
	// ------------------------------------------------------- //

	// ------------- Piecing tiled tree together ------------- //
	static NeuronTree swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength = NODE_TILE_LENGTH);
	// ------------------------------------------------------- //
	/****************************************************************************/
};

#endif