//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  Most of NeuronStructUtil class methods intend to operate on the whole neuron struct level.
*  As 'utility' it is called, the functionalities provided in this class include:
*    a. [Basic neuron struct operations]                   -- cropping SWC, scaling SWC, swc registration, etc.
*    b. [Neuron struct profiling methods]                  -- node-tile mapping, node-location mapping, etc.
*    c. [SWC - ImgAnalyzer::connectedComponent operations] -- Methods of this category convert SWC into vector<ImgAnalyzer::connectedComponent>
*
*  Most of NeuronStructUtil class methods are implemented as static functions. The input NeuronTree is always set to be const so that it will not be modified.
*  A typical function call would need at least three input arguments:
*
*		NeuronStructUtil::func(const NeuronTree& inputTree, NeuronTree& outputTree, other input arguments);
*
********************************************************************************/

#include <iostream>
#include <iterator>
#include <set>
#include <cmath>

#include <boost/filesystem.hpp>

#include "basic_4dimage.h"

#include "NeuronStructUtilities.h"
#include "NeuronStructNavigator_Define.h"
#include "ImgProcessor.h"

using namespace boost;

/* ======================================== Segment Operations ========================================= */
segUnit NeuronStructUtil::segUnitConnect_end2end(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt)
{
	// connecting FROM segUnit1 TO segUnit2.

	if (segUnit1.tails.size() > 1 || segUnit2.tails.size() > 1)
		throw invalid_argument("Currently forked segment connection is not supported. Do nothing and return");

	segUnit newSeg;
	QList<NeuronSWC> newSegNodes;
	QList<NeuronSWC> endEditedNodes;

	switch (connOrt)
	{
	case head_tail:
	{
		int connTailID = *segUnit2.tails.cbegin();
		endEditedNodes = segUnit1.nodes;
		endEditedNodes.begin()->parent = connTailID; // In current implementation, the 1st element of a seg must be a root.
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_head:
	{
		int connTailID = *segUnit1.tails.cbegin();
		endEditedNodes = segUnit2.nodes;
		endEditedNodes.begin()->parent = connTailID; // In current implementation, the 1st element of a seg must be a root. 
		newSegNodes.append(segUnit1.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	case head_head:
	{
		int connTailID = segUnit2.head;
		for (map<int, vector<size_t>>::const_iterator it = segUnit2.seg_childLocMap.cbegin(); it != segUnit2.seg_childLocMap.cend(); ++it)
		{
			size_t nodeLoc = segUnit2.seg_nodeLocMap.at(it->first);
			NeuronSWC newNode = segUnit2.nodes.at(nodeLoc);
			if (it->second.empty())
			{
				//cout << newNode.x << " " << newNode.y << " " << newNode.z << endl;
				newNode.parent = -1;
			}
			else newNode.parent = segUnit2.nodes.at(*(it->second.cbegin())).n;
			endEditedNodes.push_back(newNode);
		}

		int editedSegSize = endEditedNodes.size();
		newSegNodes.append(endEditedNodes);
		newSegNodes.append(segUnit1.nodes);
		newSegNodes[endEditedNodes.size()].parent = connTailID;
		newSeg.nodes = newSegNodes;
		break;
	}
	case tail_tail:
	{
		int connTailID = *segUnit2.tails.cbegin();
		for (map<int, vector<size_t>>::const_iterator it = segUnit1.seg_childLocMap.cbegin(); it != segUnit1.seg_childLocMap.cend(); ++it)
		{
			size_t nodeLoc = segUnit1.seg_nodeLocMap.at(it->first);
			NeuronSWC newNode = segUnit1.nodes.at(nodeLoc);
			if (it->second.empty()) newNode.parent = connTailID;
			else newNode.parent = segUnit1.nodes.at(*(it->second.cbegin())).n;
			endEditedNodes.push_back(newNode);
		}
		newSegNodes.append(segUnit2.nodes);
		newSegNodes.append(endEditedNodes);
		newSeg.nodes = newSegNodes;
		break;
	}
	default:
		break;
	}

	return newSeg;
}

/*segUnit NeuronStructUtil::segUnitConnect_end2body(const segUnit& endSegUnit, const segUnit& bodySegUnit, const NeuronSWC& endSegNode, const NeuronSWC& bodySegNode)
{
	
}*/
/* ===================================================================================================== */



/* ===================================== Neuron Struct Processing ====================================== */
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

NeuronTree NeuronStructUtil::swcCombine(const vector<NeuronTree>& inputTrees)
{
	NeuronTree outputTree;
	ptrdiff_t listSize = 0;
	int nodeIDmax = 0;
	for (vector<NeuronTree>::const_iterator it = inputTrees.begin(); it != inputTrees.end(); ++it)
	{
		for (QList<NeuronSWC>::iterator outputNodeIt = outputTree.listNeuron.begin(); outputNodeIt != outputTree.listNeuron.end(); ++outputNodeIt)
			if (outputNodeIt->n > nodeIDmax) nodeIDmax = outputNodeIt->n;

		outputTree.listNeuron.append(it->listNeuron);
		for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin() + listSize; nodeIt != outputTree.listNeuron.end(); ++nodeIt)
		{
			nodeIt->n = nodeIt->n + nodeIDmax;
			if (nodeIt->parent == -1) continue;
			else nodeIt->parent = nodeIt->parent + nodeIDmax;
		}

		listSize = ptrdiff_t(outputTree.listNeuron.size());
	}

	return outputTree;
}

NeuronTree NeuronStructUtil::swcCombine(const map<string, NeuronTree>& inputTreesMap)
{
	NeuronTree outputTree;
	ptrdiff_t listSize = 0;
	int nodeIDmax = 0;
	for (map<string, NeuronTree>::const_iterator it = inputTreesMap.begin(); it != inputTreesMap.end(); ++it)
	{
		for (QList<NeuronSWC>::iterator outputNodeIt = outputTree.listNeuron.begin(); outputNodeIt != outputTree.listNeuron.end(); ++outputNodeIt)
			if (outputNodeIt->n > nodeIDmax) nodeIDmax = outputNodeIt->n;

		outputTree.listNeuron.append(it->second.listNeuron);
		for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin() + listSize; nodeIt != outputTree.listNeuron.end(); ++nodeIt)
		{
			nodeIt->n = nodeIt->n + nodeIDmax;
			if (nodeIt->parent == -1) continue;
			else nodeIt->parent = nodeIt->parent + nodeIDmax;
		}

		listSize = ptrdiff_t(outputTree.listNeuron.size());
	}

	return outputTree;
}

NeuronTree NeuronStructUtil::swcCombine(const map<string, profiledTree>& inputProfiledTreesMap)
{
	NeuronTree outputTree;

	ptrdiff_t listSize = 0;
	int nodeIDmax = 0;
	for (map<string, profiledTree>::const_iterator it = inputProfiledTreesMap.begin(); it != inputProfiledTreesMap.end(); ++it)
	{
		for (QList<NeuronSWC>::iterator outputNodeIt = outputTree.listNeuron.begin(); outputNodeIt != outputTree.listNeuron.end(); ++outputNodeIt)
			if (outputNodeIt->n > nodeIDmax) nodeIDmax = outputNodeIt->n;

		outputTree.listNeuron.append(it->second.tree.listNeuron);
		for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin() + listSize; nodeIt != outputTree.listNeuron.end(); ++nodeIt)
		{
			nodeIt->n = nodeIt->n + nodeIDmax;
			if (nodeIt->parent == -1) continue;
			else nodeIt->parent = nodeIt->parent + nodeIDmax;
		}

		listSize = ptrdiff_t(outputTree.listNeuron.size());
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

		std::sort(delLocs.rbegin(), delLocs.rend());
		for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) inputList.erase(inputList.begin() + *delIt);
		delLocs.clear();
	}
}

