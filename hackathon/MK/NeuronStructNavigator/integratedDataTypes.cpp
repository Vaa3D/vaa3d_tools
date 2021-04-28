//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [integratedDataTypes] is part of the NeuronStructNavigator library.
*  The namespace manages all integrated data structures used by all other NeuronStructNavigator classes.
*  All data structures in this namespace are integrated with standard Vaa3D data types with additional features, aiming to make neuron structure operations and algorithms easier.
*  Any new development on the datatypes should be put in this namespace to keep them organized and avoid the confusion of header inclusion.
*
*  [profiledTree] is the core data type in throughout the whole NeuronStructNavigator library. It profiles the NeuronTree and carries crucial information of it.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

********************************************************************************/

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"
#include "NeuronStructNavigatingTester.h"

#include <iostream>

using namespace std;
using NSlibTester = NeuronStructNavigator::Tester;

integratedDataTypes::overlappedCoord::overlappedCoord(float x, float y, float z) : x(x), y(y), z(z)
{
	this->coordKey = to_string(x) + "_" + to_string(y) + "_" + to_string(z);
	set<pair<int, int>> segNodeIDpairSet;
	this->involvedSegsOriMap.insert({ integratedDataTypes::head, segNodeIDpairSet });
	this->involvedSegsOriMap.insert({ integratedDataTypes::tail, segNodeIDpairSet });
	this->involvedSegsOriMap.insert({ integratedDataTypes::body, segNodeIDpairSet });
}

integratedDataTypes::overlappedCoord::overlappedCoord(string coordKey) : coordKey(coordKey)
{
	vector<string> coord;
	string currDimValue;
	currDimValue.clear();
	for (int i = 0; i < coordKey.length(); ++i)
	{
		if (coordKey.at(i) == '_')
		{
			coord.push_back(currDimValue);
			currDimValue.clear();
			continue;
		}
		else currDimValue += coordKey.at(i);
	}

	this->x = stof(coord.at(0));
	this->y = stof(coord.at(1));
	this->z = stof(coord.at(2));

	set<pair<int, int>> segNodeIDpairSet;
	this->involvedSegsOriMap.insert({ integratedDataTypes::head, segNodeIDpairSet });
	this->involvedSegsOriMap.insert({ integratedDataTypes::tail, segNodeIDpairSet });
	this->involvedSegsOriMap.insert({ integratedDataTypes::body, segNodeIDpairSet });
}

integratedDataTypes::segUnit::segUnit(const V_NeuronSWC& inputV_NeuronSWC)
{
	if ((inputV_NeuronSWC.row.end() - 1)->parent == -1)
	{
		for (vector<V_NeuronSWC_unit>::const_iterator nodeIt = inputV_NeuronSWC.row.begin(); nodeIt != inputV_NeuronSWC.row.end(); ++nodeIt)
		{
			NeuronSWC node;
			node.n = nodeIt->data[0];
			node.x = nodeIt->data[2];
			node.y = nodeIt->data[3];
			node.z = nodeIt->data[4];
			node.type = nodeIt->data[1];
			node.parent = nodeIt->data[6];
			this->nodes.push_front(node);
		}
	}
	else if (inputV_NeuronSWC.row.begin()->parent == -1) // [Alt + B] generated segment
	{
		for (vector<V_NeuronSWC_unit>::const_iterator nodeIt = inputV_NeuronSWC.row.begin(); nodeIt != inputV_NeuronSWC.row.end(); ++nodeIt)
		{
			NeuronSWC node;
			node.n = nodeIt->data[0];
			node.x = nodeIt->data[2];
			node.y = nodeIt->data[3];
			node.z = nodeIt->data[4];
			node.type = nodeIt->data[1];
			node.parent = node.n + 1;
			this->nodes.push_front(node);
		}
		this->nodes.begin()->parent = -1;
	}
	if (!this->nodes.isEmpty()) this->head = this->nodes.begin()->n;
	
	/******************************************************************************************************/
	// Important Note: 
	//   [segUnit.seg_childLocMap] does NOT have tail node registered freshly coming out of [NeuronStructExplorer::node2loc_node2childLocMap].
	//   Adding tail nodes into the map with empty child locations here for the purpose of avoiding memory violations by accessing non-existent pair in the map.
	//   Many operations are performed on segment base, it is more conveniend to have tails also possess entries in [this->seg_childLocMap].
	NeuronStructExplorer::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);
	for (QList<NeuronSWC>::iterator nodeIt = this->nodes.begin(); nodeIt != this->nodes.end(); ++nodeIt)
	{
		if (this->seg_childLocMap.find(nodeIt->n) == this->seg_childLocMap.end())
		{
			this->tails.push_back(nodeIt->n);
			vector<size_t> emptyChildSet;
			seg_childLocMap.insert({ nodeIt->n, emptyChildSet });
		}
	}
	/******************************************************************************************************/
	
	this->to_be_deleted = inputV_NeuronSWC.to_be_deleted;
}

integratedDataTypes::segUnit::segUnit(const QList<NeuronSWC>& inputSeg) : to_be_deleted(false)
{
	this->nodes = inputSeg;
	NeuronStructExplorer::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);

	// If the [segUnit] is generated by [NeuronStructExplorer::findSegs], the 1st node in [this->nodes] is guaranteed to be the head.
	// However, [segUnit]'s nodes are not automatically sorted when it's constructed. So [*this->nodes.begin()] cannot be guaranteed to be the head.
	for (auto& node : this->nodes)
	{
		if (node.parent == -1)
		{
			this->head = node.n;
			break;
		}
	}
	
	for (QList<NeuronSWC>::iterator nodeIt = this->nodes.begin(); nodeIt != this->nodes.end(); ++nodeIt)
	{
		if (this->seg_childLocMap.find(nodeIt->n) == this->seg_childLocMap.end())
		{
			this->tails.push_back(nodeIt->n);
			vector<size_t> emptyTailSet;
			seg_childLocMap.insert({ nodeIt->n, emptyTailSet });
		}
	}
}

