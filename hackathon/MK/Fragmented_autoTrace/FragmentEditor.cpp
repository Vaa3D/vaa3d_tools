#include "NeuronStructNavigatingTester.h"

#include "FragTracer_Define.h"
#include "FragmentEditor.h"
#include "FragTraceTester.h"

using namespace std;
using namespace integratedDataTypes;
using NSlibTester = NeuronStructNavigator::Tester;

void FragmentEditor::connectingProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap)
{
	if (seg2includedNodeMap.size() > 2)
	{
		cout << "More than 2 segments included in the target circle. Do nothing." << endl;
		return;
	}
	else if (seg2includedNodeMap.size() < 2)
	{
		cout << "Not enough number of segments to proceed the operation. Do nothing." << endl;
		return;
	}

	/*for (auto& seg : seg2includedNodeMap)
	{
		cout << "segID: " << seg.first << " -- ";
		for (auto& node : seg.second) cout << node.n << " ";
		cout << endl;
	}*/

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

	segUnit newSeg;
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
		int bodySegID, segEndNodeID;
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
		newDisplaySeg.to_be_deleted = false;		
		displayingSegs.seg.push_back(newDisplaySeg);
	}
}

vector<float> FragmentEditor::getSegEndPointingVec(const segUnit& inputSeg, const int endNodeID, int nodeNum)
{
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
			if (inputSeg.seg_childLocMap.at(paNodeID).size() > 1 || 
				inputSeg.seg_childLocMap.find(paNodeID) == inputSeg.seg_childLocMap.end())
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

	this->erasingProcess_cuttingSeg(displayingSegs, outputEditingSegInfo);
}

void FragmentEditor::erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[])
{
	int nodeCount = 0; // Erasing process is node-based operation. Count node number to monitor the process here.
	for (map<int, segUnit>::iterator countIt = this->segMap.begin(); countIt != this->segMap.end(); ++countIt)
		nodeCount += countIt->second.nodes.size();

	for (vector<V_NeuronSWC>::iterator segIt = displayingSegs.seg.begin(); segIt != displayingSegs.seg.end(); ++segIt)
	{
		if (this->segMap.find(int(segIt - displayingSegs.seg.begin())) == this->segMap.end())
		{
			segUnit convertedSegUnit(*segIt);
			convertedSegUnit.segID = int(segIt - displayingSegs.seg.begin());
			for (QList<NeuronSWC>::iterator it = convertedSegUnit.nodes.begin(); it != convertedSegUnit.nodes.end(); ++it)
			{
				it->n += nodeCount;
				if (it->parent > -1) it->parent += nodeCount;
			}
			this->segMap.insert({ convertedSegUnit.segID, convertedSegUnit });
			//cout << " -- new segment added: ID(" << convertedSegUnit.segID << ") size(" << convertedSegUnit.nodes.size() << ") " << endl;
			nodeCount += convertedSegUnit.nodes.size();
		}
	}

#ifdef __ERASER_DEBUG__
	for (map<int, segUnit>::iterator it = this->segMap.begin(); it != this->segMap.end(); ++it)
	{
		cout << "ID(" << it->first << ") " << it->second.to_be_deleted << " size(" << it->second.nodes.size() << ") -- ";
		cout << "ID(" << it->first << ") " << displayingSegs.seg.at(it->first).to_be_deleted << " size(" << displayingSegs.seg.at(it->first).row.size() << ")" << endl;
	}
#endif

	NeuronTree currentTree;
	for (map<int, segUnit>::iterator segMapIt = this->segMap.begin(); segMapIt != this->segMap.end(); ++segMapIt)
		if (!segMapIt->second.to_be_deleted) currentTree.listNeuron.append(segMapIt->second.nodes);
	NeuronStructUtil::nodeSegMapGen(this->segMap, this->node2segMap); // It might need to be optimized later, slow after several clicks in the same erasing cycle.
	//cout << currentTree.listNeuron.size() << " " << this->segMap.size() << " " << displayingSegs.seg.size() << endl;

	boost::container::flat_map<string, vector<int>> nodeTileMap;
	NeuronStructUtil::nodeTileMapGen(currentTree, nodeTileMap);
	map<int, size_t> node2LocMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(currentTree.listNeuron, node2LocMap, node2childLocMap);
	int inputCoordTileX = int(nodeCoords[0] / NODE_TILE_LENGTH);
	int inputCoordTileY = int(nodeCoords[1] / NODE_TILE_LENGTH);
	int inputCoordTileZ = int(nodeCoords[2] / (NODE_TILE_LENGTH / zRATIO));
	string centralTileKey = to_string(inputCoordTileX) + "_" + to_string(inputCoordTileY) + "_" + to_string(inputCoordTileZ);

	map<int, set<int>> outputEditingSegInfo;
	if (nodeTileMap.find(centralTileKey) != nodeTileMap.end())
	{
		int currResLevel = this->CViewerPortal->getTeraflyTotalResLevel() - 1 - this->CViewerPortal->getTeraflyResLevel();
		cout << "ResDownsamplePow(" << currResLevel << ") Zoom(" << CViewerPortal->getZoomingFactor() << ") EraserSize(" << CViewerPortal->getEraserSize() << ")" << endl;
		cout << endl;
		float eraseRange = this->getErasingRange(currResLevel, CViewerPortal->getZoomingFactor(), CViewerPortal->getEraserSize());
		cout << "  ---- RANGE = " << eraseRange << endl;
		
		vector<int> centralTileNodes = nodeTileMap.at(centralTileKey);
		for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
		{
			float localCoords[3], windowCoords[3];
			localCoords[0] = currentTree.listNeuron.at(node2LocMap.at(*it)).x;
			localCoords[1] = currentTree.listNeuron.at(node2LocMap.at(*it)).y;
			localCoords[2] = currentTree.listNeuron.at(node2LocMap.at(*it)).z;
			
			if ((currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) * zRATIO * zRATIO <= eraseRange)
			{
				if (outputEditingSegInfo.find(this->node2segMap.find(*it)->second) == outputEditingSegInfo.end())
				{
					set<int> newSet;
					newSet.insert(*it);
					outputEditingSegInfo.insert({ this->node2segMap.find(*it)->second, newSet });
					this->segMap[this->node2segMap.find(*it)->second].to_be_deleted = true;
				}
				else outputEditingSegInfo.at(this->node2segMap.find(*it)->second).insert(*it);
			}
		}
	}

	for (map<int, set<int>>::iterator it = outputEditingSegInfo.begin(); it != outputEditingSegInfo.end(); ++it)
	{
		displayingSegs.seg[it->first].to_be_deleted = true;
		//cout << " -- segment to be deleted: ID(" << it->first << ") size(" << this->segMap.at(it->first).nodes.size() << ")" << endl;
		/*for (set<int>::iterator nodeIt = it->second.begin(); nodeIt != it->second.end(); ++nodeIt)
			cout << "    -- node coords: x(" << currentTree.listNeuron.at(node2LocMap.at(*nodeIt)).x << ") y("
											 << currentTree.listNeuron.at(node2LocMap.at(*nodeIt)).y << ") z("
											 << currentTree.listNeuron.at(node2LocMap.at(*nodeIt)).z << ")" << endl;
		cout << endl;*/
	}

	this->erasingProcess_cuttingSeg(displayingSegs, outputEditingSegInfo);
}

