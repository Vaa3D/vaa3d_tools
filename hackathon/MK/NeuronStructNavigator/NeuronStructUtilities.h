//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  Most of NeuronStructUtil class methods intend to operate on the whole neuron struct level. 
*  As 'utility' it is called, the functionalities provided in this class include:
*    a. [Basic neuron struct operations]                   -- cropping SWC, scaling SWC, swc registration, etc.
*    b. [Tree - subtree operations]                        -- extracting upstream or downstream of a given tree.
*    c. [Neuron struct profiling methods]                  -- node-tile mapping, node-location mapping, etc. 
*    d. [SWC - ImgAnalyzer::connectedComponent operations] -- Methods of this category convert SWC into vector<ImgAnalyzer::connectedComponent>
*  
*  Most of NeuronStructUtil class methods are implemented as static functions. The input NeuronTree is always set to be const so that it will not be modified.
*  A typical function call would need at least three input arguments:
*
*		NeuronStructUtil::func(const NeuronTree& inputTree, NeuronTree& outputTree, other input arguments);
*
********************************************************************************/

#ifndef NEURONSTRUCTUTILITIES_H
#define NEURONSTRUCTUTILITIES_H

#include <fstream>

#include <boost/algorithm/string.hpp>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "integratedDataTypes.h"
#include "ImgAnalyzer.h"

using namespace std;
using namespace integratedDataTypes;

class NeuronStructUtil
{
public: 
	/***************** Constructors and Basic Data/Function Members *****************/
	// Not needed at the moment. Will implement later if necessary.
	/********************************************************************************/


	/*************************** Segment Operations ***************************/
	static inline connectOrientation getConnOrientation(connectOrientation orit1, connectOrientation orrit2);

	// Generate a new segment that is connected with 2 input segments. Connecting orientation needs to be specified by connOrt.
	// This method is a generalized method and is normally the final step of segment connecting process.
	// NOTE, currently it only supports simple unilateral segment. Forked segment connection will result in throwing error message!!
	static segUnit segUnitConnect_executer(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt);
	/**************************************************************************/



	/************************ Neuron Struct Processing ************************/
	// ----------------- Basic Operations ----------------- //
	static void swcSlicer(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness = 1);

