//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This library intends to provide functionalities for neuron struct analysis, including graph and geometry analysis, etc.
*  Typically NeuronStructExplorer class methods need a profiledTree struct as part of the input arguments. A profiledTree can be assigned when the class is initiated or later.
*  
*  profiledTree is the core data type in NeuronStructExplorer class. It profiles the NeuronTree and carries crucial information of it.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

*  The class can be initiated with or without a profiledTree being initiated at the same time. A profiledTree can be stored and indexed in NeuronStructExplorer's treeDatabe.
*
********************************************************************************/

#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include <vector>
#include <map>
#include <string>

#include <boost/config.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/algorithm/string.hpp>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "v_neuronswc.h"
#include "NeuronStructUtilities.h"
#include "NeuronGeoGrapher.h"

class NeuronStructExplorer
{
public:
	/***************** Constructors and Basic Profiling Data/Function Members *****************/
	NeuronStructExplorer() {};
	NeuronStructExplorer(QString neuronFileName);
	NeuronStructExplorer(const NeuronTree& inputTree) { this->treeEntry(inputTree, "originalTree"); }

	NeuronTree* singleTree_onHoldPtr;
	NeuronTree singleTree_onHold;
	NeuronTree processedTree;

	map<string, profiledTree> treeDataBase;
	void treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength = SEGtileXY_LENGTH);
	static void profiledTreeReInit(profiledTree& inputProfiledTree); // Needs to incorporate with this->getSegHeadTailClusters later.
	
	V_NeuronSWC_list segmentList;
	void segmentDecompose(NeuronTree* inputTreePtr);
	static map<int, segUnit> findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap);
	
	static map<string, vector<int>> segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head = true);
	
	// ------------------- segment end clustering ------------------- //	
	/* Segment end clustering method is not automatically called during integratedDataTypes::profiledTree::profiledTree initialization. */
	/* If the following method is called, profiledTree::segHeadClusters, profiledTree::segTailClusters, profiledTree::headSeg2ClusterMap, and profiledTree::tailSeg2ClusterMap will be populated.*/
	
	// This method is a wrapper that calls this->getTilBasedSegClusters and this->mergeTileBasedSegClusters to obtain all segment ends' clustering profile. 
	void getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold = 5);

protected:
	// This method clusters segment terminals within each segment head/tail tile. 
	// NOTE, currently only simple unilateral segments are supported.
	void getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

	// This method merge segment end clusters with given distance threshold for the whole input profiledTree.
	// Note, this method is usually called after this->getTileBasedSegClusters together in this->getSegHeadTailClusters.
	void mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

public:
	static void getClusterSegPairs(profiledTree& inputProfiledTree);
	//--------------------------------------------------------------- //
	/*****************************************************************************************/



	/************************** Neuron Struct Processing Functions ***************************/
public:
	static void treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength = 5);
	
	// This method calls this->rc_segDownSample, which is a private function. Therefore, it cannot be static.
	profiledTree treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval = 2);
	
protected:
	void rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchingNodeID, int interval);
	