bool NeuronStructUtil::checkNodeType(const NeuronTree& inputTree, int nodeType)
{
	for (auto& node : inputTree.listNeuron)
	{
		if (node.type == nodeType) return true;
	}

	return false;
}

map<int, QList<NeuronSWC>> NeuronStructUtil::swcSplitByType(const NeuronTree& inputTree)
{
	map<int, QList<NeuronSWC>> outputNodeTypeMap;

	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		outputNodeTypeMap[it->type].append(*it);

	return outputNodeTypeMap;
}

int NeuronStructUtil::findSomaNodeID(const NeuronTree& inputTree)
{
	for (auto& node : inputTree.listNeuron)
	{
		if (node.type == 1 && node.parent == -1) return node.n;
	}

	return -1;
}

void NeuronStructUtil::somaCleanUp(NeuronTree& inputTree)
{
	profiledTree inputProfiledTree(inputTree);
	if (inputProfiledTree.segs.size() == 1)
	{
		int somaLoc = inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.begin()->second.head);
		if (inputProfiledTree.tree.listNeuron.at(somaLoc).type == 1)
		{

		}
	}
	
	inputProfiledTree.nodeCoordKeySegMapGen();
	
	// ------- Generate a map: Type 1 root's coordinates -> IDs of segments that have type 1 head node on it ------- //
	map<string, set<int>> somaCoordSegMap;
	string trueSomaCoordKey;
	int somaSegNum = 0;
	for (auto& seg : inputProfiledTree.segs)
	{
		segUnit& curSeg = seg.second;
		const NeuronSWC& headNode = curSeg.nodes.at(curSeg.seg_nodeLocMap.at(curSeg.head));
		
		if (headNode.type == 1)
		{
			if (curSeg.tails.size() > 1) continue;

			string headCoordKey = to_string(headNode.x) + "_" + to_string(headNode.y) + "_" + to_string(headNode.z);
			if (somaCoordSegMap.find(headCoordKey) != somaCoordSegMap.end()) continue;

			set<int> segIDs;
			pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = inputProfiledTree.nodeCoordKey2segMap.equal_range(headCoordKey);			
			for (boost::container::flat_multimap<string, int>::iterator it = range.first; it < range.second; ++it) segIDs.insert(it->second);
			somaCoordSegMap.insert({ headCoordKey, segIDs });

			if (segIDs.size() > somaSegNum)
			{
				somaSegNum = segIDs.size();
				trueSomaCoordKey = headCoordKey;
			}
		}
	}
	// ------------------------------------------------------------------------------------------------------------- //

	if (somaCoordSegMap.empty()) return;
	for (auto& somaCoord : somaCoordSegMap)
	{
		for (auto& segID : somaCoordSegMap.at(somaCoord.first))
		{
			const segUnit& currSeg = inputProfiledTree.segs.at(segID);
			for (auto& tailNodeID : currSeg.tails)
			{
				const NeuronSWC& tailNode = currSeg.nodes.at(currSeg.seg_nodeLocMap.at(tailNodeID));
				string tailCoordKey = to_string(tailNode.x) + "_" + to_string(tailNode.y) + "_" + to_string(tailNode.z);
				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = inputProfiledTree.nodeCoordKey2segMap.equal_range(tailCoordKey);
				for (boost::container::flat_multimap<string, int>::iterator it = range.first; it < range.second; ++it)
				{
					if (it->second == segID) continue;

					int type = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.segs.at(it->second).head).type;
					for (auto& node : currSeg.nodes) inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(node.n)].type = type;
					break;
				}
			}
		}
	}

	for (auto& somaSeg : somaCoordSegMap.at(trueSomaCoordKey)) 
		inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(somaSeg).head)].type = 1;
}