	template<class T>
	static inline void swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, T xlb, T xhb, T ylb, T yhb, T zlb, T zhb);

	template<class T>
	static inline NeuronTree swcScale(const NeuronTree& inputTree, T xScale, T yScale, T zScale); 

	template<class T>
	static inline NeuronTree swcShift(const NeuronTree& inputTree, T xShift, T yShift, T zShift);
	
	template<class T> // Get the coordinate boundaries of the inputTree. 6 elements stored in the retruned vector: xMin, xMax, yMin, yNax, zMin, zMax.
	static inline vector<T> getSWCboundary(const NeuronTree& inputTree);

	// Align inputTree with refTree.
	static NeuronTree swcRegister(NeuronTree& inputTree, const NeuronTree& refTree); 

	static NeuronTree swcCombine(const vector<NeuronTree>& inputTrees);
	
	static map<int, QList<NeuronSWC>> swcSplitByType(const NeuronTree& inputTree);

	// Subtract refTree from targetTree.
	static NeuronTree swcSubtraction(const NeuronTree& targetTree, const NeuronTree& refTree, int type = 0); 
	// ---------------------------------------------------- //
	
	// ------------- Higher level processing -------------- //
	static inline NeuronTree singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove = 0);
	static inline NeuronTree longConnCut(const profiledTree& inputProfiledTree, double distThre = 50);
	static inline NeuronTree segTerminalize(const profiledTree& inputProfiledTree);

	// When using SWC root nodes to represent signals, this method can be used to reduce node density.
	// -- NOTE, this method can only be used when all nodes are roots. 
	template<class T>
	static inline void swcDownSample_allRoots(const NeuronTree& inputTree, NeuronTree& outputTree, T factor, bool shrink);

	// This method creates interpolated nodes in between each pair of 2 adjacent nodes on the input tree. 
	static void treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength = 5);

	// -- This method "down samples" the input tree segment by segment. 
	// -- A recursive down sampling method [NeuronStructUtil::rc_segDownSample] is called in this function to deal with all possible braching points in each segment.
	// -- NOTE, this method is essentially used for straightening / smoothing segments when there are too many zigzagging.  
	static profiledTree treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval = 2);
	static void rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchingNodeID, int interval);
	// ---------------------------------------------------- //
	/**************************************************************************/
	


	/******************* Neuron Struct Profiling Methods **********************/
	// These profiling methods are put to make them available to NeuronTree struct. 
	// Users don't need to always initialize a integratedFataType::profiledTree to get all these node-tile node-loc maps.

	// For an input swc, profile all nodes with their locations, and the locations of their children in the container.
	static inline void node2loc_node2childLocMap(const QList<NeuronSWC>& inputNodeList, map<int, size_t>& nodeLocMap, map<int, vector<size_t>>& node2childLocMap);

	// Returns the corresponding string key with the given node or marker.
	static inline string getNodeTileKey(const NeuronSWC& inputNode);
	static inline string getNodeTileKey(const ImageMarker& inputMarker, float nodeTileLength = NODE_TILE_LENGTH);
	
	// Node - tile profiling functions
	static inline void nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<int>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<int>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, QList<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	static inline void nodeTileMapGen(const QList<NeuronSWC>& inputNodeList, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength = NODE_TILE_LENGTH);
	/**************************************************************************/



	/***************** SWC to ImgAnalyzer::connectedComponent *****************/
	// Identify the connectivity of every node in the input swc and generate connected component data from it.
	static vector<connectedComponent> swc2signal2DBlobs(const NeuronTree& inputTree);
	static vector<connectedComponent> swc2signal3DBlobs(const NeuronTree& inputTree);

	// Merge 2D connected components into 3D ones.
	static vector<connectedComponent> merge2DConnComponent(const vector<connectedComponent>& inputConnCompList);

	// Convert connected component data type into swc format.
	static NeuronTree blobs2tree(const vector<connectedComponent>& inputConnCompList, bool usingRadius2compNum = false);
	/**************************************************************************/



	/***************** Artificial SWC for Developing Purposes *****************/
	// Generates a cubical root node cluster with specified cube length.
	static inline NeuronTree randNodes(float cubeLength, float density);

	// Generates a spherical root node cluster with specified origin, radius, and node density.
	// The total amount of nodes = (4 / 3) * PI * radius^3 * density.
	static inline NeuronTree sphereRandNodes(float radius, float centerX, float centerY, float centerZ, float density);

	// Generates multiple root node clusters with specified origin, radius, and node density.
	// Each cluster center is separated by stepX, stepY, and stepZ. xRange, yRange, and zRange specify the space range.
	static NeuronTree nodeSpheresGen(float sphereRadius, float density, float stepX, float stepY, float stepZ, float xRange, float yRange, float zRange);
	/**************************************************************************
	
	
	
	/***************************** Miscellaneous ******************************/ 
	// Generates linker file for swc
	static inline void linkerFileGen_forSWC(string swcFullFileName);	
	/**************************************************************************/



	/* ~~~~~~~~~~~~~~~ Sampling Methods for Simulated Volumetric Patch Generation ~~~~~~~~~~~~~~~ */
	static void swcSlicer_DL(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness = 0);
	static void sigNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, float ratio, float distance);
	static void bkgNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, int dims[], float ratio, float distance);
	static void bkgNode_Gen_somaArea(const NeuronTree& inputTree, NeuronTree& outputTree, int xLength, int yLength, int zLength, float ratio, float distance);
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};

inline connectOrientation NeuronStructUtil::getConnOrientation(connectOrientation orit1, connectOrientation orit2)
{
	if (orit1 == head && orit2 == head) return head_head;
	else if (orit1 == head && orit2 == tail) return head_tail;
	else if (orit1 == tail && orit2 == head) return tail_head;
	else if (orit1 == tail && orit2 == tail) return tail_tail;
}

