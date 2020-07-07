#include <deque>
#include <cstdlib>
#include <thread>

#include <boost/container/flat_set.hpp>

#include "NeuronStructNavigatingTester.h"

#include "FragTracer_Define.h"
#include "FragTraceManager.h"
#include "processManager.h"
#include "FragTraceTester.h"
#include "FeatureExtractor.h"

using NSlibTester = NeuronStructNavigator::Tester;

FragTraceManager::FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices)
{
	this->finalSaveRootQ = "D:\\Work\\FragTrace";

	char* numProcsC;
	numProcsC = getenv("NUMBER_OF_PROCESSORS");
	string numProcsString(numProcsC);
	this->numProcs = stoi(numProcsString);

	this->mode = mode;
	this->partialVolumeTracing = false;
	this->partialVolumeLowerBoundaries = { 0, 0, 0 };

	displayImgDim[0] = inputImg4DSimplePtr->getXDim();
	displayImgDim[1] = inputImg4DSimplePtr->getYDim();
	displayImgDim[2] = inputImg4DSimplePtr->getZDim();
	cout << " -- Displaying image block dimensions: " << displayImgDim[0] << " " << displayImgDim[1] << " " << displayImgDim[2] << endl;
	this->currDisplayingBlockCenter.push_back(displayImgDim[0] / 2);
	this->currDisplayingBlockCenter.push_back(displayImgDim[1] / 2);
	this->currDisplayingBlockCenter.push_back(displayImgDim[2] / 2);
	int totalbyte = inputImg4DSimplePtr->getTotalBytes();
	unsigned char* img1Dptr = new unsigned char[displayImgDim[0] * displayImgDim[1] * displayImgDim[2]];
	memcpy(img1Dptr, inputImg4DSimplePtr->getRawData(), totalbyte);
	
	vector<vector<unsigned char>> imgSlices;
	ImgProcessor::imgStackSlicer(img1Dptr, imgSlices, displayImgDim);
	registeredImg inputRegisteredImg;
	inputRegisteredImg.imgAlias = "currBlockSlices"; // This is the original images.
	inputRegisteredImg.dims[0] = displayImgDim[0];
	inputRegisteredImg.dims[1] = displayImgDim[1];
	inputRegisteredImg.dims[2] = displayImgDim[2];
	delete[] img1Dptr;

	int sliceNum = 0;
	for (vector<vector<unsigned char>>::iterator sliceIt = imgSlices.begin(); sliceIt != imgSlices.end(); ++sliceIt)
	{
		++sliceNum;
		string sliceName;
		if (sliceNum / 10 == 0) sliceName = "000" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 100 == 0) sliceName = "00" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 1000 == 0) sliceName = "0" + to_string(sliceNum) + ".tif";
		else sliceName = to_string(sliceNum) + ".tif";

		unsigned char* slicePtr = new unsigned char[displayImgDim[0] * displayImgDim[1]];
		for (int i = 0; i < sliceIt->size(); ++i) slicePtr[i] = sliceIt->at(i);
		myImg1DPtr my1Dslice(new unsigned char[displayImgDim[0] * displayImgDim[1]]);
		memcpy(my1Dslice.get(), slicePtr, sliceIt->size());
		inputRegisteredImg.slicePtrs.insert({ sliceName, my1Dslice });
		delete[] slicePtr;

		sliceIt->clear();
	}
	imgSlices.clear();

	//cout << "NeuronStructExplorer's address in FragTraceManager:" << &fragTraceTreeManager << endl;
	//system("pause");
	this->fragTraceTreeGrowerPtr = new TreeGrower(&fragTraceTreeManager);
	this->fragTraceTreeTrimmerPtr = new TreeTrimmer(&fragTraceTreeManager);
	this->fragTraceImgManager.imgDatabase.clear();
	this->fragTraceImgManager.imgDatabase.insert({ inputRegisteredImg.imgAlias, inputRegisteredImg });
	this->fragTraceTreeManager.treeDataBase.clear();

	this->adaImgName.clear();
	this->histThreImgName.clear();
	cout << " -- Image slice preparation done." << endl;
	cout << "Image acquisition done. Start fragment tracing.." << endl;

	// ******* QProgressDialog is automatically in separate thread, otherwise, the dialog can NEVER be updated ******* //
	// NOTE: The parent widget it FragTraceManager, not FragTraceControlPanel. Thus FragTraceControlPanel is still frozen since FragTraceManager is not finished yet.
	//       FragTraceControlPanel and FragTraceManager are on the same thread.
	this->progressBarDiagPtr = new QProgressDialog(this); 
	this->progressBarDiagPtr->setWindowTitle("Neuron segment generation in progress");
	this->progressBarDiagPtr->setMinimumWidth(400);
	this->progressBarDiagPtr->setRange(0, 100);
	this->progressBarDiagPtr->setModal(true);
	this->progressBarDiagPtr->setCancelButtonText("Abort");
	// *************************************************************************************************************** //
}

