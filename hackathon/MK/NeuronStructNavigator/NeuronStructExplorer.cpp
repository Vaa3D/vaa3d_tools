//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This library intends to provide functionalities for neuron struct analysis, including graph and geometry analysis, etc.
*  Typically NeuronStructExplorer class methods need a profiledTree struct as part of the input arguments. A profiledTree can be assigned when the class is initiated or later.
*
*  profiledTree is the core data type in NeuronStructExplorer class. It profiles the NeuronTree and carries crucial information.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

*  The class can be initialized with or without a profiledTree being initialized at the same time. A profiledTree can be stored and indexed in NeuronStructExplorer's treeDatabe.
*
********************************************************************************/

#include <iostream>
#include <algorithm>
#include <cmath>

#include <boost/container/flat_map.hpp>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"

using namespace std;

profiledTree::profiledTree(const NeuronTree& inputTree, float segTileLength)
{
	this->tree = inputTree;
	this->segTileSize = segTileLength;
	this->nodeTileSize = NODE_TILE_LENGTH;

	NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileSize);

	NeuronStructUtil::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);
	
	this->segs = NeuronStructExplorer::findSegs(this->tree.listNeuron, this->node2childLocMap);
	//cout << "segs num: " << this->segs.size() << endl;
	vector<segUnit> allSegs;
	for (map<int, segUnit>::iterator it = this->segs.begin(); it != this->segs.end(); ++it)
	{	
		//if (it->second.tails.size() > 1) cout << " branching seg: " << it->first << endl;
		allSegs.push_back(it->second);
	}
	this->segHeadMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength);
	this->segTailMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength, false);
}

void profiledTree::nodeTileResize(float nodeTileLength)
{
	if (nodeTileLength == NODE_TILE_LENGTH) return;
	else
	{
		if (!this->nodeTileMap.empty())
		{
			this->nodeTileMap.clear();
			NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		}
		else
		{
			this->nodeTileSize = nodeTileLength;
			return;
		}
	}
}

void profiledTree::addTopoUnit(int nodeID)
{
	if (this->node2LocMap.empty() || this->node2childLocMap.empty())
	{
		cerr << "The tree profile hasn't been initialized yet. Do 'profiledTree(NeuronTree)' before calling to this method.\n Do nothing and return." << endl;
		return;
	}

	topoCharacter topoUnit(this->tree.listNeuron.at(this->node2LocMap.at(nodeID)));
	topoUnit.upstream.clear();
	topoUnit.downstreams.clear();
	
	int parentID = topoUnit.topoCenter.parent;
	while (topoUnit.upstream.size() <= 10)
	{
		if (this->node2LocMap.find(parentID) == this->node2LocMap.end()) break;
		topoUnit.upstream.push_back(this->tree.listNeuron.at(this->node2LocMap.at(parentID)));
		parentID = topoUnit.upstream.back().parent;
		if (parentID == -1) break;
	}

	for (vector<size_t>::iterator it = this->node2childLocMap.at(nodeID).begin(); it != this->node2childLocMap.at(nodeID).end(); ++it)
	{
		deque<NeuronSWC> downstream;
		int childID = this->tree.listNeuron.at(*it).n;
		while (downstream.size() <= 10)
		{
			if (this->node2LocMap.find(childID) == this->node2LocMap.end()) break;
			downstream.push_back(this->tree.listNeuron.at(this->node2LocMap.at(childID)));
			cout << downstream.back().n << " ";
			childID = this->tree.listNeuron.at(*node2childLocMap.at(childID).begin()).n;
		}
		topoUnit.downstreams.insert(pair<int, deque<NeuronSWC>>(downstream.begin()->n, downstream));
	}

	this->topoList.insert(pair<int, topoCharacter>(nodeID, topoUnit));
}

NeuronStructExplorer::NeuronStructExplorer(QString inputFileName)
{
	QStringList fileNameParse1 = inputFileName.split(".");
	if (fileNameParse1.back() == "swc")
	{
		NeuronTree inputSWC = readSWC_file(inputFileName);
		QStringList fileNameParse2 = fileNameParse1.at(0).split("\\");
		string treeName = fileNameParse2.back().toStdString();
		this->treeEntry(inputSWC, treeName);
	}
}

void NeuronStructExplorer::treeEntry(const NeuronTree& inputTree, string treeName, float segTileLength)
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

void NeuronStructExplorer::profiledTreeReInit(profiledTree& inputProfiledTree)
{
	profiledTree tempTree(inputProfiledTree.tree);
	inputProfiledTree = tempTree;
}