template<class T>
inline vector<T> NeuronStructUtil::getSWCboundary(const NeuronTree& inputTree)
{
	T xMin = 10000, xMax = 0, yMin = 10000, yMax = 0, zMin = 10000, zMax = 0;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x > xMax) xMax = it->x;
		else if (it->x < xMin) xMin = it->x;

		if (it->y > yMax) yMax = it->y;
		else if (it->y < yMin) yMin = it->y;

		if (it->z > zMax) zMax = it->z;
		else if (it->z < zMin) zMin = it->z;
	}

	vector<T> boundaries(6);
	boundaries[0] = xMin;
	boundaries[1] = xMax;
	boundaries[2] = yMin;
	boundaries[3] = yMax;
	boundaries[4] = zMin;
	boundaries[5] = zMax;

	return boundaries;
}

template<class T>
inline NeuronTree NeuronStructUtil::swcScale(const NeuronTree& inputTree, T xScale, T yScale, T zScale)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x * xScale;
		newNode.y = it->y * yScale;
		newNode.z = it->z * zScale;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

template<class T>
inline NeuronTree NeuronStructUtil::swcShift(const NeuronTree& inputTree, T xShift, T yShift, T zShift)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		NeuronSWC newNode;
		newNode = *it;
		newNode.x = it->x + xShift;
		newNode.y = it->y + yShift;
		newNode.z = it->z + zShift;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

template<class T>
inline void NeuronStructUtil::swcCrop(const NeuronTree& inputTree, NeuronTree& outputTree, T xlb, T xhb, T ylb, T yhb, T zlb, T zhb)
{
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x < xlb || it->x > xhb || it->y < ylb || it->y > yhb || it->z < zlb || it->z > zhb) continue;
		else outputTree.listNeuron.push_back(*it);
	}
}

template<class T>
inline void NeuronStructUtil::swcDownSample_allRoots(const NeuronTree& inputTree, NeuronTree& outputTree, T factor, bool shrink)
{
	QList<NeuronSWC> inputList = inputTree.listNeuron;
	outputTree.listNeuron.clear();
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		if (T(it->z) % factor == 0 && T(it->x) % factor == 0 && T(it->y) % factor == 0) 
			outputTree.listNeuron.push_back(*it);
	}

	if (shrink)
	{
		for (QList<NeuronSWC>::iterator it = outputTree.listNeuron.begin(); it != outputTree.listNeuron.end(); ++it)
		{
			it->x = it->x / 2;
			it->y = it->y / 2;
			it->z = it->z / 2;
		}
	}
}

inline NeuronTree NeuronStructUtil::singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove)
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

inline NeuronTree NeuronStructUtil::longConnCut(const profiledTree& inputProfiledTree, double distThre)
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

inline NeuronTree NeuronStructUtil::segTerminalize(const profiledTree& inputProfiledTree)
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

inline string NeuronStructUtil::getNodeTileKey(const ImageMarker& inputMarker, float nodeTileLength)
{
	string xLabel = to_string(int((inputMarker.x - 1) / nodeTileLength));
	string yLabel = to_string(int((inputMarker.y - 1) / nodeTileLength));
	string zLabel = to_string(int((inputMarker.z - 1) / (nodeTileLength / zRATIO)));
	string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
	return keyLabel;
}


