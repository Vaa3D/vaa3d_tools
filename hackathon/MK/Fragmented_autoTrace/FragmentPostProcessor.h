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

	float scalingFactor;
	float* imgOrigin;
	int* volumeAdjustedBounds;
	
	NeuronTree scaleTree(const NeuronTree& inputTree, const float factor, const float imgOrigin[]);
	NeuronTree treeScaleBack(const NeuronTree& inputTree, const float factor, const float imgOrigin[]);
	NeuronTree integrateNewTree(const NeuronTree& existingTree, const NeuronTree& preFinalizedTree, const int minNodeNum);
};

#endif