void NeuronStructUtil::removeRedunNodes(profiledTree& inputProfiledTree)
{
	boost::container::flat_set<int> delLocs;
	boost::container::flat_set<int> nodeIDMarked;
	for (auto& nodeID : inputProfiledTree.node2childLocMap)
	{
		const NeuronSWC& currNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(nodeID.first));
		for (auto& childLoc : nodeID.second)
		{
			const NeuronSWC& childNode = inputProfiledTree.tree.listNeuron.at(childLoc);
			if (childNode.x == currNode.x && childNode.y == currNode.y && childNode.z == currNode.z)
			{
				// [childNode] here is to be deleted, thus any node whose parent is the [childNode] should be changed to [childNode]'s parent, which is [currNode].
				for (auto& node : inputProfiledTree.tree.listNeuron)
					if (node.parent == childNode.n) node.parent = currNode.n;

				delLocs.insert(childLoc);
				nodeIDMarked.insert(childNode.n);
				
				// Not sure the following is still needed. The for-loop above seems to handle all cases already, but may be slower.
				// If theres only this part without the for-loop, it doesn't take care of the case where there are 3+ redundant nodes being examined structurally backward.
				// -- Need test to decide whether to keep this part.
				if (inputProfiledTree.node2childLocMap.find(childNode.n) != inputProfiledTree.node2childLocMap.end())
				{
					if (nodeIDMarked.find(currNode.n) == nodeIDMarked.end())
					{
						for (auto& grandChildLoc : inputProfiledTree.node2childLocMap.at(childNode.n)) inputProfiledTree.tree.listNeuron[grandChildLoc].parent = currNode.n;
					}
					else
					{
						int paNodeID = currNode.parent;
						while (nodeIDMarked.find(paNodeID) != nodeIDMarked.end()) paNodeID = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(paNodeID)).parent;
						for (auto& grandChildLoc : inputProfiledTree.node2childLocMap.at(childNode.n)) inputProfiledTree.tree.listNeuron[grandChildLoc].parent = paNodeID;
					}
				}
			}
		}
	}

	for (boost::container::flat_set<int>::reverse_iterator rit = delLocs.rbegin(); rit != delLocs.rend(); ++rit)
		inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + *rit);
	profiledTreeReInit(inputProfiledTree);
}

bool NeuronStructUtil::removeDupHeads(NeuronTree& inputTree)
{
	// This duplicated nodes removing method uses the head node of every segment as a guide to determine whether their head nodes can be removed.

	profiledTree inputProfiledTree(inputTree);
	if (inputProfiledTree.node2segMap.empty()) inputProfiledTree.nodeSegMapGen();
	if (inputProfiledTree.nodeCoordKey2nodeIDMap.empty()) inputProfiledTree.nodeCoordKeyNodeIDmapGen();
	boost::container::flat_map<ptrdiff_t, ptrdiff_t> childLoc2paLocMap;
	
	vector<ptrdiff_t> headDelLocs;
	inputProfiledTree.overlappedCoordMapGen();
	for (auto& dupCoord : inputProfiledTree.overlappedCoordMap)
	{
		if (dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::head).size() > 0)
		{
			// Case 1: One or more head nodes overlapping with only one either tail or body node.
			//         -> All head nodes should be the downstreams linking to the only tail or body node.
			if (dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::tail).size() + dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::body).size() == 1)
			{
				int targetNodeID;
				if (dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::tail).size() == 1) targetNodeID = dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::tail).begin()->second; 
				else targetNodeID = dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::body).begin()->second;

				for (auto& headSegNodePair : dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::head))
				{
					headDelLocs.push_back(inputProfiledTree.node2LocMap.at(headSegNodePair.second));
					for (auto& child : inputProfiledTree.node2childLocMap.at(headSegNodePair.second))
						childLoc2paLocMap.insert(pair<ptrdiff_t, ptrdiff_t>(child, inputProfiledTree.node2LocMap.at(targetNodeID)));
				}
			}
			// Case 2: One or more head nodes overlapping with only one body node. With case 1, is this still neccessary?
			else if (dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::body).size() == 1)
			{
				int targetNodeID = dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::body).begin()->second;

				for (auto& headSegNodePair : dupCoord.second.involvedSegsOriMap.at(integratedDataTypes::head))
				{
					headDelLocs.push_back(inputProfiledTree.node2LocMap.at(headSegNodePair.second));
					for (auto& child : inputProfiledTree.node2childLocMap.at(headSegNodePair.second))
						childLoc2paLocMap.insert(pair<ptrdiff_t, ptrdiff_t>(child, inputProfiledTree.node2LocMap.at(targetNodeID)));
				}
			}
		}
	}

	int delHeadLocNum = headDelLocs.size();

	for (auto& childLoc : childLoc2paLocMap)
		inputProfiledTree.tree.listNeuron[childLoc.first].parent = inputProfiledTree.tree.listNeuron.at(childLoc.second).n;
	
	std::sort(headDelLocs.rbegin(), headDelLocs.rend());
	for (auto& headDelLoc : headDelLocs) inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + headDelLoc);
	inputTree = inputProfiledTree.tree;

	return delHeadLocNum > 0 ? true : false;
}

