#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;

map<int, set<int>> FragmentEditor::erasingProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[])
{
	this->inputSegList = displayingSegs.seg;
	int effectiveNodeCount = 0;
	
	for (vector<V_NeuronSWC>::iterator segIt = displayingSegs.seg.begin(); segIt != displayingSegs.seg.end(); ++segIt)
	{
		if (this->segMap.find(int(segIt - displayingSegs.seg.begin())) == this->segMap.end())
		{
			if (!segIt->to_be_deleted)
			{
				segUnit convertedSegUnit(*segIt);
				convertedSegUnit.segID = int(segIt - displayingSegs.seg.begin());
				for (QList<NeuronSWC>::iterator it = convertedSegUnit.nodes.begin(); it != convertedSegUnit.nodes.end(); ++it)
				{
					it->n += effectiveNodeCount;
					if (it->parent > -1) it->parent += effectiveNodeCount;
				}
				cout << "1 seg added to segMap: " << int(segIt - displayingSegs.seg.begin()) << endl;
				this->segMap.insert({ convertedSegUnit.segID, convertedSegUnit });
				effectiveNodeCount += convertedSegUnit.nodes.size();
			}
		}
		else
		{
			if (segIt->to_be_deleted)
			{
				cout << "1 seg deleted from segMap: " << int(segIt - displayingSegs.seg.begin()) << endl;
				this->segMap.erase(this->segMap.find(int(segIt - displayingSegs.seg.begin())));
			}
		}
	}

	NeuronTree currentTree;
	for (map<int, segUnit>::iterator segMapIt = this->segMap.begin(); segMapIt != this->segMap.end(); ++segMapIt)
		currentTree.listNeuron.append(segMapIt->second.nodes);
	NeuronStructUtil::nodeSegMapGen(this->segMap, this->node2segMap);
	cout << currentTree.listNeuron.size() << " " << this->segMap.size() << " " << this->inputSegList.size() << endl;
	//for (QList<NeuronSWC>::iterator it = currentTree.listNeuron.begin(); it != currentTree.listNeuron.end(); ++it)
	//	cout << it->x << "_" << it->y << "_" << it->z << " ";
	//cout << endl << endl;
	
	boost::container::flat_map<string, vector<int>> nodeTileMap;
	NeuronStructUtil::nodeTileMapGen(currentTree, nodeTileMap);
	//for (boost::container::flat_map<string, vector<int>>::iterator tileIt = nodeTileMap.begin(); tileIt != nodeTileMap.end(); ++tileIt)
	//	cout << tileIt->first << " ";
	//cout << endl;

	map<int, size_t> node2LocMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(currentTree.listNeuron, node2LocMap, node2childLocMap);
	int inputCoordTileX = int(nodeCoords[0] / NODE_TILE_LENGTH);
	int inputCoordTileY = int(nodeCoords[1] / NODE_TILE_LENGTH);
	int inputCoordTileZ = int(nodeCoords[2] / (NODE_TILE_LENGTH / zRATIO));
	string centralTileKey = to_string(inputCoordTileX) + "_" + to_string(inputCoordTileY) + "_" + to_string(inputCoordTileZ);
	//cout << centralTileKey << endl;
	//cout << node2LocMap.size() << " " << node2childLocMap.size() << endl;
	
	map<int, set<int>> outputEditingSegInfo;
	if (nodeTileMap.find(centralTileKey) != nodeTileMap.end())
	{
		vector<int> centralTileNodes = nodeTileMap.at(centralTileKey);
		//for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
		//	cout << *it << ": " << currentTree.listNeuron.at(node2LocMap.at(*it)).x << " " << currentTree.listNeuron.at(node2LocMap.at(*it)).y << " " << currentTree.listNeuron.at(node2LocMap.at(*it)).z << endl;
		//cout << endl;
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

	return outputEditingSegInfo;
}
