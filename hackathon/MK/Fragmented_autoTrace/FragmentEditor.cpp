#include "FragTracer_Define.h"
#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;

void FragmentEditor::erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[], const int mouseX, const int mouseY)
{
	int nodeCount = 0;
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
	NeuronStructUtil::nodeSegMapGen(this->segMap, this->node2segMap);
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
		cout << "Res(" << CViewerPortal->getTeraflyResLevel() << ") Zoom(" << CViewerPortal->getZoomingFactor() << ") EraserSize(" << CViewerPortal->getEraserSize() << ")" << endl;
		cout << endl;
		float eraseRange = this->getErasingRange(CViewerPortal->getTeraflyResLevel(), CViewerPortal->getZoomingFactor(), CViewerPortal->getEraserSize());
		cout << "  ---- RANGE = " << eraseRange << endl;
		
		vector<int> centralTileNodes = nodeTileMap.at(centralTileKey);
		for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
		{
			float localCoords[3], windowCoords[3];
			localCoords[0] = currentTree.listNeuron.at(node2LocMap.at(*it)).x;
			localCoords[1] = currentTree.listNeuron.at(node2LocMap.at(*it)).y;
			localCoords[2] = currentTree.listNeuron.at(node2LocMap.at(*it)).z;
			CViewerPortal->convertLocalCoord2windowCoord(localCoords, windowCoords);
			//cout << " --- converted window coords: " << windowCoords[0] << " " << windowCoords[1] << endl;
			
			if ((currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) * zRATIO * zRATIO <= eraseRange)
			//if (sqrtf((float(mouseX) - windowCoords[0]) * (float(mouseX) - windowCoords[0]) + (float(mouseY) - windowCoords[1]) * (float(mouseY) - windowCoords[1])) <= 35)
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

	//return outputEditingSegInfo;
}

float FragmentEditor::getErasingRange(const int teraflyResLevel, const int zoomLevel, const int eraserSize)
{
	float outputRange;
	float zoomLevelFloat = zoomLevel;
	switch (teraflyResLevel)
	{
		case 0:

			break;
		case 1:

			break;
		case 2:
			if (zoomLevel <= 0)
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2));
			else
				outputRange = float((eraserSize + 6 - (zoomLevelFloat / 4) * 2) * (eraserSize + 6 - (zoomLevelFloat / 4) * 2));
			break;
		case 3:

			break;
		case 4:

			break;
		case 5:

			break;
	}




	if (eraserSize == 0) return 9;
	else if (eraserSize == 1) return 16;
	else if (eraserSize == 2) return 25;
	else if (eraserSize == 3) return 36;
	else if (eraserSize == -1) return 4;
	else if (eraserSize == -2) return 1;

	//return outputRange;
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
