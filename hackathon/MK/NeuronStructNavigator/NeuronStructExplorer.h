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
public:
	/***************** Constructors and Basic Profiling Data/Function Members *****************/
	NeuronStructExplorer() = default;	
	NeuronStructExplorer(const NeuronTree& inputTree, const string treeName) { this->treeEntry(inputTree, treeName); }
	virtual ~NeuronStructExplorer() = default;

	map<string, profiledTree> treeDataBase; // This is where all trees are stored and managed.
	map<string, map<string, profiledTree>> treeSeriesDataBase;
	
	// Initialize a profiledTree with input NeuronTree and store it into [treeDataBase] with a specified name.
	inline void treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength = SEGtileXY_LENGTH); 
	inline void treeEntry(const NeuronTree& inputTree, string treeName, bool replace, float segTileLength = SEGtileXY_LENGTH);
	
	V_NeuronSWC_list segmentList;

	void segmentDecompose(NeuronTree* inputTreePtr); // This function is borrowed from Vaa3D code base.

	/* --------------------------- segment profiling --------------------------- */
	// Identifiy and profile all segments in a NeuronTree. This method is called when initializing a profiledTree with a NeuronTree.
	static map<int, segUnit> findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap);
	
	// Produce a tile-segment map with a given NeuronTree. The key is the tile label and the value is the IDs of segments that belong to the tile.
	// -- head == true:  generate segment-head tile map.
	// -- head == false: generate segment-tail tile map.
	// This method is called when a profiledTree is initialized.
	static map<string, vector<int>> segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head = true);
	
	// ------------------- segment-end clustering ------------------- //	
	// Segment end clustering method is not automatically called during integratedDataTypes::profiledTree::profiledTree initialization. 
	// If the following method is called, profiledTree::segHeadClusters, profiledTree::segTailClusters, profiledTree::headSeg2ClusterMap, and profiledTree::tailSeg2ClusterMap will be populated.
	// This method is a wrapper that calls this->getTilBasedSegClusters and this->mergeTileBasedSegClusters to obtain all segment ends' clustering profile. 
	void getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold = 5);

protected:
	// This method clusters segment terminals within each segment head/tail tile. 
	// NOTE, currently only simple unilateral segments are supported.
	// NOTE, this method is only the 1st step of constructing seg-end cluster profile. Those information stored in profiledTree::segHeadClusters and profiledTree::segTailClusters are not correct yet
	//       and need to go through this->mergeTileBasedSegClusters to reach its final form.  
	void getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

	// This method merges segment-end clusters with given distance threshold for the whole input profiledTree.
	// Note, this method is has to be called after this->getTileBasedSegClusters, 
	//		 where profiledTree::segHeadClusters and profiledTree::segTailClusters are stored with tiled-based seg-end clusters (not the final correct seg-end clusters) by this->getTileBasedSegClusters first.
	void mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

public:
	// Returns a map where the key is the cluster label and the value is a vector carrying all possible pairs of segments in that cluster.
	static void getClusterSegPairs(profiledTree& inputProfiledTree);
	//--------------------------------------------------------------- //
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



	/************************* Inter/Intra-SWC Comparison/Analysis ***************************/
	// Computes the distance from every node to its nearest node in the given node lise.
	// The output is a map of 4 measures: mean, std, var, median.
	static map<string, float> selfNodeDist(const QList<NeuronSWC>& inputNodeList);

	// Identifieis identical signal nodes with a reference NeuronTree. 
	// The nodes in [subjectTree] within [distThre] range from those in [refTree] are considered the same signal nodes.
	static NeuronTree swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength = NODE_TILE_LENGTH);
	/*****************************************************************************************/



	/*************************** Debug Functions ***************************/
	inline bool __segEndClusteringExam(const profiledTree& inputProfiledTree, string segEndTestFullPath);
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
	// Like NeuronStructUtil::segUnitConnect_executer, this method currently only supports simple unilateral segments.
	map<int, segUnit> segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold);

	map<int, segUnit> segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length = false);
	inline void tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap);

	static profiledTree treeHollow(const profiledTree& inputProfiledTree, const float hollowCenterX, const float hollowCenterY, const float hollowCenterZ, const float radius);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
};

inline void NeuronStructExplorer::treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength)
{
	if (this->treeDataBase.find(treeName) == this->treeDataBase.end())
	{
		profiledTree registeredTree(inputTree, segTileLength);
		this->treeDataBase.insert(pair<string, profiledTree>(treeName, registeredTree));
	}
	else
	{
		cerr << "This tree name has already existed. The tree will not be registered for further operations." << endl;
		return;
	}
}

inline void NeuronStructExplorer::treeEntry(const NeuronTree& inputTree, string treeName, bool replace, float segTileLength)
{
	if (replace)
	{
		profiledTree registeredTree(inputTree, segTileLength);
		this->treeDataBase.insert(pair<string, profiledTree>(treeName, registeredTree));
	}
	else this->treeEntry(inputTree, treeName, segTileLength);
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

/* =========================== Debug Functions =========================== */
inline bool NeuronStructExplorer::__segEndClusteringExam(const profiledTree& inputProfiledTree, string segEndTestFullPath)
{
	profiledTree inputCopy = inputProfiledTree;
	int clusterCount = 1;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputCopy.segTailClusters.begin(); it != inputCopy.segTailClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			inputCopy.tree.listNeuron[inputCopy.node2LocMap.at(*inputCopy.segs.at(*it2).tails.begin())].type = it->first % 9;
		}
		++clusterCount;
	}
	clusterCount = 1;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputCopy.segHeadClusters.begin(); it != inputCopy.segHeadClusters.end(); ++it)
	{
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			inputCopy.tree.listNeuron[inputCopy.node2LocMap.at(inputCopy.segs.at(*it2).head)].type = it->first % 9;
		}
		++clusterCount;
	}

	profiledTree terminals(inputCopy.tree);
	NeuronTree terminalTree;
	for (QList<NeuronSWC>::iterator it = terminals.tree.listNeuron.begin(); it != terminals.tree.listNeuron.end(); ++it)
	{
		if (it->parent == -1 || terminals.node2childLocMap.find(it->n) == terminals.node2childLocMap.end())
		{
			NeuronSWC newNode = *it;
			newNode.parent = -1;
			terminalTree.listNeuron.push_back(newNode);
		}
	}

	QString saveNameQ = QString::fromStdString(segEndTestFullPath);
	writeSWC_file(saveNameQ, terminalTree);

	return true;
}
/* ======================================================================= */

#endif
