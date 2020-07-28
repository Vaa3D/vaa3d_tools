#include "NeuronStructNavigatingTester.h"

#include "FragTracer_Define.h"
#include "FragmentEditor.h"
#include "FragTraceTester.h"

using namespace std;
using namespace integratedDataTypes;
using NSlibTester = NeuronStructNavigator::Tester;

void FragmentEditor::connectingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap)
{
	//set<int> segIDs;
	//for (auto& idPair : seg2includedNodeMap) segIDs.insert(idPair.first);
	//FragTraceTester::getInstance()->printOutEditSegInfo(displayingSegs, segIDs);

	if (seg2includedNodeMap.size() != 2)
	{
		cout << "There must be 2 segments to proceed the operation. (" << seg2includedNodeMap.size() << " segments included). Do nothing." << endl;
		return;
	}

	profiledTree currDisplayProfiledTree(displayingSegs.seg);
	vector<pair<int, connectOrientation>> segConnectOris;
	int connectCase = 0;
	for (map<int, vector<NeuronSWC>>::const_iterator segIt = seg2includedNodeMap.begin(); segIt != seg2includedNodeMap.end(); ++segIt)
	{
		const NeuronSWC& headNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(currDisplayProfiledTree.segs.at(segIt->first).head));
		for (vector<NeuronSWC>::const_iterator nodeIt = segIt->second.begin(); nodeIt != segIt->second.end(); ++nodeIt)
		{
			if (nodeIt->x == headNode.x && nodeIt->y == headNode.y && nodeIt->z == headNode.z)
			{
				segConnectOris.push_back({ segIt->first, head });
				++connectCase;
				cout << " -- head" << endl;
				goto END_SEG_FOUND;
			}
			else
			{
				for (vector<int>::const_iterator tailIt = currDisplayProfiledTree.segs.at(segIt->first).tails.begin(); tailIt != currDisplayProfiledTree.segs.at(segIt->first).tails.end(); ++tailIt)
				{
					const NeuronSWC& tailNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(*tailIt));
					if (nodeIt->x == tailNode.x && nodeIt->y == tailNode.y && nodeIt->z == tailNode.z)
					{
						segConnectOris.push_back({ segIt->first, tail });
						++connectCase;
						cout << " -- tail" << endl;
						goto END_SEG_FOUND;
					}
				}
			}
		}
		segConnectOris.push_back({ segIt->first, body });
		connectCase += 2;
		cout << " -- body" << endl;

	END_SEG_FOUND:
		continue;
	}

	//vector<segUnit> segUnits = { currDisplayProfiledTree.segs.at(segConnectOris.at(0).first), currDisplayProfiledTree.segs.at(segConnectOris.at(1).first) };
	//FragTraceTester::getInstance()->printOutEditSegInfo(segUnits);
	segUnit newSeg;
	int bodySegID, segEndNodeID;
	if (connectCase == 2)
	{
		connectOrientation connOri;
		if (segConnectOris.at(0).second == head && segConnectOris.at(1).second == tail) connOri = head_tail;
		else if (segConnectOris.at(0).second == tail && segConnectOris.at(1).second == head) connOri = tail_head;
		else if (segConnectOris.at(0).second == head && segConnectOris.at(1).second == head) connOri = head_head;
		else if (segConnectOris.at(0).second == tail && segConnectOris.at(1).second == tail) connOri = tail_tail;
		newSeg = NeuronStructUtil::segUnitConnect_end2end(currDisplayProfiledTree.segs.at(segConnectOris.at(0).first), currDisplayProfiledTree.segs.at(segConnectOris.at(1).first), connOri);
	
		displayingSegs.seg[segConnectOris.at(0).first].to_be_deleted = true;
		displayingSegs.seg[segConnectOris.at(1).first].to_be_deleted = true;
	}
	else if (connectCase == 3)
	{
		vector<float> segEndPointingVec(3);		
		if (segConnectOris.at(0).second == head)
		{
			bodySegID = segConnectOris.at(1).first;
			segEndNodeID = currDisplayProfiledTree.segs.at(segConnectOris.at(0).first).head;
			const NeuronSWC& endNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(segEndNodeID));
			segEndPointingVec = this->getSegEndPointingVec(currDisplayProfiledTree.segs.at(segConnectOris.at(0).first), segEndNodeID, 4);
			NeuronSWC connBodyNode = this->mostProbableBodyNode(segEndPointingVec, seg2includedNodeMap.at(segConnectOris.at(1).first), endNode);
			newSeg = currDisplayProfiledTree.segs.at(segConnectOris.at(0).first);
			newSeg.nodes.push_front(connBodyNode);

			displayingSegs.seg[segConnectOris.at(0).first].to_be_deleted = true;
		}
		else if (segConnectOris.at(0).second == tail)
		{
			bodySegID = segConnectOris.at(1).first;
			for (auto& tail : currDisplayProfiledTree.segs.at(segConnectOris.at(0).first).tails)
			{
				const NeuronSWC& tailNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(tail));
				for (vector<NeuronSWC>::const_iterator nodeIt = seg2includedNodeMap.at(segConnectOris.at(0).first).begin(); nodeIt != seg2includedNodeMap.at(segConnectOris.at(0).first).end(); ++nodeIt)
				{
					if (tailNode.x == nodeIt->x && tailNode.y == nodeIt->y && tailNode.z == nodeIt->z)
					{
						segEndNodeID = tail;
						goto TAIL_FOUND_0;
					}
				}
			}

		TAIL_FOUND_0:
			const NeuronSWC& endNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(segEndNodeID));
			segEndPointingVec = this->getSegEndPointingVec(currDisplayProfiledTree.segs.at(segConnectOris.at(0).first), segEndNodeID, 4);
			NeuronSWC connBodyNode = this->mostProbableBodyNode(segEndPointingVec, seg2includedNodeMap.at(segConnectOris.at(1).first), endNode);
			newSeg = currDisplayProfiledTree.segs.at(segConnectOris.at(0).first);
			newSeg.nodes.push_back(connBodyNode);

			displayingSegs.seg[segConnectOris.at(0).first].to_be_deleted = true;
		}
		else if (segConnectOris.at(1).second == head)
		{
			bodySegID = segConnectOris.at(0).first;
			segEndNodeID = currDisplayProfiledTree.segs.at(segConnectOris.at(1).first).head;
			const NeuronSWC& endNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(segEndNodeID));
			segEndPointingVec = this->getSegEndPointingVec(currDisplayProfiledTree.segs.at(segConnectOris.at(1).first), segEndNodeID, 4);
			NeuronSWC connBodyNode = this->mostProbableBodyNode(segEndPointingVec, seg2includedNodeMap.at(segConnectOris.at(0).first), endNode);
			newSeg = currDisplayProfiledTree.segs.at(segConnectOris.at(1).first);
			newSeg.nodes.push_front(connBodyNode);

			displayingSegs.seg[segConnectOris.at(1).first].to_be_deleted = true;
		}
		else if (segConnectOris.at(1).second == tail)
		{
			bodySegID = segConnectOris.at(0).first;
			for (auto& tail : currDisplayProfiledTree.segs.at(segConnectOris.at(1).first).tails)
			{
				const NeuronSWC& tailNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(tail));
				for (vector<NeuronSWC>::const_iterator nodeIt = seg2includedNodeMap.at(segConnectOris.at(1).first).begin(); nodeIt != seg2includedNodeMap.at(segConnectOris.at(1).first).end(); ++nodeIt)
				{
					if (tailNode.x == nodeIt->x && tailNode.y == nodeIt->y && tailNode.z == nodeIt->z)
					{
						segEndNodeID = tail;
						goto TAIL_FOUND_1;
					}
				}
			}

		TAIL_FOUND_1:
			const NeuronSWC& endNode = currDisplayProfiledTree.tree.listNeuron.at(currDisplayProfiledTree.node2LocMap.at(segEndNodeID));
			segEndPointingVec = this->getSegEndPointingVec(currDisplayProfiledTree.segs.at(segConnectOris.at(1).first), segEndNodeID, 4);
			NeuronSWC connBodyNode = this->mostProbableBodyNode(segEndPointingVec, seg2includedNodeMap.at(segConnectOris.at(0).first), endNode);
			newSeg = currDisplayProfiledTree.segs.at(segConnectOris.at(1).first);
			newSeg.nodes.push_back(connBodyNode);

			displayingSegs.seg[segConnectOris.at(1).first].to_be_deleted = true;
		}
	}
	else if (connectCase == 4)
	{
		cout << "Segment body to segment body connection is prohibited." << endl;
		return;
	}

	if (!newSeg.nodes.isEmpty())
	{
		newSeg.nodes.begin()->n = 1;
		newSeg.nodes.begin()->parent = -1;
		for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin() + 1; it != newSeg.nodes.end(); ++it)
		{
			it->n = int(it - newSeg.nodes.begin()) + 1;
			it->parent = (it - 1)->n;
		}
		newSeg.reInit(newSeg);
		newSeg.segID = displayingSegs.seg.size() + 1;

		V_NeuronSWC newDisplaySeg = newSeg.convert2V_NeuronSWC();
		set<int> nodeTypes = this->getNodeTypesInSeg(newDisplaySeg);
		if (nodeTypes.size() == 1)
		{
			for (auto& node : newDisplaySeg.row) node.data[1] = *nodeTypes.begin();
			
			if (connectCase == 3)
				for (auto& node : displayingSegs.seg[bodySegID].row) node.data[1] = *nodeTypes.begin();

			newDisplaySeg.to_be_deleted = false;
			displayingSegs.seg.push_back(newDisplaySeg);

			if (this->sequentialTypeToggled)
				this->sequencialTypeChanging(displayingSegs, displayingSegs.seg.size() - 1, (displayingSegs.seg.end() - 1)->row.begin()->type);
		}
		else
		{
			newDisplaySeg.to_be_deleted = false;
			displayingSegs.seg.push_back(newDisplaySeg);
		}
	}
}

