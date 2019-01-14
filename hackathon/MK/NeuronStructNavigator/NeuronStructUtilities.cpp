//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This library provides functionalities for neuron struct operations, such as crop, register, sample, data extraction, etc.
*  NeuronStructUtil class is desinged to take NeuronTree struct as the input and as well output NeuronTree struct for most of the methods.
*  This class intends to operate on the whole neuron struct level, as 'utility' it is called.
*
*  Many NeuronStructUtil class methods are implemented as static functions. The input NeuronTree is always set to be const so that it will not be modified.
*  A typical function call would need at least three input arguments:
*
*		NeuronStructUtil::func(const NeuronTree& inputTree, NeuronTree& outputTree, other input arguments);
*
********************************************************************************/

#include <iostream>
#include <string>
#include <iterator>
#include <set>
#include <cmath>

#include <boost/filesystem.hpp>

#include "basic_4dimage.h"

#include "NeuronStructUtilities.h"
#include "ImgProcessor.h"
#include "ImgAnalyzer.h"

using namespace boost;

NeuronTree NeuronStructUtil::swcRegister(NeuronTree& inputTree, const NeuronTree& refTree)
{
	double xShift, yShift, zShift;
	double xScale, yScale, zScale;

	double xmin = 10000, ymin = 10000, zmin = 10000;
	double xmax = 0, ymax = 0, zmax = 0;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x < xmin) xmin = it->x;
		if (it->x > xmax) xmax = it->x;
		if (it->y < ymin) ymin = it->y;
		if (it->y > ymax) ymax = it->y;
		if (it->z < zmin) zmin = it->z;
		if (it->z > zmax) zmax = it->z;
	}
	double refXmin = 10000, refYmin = 10000, refZmin = 10000;
	double refXmax = 0, refYmax = 0, refZmax = 0;
	for (QList<NeuronSWC>::const_iterator refIt = refTree.listNeuron.begin(); refIt != refTree.listNeuron.end(); ++refIt)
	{
		if (refIt->x < refXmin) refXmin = refIt->x;
		if (refIt->x > refXmax) refXmax = refIt->x;
		if (refIt->y < refYmin) refYmin = refIt->y;
		if (refIt->y > refYmax) refYmax = refIt->y;
		if (refIt->z < refZmin) refZmin = refIt->z;
		if (refIt->z > refZmax) refZmax = refIt->z;
	}

	xScale = (refXmax - refXmin) / (xmax - xmin);
	yScale = (refYmax - refYmin) / (ymax - ymin);
	zScale = (refZmax - refZmin) / (zmax - zmin);
	xShift = refXmin - xmin * xScale;
	yShift = refYmin - ymin * yScale;
	zShift = refZmin - zmin * zScale;

	NeuronTree outputTree;
	for (int i = 0; i < inputTree.listNeuron.size(); ++i)
	{
		NeuronSWC newNode = inputTree.listNeuron.at(i);
		newNode.x = newNode.x * xScale + xShift;
		newNode.y = newNode.y * yScale + yShift;
		newNode.z = newNode.z * zScale + zShift;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

void NeuronStructUtil::swcSlicer(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness)
{
	QList<NeuronSWC> inputList = inputTree.listNeuron;
	int zMax = 0;
	ptrdiff_t thicknessPtrDiff = ptrdiff_t(thickness); // Determining largest number of z in inputTree.
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		int z = round(it->z);
		if (z >= zMax) zMax = z;
	}

	int treeNum = zMax / thickness + 1;
	vector<ptrdiff_t> delLocs;
	for (int i = 0; i < treeNum; ++i)
	{
		NeuronTree outputTree;
		outputTrees.push_back(outputTree);
		for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
		{
			if (it->z <= thickness * (i + 1))
			{
				outputTrees.at(i).listNeuron.push_back(*it);
				delLocs.push_back(it - inputList.begin());
			}
		}

		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) inputList.erase(inputList.begin() + *delIt);
		delLocs.clear();
	}
}

void NeuronStructUtil::swcSlicer_DL(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness)
{
	// -- Dissemble SWC files into "slices." Each outputSWC file represents only 1 z slice.
	// thickness * 2 + 1 = the number of consecutive z slices for one SWC node to appear. This is for the purpose producing continous masks.

	QList<NeuronSWC> inputList = inputTree.listNeuron;
	int zMax = 0;
	ptrdiff_t thicknessPtrDiff = ptrdiff_t(thickness); // Determining how many z sections to be included in 1 single slice.
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		it->z = round(it->z);
		if (it->z >= zMax) zMax = it->z;
	}

	QList<NeuronTree> slicedTrees; // Determining number of sliced trees in the list.
	for (int i = 0; i < zMax; ++i)
	{
		NeuronTree nt;
		slicedTrees.push_back(nt);
	}

	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		NeuronSWC currNode = *it;
		ptrdiff_t sliceNo = ptrdiff_t(it->z);
		(slicedTrees.begin() + sliceNo - 1)->listNeuron.push_back(currNode); // SWC starts with 1.
		float currZ = currNode.z;

		// -- Project +/- thickness slices onto the same plane, making sure the tube can be connected accross planes. -- //
		vector<ptrdiff_t> sectionNums;
		for (ptrdiff_t ptri = 1; ptri <= thicknessPtrDiff; ++ptri)
		{
			ptrdiff_t minusDiff = sliceNo - ptri;
			ptrdiff_t plusDiff = sliceNo + ptri;

			if (minusDiff < 0) continue;
			else sectionNums.push_back(minusDiff);

			if (plusDiff > ptrdiff_t(zMax)) continue;
			else sectionNums.push_back(plusDiff);
		}
		for (vector<ptrdiff_t>::iterator ptrIt = sectionNums.begin(); ptrIt != sectionNums.end(); ++ptrIt)
		{
			//cout << "current node z:" << currNode.z << " " << *ptrIt << "| ";
			NeuronSWC newNode = currNode;
			newNode.z = float(*ptrIt);
			(slicedTrees.begin() + *ptrIt - 1)->listNeuron.push_back(newNode);
		}
		//cout << endl;

		sectionNums.clear();
		// ------------------------------------------------------------------------------------------------------------- //
	}

	for (QList<NeuronTree>::iterator it = slicedTrees.begin(); it != slicedTrees.end(); ++it)
		outputTrees.push_back(*it);
}