void NeuronStructUtil::removeDupBranchingNodes(profiledTree& inputProfiledTree)
{
	if (inputProfiledTree.nodeCoordKey2segMap.empty()) inputProfiledTree.nodeCoordKeySegMapGen();

	vector<ptrdiff_t> headDelLocs;
	for (auto& seg : inputProfiledTree.segs)
	{
		const NeuronSWC& headNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(seg.second.head));
		string headSegTileKey = NeuronStructExplorer::getSegTileKey(headNode);
		if (inputProfiledTree.segTailMap.find(headSegTileKey) != inputProfiledTree.segTailMap.end())
		{
			for (auto& tailSegID : inputProfiledTree.segTailMap.at(headSegTileKey))
			{
				for (auto& tailID : inputProfiledTree.segs.at(tailSegID).tails)
				{
					const NeuronSWC& tailNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailID));
					if (headNode.x == tailNode.x && headNode.y == tailNode.y && headNode.z == tailNode.z)
					{
						headDelLocs.push_back(inputProfiledTree.node2LocMap.at(headNode.n));
						for (auto& childLoc : inputProfiledTree.node2childLocMap.at(headNode.n)) inputProfiledTree.tree.listNeuron[childLoc].parent = tailNode.n;
						goto HEADNODE_PICKED;
					}
				}
			}
		}

		{
			string headCoordKey = to_string(headNode.x) + "_" + to_string(headNode.y) + "_" + to_string(headNode.z);
			pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = inputProfiledTree.nodeCoordKey2segMap.equal_range(headCoordKey);
			for (boost::container::flat_multimap<string, int>::iterator it = range.first; it != range.second; ++it)
			{
				if (it->second == seg.first) continue;
				for (auto& node : inputProfiledTree.segs.at(it->second).nodes)
				{
					if (node.parent == -1) continue;
					if (node.x == headNode.x && node.y == headNode.y && node.z == headNode.z)
					{
						headDelLocs.push_back(inputProfiledTree.node2LocMap.at(headNode.n));
						for (auto& childLoc : inputProfiledTree.node2childLocMap.at(headNode.n)) inputProfiledTree.tree.listNeuron[childLoc].parent = node.n;
					}
				}
			}
		}

	HEADNODE_PICKED:
		continue;
	}

	sort(headDelLocs.rbegin(), headDelLocs.rend());
	for (vector<ptrdiff_t>::iterator delIt = headDelLocs.begin(); delIt != headDelLocs.end(); ++delIt)
		inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + *delIt);
}

bool NeuronStructUtil::multipleSegsCheck(const NeuronTree& inputTree)
{
	profiledTree inputProfiledTree(inputTree);
	if (inputProfiledTree.segs.size() > 1) return true;
	else return false;
}

NeuronTree NeuronStructUtil::removeDupSegs(const NeuronTree& inputTree)
{
	// **************** Remove simiple shadow segments **************** //
	// -- Simple shadow segments are simply identical segments. 
	profiledTree inputProfiledTree(inputTree);
	vector<segUnit> filteredSegs;
	for (auto& seg : inputProfiledTree.segs)
	{
		for (vector<segUnit>::iterator it = filteredSegs.begin(); it != filteredSegs.end(); ++it)
			if (*it == seg.second) goto SAME_SEG_FOUND;
		filteredSegs.push_back(seg.second);

	SAME_SEG_FOUND:
		continue;
	}
	// **************************************************************** //

	// ***************** Remove composite shadow segments ***************** //
	// -- Composite shadow segments are: 1) shadow segments that align through multiple segments
	//									 2) partial shadow segments - embbeded segments
	NeuronTree simpleShadowcleanedTree;
	for (auto& outputSeg : filteredSegs) simpleShadowcleanedTree.listNeuron.append(outputSeg.nodes);
	profiledTree profiledSimpleShadowCln(simpleShadowcleanedTree);
	profiledSimpleShadowCln.nodeCoordKeySegMapGen(); // This step could be time consuming.
	set<int> compositeDupSegIDs;
	vector<boost::container::flat_set<int>> coordKey2segIDs;
	for (auto& seg : profiledSimpleShadowCln.segs)
	{
		boost::container::flat_set<int> segIDsPreNode;
		for (auto& node : seg.second.nodes)
		{
			string nodeCoordKey = to_string(node.x) + "_" + to_string(node.y) + "_" + to_string(node.z);
			pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = profiledSimpleShadowCln.nodeCoordKey2segMap.equal_range(nodeCoordKey);
			// a. Essential segment found - singular node that results in non-overlapping edges.
			if (range.second - range.first == 1) goto TO_THE_NEXT_SEG; 
			
			// Record all involved segments other than self for every node in the segment in the same order.
			boost::container::flat_set<int> segIDsCurrNode;
			for (boost::container::flat_multimap<string, int>::iterator it = range.first; it != range.second; ++it)
				if (it->second != seg.first) segIDsCurrNode.insert(it->second);
			coordKey2segIDs.push_back(segIDsCurrNode);
		}

		// b. If a segment is an embedded shadow segment, it should have the same segment ID running through every node.
		for (auto& segID : coordKey2segIDs.at(0))
		{
			for (vector<boost::container::flat_set<int>>::iterator setIt = coordKey2segIDs.begin() + 1; setIt != coordKey2segIDs.end(); ++setIt)
				if (setIt->find(segID) == setIt->end()) goto NOT_EMBEDDED_SEG;
			compositeDupSegIDs.insert(seg.first); // Found other segment overlapping throughout the current segment, i.e., the current segment is an embedded segment, or "partial shadow segment".
			goto TO_THE_NEXT_SEG;
		}

		// c. If 2 adjacent nodes have some segment IDs that cannot be found in the other node, then this segment is a bridge segment and cannot be removed.
	NOT_EMBEDDED_SEG:
		for (vector<boost::container::flat_set<int>>::iterator setIt = coordKey2segIDs.begin() + 1; setIt != coordKey2segIDs.end(); ++setIt)
		{
			if (*setIt == *(setIt - 1)) continue;
			for (auto& preNodeSegID : *(setIt - 1))
			{
				if (setIt->find(preNodeSegID) == setIt->end())
				{
					for (auto& currNodeSegID : *setIt)
						if ((setIt - 1)->find(currNodeSegID) == (setIt - 1)->end()) goto TO_THE_NEXT_SEG; // Bridging situation found.
				}
			}
		}
		compositeDupSegIDs.insert(seg.first);

	TO_THE_NEXT_SEG:
		continue;
	}

	vector<ptrdiff_t> compositeDelLocs;
	for (set<int>::iterator segIDIt = compositeDupSegIDs.begin(); segIDIt != compositeDupSegIDs.end(); ++segIDIt)
	{
		for (auto& node : profiledSimpleShadowCln.segs.at(*segIDIt).nodes)
			compositeDelLocs.push_back(profiledSimpleShadowCln.node2LocMap.at(node.n));
	}
	std::sort(compositeDelLocs.rbegin(), compositeDelLocs.rend());
	for (auto& loc : compositeDelLocs) profiledSimpleShadowCln.tree.listNeuron.erase(profiledSimpleShadowCln.tree.listNeuron.begin() + loc);
	// ******************************************************************** //

	return profiledSimpleShadowCln.tree;
}