void FragTraceManager::reinit(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices)
{
	this->finalSaveRootQ = "D:\\Work\\FragTrace";

	char* numProcsC;
	numProcsC = getenv("NUMBER_OF_PROCESSORS");
	string numProcsString(numProcsC);
	this->numProcs = stoi(numProcsString);

	this->mode = mode;
	this->partialVolumeTracing = false;

	int displayImgDim[3];
	displayImgDim[0] = inputImg4DSimplePtr->getXDim();
	displayImgDim[1] = inputImg4DSimplePtr->getYDim();
	displayImgDim[2] = inputImg4DSimplePtr->getZDim();
	cout << " -- Displaying image block dimensions: " << displayImgDim[0] << " " << displayImgDim[1] << " " << displayImgDim[2] << endl;
	this->currDisplayingBlockCenter.push_back(displayImgDim[0] / 2);
	this->currDisplayingBlockCenter.push_back(displayImgDim[1] / 2);
	this->currDisplayingBlockCenter.push_back(displayImgDim[2] / 2);
	int totalbyte = inputImg4DSimplePtr->getTotalBytes();
	unsigned char* img1Dptr = new unsigned char[displayImgDim[0] * displayImgDim[1] * displayImgDim[2]];
	memcpy(img1Dptr, inputImg4DSimplePtr->getRawData(), totalbyte);

	vector<vector<unsigned char>> imgSlices;
	ImgProcessor::imgStackSlicer(img1Dptr, imgSlices, displayImgDim);
	registeredImg inputRegisteredImg;
	inputRegisteredImg.imgAlias = "currBlockSlices"; // This is the original images.
	inputRegisteredImg.dims[0] = displayImgDim[0];
	inputRegisteredImg.dims[1] = displayImgDim[1];
	inputRegisteredImg.dims[2] = displayImgDim[2];
	delete[] img1Dptr;

	int sliceNum = 0;
	for (vector<vector<unsigned char>>::iterator sliceIt = imgSlices.begin(); sliceIt != imgSlices.end(); ++sliceIt)
	{
		++sliceNum;
		string sliceName;
		if (sliceNum / 10 == 0) sliceName = "000" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 100 == 0) sliceName = "00" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 1000 == 0) sliceName = "0" + to_string(sliceNum) + ".tif";
		else sliceName = to_string(sliceNum) + ".tif";

		unsigned char* slicePtr = new unsigned char[displayImgDim[0] * displayImgDim[1]];
		for (int i = 0; i < sliceIt->size(); ++i) slicePtr[i] = sliceIt->at(i);
		myImg1DPtr my1Dslice(new unsigned char[displayImgDim[0] * displayImgDim[1]]);
		memcpy(my1Dslice.get(), slicePtr, sliceIt->size());
		inputRegisteredImg.slicePtrs.insert({ sliceName, my1Dslice });
		delete[] slicePtr;

		sliceIt->clear();
	}
	imgSlices.clear();

	//cout << "NeuronStructExplorer's address in FragTraceManager:" << &fragTraceTreeManager << endl;
	//system("pause");
	//this->fragTraceTreeGrowerPtr = new TreeGrower(&fragTraceTreeManager);
	//this->fragTraceTreeTrimmerPtr = new TreeTrimmer(&fragTraceTreeManager);
	this->fragTraceImgManager.imgDatabase.clear();
	this->fragTraceImgManager.imgDatabase.insert({ inputRegisteredImg.imgAlias, inputRegisteredImg });
	this->fragTraceTreeManager.treeDataBase.clear();

	this->adaImgName.clear();
	this->histThreImgName.clear();
	cout << " -- Image slice preparation done." << endl << endl;
	cout << "Image acquisition done. Start fragment tracing..." << endl;

	// ******* QProgressDialog is automatically in separate thread, otherwise, the dialog can NEVER be updated ******* //
	// NOTE: The parent widget it FragTraceManager, not FragTraceControlPanel. Thus FragTraceControlPanel is still frozen since FragTraceManager is not finished yet.
	//       FragTraceControlPanel and FragTraceManager are on the same thread.
	this->progressBarDiagPtr = new QProgressDialog(this);
	this->progressBarDiagPtr->setWindowTitle("Neuron segment generation in progress");
	this->progressBarDiagPtr->setMinimumWidth(400);
	this->progressBarDiagPtr->setRange(0, 100);
	this->progressBarDiagPtr->setModal(true);
	this->progressBarDiagPtr->setCancelButtonText("Abort");
	// *************************************************************************************************************** //
}

FragTraceManager::~FragTraceManager()
{
	if (!this->segEndClusterChains.empty())
	{
		for (auto& chain : this->segEndClusterChains) integratedDataTypes::cleanUp_segEndClusterChain_downStream(chain.second);
		this->segEndClusterChains.clear();
	}
	
	if (fragTraceTreeGrowerPtr != nullptr)
	{
		delete fragTraceTreeGrowerPtr;
		fragTraceTreeGrowerPtr = nullptr;
	}

	if (fragTraceTreeTrimmerPtr != nullptr)
	{
		delete fragTraceTreeTrimmerPtr;
		fragTraceTreeTrimmerPtr = nullptr;
	}
}


