//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [integratedDataTypes] is part of the NeuronStructNavigator library.
*  The namespace manages all integrated data structures used by all other NeuronStructNavigator classes.
*  All data structures in this namespace are integrated with standard Vaa3D data types with additional features, aiming to make developing neuron structure operations and algorithms more convenient.
*  Any new development on the datatypes should be put in this namespace to keep them organized and avoid the confusion of header inclusion.
*
*  [profiledTree] is the core data type in throughout the whole NeuronStructNavigator library. It profiles the NeuronTree and carries crucial information of it.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

********************************************************************************/

#include <iostream>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

using namespace std;

integratedDataTypes::segUnit::segUnit(const V_NeuronSWC& inputV_NeuronSWC) : to_be_deleted(false)
{
	for (vector<V_NeuronSWC_unit>::const_iterator nodeIt = inputV_NeuronSWC.row.begin(); nodeIt != inputV_NeuronSWC.row.end(); ++nodeIt)
	{
		NeuronSWC node;
		node.n = nodeIt->n;
		node.x = nodeIt->x;
		node.y = nodeIt->y;
		node.z = nodeIt->z;
		node.type = nodeIt->type;
		node.parent = nodeIt->parent;
		this->nodes.push_front(node);
	}
	NeuronStructUtil::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);

	this->head = this->nodes.begin()->n;
	for (QList<NeuronSWC>::iterator nodeIt = this->nodes.begin(); nodeIt != this->nodes.end(); ++nodeIt)
	{
		if (this->seg_childLocMap.find(nodeIt->n) == this->seg_childLocMap.end())
		{
			this->tails.push_back(nodeIt->n);
			vector<size_t> emptyTailSet;
			seg_childLocMap.insert({ nodeIt->n, emptyTailSet });
		}
	}
	
	this->to_be_deleted = inputV_NeuronSWC.to_be_deleted;
}

integratedDataTypes::segUnit::segUnit(const QList<NeuronSWC>& inputSeg) : to_be_deleted(false)
{
	this->nodes = inputSeg;
	NeuronStructUtil::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);

	this->head = this->nodes.begin()->n;
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

V_NeuronSWC integratedDataTypes::segUnit::convert2V_NeuronSWC()
{
	V_NeuronSWC outputV_NeuronSWC;
	outputV_NeuronSWC.to_be_deleted = this->to_be_deleted;
	int paNodeID = this->head;
	this->rc_nodeRegister2V_NeuronSWC(outputV_NeuronSWC, paNodeID, paNodeID);

	return outputV_NeuronSWC;
}

void integratedDataTypes::segUnit::rc_nodeRegister2V_NeuronSWC(V_NeuronSWC& sbjV_NeuronSWC, int parentID, int branchRootID)
{
	int currentPaID = parentID;
	while (1)
	{
		V_NeuronSWC_unit newNodeV;
		newNodeV.n = this->nodes.size() - sbjV_NeuronSWC.row.size();
		newNodeV.x = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).x;
		newNodeV.y = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).y;
		newNodeV.z = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).z;
		newNodeV.type = this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).type;		
		newNodeV.seg_id = this->segID;		

		if (this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent == -1) newNodeV.parent = -1;
		else
		{
			if (this->seg_childLocMap.at(this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent).size() > 1)
				newNodeV.parent = branchRootID;
			else if (this->seg_childLocMap.at(this->nodes.at(this->seg_nodeLocMap.at(currentPaID)).parent).size() == 1)
				newNodeV.parent = newNodeV.n + 1;
		}
		sbjV_NeuronSWC.row.insert(sbjV_NeuronSWC.row.begin(), newNodeV);

		if (this->seg_childLocMap.at(currentPaID).size() == 1) currentPaID = this->nodes.at(*this->seg_childLocMap.at(currentPaID).begin()).n;
		else if (this->seg_childLocMap.at(currentPaID).size() == 0) return;
		else if (this->seg_childLocMap.at(currentPaID).size() > 1)
		{
			for (vector<size_t>::iterator tailsIt = this->seg_childLocMap.at(currentPaID).begin(); tailsIt != this->seg_childLocMap.at(currentPaID).end(); ++tailsIt)
				this->rc_nodeRegister2V_NeuronSWC(sbjV_NeuronSWC, this->nodes.at(*tailsIt).n, newNodeV.n);
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
	
	this->turning12(connOrt);
	this->segsAngleDiff12(connOrt);
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
	
	if (inputTree.listNeuron.empty()) cerr << "The input tree is empty, profiledTree cannot be initialized." << endl;
	else
	{
		this->tree = inputTree;
		this->segTileSize = segTileLength;
		this->nodeTileSize = nodeTileLength;

		NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		NeuronStructUtil::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

		this->segs = NeuronStructExplorer::findSegs(this->tree.listNeuron, this->node2childLocMap);
		//cout << "segs num: " << this->segs.size() << endl;

		NeuronStructUtil::nodeSegMapGen(this->segs, this->node2segMap);

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

void integratedDataTypes::profiledTree::nodeTileResize(float nodeTileLength)
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
	profiledTree tempTree(inputProfiledTree.tree, inputProfiledTree.segTileSize);
	inputProfiledTree = tempTree;
}