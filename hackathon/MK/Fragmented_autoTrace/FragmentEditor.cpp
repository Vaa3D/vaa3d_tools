#include "neuron_format_converter.h"

#include "NeuronStructUtilities.h"

#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;

map<int, set<int>> FragmentEditor::erasingProcess(const float nodeCoords[])
{
	//profiledTree profiledInputTree(*(this->inputTreePtr));
	profiledTree profiledInputTree(this->originalTree);
	profiledInputTree.generateNodeID2segIDMap();

	
	int inputCoordTileX = int(nodeCoords[0] / NODE_TILE_LENGTH);
	int inputCoordTileY = int(nodeCoords[1] / NODE_TILE_LENGTH);
	int inputCoordTileZ = int(nodeCoords[2] / (NODE_TILE_LENGTH / zRATIO));
	string centralTileKey = to_string(inputCoordTileX) + "_" + to_string(inputCoordTileY) + "_" + to_string(inputCoordTileZ);
	
	map<int, set<int>> outputEditingSegInfo;
	if (profiledInputTree.nodeTileMap.find(centralTileKey) != profiledInputTree.nodeTileMap.end())
	{
		vector<int> centralTileNodes = profiledInputTree.nodeTileMap.at(centralTileKey);
		for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
		{
			if ((profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).x - nodeCoords[0]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).x - nodeCoords[0]) +
				(profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).y - nodeCoords[1]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).y - nodeCoords[1]) +
				(profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).z - nodeCoords[2]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).z - nodeCoords[2]) <= 100)
			{
				if (outputEditingSegInfo.find(profiledInputTree.node2segMap.find(*it)->second) == outputEditingSegInfo.end())
				{
					set<int> newSet;
					newSet.insert(*it);
					outputEditingSegInfo.insert({ profiledInputTree.node2segMap.find(*it)->second, newSet });
				}
				else outputEditingSegInfo.at(profiledInputTree.node2segMap.find(*it)->second).insert(*it);
			}
		}
	}

	return outputEditingSegInfo;
	/*set<int> involvedNodeIDs;
	vector<ptrdiff_t> delLocs;
	
	for (vector<int>::iterator it = centralTileNodes.begin(); it != centralTileNodes.end(); ++it)
	{
		cout << profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).x << " "
			<< profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).y << " "
			<< profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).z << "), ("
			<< nodeCoords[0] << " " << nodeCoords[1] << " " << nodeCoords[2] << endl;
		if ((profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).x - nodeCoords[0]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).x - nodeCoords[0]) +
			(profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).y - nodeCoords[1]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).y - nodeCoords[1]) +
			(profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).z - nodeCoords[2]) * (profiledInputTree.tree.listNeuron.at(profiledInputTree.node2LocMap.at(*it)).z - nodeCoords[2]) <= 100)
		{
			involvedNodeIDs.insert(*it);
			delLocs.push_back(ptrdiff_t(profiledInputTree.node2LocMap.at(*it)));
		}
	}
	
	sort(delLocs.rbegin(), delLocs.rend());
	cout << "number to be deleted: " << delLocs.size() << endl;
	for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
		profiledInputTree.tree.listNeuron.erase(profiledInputTree.tree.listNeuron.begin() + *delIt);
	cout << "test1" << endl;
	system("pause");
	displayedSegList = NeuronTree__2__V_NeuronSWC_list(&profiledInputTree.tree);
	cout << "test2" << endl;
	system("pause");*/

}