void NeuronStructUtil::splitNodeList(const QList<NeuronSWC>& inputNodes, const int splittingNodeID, QList<NeuronSWC>& downStreamNodes, QList<NeuronSWC>& upStreamNodes)
{
	NeuronTree inputTree;
	inputTree.listNeuron = inputNodes;
	profiledTree inputProfiledTree(inputTree);
	if (inputProfiledTree.segs.size() > 1 || inputProfiledTree.segs.empty())
	{
		cout << "Input node list constitutes no or multiple segments. Do nothing and return empty list." << endl;
		return;
	}

	QList<NeuronSWC> parents;
	QList<NeuronSWC> children;
	parents.push_back(inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(splittingNodeID)));
	vector<size_t> childLocs;
	vector<ptrdiff_t> delLocs;
	do
	{
		children.clear();
		childLocs.clear();
		for (QList<NeuronSWC>::iterator pasIt = parents.begin(); pasIt != parents.end(); ++pasIt)
		{
			if (inputProfiledTree.node2childLocMap.find(pasIt->n) != inputProfiledTree.node2childLocMap.end()) childLocs = inputProfiledTree.node2childLocMap.at(pasIt->n);
			else continue;

			for (vector<size_t>::iterator childLocIt = childLocs.begin(); childLocIt != childLocs.end(); ++childLocIt)
			{
				downStreamNodes.append(inputNodes.at(int(*childLocIt)));
				delLocs.push_back(ptrdiff_t(*childLocIt));
				children.push_back(inputNodes.at(int(*childLocIt)));
			}
		}
		parents = children;
	} while (childLocs.size() > 0);
	downStreamNodes.push_front(inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(splittingNodeID)));
	delLocs.push_back(inputProfiledTree.node2LocMap.at(splittingNodeID));

	sort(delLocs.rbegin(), delLocs.rend());
	for (auto& loc : delLocs) inputProfiledTree.tree.listNeuron.erase(inputProfiledTree.tree.listNeuron.begin() + loc);
	upStreamNodes = inputProfiledTree.tree.listNeuron;
}

NeuronTree NeuronStructUtil::swcSubtraction(const NeuronTree& targetTree, const NeuronTree& refTree, int type)
{
	boost::container::flat_map<string, QList<NeuronSWC>> targetNodeTileMap;
	boost::container::flat_map<string, QList<NeuronSWC>> refNodeTileMap;
	NeuronStructExplorer::nodeTileMapGen(targetTree, targetNodeTileMap);
	NeuronStructExplorer::nodeTileMapGen(refTree, refNodeTileMap);

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
						{
							delLocs.push_back(ptrdiff_t(checkIt1 - targetTileIt->second.begin()));
							break; // This [break] here is needed, so that [checkIt1] won't be counted multiple times if there are repeated nodes in [refTree].
						}
					}
				}
			}
			else continue;

			std::sort(delLocs.rbegin(), delLocs.rend());
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
						{
							delLocs.push_back(ptrdiff_t(checkIt1 - targetTileIt->second.begin()));
							break; // This [break] here is needed, so that [checkIt1] won't be counted multiple times if there are repeated nodes in [refTree].
						}
					}
				}
			}
			else delLocs.clear();

			std::sort(delLocs.rbegin(), delLocs.rend());
			for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
			{
				if (targetTileIt->second.begin() + *delIt >= targetTileIt->second.end()) continue;
				else targetTileIt->second.erase(targetTileIt->second.begin() + *delIt);
			}
			delLocs.clear();
		}
	}

	NeuronTree outputTree;
	boost::container::flat_set<int> nodeIDs;
	for (boost::container::flat_map<string, QList<NeuronSWC>>::iterator mapIt = targetNodeTileMap.begin(); mapIt != targetNodeTileMap.end(); ++mapIt)
	{
		outputTree.listNeuron.append(mapIt->second);
		for (QList<NeuronSWC>::iterator nodeIt = mapIt->second.begin(); nodeIt != mapIt->second.end(); ++nodeIt)
			nodeIDs.insert(nodeIt->n);
	}

	for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin(); nodeIt != outputTree.listNeuron.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1) continue;
		else 
			if (nodeIDs.find(nodeIt->parent) == nodeIDs.end()) nodeIt->parent = -1;
	}

	return outputTree;
}

bool NeuronStructUtil::isSorted(const NeuronTree& inputNeuronTree) // ~~ Not implemented yet ~~
{
	return true; 
}

NeuronTree NeuronStructUtil::sortTree(const NeuronTree& inputNeuronTree, const float somaCoord[]) // ~~ Not implemented yet ~~
{
	profiledTree inputProfiledTree(inputNeuronTree);



	NeuronTree outputTree;

	return outputTree;
}