// ***************** TRACING PROCESS CONTROLING FUNCTION ***************** //
bool FragTraceManager::imgProcPipe_wholeBlock()
{
	cout << "number of slices: " << this->fragTraceImgManager.imgDatabase.begin()->second.slicePtrs.size() << endl;
	V3DLONG dims[4];
	dims[0] = this->fragTraceImgManager.imgDatabase.begin()->second.dims[0];
	dims[1] = this->fragTraceImgManager.imgDatabase.begin()->second.dims[1];
	dims[2] = 1;
	dims[3] = 1;

	int imgDims[3];
	imgDims[0] = dims[0];
	imgDims[1] = dims[1];
	imgDims[2] = this->fragTraceImgManager.imgDatabase.begin()->second.slicePtrs.size();

	if (this->ada)
		this->myImgProcessor.adaThresholding("currBlockSlices", this->adaImgName, this->fragTraceImgManager.imgDatabase, this->simpleAdaStepsize, this->simpleAdaRate);

	if (this->cutoffIntensity != 0)
	{
		// The signal mask is generated here with adaptive threshodling followed by simple thresholding.
		// Original signal intensity is preserved and background is zero.
		this->myImgProcessor.simpleThre(this->adaImgName, "ada_cutoff", this->fragTraceImgManager.imgDatabase, this->cutoffIntensity);

		if (this->gammaCorrection)
		{
			this->myImgProcessor.gammaCorrect("ada_cutoff", "gammaCorrected", this->fragTraceImgManager.imgDatabase);
			this->myImgProcessor.histThreImg3D("gammaCorrected", this->histThreImgName, this->fragTraceImgManager.imgDatabase, this->stdFold); // still needs this with gamma?
			if (!this->mask2swc(this->histThreImgName, "blobTree")) return false;
		}
		else
		{
			if (this->mode == dendriticTree) // MOST DENDRITE GOES HERE
			{
				//clock_t begin = clock();
				if (!this->mask2swc("ada_cutoff", "blobTree")) return false;
				//clock_t end = clock();
				//float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
				//cout << "Connected component TIME ELAPSED: " << elapsed_secs << " SECS" << endl << endl;
			}
			else // MOST AXON GOES HERE
			{
				this->myImgProcessor.histThreImg3D("ada_cutoff", this->histThreImgName, this->fragTraceImgManager.imgDatabase, this->stdFold);
				if (!this->mask2swc(this->histThreImgName, "blobTree")) return false;
			}
		}
	}
	else
	{
		if (this->gammaCorrection)
		{
			this->myImgProcessor.gammaCorrect(this->adaImgName, "gammaCorrected", this->fragTraceImgManager.imgDatabase);
			this->myImgProcessor.histThreImg3D("gammaCorrected", this->histThreImgName, this->fragTraceImgManager.imgDatabase, this->stdFold);
			if (!this->mask2swc(this->histThreImgName, "blobTree")) return false;
		}
		else
		{
			this->myImgProcessor.histThreImg3D(this->adaImgName, this->histThreImgName, this->fragTraceImgManager.imgDatabase, this->stdFold);
			if (!this->mask2swc(this->histThreImgName, "blobTree")) return false;
		}
	}

	NeuronTree FINALOUTPUT_TREE, PRE_FINALOUTPUT_TREE, CONTINUOUS_AXON_PREFINAL_TREE;
	this->treeAssembly(PRE_FINALOUTPUT_TREE);
	
	this->myFragPostProcessor.scalingFactor = this->scalingFactor;
	this->myFragPostProcessor.imgOrigin = this->imgOrigin;
	this->myFragPostProcessor.volumeAdjustedBounds = this->volumeAdjustedBounds;
	if (this->partialVolumeTracing)
		PRE_FINALOUTPUT_TREE = NeuronStructUtil::swcShift(PRE_FINALOUTPUT_TREE, this->volumeAdjustedBounds[0] - 1, this->volumeAdjustedBounds[2] - 1, this->volumeAdjustedBounds[4] - 1);
	
	this->getExistingFinalTree(this->existingTree);
	if (this->continuousAxon)
	{
		NeuronTree scaledBackExistingTree = this->myFragPostProcessor.treeScaleBack(existingTree, this->scalingFactor, this->imgOrigin);
		NeuronTree newTreePart = TreeGrower::swcSamePartExclusion(PRE_FINALOUTPUT_TREE, scaledBackExistingTree, 4, 8);
		CONTINUOUS_AXON_PREFINAL_TREE = axonGrow(newTreePart, scaledBackExistingTree);
		CONTINUOUS_AXON_PREFINAL_TREE = NeuronStructUtil::singleDotRemove(CONTINUOUS_AXON_PREFINAL_TREE, this->minNodeNum);
		vector<NeuronTree> trees = { scaledBackExistingTree, CONTINUOUS_AXON_PREFINAL_TREE };
		FINALOUTPUT_TREE = this->myFragPostProcessor.scaleTree(NeuronStructUtil::swcCombine(trees), this->scalingFactor, this->imgOrigin);

		// ------- Debug ------- //
		if (FragTraceTester::isInstantiated())
		{
			QString prefixQ = this->finalSaveRootQ + "\\";
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::newTracedPart, newTreePart, prefixQ);
		}
		// --------------------- //
	}
	else
		FINALOUTPUT_TREE = this->myFragPostProcessor.integrateNewTree(this->existingTree, PRE_FINALOUTPUT_TREE, this->minNodeNum);

	QString localSWCFullName = this->finalSaveRootQ + "/currBlock.swc";
	//writeSWC_file(localSWCFullName, FINALOUTPUT_TREE);
	
	if (this->progressBarDiagPtr->isVisible()) this->progressBarDiagPtr->close();

	emit emitTracedTree(FINALOUTPUT_TREE);

	if (!this->segEndClusterChains.empty())
	{
		for (auto& chain : this->segEndClusterChains) integratedDataTypes::cleanUp_segEndClusterChain_downStream(chain.second);
		this->segEndClusterChains.clear();
	}
}