map<int, QList<NeuronSWC>> NeuronStructUtil::swcSplitByType(const NeuronTree& inputTree)
{
	map<int, QList<NeuronSWC>> outputNodeTypeMap;
	map<int, boost::container::flat_set<int>> nodeIDsetMap;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		outputNodeTypeMap[it->type].append(*it);
		nodeIDsetMap[it->type].insert(it->n);
	}
	for (map<int, QList<NeuronSWC>>::iterator mapIt = outputNodeTypeMap.begin(); mapIt != outputNodeTypeMap.end(); ++mapIt)
	{
		for (QList<NeuronSWC>::iterator nodeIt = mapIt->second.begin(); nodeIt != mapIt->second.end(); ++nodeIt)
		{
			if (nodeIt->parent == -1) continue;
			else if (nodeIDsetMap.at(nodeIt->type).find(nodeIt->parent) == nodeIDsetMap.at(nodeIt->type).end()) nodeIt->parent = -1;
		}
	}

	return outputNodeTypeMap;
}

NeuronTree NeuronStructUtil::swcSubtraction(const NeuronTree& targetTree, const NeuronTree& refTree, int type)
{
	boost::container::flat_map<string, QList<NeuronSWC>> targetNodeTileMap;
	boost::container::flat_map<string, QList<NeuronSWC>> refNodeTileMap;
	NeuronStructUtil::nodeTileMapGen(targetTree, targetNodeTileMap);
	NeuronStructUtil::nodeTileMapGen(refTree, refNodeTileMap);

	if (type == 0)
	{
		vector<ptrdiff_t> delLocs;
		for (boost::container::flat_map<string, QList<NeuronSWC>>::iterator targetTileIt = targetNodeTileMap.begin(); targetTileIt != targetNodeTileMap.end(); ++targetTileIt)
		{
			if (refNodeTileMap.find(targetTileIt->first) != refNodeTileMap.end())
			{
				for (QList<NeuronSWC>::iterator checkIt1 = targetTileIt->second.begin(); checkIt1 != targetTileIt->second.end(); ++checkIt1)
				{
					for (QList<NeuronSWC>::iterator checkIt2 = refNodeTileMap.at(targetTileIt->first).begin(); checkIt2 != refNodeTileMap.at(targetTileIt->first).end(); ++checkIt2)
					{
						if (checkIt1->x == checkIt2->x && checkIt1->y == checkIt2->y && checkIt1->z == checkIt2->z)
							delLocs.push_back(ptrdiff_t(checkIt1 - targetTileIt->second.begin()));
					}
				}
			}
			else continue;

			sort(delLocs.rbegin(), delLocs.rend());
			for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) targetTileIt->second.erase(targetTileIt->second.begin() + *delIt);
			delLocs.clear();
		}
	}
	else
	{
		vector<ptrdiff_t> delLocs;
		for (boost::container::flat_map<string, QList<NeuronSWC>>::iterator targetTileIt = targetNodeTileMap.begin(); targetTileIt != targetNodeTileMap.end(); ++targetTileIt)
		{
			if (refNodeTileMap.find(targetTileIt->first) != refNodeTileMap.end())
			{
				for (QList<NeuronSWC>::iterator checkIt1 = targetTileIt->second.begin(); checkIt1 != targetTileIt->second.end(); ++checkIt1)
				{
					for (QList<NeuronSWC>::iterator checkIt2 = refNodeTileMap.at(targetTileIt->first).begin(); checkIt2 != refNodeTileMap.at(targetTileIt->first).end(); ++checkIt2)
					{
						if (checkIt1->x == checkIt2->x && checkIt1->y == checkIt2->y && checkIt1->z == checkIt2->z && checkIt2->type == type)
							delLocs.push_back(ptrdiff_t(checkIt1 - targetTileIt->second.begin()));
					}
				}
			}
			else continue;

			sort(delLocs.rbegin(), delLocs.rend());
			for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
			{
				if (targetTileIt->second.begin() + *delIt >= targetTileIt->second.end()) continue;
				else targetTileIt->second.erase(targetTileIt->second.begin() + *delIt);
			}
			delLocs.clear();
		}
	}

	NeuronTree outputTree;
	for (boost::container::flat_map<string, QList<NeuronSWC>>::iterator mapIt = targetNodeTileMap.begin(); mapIt != targetNodeTileMap.end(); ++mapIt)
		outputTree.listNeuron.append(mapIt->second);

	return outputTree;
}


// ======================================= SWC Tracing-related Operations =======================================
void NeuronStructUtil::downstream_subTreeExtract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& subTreeList, const NeuronSWC& startingNode, map<int, size_t>& node2locMap, map<int, vector<size_t>>& node2childLocMap)
{
	NeuronStructUtil::node2loc_node2childLocMap(inputList, node2locMap, node2childLocMap);
	
	QList<NeuronSWC> parents;
	QList<NeuronSWC> children;
	parents.push_back(startingNode);
	vector<size_t> childLocs;
	do
	{
		children.clear();
		childLocs.clear();
		for (QList<NeuronSWC>::iterator pasIt = parents.begin(); pasIt != parents.end(); ++pasIt)
		{
			if (node2childLocMap.find(pasIt->n) != node2childLocMap.end()) childLocs = node2childLocMap.at(pasIt->n);
			else continue;
			
			for (vector<size_t>::iterator childLocIt = childLocs.begin(); childLocIt != childLocs.end(); ++childLocIt)
			{
				subTreeList.append(inputList.at(*childLocIt));
				children.push_back(inputList.at(*childLocIt));
			}
		}
		parents = children;
	} while (childLocs.size() > 0);
	
	subTreeList.push_front(startingNode);

	return;
}

void NeuronStructUtil::wholeSingleTree_extract(const QList<NeuronSWC>& inputList, QList<NeuronSWC>& tracedList, const NeuronSWC& startingNode)
{
	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(inputList, node2locMap, node2childLocMap);

	if (startingNode.parent == -1) NeuronStructUtil::downstream_subTreeExtract(inputList, tracedList, startingNode, node2locMap, node2childLocMap);
	else
	{
		int parentID = startingNode.parent;
		int somaNodeID = inputList.at(node2locMap.at(parentID)).n;

		while (1)
		{
			parentID = inputList.at(node2locMap.at(parentID)).parent;
			if (parentID != -1) somaNodeID = inputList.at(node2locMap.at(parentID)).n;
			else break;
		}
		
		NeuronSWC rootNode = inputList.at(node2locMap.at(somaNodeID));
		NeuronStructUtil::downstream_subTreeExtract(inputList, tracedList, rootNode, node2locMap, node2childLocMap);
	}
}
//============================================================================================================