inline void NeuronStructUtil::nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<int>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::nodeTileMapGen(const NeuronTree& inputTree, map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<int>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, QList<NeuronSWC>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::nodeTileMapGen(const NeuronTree& inputTree, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::nodeTileMapGen(const QList<NeuronSWC>& inputNodeList, boost::container::flat_map<string, vector<NeuronSWC>>& nodeTileMap, float nodeTileLength)
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

inline void NeuronStructUtil::node2loc_node2childLocMap(const QList<NeuronSWC>& inputNodeList, map<int, size_t>& nodeLocMap, map<int, vector<size_t>>& node2childLocMap)
{
	// This method profiles node-location node-child_location of a given NeuronTree.
	// In current implementation, a single node will carry a node.n-vector<size_t> pair in node2childLocMap where its vector<size> is empty.
	// However, any tip node will not have an entry in node2childLocMap.

	nodeLocMap.clear();
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
		nodeLocMap.insert(pair<int, size_t>(it->n, (it - inputNodeList.begin())));
	//cout << " Node - Locations mapping done. size: " << nodeLocMap.size() << endl;

	node2childLocMap.clear();
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
	{
		int paID = it->parent;
		if (paID == -1)
		{
			vector<size_t> childSet;
			childSet.clear();
			node2childLocMap.insert(pair<int, vector<size_t>>(it->n, childSet));
		}
		else
		{
			if (node2childLocMap.find(paID) != node2childLocMap.end())
			{
				node2childLocMap[paID].push_back(size_t(it - inputNodeList.begin()));
				//cout << paID << " " << size_t(it - inputNodeList.begin()) << endl;
			}
			else
			{
				vector<size_t> childSet;
				childSet.clear();
				childSet.push_back(size_t(it - inputNodeList.begin()));
				node2childLocMap.insert(pair<int, vector<size_t>>(paID, childSet));
				//cout << paID << " " << size_t(it - inputNodeList.begin()) << endl;
			}
		}
	}
	//cout << " node - Child location mapping done. size: " << node2childLocMap.size() << endl;
}

inline NeuronTree NeuronStructUtil::randNodes(float cubeLength, float density)
{
	NeuronTree outputTree;
	int targetNodeCount = int(cubeLength * cubeLength * cubeLength * density);
	int producedNodeCount = 0;
	while (producedNodeCount <= targetNodeCount)
	{
		int randNumX = rand() % int(cubeLength) + 1;
		int randNumY = rand() % int(cubeLength) + 1;
		int randNumZ = rand() % int(cubeLength) + 1;

		NeuronSWC newNode;
		newNode.n = producedNodeCount + 1;
		newNode.x = randNumX;
		newNode.y = randNumY;
		newNode.z = randNumZ;
		newNode.type = 2;
		newNode.parent = -1;
		outputTree.listNeuron.push_back(newNode);

		++producedNodeCount;
	}

	return outputTree;
}

inline NeuronTree NeuronStructUtil::sphereRandNodes(float radius, float centerX, float centerY, float centerZ, float density)
{
	int targetNum = int((4 / 3) * PI * radius * radius * radius * density);
	int nodeCount = 1;
	NeuronTree outputTree;
	while (nodeCount <= targetNum)
	{
		float randomX = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerX - radius);
		float randomY = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerY - radius);
		float randomZ = float(rand()) / float(RAND_MAX) * (2 * radius) + (centerZ - radius);

		float dist = sqrtf((randomX - centerX) * (randomX - centerX) + (randomY - centerY) * (randomY - centerY) + (randomZ - centerZ) * (randomZ - centerZ));
		if (dist > radius) continue;

		NeuronSWC newNode;
		newNode.n = nodeCount;
		newNode.x = randomX;
		newNode.y = randomY;
		newNode.z = randomZ / zRATIO;
		newNode.type = 2;
		newNode.parent = -1;
		outputTree.listNeuron.push_back(newNode);

		++nodeCount;
	}
	
	return outputTree;
}

inline void NeuronStructUtil::linkerFileGen_forSWC(string swcFullFileName)
{
	vector<string> swcFullNameParse;
	boost::split(swcFullNameParse, swcFullFileName, boost::is_any_of("\\"));
	string swcName = swcFullNameParse.back();
	vector<string> swcBaseNameParse;
	boost::split(swcBaseNameParse, swcName, boost::is_any_of("."));
	string swcBaseName = swcBaseNameParse.front();

	string rootPath;
	for (vector<string>::iterator it = swcFullNameParse.begin(); it != swcFullNameParse.end() - 1; ++it)
		rootPath = rootPath + *it + "/";
	string anoFullName = rootPath + swcBaseName + ".ano";

	ofstream outputFile(anoFullName.c_str());
	outputFile << "SWCFILE=" << swcName << endl;
	outputFile.close();
}

#endif 