const bool integratedDataTypes::segUnit::operator==(const segUnit& comparedSeg) const
{
	// Given 2 segUnits with the same NeuronSWC composition, it is nearly impossible for them being actually different.
	// Therefore, current implementation is based on node-wise comparing without considering its morphology.

	if (this->nodes.size() != comparedSeg.nodes.size()) return false;
	else if (this->nodes.isEmpty() && comparedSeg.nodes.isEmpty()) return true;
	else
	{
		boost::container::flat_set<int> comparedNodeIDs;
		for (QList<NeuronSWC>::const_iterator it = comparedSeg.nodes.begin(); it != comparedSeg.nodes.end(); ++it) 
			comparedNodeIDs.insert(it->n);
		for (QList<NeuronSWC>::const_iterator it = this->nodes.begin(); it != this->nodes.end(); ++it)
		{
			for (boost::container::flat_set<int>::iterator IDit = comparedNodeIDs.begin(); IDit != comparedNodeIDs.end(); ++IDit)
			{
				const NeuronSWC& comparedNode = comparedSeg.nodes.at(comparedSeg.seg_nodeLocMap.at(*IDit));
				if (it->x == comparedNode.x && it->y == comparedNode.y && it->z == comparedNode.z)
				{
					comparedNodeIDs.erase(IDit);
					goto FOUND_SAME_NODE;
				}
			}
			return false;

		FOUND_SAME_NODE:
			continue;
		}

		return true;
	}
}

const bool integratedDataTypes::segUnit::operator>(const segUnit& comparedSeg) const
{
	if (this->nodes.size() > comparedSeg.nodes.size()) return true;
	else if (this->nodes.size() < comparedSeg.nodes.size()) return false;
}

const bool integratedDataTypes::segUnit::operator<(const segUnit& comparedSeg) const
{
	if (this->nodes.size() < comparedSeg.nodes.size()) return true;
	else if (this->nodes.size() > comparedSeg.nodes.size()) return false;
}

void integratedDataTypes::segUnit::reInit(segUnit& inputSegUnit)
{
	segUnit reInitUnit(inputSegUnit.nodes);
	reInitUnit.segID = inputSegUnit.segID;
	inputSegUnit = reInitUnit; // Copy reInitUnit to the memory where inputSegUnit refers to -- OK.
}

bool integratedDataTypes::segUnit::reverse(int nodeID)
{
	if (nodeID == -1 && this->tails.size() > 1) return false;
	else if (this->nodes.at(this->seg_nodeLocMap.at(nodeID)).parent != -1 && !this->seg_childLocMap.at(nodeID).empty()) return false;
	else if (nodeID == this->head) return true;

	if (nodeID == -1)
	{
		QList<NeuronSWC> newNodeList;
		for (auto& node : this->nodes)
		{
			NeuronSWC newNode = node;
			if (this->seg_childLocMap.find(node.n) == this->seg_childLocMap.end())
			{
				newNode.parent = -1;
				newNodeList.push_front(newNode);
				continue;
			}
			const NeuronSWC& childNode = this->nodes.at(*this->seg_childLocMap.at(newNode.n).begin());
			newNode.parent = childNode.n;
			newNodeList.push_front(newNode);
		}

		this->nodes = newNodeList;
		this->reInit(*this);
		return true;
	}
	else
	{
		this->nodes = this->changeTreeHead(nodeID);
		this->reInit(*this);
		return true;
	}
}

QList<NeuronSWC> integratedDataTypes::segUnit::changeTreeHead(const int newHeadID) const
{
	QList<NeuronSWC> outputNodes;
	NeuronSWC newHeadNode = this->nodes.at(this->seg_nodeLocMap.at(newHeadID));
	int currDominoID = newHeadNode.parent;
	newHeadNode.parent = -1;
	outputNodes.append(newHeadNode);

	NeuronSWC currDominoNode, previousDominoNode = newHeadNode;
	while (currDominoID != this->head)
	{
		currDominoNode = this->nodes.at(this->seg_nodeLocMap.at(currDominoID));
		currDominoNode.parent = previousDominoNode.n;
		outputNodes.append(currDominoNode);

		for (auto& childLoc : this->seg_childLocMap.at(currDominoID))
		{
			if (this->nodes.at(childLoc).n == previousDominoNode.n) continue;
			else
			{
				QList<NeuronSWC> intactBranch;
				NeuronSWC startingNode = this->nodes.at(childLoc);
				map<int, size_t> node2locMap;
				map<int, vector<size_t>> node2childLocMap;
				NeuronStructExplorer::downstream_subTreeExtract(this->nodes, intactBranch, startingNode, node2locMap, node2childLocMap);
				outputNodes.append(intactBranch);
			}
		}

		previousDominoNode = currDominoNode;
		currDominoID = this->nodes.at(this->seg_nodeLocMap.at(currDominoID)).parent;
	}

	currDominoNode = this->nodes.at(this->seg_nodeLocMap.at(this->head));
	currDominoNode.parent = previousDominoNode.n;
	outputNodes.append(currDominoNode);
	for (auto& headChildLoc : this->seg_childLocMap.at(this->head))
	{
		if (this->nodes.at(headChildLoc).n == previousDominoNode.n) continue;
		else
		{
			QList<NeuronSWC> intactBranch;
			NeuronSWC startingNode = this->nodes.at(headChildLoc);
			map<int, size_t> node2locMap;
			map<int, vector<size_t>> node2childLocMap;
			NeuronStructExplorer::downstream_subTreeExtract(this->nodes, intactBranch, startingNode, node2locMap, node2childLocMap);
			outputNodes.append(intactBranch);
		}
	}

	return outputNodes;
}

V_NeuronSWC integratedDataTypes::segUnit::convert2V_NeuronSWC() const
{
	V_NeuronSWC outputV_NeuronSWC;
	outputV_NeuronSWC.to_be_deleted = this->to_be_deleted;
	int paNodeID = this->head;
	if (this->nodes.size() == 1)
	{
		V_NeuronSWC_unit newNodeV;
		newNodeV.data[0] = 1;
		newNodeV.data[2] = this->nodes.begin()->x;
		newNodeV.data[3] = this->nodes.begin()->y;
		newNodeV.data[4] = this->nodes.begin()->z;
		newNodeV.data[1] = this->nodes.begin()->type;
		newNodeV.data[6] = -1;
		newNodeV.seg_id = this->segID;
		outputV_NeuronSWC.row.push_back(newNodeV);
		return outputV_NeuronSWC;
	}
	this->rc_nodeRegister2V_NeuronSWC(outputV_NeuronSWC, paNodeID, -1);

	return outputV_NeuronSWC;
}