// ========================================== SWC Profiling Methods ==========================================
QList<NeuronSWC> NeuronStructUtil::removeRednNode(const NeuronTree& inputTree)
{
	// -- This method removes dupliated nodes.

	boost::container::flat_map<string, QList<NeuronSWC>> nodeTileMap;
	NeuronStructUtil::nodeTileMapGen(inputTree, nodeTileMap);
	cout << "SWC tile number: " << nodeTileMap.size() << endl;

	QList<NeuronSWC> outputNodeList;
	cout << "processing tile: ";
	int tileCount = 0;
	for (boost::container::flat_map<string, QList<NeuronSWC>>::iterator tileIt = nodeTileMap.begin(); tileIt != nodeTileMap.end(); ++tileIt)
	{
		++tileCount;
		cout << tileCount << " ";
		bool redun = true;
		while (redun)
		{
			for (QList<NeuronSWC>::iterator it1 = tileIt->second.begin(); it1 != tileIt->second.end(); ++it1)
			{
				for (QList<NeuronSWC>::iterator it2 = tileIt->second.begin(); it2 != tileIt->second.end(); ++it2)
				{
					if (it2 == it1) continue;

					if (it2->x == it1->x && it2->y == it1->y && it2->z == it1->z)
					{
						if (it2->parent == -1)
						{
							tileIt->second.erase(it2);
							goto NODE_DELETED;
						}
					}
				}
			}
			redun = false;

		NODE_DELETED:
			continue;
		}

		outputNodeList.append(tileIt->second);
	}
	cout << endl;

	return outputNodeList;
}

NeuronTree NeuronStructUtil::swcZclenUP(const NeuronTree& inputTree, float zThre)
{
	// -- This method removes signal nodes from different depths that share the same x-y coordinates due to 2D connected component procedure for 3D structure.
	//      zThre: depth window allowance.
	//      Nodes that share the same x-y coordinates but belong to different zThre groups will still be considered different nodes.

	map<string, vector<NeuronSWC>> xyLabeledNodeMap;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int(it->x / 2));
		string yLabel = to_string(int(it->y / 2));
		string labelKey = xLabel + "_" + yLabel;
		if (xyLabeledNodeMap.find(labelKey) != xyLabeledNodeMap.end()) xyLabeledNodeMap[labelKey].push_back(*it);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*it);
			xyLabeledNodeMap.insert(pair<string, vector<NeuronSWC>>(labelKey, newSet));
		}
	}

	NeuronTree outputTree;
	for (map<string, vector<NeuronSWC>>::iterator it = xyLabeledNodeMap.begin(); it != xyLabeledNodeMap.end(); ++it)
	{
		boost::container::flat_map<float, NeuronSWC> zSortedNodes; // boost::container::flat_map is vector based, so it can do pointer arithmetic.
		if (it->second.size() == 1)
		{
			outputTree.listNeuron.push_back(*(it->second.begin()));
			continue;
		}
		else
		{
			for (vector<NeuronSWC>::iterator nodeIt = it->second.begin(); nodeIt != it->second.end(); ++nodeIt)
				zSortedNodes.insert(pair<float, NeuronSWC>(nodeIt->z, *nodeIt));

			boost::container::flat_map<float, NeuronSWC>::iterator startIt = zSortedNodes.begin();
			for (boost::container::flat_map<float, NeuronSWC>::iterator zIt = zSortedNodes.begin() + 1; zIt != zSortedNodes.end(); ++zIt)
			{
				if (zIt->second.z - (zIt - 1)->second.z > zThre)
				{
					ptrdiff_t steps = zIt - startIt;
					ptrdiff_t selectedLoc = ptrdiff_t(int(steps) / 2);
					outputTree.listNeuron.push_back((startIt + selectedLoc)->second);
					startIt = zIt;
				}
			}
			if (startIt == zSortedNodes.begin())
			{
				ptrdiff_t selectedLoc = ptrdiff_t(int((zSortedNodes.end() - zSortedNodes.begin()) / 2));
				outputTree.listNeuron.push_back((startIt + selectedLoc)->second);
			}
		}
	}

	return outputTree;
}

map<string, float> NeuronStructUtil::selfNodeDist(const QList<NeuronSWC>& inputNodeList)
{
	// -- This method computes the distance of every node to its nearest node in the given swc.
	// -- The output is a map that contains 4 measures: mean, std, var, median. 

	boost::container::flat_map<string, vector<NeuronSWC>> labeledNodeMap;
	NeuronStructUtil::nodeTileMapGen(inputNodeList, labeledNodeMap, 30);

	float distSum = 0;
	vector<float> distVec;
	for (boost::container::flat_map<string, vector<NeuronSWC>>::iterator it = labeledNodeMap.begin(); it != labeledNodeMap.end(); ++it)
	{
		if (it->second.size() == 1)
		{
			float dist = 10000;
			for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
			{
				if (it->second.begin()->x == nodeIt->x && it->second.begin()->y == nodeIt->y && it->second.begin()->z == nodeIt->z)
					continue;

				float tempDist = sqrtf((it->second.begin()->x - nodeIt->x) * (it->second.begin()->x - nodeIt->x) +
					(it->second.begin()->y - nodeIt->y) * (it->second.begin()->y - nodeIt->y) +
					(it->second.begin()->z - nodeIt->z) * (it->second.begin()->z - nodeIt->z) * zRATIO * zRATIO);
				if (tempDist < dist) dist = tempDist;
			}
			distVec.push_back(dist);
			distSum = distSum + dist;
			continue;
		}

		for (vector<NeuronSWC>::iterator nodeIt1 = it->second.begin(); nodeIt1 != it->second.end(); ++nodeIt1)
		{
			float dist = 10000;
			for (vector<NeuronSWC>::iterator nodeIt2 = it->second.begin(); nodeIt2 != it->second.end(); ++nodeIt2)
			{
				if (nodeIt1 == nodeIt2) continue;

				float tempDist = sqrtf((nodeIt1->x - nodeIt2->x) * (nodeIt1->x - nodeIt2->x) +
					(nodeIt1->y - nodeIt2->y) * (nodeIt1->y - nodeIt2->y) +
					(nodeIt1->z - nodeIt2->z) * (nodeIt1->z - nodeIt2->z) * zRATIO * zRATIO);
				if (tempDist < dist) dist = tempDist;
			}
			distVec.push_back(dist);
			distSum = distSum + dist;
		}
	}

	float distMean = distSum / float(distVec.size());
	float distVarSum = 0;
	for (vector<float>::iterator it = distVec.begin(); it != distVec.end(); ++it) distVarSum = distVarSum + (*it - distMean) * (*it - distMean);
	float distVar = distVarSum / float(distVec.size());
	float distStd = sqrtf(distVar);
	sort(distVec.begin(), distVec.end());
	float distMedian = float(distVec.at(floor(distVec.size() / 2)));

	map<string, float> outputMap;
	outputMap.insert(pair<string, float>("mean", distMean));
	outputMap.insert(pair<string, float>("std", distStd));
	outputMap.insert(pair<string, float>("var", distVar));
	outputMap.insert(pair<string, float>("median", distMedian));

	return outputMap;
}

