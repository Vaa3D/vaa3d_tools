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
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"
#include "v_neuronswc.h"
#include "NeuronStructUtilities.h"

using namespace std;
using namespace integratedDataTypes;

enum edge_lastvoted_t { edge_lastvoted };
namespace boost 
{
	BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef boost::property<boost::edge_weight_t, double> weights;
typedef boost::property<edge_lastvoted_t, int, weights> lastVoted;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, lastVoted> undirectedGraph;

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
	void profiledTreeReInit(profiledTree& inputProfiledTree); // Needs to incorporate with this->getSegHeadTailClusters later.
	
	V_NeuronSWC_list segmentList;
	void segmentDecompose(NeuronTree* inputTreePtr);
	static map<int, segUnit> findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap);
	
	static map<string, vector<int>> segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head = true);
	
	// This method is a wrapper that calls this->getTilBasedSegClusters and this->mergeTileBasedSegClusters to obtain all segment ends' clustering profile. 
	void getSegHeadTailClusters(profiledTree& inputProfiledTree, float distThreshold = 5);

//private: --> will set to be private later after more tests
	// This method clusters segment terminals within each segment head/tail tile. 
	// NOTE, currently only simple unilateral segments are supported.
	void getTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);

	// This method merge segment end clusters with given distance threshold for the whole input profiledTree.
	// Note, this method is usually called after this->getTileBasedSegClusters together in this->getSegHeadTailClusters.
	void mergeTileBasedSegClusters(profiledTree& inputProfiledTree, float distThreshold);
	/*****************************************************************************************/


	/***************** Neuron Struct Processing Functions *****************/
public:
	static void treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength = 5);
	
	// This method calls this->rc_segDownSample, which is a private function. Therefore, it cannot be static.
	profiledTree treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval = 2);
	
private:
	void rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchingNodeID, int interval);
	
public:
	static inline NeuronTree singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove = 0);
	static inline NeuronTree longConnCut(const profiledTree& inputProfiledTree, double distThre = 50);
	static inline NeuronTree segTerminalize(const profiledTree& inputProfiledTree);
	/**********************************************************************/


	/***************** Auto-tracing Related Neuron Struct Functions *****************/
	// ----------------- MST ----------------- //
	NeuronTree SWC2MSTtree(NeuronTree const& inputTree);
	NeuronTree SWC2MSTtree_tiled(NeuronTree const& inputTree, float tileLength = SEGtileXY_LENGTH, float zDivideNum = 1);
	static NeuronTree MSTbranchBreak(const profiledTree& inputProfiledTree, double spikeThre = 10, bool spikeRemove = true);
	static inline NeuronTree MSTtreeCut(NeuronTree& inputTree, double distThre = 10);
	// -------------------------------------- //
	
	profiledTree segElongate(const profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);
	profiledTree itered_segElongate(profiledTree& inputProfiledTree, double angleThre = radANGLE_THRE);

	boost::container::flat_map<int, vector<segPairProfile>> getSegConnPairs_cluster(const profiledTree& inputProfiledTree);
	profiledTree connectLongNeurite(const profiledTree& inputProfiledTree, float distThreshold = 5);
	profiledTree itered_connectLongNeurite(profiledTree& inputProfiledTree, float distThreshold = 5);
	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ currently DEPRECATED ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
	profiledTree segElongate_cluster(const profiledTree& inputProfiledTree);
	profiledTree itered_segElongate_cluster(profiledTree& inputProfiledTree, float distThreshold);

	// Like this->segUnitConnect_executer, this method currently only supports simple unilateral segments.
	map<int, segUnit> segUnitConnPicker_dist(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, float distThreshold);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	static inline connectOrientation getConnOrientation(connectOrientation orit1, connectOrientation orrit2);
	
	// Generate a new segment that is connected with 2 input segments. Connecting orientation needs to be specified by connOrt.
	// This method is a generalized method and is normally the final step of segment connecting process.
	// NOTE, currently it only supports simple unilateral segment. Forked segment connection will result in throwing error message!!
	segUnit segUnitConnect_executer(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt);
	
	map<int, segUnit> segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length = false);
	inline void tileSegConnOrganizer_angle(const map<string, double>& segAngleMap, set<int>& connectedSegs, map<int, int>& elongConnMap);
	
	profiledTree treeUnion_MSTbased(const profiledTree& expandingPart, const profiledTree& baseTree);
	/********************************************************************************/


	/***************** Geometry *****************/
	inline static vector<float> getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode);
	inline static vector<float> getDispUnitVector(const vector<float>& headVector, const vector<float>& tailVector);
	inline static vector<pair<float, float>> getVectorLocPair(const NeuronSWC& startNode, const NeuronSWC& endNode);

	inline static double getVectorCosine(const vector<float>& vector1, const vector<float>& vector2);
	inline static double getVectorSine(const vector<float>& vector1, const vector<float>& vector2);
	inline static double getPiAngle(const vector<float>& vector1, const vector<float>& vector2);
	inline static double getRadAngle(const vector<float>& vector1, const vector<float>& vector2);

	static vector<pair<float, float>> getProjectedVector(const vector<pair<float, float>>& axialVector, const vector<pair<float, float>>& projectingVector);
	
	// This method computes the sum of turning angles of from one node to the next node for a segment.
	inline static double selfTurningRadAngleSum(const vector<vector<float>>& inputSegment);

	static segUnit segmentStraighten(const segUnit& inputSeg);

	double segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt);
	static double segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt);
	/********************************************/


	/***************** Neuron Struct Refining Method *****************/
	static profiledTree spikeRemove(const profiledTree& inputProfiledTree);
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