void integratedDataTypes::segUnit::rc_nodeRegister2V_NeuronSWC(V_NeuronSWC& sbjV_NeuronSWC, int parentID, int branchRootID) const
{
	int currentPaID = parentID;
	while (1)
	{
		V_NeuronSWC_unit newNodeV;
		newNodeV.data[0] = this->nodes.size() - sbjV_NeuronSWC.row.size();
		newNodeV.data[2] = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).x;
		newNodeV.data[3] = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).y;
		newNodeV.data[4] = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).z;
		newNodeV.data[1] = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).type;		
		newNodeV.seg_id = this->segID;		

		if (this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent == -1) newNodeV.data[6] = -1;
		else
		{
			if (this->seg_childLocMap.at(this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent).size() > 1)
				newNodeV.data[6] = branchRootID;
			
			if (this->seg_childLocMap.at(this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent).size() == 1)
				newNodeV.data[6] = newNodeV.n + 1;
		}
		sbjV_NeuronSWC.row.insert(sbjV_NeuronSWC.row.begin(), newNodeV);

		if (this->seg_childLocMap.at(currentPaID).size() == 1) currentPaID = this->nodes.at(*this->seg_childLocMap.at(currentPaID).begin()).n;
		else if (this->seg_childLocMap.at(currentPaID).empty()) return;
		else if (this->seg_childLocMap.at(currentPaID).size() > 1)
		{
			for (vector<size_t>::const_iterator tailsIt = this->seg_childLocMap.at(currentPaID).begin(); tailsIt != this->seg_childLocMap.at(currentPaID).end(); ++tailsIt)
				this->rc_nodeRegister2V_NeuronSWC(sbjV_NeuronSWC, this->nodes.at(*tailsIt).n, newNodeV.data[0]);
			return;
		}	
	}
}

integratedDataTypes::segPairProfile::segPairProfile(const segUnit& inputSeg1, const segUnit& inputSeg2, connectOrientation connOrt) : seg1Ptr(&inputSeg1), seg2Ptr(&inputSeg2), currConnOrt(connOrt)
{
	this->getSegDistance(connOrt);

	if (inputSeg1.tails.size() > 1 || inputSeg2.tails.size() > 1)
	{
		cerr << " -- Branched segments are currently not supported. The turnning angle of 2 segments won't be computed." << endl;
		return;
	}
	
	if (connOrt != all_ort)
	{
		this->turning12(connOrt);
		this->segsAngleDiff12(connOrt);
	}
}

void integratedDataTypes::segPairProfile::getSegDistance(connectOrientation connOrt)
{
	if (connOrt == head_head)
	{
		this->currConnOrt = head_head;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		this->connDistMap.insert({ head_head, sqrt((seg1Head.x - seg2Head.x) * (seg1Head.x - seg2Head.x) + (seg1Head.y - seg2Head.y) * (seg1Head.y - seg2Head.y) + (seg1Head.z - seg2Head.z) * (seg1Head.z - seg2Head.z)) });
	}
	else if (connOrt == head_tail)
	{
		this->currConnOrt = head_tail;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ head_tail, sqrt((seg1Head.x - seg2Tail.x) * (seg1Head.x - seg2Tail.x) + (seg1Head.y - seg2Tail.y) * (seg1Head.y - seg2Tail.y) + (seg1Head.z - seg2Tail.z) * (seg1Head.z - seg2Tail.z)) });
	}
	else if (connOrt == tail_head)
	{
		this->currConnOrt = tail_head;
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		this->connDistMap.insert({ tail_head, sqrt((seg1Tail.x - seg2Head.x) * (seg1Tail.x - seg2Head.x) + (seg1Tail.y - seg2Head.y) * (seg1Tail.y - seg2Head.y) + (seg1Tail.z - seg2Head.z) * (seg1Tail.z - seg2Head.z)) });
	}
	else if (connOrt == tail_tail)
	{
		this->currConnOrt = tail_tail;
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ tail_tail, sqrt((seg1Tail.x - seg2Tail.x) * (seg1Tail.x - seg2Tail.x) + (seg1Tail.y - seg2Tail.y) * (seg1Tail.y - seg2Tail.y) + (seg1Tail.z - seg2Tail.z) * (seg1Tail.z - seg2Tail.z)) });
	}
	else if (connOrt == all_ort)
	{
		this->currConnOrt = all_ort;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ head_head, sqrt((seg1Head.x - seg2Head.x) * (seg1Head.x - seg2Head.x) + (seg1Head.y - seg2Head.y) * (seg1Head.y - seg2Head.y) + (seg1Head.z - seg2Head.z) * (seg1Head.z - seg2Head.z)) });
		this->connDistMap.insert({ head_tail, sqrt((seg1Head.x - seg2Tail.x) * (seg1Head.x - seg2Tail.x) + (seg1Head.y - seg2Tail.y) * (seg1Head.y - seg2Tail.y) + (seg1Head.z - seg2Tail.z) * (seg1Head.z - seg2Tail.z)) });
		this->connDistMap.insert({ tail_head, sqrt((seg1Tail.x - seg2Head.x) * (seg1Tail.x - seg2Head.x) + (seg1Tail.y - seg2Head.y) * (seg1Tail.y - seg2Head.y) + (seg1Tail.z - seg2Head.z) * (seg1Tail.z - seg2Head.z)) });
		this->connDistMap.insert({ tail_tail, sqrt((seg1Tail.x - seg2Tail.x) * (seg1Tail.x - seg2Tail.x) + (seg1Tail.y - seg2Tail.y) * (seg1Tail.y - seg2Tail.y) + (seg1Tail.z - seg2Tail.z) * (seg1Tail.z - seg2Tail.z)) });
	}
}

void integratedDataTypes::segPairProfile::turning12(connectOrientation connOrt)
{
	if (connOrt == head_head) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == head_tail) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == tail_head) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == tail_tail) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
}

void integratedDataTypes::segPairProfile::segsAngleDiff12(connectOrientation connOrt)
{
	vector<double> dispVec1, dispVec2;
	if (connOrt == head_head)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, head);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, tail);
	}
	else if (connOrt == head_tail)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, head);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, head);
	}
	else if (connOrt == tail_head)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, tail);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, tail);
	}
	else if (connOrt == tail_tail)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, tail);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, head);
	}

	this->segsAngleDiff = NeuronGeoGrapher::getPiAngle(dispVec1, dispVec2);
}