NeuronTree NeuronStructUtil::swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float distThre, float nodeTileLength)
{
	// -- This method identifies positive signal nodes from noise with given reference NeuronTree.
	// -- radius: tile length
	// -- distThre: distance threshold for nearest node pair

	map<string, vector<NeuronSWC>> gridSWCmap; // Better use vector instead of set here, as set by default sorts the elements.
	// This can cause complication if the element is a data struct.

	NeuronStructUtil::nodeTileMapGen(refTree, gridSWCmap, nodeTileLength);

	NeuronTree outputTree;
	NeuronTree refConfinedFilteredTree;
	for (QList<NeuronSWC>::const_iterator suIt = subjectTree.listNeuron.begin(); suIt != subjectTree.listNeuron.end(); ++suIt)
	{
		string xLabel = to_string(int((suIt->x) / (nodeTileLength)));
		string yLabel = to_string(int((suIt->y) / (nodeTileLength)));
		string zLabel = to_string(int((suIt->z) / (nodeTileLength / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;

		if (gridSWCmap.find(keyLabel) != gridSWCmap.end())
		{
			bool identified = false;
			for (vector<NeuronSWC>::iterator nodeIt = gridSWCmap[keyLabel].begin(); nodeIt != gridSWCmap[keyLabel].end(); ++nodeIt)
			{
				float dist = sqrt((nodeIt->x - suIt->x) * (nodeIt->x - suIt->x) + (nodeIt->y - suIt->y) * (nodeIt->y - suIt->y) +
					zRATIO * zRATIO * (nodeIt->z - suIt->z) * (nodeIt->z - suIt->z));

				if (dist <= distThre)
				{
					outputTree.listNeuron.push_back(*suIt);
					(outputTree.listNeuron.end() - 1)->type = 2;
					identified = true;
					break;
				}
			}

			if (!identified)
			{
				outputTree.listNeuron.push_back(*suIt);
				(outputTree.listNeuron.end() - 1)->type = 3;
			}
		}
		else refConfinedFilteredTree.listNeuron.push_back(*suIt);
	}

	return outputTree;
}
// ===================================== END of [SWC Profiling Methods] =====================================


// =================================== SWC <-> Connected Components ====================================
vector<connectedComponent> NeuronStructUtil::swc2signal2DBlobs(const NeuronTree& inputTree)
{
	// -- Finds signal blobs "slice by slice" from input NeuronTree. Each slice is independent to one another.
	// -- Therefore, the same real blobs in 3D are consists of certain amount of 2D "blob slices" produced by this method. 
	// -- Each 2D blob slice accounts for 1 ImgAnalyzer::connectedComponent.

	vector<NeuronSWC> allNodes;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it) allNodes.push_back(*it);

	vector<connectedComponent> connComps2D;
	int islandCount = 0;
	cout << "number of SWC nodes processed: ";
	for (vector<NeuronSWC>::iterator nodeIt = allNodes.begin(); nodeIt != allNodes.end(); ++nodeIt)
	{
		if (int(nodeIt - allNodes.begin()) % 10000 == 0) cout << int(nodeIt - allNodes.begin()) << " ";
		for (vector<connectedComponent>::iterator connIt = connComps2D.begin(); connIt != connComps2D.end(); ++connIt)
		{
			if (connIt->coordSets.empty()) continue;
			else if (int(nodeIt->z) == connIt->coordSets.begin()->first)
			{
				for (set<vector<int>>::iterator dotIt = connIt->coordSets[int(nodeIt->z)].begin(); dotIt != connIt->coordSets[int(nodeIt->z)].end(); ++dotIt)
				{
					if (int(nodeIt->x) <= dotIt->at(0) + 1 && int(nodeIt->x) >= dotIt->at(0) - 1 &&
						int(nodeIt->y) <= dotIt->at(1) + 1 && int(nodeIt->y) >= dotIt->at(1) - 1)
					{
						vector<int> newCoord(3);
						newCoord[0] = int(nodeIt->x);
						newCoord[1] = int(nodeIt->y);
						newCoord[2] = int(nodeIt->z);
						connIt->coordSets[newCoord[2]].insert(newCoord);
						connIt->size = connIt->size + 1;

						if (newCoord[0] < connIt->xMin) connIt->xMin = newCoord[0];
						else if (newCoord[0] > connIt->xMax) connIt->xMax = newCoord[0];

						if (newCoord[1] < connIt->yMin) connIt->yMin = newCoord[1];
						else if (newCoord[1] > connIt->yMax) connIt->yMax = newCoord[1];

						goto NODE_INSERTED;
					}
				}
			}
		}

		{
			++islandCount;
			connectedComponent newIsland;
			newIsland.islandNum = islandCount;
			vector<int> newCoord(3);
			newCoord[0] = int(nodeIt->x);
			newCoord[1] = int(nodeIt->y);
			newCoord[2] = int(nodeIt->z);
			set<vector<int>> coordSet;
			coordSet.insert(newCoord);
			newIsland.coordSets.insert(pair<int, set<vector<int>>>(newCoord[2], coordSet));
			newIsland.xMax = newCoord[0];
			newIsland.xMin = newCoord[0];
			newIsland.yMax = newCoord[1];
			newIsland.yMin = newCoord[1];
			newIsland.zMin = newCoord[2];
			newIsland.zMax = newCoord[2];
			newIsland.size = 1;
			connComps2D.push_back(newIsland);
		}

	NODE_INSERTED:
		continue;
	}
	cout << endl << endl;

	vector<float> center(3);
	for (vector<connectedComponent>::iterator it = connComps2D.begin(); it != connComps2D.end(); ++it)
		ImgAnalyzer::ChebyshevCenter_connComp(*it);

	return connComps2D;
}

vector<connectedComponent> NeuronStructUtil::swc2signal3DBlobs(const NeuronTree& inputTree)
{
	// -- This method is a wrapper of NeuronStructUtil::swc2signal2DBlobs and NeuronStructUtil::merge2DConnComponent.
	// -- It produces 3D signal blobs by calling the two swc2signal2DBlobs and merge2DConnComponent sequentially.

	vector<connectedComponent> inputConnCompList = this->swc2signal2DBlobs(inputTree);	
	vector<connectedComponent> outputConnCompList = this->merge2DConnComponent(inputConnCompList);

	return outputConnCompList;
}

vector<connectedComponent> NeuronStructUtil::merge2DConnComponent(const vector<connectedComponent>& inputConnCompList)
{
	// -- This method finds 3D signal blobs by grouping 2D signal blobs together, which are generated by NeuronStructUtil::swc2signal2DBlobs.
	// -- This method is typically called by NeuronStructUtil::swc2signal2DBlobs when identifying 3D blobs from 2D ones.
	// -- The approach is consists of 2 stages:
	//		1. Identifying the same 3D blobs slice by slice.
	//		2. Merging 3D blobs that contain the same 2D blobs.

	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";

	vector<connectedComponent> outputConnCompList;

	int zMax = 0;

	// -- I notice that boost's container templates are able to lift up the performace by ~30%.
	boost::container::flat_map<int, boost::container::flat_set<int>> b2Dtob3Dmap;
	b2Dtob3Dmap.clear();
	boost::container::flat_map<int, boost::container::flat_set<int>> b3Dcomps;  // a map from 3D connected components to all of its associated 2D connected components
	b3Dcomps.clear();
	// ---------------------------------------------------------------------------------------

	// --------- First slice, container initialization --------------
	int sliceBlobCount = 0;
	for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
	{
		if (it->coordSets.begin()->first > zMax) zMax = it->coordSets.begin()->first;

		if (it->coordSets.begin()->first == 0) // 1st slice connected components profile initialization
		{
			++sliceBlobCount;
			boost::container::flat_set<int> blob3D;
			blob3D.insert(sliceBlobCount);
			b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(it->islandNum, blob3D));
			boost::container::flat_set<int> comps;
			comps.insert(it->islandNum);
			b3Dcomps[sliceBlobCount] = comps;
		}
	}
	// -----------------------------------------------------------

	// ------------------------------------------- Merge 2D blobs from 2 adjacent slices -------------------------------------------
	vector<connectedComponent> currSliceConnComps;
	vector<connectedComponent> preSliceConnComps;
	size_t increasedSize;
	for (int i = 1; i <= zMax; ++i)
	{
		currSliceConnComps.clear();
		preSliceConnComps.clear();

		increasedSize = 0;
		for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
			if (it->coordSets.begin()->first == i) currSliceConnComps.push_back(*it); // collect all connected components from the current slice
		if (currSliceConnComps.empty())
		{
			cout << i << "->0 ";
			continue;
		}

		cout << i << "->";
		for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
			if (it->coordSets.begin()->first == i - 1) preSliceConnComps.push_back(*it);  // collect all connected components from the previous slice
		if (preSliceConnComps.empty())
		{
			// If the previous slice is empty, all 2D components found in the current slice will be part of new 3D components.
			for (vector<connectedComponent>::iterator newCompsIt = currSliceConnComps.begin(); newCompsIt != currSliceConnComps.end(); ++newCompsIt)
			{
				++sliceBlobCount;
				boost::container::flat_set<int> blob3D;
				blob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(newCompsIt->islandNum, blob3D));
				boost::container::flat_set<int> comps;
				comps.insert(newCompsIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
			continue;
		}

		for (vector<connectedComponent>::iterator currIt = currSliceConnComps.begin(); currIt != currSliceConnComps.end(); ++currIt)
		{
			bool merged = false;
			for (vector<connectedComponent>::iterator preIt = preSliceConnComps.begin(); preIt != preSliceConnComps.end(); ++preIt)
			{
				// First, use component boundaries to quickly exclude those pixels that can't be connected to any existing components.
				// And then create new components for these pixels.
				if (currIt->xMin > preIt->xMax + 2 || currIt->xMax < preIt->xMin - 2 ||
					currIt->yMin > preIt->yMax + 2 || currIt->yMax < preIt->yMin - 2) continue; 

				for (set<vector<int>>::iterator currDotIt = currIt->coordSets.begin()->second.begin(); currDotIt != currIt->coordSets.begin()->second.end(); ++currDotIt)
				{
					for (set<vector<int>>::iterator preDotIt = preIt->coordSets.begin()->second.begin(); preDotIt != preIt->coordSets.begin()->second.end(); ++preDotIt)
					{
						if (currDotIt->at(0) >= preDotIt->at(0) - 1 && currDotIt->at(0) <= preDotIt->at(0) + 1 &&
							currDotIt->at(1) >= preDotIt->at(1) - 1 && currDotIt->at(1) <= preDotIt->at(1) + 1)
						{
							merged = true;
							// Find out to which 3D component the 2D component connected to the pixel belong.    
							boost::container::flat_set<int> asso3Dblob = b2Dtob3Dmap[preIt->islandNum];
							
							// Register the component of the pixel in the current slice to b2Dtob3Dmap.
							b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(currIt->islandNum, asso3Dblob));
							
							// Add a new entry of newly identified 2D component that is connected to the existing 3D component to b3Dcomps.
							for (boost::container::flat_set<int>::iterator blob3DIt = asso3Dblob.begin(); blob3DIt != asso3Dblob.end(); ++blob3DIt)
								b3Dcomps[*blob3DIt].insert(currIt->islandNum);

							goto BLOB_MERGED;
						}
					}
				}

				if (!merged) continue;

			BLOB_MERGED:
				merged = true;
			}

			if (!merged) // All 2D blobs in the current slice fail to find its associated 3D blobs. Create new 3D blobs for them here.
			{
				++sliceBlobCount;
				boost::container::flat_set<int> newBlob3D;
				newBlob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(currIt->islandNum, newBlob3D));
				boost::container::flat_set<int> comps;
				comps.insert(currIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
		}
		cout << increasedSize << ", ";
	}
	cout << endl << endl;
	cout << "Done merging 2D blobs from every 2 slices." << endl << endl;
	// ---------------------------------------- END of [Merge 2D blobs from 2 adjacent slices] -------------------------------------------

	// ------------------------------------------ Merge 3D blobs --------------------------------------------
	// Merge any 3D blobs if any of them share the same 2D blob members.
	cout << "Now merging 3D blobs.." << endl;
	cout << " -- oroginal 3D blobs number: " << b3Dcomps.size() << endl;
	bool mergeFinish = false;
	int currBaseBlob = 1;
	while (!mergeFinish)
	{
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator checkIt1 = b3Dcomps.begin(); checkIt1 != b3Dcomps.end(); ++checkIt1)
		{
			if (checkIt1->first < currBaseBlob) continue;
			for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator checkIt2 = checkIt1 + 1; checkIt2 != b3Dcomps.end(); ++checkIt2)
			{
				//if (checkIt2 == checkIt1) continue;
				for (boost::container::flat_set<int>::iterator member1 = checkIt1->second.begin(); member1 != checkIt1->second.end(); ++member1)
				{
					for (boost::container::flat_set<int>::iterator member2 = checkIt2->second.begin(); member2 != checkIt2->second.end(); ++member2)
					{
						if (*member2 == *member1)
						{
							checkIt1->second.insert(checkIt2->second.begin(), checkIt2->second.end());
							b3Dcomps.erase(checkIt2);
							currBaseBlob = checkIt1->first;
							cout << "  merging blob " << checkIt1->first << " and blob " << checkIt2->first << endl;
							goto MERGED;
						}
					}
				}
			}
		}
		mergeFinish = true;

	MERGED:
		continue;
	}
	cout << " -- new 3D blobs number: " << b3Dcomps.size() << endl;
	// --------------------------------------- END of [Merge 3D blobs] --------------------------------------

	// ------------------------------------- Create 3D connected component data -------------------------------------
	map<int, connectedComponent> compsMap;
	for (vector<connectedComponent>::const_iterator inputIt = inputConnCompList.begin(); inputIt != inputConnCompList.end(); ++inputIt)
		compsMap.insert(pair<int, connectedComponent>(inputIt->islandNum, *inputIt));
	int newLabel = 0;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = b3Dcomps.begin(); it != b3Dcomps.end(); ++it)
	{
		++newLabel;
		connectedComponent newComp;
		newComp.islandNum = newLabel;
		newComp.size = 0;
		newComp.xMax = 0; newComp.xMin = 0;
		newComp.yMax = 0; newComp.yMin = 0;
		newComp.zMax = 0; newComp.zMin = 0;
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			// A 3D connected component may contain different 2D components from the same slice.
			if (newComp.coordSets.find(compsMap.at(*it2).coordSets.begin()->first) != newComp.coordSets.end())
			{
				for (set<vector<int>>::iterator it3 = compsMap.at(*it2).coordSets.begin()->second.begin();
					it3 != compsMap.at(*it2).coordSets.begin()->second.end(); ++it3)
					newComp.coordSets.at(compsMap.at(*it2).coordSets.begin()->first).insert(*it3);
				
				newComp.xMax = getMax(newComp.xMax, compsMap.at(*it2).xMax);
				newComp.xMin = getMin(newComp.xMin, compsMap.at(*it2).xMin);
				newComp.yMax = getMax(newComp.yMax, compsMap.at(*it2).yMax);
				newComp.yMin = getMin(newComp.yMin, compsMap.at(*it2).yMin);
				newComp.zMax = getMax(newComp.zMax, compsMap.at(*it2).zMax);
				newComp.zMin = getMin(newComp.zMin, compsMap.at(*it2).zMin);
				newComp.size = newComp.size + compsMap.at(*it2).size;
			}
			else
			{
				newComp.coordSets.insert(pair<int, set<vector<int>>>(compsMap.at(*it2).coordSets.begin()->first, compsMap.at(*it2).coordSets.begin()->second));
				newComp.xMax = getMax(newComp.xMax, compsMap.at(*it2).xMax);
				newComp.xMin = getMin(newComp.xMin, compsMap.at(*it2).xMin);
				newComp.yMax = getMax(newComp.yMax, compsMap.at(*it2).yMax);
				newComp.yMin = getMin(newComp.yMin, compsMap.at(*it2).yMin);
				newComp.zMax = getMax(newComp.zMax, compsMap.at(*it2).zMax);
				newComp.zMin = getMin(newComp.zMin, compsMap.at(*it2).zMin);
				newComp.size = newComp.size + compsMap.at(*it2).size;
			}
		}

		outputConnCompList.push_back(newComp);
	}
	// --------------------------------- END of [Create 3D connected component data] ---------------------------------

	return outputConnCompList;
}

