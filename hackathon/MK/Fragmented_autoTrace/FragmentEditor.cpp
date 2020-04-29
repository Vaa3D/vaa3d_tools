#include "NeuronStructNavigatingTester.h"

#include "FragTracer_Define.h"
#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;
using NSlibTester = NeuronStructNavigator::Tester;

void FragmentEditor::connectingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[])
{
	vector<segUnit> segUnits;
	for (vector<V_NeuronSWC>::iterator dSegIt = displayingSegs.seg.begin(); dSegIt != displayingSegs.seg.end(); ++dSegIt)
	{
		segUnit convertedSegUnit(*dSegIt);
		// The segments in the tree traced by Neuron Assembler are not labeled.
		// Therefore, dSegIt->row.begin()->seg_id cannot be used.
		convertedSegUnit.segID = int(dSegIt - displayingSegs.seg.begin()); 
		this->segMap.insert({ convertedSegUnit.segID, convertedSegUnit });
	}

	int connSize = this->CViewerPortal->getConnectorSize();
	int searchRange;
	switch (connSize)
	{
	case 0:
		searchRange = 6;
		break;
	case 1:
		searchRange = 10;
		break;
	case -1:
		searchRange = 3;
		break;
	default:
		break;
	}
	cout << "search range = " << searchRange << endl;

	vector<pair<int, connectOrientation>> segConnOris;
	for (auto& seg : this->segMap)
	{
		int headLoc = seg.second.seg_nodeLocMap.at(seg.second.head);
		int tailLoc = seg.second.seg_nodeLocMap.at(*seg.second.tails.begin());
		if ((seg.second.nodes.at(headLoc).x - nodeCoords[0]) * (seg.second.nodes.at(headLoc).x - nodeCoords[0]) +
			(seg.second.nodes.at(headLoc).y - nodeCoords[1]) * (seg.second.nodes.at(headLoc).y - nodeCoords[1]) +
			(seg.second.nodes.at(headLoc).z - nodeCoords[2]) * (seg.second.nodes.at(headLoc).z - nodeCoords[2]) <= searchRange * searchRange)
		{
			if (!displayingSegs.seg.at(seg.first).to_be_deleted) segConnOris.push_back({ seg.first, head });
		}
		else if ((seg.second.nodes.at(tailLoc).x - nodeCoords[0]) * (seg.second.nodes.at(tailLoc).x - nodeCoords[0]) +
				 (seg.second.nodes.at(tailLoc).y - nodeCoords[1]) * (seg.second.nodes.at(tailLoc).y - nodeCoords[1]) +
				 (seg.second.nodes.at(tailLoc).z - nodeCoords[2]) * (seg.second.nodes.at(tailLoc).z - nodeCoords[2]) <= searchRange * searchRange)
		{
			if (!displayingSegs.seg.at(seg.first).to_be_deleted) segConnOris.push_back({ seg.first, tail });
		}
	}

	if (segConnOris.size() > 2)
	{
		cout << "Only 2 segments allowed for this operation. Do nothing." << endl;
		return;
	}
	else if (segConnOris.size() == 2)
	{
		cout << segConnOris.at(0).first << " " << segConnOris.at(1).first << endl;
		connectOrientation ori;
		if (segConnOris.at(0).second == head && segConnOris.at(1).second == head) ori = head_head;
		else if (segConnOris.at(0).second == head && segConnOris.at(1).second == tail) ori = head_tail;
		else if (segConnOris.at(0).second == tail && segConnOris.at(1).second == head) ori = tail_head;
		else if (segConnOris.at(0).second == tail && segConnOris.at(1).second == tail) ori = tail_tail;
		segUnit newSeg = NeuronStructUtil::segUnitConnect_executer(this->segMap.at(segConnOris.at(0).first), this->segMap.at(segConnOris.at(1).first), ori);
	
		// Rearranging nodes is necessary as [NeuronStructUtil::segUnitConnect_executer] does not take care of node hierarchy.
		// The function only focuses on segment ends since it soley serves connecting purpose.
		newSeg.nodes.begin()->n = 1;
		newSeg.nodes.begin()->parent = -1;
		for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin() + 1; it != newSeg.nodes.end(); ++it)
		{
			it->n = int(it - newSeg.nodes.begin()) + 1;
			it->parent = (it - 1)->n;
		}
		newSeg.reInit(newSeg);
		newSeg.segID = displayingSegs.seg.size() + 1;
		/*if (!NSlibTester::isInstantiated())
		{
			NSlibTester::instance();
			NSlibTester::getInstance()->printoutSegUnitInfo(newSeg);
		}*/

		V_NeuronSWC newDisplaySeg = newSeg.convert2V_NeuronSWC();
		newDisplaySeg.to_be_deleted = false;
		displayingSegs.seg[segConnOris.at(0).first].to_be_deleted = true;
		displayingSegs.seg[segConnOris.at(1).first].to_be_deleted = true;
		displayingSegs.seg.push_back(newDisplaySeg);
	}
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
		segUnit targetSegUnit(this->segMap.at(editIt->first).nodes);
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