public:
	static inline NeuronTree singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove = 0); // Reorg
	static inline NeuronTree longConnCut(const profiledTree& inputProfiledTree, double distThre = 50); // Reorg
	static inline NeuronTree segTerminalize(const profiledTree& inputProfiledTree); // Reorg
	/*****************************************************************************************/



	/************************* Inter/Intra-SWC Comparison/Analysis ***************************/
	// Computes the distance from every node to its nearest node in the given node lise.
	// The output is a map of 4 measures: mean, std, var, median.
	static map<string, float> selfNodeDist(const QList<NeuronSWC>& inputNodeList);

	// Identifieis identical signal nodes with a reference NeuronTree. 
	// The nodes in [subjectTree] within [distThre] range from those in [refTree] are considered the same signal nodes.
	static NeuronTree swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength = NODE_TILE_LENGTH);

	static NeuronTree swcSamePartExclusion(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThreshold, float nodeTileLength = NODE_TILE_LENGTH);
	/*****************************************************************************************/



	/********************* Auto-tracing Related Neuron Struct Functions **********************/	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ currently DEPRECATED ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
	profiledTree segElongate(const profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);
	profiledTree itered_segElongate(profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);

	profiledTree segElongate_cluster(const profiledTree& inputProfiledTree);
	profiledTree itered_segElongate_cluster(profiledTree& inputProfiledTree, float distThreshold);

	// Like this->segUnitConnect_executer, this method currently only supports simple unilateral segments.
	map<int, segUnit> segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	static inline connectOrientation getConnOrientation(connectOrientation orit1, connectOrientation orrit2);
	
	// Generate a new segment that is connected with 2 input segments. Connecting orientation needs to be specified by connOrt.
	// This method is a generalized method and is normally the final step of segment connecting process.
	// NOTE, currently it only supports simple unilateral segment. Forked segment connection will result in throwing error message!!
	segUnit segUnitConnect_executer(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt); // PUT IN UTILITY CLASS?
	
	map<int, segUnit> segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length = false);
	inline void tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap);
	
	profiledTree treeUnion_MSTbased(const profiledTree& expandingPart, const profiledTree& baseTree);

	profiledTree somaAmputatedTree(const profiledTree& inputProfiledTree, const int xRange, const int yRange, const int zRange);

	static profiledTree treeHollow(const profiledTree& inputProfiledTree, const float hollowCenterX, const float hollowCenterY, const float hollowCenterZ, const float radius);	
	/*****************************************************************************************/



	/***************** Neuron Struct Refining Method *****************/
	static profiledTree spikeRemove(const profiledTree& inputProfiledTree, int spikeNodeNum = 3); // Reorg
	/*****************************************************************/



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
};

inline connectOrientation NeuronStructExplorer::getConnOrientation(connectOrientation orit1, connectOrientation orit2)
{
	if (orit1 == head && orit2 == head) return head_head;
	else if (orit1 == head && orit2 == tail) return head_tail;
	else if (orit1 == tail && orit2 == head) return tail_head;
	else if (orit1 == tail && orit2 == tail) return tail_tail;
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

inline NeuronTree NeuronStructExplorer::segTerminalize(const profiledTree& inputProfiledTree)
{
	NeuronTree outputTree;
	for (map<int, segUnit>::const_iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		outputTree.listNeuron.push_back(inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(segIt->second.head)));
		for (vector<int>::const_iterator tailIt = segIt->second.tails.begin(); tailIt != segIt->second.tails.end(); ++tailIt)
		{
			if (*tailIt == segIt->second.head) continue;
			outputTree.listNeuron.push_back(inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(*tailIt)));
			outputTree.listNeuron.back().parent = -1;
		}
	}

	return outputTree;
}

inline NeuronTree NeuronStructExplorer::singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove)
{
	NeuronTree outputTree;
	for (map<int, segUnit>::const_iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		//cout << "seg ID:" << segIt->first << " ";
		if (segIt->second.nodes.size() <= shortSegRemove)
		{
			//cout << "not included" << endl;
			continue;
		}
		else
		{
			//cout << "included" << endl;
			for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}
	}

	return outputTree;
}

inline NeuronTree NeuronStructExplorer::longConnCut(const profiledTree& inputProfiledTree, double distThre)
{
	NeuronTree outputTree;
	for (map<int, segUnit>::const_iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			if (nodeIt->parent == -1) outputTree.listNeuron.push_back(*nodeIt);
			else
			{
				NeuronSWC paNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(nodeIt->parent));
				double dist = sqrt((paNode.x - nodeIt->x) * (paNode.x - nodeIt->x) + (paNode.y - nodeIt->y) * (paNode.y - nodeIt->y) + (paNode.z - nodeIt->z) * (paNode.z - nodeIt->z) * zRATIO * zRATIO);
				if (dist > distThre)
				{
					outputTree.listNeuron.push_back(*nodeIt);
					(outputTree.listNeuron.end() - 1)->parent = -1;
				}
				else outputTree.listNeuron.push_back(*nodeIt);
			}
		}
	}

	return outputTree;
}

#endif
