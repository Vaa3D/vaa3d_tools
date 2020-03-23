#include "FragmentPostProcessor.h"
#include "FragTraceTester.h"


NeuronTree FragmentPostProcessor::scaleTree(const NeuronTree& inputTree, const float factor, const float imgOrigin[])
{
	NeuronTree scaledTree = NeuronStructUtil::swcScale(inputTree, factor, factor, factor);
	NeuronTree scaledShiftedTree = NeuronStructUtil::swcShift(scaledTree, imgOrigin[0], imgOrigin[1], imgOrigin[2]);

	return scaledShiftedTree;
}

NeuronTree FragmentPostProcessor::treeScaleBack(const NeuronTree& inputTree, const float factor, const float imgOrigin[])
{
	NeuronTree shiftBackTree = NeuronStructUtil::swcShift(inputTree, -imgOrigin[0], -imgOrigin[1], -imgOrigin[2]);
	NeuronTree shiftScaleBackTree = NeuronStructUtil::swcScale(shiftBackTree, 1 / factor, 1 / factor, 1 / factor);

	return shiftScaleBackTree;
}

NeuronTree FragmentPostProcessor::integrateNewTree(const NeuronTree& existingTree, const NeuronTree& preFinalizedTree, const int minNodeNum)
{
	if (!existingTree.listNeuron.isEmpty())
	{
		NeuronTree scaledBackExistingTree = this->treeScaleBack(existingTree, this->scalingFactor, this->imgOrigin);
		profiledTree profiledPRE_FINALIZED_newPart(TreeGrower::swcSamePartExclusion(preFinalizedTree, scaledBackExistingTree, 4, 8));
		if (minNodeNum > 0) profiledPRE_FINALIZED_newPart.tree = NeuronStructUtil::singleDotRemove(profiledPRE_FINALIZED_newPart, minNodeNum);
		vector<NeuronTree> trees = { profiledPRE_FINALIZED_newPart.tree, scaledBackExistingTree };
		profiledTree combinedProfiledTree(NeuronStructUtil::swcCombine(trees));
		return this->scaleTree(combinedProfiledTree.tree, this->scalingFactor, this->imgOrigin);

	}
	else return this->scaleTree(preFinalizedTree, this->scalingFactor, this->imgOrigin);
}