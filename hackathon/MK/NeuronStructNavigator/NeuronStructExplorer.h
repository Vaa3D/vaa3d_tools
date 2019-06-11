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

	map<string, profiledTree> treeDataBase; // This is where all trees are stored and managed.
	
	// Initialize a profiledTree with input NeuronTree and store it into [treeDataBase] with a specified name.
	void treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength = SEGtileXY_LENGTH); 
	
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
	/* Segment end clustering method is not automatically called during integratedDataTypes::profiledTree::profiledTree initialization. */
	/* If the following method is called, profiledTree::segHeadClusters, profiledTree::segTailClusters, profiledTree::headSeg2ClusterMap, and profiledTree::tailSeg2ClusterMap will be populated.*/	
	// This method is a wrapper that calls this->getTilBasedSegClusters and this->mergeTileBasedSegClusters to obtain all segment ends' clustering profile. 
	void getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold = 5);

protected:
	// This method clusters segment terminals within each segment head/tail tile. 
	// NOTE, currently only simple unilateral segments are supported.
	void getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

	// This method merges segment-end clusters with given distance threshold for the whole input profiledTree.
	// Note, this method is usually called after this->getTileBasedSegClusters in this->getSegHeadTailClusters.
	void mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

public:
	// Returns a map where the key is the cluster label and the value is a vector carrying all possible pairs of segments in that cluster.
	static void getClusterSegPairs(profiledTree& inputProfiledTree);
	//--------------------------------------------------------------- //
	/* ------------------------------------------------------------------------- */
	/*****************************************************************************************/



	/************************* Inter/Intra-SWC Comparison/Analysis ***************************/
	// Computes the distance from every node to its nearest node in the given node lise.
	// The output is a map of 4 measures: mean, std, var, median.
	static map<string, float> selfNodeDist(const QList<NeuronSWC>& inputNodeList);

	// Identifieis identical signal nodes with a reference NeuronTree. 
	// The nodes in [subjectTree] within [distThre] range from those in [refTree] are considered the same signal nodes.
	static NeuronTree swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength = NODE_TILE_LENGTH);
	/*****************************************************************************************/



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
	profiledTree segElongate(const profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);
	profiledTree itered_segElongate(profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);

	profiledTree segElongate_cluster(const profiledTree& inputProfiledTree);
	profiledTree itered_segElongate_cluster(profiledTree& inputProfiledTree, float distThreshold);

	// Like NeuronStructUtil::segUnitConnect_executer, this method currently only supports simple unilateral segments.
	map<int, segUnit> segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold);

	map<int, segUnit> segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length = false);
	inline void tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap);

	profiledTree treeUnion_MSTbased(const profiledTree& expandingPart, const profiledTree& baseTree);

	profiledTree somaAmputatedTree(const profiledTree& inputProfiledTree, const int xRange, const int yRange, const int zRange);

	static profiledTree treeHollow(const profiledTree& inputProfiledTree, const float hollowCenterX, const float hollowCenterY, const float hollowCenterZ, const float radius);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
};

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

#endif
