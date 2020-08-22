//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  NeuronStructExplorer class intends to manage Neuron data struct by providing the following functionalities:
*  
*    a. [profiledTree] data struct management
*    b. Essential segment profiling methods, i.e., NeuronStructExplorer::findSegs, NeuronStructExplorer::segTileMap, and NeuronStructExplorer::getSegHeadTailClusters, etc.
*    c. Inter/intra neuron struct analysis.
*    
*  This is the base class of other derived class including TreeGrower, etc.  
*  Since segment profiling methods are critical and often are the foundation of higher level algorithms which is mainly included in derived classes,
*  implementing these methods in the base class grants the derived class direct access to them and makes the development cleaner and more convenient.
*
********************************************************************************/

#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include "v_neuronswc.h"
#include "NeuronStructUtilities.h"
#include "NeuronGeoGrapher.h"

class NeuronStructExplorer
{
	friend class Tester;

public:
	/***************** Constructors and Basic Profiling Data/Function Members *****************/
	NeuronStructExplorer() = default;	
	NeuronStructExplorer(const NeuronTree& inputTree, const string treeName) { this->treeEntry(inputTree, treeName); }

	map<string, profiledTree> treeDataBase; // This is where all trees are stored and managed.
	map<string, map<string, profiledTree>> treeDataBases;
	
	// Initialize a profiledTree with input NeuronTree and store it into [treeDataBase] with a specified name.
	void treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength = SEGtileXY_LENGTH); 
	void treeEntry(const NeuronTree& inputTree, string treeName, bool replace, float segTileLength = SEGtileXY_LENGTH);

	/* ------------------------- neuron node profiling ------------------------- */
	// Return the root node of the input [NeuronTree]. If there are multiple root nodes, return the root node with the largest structure.
	static NeuronSWC findRootNode(const NeuronTree& inputTree);

	// For an input swc, profile all nodes with their locations, and the locations of their children in the container.
	static void node2loc_node2childLocMap(const QList<NeuronSWC>& inputNodeList, map<int, size_t>& nodeLocMap, map<int, vector<size_t>>& node2childLocMap);

	// Returns the corresponding string key with the given node or marker.
	static inline string getNodeTileKey(const NeuronSWC& inputNode, float nodeTileLength = NODE_TILE_LENGTH);
	static inline string getNodeTileKey_noZratio(const NeuronSWC& inputNode, float nodeTileLength = NODE_TILE_LENGTH);
	static inline string getNodeTileKey(const float nodeCoords[], float nodeTileLength = NODE_TILE_LENGTH);
	static inline string getNodeTileKey(const ImageMarker& inputMarker, float nodeTileLength = NODE_TILE_LENGTH);

	// Node - tile profiling functions
	static inline void nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<int>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<int>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, QList<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const QList<NeuronSWC>& inputNodeList, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	/* -------------------- END of [neuron node profiling] --------------------- */

	/* --------------------------- segment profiling --------------------------- */
	V_NeuronSWC_list segmentList;
	void segmentDecompose(NeuronTree* inputTreePtr); // This function is borrowed from Vaa3D code base.

	// Identifiy and profile all segments in a NeuronTree. This method is called when initializing a profiledTree with a NeuronTree.
	static map<int, segUnit> findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap);
	
	// Produce a tile-segment map with a given NeuronTree. The key is the tile label and the value is the IDs of segments that belong to the tile.
	// -- head == true:  generate segment-head tile map.
	// -- head == false: generate segment-tail tile map.
	// This method is called when a profiledTree is initialized.
	static map<string, vector<int>> segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head = true);

	static void segMorphProfile(profiledTree& inputProfiledTree, int range = 3);
	
	// ------------------- segment-end clustering ------------------- //	
	// Segment end clustering method is not automatically called during integratedDataTypes::profiledTree::profiledTree initialization. 
	// If the following method is called, profiledTree::segHeadClusters, profiledTree::segTailClusters, profiledTree::headSeg2ClusterMap, and profiledTree::tailSeg2ClusterMap will be populated.
	// This method is a wrapper that calls this->getTilBasedSegClusters and this->mergeTileBasedSegClusters to obtain all segment ends' clustering profile. 
	void getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold = 5) const;