NeuronTree NeuronStructUtil::singleDotRemove(const profiledTree& inputProfiledTree, int shortSegRemove)
{
	if (shortSegRemove <= 0) return inputProfiledTree.tree;

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

NeuronTree NeuronStructUtil::longConnCut(const profiledTree& inputProfiledTree, double distThre)
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

NeuronTree NeuronStructUtil::segTerminalize(const profiledTree& inputProfiledTree)
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

void NeuronStructUtil::treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength)
{
	size_t maxNodeID = 0;
	for (QList<NeuronSWC>::const_iterator it = inputProfiledTree.tree.listNeuron.begin(); it != inputProfiledTree.tree.listNeuron.end(); ++it)
		if (it->n > maxNodeID) maxNodeID = it->n;

	for (map<int, segUnit>::const_iterator segIt = inputProfiledTree.segs.begin(); segIt != inputProfiledTree.segs.end(); ++segIt)
	{
		QList<NeuronSWC> newSegNodes;
		map<int, vector<QList<NeuronSWC>>> interpolatedNodeMap;
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end() - 1; ++nodeIt)
		{
			vector<size_t> childLocs = segIt->second.seg_childLocMap.at(nodeIt->n);
			for (vector<size_t>::iterator childLocIt = childLocs.begin(); childLocIt != childLocs.end(); ++childLocIt)
			{
				float dist = sqrt((segIt->second.nodes.at(*childLocIt).x - nodeIt->x) * (segIt->second.nodes.at(*childLocIt).x - nodeIt->x) +
					(segIt->second.nodes.at(*childLocIt).y - nodeIt->y) * (segIt->second.nodes.at(*childLocIt).y - nodeIt->y) +
					(segIt->second.nodes.at(*childLocIt).z - nodeIt->z) * (segIt->second.nodes.at(*childLocIt).z - nodeIt->z));
				int intervals = int(dist / intervalLength);
				float intervalX = (segIt->second.nodes.at(*childLocIt).x - nodeIt->x) / float(intervals);
				float intervalY = (segIt->second.nodes.at(*childLocIt).y - nodeIt->y) / float(intervals);
				float intervalZ = (segIt->second.nodes.at(*childLocIt).z - nodeIt->z) / float(intervals);

				QList<NeuronSWC> interpolatedNodes;
				for (int i = 1; i < intervals; ++i)
				{
					NeuronSWC newNode;
					newNode.x = nodeIt->x + intervalX * float(i);
					newNode.y = nodeIt->y + intervalY * float(i);
					newNode.z = nodeIt->z + intervalZ * float(i);
					newNode.type = nodeIt->type;
					++maxNodeID;
					newNode.n = maxNodeID;
					newNode.parent = maxNodeID - 1;
					interpolatedNodes.push_back(newNode);
				}
				interpolatedNodes.push_back(segIt->second.nodes.at(*childLocIt));
				if (interpolatedNodes.size() >= 2)
				{
					interpolatedNodes.back().parent = (interpolatedNodes.end() - 2)->n;
					interpolatedNodes.begin()->parent = nodeIt->n;
				}

				interpolatedNodeMap[nodeIt->n].push_back(interpolatedNodes);
			}
		}
		for (map<int, vector<QList<NeuronSWC>>>::iterator mapIt = interpolatedNodeMap.begin(); mapIt != interpolatedNodeMap.end(); ++mapIt)
			for (vector<QList<NeuronSWC>>::iterator qlistIt = mapIt->second.begin(); qlistIt != mapIt->second.end(); ++qlistIt) newSegNodes.append(*qlistIt);
		newSegNodes.push_front(segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.head)));

		segUnit newSegUnit;
		newSegUnit.nodes = newSegNodes;
		outputProfiledTree.segs.insert(pair<int, segUnit>(segIt->first, newSegUnit));
		outputProfiledTree.tree.listNeuron.append(newSegNodes);
	}
}

profiledTree NeuronStructUtil::treeDownSample(const profiledTree& inputProfiledTree, int nodeInterval)
{
	// -- This method "down samples" the input tree segment by segment. 
	// -- A recursive down sampling method [NeuronStructExplorer::rc_segDownSample] is called in this function to deal with all possible braching points in each segment.
	// -- NOTE, this method is essentially used for straightening / smoothing segments when there are too many zigzagging.  

	NeuronTree outputTree;
	QList<NeuronSWC> currSegOutputList;
	for (map<int, segUnit>::const_iterator it = inputProfiledTree.segs.begin(); it != inputProfiledTree.segs.end(); ++it)
	{
		//if (it->second.seg_childLocMap.empty()) continue; => Using this line is not safe. Can occasionally result in program crash.
		// The safety of seg_childLocMap needs to be investigated later.
		if (it->second.nodes.size() <= 3) continue;

		currSegOutputList.clear();
		currSegOutputList.push_back(*(it->second.nodes.begin()));
		NeuronStructUtil::rc_segDownSample(it->second, currSegOutputList, it->second.head, nodeInterval);
		outputTree.listNeuron.append(currSegOutputList);
	}

	vector<size_t> delLocs;
	for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin(); nodeIt != outputTree.listNeuron.end(); ++nodeIt)
		if (nodeIt->n == nodeIt->parent) delLocs.push_back(size_t(nodeIt - outputTree.listNeuron.begin()));
	std::sort(delLocs.rbegin(), delLocs.rend());
	for (vector<size_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) outputTree.listNeuron.erase(outputTree.listNeuron.begin() + ptrdiff_t(*delIt));
	profiledTree outputProfiledTree(outputTree);

	return outputProfiledTree;
}

