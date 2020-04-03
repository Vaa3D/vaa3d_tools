#include "NeuronStructNavigatingTester.h"

using namespace std;

// Declare a singleton testing class here.
NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::testerInstance = nullptr;

NeuronStructNavigator::Tester::Tester(const NeuronStructExplorer* explorerPtr)
{
	this->sharedExplorerPtr = make_shared<const NeuronStructExplorer*>(explorerPtr);
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

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::instance(const NeuronStructExplorer* explorerPtr)
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

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::reInstance(const NeuronStructExplorer* explorerPtr)
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

map<int, set<vector<float>>> NeuronStructNavigator::Tester::getSegEndClusterNodeMap(const profiledTree& inputProfiledTree) const
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

void NeuronStructNavigator::Tester::checkSegHeadClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	map<int, pair<boost::container::flat_set<int>, RGBA8>> headClusterAPOs;
	RGBA8 color;
	for (auto& headCluster: inputProfiledTree.segHeadClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = headCluster.second.begin(); it != headCluster.second.end(); ++it)
			headClusterAPOs.insert({ headCluster.first, pair<boost::container::flat_set<int>, RGBA8>(headCluster.second, color) });
	}
	this->assignRGBcolors(headClusterAPOs);

	QList<CellAPO> segHeadAPOs;
	for (auto& headCluster : headClusterAPOs)
	{
		for (auto& headSegID : headCluster.second.first)
		{
			CellAPO newAPO;
			newAPO.x = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).x;
			newAPO.y = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).y;
			newAPO.z = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).z;
			newAPO.color = headCluster.second.second;
			segHeadAPOs.push_back(newAPO);
		}
	}

	writeSWC_file(saveNamePrefixQ + "_inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "_inputTree_headClusters.apo", segHeadAPOs);
}

void NeuronStructNavigator::Tester::checkSegTailClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	map<int, pair<boost::container::flat_set<int>, RGBA8>> tailClusterApos;
	RGBA8 color;
	for (auto& tailCluster : inputProfiledTree.segTailClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = tailCluster.second.begin(); it != tailCluster.second.end(); ++it)
			tailClusterApos.insert({ tailCluster.first, pair<boost::container::flat_set<int>, RGBA8>(tailCluster.second, color) });
	}
	this->assignRGBcolors(tailClusterApos);

	QList<CellAPO> segTailAPOs;
	for (auto& tailCluster : tailClusterApos)
	{
		for (auto& tailSegID : tailCluster.second.first)
		{
			for (auto& tailNodeID : inputProfiledTree.segs.at(tailSegID).tails)
			{
				CellAPO newAPO;
				newAPO.x = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailNodeID)).x;
				newAPO.y = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailNodeID)).y;
				newAPO.z = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailNodeID)).z;
				newAPO.color = tailCluster.second.second;
				segTailAPOs.push_back(newAPO);
			}
		}
	}

	writeSWC_file(saveNamePrefixQ + "_inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "_inputTree_tailClusters.apo", segTailAPOs);
}

void NeuronStructNavigator::Tester::checkHeadSegID2cluslter(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	profiledTree profiledCopy = inputProfiledTree;
	map<int, pair<boost::container::flat_set<int>, RGBA8>> headClusterAPOs;
	
	RGBA8 color;
	for (boost::container::flat_map<int, int>::iterator it = profiledCopy.headSeg2ClusterMap.begin(); it != profiledCopy.headSeg2ClusterMap.end(); ++it)
	{
		if (headClusterAPOs.find(it->second) != headClusterAPOs.end()) headClusterAPOs[it->second].first.insert(it->first);		
		else
		{
			boost::container::flat_set<int> headSegs = { it->first };
			pair<boost::container::flat_set<int>, RGBA8> newPair(headSegs, color);
			headClusterAPOs.insert({ it->second, newPair });
		}
	}

	QList<CellAPO> segHeadAPOs;
	for (auto& headCluster : headClusterAPOs)
	{
		for (auto& headSegID : headCluster.second.first)
		{
			CellAPO newAPO;
			newAPO.x = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).x;
			newAPO.y = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).y;
			newAPO.z = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).z;
			newAPO.color = headCluster.second.second;
			segHeadAPOs.push_back(newAPO);
		}
	}

	writeSWC_file(saveNamePrefixQ + "_inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "_inputTree_headSeg2clusterCheck.apo", segHeadAPOs);
}

void NeuronStructNavigator::Tester::checkTailSegID2cluslter(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	profiledTree profiledCopy = inputProfiledTree;
	map<int, pair<boost::container::flat_set<int>, RGBA8>> tailClusterAPOs;

	RGBA8 color;
	for (boost::container::flat_map<int, int>::iterator it = profiledCopy.tailSeg2ClusterMap.begin(); it != profiledCopy.tailSeg2ClusterMap.end(); ++it)
	{
		if (tailClusterAPOs.find(it->second) != tailClusterAPOs.end()) tailClusterAPOs[it->second].first.insert(it->first);
		else
		{
			boost::container::flat_set<int> tailSegs = { it->first };
			pair<boost::container::flat_set<int>, RGBA8> newPair(tailSegs, color);
			tailClusterAPOs.insert({ it->second, newPair });
		}
	}

	QList<CellAPO> segTailAPOs;
	for (auto& tailCluster : tailClusterAPOs)
	{
		for (auto& tailSegID : tailCluster.second.first)
		{
			CellAPO newAPO;
			newAPO.x = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(tailSegID).head)).x;
			newAPO.y = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(tailSegID).head)).y;
			newAPO.z = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(tailSegID).head)).z;
			newAPO.color = tailCluster.second.second;
			segTailAPOs.push_back(newAPO);
		}
	}

	writeSWC_file(saveNamePrefixQ + "_inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "_inputTree_tailSeg2clusterCheck.apo", segTailAPOs);
}

void NeuronStructNavigator::Tester::assignRGBcolors(map<int, pair<boost::container::flat_set<int>, RGBA8>>& inputMap) const
{
	int colorR = 10, colorG = 10, colorB = 10;
	for (auto& mapPair : inputMap)
	{
		RGBA8 newColor;
		if (mapPair.first % 3 == 0)
		{
			newColor.r = colorR % 255;
			newColor.g = 0;
			newColor.b = 0;
			colorR += 10;
		}
		else if (mapPair.first % 3 == 1)
		{
			newColor.r = 0;
			newColor.g = colorG % 255;
			newColor.b = 0;
			colorG += 10;
		}
		else if (mapPair.first % 3 == 2)
		{
			newColor.r = 0;
			newColor.g = 0;
			newColor.b = colorB % 255;
			colorB += 10;
		}

		mapPair.second.second = newColor;
	}
}