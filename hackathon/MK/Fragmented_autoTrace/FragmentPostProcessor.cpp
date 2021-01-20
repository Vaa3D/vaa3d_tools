#include "FragmentPostProcessor.h"
#include "FragTraceTester.h"

FragmentPostProcessor::~FragmentPostProcessor()
{
	if (this->imgOrigin != nullptr)
	{
		delete[] this->imgOrigin;
		this->imgOrigin = nullptr;
	}

	if (this->volumeAdjustedBounds != nullptr)
	{
		delete[] this->volumeAdjustedBounds;
		this->volumeAdjustedBounds = nullptr;
	}
}

NeuronTree FragmentPostProcessor::scaleTree(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const
{
	NeuronTree scaledTree = NeuronStructUtil::swcScale(inputTree, factor, factor, factor);
	NeuronTree scaledShiftedTree = NeuronStructUtil::swcShift(scaledTree, imgOrigin[0], imgOrigin[1], imgOrigin[2]);

	return scaledShiftedTree;
}

NeuronTree FragmentPostProcessor::treeScaleBack(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const
{
	NeuronTree shiftBackTree = NeuronStructUtil::swcShift(inputTree, -imgOrigin[0], -imgOrigin[1], -imgOrigin[2]);
	NeuronTree shiftScaleBackTree = NeuronStructUtil::swcScale(shiftBackTree, 1 / factor, 1 / factor, 1 / factor);

	return shiftScaleBackTree;
}

profiledTree FragmentPostProcessor::straightenSpikeRoots(const profiledTree& inputProfiledTree, double angleThre)
{
	profiledTree outputProfiledTree = inputProfiledTree;
	//cout << " -- spike root number:" << outputProfiledTree.spikeRootIDs.size() << endl;
	for (boost::container::flat_set<int>::iterator it = outputProfiledTree.spikeRootIDs.begin(); it != outputProfiledTree.spikeRootIDs.end(); ++it)
	{
		if (outputProfiledTree.node2LocMap.find(*it) != outputProfiledTree.node2LocMap.end() && outputProfiledTree.node2childLocMap.find(*it) != outputProfiledTree.node2childLocMap.end())
		{
			if (outputProfiledTree.tree.listNeuron.at(outputProfiledTree.node2LocMap.at(*it)).parent != -1 && outputProfiledTree.node2childLocMap.at(*it).size() == 1)
			{
				NeuronSWC angularNode = outputProfiledTree.tree.listNeuron.at(outputProfiledTree.node2LocMap.at(*it));
				NeuronSWC endNode1 = outputProfiledTree.tree.listNeuron.at(outputProfiledTree.node2LocMap.at(angularNode.parent));
				NeuronSWC endNode2 = outputProfiledTree.tree.listNeuron.at(*outputProfiledTree.node2childLocMap.at(*it).begin());

				float angle = NeuronGeoGrapher::get3nodesFormingAngle<float>(angularNode, endNode1, endNode2);
				if (angle <= 0.6)
				{
					outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*it)].x = (endNode1.x + endNode2.x) / 2;
					outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*it)].y = (endNode1.y + endNode2.y) / 2;
					outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(*it)].z = (endNode1.z + endNode2.z) / 2;
				}
			}
		}
	}

	profiledTreeReInit(outputProfiledTree);
	return outputProfiledTree;
}


/* =========================== Extended Axon Tracing Methods =========================== */
set<vector<float>> FragmentPostProcessor::getProbesFromLabeledExistingSegs(const NeuronTree& inputTree) const
{
	set<vector<float>> outputProbeSet;
	profiledTree profiledExistingTree(inputTree);
	for (map<int, segUnit>::iterator segIt = profiledExistingTree.segs.begin(); segIt != profiledExistingTree.segs.end(); ++segIt)
	{
		if (segIt->second.nodes.begin()->type == 16) continue;

		const NeuronSWC& headNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(segIt->second.head));
		if (headNode.x >= this->volumeAdjustedBounds[0] && headNode.y <= this->volumeAdjustedBounds[1] &&
			headNode.y >= this->volumeAdjustedBounds[2] && headNode.y <= this->volumeAdjustedBounds[3] &&
			headNode.z >= this->volumeAdjustedBounds[4] && headNode.z <= this->volumeAdjustedBounds[5])
		{
			vector<float> newProbe = { headNode.x, headNode.y, headNode.z };
			outputProbeSet.insert(newProbe);
		}

		for (auto& tailID : segIt->second.tails)
		{
			const NeuronSWC& tailNode = segIt->second.nodes.at(segIt->second.seg_nodeLocMap.at(tailID));
			if (tailNode.x >= this->volumeAdjustedBounds[0] && tailNode.y <= this->volumeAdjustedBounds[1] &&
				tailNode.y >= this->volumeAdjustedBounds[2] && tailNode.y <= this->volumeAdjustedBounds[3] &&
				tailNode.z >= this->volumeAdjustedBounds[4] && tailNode.z <= this->volumeAdjustedBounds[5])
			{
				vector<float> newProbe = { tailNode.x, tailNode.y, tailNode.z };
				outputProbeSet.insert(newProbe);
			}
		}
	}
	
	return outputProbeSet;
}