bool FragTraceManager::treeAssembly(NeuronTree& PRE_FINALOUTPUT_TREE)
{
	if (this->mode == axon)
	{
		// 1. Generated skeletons of segmented blobs.
		profiledTree objSkeletonProfiledTree;
		if (!this->generateTree(axon, objSkeletonProfiledTree)) return false;;

		// 2. Break all branches.
		NeuronTree MSTbranchBreakTree = TreeTrimmer::branchBreak(objSkeletonProfiledTree);
		profiledTree objBranchBreakTree(MSTbranchBreakTree);

		// 3. Downsample the node density to reduce segment zig-zagging.
		profiledTree downSampledProfiledTree = NeuronStructUtil::treeDownSample(objBranchBreakTree, 2);

		// 4. Iteratively connect segments within seg-end clusters.
		profiledTree iteredConnectedTree = this->segConnect_withinCurrBlock(downSampledProfiledTree, 10);

		// 5. Further smooth segments with their profiled morphology using length/distance ratio.
		this->fragTraceTreeManager.segMorphProfile(iteredConnectedTree, 3);
		profiledTree angleSmoothedTree = this->fragTraceTreeTrimmerPtr->itered_segSharpAngleSmooth_lengthDistRatio(iteredConnectedTree, 1.2);

		// 6. Eliminate small floating segments.
		if (this->minNodeNum > 0) angleSmoothedTree.tree = NeuronStructUtil::singleDotRemove(iteredConnectedTree.tree, this->minNodeNum);

		// 7. Iteratively connect segments within seg-end clusters.
		profiledTreeReInit(angleSmoothedTree);
		//profiledTree noDotProfiledTree(noDotTree);
		profiledTree iteredConnectedTree2 = this->segConnect_withinCurrBlock(angleSmoothedTree, 10);
		this->fragTraceTreeManager.treeDataBase.insert({ "tracedBlock_axon", iteredConnectedTree2 });

		// FINALIZED AXON TREE of current tracing block.
		PRE_FINALOUTPUT_TREE = iteredConnectedTree2.tree;

		// ------- Debug ------- //
		if (FragTraceTester::isInstantiated())
		{
			QString savingPrefixQ = this->finalSaveRootQ + "\\";
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::axonBranchBreak, objBranchBreakTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::axonDownSampled, downSampledProfiledTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::iteredConnected, iteredConnectedTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::angleSmooth_lengthDistRatio, angleSmoothedTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->axonTreeFormingInterResults(FragTraceTester::noFloatingTinyFrag, PRE_FINALOUTPUT_TREE, savingPrefixQ);
		}
		// --------------------- //
	}
	else if (this->mode == dendriticTree)
	{
		// 1. Generate raw dendritic tree.
		profiledTree profiledDenTree;
		if (!this->generateTree(dendriticTree, profiledDenTree)) return false;

		// 2. Prepare peripheral dendritic signal tree.
		NeuronTree periTree = this->getSmoothedPeriDenTree();

		// 3. Downsample the node density to reduce segment zig-zagging.
		profiledTree downSampledDenTree = NeuronStructUtil::treeDownSample(profiledDenTree, 2);

		// 4. 1st time removing floating tiny segments.
		NeuronTree floatingExcludedTree;
		if (this->minNodeNum > 0) floatingExcludedTree = NeuronStructUtil::singleDotRemove(downSampledDenTree, this->minNodeNum);
		else floatingExcludedTree = downSampledDenTree.tree;
		profiledTree dnSampledProfiledTree(floatingExcludedTree);

		// 5. Remove spikes
		profiledTree spikeRemovedProfiledTree = TreeTrimmer::itered_spikeRemoval(dnSampledProfiledTree, 2);

		// 6. Straighten up sharp angles caused by spikes.
		profiledTree profiledSpikeRootStraightTree = this->straightenSpikeRoots(spikeRemovedProfiledTree);

		// 7. Break branches.
		NeuronTree branchBrokenTree = TreeTrimmer::branchBreak(profiledSpikeRootStraightTree);

		// 8. Remove hooking and sharp-angled segment ends.
		float angleThre = (float(2) / float(3)) * PI_MK;
		profiledTree branchBrokenProfiledTree(branchBrokenTree);
		profiledTree hookRemovedProfiledTree = TreeTrimmer::itered_removeHookingHeadTail(branchBrokenProfiledTree, angleThre);

		// 9. Combine main dendritic tree with peripheral dendritic tree.
		vector<NeuronTree> allTrees;
		allTrees.push_back(hookRemovedProfiledTree.tree);
		allTrees.push_back(periTree);
		profiledTree completeProfiledTree(NeuronStructUtil::swcCombine(allTrees));

		// 10. Iteratively connect segments within seg-end clusters.
		profiledTree iteredConnectedProfiledTree = this->segConnect_withinCurrBlock(completeProfiledTree, 10);

		// 11. Smooth segments with their profiled morphology using length/distance ratio. 
		this->fragTraceTreeManager.segMorphProfile(iteredConnectedProfiledTree, 3);
		profiledTree angleSmoothedTree = this->fragTraceTreeTrimmerPtr->itered_segSharpAngleSmooth_lengthDistRatio(iteredConnectedProfiledTree, 1.2);

		// 12. Further smooth out sharp angles within each segment using 3 node window.
		profiledTree noJumpProfiledTree = this->fragTraceTreeTrimmerPtr->segSharpAngleSmooth_distThre_3nodes(angleSmoothedTree);

		// 13. 2nd time removing floating tiny segments
		NeuronTree smallSegsCleanedUpTree = NeuronStructUtil::singleDotRemove(noJumpProfiledTree, this->minNodeNum);
		profiledTree cleanedUpProfiledTree(smallSegsCleanedUpTree);
		this->fragTraceTreeManager.treeDataBase.insert({ "tracedBlock_dendrite", cleanedUpProfiledTree });

		// FINALIZED DENDRITIC TREE.
		PRE_FINALOUTPUT_TREE = smallSegsCleanedUpTree;

		// ------- Debug ------- //
		if (FragTraceTester::isInstantiated())
		{
			QString savingPrefixQ = this->finalSaveRootQ + "\\";
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denRaw, profiledDenTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denDnSampledRaw, downSampledDenTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::noFloatingTinyFrag, dnSampledProfiledTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::spikeRemove, spikeRemovedProfiledTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::spikeRootStraighten, profiledSpikeRootStraightTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::hookRemove, hookRemovedProfiledTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::branchBreak, branchBrokenTree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::iteredConnected, iteredConnectedProfiledTree.tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::angleSmooth_lengthDistRatio, angleSmoothedTree.tree, savingPrefixQ);
		}
		// --------------------- //
	}
}

NeuronTree FragTraceManager::axonGrow(const NeuronTree& inputTree, const NeuronTree& scaledExistingTree)
{
	set<vector<float>> probes = this->myFragPostProcessor.getProbesFromLabeledExistingSegs(scaledExistingTree);
	set<vector<float>> probes_marker = this->getAxonMarkerProbes();
	probes.insert(probes_marker.begin(), probes_marker.end());
	for (auto probe : probes)
		cout << "(" << probe.at(0) << ", " << probe.at(1) << ", " << probe.at(2) << ") ";
	cout << endl;

	NeuronTree outputTree;
	profiledTree inputProfiledTree(inputTree);
	this->seedCluster = this->fragTraceTreeManager.segEndClusterProbe(inputProfiledTree, probes, this->axonMarkerAllowance);
	if (this->seedCluster.empty())
	{
		cerr << "No seed cluster identified." << endl;
		return outputTree;
	}
	cout << " -- SEED CLUSTERS: ";
	for (auto id : this->seedCluster) cout << id << " ";
	cout << endl;

	this->myFragPostProcessor.getClusterChain(inputProfiledTree, this->seedCluster, this->segEndClusterChains);
	outputTree = this->myFragPostProcessor.getTreeFromClusterChains(this->segEndClusterChains, inputProfiledTree);

	//if (FragTraceTester::isInstantiated())
	//	FragTraceTester::getInstance()->clusterSegEndMarkersGen_axonChain(this->segEndClusterChains, inputProfiledTree);
	
	return outputTree;
}
// *********************************************************************** //


