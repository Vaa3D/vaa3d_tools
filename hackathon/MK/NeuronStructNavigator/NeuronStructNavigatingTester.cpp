#include <iostream>
#include <fstream>

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

NeuronStructNavigator::Tester* NeuronStructNavigator::Tester::instance()
{
	NeuronStructNavigator::Tester::testerInstance = new NeuronStructNavigator::Tester();
	return NeuronStructNavigator::Tester::testerInstance;
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

void NeuronStructNavigator::Tester::saveLoops(const profiledTree& inputProfiledTree, const set<int>& loopingSegIDs, const QString& saveNameQ) const
{
	NeuronTree outputTree;
	//for (auto& segID : inputProfiledTree.loopingSegs) outputTree.listNeuron.append(inputProfiledTree.segs.at(segID).nodes);

	writeSWC_file(saveNameQ, outputTree);
}

void NeuronStructNavigator::Tester::checkClusterNodeMap(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	if (inputProfiledTree.segEndClusterNodeMap.empty())
	{
		cout << "Seg end cluster node map is empty. Do nothing." << endl;
		return;
	}

	string outputTextFullName = saveNamePrefixQ.toStdString() + "clusterNode.txt";
	ofstream outFile(outputTextFullName);
	for (auto& cluster : inputProfiledTree.segEndClusterNodeMap)
	{
		outFile << cluster.first << ": ";
		for (auto& node : cluster.second)
			outFile << "(" << node.at(0) << ", " << node.at(1) << ", " << node.at(2) << ")\t";
		outFile << endl;
	}
}

void NeuronStructNavigator::Tester::checkClusterCentroidMap(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	if (inputProfiledTree.segEndClusterCentroidMap.empty())
	{
		cout << "Seg end cluster centroid map is empty. Do nothing." << endl;
		return;
	}

	string outputTextFullName = saveNamePrefixQ.toStdString() + "clusterCentroid.txt";
	ofstream outFile(outputTextFullName);
	for (auto& cluster : inputProfiledTree.segEndClusterCentroidMap)
		outFile << cluster.first << ": (" << cluster.second.at(0) << ", " << cluster.second.at(1) << ", " << cluster.second.at(2) << ")" << endl;
}

map<int, set<vector<float>>> NeuronStructNavigator::Tester::getSegEndClusterNodeMap(const profiledTree& inputProfiledTree) const
{
	map<int, set<vector<float>>> outputSegClusterNodeMap;
	
	for (auto& headCluster : inputProfiledTree.segHeadClusters)
	{
		set<vector<float>> headSet;
		for (auto& headSegID : headCluster.second)
		{
			vector<float> headCoord;
			const NeuronSWC headNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head));
			headCoord.push_back(headNode.x);
			headCoord.push_back(headNode.y);
			headCoord.push_back(headNode.z);
			headSet.insert(headCoord);
		}
		outputSegClusterNodeMap.insert({ headCluster.first, headSet });
	}

	for (auto& tailCluster : inputProfiledTree.segTailClusters)
	{
		set<vector<float>> tailSet;
		for (auto& tailSegID : tailCluster.second)
		{
			for (auto& tailNodeID : inputProfiledTree.segs.at(tailSegID).tails)
			{
				vector<float> tailCoord;
				const NeuronSWC tailNode = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(tailNodeID));
				tailCoord.push_back(tailNode.x);
				tailCoord.push_back(tailNode.y);
				tailCoord.push_back(tailNode.z);
				tailSet.insert(tailCoord);
			}
		}

		if (outputSegClusterNodeMap.find(tailCluster.first) != outputSegClusterNodeMap.end())
			outputSegClusterNodeMap[tailCluster.first].insert(tailSet.begin(), tailSet.end());
		else outputSegClusterNodeMap.insert({ tailCluster.first, tailSet });
	}

	return outputSegClusterNodeMap;
}

void NeuronStructNavigator::Tester::checkSegHeadClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	map<int, pair<boost::container::flat_set<int>, RGBA8>> headClusterAPOs;
	RGBA8 color;
	string outputTextFullName = saveNamePrefixQ.toStdString() + "headClusterCheck.txt";
	ofstream outfile(outputTextFullName);
	for (auto& headCluster: inputProfiledTree.segHeadClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = headCluster.second.begin(); it != headCluster.second.end(); ++it)
			headClusterAPOs.insert({ headCluster.first, pair<boost::container::flat_set<int>, RGBA8>(headCluster.second, color) });
	}
	this->assignRGBcolors(headClusterAPOs);

	QList<CellAPO> segHeadAPOs;
	for (auto& headCluster : headClusterAPOs)
	{
		outfile << headCluster.first << ":\t";
		for (auto& headSegID : headCluster.second.first)
		{
			CellAPO newAPO;
			newAPO.x = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).x;
			newAPO.y = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).y;
			newAPO.z = inputProfiledTree.tree.listNeuron.at(inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(headSegID).head)).z;
			newAPO.color = headCluster.second.second;
			segHeadAPOs.push_back(newAPO);

			outfile << "(" << newAPO.x << ", " << newAPO.y << ", " << newAPO.z << ")\t";
		}
		outfile << endl;
	}
	outfile.close();

	writeSWC_file(saveNamePrefixQ + "inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "inputTree_headClusters.apo", segHeadAPOs);
}