void NeuronStructUtil::rc_segDownSample(const segUnit& inputSeg, QList<NeuronSWC>& outputNodeList, int branchigNodeID, int interval)
{
	int currNodeID = 0, count = 0;
	for (vector<size_t>::const_iterator childIt = inputSeg.seg_childLocMap.at(branchigNodeID).begin(); childIt != inputSeg.seg_childLocMap.at(branchigNodeID).end(); ++childIt)
	{
		outputNodeList.push_back(inputSeg.nodes.at(*childIt));
		outputNodeList.last().parent = branchigNodeID;
		currNodeID = inputSeg.nodes.at(*childIt).n;
		count = 0;
		while (inputSeg.seg_childLocMap.at(currNodeID).size() > 0)
		{
			if (inputSeg.seg_childLocMap.at(currNodeID).size() >= 2) // branching point found, function recursively called
			{
				outputNodeList.push_back(inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(currNodeID)));
				outputNodeList.last().parent = (outputNodeList.end() - 2)->n;
				NeuronStructUtil::rc_segDownSample(inputSeg, outputNodeList, currNodeID, interval);
				break;
			}

			++count;
			currNodeID = inputSeg.nodes.at(*(inputSeg.seg_childLocMap.at(currNodeID).begin())).n;
			if (count % interval == 0 || inputSeg.seg_childLocMap.at(currNodeID).size() == 0) // The tail(s) of the segment needs to stay.
			{
				outputNodeList.push_back(inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(currNodeID)));
				outputNodeList.last().parent = (outputNodeList.end() - 2)->n;
			}
		}
	}
}

QList<NeuronSWC> NeuronStructUtil::V_NeuronSWC2nodeList(const vector<V_NeuronSWC_unit>& inputV_NeuronSWC)
{
	QList<NeuronSWC> outputList;
	for (vector<V_NeuronSWC_unit>::const_iterator it = inputV_NeuronSWC.cbegin(); it != inputV_NeuronSWC.cend(); ++it)
	{
		NeuronSWC newNode;
		newNode.n = it->n;
		newNode.x = it->x;
		newNode.y = it->y;
		newNode.z = it->z;
		newNode.parent = it->parent;
		newNode.type = it->type;
		outputList.push_back(newNode);
	}

	return outputList;
}
/* ===================================== END of [Neuron Struct Processing] ===================================== */



/* ================================== SWC <-> ImgAnalyzer::connectedComponents ================================== */
vector<connectedComponent> NeuronStructUtil::swc2signal2DBlobs(const NeuronTree& inputTree)
{
	// -- Finds signal blobs "slice by slice" from input NeuronTree. Each slice is independent to one another.
	// -- Therefore, the same real blobs in 3D are consists of certain amount of 2D "blob slices" produced by this method. 
	// -- Each 2D blob slice accounts for 1 ImgAnalyzer::connectedComponent.

	vector<NeuronSWC> allNodes;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it) allNodes.push_back(*it);
	bool longList = false;

	vector<connectedComponent> connComps2D;
	int islandCount = 0;
	if (allNodes.size() >= 100000)
	{
		longList = true;
		cout << "number of SWC nodes processed: ";
	}
	for (vector<NeuronSWC>::iterator nodeIt = allNodes.begin(); nodeIt != allNodes.end(); ++nodeIt)
	{
		if (longList)
		{
			if (int(nodeIt - allNodes.begin()) % 10000 == 0) cout << int(nodeIt - allNodes.begin()) << " ";
		}

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
	//cout << endl << endl;

	vector<float> center(3);
	for (vector<connectedComponent>::iterator it = connComps2D.begin(); it != connComps2D.end(); ++it)
		ChebyshevCenter_connComp(*it);

	return connComps2D;
}

vector<connectedComponent> NeuronStructUtil::swc2signal3DBlobs(const NeuronTree& inputTree)
{
	// -- This method is a wrapper of NeuronStructUtil::swc2signal2DBlobs and NeuronStructUtil::merge2DConnComponent.
	// -- It produces 3D signal blobs by calling the two swc2signal2DBlobs and merge2DConnComponent sequentially.

	vector<connectedComponent> inputConnCompList = NeuronStructUtil::swc2signal2DBlobs(inputTree);
	vector<connectedComponent> outputConnCompList = NeuronStructUtil::merge2DConnComponent(inputConnCompList);

	return outputConnCompList;
}

vector<connectedComponent> NeuronStructUtil::merge2DConnComponent(const vector<connectedComponent>& inputConnCompList)
{
	// -- This method finds 3D signal blobs by grouping 2D signal blobs together, which are generated by NeuronStructUtil::swc2signal2DBlobs.
	// -- This method is typically called by NeuronStructUtil::swc2signal2DBlobs when identifying 3D blobs from 2D ones.
	// -- The approach is consists of 2 stages:
	//		1. Identifying the same 3D blobs slice by slice.
	//		2. Merging 3D blobs that contain the same 2D blobs.

#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";
#endif

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
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
			cout << i << "->0 ";
#endif
			continue;
		}

#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
		cout << i << "->";
#endif
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
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
		cout << increasedSize << ", ";
#endif
	}
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
	cout << endl;
	cout << "Done merging 2D blobs from every 2 slices." << endl;
#endif
	// ---------------------------------------- END of [Merge 2D blobs from 2 adjacent slices] -------------------------------------------

	// ------------------------------------------ Merge 3D blobs --------------------------------------------
	// Merge any 3D blobs if any of them share the same 2D blob members.
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
	cout << "Now merging 3D blobs.." << endl;
	cout << " -- original 3D blobs number: " << b3Dcomps.size() << endl;
#endif
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
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
							cout << "  merging blob " << checkIt1->first << " and blob " << checkIt2->first << endl;
#endif
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
#ifdef __MERGE2DCONNCOMPONENT_PRINTOUT__
	cout << " -- new 3D blobs number: " << b3Dcomps.size() << endl;
	cout << "    ------------------------------" << endl << endl;
