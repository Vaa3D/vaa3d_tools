#include <iostream>

#include "NeuronStructUtilities.h"
#include "TreeGrower.h"

using namespace std;

boost::container::flat_map<double, NeuronTree> TreeGrower::radiusShellNeuronTreeMap(const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList)
{
	boost::container::flat_map<double, NeuronTree> outputRadius2NeuronTreeMap;
	for (boost::container::flat_map<double, boost::container::flat_set<int>>::const_iterator it = inputRadiusMap.begin(); it != inputRadiusMap.end(); ++it)
	{
		NeuronTree currShellTree;
		for (boost::container::flat_set<int>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode = NeuronGeoGrapher::polar2CartesianNode(inputPolarNodeList.at(*it2));
			currShellTree.listNeuron.push_back(newNode);
		}

		outputRadius2NeuronTreeMap.insert(pair<double, NeuronTree>(it->first, currShellTree));
	}

	return outputRadius2NeuronTreeMap;
}

boost::container::flat_map<double, vector<connectedComponent>> TreeGrower::shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap)
{
	NeuronStructUtil myNeuronStructUtil;
	boost::container::flat_map<double, vector<connectedComponent>> outputShell2radiusConnMap;

	for (boost::container::flat_map<double, NeuronTree>::const_iterator it = inputRadius2NeuronTreeMap.begin(); it != inputRadius2NeuronTreeMap.end(); ++it)
	{
		vector<connectedComponent> currConnCompList = myNeuronStructUtil.swc2signal3DBlobs(it->second);
		outputShell2radiusConnMap.insert(pair<double, vector<connectedComponent>>(it->first, currConnCompList));
	}

	return outputShell2radiusConnMap;
}