private:
	// This method clusters segment terminals within each segment head/tail tile. 
	// NOTE, currently only simple unilateral segments are supported.
	// NOTE, this method is only the 1st step of constructing seg-end cluster profile. Those information stored in profiledTree::segHeadClusters and profiledTree::segTailClusters are not correct yet
	//       and need to go through this->mergeTileBasedSegClusters to reach its final form.  
	void getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold) const;

	// This method merges segment-end clusters with given distance threshold for the whole input profiledTree.
	// Note, this method is has to be called after this->getTileBasedSegClusters, 
	//		 where profiledTree::segHeadClusters and profiledTree::segTailClusters are stored with tiled-based seg-end clusters (not the final correct seg-end clusters) by this->getTileBasedSegClusters first.
	void mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold) const;

public:
	// Returns a map where the key is the cluster label and the value is a vector carrying all possible pairs of segments in that cluster.
	void getClusterSegPairs(profiledTree& inputProfiledTree) const;
	// --------------- END of [segment-end clustering] --------------- //
	/* ----------------------- END of [segment profiling] ----------------------- */

	/* ----------------------- Seg-End Cluster Topology ------------------------ */	
	set<int> segEndClusterProbe(profiledTree& inputProfiledTree, const set<vector<float>>& inputProbes, const float rangeAllowance) const;
	/* ------------------------------------------------------------------------- */
	/*****************************************************************************************/


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


	/************************** Morphological Features **************************/
	static pair<NeuronSWC, NeuronSWC> getMaxEucliDistNode(const NeuronTree& inputTree);
	static int getBranchNum(const NeuronTree& inputTree, bool onlyBifur = false);
	static boost::container::flat_map<int, int> getOuterNodeBifurMap(const NeuronTree& inputTree, float outerFraction, bool onlyBifur = false);
	/****************************************************************************/

	/************************* Inter/Intra-SWC Comparison/Analysis ***************************/
	// Computes the distance from every node to its nearest node in the given node lise.
	// The output is a map of 4 measures: mean, std, var, median.
	static map<string, float> selfNodeDist(const QList<NeuronSWC>& inputNodeList);

	// Identifieis identical signal nodes with a reference NeuronTree. 
	// The nodes in [subjectTree] within [distThre] range from those in [refTree] are considered the same signal nodes.
	static NeuronTree swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength = NODE_TILE_LENGTH);
	/*****************************************************************************************/


	/*************************** Debug Functions ***************************/
	bool __segEndClusteringExam(const profiledTree& inputProfiledTree, string segEndTestFullPath) const;

	static void __segMorphProfiled_lengthDistRatio(profiledTree& inputProfiledTree, int range, double lengthDistRatio);
	/***********************************************************************/


	/* ~~~~~~~~~~~ Distance-based SWC analysis ~~~~~~~~~~~ */
	vector<vector<float>> FPsList;
	vector<vector<float>> FNsList;
	void falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2);

	map<int, long int> nodeDistCDF;
	map<int, long int> nodeDistPDF;
	void shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum = 500);
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ currently DEPRECATED ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
	// Like NeuronStructUtil::segUnitConnect_end2end, this method currently only supports simple unilateral segments.
	map<int, segUnit> segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold);

	map<int, segUnit> segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length = false);
	inline void tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap);

	static profiledTree treeHollow(const profiledTree& inputProfiledTree, const float hollowCenterX, const float hollowCenterY, const float hollowCenterZ, const float radius);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
};

inline string NeuronStructExplorer::getNodeTileKey(const NeuronSWC& inputNode, float nodeTileLength)
{
	string xLabel = to_string(int(inputNode.x / nodeTileLength));
	string yLabel = to_string(int(inputNode.y / nodeTileLength));
	string zLabel = to_string(int(inputNode.z / nodeTileLength / zRATIO));
	string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
	return keyLabel;
}

