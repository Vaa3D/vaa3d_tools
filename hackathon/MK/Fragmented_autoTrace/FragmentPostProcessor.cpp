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