float FragmentEditor::getErasingRange(const int teraflyResPow, const int zoomLevel, const int eraserSize)
{
	float outputRange;
	float zoomLevelFloat = zoomLevel;
	switch (teraflyResPow)
	{
		case 0:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2)) / 1.4;
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4)) * 1.2;
			break;
		case 1:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2)) / 1.6;
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4)) * 1.2;
			break;
		case 2:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2));
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4)) * 2;
			break;
		case 3:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2)) / 2;
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4));
			break;
		case 4:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2)) / 4;
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4)) * 0.6;
			break;
		case 5:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2)) / 8;
			else
				outputRange = float((1.25 * zoomLevelFloat + 0.5 * eraserSize) * (1.25 * zoomLevelFloat + 0.5 * eraserSize)) / ((zoomLevelFloat / 4) * (zoomLevelFloat / 4) * (zoomLevelFloat / 4)) * 0.5;
			break;
	}

	return outputRange;
}

void FragmentEditor::erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo)
{
	for (map<int, set<int>>::const_iterator editIt = seg2BeditedInfo.begin(); editIt != seg2BeditedInfo.end(); ++editIt)
	{
		segUnit targetSegUnit(displayingSegs.seg.at(editIt->first));
		vector<ptrdiff_t> delLocs;
		for (set<int>::const_iterator nodeIt = editIt->second.begin(); nodeIt != editIt->second.end(); ++nodeIt)
		{
			for (vector<size_t>::iterator childIt = targetSegUnit.seg_childLocMap.at(*nodeIt).begin(); childIt != targetSegUnit.seg_childLocMap.at(*nodeIt).end(); ++childIt)
			{
				targetSegUnit.nodes[*childIt].parent = -1;
				delLocs.push_back(targetSegUnit.seg_nodeLocMap.at(*nodeIt));
			}
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
			targetSegUnit.nodes.erase(targetSegUnit.nodes.begin() + *delIt);

		map<int, size_t> node2LocMap;
		map<int, vector<size_t>> node2childLocMap;
		NeuronStructUtil::node2loc_node2childLocMap(targetSegUnit.nodes, node2LocMap, node2childLocMap);
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