integratedDataTypes::profiledTree::profiledTree(const NeuronTree& inputTree, float nodeTileLength, float segTileLength)
{
	this->tree.listNeuron.clear();
	
	if (inputTree.listNeuron.empty())
	{
		cerr << "The input tree is empty, profiledTree cannot be initialized." << endl;
		return;
	}
	else
	{
		this->tree = inputTree;
		this->segTileSize = segTileLength;
		this->nodeTileSize = nodeTileLength;

		NeuronStructExplorer::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		
		// -- NeuronStructExplorer::node2loc_node2childLocMap does Not create entries in node2childLocMap for tip nodes (nodes without childs).
		NeuronStructExplorer::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

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
}

/*integratedDataTypes::profiledTree::profiledTree(const vector<segUnit>& inputSegs, float nodeTileLength, float segTileLength)
{
	if (inputSegs.empty())
	{
		cerr << "The input V_NeuronSWC list is empty, profiledTree cannot be initialized." << endl;
		return;
	}
	else
	{
		this->segTileSize = segTileLength;
		this->nodeTileSize = nodeTileLength;



		vector<V_NeuronSWC> displayV_NeuronSWCcopy = inputV_NeuronSWC;
		for (vector<V_NeuronSWC>::iterator it = displayV_NeuronSWCcopy.begin() + 1; it != displayV_NeuronSWCcopy.end(); ++it)
		{
			// In V_NeuronSWC_unit, all node information is stored in [data].
			//if (it->row.begin()->data[6] == -1) reverse(it->row.begin(), it->row.end());

			if ((it->row.end() - 1)->data[6] == -1)
			{
				for (vector<V_NeuronSWC_unit>::iterator unitIt = it->row.begin(); unitIt != it->row.end(); ++unitIt)
				{
					unitIt->data[0] += ((it - 1)->row.end() - 1)->data[0];
					unitIt->data[6] = unitIt->data[0] + 1;
				}
				(it->row.end() - 1)->data[6] = -1;
			}
			else if (it->row.begin()->data[6] == -1)
			{
				for (vector<V_NeuronSWC_unit>::iterator unitIt = it->row.begin(); unitIt != it->row.end(); ++unitIt)
				{
					unitIt->data[0] += ((it - 1)->row.end() - 1)->data[0];
					unitIt->data[6] = unitIt->data[0] - 1;
				}
				it->row.begin()->data[6] = -1;
			}
		}

		vector<segUnit> allSegs;
		for (vector<V_NeuronSWC>::iterator it = displayV_NeuronSWCcopy.begin(); it != displayV_NeuronSWCcopy.end(); ++it)
		{
			segUnit newSegUnit(*it);

			// segID starts from 0 instead of 1 => to be consistent with input vector<V_NeuronSWC>'s subscriptor. 
			newSegUnit.segID = int(it - displayV_NeuronSWCcopy.begin());

			this->segs.insert({ newSegUnit.segID, newSegUnit });
			allSegs.push_back(newSegUnit);
			this->tree.listNeuron.append(newSegUnit.nodes);
		}

		NeuronStructExplorer::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		// -- NeuronStructExplorer::node2loc_node2childLocMap does Not create entries in node2childLocMap for tip nodes (nodes without childs).
		NeuronStructExplorer::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

		this->segHeadMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength);
		this->segTailMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength, false);
	}
}*/

integratedDataTypes::profiledTree::profiledTree(const vector<V_NeuronSWC>& inputV_NeuronSWC, float nodeTileLength, float segTileLength)
{
	if (inputV_NeuronSWC.empty())
	{
		cerr << "The input V_NeuronSWC list is empty, profiledTree cannot be initialized." << endl;
		return;
	}
	else
	{
		this->segTileSize = segTileLength;
		this->nodeTileSize = nodeTileLength;

		vector<V_NeuronSWC> displayV_NeuronSWCcopy = inputV_NeuronSWC;
		for (vector<V_NeuronSWC>::iterator it = displayV_NeuronSWCcopy.begin() + 1; it != displayV_NeuronSWCcopy.end(); ++it)
		{
			// In V_NeuronSWC_unit, all node information is stored in [data].
			//if (it->row.begin()->data[6] == -1) reverse(it->row.begin(), it->row.end());

			if ((it->row.end() - 1)->data[6] == -1)
			{
				for (vector<V_NeuronSWC_unit>::iterator unitIt = it->row.begin(); unitIt != it->row.end(); ++unitIt)
				{
					unitIt->data[0] += ((it - 1)->row.end() - 1)->data[0];
					unitIt->data[6] = unitIt->data[0] + 1;
				}
				(it->row.end() - 1)->data[6] = -1;
			}
			else if (it->row.begin()->data[6] == -1)
			{
				for (vector<V_NeuronSWC_unit>::iterator unitIt = it->row.begin(); unitIt != it->row.end(); ++unitIt)
				{
					unitIt->data[0] += ((it - 1)->row.end() - 1)->data[0];
					unitIt->data[6] = unitIt->data[0] - 1;
				}
				it->row.begin()->data[6] = -1;
			}
		}

		vector<segUnit> allSegs;
		for (vector<V_NeuronSWC>::iterator it = displayV_NeuronSWCcopy.begin(); it != displayV_NeuronSWCcopy.end(); ++it)
		{
			segUnit newSegUnit(*it);
			
			// segID starts from 0 instead of 1 => to be consistent with input vector<V_NeuronSWC>'s subscriptor. 
			newSegUnit.segID = int(it - displayV_NeuronSWCcopy.begin()); 

			this->segs.insert({ newSegUnit.segID, newSegUnit });
			allSegs.push_back(newSegUnit);
			this->tree.listNeuron.append(newSegUnit.nodes);
		}

		NeuronStructExplorer::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		// -- NeuronStructExplorer::node2loc_node2childLocMap does Not create entries in node2childLocMap for tip nodes (nodes without childs).
		NeuronStructExplorer::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

		this->segHeadMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength);
		this->segTailMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength, false);
	}
}

void integratedDataTypes::profiledTree::nodeTileResize(float nodeTileLength)
{
	if (nodeTileLength == NODE_TILE_LENGTH) return;
	else
	{
		if (!this->nodeTileMap.empty())
		{
			this->nodeTileMap.clear();
			NeuronStructExplorer::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		}
		else
		{
			this->nodeTileSize = nodeTileLength;
			return;
		}
	}
}

