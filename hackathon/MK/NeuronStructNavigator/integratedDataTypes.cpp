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
#include "NeuronStructNavigatingTester.h"

using namespace std;
using NSlibTester = NeuronStructNavigator::Tester;

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
	this->head = this->nodes.begin()->n;
	
	/******************************************************************************************************/
	// Important Note: 
	//   [segUnit.seg_childLocMap] does NOT have tail node registered freshly coming out of [NeuronStructUtil::node2loc_node2childLocMap].
	//   Adding tail nodes into the map with empty child locations here for the purpose of 
	//   avoiding memory violations by accessing non-existent pair in the map.
	NeuronStructUtil::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);
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

void integratedDataTypes::segUnit::reInit(segUnit& inputSegUnit)
{
	segUnit reInitUnit(inputSegUnit.nodes);
	reInitUnit.segID = inputSegUnit.segID;
	inputSegUnit = reInitUnit;
}

V_NeuronSWC integratedDataTypes::segUnit::convert2V_NeuronSWC() const
{
	V_NeuronSWC outputV_NeuronSWC;
	outputV_NeuronSWC.to_be_deleted = this->to_be_deleted;
	int paNodeID = this->head;
	this->rc_nodeRegister2V_NeuronSWC(outputV_NeuronSWC, paNodeID, paNodeID);

	return outputV_NeuronSWC;
}

void integratedDataTypes::segUnit::rc_nodeRegister2V_NeuronSWC(V_NeuronSWC& sbjV_NeuronSWC, int parentID, int branchRootID) const
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
			for (vector<size_t>::const_iterator tailsIt = this->seg_childLocMap.at(currentPaID).begin(); tailsIt != this->seg_childLocMap.at(currentPaID).end(); ++tailsIt)
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

		NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		NeuronStructUtil::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

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

	if (!NSlibTester::isInstantiated())
	{
		NSlibTester::instance();
		NSlibTester::getInstance()->checkClusterNodeMap(*this, "D:\\Work\\FragTrace\\");
		NSlibTester::getInstance()->checkClusterCentroidMap(*this, "D:\\Work\\FragTrace\\");
		NSlibTester::uninstance();
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