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

	void imgVolumeCheck(const Image4DSimple* inputImage, const string& saveName);
	void saveIntermediateImgSlices(const string& regImgName, const QString& prefixQ, V3DLONG dims[]);

	enum interResultType { combine,
						   branchBreak, downSampled, iteredConnected, angleSmooth_lengthDistRatio, angleSmooth_distThre_3nodes, noFloatingTinyFrag, 
						   axonCentroid, axonSkeleton,
						   denBlob, denPeriBlob, denRaw, denDnSampledRaw, spikeRemove, spikeRootStraighten, hookRemove, denCentroid, denSkeleton };

	inline void axonTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);
	inline void denTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);
	inline void generalTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);

	void pushMarkers(const set<vector<float>>& markerCoords) { (*this->sharedControlPanelPtr)->CViewerPortal->pushMarkersfromTester(markerCoords); }
	profiledTree segEndClusterCheck(const profiledTree& inputProfiledTree, QString savePathQ);
	void scale(profiledTree& inputProfiledTree);
};

inline void FragTraceTester::generalTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ)
{
	QString savePathQ = prefixQ;

	switch (resultType)
	{
	case combine:
		savePathQ += "combinedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	default:
		break;
	}
}

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
	case angleSmooth_distThre_3nodes:
		savePathQ += "axonAngleSmoothedDistThre3nodesTree.swc";
		writeSWC_file(savePathQ, tree);
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
		savePathQ += "denSpikeRemovedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case spikeRootStraighten:
		savePathQ += "denSpikeRootStraightenedTree.swc";
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
	case angleSmooth_lengthDistRatio:
		savePathQ += "denAngleSmoothedLenghDistRatioTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case angleSmooth_distThre_3nodes:
		savePathQ += "denAngleSmoothedDistThre3nodesTree.swc";
		writeSWC_file(savePathQ, tree);
	case noFloatingTinyFrag:
		savePathQ += "denNoFloatingTinyFragTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case branchBreak:
		savePathQ += "denBranchBreakTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case iteredConnected:
		savePathQ += "denIteredConnectedTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	case downSampled:
		savePathQ += "denDownSampledTree.swc";
		writeSWC_file(savePathQ, tree);
		break;
	default:
		break;
	}
}

#endif