int integratedDataTypes::profiledTree::findNearestSegEndNodeID(const CellAPO inputAPO, int threshold)
{
	this->segEndCoordKey2segMap.clear();
	this->segEndCoordKeySegMapGen();
	this->nodeCoordKey2segMap.clear();
	this->nodeCoordKeySegMapGen();

	string targetNodeTileKey = NeuronStructExplorer::getNodeTileKey(inputAPO);
	float dist = 100000;
	int outputNodeID = 0;
	vector<int> nodeIDs;
	if (this->nodeTileMap.find(targetNodeTileKey) != this->nodeTileMap.end())
	{
		//cout << "soma node tile: " << targetNodeTileKey << endl;
		nodeIDs = this->nodeTileMap.at(targetNodeTileKey);
		for (auto& nodeID : nodeIDs)
		{
			const NeuronSWC& node = this->tree.listNeuron.at(this->node2LocMap.at(nodeID));
			string nodeCoordKey = to_string(node.x) + "_" + to_string(node.y) + "_" + to_string(node.z);
			if (this->segEndCoordKey2segMap.find(nodeCoordKey) != this->segEndCoordKey2segMap.end())
			{
				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->segEndCoordKey2segMap.equal_range(nodeCoordKey);
				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range2 = this->nodeCoordKey2segMap.equal_range(nodeCoordKey);
				if (range.second - range.first == 1 && range2.second - range2.first == 1)
				{
					float currNodeDist = sqrtf((node.x - inputAPO.x) * (node.x - inputAPO.x) + (node.y - inputAPO.y) * (node.y - inputAPO.y) + (node.z - inputAPO.z) * (node.z - inputAPO.z));
					if (currNodeDist < dist)
					{
						outputNodeID = nodeID;
						dist = currNodeDist;
					}
				}
			}
		}
	}
	
	if (outputNodeID != 0 && dist <= threshold) return outputNodeID;
	else
	{
		for (int k = -1; k <= 1; ++k)
		{
			for (int j = -1; j <= 1; ++j)
			{
				for (int i = -1; i <= 1; ++i)
				{
					vector<string> tileKeyStrings;
					stringstream ss(targetNodeTileKey);
					while (ss.good())
					{
						string subStr;
						getline(ss, subStr, '_');
						tileKeyStrings.push_back(subStr);
					}

					int newXkey = stoi(tileKeyStrings[0]) - i;
					int newYkey = stoi(tileKeyStrings[1]) - j;
					int newZkey = stoi(tileKeyStrings[2]) - k;
					string newTileKey = to_string(newXkey) + "_" + to_string(newYkey) + "_" + to_string(newZkey);
					//cout << "surrounding tile: " << newTileKey << endl;
					if (this->nodeTileMap.find(newTileKey) != this->nodeTileMap.end())
					{
						nodeIDs.clear();
						nodeIDs = this->nodeTileMap.at(newTileKey);
						for (auto& nodeID : nodeIDs)
						{
							const NeuronSWC& node = this->tree.listNeuron.at(this->node2LocMap.at(nodeID));
							string nodeCoordKey = to_string(node.x) + "_" + to_string(node.y) + "_" + to_string(node.z);
							if (this->segEndCoordKey2segMap.find(nodeCoordKey) != this->segEndCoordKey2segMap.end())
							{
								pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->segEndCoordKey2segMap.equal_range(nodeCoordKey);
								pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range2 = this->nodeCoordKey2segMap.equal_range(nodeCoordKey);
								if (range.second - range.first == 1 && range2.second - range2.first == 1)
								{
									float currNodeDist = sqrtf((node.x - inputAPO.x) * (node.x - inputAPO.x) + (node.y - inputAPO.y) * (node.y - inputAPO.y) + (node.z - inputAPO.z) * (node.z - inputAPO.z));
									if (currNodeDist < dist)
									{
										outputNodeID = nodeID;
										dist = currNodeDist;
									}
								}
							}
						}
					}
				}
			}
		}

		if (outputNodeID != 0 && dist <= threshold) return outputNodeID;
		else
		{
			//cout << "No nearest node found." << endl;
			for (auto& segEndCoord : this->segEndCoordKey2segMap)
			{
				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->segEndCoordKey2segMap.equal_range(segEndCoord.first);
				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range2 = this->nodeCoordKey2segMap.equal_range(segEndCoord.first);
				if (range.second - range.first == 1 && range2.second - range2.first == 1)
				{
					const segUnit& currSeg = this->segs.at(segEndCoord.second);
					const NeuronSWC& currHeadNode = currSeg.nodes.at(currSeg.seg_nodeLocMap.at(currSeg.head));
					string headCoordKey = to_string(currHeadNode.x) + "_" + to_string(currHeadNode.y) + "_" + to_string(currHeadNode.z);
					if (!headCoordKey.compare(segEndCoord.first)) return currSeg.head;
					else
					{
						for (auto& tailID : currSeg.tails)
						{
							const NeuronSWC& tailNode = currSeg.nodes.at(currSeg.seg_nodeLocMap.at(tailID));
							string tailCoordKey = to_string(tailNode.x) + "_" + to_string(tailNode.y) + "_" + to_string(tailNode.z);
							if (!tailCoordKey.compare(segEndCoord.first)) return tailID;
						}
					}
				}
			}
		}
	}
}

void integratedDataTypes::profiledTree::assembleSegs2singleTree(int rootNodeID)
{
	// This method iteratively calls [this->combSegs] to link all segments to become a single tree structure (segment).

	if (this->segs.size() == 1)
	{
		this->combSegs(rootNodeID);
		this->tree.listNeuron = this->segs.begin()->second.nodes;
		profiledTreeReInit(*this);
		return;
	}

	while (this->segs.size() > 1)
	{
		int currSegSize = this->segs.size();
		this->combSegs(rootNodeID);
		this->tree.listNeuron.clear();
		for (auto& seg : this->segs)
		{
			if (seg.second.to_be_deleted) this->tree.listNeuron.append(this->seg2MiddleBranchingMap.at(seg.first).nodes);
			else this->tree.listNeuron.append(seg.second.nodes);
		}
		NeuronStructUtil::removeDupHeads(this->tree);
		profiledTreeReInit(*this);
		cout << " --> segment number: " << this->segs.size() << endl;
		
		if (this->segs.size() == currSegSize) return;
	}
}

//int end2bodyCount = 0;
void integratedDataTypes::profiledTree::combSegs(int rootNodeID)
{
	if (this->node2segMap.empty()) this->nodeSegMapGen();
	if (this->nodeCoordKey2segMap.empty()) this->nodeCoordKeySegMapGen();
	if (this->segEndCoordKey2segMap.empty()) this->segEndCoordKeySegMapGen();
	if (this->nodeCoordKey2nodeIDMap.empty()) this->nodeCoordKeyNodeIDmapGen();
	int leadingSegID = this->node2segMap.at(rootNodeID);
	set<int> checkedSegIDs = { this->node2segMap.at(rootNodeID) };
	this->rc_reverseSegs(leadingSegID, rootNodeID, checkedSegIDs);

	cout << "original segment number: " << this->segs.size() << endl;
	cout << "checked segment number: " << checkedSegIDs.size() << endl;
	//cout << "total end to body count: " << end2bodyCount << endl;
}

