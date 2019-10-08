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
*  This class is inherited from NeuronStructExplorer class, as it needs NeuronStructExplorer's capability to manage and process neuron tree and neuron segments.
*  TreeGrower is the main interface in NeuronStructNavigator library for "gorwing" trees out of [NeuronSWC]-based signals.
*
********************************************************************************/

#ifndef TREEGROWER_H
#define TREEGROWER_H

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

class TreeGrower: public NeuronStructExplorer
{
public:

	void treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength = SEGtileXY_LENGTH) override;

	/************** Constructors and Basic Data/Function Members ****************/	
	vector<polarNeuronSWC> polarNodeList;
	
	boost::container::flat_map<double, boost::container::flat_set<int>> radiusShellMap_loc;  // radius -> polarNeuronSWC's location on [polarNodeList]
	boost::container::flat_map<double, NeuronTree> radius2shellTreeMap;                      // radius -> all NeuronSWCs on the shell specified by radius (in the form of NeuronTree)
	boost::container::flat_map<double, vector<connectedComponent>> radius2shellConnCompMap;  // radius -> all connected components on the shell specified by radius
	/****************************************************************************/



	/********************** Segment Forming / Elongation ************************/
	// Connects segments that meet the predefined criteria in which their segment ends are in the same cluster with given distance threshold [destThreshold].
	// NOTE: This is the framework under which segment connection/elongation takes place. The algorithmic requirements of connecting shall be integrated here.
	//       Since segments can only be connected 1 at a time, therefore, this operation is completed with a iterative process - TreeGrower::itered_connectSegsWithinClusters.
	profiledTree connectSegsWithinClusters(const profiledTree& inputProfiledTree, float distThreshold = 5);

	profiledTree itered_connectSegsWithinClusters(profiledTree& inputProfiledTree, float distThreshold = 5);
	/****************************************************************************/



	/******************************* Tree Forming *******************************/
	// ----------------- MST growing methods ----------------- //
	// Connects nodes/segments in an SWC file using MST algorithn provided by boost library.
	static NeuronTree SWC2MSTtree_boost(const NeuronTree& inputTree);

	// Connects nodes/segments in an SWC file using MST algorithn provided by boost library in each predefined tile. (The tile size is predefined.)
	static NeuronTree SWC2MSTtreeTiled_boost(NeuronTree const& inputTree, float tileLength = SEGtileXY_LENGTH, float zDivideNum = 1);	
	// ------------------------------------------------------- //

	// - Dendritic tree forming(polar coord radial approach) - //
	void dendriticTree_shellCentroid(double distThre = 1);
	void dendriticTree_shellCentroid(string treeName, double distThre = 1);
	// ------------------------------------------------------- //

	// ------------- Piecing tiled tree together ------------- //
	static NeuronTree swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength = NODE_TILE_LENGTH);
	// ------------------------------------------------------- //
	/****************************************************************************/



	/************************* Tree Trimming / Refining *************************/
	// Removes short spikes on segments. The skipe length criterion is predefined by users in node count measure.
	static profiledTree spikeRemoval(const profiledTree& inputProfiledTree, int spikeNodeNum = 3);
	
	static profiledTree itered_spikeRemoval(profiledTree& inputProfiledTree, int spikeNodeNum = 3);

	static profiledTree removeHookingHeadTail(const profiledTree& inputProiledTree, float radAngleThre);

	// Breaks all branches in [inputProfiledTree].
	// Note, if [spikeRemove] == true, any short branches less than [spikeThre] in length will be removed as undesired spikes in stead of being recognized as branches.
	static NeuronTree branchBreak(const profiledTree& inputProfiledTree, double spikeThre = 10, bool spikeRemove = true);

	// Breaks any node-node length that is greater than [distThre].
	static inline NeuronTree treeCut(NeuronTree& inputTree, double distThre = 10);
	/****************************************************************************/
};

inline NeuronTree TreeGrower::treeCut(NeuronTree& inputTree, double distThre)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->parent != -1)
		{
			double x1 = it->x;
			double y1 = it->y;
			double z1 = it->z;
			int paID = it->parent;
			double x2 = inputTree.listNeuron.at(paID - 1).x;
			double y2 = inputTree.listNeuron.at(paID - 1).y;
			double z2 = inputTree.listNeuron.at(paID - 1).z;
			double dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + zRATIO * zRATIO * (z1 - z2) * (z1 - z2));
			if (dist > distThre)
			{
				outputTree.listNeuron.push_back(*it);
				(outputTree.listNeuron.end() - 1)->parent = -1;
			}
			else outputTree.listNeuron.push_back(*it);
		}
	}

	return outputTree;
}

#endif