/*************************** Image Segmentation ***************************/
bool FragTraceManager::mask2swc(const string inputImgName, string outputTreeName)
{
	int sliceDims[3];
	sliceDims[0] = this->fragTraceImgManager.imgDatabase.at(inputImgName).dims[0];
	sliceDims[1] = this->fragTraceImgManager.imgDatabase.at(inputImgName).dims[1];
	sliceDims[2] = 1;

	vector<unsigned char**> slice2DVector;
	unsigned char* mipPtr = new unsigned char[sliceDims[0] * sliceDims[1]];
	for (int i = 0; i < sliceDims[0] * sliceDims[1]; ++i) mipPtr[i] = 0;
	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputImgName).slicePtrs.begin(); 
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputImgName).slicePtrs.end(); ++sliceIt)
	{
		ImgProcessor::imgMax(sliceIt->second.get(), mipPtr, mipPtr, sliceDims);

		unsigned char** slice2DPtr = new unsigned char*[sliceDims[1]];
		for (int j = 0; j < sliceDims[1]; ++j)
		{
			slice2DPtr[j] = new unsigned char[sliceDims[0]];
			for (int i = 0; i < sliceDims[0]; ++i) slice2DPtr[j][i] = sliceIt->second.get()[sliceDims[0] * j + i];
		}
		slice2DVector.push_back(slice2DPtr);
	}
	
	this->signalBlobs.clear();
	this->signalBlobs = this->fragTraceImgAnalyzer.findSignalBlobs(slice2DVector, sliceDims, 3, mipPtr);

	NeuronTree blob3Dtree = NeuronStructUtil::blobs2tree(this->signalBlobs, true);
	
	QString blobTreeFullFilenameQ = this->finalSaveRootQ + "\\blob.swc";
	//writeSWC_file(blobTreeFullFilenameQ, blob3Dtree);
	
	profiledTree profiledSigTree(blob3Dtree);
	this->fragTraceTreeManager.treeDataBase.insert({ outputTreeName, profiledSigTree });

	// ----------- Releasing memory ------------ //
	delete[] mipPtr;
	mipPtr = nullptr;
	for (vector<unsigned char**>::iterator slice2DPtrIt = slice2DVector.begin(); slice2DPtrIt != slice2DVector.end(); ++slice2DPtrIt)
	{
		for (int yi = 0; yi < sliceDims[1]; ++yi)
		{
			delete[] (*slice2DPtrIt)[yi];
			(*slice2DPtrIt)[yi] = nullptr;
		}
		delete[] *slice2DPtrIt;
		*slice2DPtrIt = nullptr;
	}
	slice2DVector.clear();
	// ------- END of [Releasing memory] ------- //
}

// ----------------- Object Classification ----------------- //
void FragTraceManager::smallBlobRemoval(vector<connectedComponent>& signalBlobs, const int sizeThre)
{
	if (signalBlobs.empty())
	{
		cerr << "No signal blobs data exists. Do nothing and return." << endl;
		return;
	}

	cout << " ==> removing small blob, size threshold = " << sizeThre << endl;

	vector<ptrdiff_t> delLocs;
	for (vector<connectedComponent>::iterator compIt = signalBlobs.begin(); compIt != signalBlobs.end(); ++compIt)
		if (compIt->size <= sizeThre) delLocs.push_back(compIt - signalBlobs.begin());

	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt)
		signalBlobs.erase(signalBlobs.begin() + *delIt);
}
// ------------ END of [Object Classification] ------------- //
/*********************** END of [Image Segmentation] **********************/


/*************************** Final Traced Tree Generation ***************************/
profiledTree FragTraceManager::straightenSpikeRoots(const profiledTree& inputProfiledTree, double angleThre)
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

