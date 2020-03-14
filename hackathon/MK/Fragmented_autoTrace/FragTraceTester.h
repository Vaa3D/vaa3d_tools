#ifndef FRAGTRACETESTER_H
#define FRAGTRACETESTER_H

#include <memory>

#include "FragTraceControlPanel.h"
#include "FragTraceManager.h"

class FragTraceTester
{
public:
	FragTraceTester(FragTraceControlPanel* controlPanelPtr);

	static FragTraceTester* testerInstance;
	static FragTraceTester* instance(FragTraceControlPanel* controlPanelPtr);
	static FragTraceTester* getInstance();
	static void uninstance();
	static bool isInstantiated() { return testerInstance != nullptr; }

	shared_ptr<FragTraceControlPanel*> sharedControlPanelPtr;
	shared_ptr<FragTraceManager*> sharedTraceManagerPtr;

	enum interResultType { branchBreak, downSampled, iteredConnected, angleSmooth_lengthDistRatio, noFloatingTinyFrag, axonCentroid, axonSkeleton,
						   denBlob, denPeriBlob, denRaw, denDnSampledRaw, spikeRemove, spikeRootStraighten, hookRemove, denCentroid, denSkeleton };

	inline void axonTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);
	inline void denTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);

	profiledTree segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ);
	void scale(profiledTree& inputProfiledTree);
};

inline void FragTraceTester::axonTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ)
{
	QString savePathQ = prefixQ;

	switch (resultType)
	{
	case branchBreak:
		savePathQ += "axonBranchBreakTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case downSampled:
		savePathQ += "axonDownSampledTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case iteredConnected:
		savePathQ += "axonIteredConnectedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case angleSmooth_lengthDistRatio:
		savePathQ += "axonAngleSmoothedLenghDistRatioTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case noFloatingTinyFrag:
		savePathQ += "axonNoFloatingTinyFragTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case axonCentroid:
		savePathQ += "axonCentroidTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case axonSkeleton:
		savePathQ += "axonSkeletonTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	default:
		break;
	}
}

inline void FragTraceTester::denTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ)
{
	QString savePathQ = prefixQ;

	switch (resultType)
	{
	case denBlob:
		savePathQ += "denBlob.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case denPeriBlob:
		savePathQ += "periBlob.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case denRaw:
		savePathQ += "denRawTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case denDnSampledRaw:
		savePathQ += "denRawDownSampledTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case spikeRemove:
		savePathQ += "denSplikeRemovedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case spikeRootStraighten:
		savePathQ += "denSplikeRootStraightenedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case hookRemove:
		savePathQ += "denHookRemovedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case denCentroid:
		savePathQ += "denCentroidTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case denSkeleton:
		savePathQ += "denSkeletonTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	default:
		break;
	}
}

#endif