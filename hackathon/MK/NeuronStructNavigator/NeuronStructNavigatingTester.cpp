#include "NeuronStructNavigatingTester.h"

using namespace std;

// Declare a singleton testing class here.
NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::testerInstance = nullptr;

NeuronStructNavigator::Tester::Tester(NeuronStructExplorer* explorerPtr)
{
	this->sharedExplorerPtr = make_shared<NeuronStructExplorer*>(explorerPtr);
	this->sharedGrowerPtr = nullptr;
	this->sharedTrimmerPtr = nullptr;
}

NeuronStructNavigator::Tester::Tester(TreeGrower* growerPtr)
{
	this->sharedGrowerPtr = make_shared<TreeGrower*>(growerPtr);
	this->sharedExplorerPtr = nullptr;
	this->sharedTrimmerPtr = nullptr;
}

NeuronStructNavigator::Tester::Tester(TreeTrimmer* trimmerPtr)
{
	this->sharedTrimmerPtr = make_shared<TreeTrimmer*>(trimmerPtr);
	this->sharedExplorerPtr = nullptr;
	this->sharedGrowerPtr = nullptr;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::instance(NeuronStructExplorer* explorerPtr)
{
	NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(explorerPtr);
	return NeuronStructNavigator::Tester::testerInstance;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::instance(TreeGrower* growerPtr)
{
	NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(growerPtr);
	return NeuronStructNavigator::Tester::testerInstance;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::instance(TreeTrimmer* trimmerPtr)
{
	NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(trimmerPtr);
	return NeuronStructNavigator::Tester::testerInstance;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::getInstance()
{
	if (NeuronStructNavigator::Tester::testerInstance != nullptr) return NeuronStructNavigator::Tester::testerInstance;
	else return nullptr;
}

void NeuronStructNavigator::Tester::uninstance()
{
	if (NeuronStructNavigator::Tester::getInstance() != nullptr) delete NeuronStructNavigator::Tester::testerInstance;
	NeuronStructNavigator::Tester::testerInstance = nullptr;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::reInstance(NeuronStructExplorer* explorerPtr)
{
	if (NeuronStructNavigator::Tester::isInstantiated())
	{
		NeuronStructNavigator::Tester::uninstance();
		NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(explorerPtr);
	}
	else NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(explorerPtr);

	return NeuronStructNavigator::Tester::testerInstance;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::reInstance(TreeGrower* growerPtr)
{
	if (NeuronStructNavigator::Tester::isInstantiated())
	{
		NeuronStructNavigator::Tester::uninstance();
		NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(growerPtr);
	}
	else NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(growerPtr);

	return NeuronStructNavigator::Tester::testerInstance;
}

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::reInstance(TreeTrimmer* trimmerPtr)
{
	if (NeuronStructNavigator::Tester::isInstantiated())
	{
		NeuronStructNavigator::Tester::uninstance();
		NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(trimmerPtr);
	}
	else NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester(trimmerPtr);

	return NeuronStructNavigator::Tester::testerInstance;
}

map<int, set<vector<float>>> NeuronStructNavigator::Tester::getSegEndClusterNodeMap(const profiledTree& inputProfiledTree)
{
	map<int, set<vector<float>>> outputMap;
	
	if (inputProfiledTree.segHeadClusters.empty() && inputProfiledTree.segTailClusters.empty())
	{
		outputMap.clear();
		cerr << "Segment end clusters are empty. Do nothing and return empty map." << endl;
		return outputMap;
	}

	for (auto& headCluster : inputProfiledTree.segHeadClusters)
	{
		set<vector<float>> headCoords;
		for (auto& headSeg : headCluster.second)
		{
			const NeuronSWC headNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSeg).head));
			vector<float> headCoord = { headNode.x, headNode.y, headNode.z };
			headCoords.insert(headCoord);
		}

		if (outputMap.find(headCluster.first) == outputMap.end()) outputMap.insert({ headCluster.first, headCoords });
		else outputMap[headCluster.first].insert(headCoords.begin(), headCoords.end());
	}

	for (auto& tailCluster : inputProfiledTree.segTailClusters)
	{
		set<vector<float>> tailCoords;
		for (auto& tailSeg : tailCluster.second)
		{
			for (auto& tailID : inputProfiledTree.segs.at(tailSeg).tails)
			{
				const NeuronSWC tailNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailID));
				vector<float> tailCoord = { tailNode.x, tailNode.y, tailNode.z };
				tailCoords.insert(tailCoord);
			}
		}

		if (outputMap.find(tailCluster.first) == outputMap.end()) outputMap.insert({ tailCluster.first, tailCoords });
		else outputMap[tailCluster.first].insert(tailCoords.begin(), tailCoords.end());
	}

	return outputMap;
}