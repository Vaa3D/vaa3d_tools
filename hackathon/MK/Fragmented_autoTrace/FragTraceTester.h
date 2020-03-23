#ifndef FRAGTRACETESTER_H
#define FRAGTRACETESTER_H

#include <memory>

#include "FragTraceControlPanel.h"
#include "FragTraceManager.h"

class FragTraceTester
{
public:
	static FragTraceTester* testerInstance;
	static FragTraceTester* instance(FragTraceControlPanel* controlPanelPtr);
	static FragTraceTester* getInstance();
	static void uninstance();
	static bool isInstantiated() { return testerInstance != nullptr; }

	shared_ptr<FragTraceControlPanel*> sharedControlPanelPtr;
	shared_ptr<FragTraceManager*> sharedTraceManagerPtr;

	// These 3 pointers get assigned from FragTraceControlPanel and are pointing to FragTraceControlPanel's own arrays. 
	// No need to delete when finishing FragTraceTester instance.
	float *imgDims, *imgRes, *imgOrigin; 
	void printOutVolInfo();
	void printOutImgInfo();
	void imgVolumeCheck(const Image4DSimple* inputImage, const string& saveName);
	void saveIntermediateImgSlices(const string& regImgName, const QString& prefixQ, V3DLONG dims[]);

	enum interResultType { combine,
						   branchBreak, downSampled, iteredConnected, angleSmooth_lengthDistRatio, angleSmooth_distThre_3nodes, noFloatingTinyFrag, 
						   axonCentroid, axonSkeleton,
						   denBlob, denPeriBlob, denRaw, denDnSampledRaw, spikeRemove, spikeRootStraighten, hookRemove, denCentroid, denSkeleton };

	inline void axonTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);
	inline void denTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);
	inline void generalTreeFormingInterResults(interResultType resultType, const NeuronTree& tree, const QString prefixQ);

	map<int, set<vector<float>>> clusterSegEndMarkersGen(const set<int>& clusterIDs, const profiledTree& inputProfiledTree);
	map<int, RGBA8> clusterColorGen_RGB(const set<int>& clusterIDs);
	void pushMarkers(const set<vector<float>>& markerCoords, RGBA8 color) { (*this->sharedControlPanelPtr)->CViewerPortal->pushMarkersfromTester(markerCoords, color); }

private:
	FragTraceTester(FragTraceControlPanel* controlPanelPtr);
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