void FragmentPostProcessor::getClusterChain(const profiledTree& inputProfiledTree, const set<int>& seedCluster, map<int, segEndClusterUnit*>& clusterChains) const
{
	if (inputProfiledTree.segHeadClusters.empty() || inputProfiledTree.segTailClusters.empty()) return;

	if (!clusterChains.empty())
	{
		for (auto& chain : clusterChains) this->rc_clusterChain_cleanUp(chain.second);
		clusterChains.clear();
	}

	set<int> includedClusterIDs;
	for (set<int>::const_iterator it = seedCluster.begin(); it != seedCluster.end(); ++it)
	{
		includedClusterIDs.insert(*it);
		segEndClusterUnit* headUnit = new segEndClusterUnit;
		headUnit->ID = *it;
		this->rc_clusterChain(inputProfiledTree, headUnit, includedClusterIDs);
		clusterChains.insert({ *it, headUnit });
	}
}

void FragmentPostProcessor::rc_clusterChain(const profiledTree& inputProfiledTree, segEndClusterUnit* currClusterUnitPtr, set<int>& includedClusterIDs) const
{
	currClusterUnitPtr->childClusterMap.clear();
	currClusterUnitPtr->headSegs = inputProfiledTree.segHeadClusters.at(currClusterUnitPtr->ID);
	currClusterUnitPtr->tailSegs = inputProfiledTree.segTailClusters.at(currClusterUnitPtr->ID);
	
	set<segEndClusterUnit*> currLevelClusterUnits;
	for (auto& headSegID : currClusterUnitPtr->headSegs)
	{
		int tailSegClusterID = inputProfiledTree.tailSeg2ClusterMap.at(headSegID);
		if (includedClusterIDs.find(tailSegClusterID) == includedClusterIDs.end())
		{
			includedClusterIDs.insert(tailSegClusterID);
			segEndClusterUnit* newUnit = new segEndClusterUnit;
			newUnit->ID = tailSegClusterID;
			newUnit->parentCluster = currClusterUnitPtr;
			currClusterUnitPtr->childClusterMap.insert({ tailSegClusterID, newUnit });
		}
	}

	for (auto& tailSegID : currClusterUnitPtr->tailSegs)
	{
		int headSegClusterID = inputProfiledTree.headSeg2ClusterMap.at(tailSegID);
		if (includedClusterIDs.find(headSegClusterID) == includedClusterIDs.end())
		{
			includedClusterIDs.insert(headSegClusterID);
			segEndClusterUnit* newUnit = new segEndClusterUnit;
			newUnit->ID = headSegClusterID;
			newUnit->parentCluster = currClusterUnitPtr;
			currClusterUnitPtr->childClusterMap.insert({ headSegClusterID, newUnit });
		}
	}

	if (currClusterUnitPtr->childClusterMap.empty()) return;
	else
	{
		for (map<int, segEndClusterUnit*>::iterator it = currClusterUnitPtr->childClusterMap.begin(); it != currClusterUnitPtr->childClusterMap.end(); ++it)
			this->rc_clusterChain(inputProfiledTree, it->second, includedClusterIDs);
	}
}

void FragmentPostProcessor::rc_clusterChain_cleanUp(segEndClusterUnit* currCluster) const
{
	if (currCluster->childClusterMap.empty())
	{
		delete currCluster;
		return;
	}

	for (map<int, segEndClusterUnit*>::iterator childClusterIt = currCluster->childClusterMap.begin(); childClusterIt != currCluster->childClusterMap.end(); ++childClusterIt)
		this->rc_clusterChain_cleanUp(childClusterIt->second); 
}

NeuronTree FragmentPostProcessor::getTreeFromClusterChains(const map<int, segEndClusterUnit*>& clusterChains, const profiledTree& inputProfiledTree) const
{
	boost::container::flat_set<int> segIDs;
	for (auto& chain : clusterChains)
	{
		segIDs.insert(inputProfiledTree.segHeadClusters.at(chain.second->ID).begin(), inputProfiledTree.segHeadClusters.at(chain.second->ID).end());
		segIDs.insert(inputProfiledTree.segTailClusters.at(chain.second->ID).begin(), inputProfiledTree.segTailClusters.at(chain.second->ID).end());

		for (auto& childCluster : chain.second->childClusterMap)
			this->rc_getSegIDsFromClusterChain(segIDs, childCluster.second);
	}

	NeuronTree outputTree;
	for (auto& segID : segIDs)
		outputTree.listNeuron.append(inputProfiledTree.segs.at(segID).nodes);

	return outputTree;
}