bool FragTraceManager::generateTree(workMode mode, profiledTree& objSkeletonProfiledTree)
{
	cout << endl << "Finishing up processing objects.." << endl;

	if (!this->progressBarDiagPtr->isVisible()) this->progressBarDiagPtr->show();
	this->progressBarDiagPtr->setLabelText("Extracting fragments from 3D signal objects..");

	if (mode == axon)
	{		
		vector<NeuronTree> objTrees;
		NeuronTree finalCentroidTree;
		for (vector<connectedComponent>::iterator it = this->signalBlobs.begin(); it != this->signalBlobs.end(); ++it)
		{
			qApp->processEvents();
			if (this->progressBarDiagPtr->wasCanceled())
			{
				this->progressBarDiagPtr->setLabelText("Process aborted.");
				return false;
			}

			if (it->size < voxelCount) continue;
			if (int(it - this->signalBlobs.begin()) % 500 == 0)
			{
				double progressBarValue = (double(it - this->signalBlobs.begin()) / this->signalBlobs.size()) * 100;
				int progressBarValueInt = ceil(progressBarValue);
				this->progressBarDiagPtr->setValue(progressBarValueInt);
				cout << int(it - this->signalBlobs.begin()) << " ";
			}

			NeuronTree centroidTree;
			boost::container::flat_set<deque<float>> sectionalCentroids = this->fragTraceImgAnalyzer.getSectionalCentroids(*it);
			for (boost::container::flat_set<deque<float>>::iterator nodeIt = sectionalCentroids.begin(); nodeIt != sectionalCentroids.end(); ++nodeIt)
			{
				NeuronSWC newNode;
				newNode.x = nodeIt->at(0);
				newNode.y = nodeIt->at(1);
				newNode.z = nodeIt->at(2);
				newNode.type = 2;
				newNode.parent = -1;
				centroidTree.listNeuron.push_back(newNode);
			}
			finalCentroidTree.listNeuron.append(centroidTree.listNeuron);

			NeuronTree MSTtree = TreeGrower::SWC2MSTtree_boost(centroidTree);
			profiledTree profiledMSTtree(MSTtree);				
			//profiledTree smoothedTree = NeuronStructExplorer::spikeRemove(profiledMSTtree); -> This can cause error and terminate the program. Need to investigate the implementation.
			objTrees.push_back(profiledMSTtree.tree);
		}
		cout << endl;

		NeuronTree objSkeletonTree = NeuronStructUtil::swcCombine(objTrees);
		for (QList<NeuronSWC>::iterator nodeIt = objSkeletonTree.listNeuron.begin(); nodeIt != objSkeletonTree.listNeuron.end(); ++nodeIt)
			nodeIt->type = 16;
		profiledTree outputProfiledTree(objSkeletonTree);
		objSkeletonProfiledTree = outputProfiledTree;

		// ------- Debug ------- //
		if (FragTraceTester::isInstantiated())
		{
			QString savingPrefixQ = this->finalSaveRootQ + "\\";
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::axonCentroid, finalCentroidTree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::axonSkeleton, outputProfiledTree.tree, savingPrefixQ);
		}
		// --------------------- //
		
		return true;
	}
	// ************************************************************************************************* //
	else if (mode == dendriticTree)
	{	
		this->peripheralSignalBlobMap.clear();
		if (this->selectedSomaMap.empty())
		{
			connectedComponent dendriteComponent = *this->signalBlobs.begin();
			for (vector<connectedComponent>::iterator compIt = this->signalBlobs.begin() + 1; compIt != this->signalBlobs.end(); ++compIt)
				if (compIt->size > dendriteComponent.size) dendriteComponent = *compIt;
			
			vector<connectedComponent> denComp = { dendriteComponent };
			NeuronTree denBlobTree = NeuronStructUtil::blobs2tree(denComp);
	
			// ------- Debug ------- //
			if (FragTraceTester::isInstantiated())
			{
				QString savingPrefixQ = this->finalSaveRootQ + "\\";
				//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denBlob, denBlobTree, savingPrefixQ);
			}
			// --------------------- //

			vector<int> origin = this->currDisplayingBlockCenter;
			NeuronGeoGrapher::nodeList2polarNodeList(denBlobTree.listNeuron, this->fragTraceTreeGrowerPtr->polarNodeList, origin);  // Converts NeuronSWC list to polarNeuronSWC list.
			this->fragTraceTreeGrowerPtr->radiusShellMap_loc = NeuronGeoGrapher::getShellByRadius_loc(this->fragTraceTreeGrowerPtr->polarNodeList);
			profiledTree dendriticProfiledTree(this->fragTraceTreeGrowerPtr->dendriticTree_shellCentroid()); // Dendritic tree is generated here.
			if (dendriticProfiledTree.tree.listNeuron.empty())
			{
				cerr << "No connected components captured. Do nothing and return." << endl;
				return false;
			}
			for (QList<NeuronSWC>::iterator nodeIt = dendriticProfiledTree.tree.listNeuron.begin(); nodeIt != dendriticProfiledTree.tree.listNeuron.end(); ++nodeIt)
				nodeIt->type = 16;
			objSkeletonProfiledTree = dendriticProfiledTree.tree;

			// Get peripheral signal blobs
			NeuronTree periSignalTree = this->getPeripheralSigTree(dendriticProfiledTree.tree, this->minNodeNum);
			vector<connectedComponent> periBlobs = this->getPeripheralBlobs(periSignalTree, origin);
			this->peripheralSignalBlobMap.insert({ "dendriticTree", periBlobs });
			
			// ------- Debug ------- //
			if (FragTraceTester::isInstantiated())
			{
				NeuronTree periBlobTree = NeuronStructUtil::blobs2tree(periBlobs);
				QString savingPrefixQ = this->finalSaveRootQ + "\\";
				//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denPeriBlob, periBlobTree, savingPrefixQ);
			}
			// --------------------- //
		}
		else
		{
			map<string, profiledTree> dendriticTreesMap;
			set<connectedComponent> processedConnComp;
			int compLoc = -1;
			for (map<int, ImageMarker>::iterator it = this->selectedLocalSomaMap.begin(); it != this->selectedLocalSomaMap.end(); ++it)
			{
				vector<int> origin(3);
				origin[0] = int(it->second.x) - this->partialVolumeLowerBoundaries[0];
				origin[1] = int(it->second.y) - this->partialVolumeLowerBoundaries[1];
				origin[2] = int(it->second.z) - this->partialVolumeLowerBoundaries[2];
				//cout << origin[0] << " " << origin[1] << " " << origin[2] << endl;
				//cout << this->partialVolumeLowerBoundaries[0] << " " << this->partialVolumeLowerBoundaries[1] << " " << this->partialVolumeLowerBoundaries[2] << endl;
				//system("pause");

				bool repeatedBlob = false;
				connectedComponent dendriteComponent;
				for (vector<connectedComponent>::iterator compIt = this->signalBlobs.begin(); compIt != this->signalBlobs.end(); ++compIt)
				{
					if (compIt->coordSets.find(origin.at(2)) != compIt->coordSets.end())
					{
						if (compIt->coordSets.at(origin.at(2)).find(origin) != compIt->coordSets.at(origin.at(2)).end())
						{
							if (int(compIt - this->signalBlobs.begin()) == compLoc)
							{
								repeatedBlob = true;
								break;
							}
							else
							{
								dendriteComponent = *compIt;
								compLoc = int(compIt - this->signalBlobs.begin());
								break;
							}
						}
					}
				}
				if (repeatedBlob) continue;
				
				vector<connectedComponent> denComp = { dendriteComponent };
				NeuronTree denBlobTree = NeuronStructUtil::blobs2tree(denComp);
				
				if (FragTraceTester::isInstantiated())
				{
					QString savingPrefixQ = this->finalSaveRootQ + "\\soma" + QString::number(it->first) + "_";
					//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denBlob, denBlobTree, savingPrefixQ);
				}

				NeuronGeoGrapher::nodeList2polarNodeList(denBlobTree.listNeuron, this->fragTraceTreeGrowerPtr->polarNodeList, origin);
				this->fragTraceTreeGrowerPtr->radiusShellMap_loc.clear();
				this->fragTraceTreeGrowerPtr->radiusShellMap_loc = NeuronGeoGrapher::getShellByRadius_loc(this->fragTraceTreeGrowerPtr->polarNodeList);

				string treeName = "dendriticTree" + to_string(dendriticTreesMap.size() + 1);
				profiledTree dendriticProfiledTree(this->fragTraceTreeGrowerPtr->dendriticTree_shellCentroid());
				if (dendriticProfiledTree.tree.listNeuron.empty())
				{
					cerr << "No connected components captured. Do nothing and return." << endl;
					return false;
				}
				for (QList<NeuronSWC>::iterator nodeIt = dendriticProfiledTree.tree.listNeuron.begin(); nodeIt != dendriticProfiledTree.tree.listNeuron.end(); ++nodeIt)
					nodeIt->type = 16 + dendriticTreesMap.size();
				dendriticTreesMap.insert({ treeName, dendriticProfiledTree });

				// Get peripheral signal blobs
				NeuronTree periSignalTree = this->getPeripheralSigTree(dendriticProfiledTree.tree, this->minNodeNum);
				vector<connectedComponent> periBlobs = this->getPeripheralBlobs(periSignalTree, origin);
				this->peripheralSignalBlobMap.insert({ treeName, periBlobs });
				
				// ------- Debug ------- //
				if (FragTraceTester::isInstantiated())
				{
					NeuronTree periBlobTree = NeuronStructUtil::blobs2tree(periBlobs);
					QString savingPrefixQ = this->finalSaveRootQ + "\\soma" + QString::number(it->first) + "_";
					//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denPeriBlob, periBlobTree, savingPrefixQ);
				}
				// --------------------- //
			}

			profiledTree combinedProfiledDenTree(NeuronStructUtil::swcCombine(dendriticTreesMap));
			objSkeletonProfiledTree = combinedProfiledDenTree;
		}
		
		return true;
	}
}

