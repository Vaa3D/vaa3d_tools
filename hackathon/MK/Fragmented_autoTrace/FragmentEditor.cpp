#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;

map<int, set<int>> FragmentEditor::erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[])
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
			nodeCount += convertedSegUnit.nodes.size();
		}
	}

	NeuronTree currentTree;
	for (map<int, segUnit>::iterator segMapIt = this->segMap.begin(); segMapIt != this->segMap.end(); ++segMapIt)
		if (!segMapIt->second.to_be_deleted) currentTree.listNeuron.append(segMapIt->second.nodes);
	NeuronStructUtil::nodeSegMapGen(this->segMap, this->node2segMap);
	cout << currentTree.listNeuron.size() << " " << this->segMap.size() << " " << displayingSegs.seg.size() << endl;

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
		vector<int> centralTileNodes = nodeTileMap.at(centralTileKey);
		for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
		{
			if ((currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).x - nodeCoords[0]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).y - nodeCoords[1]) +
				(currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) * (currentTree.listNeuron.at(node2LocMap.at(*it)).z - nodeCoords[2]) <= 100)
			{
				if (outputEditingSegInfo.find(this->node2segMap.find(*it)->second) == outputEditingSegInfo.end())
				{
					set<int> newSet;
					newSet.insert(*it);
					outputEditingSegInfo.insert({ this->node2segMap.find(*it)->second, newSet });
				}
				else outputEditingSegInfo.at(this->node2segMap.find(*it)->second).insert(*it);
			}
		}
	}

	this->erasingProcess_cuttingSeg(displayingSegs, outputEditingSegInfo);

	return outputEditingSegInfo;
}

void FragmentEditor::erasingProcess_cuttingSeg(V_NeuronSWC_list& displayingSegs, const map<int, set<int>>& seg2BeditedInfo)
{
	for (map<int, set<int>>::const_iterator editIt = seg2BeditedInfo.begin(); editIt != seg2BeditedInfo.end(); ++editIt)
	{
		segUnit targetSegUnit(this->segMap.at(editIt->first).nodes);
		vector<ptrdiff_t> delLocs;
		for (set<int>::const_iterator nodeIt = editIt->second.begin(); nodeIt != editIt->second.end(); ++nodeIt)
		{
			for (vector<size_t>::iterator tailIt = targetSegUnit.seg_childLocMap.at(*nodeIt).begin(); tailIt != targetSegUnit.seg_childLocMap.at(*nodeIt).end(); ++tailIt)
			{
				targetSegUnit.nodes[*tailIt].parent = -1;
				delLocs.push_back(targetSegUnit.seg_nodeLocMap.at(*nodeIt));
			}
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
			targetSegUnit.nodes.erase(targetSegUnit.nodes.begin() + *delIt);

		segUnit trimmedSegUnit(targetSegUnit.nodes);
		V_NeuronSWC newV_NeuronSWC = trimmedSegUnit.convert2V_NeuronSWC();
		displayingSegs.seg.push_back(newV_NeuronSWC);
	}
}