void FragmentPostProcessor::rc_getSegIDsFromClusterChain(boost::container::flat_set<int>& segIDs, const segEndClusterUnit* currClusterUnitPtr) const
{
	segIDs.insert(currClusterUnitPtr->headSegs.begin(), currClusterUnitPtr->headSegs.end());
	segIDs.insert(currClusterUnitPtr->tailSegs.begin(), currClusterUnitPtr->tailSegs.end());

	if (currClusterUnitPtr->childClusterMap.empty()) return;
	else
	{
		for (auto& childCluster : currClusterUnitPtr->childClusterMap)
			this->rc_getSegIDsFromClusterChain(segIDs, childCluster.second);
	}
}

profiledTree FragmentPostProcessor::selectiveType16(const profiledTree& extendedTree, const profiledTree& autoTracedTree, int radius) const
{
	profiledTree outputProfiledTree;
	if (extendedTree.tree.listNeuron.isEmpty())
	{
		cerr << "Extended tree is empty. Do nothing and return." << endl;
		return outputProfiledTree;
	}
	if (autoTracedTree.segEndClusterNodeMap.empty() || autoTracedTree.segEndClusterCentroidMap.empty())
	{
		cerr << "Auto-traced tree doesn't have segEndClusterNodeMap or segEndClusterCentroidMap propagated." << endl;
		cerr << "Do nothing and return." << endl;
		return outputProfiledTree;
	}

	for (auto& cluster : autoTracedTree.segEndClusterCentroidMap)
	{
		cout << "cluster ID - " << cluster.first << endl;
		for (auto& node : extendedTree.tree.listNeuron)
		{
			if (sqrtf((cluster.second.at(0) - node.x) * (cluster.second.at(0) - node.x) +
					  (cluster.second.at(1) - node.y) * (cluster.second.at(1) - node.y) +
					  (cluster.second.at(2) - node.z) * (cluster.second.at(2) - node.z)) <= radius)
			{
				for (auto& clusterNode : autoTracedTree.segEndClusterNodeMap.at(cluster.first))
				{
					string nodeKey = to_string(clusterNode.at(0)) + "_" + to_string(clusterNode.at(1)) + "_" + to_string(clusterNode.at(2));
					pair<boost::container::flat_multimap<string, int>::const_iterator, boost::container::flat_multimap<string, int>::const_iterator> range = autoTracedTree.nodeCoordKey2segMap.equal_range(nodeKey);
					for (boost::container::flat_multimap<string, int>::const_iterator rangeIt = range.first; rangeIt != range.second; ++rangeIt)
					{
						outputProfiledTree.tree.listNeuron.append(autoTracedTree.segs.at(rangeIt->second).nodes);
						for (auto& node : autoTracedTree.segs.at(rangeIt->second).nodes)
							cout << node.n << " " << node.x << " " << node.y << " " << node.z << " " << node.parent << endl;
					}
				}
				goto SEGEND_CLUSTER_INCLUDED;
			}
		}

	SEGEND_CLUSTER_INCLUDED:
		cout << endl;
		continue;
	}
	integratedDataTypes::profiledTreeReInit(outputProfiledTree);
	cout << outputProfiledTree.segs.size() << endl;
	writeSWC_file("D:\\Work\\FragTrace\\selective16.swc", outputProfiledTree.tree);

	return outputProfiledTree;
}
/* ===================== END of [Extended Axon Tracing Methods] ======================== */


NeuronTree FragmentPostProcessor::integrateNewTree(const NeuronTree& existingTree, const NeuronTree& preFinalizedTree, const int minNodeNum) const
{
	if (!existingTree.listNeuron.isEmpty())
	{
		NeuronTree scaledBackExistingTree = this->treeScaleBack(existingTree, this->scalingFactor, this->imgOrigin);
		profiledTree profiledPRE_FINALIZED_newPart(TreeGrower::swcSamePartExclusion(preFinalizedTree, scaledBackExistingTree, 4, 8));
		if (minNodeNum > 0) profiledPRE_FINALIZED_newPart.tree = NeuronStructUtil::singleDotRemove(profiledPRE_FINALIZED_newPart, minNodeNum);
		vector<NeuronTree> trees = { profiledPRE_FINALIZED_newPart.tree, scaledBackExistingTree };
		NeuronTree outputTree = NeuronStructUtil::swcCombine(trees);
		return this->scaleTree(outputTree, this->scalingFactor, this->imgOrigin);
	}
	else return this->scaleTree(preFinalizedTree, this->scalingFactor, this->imgOrigin);
}