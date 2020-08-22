//------------------------------------------------------------------------------
// Copyright (c) 2020 Hsienchi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [FragmentPostProcessor] handles segment post-processing after segments are generated, such as:
*    1. extended axon tracing, 2. selection of nearby segments, etc.
*
*  This class is owned by [FragTraceManager] and is not accessible to any other class. 
*  It is intended to serve as a solution provider to [FragTraceManager].
*
********************************************************************************/

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
	// Destructor cleans up image property pointers.
	~FragmentPostProcessor();

	float scalingFactor;
	float* imgOrigin;
	int* volumeAdjustedBounds;
	
	// Scale tree to global coordinate scale.
	NeuronTree scaleTree(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const;

	// Scale tree to local volume coordinate scale.
	NeuronTree treeScaleBack(const NeuronTree& inputTree, const float factor, const float imgOrigin[]) const;

	profiledTree straightenSpikeRoots(const profiledTree& inputProfiledTree, double angleThre = 0.5);

	/* ----------------- Extended Axon Tracing Methods ----------------- */
	// Extended axon tracing employs "chained" segment end cluster approach.
	// Each [integratedDataTypes::segEndClusterUnit] leads to its next level of [integratedDataTypes::segEndClusterUnit]s connected by segments.

	// Acquire all labeled segment ends' coordinates as "probes".
	set<vector<float>> getProbesFromLabeledExistingSegs(const NeuronTree& inputTree) const;

	// Use distance threshold to establish the hierarchical segment end cluster chain starting from the labeled segment ends. 
	void getClusterChain(const profiledTree& inputProfiledTree, const set<int>& seedCluster, map<int, segEndClusterUnit*>& clusterChains) const;
	
	// Acquire all segments that are included in the segment end cluster chain.
	NeuronTree getTreeFromClusterChains(const map<int, segEndClusterUnit*>& clusterChains, const profiledTree& inputProfiledTree) const;

	// Recursive [integratedDataTypes::segEndClusterUnit] pointer deletion and memory release.
	void rc_clusterChain_cleanUp(segEndClusterUnit* currCluster) const;	

	profiledTree selectiveType16(const profiledTree& extendedTree, const profiledTree& autoTracedTree, int radius = 10) const;

private:
	void rc_clusterChain(const profiledTree& inputProfiledTree, segEndClusterUnit* currClusterUnitPtr, set<int>& includedClusterIDs) const;
	void rc_getSegIDsFromClusterChain(boost::container::flat_set<int>& segIDs, const segEndClusterUnit* currClusterUnitPtr) const;
	/* ------------- END of [Extended Axon Tracing Methods] ------------ */

public:
	// Take the newly traced tree, exclude the part that overlaps the existing tree, and then combine as 1 single tree.
	NeuronTree integrateNewTree(const NeuronTree& existingTree, const NeuronTree& preFinalizedTree, const int minNodeNum) const;

};

#endif