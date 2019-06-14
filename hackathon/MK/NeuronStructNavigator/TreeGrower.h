#ifndef TREEGROWER_H
#define TREEGROWER_H

#include <boost/config.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"
#include "ImgAnalyzer.h"

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
	/************** Constructors and Basic Data/Function Members ****************/	
	vector<polarNeuronSWC> polarNodeList;
	boost::container::flat_map<double, boost::container::flat_set<int>> radiusShellMap_loc;
	boost::container::flat_map<double, NeuronTree> radius2shellTreeMap;
	boost::container::flat_map<double, vector<connectedComponent>> radius2shellConnCompMap;
	/****************************************************************************/



	/********************** Polar Coord System Operations ***********************/
	static boost::container::flat_map<double, NeuronTree> radiusShellNeuronTreeMap(
		const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList);

	static boost::container::flat_map<double, vector<connectedComponent>> shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap);
	/****************************************************************************/



	/********************** Segment Forming / Elongation ************************/
	profiledTree connectSegsWithinClusters(const profiledTree& inputProfiledTree, float distThreshold = 5);
	profiledTree itered_connectSegsWithinClusters(profiledTree& inputProfiledTree, float distThreshold = 5);
	/****************************************************************************/



	/******************************* Tree Forming *******************************/
	// ----------------- MST growing methods ----------------- //
	static NeuronTree SWC2MSTtree_boost(const NeuronTree& inputTree);
	static NeuronTree SWC2MSTtreeTiled_boost(NeuronTree const& inputTree, float tileLength = SEGtileXY_LENGTH, float zDivideNum = 1);
	static NeuronTree MSTbranchBreak(const profiledTree& inputProfiledTree, double spikeThre = 10, bool spikeRemove = true);
	static inline NeuronTree MSTtreeCut(NeuronTree& inputTree, double distThre = 10);
	// ------------------------------------------------------- //

	// - Dendritic tree forming(polar coord radial approach) - //
	static boost::container::flat_map<double, NeuronTree> radius2NeuronTreeMap(const boost::container::flat_map<double, boost::container::flat_set<int>>& radiusShellMap_loc, const vector<polarNeuronSWC>& inputPolarNodeList);
	static boost::container::flat_map<double, vector<connectedComponent>> radius2connCompsShell(const boost::container::flat_map<double, NeuronTree>& inputRadius2TreeMap);
	void dendriticTree_shellCentroid(double distThre = 1);
	// ------------------------------------------------------- //

	// ------------- Piecing tiled tree together ------------- //
	static NeuronTree swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength = NODE_TILE_LENGTH);
	// ------------------------------------------------------- //
	/****************************************************************************/



	/*********************** Tree - Subtree Operations **************************/
	// Extract a subtree that is the upstream of a given starting node from the original tree.
	static inline void upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& upstreamEnd, const NeuronSWC& downstreamEnd, const map<int, size_t>& node2locMap);
	static inline void upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum = 10);
	static inline void upstreamPath(const QList<NeuronSWC>& inputList, vector<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum = 10);

	// Extract a subtree that is the downstream of a given starting node from the original tree.
	static void downstream_subTreeExtract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& subTreeList, const NeuronSWC& startingNode, map<int, size_t>& node2locMap, map<int, vector<size_t>>& node2childLocMap);

	// Extract a complete tree from a given swc with a given starting node. If all nodes are connected in the input swc, the extracted tree will be identical to the input itself.
	static void wholeSingleTree_extract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode);
	/****************************************************************************/



	/************************* Tree Trimming / Refining *************************/
	static profiledTree spikeRemove(const profiledTree& inputProfiledTree, int spikeNodeNum = 3);
	/****************************************************************************/
};

inline NeuronTree TreeGrower::MSTtreeCut(NeuronTree& inputTree, double distThre)
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

inline void TreeGrower::upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& upstreamEnd, const NeuronSWC& downstreamEnd, const map<int, size_t>& node2locMap)
{
	tracedList.push_front(downstreamEnd);
	while (tracedList.front().parent != upstreamEnd.n) tracedList.push_front(inputList.at(int(node2locMap.at(tracedList.front().parent))));
	tracedList.push_front(upstreamEnd);
}

inline void TreeGrower::upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum)
{
	tracedList.push_front(startingNode);
	int parentID = startingNode.parent;
	while (tracedList.size() < nodeNum)
	{
		if (node2locMap.find(parentID) == node2locMap.end()) break;
		tracedList.push_front(inputList.at(int(node2locMap.at(parentID))));
		parentID = tracedList.front().parent;
		if (parentID == -1) break;
	}
}

inline void TreeGrower::upstreamPath(const QList<NeuronSWC>& inputList, vector<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum)
{
	tracedList.push_back(startingNode);
	int parentID = startingNode.parent;
	while (tracedList.size() < nodeNum)
	{
		if (node2locMap.find(parentID) == node2locMap.end()) break;
		tracedList.push_back(inputList.at(int(node2locMap.at(parentID))));
		parentID = tracedList.back().parent;
		if (parentID == -1) break;
	}

	reverse(tracedList.begin(), tracedList.end());
}

#endif