void integratedDataTypes::profiledTree::rc_reverseSegs(const int leadingSegID, const int startingEndNodeID, set<int>& checkedSegIDs)
{
	// Taking the leading segment as upstream, any segments that are attached to it need to be connected with their heads. This is the idea of "combing through".
	// The process goes on recursively until all segments are checked and arranged.
	// -- Note, running this method only once doesn't guarantee that the all connected segments are arranged in the right orientation down the stream. 
	//    This is the reason why [this->assembleSegs2singleTree] calls this method with [this->combSegs] iteratively until all segments have been linked and become 1 single structure (segment).

	checkedSegIDs.insert(leadingSegID);

	// If the given startingEndNodeID is some segment's tail, then the input segment needs to be reversed first.
	if (find(this->segs.at(leadingSegID).tails.begin(), this->segs.at(leadingSegID).tails.end(), startingEndNodeID) != this->segs.at(leadingSegID).tails.end())
		this->segs.find(leadingSegID)->second.reverse(startingEndNodeID);
	const NeuronSWC& leadingSegHeadNode = this->tree.listNeuron.at(this->node2LocMap.at(this->segs.at(leadingSegID).head));
	string leadingSegHeadCoordKey = to_string(leadingSegHeadNode.x) + "_" + to_string(leadingSegHeadNode.y) + "_" + to_string(leadingSegHeadNode.z);

	// ******* Scanning through every node in the segment and see if it's connected with other segment's end ******* //
	for (auto& node : this->segs.at(leadingSegID).nodes)
	{
		string nodeCoordKey = to_string(node.x) + "_" + to_string(node.y) + "_" + to_string(node.z);
		if (this->segEndCoordKey2segMap.find(nodeCoordKey) == this->segEndCoordKey2segMap.end()) continue;

		pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->segEndCoordKey2segMap.equal_range(nodeCoordKey);
		for (boost::container::flat_multimap<string, int>::iterator it = range.first; it != range.second; ++it)
		{
			if (checkedSegIDs.find(it->second) != checkedSegIDs.end()) continue;

			const NeuronSWC& connectedSegHeadNode = this->tree.listNeuron.at(this->node2LocMap.at(this->segs.at(it->second).head));
			string headCoordKey = to_string(connectedSegHeadNode.x) + "_" + to_string(connectedSegHeadNode.y) + "_" + to_string(connectedSegHeadNode.z);
			if (!headCoordKey.compare(nodeCoordKey)) this->rc_reverseSegs(it->second, connectedSegHeadNode.n, checkedSegIDs);
			else
			{
				for (auto& tailID : this->segs.at(it->second).tails)
				{
					const NeuronSWC& connectedSegTailNode = this->tree.listNeuron.at(this->node2LocMap.at(tailID));
					string tailCoordKey = to_string(connectedSegTailNode.x) + "_" + to_string(connectedSegTailNode.y) + "_" + to_string(connectedSegTailNode.z);
					if (!tailCoordKey.compare(nodeCoordKey)) this->rc_reverseSegs(it->second, tailID, checkedSegIDs);
				}
			}	
		}
	}
	// ************************************************************************************************************ //

	// **************** Checking if the current segment ends are attaching to other segment's body **************** //
	const segUnit& curSeg = this->segs.at(leadingSegID);
	const NeuronSWC& headNode = this->tree.listNeuron.at(this->node2LocMap.at(curSeg.head));
	string headCoordKey = to_string(headNode.x) + "_" + to_string(headNode.y) + "_" + to_string(headNode.z);
	pair<boost::container::flat_multimap<string, int>::const_iterator, boost::container::flat_multimap<string, int>::const_iterator> range = this->nodeCoordKey2nodeIDMap.equal_range(headCoordKey);
	for (boost::container::flat_multimap<string, int>::const_iterator it = range.first; it != range.second; ++it)
	{
		if (this->node2segMap.at(it->second) == leadingSegID || checkedSegIDs.find(this->node2segMap.at(it->second)) != checkedSegIDs.end()) continue;
		else
		{
			cout << "---------" << endl;
			cout << "head to body: " << headNode.n << " -> " << it->second << endl;
			cout << "---------" << endl << endl;
			//++end2bodyCount;
			this->rc_reverseSegs(this->node2segMap.at(it->second), it->second, checkedSegIDs);	
		}
	}

	for (auto& tailID : curSeg.tails)
	{
		const NeuronSWC& tailNode = this->tree.listNeuron.at(this->node2LocMap.at(tailID));
		string tailCoordKey = to_string(tailNode.x) + "_" + to_string(tailNode.y) + "_" + to_string(tailNode.z);
		pair <boost::container::flat_multimap<string, int>::const_iterator, boost::container::flat_multimap<string, int>::const_iterator> range = this->nodeCoordKey2nodeIDMap.equal_range(tailCoordKey);
		for (boost::container::flat_multimap<string, int>::const_iterator it = range.first; it != range.second; ++it)
		{
			if (this->node2segMap.at(it->second) == leadingSegID || checkedSegIDs.find(this->node2segMap.at(it->second)) != checkedSegIDs.end()) continue;
			else
			{
				cout << "---------" << endl;
				cout << "tail to body: " << tailNode.n << " -> " << it->second << endl;
				//++end2bodyCount;
				this->seg2MiddleBranchingMap.insert(this->splitSegWithMiddleHead(this->segs.at(this->node2segMap.at(it->second)), it->second));
				cout << "---------" << endl << endl;
				this->segs[this->node2segMap.at(it->second)].to_be_deleted = true;
				this->rc_reverseSegs(this->node2segMap.at(it->second), it->second, checkedSegIDs);			
			}
		}
	}
	// ************************************************************************************************************ //
}

pair<int, segUnit> integratedDataTypes::profiledTree::splitSegWithMiddleHead(const segUnit& inputSeg, int newHeadID)
{
	cout << "Full segment node list:" << endl;
	for (auto& node : inputSeg.nodes) cout << node.n << " " << node.parent << endl;
	cout << endl;

	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	const NeuronSWC& startingNode = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(newHeadID));
	QList<NeuronSWC> downstreamNodes, upstreamNodes;
	
	NeuronStructUtil::splitNodeList(inputSeg.nodes, newHeadID, downstreamNodes, upstreamNodes);
	downstreamNodes.front().parent = -1;
	cout << "downstream node list:" << endl;
	for (auto& node : downstreamNodes) cout << node.n << " " << node.parent << endl;
	cout << endl;

	int newHeadPaID = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(newHeadID)).parent;
	cout << "upstream new head ID: " << newHeadPaID << endl;
	NeuronTree inputSegTree, downstreamSegTree;
	inputSegTree.listNeuron = inputSeg.nodes;
	segUnit upstreamSeg(upstreamNodes);
	
	//cout << "upstream nodes: " << endl;
	//for (auto& node : upstreamSeg.nodes) cout << node.n << " ";
	//cout << endl << endl;
	
	upstreamSeg.reverse(newHeadPaID);
	upstreamSeg.nodes.begin()->parent = newHeadID;
	cout << "reversed upstream node list:" << endl;
	for (auto& node : upstreamSeg.nodes) cout << node.n << " " << node.parent << endl;

	downstreamNodes.append(upstreamSeg.nodes);
	segUnit newSeg(downstreamNodes);
	
	return pair<int, segUnit>({ inputSeg.segID, newSeg });
}