map<int, segUnit> NeuronStructExplorer::findSegs(const QList<NeuronSWC>& inputNodeList, const map<int, vector<size_t>>& node2childLocMap)
{
	// -- This method profiles all segments in a given input tree.

	map<int, segUnit> segs;
	vector<NeuronSWC> inputNodes;
	for (QList<NeuronSWC>::const_iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
		inputNodes.push_back(*nodeIt);

	int segCount = 0;
	for (vector<NeuronSWC>::iterator nodeIt = inputNodes.begin(); nodeIt != inputNodes.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1)
		{
			// ------------ Identify segments ------------
			++segCount;
			//cout << " processing segment " << segCount << "..  head node ID: " << nodeIt->n << endl << "  ";
			segUnit newSeg;
			newSeg.segID = segCount;
			newSeg.head = nodeIt->n;
			newSeg.nodes.push_back(*nodeIt);
			vector<size_t> childLocs = node2childLocMap.find(nodeIt->n)->second;
				// In NeuronStructUtilities::node2loc_node2childLocMap, 
				// if a segment is only with a head, its childLocs will also be assigned as empty set.
			if (childLocs.empty()) 
			{
				newSeg.tails.push_back(nodeIt->n);
				newSeg.seg_nodeLocMap.insert(pair<int, size_t>(nodeIt->n, 0));
				segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
				continue;
			}

			vector<size_t> grandChildLocs;
			vector<size_t> thisGrandChildLocs;
			bool childNum = true;
			while (childNum)
			{
				grandChildLocs.clear();
				for (vector<size_t>::iterator childIt = childLocs.begin(); childIt != childLocs.end(); ++childIt)
				{
					//cout << inputNodes.at(*childIt).n << " ";
					thisGrandChildLocs.clear();
					for (QList<NeuronSWC>::iterator checkIt = newSeg.nodes.begin(); checkIt != newSeg.nodes.end(); ++checkIt)
						if (checkIt->n == inputNodes.at(*childIt).n) goto LOOP;

					newSeg.nodes.push_back(inputNodes.at(*childIt));
					if (node2childLocMap.find(inputNodes.at(*childIt).n) == node2childLocMap.end())
					{
							newSeg.tails.push_back(inputNodeList[*childIt].n);
							//cout << "<-tail ";
					}
					else
					{
						thisGrandChildLocs = node2childLocMap.at(inputNodes.at(*childIt).n);
						grandChildLocs.insert(grandChildLocs.end(), thisGrandChildLocs.begin(), thisGrandChildLocs.end());
					}

				LOOP:
					continue;
				}
				if (grandChildLocs.size() == 0) childNum = false;

				childLocs = grandChildLocs;
			}
			//cout << endl; 
			// --------- END of [Identify segments] ---------

			// ------------- Identify node/child location in the segment -------------
			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it) 
				newSeg.seg_nodeLocMap.insert(pair<int, size_t>(it->n, size_t(it - newSeg.nodes.begin())));

			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it)
			{
				vector<size_t> childSegLocs;
				childSegLocs.clear();
				if (newSeg.seg_childLocMap.find(it->n) == newSeg.seg_childLocMap.end())
					newSeg.seg_childLocMap.insert(pair<int, vector<size_t>>(it->n, childSegLocs));
				// This ensures that tail node still has its own pair in seg_chileLocMap, and its childSegLocs is empty.

				if (node2childLocMap.find(it->n) != node2childLocMap.end())
				{
					vector<size_t> childLocs = node2childLocMap.at(it->n);
					for (vector<size_t>::iterator globalChildLocIt = childLocs.begin(); globalChildLocIt != childLocs.end(); ++globalChildLocIt)
					{
						size_t childSegLoc = newSeg.seg_nodeLocMap[inputNodeList.at(*globalChildLocIt).n];
						newSeg.seg_childLocMap[it->n].push_back(childSegLoc);
					}
				}
			}
			segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
			// --------- END of [Identify node/child location in the segment] ---------
		}
	}

	return segs;
}

map<string, vector<int>> NeuronStructExplorer::segTileMap(const vector<segUnit>& inputSegs, float xyLength, bool head)
{
	if (head)
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			string xLabel = to_string(int(it->nodes.begin()->x / xyLength));
			string yLabel = to_string(int(it->nodes.begin()->y / xyLength));
			string zLabel = to_string(int(it->nodes.begin()->z / (xyLength / zRATIO)));
			string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
			if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
			else
			{
				vector<int> newSet;
				newSet.push_back(it->segID);
				outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
			}
		}
		return outputSegTileMap;
	}
	else
	{
		map<string, vector<int>> outputSegTileMap;
		for (vector<segUnit>::const_iterator it = inputSegs.begin(); it != inputSegs.end(); ++it)
		{
			for (vector<int>::const_iterator tailIt = it->tails.begin(); tailIt != it->tails.end(); ++tailIt)
			{
				NeuronSWC tailNode = it->nodes.at(it->seg_nodeLocMap.at(*tailIt));
				string xLabel = to_string(int(tailNode.x / xyLength));
				string yLabel = to_string(int(tailNode.y / xyLength));
				string zLabel = to_string(int(tailNode.z / (xyLength / zRATIO)));
				string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
				if (outputSegTileMap.find(keyLabel) != outputSegTileMap.end()) outputSegTileMap[keyLabel].push_back(it->segID);
				else
				{
					vector<int> newSet;
					newSet.push_back(it->segID);
					outputSegTileMap.insert(pair<string, vector<int>>(keyLabel, newSet));
				}
			}
		}
		return outputSegTileMap;
	}
}

void NeuronStructExplorer::treeUpSample(const profiledTree& inputProfiledTree, profiledTree& outputProfiledTree, float intervalLength)
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