vector<float> FragmentEditor::getSegEndPointingVec(const segUnit& inputSeg, const int endNodeID, int nodeNum)
{
	//FragTraceTester::getInstance()->printOutTerminalSegInfo(inputSeg);

	vector<float> outputVec(3);
	
	if (inputSeg.nodes.isEmpty())
	{
		cout << "Invalid input segment unit. Return empty vector." << endl;
		return outputVec;
	}

	if (endNodeID == inputSeg.head)
	{
		int nodeCount = 1, paNodeID = endNodeID, childNodeID;
		const NeuronSWC& headNode = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(endNodeID));
		while (nodeCount <= nodeNum)
		{ 
			if (inputSeg.seg_childLocMap.at(paNodeID).empty() || inputSeg.seg_childLocMap.at(paNodeID).size() > 1) // branching or segment tail
			{
				outputVec[0] = headNode.x - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).x;
				outputVec[1] = headNode.y - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).y;
				outputVec[2] = headNode.z - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).z;
				return outputVec;
			}
			paNodeID = inputSeg.nodes.at(*inputSeg.seg_childLocMap.at(paNodeID).begin()).n;
			++nodeCount;
		}	
		outputVec[0] = headNode.x - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).x;
		outputVec[1] = headNode.y - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).y;
		outputVec[2] = headNode.z - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).z;

		return outputVec;
	}
	else
	{
		int nodeCount = 1, childNodeID = endNodeID, paNodeID;
		const NeuronSWC& tailNode = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(endNodeID));
		while (nodeCount <= nodeNum)
		{
			paNodeID = inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(childNodeID)).parent;
			++nodeCount;
			if (inputSeg.seg_childLocMap.at(paNodeID).size() > 1 ||
				inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).parent == -1)
			{
				outputVec[0] = tailNode.x - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).x;
				outputVec[1] = tailNode.y - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).y;
				outputVec[2] = tailNode.z - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).z;
				return outputVec;
			}
		}
		outputVec[0] = tailNode.x - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).x;
		outputVec[1] = tailNode.y - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).y;
		outputVec[2] = tailNode.z - inputSeg.nodes.at(inputSeg.seg_nodeLocMap.at(paNodeID)).z;

		return outputVec;
	}
}