inline string NeuronStructExplorer::getNodeTileKey_noZratio(const NeuronSWC& inputNode, float nodeTileLength)
{
	string xLabel = to_string(int(inputNode.x / nodeTileLength));
	string yLabel = to_string(int(inputNode.y / nodeTileLength));
	string zLabel = to_string(int(inputNode.z / nodeTileLength));
	string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
	return keyLabel;
}

inline string NeuronStructExplorer::getNodeTileKey(const float inputNodeCoords[], float nodeTileLength)
{
	string xLabel = to_string(int((inputNodeCoords[0]) / nodeTileLength));
	string yLabel = to_string(int((inputNodeCoords[1]) / nodeTileLength));
	string zLabel = to_string(int((inputNodeCoords[2]) / nodeTileLength / zRATIO));
	string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
	return keyLabel;
}

inline string NeuronStructExplorer::getNodeTileKey(const ImageMarker& inputMarker, float nodeTileLength)
{
	string xLabel = to_string(int((inputMarker.x - 1) / nodeTileLength));
	string yLabel = to_string(int((inputMarker.y - 1) / nodeTileLength));
	string zLabel = to_string(int((inputMarker.z - 1) / (nodeTileLength / zRATIO)));
	string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
	return keyLabel;
}

inline void NeuronStructExplorer::nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<int>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(it->n);
		else
		{
			vector<int> newSet;
			newSet.push_back(it->n);
			nodeTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(*it);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*it);
			nodeTileMap.insert(pair<string, vector<NeuronSWC>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<int>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(it->n);
		else
		{
			vector<int> newSet;
			newSet.push_back(it->n);
			nodeTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, QList<NeuronSWC>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(*it);
		else
		{
			QList<NeuronSWC> newSet;
			newSet.push_back(*it);
			nodeTileMap.insert(pair<string, QList<NeuronSWC>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(*it);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*it);
			nodeTileMap.insert(pair<string, vector<NeuronSWC>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::nodeTileMapGen(const QList<NeuronSWC>& inputNodeList, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
{
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
	{
		string xLabel = to_string(int(it->x / nodeTileLength));
		string yLabel = to_string(int(it->y / nodeTileLength));
		string zLabel = to_string(int(it->z / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (nodeTileMap.find(keyLabel) != nodeTileMap.end()) nodeTileMap[keyLabel].push_back(*it);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*it);
			nodeTileMap.insert(pair<string, vector<NeuronSWC>>(keyLabel, newSet));
		}
	}
}

inline void NeuronStructExplorer::tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap)
{
	map<double, string> r_segAngleMap;
	for (map<string, double>::const_iterator reIt = segAngleMap.begin(); reIt != segAngleMap.end(); ++reIt)
		r_segAngleMap.insert(pair<double, string>(reIt->second, reIt->first));

	for (map<double, string>::iterator angleIt = r_segAngleMap.begin(); angleIt != r_segAngleMap.end(); ++angleIt)
	{
		vector<string> key1key2;
		boost::split(key1key2, angleIt->second, boost::is_any_of("_"));

		int seg1 = stoi(key1key2.at(0));
		int seg2 = stoi(key1key2.at(1));
		if (connectedSegs.find(seg1) == connectedSegs.end() && connectedSegs.find(seg2) == connectedSegs.end())
		{
			connectedSegs.insert(seg1);
			connectedSegs.insert(seg2);
			elongConnMap.insert(pair<int, int>(seg1, seg2));
		}
	}
}

inline void NeuronStructExplorer::upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& upstreamEnd, const NeuronSWC& downstreamEnd, const map<int, size_t>& node2locMap)
{
	tracedList.push_front(downstreamEnd);
	while (tracedList.front().parent != upstreamEnd.n) tracedList.push_front(inputList.at(int(node2locMap.at(tracedList.front().parent))));
	tracedList.push_front(upstreamEnd);
}

inline void NeuronStructExplorer::upstreamPath(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum)
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

inline void NeuronStructExplorer::upstreamPath(const QList<NeuronSWC>& inputList, vector<NeuronSWC>& tracedList, const NeuronSWC& startingNode, const map<int, size_t>& node2locMap, int nodeNum)
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