// ========================================= Segment Analysis / Operations =========================================
segUnit NeuronStructExplorer::segUnitConnect_executer(const segUnit& segUnit1, const segUnit& segUnit2, connectOrientation connOrt, NeuronSWC* tailNodePtr1, NeuronSWC* tailNodePtr2)
{
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
			newSegNodes.append(segUnit1.nodes);
			newSegNodes.begin()->parent = connTailID;
			newSegNodes.append(endEditedNodes);
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

map<int, segUnit> NeuronStructExplorer::segRegionConnector_angle(const vector<int>& currTileHeadSegIDs, const vector<int>& currTileTailSegIDs, profiledTree& currProfiledTree, double angleThre, bool length)
{
	set<int> connectedSegs;
	map<int, int> elongConnMap;
	map<string, double> segAngleMap;
	map<int, segUnit> newSegs;

	//__________________________________________________________________________________________________________________________
	
	//cout << "------- head-tail:" << endl;
	for (vector<int>::const_iterator headIt = currTileHeadSegIDs.begin(); headIt != currTileHeadSegIDs.end(); ++headIt)
	{
		for (vector<int>::const_iterator tailIt = currTileTailSegIDs.begin(); tailIt != currTileTailSegIDs.end(); ++tailIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt).to_be_deleted || currProfiledTree.segs.at(*tailIt).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt), currProfiledTree.segs.at(*tailIt), head_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *headIt << "_" << *tailIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt) + "_" + to_string(*tailIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- tail-head:" << endl;
	for (vector<int>::const_iterator tailIt = currTileHeadSegIDs.begin(); tailIt != currTileHeadSegIDs.end(); ++tailIt)
	{
		for (vector<int>::const_iterator headIt = currTileTailSegIDs.begin(); headIt != currTileTailSegIDs.end(); ++headIt)
		{
			if (*headIt == *tailIt) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt).to_be_deleted || currProfiledTree.segs.at(*headIt).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt), currProfiledTree.segs.at(*headIt), tail_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *tailIt << "_" << *headIt << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt) + "_" + to_string(*headIt);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- head-head:" << endl;
	for (vector<int>::const_iterator headIt1 = currTileHeadSegIDs.begin(); headIt1 != currTileHeadSegIDs.end(); ++headIt1)
	{
		for (vector<int>::const_iterator headIt2 = currTileHeadSegIDs.begin(); headIt2 != currTileHeadSegIDs.end(); ++headIt2)
		{
			if (*headIt1 == *headIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*headIt1).to_be_deleted || currProfiledTree.segs.at(*headIt2).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*headIt1), currProfiledTree.segs.at(*headIt2), head_head);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *headIt1 << "_" << *headIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*headIt1) + "_" + to_string(*headIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], head_head);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl;
	//__________________________________________________________________________________________________________________________

	//cout << "------- tail-tail:" << endl;
	for (vector<int>::const_iterator tailIt1 = currTileHeadSegIDs.begin(); tailIt1 != currTileHeadSegIDs.end(); ++tailIt1)
	{
		for (vector<int>::const_iterator tailIt2 = currTileTailSegIDs.begin(); tailIt2 != currTileTailSegIDs.end(); ++tailIt2)
		{
			if (*tailIt1 == *tailIt2) continue;
			else
			{
				if (currProfiledTree.segs.at(*tailIt1).to_be_deleted || currProfiledTree.segs.at(*tailIt2).to_be_deleted) continue;

				double pointingRadAngle = this->segPointingCompare(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				double turningRadAngle = this->segTurningAngle(currProfiledTree.segs.at(*tailIt1), currProfiledTree.segs.at(*tailIt2), tail_tail);
				if (pointingRadAngle == -1 || turningRadAngle == -1) continue;

				if (pointingRadAngle < angleThre && turningRadAngle < angleThre)
				{
					//cout << *tailIt1 << "_" << *tailIt2 << "->(" << pointingRadAngle << "," << turningRadAngle << ") ";
					string segAngleKey = to_string(*tailIt1) + "_" + to_string(*tailIt2);
					segAngleMap.insert(pair<string, double>(segAngleKey, (pointingRadAngle + turningRadAngle)));
				}
			}
		}
	}
	this->tileSegConnOrganizer_angle(segAngleMap, connectedSegs, elongConnMap);
	for (map<int, int>::iterator it = elongConnMap.begin(); it != elongConnMap.end(); ++it)
	{
		currProfiledTree.segs[it->first].to_be_deleted = true;
		currProfiledTree.segs[it->second].to_be_deleted = true;
		segUnit newSeg = this->segUnitConnect_executer(currProfiledTree.segs[it->first], currProfiledTree.segs[it->second], tail_tail);
		newSeg.segID = currProfiledTree.segs.size() + 1;
		while (currProfiledTree.segs.find(newSeg.segID) != currProfiledTree.segs.end()) ++newSeg.segID;
		currProfiledTree.segs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
		newSegs.insert(pair<int, segUnit>(newSeg.segID, newSeg));
	}
	segAngleMap.clear();
	elongConnMap.clear();
	//cout << endl << endl;
	//__________________________________________________________________________________________________________________________

	return newSegs;
}