map<string, profiledTree> FragTraceManager::generatePeriRawDenTree(const map<string, vector<connectedComponent>>& periSigBlobMap)
{
	map<string, profiledTree> outputPeriRawDenTreeMap;
	NeuronTree finalPeriCentroidTree;
	vector<NeuronTree> objTrees;
	int somaCount = 0;
	for (map<string, vector<connectedComponent>>::const_iterator periBlobIt = periSigBlobMap.begin(); periBlobIt != periSigBlobMap.end(); ++periBlobIt)
	{
		objTrees.clear();
		for (vector<connectedComponent>::const_iterator compIt = periBlobIt->second.begin(); compIt != periBlobIt->second.end(); ++compIt)
		{
			if (compIt->size < this->voxelCount) continue;

			NeuronTree centroidTree;
			boost::container::flat_set<deque<float>> sectionalCentroids = this->fragTraceImgAnalyzer.getSectionalCentroids(*compIt);
			for (boost::container::flat_set<deque<float>>::iterator nodeIt = sectionalCentroids.begin(); nodeIt != sectionalCentroids.end(); ++nodeIt)
			{
				NeuronSWC newNode;
				newNode.x = nodeIt->at(0);
				newNode.y = nodeIt->at(1);
				newNode.z = nodeIt->at(2);
				newNode.type = 3;
				newNode.parent = -1;
				centroidTree.listNeuron.push_back(newNode);
			}
			finalPeriCentroidTree.listNeuron.append(centroidTree.listNeuron);

			NeuronTree MSTtree = TreeGrower::SWC2MSTtree_boost(centroidTree);
			profiledTree profiledMSTtree(MSTtree);
			objTrees.push_back(profiledMSTtree.tree);
		}
		NeuronTree thisPeriSkeleton = NeuronStructUtil::swcCombine(objTrees);

		for (QList<NeuronSWC>::iterator nodeIt = thisPeriSkeleton.listNeuron.begin(); nodeIt != thisPeriSkeleton.listNeuron.end(); ++nodeIt)
			nodeIt->type = 16 + this->peripheralSignalBlobMap.size() - 1;
		profiledTree thisPeriProfiledTree(thisPeriSkeleton);
		string thisPeriSkeletonName = "periTree" + to_string(somaCount);
		outputPeriRawDenTreeMap.insert({ thisPeriSkeletonName, thisPeriProfiledTree });
		++somaCount;

		// ------- Debug ------- //
		if (FragTraceTester::isInstantiated())
		{
			QString savingPrefixQ = this->finalSaveRootQ + "\\soma" + QString::number(somaCount) + "_";
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::denSkeleton, thisPeriProfiledTree.tree, savingPrefixQ);
		}
		// --------------------- //
	}

	return outputPeriRawDenTreeMap;
}

vector<connectedComponent> FragTraceManager::getPeripheralBlobs(const NeuronTree& inputNeuronTree, const vector<int> origin)
{
	vector<connectedComponent> compList, peripheralComponents;
	map<double, set<ptrdiff_t>> pickedComponentsMap;

	for (boost::container::flat_map<double, vector<connectedComponent>>::iterator it = this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.begin(); it != this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.end(); ++it)
	{
		set<ptrdiff_t> newSet;
		pickedComponentsMap.insert(pair<double, set<ptrdiff_t>>(it->first, newSet));
	}

	for (QList<NeuronSWC>::const_iterator nodeIt = inputNeuronTree.listNeuron.begin(); nodeIt != inputNeuronTree.listNeuron.end(); ++nodeIt)
	{
		float dist = sqrtf((nodeIt->x - float(origin.at(0))) * (nodeIt->x - float(origin.at(0))) +
						   (nodeIt->y - float(origin.at(1))) * (nodeIt->y - float(origin.at(1))) +
						   (nodeIt->z - float(origin.at(2))) * (nodeIt->z - float(origin.at(2))));
		dist = round(dist);
		if (dist <= 10) continue;

		if (this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.find(dist) != this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.end())
		{
			for (vector<connectedComponent>::iterator compIt = this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.at(dist).begin(); compIt != this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.at(dist).end(); ++compIt)
			{
				if (pickedComponentsMap.at(dist).find(compIt - this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.at(dist).begin()) != pickedComponentsMap.at(dist).end())
					continue;

				if (nodeIt->x == compIt->ChebyshevCenter[0] && nodeIt->y == compIt->ChebyshevCenter[1] && nodeIt->z == compIt->ChebyshevCenter[2])
				{
					compList.push_back(*compIt);
					pickedComponentsMap.at(dist).insert(compIt - this->fragTraceTreeGrowerPtr->radius2shellConnCompMap.at(dist).begin());
					goto COMPONENT_PICKED;
				}
			}
		}

	COMPONENT_PICKED:
		continue;
	}

	// ------- regroup connected components -------
	NeuronTree blobTree = NeuronStructUtil::blobs2tree(compList);
	peripheralComponents = NeuronStructUtil::swc2signal3DBlobs(blobTree);

	return peripheralComponents;
}

