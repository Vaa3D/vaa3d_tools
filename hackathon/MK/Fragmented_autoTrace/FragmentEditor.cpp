#include "neuron_format_converter.h"

#include "NeuronStructUtilities.h"

#include "FragmentEditor.h"

using namespace std;
using namespace integratedDataTypes;

FragmentEditor::FragmentEditor(QWidget* parent, V3DPluginCallback2* callback) : thisCallback(callback)
{
	
}

vector<V_NeuronSWC> FragmentEditor::erasingProcess(const float nodeCoords[])
{
	V_NeuronSWC_list displayedSegList;
	displayedSegList.seg = this->inputV_NeuronSWCs;
	
	profiledTree profiledInputTree = NeuronStructUtil::v_NeuronSWC_list2profiledTree(displayedSegList);
	//cout << profiledInputTree.tree.listNeuron.size() << endl;
	//cout << endl << nodeCoords[0] << " " << nodeCoords[1] << " " << nodeCoords[2] << endl;
	
	int inputCoordTileX = int(nodeCoords[0] / NODE_TILE_LENGTH);
	int inputCoordTileY = int(nodeCoords[1] / NODE_TILE_LENGTH);
	int inputCoordTileZ = int(nodeCoords[2] / NODE_TILE_LENGTH / zRATIO);
	string centralTileKey = to_string(inputCoordTileX) + "_" + to_string(inputCoordTileY) + "_" + to_string(inputCoordTileZ);
	
	vector<int> centralTileNodes = profiledInputTree.nodeTileMap.at(centralTileKey);
	cout << centralTileNodes.size() << endl;
	set<int> involvedNodeIDs;
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
	system("pause");

	return displayedSegList.seg;
}