profiledTree NeuronStructExplorer::segElongate(const profiledTree& inputProfiledTree, double angleThre)
{
	//cout << inputProfiledTree.segHeadMap.size() << " " << inputProfiledTree.segTailMap.size() << endl;

	profiledTree outputProfiledTree = inputProfiledTree;
	QList<NeuronSWC> newNodeList = inputProfiledTree.tree.listNeuron;
	map<int, segUnit> allNewSegs;
	
	set<string> allTileKeys;
	for (map<string, vector<int>>::const_iterator headTileIt = inputProfiledTree.segHeadMap.begin(); headTileIt != inputProfiledTree.segHeadMap.end(); ++headTileIt)
		allTileKeys.insert(headTileIt->first);
	for (map<string, vector<int>>::const_iterator tailTileIt = inputProfiledTree.segTailMap.begin(); tailTileIt != inputProfiledTree.segTailMap.end(); ++tailTileIt)
		allTileKeys.insert(tailTileIt->first);
	
	vector<int> currTileHeadSegIDs;
	vector<int> currTileTailSegIDs;
	set<int> connectedSegs;
	for (set<string>::iterator keyIt = allTileKeys.begin(); keyIt != allTileKeys.end(); ++keyIt)
	{
		//cout << "TILE " << *keyIt << " =======" << endl;
		//cout << " - Heads: ";
		if (inputProfiledTree.segHeadMap.find(*keyIt) != inputProfiledTree.segHeadMap.end())
		{
			for (vector<int>::const_iterator headIt = inputProfiledTree.segHeadMap.at(*keyIt).begin(); headIt != inputProfiledTree.segHeadMap.at(*keyIt).end(); ++headIt)
			{
				//cout << *headIt << " ";
				currTileHeadSegIDs.push_back(*headIt);
			}
		}
		//cout << endl;
		//cout << " - Tails: ";
		if (inputProfiledTree.segTailMap.find(*keyIt) != inputProfiledTree.segTailMap.end())
		{
			for (vector<int>::const_iterator tailIt = inputProfiledTree.segTailMap.at(*keyIt).begin(); tailIt != inputProfiledTree.segTailMap.at(*keyIt).end(); ++tailIt)
			{
				//cout << *tailIt << " ";
				currTileTailSegIDs.push_back(*tailIt);
			}
		}
		//cout << endl;
		if (currTileHeadSegIDs.size() + currTileTailSegIDs.size() <= 1) // If there is <= 1 head or tail (only 1 terminal) in the tile, no further process is needed.
		{
			currTileHeadSegIDs.clear();
			currTileTailSegIDs.clear();
			//cout << endl;
			continue;
		}

		map<int, segUnit> newSegs = this->segRegionConnector_angle(currTileHeadSegIDs, currTileTailSegIDs, outputProfiledTree, angleThre);
		for (map<int, segUnit>::iterator newSegIt = newSegs.begin(); newSegIt != newSegs.end(); ++newSegIt) 
			allNewSegs.insert(pair<int, segUnit>(newSegIt->first, newSegIt->second));

		currTileHeadSegIDs.clear();
		currTileTailSegIDs.clear();
	}

	vector<size_t> nodeDeleteLocs;
	bool segDeleted = true;
	while (segDeleted)
	{
		for (map<int, segUnit>::iterator it = outputProfiledTree.segs.begin(); it != outputProfiledTree.segs.end(); ++it)
		{
			if (it->second.to_be_deleted)
			{
				//cout << it->first << ": " << it->second.nodes.size() << endl;
				for (QList<NeuronSWC>::iterator nodeIt = it->second.nodes.begin(); nodeIt != it->second.nodes.end(); ++nodeIt)
					nodeDeleteLocs.push_back(outputProfiledTree.node2LocMap.at(nodeIt->n));
				outputProfiledTree.segs.erase(it);

				goto SEG_DELETED;
			}
		}
		segDeleted = false;

	SEG_DELETED:
		continue;
	}
	//cout << endl;

	sort(nodeDeleteLocs.rbegin(), nodeDeleteLocs.rend());
	for (vector<size_t>::iterator it = nodeDeleteLocs.begin(); it != nodeDeleteLocs.end(); ++it) 
		outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*it));

	//cout << endl << "NEW SEGMENTS ------------- " << endl;
	for (map<int, segUnit>::iterator it = allNewSegs.begin(); it != allNewSegs.end(); ++it)
	{
		//cout << it->first << ": " << it->second.nodes.size() << endl;
		outputProfiledTree.tree.listNeuron.append(it->second.nodes);
	}

	this->profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}

profiledTree NeuronStructExplorer::itered_segElongate(profiledTree& inputProfiledTree, double angleThre)
{
	cout << "iteration 1 ";
	int iterCount = 1;
	profiledTree elongatedTree = this->segElongate(inputProfiledTree, angleThre);
	while (elongatedTree.segs.size() != inputProfiledTree.segs.size())
	{
		++iterCount;
		cout << "iterator " << iterCount << " " << endl;
		inputProfiledTree = elongatedTree;
		elongatedTree = this->segElongate(inputProfiledTree, angleThre);
	}
	cout << endl;

	return elongatedTree;
}