inline NeuronTree NeuronStructExplorer::MSTtreeCut(NeuronTree& inputTree, double distThre)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->parent != -1)
		{
			double x1 = it->x;
			double y1 = it->y;
			double z1 = it->z;
			size_t paID = it->parent;
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

inline connectOrientation NeuronStructExplorer::getConnOrientation(connectOrientation orit1, connectOrientation orit2)
{
	if (orit1 == head && orit2 == head) return head_head;
	else if (orit1 == head && orit2 == tail) return head_tail;
	else if (orit1 == tail && orit2 == head) return tail_head;
	else if (orit1 == tail && orit2 == tail) return tail_tail;
}

inline vector<float> NeuronStructExplorer::getVector_NeuronSWC(const NeuronSWC& startNode, const NeuronSWC& endNode)
{
	vector<float> vec(3);
	vec[0] = endNode.x - startNode.x;
	vec[1] = endNode.y - startNode.y;
	vec[2] = endNode.z - startNode.z;
	return vec;
}

inline vector<float> NeuronStructExplorer::getDispUnitVector(const vector<float>& headVector, const vector<float>& tailVector)
{
	float disp = sqrt((headVector.at(0) - tailVector.at(0)) * (headVector.at(0) - tailVector.at(0)) +
					  (headVector.at(1) - tailVector.at(1)) * (headVector.at(1) - tailVector.at(1)) +
					  (headVector.at(2) - tailVector.at(2)) * (headVector.at(2) - tailVector.at(2)));
	vector<float> dispUnitVector;
	dispUnitVector.push_back((headVector.at(0) - tailVector.at(0)) / disp);
	dispUnitVector.push_back((headVector.at(1) - tailVector.at(1)) / disp);
	dispUnitVector.push_back((headVector.at(2) - tailVector.at(2)) / disp);

	return dispUnitVector;
}

inline vector<pair<float, float>> NeuronStructExplorer::getVectorLocPair(const NeuronSWC& startNode, const NeuronSWC& endNode)
{
	vector<float> thisVector = NeuronStructExplorer::getVector_NeuronSWC(startNode, endNode);

	vector<pair<float, float>> outputVectorPairs(3);
	outputVectorPairs[0] = pair<float, float>(startNode.x, thisVector.at(0));
	outputVectorPairs[1] = pair<float, float>(startNode.y, thisVector.at(1));
	outputVectorPairs[2] = pair<float, float>(startNode.z, thisVector.at(2));

	return outputVectorPairs;
}

inline double NeuronStructExplorer::getVectorSine(const vector<float>& vector1, const vector<float>& vector2)
{
	double thisVectorCos = NeuronStructExplorer::getVectorCosine(vector1, vector2);
	double thisVectorSin = sqrt(1 - thisVectorCos * thisVectorCos);

	return thisVectorSin;
}

inline double NeuronStructExplorer::getVectorCosine(const vector<float>& vector1, const vector<float>& vector2)
{
	double dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	double sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	double sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	
	return dot / sqrt(sq1 * sq2);
}

inline double NeuronStructExplorer::getPiAngle(const vector<float>& vector1, const vector<float>& vector2)
{
	double dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	double sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	double sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	double angle = acos(dot / sqrt(sq1 * sq2));
    
	if (std::isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle / PI;
}

inline double NeuronStructExplorer::getRadAngle(const vector<float>& vector1, const vector<float>& vector2)
{
	double dot = (vector1.at(0) * vector2.at(0) + vector1.at(1) * vector2.at(1) + vector1.at(2) * vector2.at(2));
	double sq1 = (vector1.at(0) * vector1.at(0) + vector1.at(1) * vector1.at(1) + vector1.at(2) * vector1.at(2));
	double sq2 = (vector2.at(0) * vector2.at(0) + vector2.at(1) * vector2.at(1) + vector2.at(2) * vector2.at(2));
	double angle = acos(dot / sqrt(sq1 * sq2));
	
	if (std::isnan(acos(dot / sqrt(sq1 * sq2)))) return -10;
	else return angle;
}

inline double NeuronStructExplorer::selfTurningRadAngleSum(const vector<vector<float>>& inputSegment)
{
	double radAngleSum = 0;
	for (vector<vector<float>>::const_iterator it = inputSegment.begin() + 1; it != inputSegment.end() - 1; ++it)
	{
		vector<float> vector1(3), vector2(3);
		vector1[0] = it->at(0) - (it - 1)->at(0);
		vector1[1] = it->at(1) - (it - 1)->at(1);
		vector1[2] = it->at(2) - (it - 1)->at(2);
		vector2[0] = (it + 1)->at(0) - it->at(0);
		vector2[1] = (it + 1)->at(1) - it->at(1);
		vector2[2] = (it + 1)->at(2) - it->at(2);
		//cout << "(" << vector1[0] << ", " << vector1[1] << ", " << vector1[2] << ") (" << vector2[0] << ", " << vector2[1] << ", " << vector2[2] << ")" << endl;
		double radAngle = NeuronStructExplorer::getRadAngle(vector1, vector2);
		
		if (radAngle == -1) continue;
		else radAngleSum = radAngleSum + radAngle;
	}

	return radAngleSum;
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