void integratedDataTypes::profiledTree::nodeSegMapGen()
{
	this->nodeSegMapGen(this->segs, this->node2segMap);
}

void integratedDataTypes::profiledTree::nodeSegMapGen(const map<int, segUnit>& segMap, boost::container::flat_map<int, int>& node2segMap)
{
	for (map<int, segUnit>::const_iterator segIt = segMap.begin(); segIt != segMap.end(); ++segIt)
	{
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
			node2segMap.insert(pair<int, int>(nodeIt->n, segIt->first));
	}
}

void integratedDataTypes::profiledTree::nodeCoordKeySegMapGen()
{
	this->nodeCoordKeySegMapGen(this->segs, this->nodeCoordKey2segMap);
}

void integratedDataTypes::profiledTree::nodeCoordKeySegMapGen(const map<int, segUnit>& segMap, boost::container::flat_multimap<string, int>& nodeCoordKey2segMap)
{
	for (map<int, segUnit>::const_iterator segIt = segMap.begin(); segIt != segMap.end(); ++segIt)
	{
		for (QList<NeuronSWC>::const_iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			string nodeCoordKey = to_string(nodeIt->x) + "_" + to_string(nodeIt->y) + "_" + to_string(nodeIt->z);
			nodeCoordKey2segMap.insert(pair<string, int>(nodeCoordKey, segIt->first));
		}
	}
}

void integratedDataTypes::profiledTree::segEndCoordKeySegMapGen()
{
	this->segEndCoordKeySegMapGen(this->segs, this->segEndCoordKey2segMap);
}

void integratedDataTypes::profiledTree::segEndCoordKeySegMapGen(const map<int, segUnit>& segMap, boost::container::flat_multimap<string, int>& segEndCoordKey2segMap)
{
	for (map<int, segUnit>::const_iterator segIt = segMap.begin(); segIt != segMap.end(); ++segIt)
	{
		const NeuronSWC& headNode = this->tree.listNeuron.at(this->node2LocMap.at(segIt->second.head));
		string headNodeCoordKey = to_string(headNode.x) + "_" + to_string(headNode.y) + "_" + to_string(headNode.z);
		segEndCoordKey2segMap.insert(pair<string, int>(headNodeCoordKey, segIt->first));

		for (auto& tailID : segIt->second.tails)
		{
			const NeuronSWC& tailNode = this->tree.listNeuron.at(this->node2LocMap.at(tailID));
			string tailNodeCoordKey = to_string(tailNode.x) + "_" + to_string(tailNode.y) + "_" + to_string(tailNode.z);
			segEndCoordKey2segMap.insert(pair<string, int>(tailNodeCoordKey, segIt->first));
		}
	}
}

void integratedDataTypes::profiledTree::nodeCoordKeyNodeIDmapGen()
{
	this->nodeCoordKeyNodeIDmapGen(this->tree.listNeuron, this->nodeCoordKey2nodeIDMap);
}