NeuronTree NeuronStructUtil::blobs2tree(const vector<connectedComponent>& inputconnComp, bool usingRadius2compNum)
{
	// -- This method produces a NeuronTree that is used to store connected component information. The radius column is ususally used for component label.

	NeuronTree outputTree;
	for (vector<connectedComponent>::const_iterator it = inputconnComp.begin(); it != inputconnComp.end(); ++it)
	{
		for (map<int, set<vector<int>>>::const_iterator sliceIt = it->coordSets.begin(); sliceIt != it->coordSets.end(); ++sliceIt)
		{
			for (set<vector<int>>::const_iterator pointIt = sliceIt->second.begin(); pointIt != sliceIt->second.end(); ++pointIt)
			{
				NeuronSWC newNode;
				newNode.x = pointIt->at(0);
				newNode.y = pointIt->at(1);
				newNode.z = pointIt->at(2);
				newNode.type = it->islandNum % 500;
				newNode.parent = -1;
				
				if (usingRadius2compNum) newNode.radius = it->islandNum; // Use SWC's radius column to keep component label information.

				outputTree.listNeuron.push_back(newNode);
			}
		}
	}

	return outputTree;
}
// =============================== END of [SWC <-> Connected Components] ===============================


// ==================================== Neuron Struct Clustering Methods ========================================
vector<connectedComponent> NeuronStructUtil::swc2clusters_distance(const NeuronTree& inputTree, float dist)
{
	QList<NeuronSWC> inputList = inputTree.listNeuron;
	vector<connectedComponent> outputConnCompList;
	outputConnCompList.clear();
	connectedComponent connComp1;
	vector<int> node1Coord(3);
	node1Coord.at(0) = int(inputList.begin()->x);
	node1Coord.at(1) = int(inputList.begin()->y);
	node1Coord.at(2) = int(inputList.begin()->z);
	set<vector<int>> connComp1Set;
	connComp1Set.insert(node1Coord);
	connComp1.coordSets.insert(pair<int, set<vector<int>>>(node1Coord.at(2), connComp1Set));
	connComp1.size = 1;
	outputConnCompList.push_back(connComp1);
	inputList.erase(inputList.begin());

	while (inputList.size() > 0)
	{
		for (QList<NeuronSWC>::iterator nodeIt = inputList.begin(); nodeIt != inputList.end(); ++nodeIt)
		{
			for (vector<connectedComponent>::iterator cluIt = outputConnCompList.begin(); cluIt != outputConnCompList.end(); ++cluIt)
			{
				for (map<int, set<vector<int>>>::iterator zIt = cluIt->coordSets.begin(); zIt != cluIt->coordSets.end(); ++zIt)
				{
					for (set<vector<int>>::iterator cluNodeIt = zIt->second.begin(); cluNodeIt != zIt->second.end(); ++cluNodeIt)
					{
						if (sqrt((nodeIt->x - float(cluNodeIt->at(0))) * (nodeIt->x - float(cluNodeIt->at(0))) +
							     (nodeIt->y - float(cluNodeIt->at(1))) * (nodeIt->y - float(cluNodeIt->at(1))) +
							     (nodeIt->z - float(cluNodeIt->at(2))) * (nodeIt->z - float(cluNodeIt->at(2))) * zRATIO * zRATIO) <= dist)
						{
							if (cluIt->coordSets.find(nodeIt->z) != cluIt->coordSets.end())
							{
								vector<int> newPoint(3);
								newPoint.at(0) = int(nodeIt->x);
								newPoint.at(1) = int(nodeIt->y);
								newPoint.at(2) = int(nodeIt->z);
								cluIt->coordSets[newPoint.at(2)].insert(newPoint);
								++cluIt->size;
							}
							else
							{
								vector<int> newPoint(3);
								newPoint.at(0) = int(nodeIt->x);
								newPoint.at(1) = int(nodeIt->y);
								newPoint.at(2) = int(nodeIt->z);
								set<vector<int>> newSet;
								newSet.insert(newPoint);
								cluIt->coordSets.insert(pair<int, set<vector<int>>>(newPoint.at(2), newSet));
								++cluIt->size;
							}

							inputList.erase(nodeIt);
							goto NODE_PROCESSED;
						}
					}
				}
			}

			connectedComponent newComp;
			vector<int> newCompPoint(3);
			newCompPoint.at(0) = int(nodeIt->x);
			newCompPoint.at(1) = int(nodeIt->y);
			newCompPoint.at(2) = int(nodeIt->z);
			set<vector<int>> newCompSet;
			newCompSet.insert(newCompPoint);
			newComp.coordSets.insert(pair<int, set<vector<int>>>(newCompPoint.at(2), newCompSet));
			newComp.size = 1;
			outputConnCompList.push_back(newComp);
			inputList.erase(nodeIt);
			break;
		}

	NODE_PROCESSED:
		continue;
	}

	vector<connectedComponent>::iterator mergeIt;
	vector<connectedComponent>::iterator currCompIt;
	while (1)
	{
		for (vector<connectedComponent>::iterator compBaseIt = outputConnCompList.begin(); compBaseIt != outputConnCompList.end(); ++compBaseIt)
		{
			for (vector<connectedComponent>::iterator checkCompIt = compBaseIt + 1; checkCompIt != outputConnCompList.end(); ++checkCompIt)
			{
				for (map<int, set<vector<int>>>::iterator checkIt = checkCompIt->coordSets.begin(); checkIt != checkCompIt->coordSets.end(); ++checkIt)
				{
					for (set<vector<int>>::iterator checkNodeIt = checkIt->second.begin(); checkNodeIt != checkIt->second.end(); ++checkNodeIt)
					{
						for (map<int, set<vector<int>>>::iterator currIt = compBaseIt->coordSets.begin(); currIt != compBaseIt->coordSets.end(); ++currIt)
						{
							for (set<vector<int>>::iterator currNodeIt = currIt->second.begin(); currNodeIt != currIt->second.end(); ++currNodeIt)
							{
								if (sqrt((float(currNodeIt->at(0)) - float(checkNodeIt->at(0))) * (float(currNodeIt->at(0)) - float(checkNodeIt->at(0))) +
										 (float(currNodeIt->at(1)) - float(checkNodeIt->at(1))) * (float(currNodeIt->at(1)) - float(checkNodeIt->at(1))) +
										 (float(currNodeIt->at(2)) - float(checkNodeIt->at(2))) * (float(currNodeIt->at(2)) - float(checkNodeIt->at(2))) * zRATIO * zRATIO) <= dist)
								{
									mergeIt = checkCompIt;
									currCompIt = compBaseIt;
									goto CLUSTER_MERGE;
								}
							}
						}
					}
				}
			}		
		}
		break;

	CLUSTER_MERGE:
		{
			for (map<int, set<vector<int>>>::iterator zIt = mergeIt->coordSets.begin(); zIt != mergeIt->coordSets.end(); ++zIt)
			{
				for (set<vector<int>>::iterator nodeIt = zIt->second.begin(); nodeIt != zIt->second.end(); ++nodeIt)
				{
					if (currCompIt->coordSets.find(nodeIt->at(0)) != currCompIt->coordSets.end())
					{
						vector<int> newPoint(3);
						newPoint.at(0) = nodeIt->at(0);
						newPoint.at(1) = nodeIt->at(1);
						newPoint.at(2) = nodeIt->at(2);
						currCompIt->coordSets[newPoint.at(2)].insert(newPoint);
					}
					else
					{
						vector<int> newPoint(3);
						newPoint.at(0) = nodeIt->at(0);
						newPoint.at(1) = nodeIt->at(1);
						newPoint.at(2) = nodeIt->at(2);
						set<vector<int>> newSet;
						newSet.insert(newPoint);
						currCompIt->coordSets.insert(pair<int, set<vector<int>>>(newPoint.at(2), newSet));
					}
				}
			}

			outputConnCompList.erase(mergeIt);
			continue;
		}
	}

	return outputConnCompList;
}
// ================================ END of [Neuron Struct Clustering Methods] ===================================