profiledTree NeuronStructExplorer::treeUnion_MSTbased(const profiledTree& expandingPart, const profiledTree& baseTree)
{
	set<int> connectedSegs;
	profiledTree outputProfiledTree = baseTree;

	for (map<int, segUnit>::const_iterator segIt = expandingPart.segs.begin(); segIt != expandingPart.segs.end(); ++segIt)
	{
		NeuronSWC headNode = *segIt->second.nodes.begin();
		string xLabel = to_string(int(headNode.x / tileXY_LENGTH));
		string yLabel = to_string(int(headNode.y / tileXY_LENGTH));
		string zLabel = to_string(int(headNode.z / (tileXY_LENGTH / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		//cout << keyLabel << ": ";

		vector<int> baseSegHeads;
		vector<int> baseSegTails;
		vector<int> baseSegs;
		if (baseTree.segHeadMap.find(keyLabel) != baseTree.segHeadMap.end()) baseSegHeads = baseTree.segHeadMap.at(keyLabel);
		if (baseTree.segTailMap.find(keyLabel) != baseTree.segTailMap.end()) baseSegTails = baseTree.segTailMap.at(keyLabel);
		baseSegs = baseSegHeads;
		for (vector<int>::iterator tileSegIt = baseSegTails.begin(); tileSegIt != baseSegTails.end(); ++tileSegIt)
		{
			if (find(baseSegHeads.begin(), baseSegHeads.end(), *tileSegIt) == baseSegs.end())
				baseSegs.push_back(*tileSegIt);
		}
		//for (vector<int>::iterator checkSegIt = baseSegs.begin(); checkSegIt != baseSegs.end(); ++checkSegIt) cout << *checkSegIt << " ";
		//cout << endl;

		boost::container::flat_map<int, connectOrientation> connRoutingMap;
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			for (vector<int>::iterator baseSegIt = baseSegs.begin(); baseSegIt != baseSegs.end(); ++baseSegIt)
			{
				NeuronSWC baseSegHeadNode = *baseTree.segs.at(*baseSegIt).nodes.begin();
				if (nodeIt->x == baseSegHeadNode.x && nodeIt->y == baseSegHeadNode.y && nodeIt->z == baseSegHeadNode.z)
				{
					connRoutingMap.insert(pair<int, connectOrientation>(*baseSegIt, head));
					goto SEGMENT_CONFIRMED;
				}

				for (vector<int>::const_iterator tailIt = baseTree.segs.at(*baseSegIt).tails.begin(); tailIt != baseTree.segs.at(*baseSegIt).tails.end(); ++tailIt)
				{
					NeuronSWC baseSegTailNode = baseTree.segs.at(*baseSegIt).nodes.at(baseTree.segs.at(*baseSegIt).seg_nodeLocMap.at(*tailIt));
					if (nodeIt->x == baseSegTailNode.x && nodeIt->y == baseSegTailNode.y && nodeIt->z == baseSegTailNode.z)
					{
						connRoutingMap.insert(pair<int, connectOrientation>(*baseSegIt, tail));
						goto SEGMENT_CONFIRMED;
					}
				}
			}

		SEGMENT_CONFIRMED:
			continue;
		}

		if (connRoutingMap.size() == 0) continue;

		int segID1 = baseTree.segs.at(connRoutingMap.begin()->first).segID;
		connectOrientation ort1 = connRoutingMap.begin()->second;
		for (boost::container::flat_map<int, connectOrientation>::iterator connIt = connRoutingMap.begin() + 1; connIt != connRoutingMap.end(); ++connIt)
		{
			int segID2 = connIt->first;
			connectOrientation ort2 = connIt->second;
			if (segID1 == segID2)
			{
				segID1 = segID2;
				ort1 = ort2;
				continue;
			}

			if (ort1 == head && ort2 == head)
			{
				int nodeID1 = outputProfiledTree.segs.at(segID1).head;
				int nodeID2 = outputProfiledTree.segs.at(segID2).head;
				size_t nodeLoc1 = outputProfiledTree.node2LocMap.at(nodeID1);
				outputProfiledTree.tree.listNeuron[nodeLoc1].parent = nodeID2;
			}
			else if (ort1 == head && ort2 == tail)
			{
				int nodeID1 = outputProfiledTree.segs.at(segID1).head;
				int nodeID2 = *outputProfiledTree.segs.at(segID2).tails.begin();
				size_t nodeLoc1 = outputProfiledTree.node2LocMap.at(nodeID1);
				outputProfiledTree.tree.listNeuron[nodeLoc1].parent = nodeID2;
			}
			else if (ort1 == tail && ort2 == head)
			{
				int nodeID1 = *outputProfiledTree.segs.at(segID1).tails.begin();
				int nodeID2 = outputProfiledTree.segs.at(segID2).head;
				size_t nodeLoc2 = outputProfiledTree.node2LocMap.at(nodeID2);
				outputProfiledTree.tree.listNeuron[nodeLoc2].parent = nodeID1;
			}
		}
	}

	return outputProfiledTree;
}
// ===================================== END of [Segment Analysis / Operations] =====================================



// ================================================== Geometry =================================================
double NeuronStructExplorer::segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> connHeadLoc;
	connHeadLoc.push_back(connHeadNode.x);
	connHeadLoc.push_back(connHeadNode.y);
	connHeadLoc.push_back(connHeadNode.z * zRATIO);

	vector<float> connTailLoc;
	connTailLoc.push_back(connTailNode.x);
	connTailLoc.push_back(connTailNode.y);
	connTailLoc.push_back(connTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connDispUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connTailLoc, connHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		connDispUnitVec = this->getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
}

double NeuronStructExplorer::segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connPointUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = this->getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = this->getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = this->getRadAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
}

segUnit NeuronStructExplorer::segmentStraighten(const segUnit& inputSeg)
{
	segUnit outputSeg = inputSeg;

	if (inputSeg.nodes.size() <= 3) return outputSeg;

	cout << " seg ID: " << inputSeg.segID << "   number of nodes contained in this segment: " << inputSeg.nodes.size() << endl;

	double segDistSqr = ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) + 
		((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
		((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
	double segDist = sqrt(segDistSqr);
	cout << "segment displacement: " << segDist << endl << endl;
	
	for (QList<NeuronSWC>::const_iterator check = inputSeg.nodes.begin(); check != inputSeg.nodes.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;

	vector<profiledNode> pickedNode;
	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostSum = 0, turnCostMean;
	double nodeDeviation, nodeHeadDistSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistSum = 0, nodeToMainDistMean;
	for (QList<NeuronSWC>::const_iterator it = inputSeg.nodes.begin() + 1; it != inputSeg.nodes.end() - 1; ++it)
	{
		dot = ((it - 1)->x - it->x) * ((it + 1)->x - it->x) + ((it - 1)->y - it->y) * ((it + 1)->y - it->y) + ((it - 1)->z - it->z) * ((it + 1)->z - it->z);
		sq1 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
		sq2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
		if (isnan(acos(dot / sqrt(sq1 * sq2)))) return outputSeg;
		radAngle = acos(dot / sqrt(sq1 * sq2));
		
		nodeDeviation = (it->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) +
			(it->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
			(it->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
		nodeHeadDistSqr = (it->x - inputSeg.nodes.begin()->x) * (it->x - inputSeg.nodes.begin()->x) + 
			(it->y - inputSeg.nodes.begin()->y) * (it->y - inputSeg.nodes.begin()->y) + 
			(it->z - inputSeg.nodes.begin()->z) * (it->z - inputSeg.nodes.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeDeviation / sqrt(segDistSqr * nodeHeadDistSqr));
		nodeToMainDist = sqrt(nodeHeadDistSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistSum = nodeToMainDistSum + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle / PI) << " turning cost:" << (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI) << " " << it->x << " " << it->y << " " << it->z << endl;

		if ((radAngle / PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			profiledNode sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;

			sharp.segID = inputSeg.segID;
			sharp.distToMainRoute = nodeToMainDist;
			sharp.previousSqr = sq1; sharp.nextSqr = sq2; sharp.innerProduct = dot;
			sharp.radAngle = radAngle;
			sharp.index = int(it - inputSeg.nodes.begin());
			sharp.turnCost = (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI);

			pickedNode.push_back(sharp);
			turnCostSum = turnCostSum + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return outputSeg;

	nodeToMainDistMean = nodeToMainDistSum / (inputSeg.nodes.size() - 2);
	turnCostMean = turnCostSum / pickedNode.size();

	cout << endl << endl << "  ==== start deleting nodes... " << endl;
	int delete_count = 0;
	vector<int> delLocs;
	for (vector<profiledNode>::iterator it = pickedNode.begin(); it != pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMainRoute << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMainRoute >= nodeToMainDistMean || it->turnCost >= turnCostMean || it->distToMainRoute >= segDist)
		{
			outputSeg.nodes[it->index + 1].parent = -1;
			delLocs.push_back(it->index);
		}
	}
	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
	delLocs.clear();

	cout << endl << "  ==== cheking angles... " << endl;
	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
		{
			double dot2 = (it->x - (it - 1)->x) * ((it + 1)->x - it->x) + (it->y - (it - 1)->y) * ((it + 1)->y - it->y) + (it->z - (it - 1)->z) * ((it + 1)->z - it->z);
			double sq1_2 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
			double sq2_2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
			if (isnan(acos(dot2 / sqrt(sq1_2 * sq2_2)))) break;
			double radAngle_2 = acos(dot2 / sqrt(sq1_2 * sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;

			if ((radAngle_2 / PI) * 180 > 75)
			{
				if (sqrt(sq1_2) > (1 / 10) * sqrt(sq2_2))
				{
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((outputSeg.nodes.size() - deleteCount2) <= 2)
					{
						--deleteCount2;
						break;
					}

					(it + 1)->parent = -1;
					delLocs.push_back(int(it - outputSeg.nodes.begin()));
				}
			}
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
		delLocs.clear();
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);

	size_t label = 1;
	cout << "number of nodes after straightening process: " << outputSeg.nodes.size() << " ( segID = " << outputSeg.segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
	{
		if (it->parent == -1) it->parent = (it - 1)->n;
	}

	return outputSeg;
}
// ============================================= End of [Geometry] =============================================

NeuronTree NeuronStructExplorer::SWC2MSTtree(NeuronTree const& inputTree)
{
	NeuronTree MSTtrees;
	undirectedGraph graph(inputTree.listNeuron.size());
	//cout << "processing nodes: \n -- " << endl;
	for (int i = 0; i < inputTree.listNeuron.size(); ++i)
	{

		float x1, y1, z1;
		x1 = inputTree.listNeuron.at(i).x;
		y1 = inputTree.listNeuron.at(i).y;
		z1 = inputTree.listNeuron.at(i).z;
		for (int j = 0; j < inputTree.listNeuron.size(); ++j)
		{
			float x2, y2, z2;
			x2 = inputTree.listNeuron.at(j).x;
			y2 = inputTree.listNeuron.at(j).y;
			z2 = inputTree.listNeuron.at(j).z;

			double Vedge = sqrt(double(x1 - x2) * double(x1 - x2) + double(y1 - y2) * double(y1 - y2) + zRATIO * zRATIO * double(z1 - z2) * double(z1 - z2));
			pair<undirectedGraph::edge_descriptor, bool> edgeQuery = boost::edge(i, j, graph);
			if (!edgeQuery.second && i != j) boost::add_edge(i, j, lastVoted(i, weights(Vedge)), graph);
		}

		//if (i % 1000 == 0) cout << i << " ";
	}
	//cout << endl;

	vector <boost::graph_traits<undirectedGraph>::vertex_descriptor > p(num_vertices(graph));
	boost::prim_minimum_spanning_tree(graph, &p[0]);
	NeuronTree MSTtree;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	for (size_t ii = 0; ii != p.size(); ++ii)
	{
		int pn;
		if (p[ii] == ii) pn = -1;
		else pn = p[ii] + 1;

		NeuronSWC S;
		S.n = ii + 1;
		S.type = 7;
		S.x = inputTree.listNeuron.at(ii).x;
		S.y = inputTree.listNeuron.at(ii).y;
		S.z = inputTree.listNeuron.at(ii).z;
		S.r = 1;
		S.pn = pn;
		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size() - 1);
	}
	MSTtree.listNeuron = listNeuron;
	MSTtree.hashNeuron = hashNeuron;

	return MSTtree;
}

NeuronTree NeuronStructExplorer::SWC2MSTtree_tiled(NeuronTree const& inputTree, float tileLength, float zDivideNum)
{
	map<string, QList<NeuronSWC>> tiledSWCmap;

		QList<NeuronSWC> tileSWCList;
		tileSWCList.clear();
		for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		{
			int tileXlabel = int(floor(it->x / tileLength));
			int tileYlabel = int(floor(it->y / tileLength));
			int tileZlabel = int(floor(it->z / ((tileLength / zRATIO) / zDivideNum)));
			string swcTileKey = to_string(tileXlabel) + "_" + to_string(tileYlabel) + "_" + to_string(tileZlabel);
			tiledSWCmap.insert(pair<string, QList<NeuronSWC>>(swcTileKey, tileSWCList));
			tiledSWCmap[swcTileKey].push_back(*it);
		}
		cout << "tiledSWCmap size = " << tiledSWCmap.size() << endl;

		NeuronTree assembledTree;
		for (map<string, QList<NeuronSWC>>::iterator it = tiledSWCmap.begin(); it != tiledSWCmap.end(); ++it)
		{
			NeuronTree tileTree;
			tileTree.listNeuron = it->second;
			NeuronTree tileMSTtree = this->SWC2MSTtree(tileTree);

			int currnodeNum = assembledTree.listNeuron.size();
			for (QList<NeuronSWC>::iterator nodeIt = tileMSTtree.listNeuron.begin(); nodeIt != tileMSTtree.listNeuron.end(); ++nodeIt)
			{
				nodeIt->n = nodeIt->n + currnodeNum;
				if (nodeIt->parent != -1)
				{
					nodeIt->parent = nodeIt->parent + currnodeNum;
					//cout << "  " << nodeIt->parent << " " << currnodeNum << endl;
				}

				//cout << nodeIt->n << " " << nodeIt->parent << endl;
				assembledTree.listNeuron.push_back(*nodeIt);
			}
		}

		return assembledTree;
}

NeuronTree NeuronStructExplorer::MSTbranchBreak(const profiledTree& inputProfiledTree, double spikeThre, bool spikeRemove)
{
	// -- This method breaks all branching points of a MST-connected tree.
	// -- When the value of spikeRemove is true, it eliminates spikes on a main route without breaking it. The default value is true. 

	profiledTree outputProfiledTree(inputProfiledTree.tree);

	vector<size_t> spikeLocs;
	if (spikeRemove)
	{
		for (QList<NeuronSWC>::iterator it = outputProfiledTree.tree.listNeuron.begin(); it != outputProfiledTree.tree.listNeuron.end(); ++it)
		{
			if (outputProfiledTree.node2childLocMap.find(it->n) != outputProfiledTree.node2childLocMap.end())
			{
				vector<size_t> childLocs = outputProfiledTree.node2childLocMap.at(it->n);
				if (childLocs.size() >= 2)
				{
					int nodeRemoveCount = 0;
					for (vector<size_t>::iterator locIt = childLocs.begin(); locIt != childLocs.end(); ++locIt)
					{
						if (outputProfiledTree.node2childLocMap.find(outputProfiledTree.tree.listNeuron.at(*locIt).n) == outputProfiledTree.node2childLocMap.end())
						{
							double spikeDist = sqrt((outputProfiledTree.tree.listNeuron.at(*locIt).x - it->x) * (outputProfiledTree.tree.listNeuron.at(*locIt).x - it->x) +
								(outputProfiledTree.tree.listNeuron.at(*locIt).y - it->y) * (outputProfiledTree.tree.listNeuron.at(*locIt).y - it->y) +
								(outputProfiledTree.tree.listNeuron.at(*locIt).z - it->z) * (outputProfiledTree.tree.listNeuron.at(*locIt).z - it->z) * zRATIO * zRATIO);
							if (spikeDist <= spikeThre) // Take out splikes.
							{
								spikeLocs.push_back(*locIt);
								++nodeRemoveCount;
							}
						}
					}

					if (nodeRemoveCount == childLocs.size() - 1) continue; // If there is only 1 child left after taking out all spikes, then this node is supposed to be on the main route.
					else
					{
						for (vector<size_t>::iterator locCheckIt = childLocs.begin(); locCheckIt != childLocs.end(); ++locCheckIt)
						{
							if (find(spikeLocs.begin(), spikeLocs.end(), *locCheckIt) == spikeLocs.end())
								outputProfiledTree.tree.listNeuron[*locCheckIt].parent = -1; // 'at' operator treats the container as const, and cannot assign values. Therefore, use [] instead.
						}
					}
				}
			}
		}
	}
	else
	{
		for (QList<NeuronSWC>::iterator it = outputProfiledTree.tree.listNeuron.begin(); it != outputProfiledTree.tree.listNeuron.end(); ++it)
		{
			if (outputProfiledTree.node2childLocMap.find(it->n) != outputProfiledTree.node2childLocMap.end())
			{
				if (outputProfiledTree.node2childLocMap.at(it->n).size() >= 2)
				{
					for (vector<size_t>::const_iterator locIt = outputProfiledTree.node2childLocMap.at(it->n).begin(); locIt != outputProfiledTree.node2childLocMap.at(it->n).end(); ++locIt)
						outputProfiledTree.tree.listNeuron[*locIt].parent = -1;
				}
			}
		}
	}

	if (spikeRemove) // Erase spike nodes from outputTree.listNeuron.
	{
		sort(spikeLocs.rbegin(), spikeLocs.rend());
		for (vector<size_t>::iterator delIt = spikeLocs.begin(); delIt != spikeLocs.end(); ++delIt)
			outputProfiledTree.tree.listNeuron.erase(outputProfiledTree.tree.listNeuron.begin() + ptrdiff_t(*delIt));
	}

	return outputProfiledTree.tree;
}



// =================================== Neuron Struct Refining Method =================================== //
profiledTree NeuronStructExplorer::spikeRemove(const profiledTree& inputProfiledTree)
{
	QList<NeuronSWC> outputList = inputProfiledTree.tree.listNeuron;

	vector<size_t> delLocs;
	for (QList<NeuronSWC>::const_iterator it = inputProfiledTree.tree.listNeuron.begin(); it != inputProfiledTree.tree.listNeuron.end(); ++it)
	{
		if (inputProfiledTree.node2childLocMap.find(it->n) == inputProfiledTree.node2childLocMap.end()) // tip point
		{
			if (inputProfiledTree.node2childLocMap.at(it->parent).size() >= 2) delLocs.push_back(inputProfiledTree.node2LocMap.at(it->n));
		}
	}

	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<size_t>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputList.erase(outputList.begin() + ptrdiff_t(*it));

	NeuronTree outputTree;
	outputTree.listNeuron = outputList;
	profiledTree outputProfiledTree(outputTree);

	return outputProfiledTree;
}
// =================================================================================================== //










void NeuronStructExplorer::falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- Determing false positives out of the detected structure by the threshold of specified distance to the manual structure.
	// -- if (the shortest distance of node i from detectedTree to manualTree > distThreshold) node i is deemed false positive.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = detectedTreePtr->listNeuron.begin(); it != detectedTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = manualTreePtr->listNeuron.begin();
		while (checkIt != manualTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist < distThreshold)
			{
				this->processedTree.listNeuron.push_back(*it);
				break;
			}

			++checkIt;
			if (checkIt == manualTreePtr->listNeuron.end())
			{
				vector<float> FPcoords;
				FPcoords.push_back(it->x);
				FPcoords.push_back(it->y);
				FPcoords.push_back(it->z);
				this->FPsList.push_back(FPcoords);
				FPcoords.clear();
			}
		}
	}
}

void NeuronStructExplorer::falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	// -- False negavies are selected out of the manual structure if none of detected nodes are close enough to them.
	// -- if (the shortest distance of node i from manualTree to detectedTree > distThreshold) node i is deemed false negative.

	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = manualTreePtr->listNeuron.begin(); it != manualTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = detectedTreePtr->listNeuron.begin();
		while (checkIt != detectedTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist > distThreshold)
			{
				vector<float> FNcoords;
				FNcoords.push_back(it->x);
				FNcoords.push_back(it->y);
				FNcoords.push_back(it->z);
				this->FNsList.push_back(FNcoords);
				FNcoords.clear();

				break;
			}

			++checkIt;
		}
	}
}

void NeuronStructExplorer::detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2)
{
	// -- Compute the shortest distance of each node from input structure (inputTreePtr1) to refernce structure (inputTreePtr2) 
	// -- The order of inputTreePtr1 and inputTreePtr2 represent the distances from opposite comparing directions.

	vector<float> minDists;
	long int count = 1;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		++count;
		//cout << "node No. " << count << ".." << endl;
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		minDists.push_back(minDist);
	}

	float distSum = 0;
	for (vector<float>::iterator it = minDists.begin(); it != minDists.end(); ++it) distSum = distSum + *it;
	float distMean = distSum / minDists.size();
	float varSum = 0;
	for (int i = 0; i < minDists.size(); ++i) varSum = varSum + (minDists[i] - distMean) * (minDists[i] - distMean);
	float distVar = varSum / minDists.size();
	float distStd = sqrtf(distVar);
	
	cout << distMean << " " << distStd << endl;
}

void NeuronStructExplorer::shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum)
{
	// -- Compute the PDF and CDF of shortest distance of detected nodes to manual nodes.

	float interval = float(upperBound) / float(binNum);
	vector<float> shortestDists;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		shortestDists.push_back(minDist);
	}

	for (int i = 0; i < binNum; ++i)
	{
		this->nodeDistPDF[i] = 0;
		this->nodeDistPDF[i] = 0;
	}
	for (vector<float>::iterator distIt = shortestDists.begin(); distIt != shortestDists.end(); ++distIt)
	{
		int binNo = (*distIt) / interval;
		++(this->nodeDistPDF[binNo]);
	}
	this->nodeDistCDF[0] = this->nodeDistPDF[0];
	for (int i = 1; i < binNum; ++i) this->nodeDistCDF[i] = this->nodeDistCDF[i - 1] + nodeDistPDF[i];
}

void NeuronStructExplorer::segmentDecompose(NeuronTree* inputTreePtr)
{
	// -- This function used get_link_map and decompose in v_neuronswc.cpp to get segment hierarchical information.

	this->segmentList.clear();
	this->segmentList = NeuronTree__2__V_NeuronSWC_list(*inputTreePtr);
}