NeuronSWC FragmentEditor::mostProbableBodyNode(const vector<float>& segEndPointingVec, const vector<NeuronSWC>& bodyNodes, const NeuronSWC& endNode)
{
	float radAngle = 100000;
	NeuronSWC outputNode;
	for (vector<NeuronSWC>::const_iterator nodeIt = bodyNodes.begin(); nodeIt != bodyNodes.end(); ++nodeIt)
	{
		vector<float> bodyEndVec = NeuronGeoGrapher::getVector_NeuronSWC<float>(endNode, *nodeIt);
		float currRadAngle = NeuronGeoGrapher::getRadAngle(bodyEndVec, segEndPointingVec);
		if (currRadAngle < radAngle)
		{
			radAngle = currRadAngle;
			outputNode = *nodeIt;
		}
	}

	return outputNode;
}

set<int> FragmentEditor::getNodeTypesInSeg(const V_NeuronSWC& inputVneuronSWC)
{
	set<int> outputNodeTypes;
	for (auto& node : inputVneuronSWC.row)
	{
		if (node.data[1] == 16 || node.data[1] == 18) continue;
		else outputNodeTypes.insert(node.data[1]);
	}

	return outputNodeTypes;
}

void FragmentEditor::erasingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap)
{
	map<int, set<int>> outputEditingSegInfo;
	for (map<int, vector<NeuronSWC>>::const_iterator segIt = seg2includedNodeMap.begin(); segIt != seg2includedNodeMap.end(); ++segIt)
	{
		displayingSegs.seg[segIt->first].to_be_deleted = true;

		set<int> newSet;
		outputEditingSegInfo.insert({ segIt->first, newSet });
		for (vector<NeuronSWC>::const_iterator nodeIt = segIt->second.begin(); nodeIt != segIt->second.end(); ++nodeIt)
		{
			for (auto& Vnode : displayingSegs.seg.at(segIt->first).row)
			{
				if (Vnode.data[2] == nodeIt->x && Vnode.data[3] == nodeIt->y && Vnode.data[4] == nodeIt->z)
					outputEditingSegInfo[segIt->first].insert(Vnode.data[0]);
			}
		}
	}

	for (auto& seg : outputEditingSegInfo)
	{
		cout << "seg ID - " << seg.first << ": ";
		for (auto& node : seg.second) cout << node << " ";
		cout << endl;
	}


 	this->erasingProcess_cuttingSeg(displayingSegs, outputEditingSegInfo);
}