#endif
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
		newComp.xMax = 0; newComp.xMin = 1000000;
		newComp.yMax = 0; newComp.yMin = 1000000;
		newComp.zMax = 0; newComp.zMin = 1000000;
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) // *it2 = 2D connected component's [islandNum]
		{
			// A 3D connected component may contain different 2D components from the same slice.
			if (newComp.coordSets.find(compsMap.at(*it2).coordSets.begin()->first) != newComp.coordSets.end())
			{
				for (set<vector<int>>::iterator it3 = compsMap.at(*it2).coordSets.begin()->second.begin(); it3 != compsMap.at(*it2).coordSets.begin()->second.end(); ++it3)
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
/* =============================== END of [SWC <-> ImgAnalyzer::connectedComponents] =============================== */



/* ==================================== Artificial SWC for Developing Purposes ===================================== */
NeuronTree NeuronStructUtil::nodeSpheresGen(float sphereRadius, float density, float stepX, float stepY, float stepZ, float xRange, float yRange, float zRange)
{
	NeuronTree outputTree;
	for (float x = 0; x <= xRange; x += stepX)
	{
		for (float y = 0; y <= yRange; y += stepY)
		{
			for (float z = 0; z <= zRange; z += stepZ)
			{
				NeuronTree currSphereTree = NeuronStructUtil::sphereRandNodes(sphereRadius, x, y, z, density);
				size_t existingNodeNum = outputTree.listNeuron.size();
				for (QList<NeuronSWC>::iterator it = currSphereTree.listNeuron.begin(); it != currSphereTree.listNeuron.end(); ++it) it->n = it->n + existingNodeNum;
				outputTree.listNeuron.append(currSphereTree.listNeuron);
			}
		}
	}

	return outputTree;
}
/* ================================ END of [Artificial SWC for Developing Purposes] ================================ */



/* =========================================== Miscellaneous ============================================ */
NeuronTree NeuronStructUtil::convertHUSTswc(QString inputFileNameQ)
{
	NeuronTree outputTree;
	string inputName = inputFileNameQ.toStdString();
	string line, element;
	ifstream inFile(inputName);
	vector<string> lineSplit;
	if (inFile.is_open())
	{
		while (getline(inFile, line))
		{
			stringstream ss(line);
			while (ss >> element) lineSplit.push_back(element);
			//for (auto& ele : lineSplit) cout << ele << " ";
			//cout << endl;

			NeuronSWC newNode;
			newNode.n = stoi(lineSplit.at(0));
			newNode.type = stoi(lineSplit.at(1));
			newNode.x = stof(lineSplit.at(4));
			newNode.y = stof(lineSplit.at(3));
			newNode.z = 11400 - stof(lineSplit.at(2));
			newNode.parent = stoi(lineSplit.at(6));
			newNode.radius = stof(lineSplit.at(5));
			if (newNode.n == 0 || newNode.type == 0 || newNode.parent == 0)
			{
				lineSplit.clear();
				continue;
			}
			outputTree.listNeuron.append(newNode);

			lineSplit.clear();
		}
	}
	
	return outputTree;
}

vector<NeuronTree> NeuronStructUtil::convertHUSTswc_old(QString inputQ)
{
	vector<NeuronTree> outputTrees;

	QFileInfo inputPathInfo(inputQ);
	if (inputPathInfo.isFile())
	{
		NeuronTree outputTree;
		string inputName = inputQ.toStdString();		
		string line, element;
		ifstream inFile(inputName);
		vector<string> lineSplit;
		if (inFile.is_open())
		{
			while (getline(inFile, line))
			{
				stringstream ss(line);
				while (ss >> element) lineSplit.push_back(element);
				//for (auto& ele : lineSplit) cout << ele << " ";
				//cout << endl;

				NeuronSWC newNode;
				newNode.n = stoi(lineSplit.at(0));
				newNode.type = stoi(lineSplit.at(1));
				newNode.x = stof(lineSplit.at(4));
				newNode.y = stof(lineSplit.at(2)) - 500;
				newNode.z = 11400 - stof(lineSplit.at(3));
				newNode.parent = stoi(lineSplit.at(6));
				newNode.radius = stof(lineSplit.at(5));
				if (newNode.n == 0 || newNode.type == 0 || newNode.parent == 0) continue;
				outputTree.listNeuron.append(newNode);

				lineSplit.clear();
			}
		}
		outputTrees.push_back(outputTree);
	}
	else if (inputPathInfo.isDir())
	{
		QDir inputFolderQ(inputQ);
		inputFolderQ.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		QStringList fileNameListQ = inputFolderQ.entryList();
		for (auto& fileNameQ : fileNameListQ)
		{
			NeuronTree outputTree;
			string inputName = inputQ.toStdString() + "\\" + fileNameQ.toStdString();
			string line, element;
			ifstream inFile(inputName);
			vector<string> lineSplit;
			if (inFile.is_open())
			{
				while (getline(inFile, line))
				{
					stringstream ss(line);
					while (ss >> element) lineSplit.push_back(element);

					NeuronSWC newNode;
					newNode.n = stoi(lineSplit.at(0));
					newNode.type = stoi(lineSplit.at(1));
					newNode.x = stof(lineSplit.at(4));
					newNode.y = stof(lineSplit.at(2)) - 500;
					newNode.z = 11400 - stof(lineSplit.at(3));
					newNode.parent = stoi(lineSplit.at(6));
					newNode.radius = stof(lineSplit.at(5));
					if (newNode.n == 0 || newNode.type == 0 || newNode.parent == 0) continue;
					outputTree.listNeuron.append(newNode);

					lineSplit.clear();
				}
			}
			outputTrees.push_back(outputTree);
		}
	}

	return outputTrees;
}
/* ======================================== END of [Miscellaneous] ======================================== */



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
/* =================================== Volumetric SWC sampling methods =================================== */