NeuronTree FragTraceManager::getSmoothedPeriDenTree()
{
	map<string, profiledTree> periRawDenTreeMap = this->generatePeriRawDenTree(this->peripheralSignalBlobMap);

	map<string, profiledTree> periMSTtreeMap, periDnSampledTreeMap, periIteredconnectedTreeMap, periNoHookTreeMap, periNoJumpSmoothedTreeMap;
	int periCount = 0;
	for (map<string, profiledTree>::iterator it = periRawDenTreeMap.begin(); it != periRawDenTreeMap.end(); ++it)
	{
		profiledTree periMSTprofiledTree(TreeTrimmer::branchBreak(it->second));
		periMSTtreeMap.insert({ it->first, periMSTprofiledTree });

		profiledTree periDnSampledProfiledTree = NeuronStructUtil::treeDownSample(periMSTprofiledTree, 3);
		periDnSampledTreeMap.insert({ it->first, periDnSampledProfiledTree });

		profiledTree iteredConnectedProfiledTree = this->segConnect_withinCurrBlock(periDnSampledProfiledTree, 3);
		profiledTree noDotProfiledTree(NeuronStructUtil::singleDotRemove(iteredConnectedProfiledTree.tree, this->minNodeNum));
		periIteredconnectedTreeMap.insert({ it->first, noDotProfiledTree });

		float angleThre = (float(2) / float(3)) * PI_MK;
		profiledTree noHookProfiledTree = TreeTrimmer::itered_removeHookingHeadTail(noDotProfiledTree, angleThre);
		periNoHookTreeMap.insert({ it->first, noHookProfiledTree });

		this->fragTraceTreeManager.segMorphProfile(noHookProfiledTree, 3);
		profiledTree angleSmoothedTree = this->fragTraceTreeTrimmerPtr->itered_segSharpAngleSmooth_lengthDistRatio(noHookProfiledTree, 1.2);
		profiledTree noJumpProfiledTree = this->fragTraceTreeTrimmerPtr->segSharpAngleSmooth_distThre_3nodes(angleSmoothedTree);
		periNoJumpSmoothedTreeMap.insert({ it->first, noJumpProfiledTree });

		for (QList<NeuronSWC>::iterator nodeIt = periNoJumpSmoothedTreeMap.at(it->first).tree.listNeuron.begin(); nodeIt != periNoJumpSmoothedTreeMap.at(it->first).tree.listNeuron.end(); ++nodeIt)
			nodeIt->type = 16 + periCount;
		++periCount;
	}
	NeuronTree periTree = NeuronStructUtil::swcCombine(periNoJumpSmoothedTreeMap);

	// ------- Debug ------- //
	if (FragTraceTester::isInstantiated())
	{
		int periCount = 0;
		for (map<string, profiledTree>::iterator it = periMSTtreeMap.begin(); it != periMSTtreeMap.end(); ++it)
		{
			QString savingPrefixQ = this->finalSaveRootQ + "\\soma" + QString::number(periCount) + "_";
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::branchBreak, periMSTtreeMap.at(it->first).tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::downSampled, periDnSampledTreeMap.at(it->first).tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::iteredConnected, periIteredconnectedTreeMap.at(it->first).tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::hookRemove, periNoHookTreeMap.at(it->first).tree, savingPrefixQ);
			//FragTraceTester::getInstance()->denTreeFormingInterResults(FragTraceTester::angleSmooth_distThre_3nodes, periNoJumpSmoothedTreeMap.at(it->first).tree, savingPrefixQ);
			++periCount;
		}
	}
	// --------------------- //

	return periTree;
}

profiledTree FragTraceManager::segConnect_withinCurrBlock(const profiledTree& inputProfiledTree, float distThreshold)
{
	profiledTree tmpTree = inputProfiledTree; 
	profiledTree outputProfiledTree = this->fragTraceTreeGrowerPtr->itered_connectSegsWithinClusters(tmpTree, distThreshold);

	bool typeAssigned = false;
	int assignedType;
	cout << "  looking through existing segments";
	int segCount = 0;
	for (map<int, segUnit>::iterator segIt = outputProfiledTree.segs.begin(); segIt != outputProfiledTree.segs.end(); ++segIt)
	{
		++segCount;
		if (segCount % 50 == 0) cout << ".";

		for (QList<NeuronSWC>::iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			if (nodeIt->type != 16)
			{
				typeAssigned = true;
				assignedType = nodeIt->type;
				break;
			}
		}

		if (typeAssigned)
		{
			for (QList<NeuronSWC>::iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
			{
				nodeIt->type = assignedType;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->n)].type = assignedType;
			}

			typeAssigned = false;
			assignedType = 16;
		}
	}
	cout << endl;

	bool duplicatedRemove = true;
	while (duplicatedRemove)
	{
		for (QList<NeuronSWC>::iterator it = outputProfiledTree.tree.listNeuron.begin(); it != outputProfiledTree.tree.listNeuron.end(); ++it)
		{
			if (it->n == it->parent)
			{
				outputProfiledTree.tree.listNeuron.removeAt(int(it - outputProfiledTree.tree.listNeuron.begin()));
				goto DUPLICATE_REMOVED;
			}
		}
		break;

	DUPLICATE_REMOVED:
		continue;
	}
	
	// ------- Debug ------- //
	if (FragTraceTester::isInstantiated())
	{
		QString savingPrefixQ = this->finalSaveRootQ + "\\combinedTree.swc";
		//FragTraceTester::getInstance()->generalTreeFormingInterResults(FragTraceTester::combine, outputProfiledTree.tree, savingPrefixQ);
	}
	// --------------------- //

	return outputProfiledTree;
}

NeuronTree FragTraceManager::getPeripheralSigTree(const profiledTree& inputProfiledTree, int lengthThreshold) const
{
	NeuronTree outputTree;
	for (map<int, segUnit>::const_iterator it = inputProfiledTree.segs.begin(); it != inputProfiledTree.segs.end(); ++it)
	{
		if (it->second.nodes.size() > 5) continue;
		else
		{
			for (QList<NeuronSWC>::const_iterator nodeIt = it->second.nodes.begin(); nodeIt != it->second.nodes.end(); ++nodeIt)
			{
				NeuronSWC newNode = *nodeIt;
				newNode.parent = -1;
				outputTree.listNeuron.push_back(newNode);
			}
		}
	}

	return outputTree;
}
/********************** END of [Final Traced Tree Generation] ***********************/