void FragmentEditor::erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo)
{
	//FragTraceTester::getInstance()->printOutEditSegInfo(displayingSegs, seg2BeditedInfo);

	for (map<int, set<int>>::const_iterator editIt = seg2BeditedInfo.begin(); editIt != seg2BeditedInfo.end(); ++editIt)
	{
		segUnit targetSegUnit(displayingSegs.seg.at(editIt->first));
		vector<ptrdiff_t> delLocs;
		for (set<int>::const_iterator nodeIt = editIt->second.begin(); nodeIt != editIt->second.end(); ++nodeIt)
		{
			delLocs.push_back(targetSegUnit.seg_nodeLocMap.at(*nodeIt));
			for (vector<size_t>::iterator childIt = targetSegUnit.seg_childLocMap.at(*nodeIt).begin(); childIt != targetSegUnit.seg_childLocMap.at(*nodeIt).end(); ++childIt)
				targetSegUnit.nodes[*childIt].parent = -1;
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
			targetSegUnit.nodes.erase(targetSegUnit.nodes.begin() + *delIt);

		map<int, size_t> node2LocMap;
		map<int, vector<size_t>> node2childLocMap;
		NeuronStructExplorer::node2loc_node2childLocMap(targetSegUnit.nodes, node2LocMap, node2childLocMap);
		vector<QList<NeuronSWC>> nodeLists;
		QList<NeuronSWC> nodeList;
		for (QList<NeuronSWC>::iterator it = targetSegUnit.nodes.begin(); it != targetSegUnit.nodes.end(); ++it)
		{
			nodeList.push_back(*it);
			if (node2childLocMap.find(it->n) == node2childLocMap.end())
			{
				nodeLists.push_back(nodeList);
				nodeList.clear();
			}
		}

		for (vector<QList<NeuronSWC>>::iterator listIt = nodeLists.begin(); listIt != nodeLists.end(); ++listIt)
		{
			segUnit trimmedSegUnit(*listIt);
			V_NeuronSWC newV_NeuronSWC = trimmedSegUnit.convert2V_NeuronSWC();
			displayingSegs.seg.push_back(newV_NeuronSWC);
		}
	}
}

void FragmentEditor::sequencialTypeChanging(V_NeuronSWC_list& displayingSegs, const int seedSegID, const int type)
{
	set<int> segs2BtypeChanged;
	profiledTree currDisplayProfiledTree(displayingSegs.seg);
	currDisplayProfiledTree.nodeCoordKeySegMapGen(currDisplayProfiledTree.segs, currDisplayProfiledTree.nodeCoordKey2segMap);

	set<int> startingSegs = { seedSegID };
	this->rc_findConnectedSegs(currDisplayProfiledTree, startingSegs, segs2BtypeChanged);

	for (auto& seg : segs2BtypeChanged)
	{
		if (!currDisplayProfiledTree.segs.at(seg).to_be_deleted)
		{
			for (auto& node : displayingSegs.seg[seg].row)
				node.type = type;
		}
	}
}

void FragmentEditor::sequencialTypeChanging(V_NeuronSWC_list& displayingSegs, const set<int>& startingSegs, const int type)
{
	set<int> segs2BtypeChanged;
	profiledTree currDisplayProfiledTree(displayingSegs.seg);
	currDisplayProfiledTree.nodeCoordKeySegMapGen(currDisplayProfiledTree.segs, currDisplayProfiledTree.nodeCoordKey2segMap);
	
	this->rc_findConnectedSegs(currDisplayProfiledTree, startingSegs, segs2BtypeChanged);

	for (auto& seg : segs2BtypeChanged)
	{
		if (!currDisplayProfiledTree.segs.at(seg).to_be_deleted)
		{
			for (auto& node : displayingSegs.seg[seg].row)
				node.type = type;
		}
	}
}

void FragmentEditor::rc_findConnectedSegs(const profiledTree& inputProfiledTree, const set<int>& seedSegs, set<int>& connectedSegs)
{
	set<int> newSegs;

	for (set<int>::iterator segIt = seedSegs.begin(); segIt != seedSegs.end(); ++segIt)
	{
		for (QList<NeuronSWC>::const_iterator nodeIt = inputProfiledTree.segs.at(*segIt).nodes.begin(); nodeIt != inputProfiledTree.segs.at(*segIt).nodes.end(); ++nodeIt)
		{
			string nodeCoordKey = to_string(nodeIt->x) + "_" + to_string(nodeIt->y) + "_" + to_string(nodeIt->z);
			pair<boost::container::flat_multimap<string, int>::const_iterator, boost::container::flat_multimap<string, int>::const_iterator> range = inputProfiledTree.nodeCoordKey2segMap.equal_range(nodeCoordKey);
			if (range.second - range.first == 1) continue;
			else
			{
				for (boost::container::flat_multimap<string, int>::const_iterator it = range.first; it != range.second; ++it)
				{
					if (seedSegs.find(it->second) == seedSegs.end() && connectedSegs.find(it->second) == connectedSegs.end())
					{
						newSegs.insert(it->second);
						connectedSegs.insert(it->second);
					}
				}
			}
		}
	}

	if (newSegs.empty()) return;
	else this->rc_findConnectedSegs(inputProfiledTree, newSegs, connectedSegs);
}