/* =================================== Volumetric SWC sampling methods =================================== */
void NeuronStructUtil::sigNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, float ratio, float distance)
{
	// -- Randomly generate signal patches within given distance range
	//      ratio:    the ratio of targeted number of upsampling nodes to the number of maunal nodes in the inputTree
	//      distance: the radius allowed with SWC node centered

	cout << "target signal patch number: " << int(inputTree.listNeuron.size() * ratio) << endl;
	int nodeCount = 0;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		outputTree.listNeuron.push_back(*it);
		(outputTree.listNeuron.end() - 1)->parent = -1;
		(outputTree.listNeuron.end() - 1)->type = 2;
		int foldCount = 2;
		while (foldCount <= ratio)
		{
			int randNumX = rand() % int(distance * 2) + int(it->x - distance);
			int randNumY = rand() % int(distance * 2) + int(it->y - distance);
			int randNumZ = rand() % int(distance * 2) + int(it->z - distance);

			++nodeCount;
			if (nodeCount % 10000 == 0) cout << nodeCount << " signal nodes generated." << endl;

			NeuronSWC newNode;
			newNode.x = randNumX;
			newNode.y = randNumY;
			newNode.z = randNumZ;
			newNode.type = 2;
			newNode.radius = 1;
			newNode.parent = -1;
			outputTree.listNeuron.push_back(newNode);

			++foldCount;
		}
	}
}

