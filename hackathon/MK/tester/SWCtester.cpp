#include <map>

#include "SWCtester.h"

using namespace std;

SWCtester::SWCtester()
{
	this->myImgAnalyzerPtr = new ImgAnalyzer;
	this->myNeuronStructUtilPtr = new NeuronStructUtil;
}

vector<connectedComponent> SWCtester::connComponent2DmergeTest(QString inputSWCfileName)
{
	NeuronTree inputTree = readSWC_file(inputSWCfileName);
	vector<connectedComponent> outputConnCompList = NeuronStructUtil::swc2signal3DBlobs(inputTree);

	return outputConnCompList;
}

QList<NeuronSWC> SWCtester::polarCoordShellPeeling(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius)
{
	map<int, int> nodeID2LocMap;
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
		nodeID2LocMap.insert(pair<int, int>(it->n, int(it - inputNodeList.begin())));

	vector<polarNeuronSWC> polarNodeList;
	NeuronGeoGrapher::nodeList2polarNodeList(inputNodeList, polarNodeList, origin);

	map<int, vector<int>> shells;
	for (vector<polarNeuronSWC>::iterator it = polarNodeList.begin(); it != polarNodeList.end(); ++it)
	{
		if (shells.find(int(it->radius)) != shells.end()) shells.at(int(it->radius)).push_back(it->ID);
		else
		{
			vector<int> newGroup;
			newGroup.push_back(it->ID);
			shells.insert(pair<int, vector<int>>(int(it->radius), newGroup));
		}
	}

	QList<NeuronSWC> outputList;
	for (map<int, vector<int>>::iterator it = shells.begin(); it != shells.end(); ++it)
	{
		for (vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode;
			newNode = inputNodeList.at(nodeID2LocMap.at(*it2));
			newNode.type = it->first;
			outputList.push_back(newNode);
		}
	}

	return outputList;
}

QList<NeuronSWC> SWCtester::polarCoordAngle_horizontal(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius)
{
	map<int, int> nodeID2LocMap;
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
		nodeID2LocMap.insert(pair<int, int>(it->n, int(it - inputNodeList.begin())));

	vector<polarNeuronSWC> polarNodeList;
	NeuronGeoGrapher::nodeList2polarNodeList(inputNodeList, polarNodeList, origin);

	map<int, vector<int>> angles;
	vector<int> empty;
	empty.clear();
	for (int i = 0; i <= 12; ++i) angles.insert(pair<int, vector<int>>(30 * i, empty));
	
	for (vector<polarNeuronSWC>::iterator it = polarNodeList.begin(); it != polarNodeList.end(); ++it)
	{
		int angleIndex = int((it->theta) * 3) * 30;
		//cout << it->theta << " " << angleIndex << endl;
		angles.at(angleIndex).push_back(it->ID);
	}
	//cout << endl;

	QList<NeuronSWC> outputList;
	for (map<int, vector<int>>::iterator it = angles.begin(); it != angles.end(); ++it)
	{
		for (vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode;
			newNode = inputNodeList.at(nodeID2LocMap.at(*it2));
			newNode.type = it->first / 30;
			outputList.push_back(newNode);
		}
	}

	return outputList;
}

QList<NeuronSWC> SWCtester::polarCoordAngle_vertical(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius)
{
	map<int, int> nodeID2LocMap;
	for (QList<NeuronSWC>::const_iterator it = inputNodeList.begin(); it != inputNodeList.end(); ++it)
		nodeID2LocMap.insert(pair<int, int>(it->n, int(it - inputNodeList.begin())));

	vector<polarNeuronSWC> polarNodeList;
	NeuronGeoGrapher::nodeList2polarNodeList(inputNodeList, polarNodeList, origin);

	map<int, vector<int>> angles;
	vector<int> empty;
	empty.clear();
	for (int i = 0; i <= 12; ++i) angles.insert(pair<int, vector<int>>(30 * i, empty));

	for (vector<polarNeuronSWC>::iterator it = polarNodeList.begin(); it != polarNodeList.end(); ++it)
	{
		int angleIndex = int((it->phi) * 3) * 30;
		//cout << it->theta << " " << angleIndex << endl;
		angles.at(angleIndex).push_back(it->ID);
	}
	//cout << endl;

	QList<NeuronSWC> outputList;
	for (map<int, vector<int>>::iterator it = angles.begin(); it != angles.end(); ++it)
	{
		for (vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode;
			newNode = inputNodeList.at(nodeID2LocMap.at(*it2));
			newNode.type = it->first / 30;
			outputList.push_back(newNode);
		}
	}

	return outputList;
}