void NeuronStructNavigator::Tester::checkSegTailClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	map<int, pair<boost::container::flat_set<int>, RGBA8>> tailClusterApos;
	RGBA8 color;
	string outputTextFullName = saveNamePrefixQ.toStdString() + "tailClusterCheck.txt";
	ofstream outfile(outputTextFullName);
	for (auto& tailCluster : inputProfiledTree.segTailClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = tailCluster.second.begin(); it != tailCluster.second.end(); ++it)
			tailClusterApos.insert({ tailCluster.first, pair<boost::container::flat_set<int>, RGBA8>(tailCluster.second, color) });
	}
	this->assignRGBcolors(tailClusterApos);

	QList<CellAPO> segTailAPOs;
	for (auto& tailCluster : tailClusterApos)
	{
		outfile << tailCluster.first << ":\t";
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

				outfile << "(" << newAPO.x << ", " << newAPO.y << ", " << newAPO.z << ")\t";
			}
		}
		outfile << endl;
	}
	outfile.close();

	writeSWC_file(saveNamePrefixQ + "inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "inputTree_tailClusters.apo", segTailAPOs);
}

void NeuronStructNavigator::Tester::checkSegEndClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const
{
	map<int, pair<boost::container::flat_set<int>, RGBA8>> headClusterApos;
	map<int, pair<boost::container::flat_set<int>, RGBA8>> tailClusterApos;
	
	RGBA8 color;
	for (auto& headCluster : inputProfiledTree.segHeadClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = headCluster.second.begin(); it != headCluster.second.end(); ++it)
			headClusterApos.insert({ headCluster.first, pair<boost::container::flat_set<int>, RGBA8>(headCluster.second, color) });
	}
	for (auto& tailCluster : inputProfiledTree.segTailClusters)
	{
		for (boost::container::flat_set<int>::const_iterator it = tailCluster.second.begin(); it != tailCluster.second.end(); ++it)
			tailClusterApos.insert({ tailCluster.first, pair<boost::container::flat_set<int>, RGBA8>(tailCluster.second, color) });
	}
	this->assignRGBcolors(headClusterApos, tailClusterApos);

	QList<CellAPO> segHeadAPOs, segTailAPOs;
	for (auto& headCluster : headClusterApos)
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

	writeSWC_file(saveNamePrefixQ + "inputTree.swc", inputProfiledTree.tree);
	writeAPO_file(saveNamePrefixQ + "inputTree_headClusters.apo", segHeadAPOs);
	writeAPO_file(saveNamePrefixQ + "inputTree_tailClusters.apo", segTailAPOs);
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

void NeuronStructNavigator::Tester::assignRGBcolors(map<int, pair<boost::container::flat_set<int>, RGBA8>>& inputHeadMap, map<int, pair<boost::container::flat_set<int>, RGBA8>>& inputTailMap) const
{
	int maxSegID;
	inputHeadMap.rbegin()->first > inputTailMap.rbegin()->first ? maxSegID = inputHeadMap.rbegin()->first : inputTailMap.rbegin()->first;
	
	int colorR = 10, colorG = 10, colorB = 10;
	for (int i = 0; i <= maxSegID; ++i)
	{
		RGBA8 newColor;
		switch (i % 3)
		{
		case 0:
			newColor.r = colorR % 255;
			newColor.g = 0;
			newColor.b = 0;
			colorR += 10;
			if (inputHeadMap.find(i) != inputHeadMap.end()) inputHeadMap[i].second = newColor;
			if (inputTailMap.find(i) != inputTailMap.end()) inputTailMap[i].second = newColor;
			break;
		case 1:
			newColor.r = 0;
			newColor.g = colorG % 255;
			newColor.b = 0;
			colorG += 10;
			if (inputHeadMap.find(i) != inputHeadMap.end()) inputHeadMap[i].second = newColor;
			if (inputTailMap.find(i) != inputTailMap.end()) inputTailMap[i].second = newColor;
			break;
		case 2:
			newColor.r = 0;
			newColor.g = 0;
			newColor.b = colorB % 255;
			colorB += 10;
			if (inputHeadMap.find(i) != inputHeadMap.end()) inputHeadMap[i].second = newColor;
			if (inputTailMap.find(i) != inputTailMap.end()) inputTailMap[i].second = newColor;
			break;
		default:
			break;
		}
	}
}

void NeuronStructNavigator::Tester::printoutSegUnitInfo(const segUnit& inputSegUnit) const
{
	cout << "seg ID: " << inputSegUnit.segID << endl;
	cout << "head node: " << inputSegUnit.head << endl << endl;
	for (auto& node : inputSegUnit.nodes)
		cout << "n:" << node.n << " parent:" << node.parent << endl;
	cout << endl;
}