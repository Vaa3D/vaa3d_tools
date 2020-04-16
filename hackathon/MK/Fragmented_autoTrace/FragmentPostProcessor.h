#ifndef FRAGMENTPOSTPROCESSOR_H
#define FRAGMENTPOSTPROCESSOR_H

#include "NeuronStructExplorer.h"
#include "TreeGrower.h"
#include "TreeTrimmer.h"
#include "NeuronStructUtilities.h"

class FragmentPostProcessor
{
	friend class FragTraceTester;

public:
	~FragmentPostProcessor();

	float scalingFactor;
	float* imgOrigin;
	int* volumeAdjustedBounds;
	
	NeuronTree scaleTree(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const;
	NeuronTree treeScaleBack(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const;

	set<vector<float>> getProbesFromLabeledExistingSegs(const NeuronTree& inputTree) const;
	void getClusterChain(const profiledTree& inputProfiledTree, const set<int>& seedCluster, map<int, segEndClusterUnit*>& clusterChains) const;
	void rc_clusterChain_cleanUp(segEndClusterUnit* currCluster) const;
	NeuronTree getTreeFromClusterChains(const map<int, segEndClusterUnit*>& clusterChains, const profiledTree& inputProfiledTree) const;

private:
	void rc_clusterChain(const profiledTree& inputProfiledTree, segEndClusterUnit* currClusterUnitPtr, set<int>& includedClusterIDs) const;
	void rc_getSegIDsFromClusterChain(boost::container::flat_set<int>& segIDs, const segEndClusterUnit* currClusterUnitPtr) const;

public:
	NeuronTree integrateNewTree(const NeuronTree& existingTree, const NeuronTree& preFinalizedTree, const int minNodeNum) const;

};

#endif