void NeuronStructUtil::bkgNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, int dims[], float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance
	//      dims:     image stack dimension
	//      ratio:    the ratio of targeted number of background nodes to the number of manual nodes in the inputTree
	//      distance: the forbidden distance from each SWC node

	QList<NeuronSWC> neuronList = inputTree.listNeuron;
	int targetBkgNodeNum = int(neuronList.size() * ratio);
	cout << targetBkgNodeNum << " targeted bkg nodes to ge generated." << endl;
	int bkgNodeCount = 0;
	while (bkgNodeCount <= targetBkgNodeNum)
	{
		int randNumX = rand() % (dims[0] - 20) + 10;
		int randNumY = rand() % (dims[1] - 20) + 10;
		int randNumZ = rand() % dims[2];

		bool flag = false;
		for (QList<NeuronSWC>::iterator it = neuronList.begin(); it != neuronList.end(); ++it)
		{
			float distSqr;
			float diffx = float(randNumX) - it->x;
			float diffy = float(randNumY) - it->y;
			float diffz = float(randNumZ) - it->z;
			distSqr = diffx * diffx + diffy * diffy + diffz * diffz;

			if (distSqr <= distance * distance)
			{
				flag = true;
				break;
			}
		}

		if (flag == false)
		{
			++bkgNodeCount;
			if (bkgNodeCount % 10000 == 0) cout << bkgNodeCount << " bkg nodes generated." << endl;

			NeuronSWC newBkgNode;
			newBkgNode.x = randNumX;
			newBkgNode.y = randNumY;
			newBkgNode.z = randNumZ;
			newBkgNode.type = 3;
			newBkgNode.radius = 1;
			newBkgNode.parent = -1;
			outputTree.listNeuron.push_back(newBkgNode);
		}
	}
}