void integratedDataTypes::profiledTree::nodeCoordKeyNodeIDmapGen(const QList<NeuronSWC>& nodeList, boost::container::flat_multimap<string, int>& nodeCoordKey2nodeIDmap)
{
	for (QList<NeuronSWC>::const_iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
	{
		string nodeCoordKey = to_string(nodeIt->x) + "_" + to_string(nodeIt->y) + "_" + to_string(nodeIt->z);
		nodeCoordKey2nodeIDMap.insert(pair<string, int>(nodeCoordKey, nodeIt->n));
	}
}

void integratedDataTypes::profiledTree::overlappedCoordMapGen()
{
	// Figuring how segments are spatially involved with a given coordinate is mostly for the purpose of linking segments.
	// Therefore, the search is done by examining each segment's head and tail nodes.

	if (this->segs.empty()) integratedDataTypes::profiledTreeReInit(*this);
	this->nodeCoordKeySegMapGen();

	this->overlappedCoordMap.clear();
	for (auto& headSegTile : this->segHeadMap)
	{
		for (auto& segHead : headSegTile.second)
		{
			const NeuronSWC& headNode = this->tree.listNeuron.at(this->node2LocMap.at(this->segs.at(segHead).head));
			string headNodeCoordKey = to_string(headNode.x) + "_" + to_string(headNode.y) + "_" + to_string(headNode.z);
			pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->nodeCoordKey2segMap.equal_range(headNodeCoordKey);

			if (range.second - range.first == 1) continue;
			else
			{
				integratedDataTypes::overlappedCoord currCoord(headNode.x, headNode.y, headNode.z);
				for (boost::container::flat_multimap<string, int>::iterator it = range.first; it != range.second; ++it)
				{
					const segUnit& seg = this->segs.at(it->second);

					const NeuronSWC& currHeadNode = this->tree.listNeuron.at(this->node2LocMap.at(seg.head));
					if (headNode.x == currHeadNode.x && headNode.y == currHeadNode.y && headNode.z == currHeadNode.z)
					{
						currCoord.involvedSegsOriMap[integratedDataTypes::head].insert(pair<int, int>(seg.segID, currHeadNode.n));
						goto NODE_REGISTERED1;
					}

					for (auto& tail : seg.tails)
					{
						const NeuronSWC& tailNode = this->tree.listNeuron.at(this->node2LocMap.at(tail));
						if (tailNode.x == headNode.x && tailNode.y == headNode.y && tailNode.z == headNode.z)
						{
							currCoord.involvedSegsOriMap[integratedDataTypes::tail].insert(pair<int, int>(seg.segID, tailNode.n));
							goto NODE_REGISTERED1;
						}
					}

					for (auto& bodyNode : seg.nodes)
					{
						if (bodyNode.x == headNode.x && bodyNode.y == headNode.y && bodyNode.z == headNode.z)
						{
							currCoord.involvedSegsOriMap[integratedDataTypes::body].insert(pair<int, int>(seg.segID, bodyNode.n));
							break;
						}
					}

				NODE_REGISTERED1:
					continue;
				}

				this->overlappedCoordMap.insert(pair<string, integratedDataTypes::overlappedCoord>(headNodeCoordKey, currCoord));
			}
		}
	}

	for (auto& tailSegTile : this->segTailMap)
	{
		for (auto& segTail : tailSegTile.second)
		{
			for (auto& tailID : this->segs.at(segTail).tails)
			{
				const NeuronSWC& tailNode = this->tree.listNeuron.at(this->node2LocMap.at(tailID));
				string tailNodeCoordKey = to_string(tailNode.x) + "_" + to_string(tailNode.y) + "_" + to_string(tailNode.z);
				if (this->overlappedCoordMap.find(tailNodeCoordKey) != this->overlappedCoordMap.end()) continue;

				pair<boost::container::flat_multimap<string, int>::iterator, boost::container::flat_multimap<string, int>::iterator> range = this->nodeCoordKey2segMap.equal_range(tailNodeCoordKey);
				if (range.second - range.first == 1) continue;
				else
				{
					integratedDataTypes::overlappedCoord currCoord(tailNode.x, tailNode.y, tailNode.z);
					for (boost::container::flat_multimap<string, int>::iterator it = range.first; it != range.second; ++it)
					{
						const segUnit& seg = this->segs.at(it->second);

						for (auto& tail : seg.tails)
						{
							const NeuronSWC& currTailNode = this->tree.listNeuron.at(this->node2LocMap.at(tail));
							if (tailNode.x == currTailNode.x && tailNode.y == currTailNode.y && tailNode.z == currTailNode.z)
							{
								currCoord.involvedSegsOriMap[integratedDataTypes::tail].insert(pair<int, int>(seg.segID, tailNode.n));
								goto NODE_REGISTERED2;
							}
						}

						for (auto& bodyNode : seg.nodes)
						{
							if (bodyNode.x == tailNode.x && bodyNode.y == tailNode.y && bodyNode.z == tailNode.z)
							{
								currCoord.involvedSegsOriMap[integratedDataTypes::body].insert(pair<int, int>(seg.segID, bodyNode.n));
								break;
							}
						}

					NODE_REGISTERED2:
						continue;
					}

					this->overlappedCoordMap.insert(pair<string, integratedDataTypes::overlappedCoord>(tailNodeCoordKey, currCoord));
				}
			}
		}
	}
}

void integratedDataTypes::profiledTree::getSegEndClusterNodeMap()
{
	if (this->segHeadClusters.empty() && this->segTailClusters.empty())
	{
		cerr << "Segment end clusters are empty. Do nothing and return empty map." << endl;
		return;
	}

	for (auto& headCluster : this->segHeadClusters)
	{
		boost::container::flat_set<vector<float>> headCoords;
		for (auto& headSeg : headCluster.second)
		{
			const NeuronSWC headNode = this->tree.listNeuron.at(this->node2LocMap.at(this->segs.at(headSeg).head));
			vector<float> headCoord = { headNode.x, headNode.y, headNode.z };
			headCoords.insert(headCoord);
		}

		this->segEndClusterNodeMap.insert(pair<int, boost::container::flat_set<vector<float>>>(headCluster.first, headCoords));
	}

	for (auto& tailCluster : this->segTailClusters)
	{
		boost::container::flat_set<vector<float>> tailCoords;
		for (auto& tailSeg : tailCluster.second)
		{
			for (auto& tailID : this->segs.at(tailSeg).tails)
			{
				const NeuronSWC tailNode = this->tree.listNeuron.at(this->node2LocMap.at(tailID));
				vector<float> tailCoord = { tailNode.x, tailNode.y, tailNode.z };
				tailCoords.insert(tailCoord);
			}
		}

		if (this->segEndClusterNodeMap.find(tailCluster.first) == this->segEndClusterNodeMap.end()) 
			this->segEndClusterNodeMap.insert(pair<int, boost::container::flat_set<vector<float>>>(tailCluster.first, tailCoords));
		else this->segEndClusterNodeMap[tailCluster.first].insert(tailCoords.begin(), tailCoords.end());
	}

	// ------- For debug purpose ------- //
	/*for (auto& cluster : this->segEndClusterNodeMap)
	{
		cout << "cluster " << cluster.first << ": " << endl;
		for (auto& node : cluster.second)
			cout << "(" << node.at(0) << ", " << node.at(1) << ", " << node.at(2) << ")  ";
		cout << endl;
	}*/
}

void integratedDataTypes::profiledTree::getSegEndClusterCentoirds()
{
	if (this->segEndClusterNodeMap.empty()) this->getSegEndClusterNodeMap();
	
	for (auto& segEndCluster : this->segEndClusterNodeMap)
	{
		vector<float> centroid(3);
		this->segEndClusterCentroidMap.insert(pair<int, vector<float>>(segEndCluster.first, centroid));
		integratedDataTypes::segEndClusterCentroid(segEndCluster.second, this->segEndClusterCentroidMap[segEndCluster.first]);
	}

	/*if (!NSlibTester::isInstantiated())
	{
		NSlibTester::instance();
		NSlibTester::getInstance()->checkClusterNodeMap(*this, "D:\\Work\\FragTrace\\");
		NSlibTester::getInstance()->checkClusterCentroidMap(*this, "D:\\Work\\FragTrace\\");
		NSlibTester::uninstance();
	}*/
}

void integratedDataTypes::profiledTree::addTopoUnit(int nodeID)
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

void integratedDataTypes::profiledTreeReInit(profiledTree& inputProfiledTree)
{
	profiledTree tempTree(inputProfiledTree.tree, inputProfiledTree.nodeTileSize, inputProfiledTree.segTileSize);
	inputProfiledTree = tempTree; // Copy tempTree to the memory where [inputProfiledTree] refers to -- OK.
}

integratedDataTypes::segEndClusterUnit::~segEndClusterUnit()
{
	for (map<int, segEndClusterUnit*>::iterator it = this->childClusterMap.begin(); it != this->childClusterMap.end(); ++it)
		it->second->parentCluster = nullptr;

	if (this->parentCluster != nullptr)
		this->parentCluster->childClusterMap.erase(this->parentCluster->childClusterMap.find(this->ID));
}

void integratedDataTypes::cleanUp_segEndClusterChain_downStream(segEndClusterUnit* currCluster)
{
	if (currCluster->childClusterMap.empty())
	{
		delete currCluster;
		return;
	}

	for (map<int, segEndClusterUnit*>::iterator it = currCluster->childClusterMap.begin(); it != currCluster->childClusterMap.end(); ++it)
		integratedDataTypes::cleanUp_segEndClusterChain_downStream(it->second);
}