void NeuronStructUtil::bkgNode_Gen_somaArea(const NeuronTree& intputTree, NeuronTree& outputTree, int xLength, int yLength, int zLength, float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance. This method aims to reinforce the background recognition near soma area.
	//      xLength, yLength, zLength: decide the range to apply with soma centered
	//      ratio:    the ratio of targeted number of soma background nodes to the number of manual nodes in the inputTree
	//      distance: the forbidden distance from each SWC node

	NeuronSWC somaNode;
	for (QList<NeuronSWC>::const_iterator it = intputTree.listNeuron.begin(); it != intputTree.listNeuron.end(); ++it)
	{
		if (it->parent == -1)
		{
			somaNode = *it;
			break;
		}
	}
	float xlb = somaNode.x - float(xLength);
	float xhb = somaNode.x + float(xLength);
	float ylb = somaNode.y - float(yLength);
	float yhb = somaNode.y + float(yLength);
	float zlb = somaNode.z - float(zLength);
	float zhb = somaNode.z + float(zLength);

	list<NeuronSWC> confinedNodes;
	for (QList<NeuronSWC>::const_iterator it = intputTree.listNeuron.begin(); it != intputTree.listNeuron.end(); ++it)
		if (xlb <= it->x && xhb >= it->x && ylb <= it->y && yhb >= it->y && zlb <= it->z && zhb >= it->z) confinedNodes.push_back(*it);
	
	int targetBkgNodeNum = int(float(xLength) * float(yLength) * float(zLength) * ratio);
	cout << targetBkgNodeNum << " targeted bkg nodes to ge generated." << endl;
	int bkgNodeCount = 0;
	while (bkgNodeCount <= targetBkgNodeNum)
	{
		int randNumX = rand() % int(xhb - xlb + 1) + int(xlb);
		int randNumY = rand() % int(yhb - ylb + 1) + int(ylb);
		int randNumZ = rand() % int(zhb - zlb + 1) + int(zlb);

		bool flag = false;
		for (list<NeuronSWC>::iterator it = confinedNodes.begin(); it != confinedNodes.end(); ++it)
		{
			float distSqr;
			float diffx = float(randNumX) - it->x;
			float diffy = float(randNumY) - it->y;
			float diffz = float(randNumZ) - it->z;
			distSqr = diffx * diffx + diffy * diffy + diffz * diffz;

			if (distSqr <= distance * distance)
			{
				flag = true;
				break;
			}
		}

		if (flag == false)
		{
			++bkgNodeCount;
			if (bkgNodeCount % 10000 == 0) cout << bkgNodeCount << " bkg nodes generated within the soma area." << endl;

			NeuronSWC newBkgNode;
			newBkgNode.x = randNumX;
			newBkgNode.y = randNumY;
			newBkgNode.z = randNumZ;
			newBkgNode.type = 3;
			newBkgNode.radius = 1;
			newBkgNode.parent = -1;
			outputTree.listNeuron.push_back(newBkgNode);
		}
	}
}
/* =================================== Volumetric SWC sampling methods =================================== */