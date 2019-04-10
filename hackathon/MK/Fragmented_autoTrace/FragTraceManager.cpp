#include <deque>
#include <omp.h>
#include <cstdlib>

#include <boost/container/flat_set.hpp>

#include "FragTraceManager.h"
#include "FeatureExtractor.h"

FragTraceManager::FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, workMode mode, bool slices)
{
	char* numProcsC;
	numProcsC = getenv("NUMBER_OF_PROCESSORS");
	string numProcsString(numProcsC);
	this->numProcs = stoi(numProcsString);

	this->mode = mode;

	int dims[3];
	dims[0] = inputImg4DSimplePtr->getXDim();
	dims[1] = inputImg4DSimplePtr->getYDim();
	dims[2] = inputImg4DSimplePtr->getZDim();
	cout << " -- Current image block dimensions: " << dims[0] << " " << dims[1] << " " << dims[2] << endl;
	int totalbyte = inputImg4DSimplePtr->getTotalBytes();
	unsigned char* img1Dptr = new unsigned char[dims[0] * dims[1] * dims[2]];
	memcpy(img1Dptr, inputImg4DSimplePtr->getRawData(), totalbyte);
		
	if (mode == dendriticTree)
	{
		unsigned char* dendrite1Dptr = new unsigned char[(dims[0] / 2) * (dims[1] / 2) * dims[2]];
		int downFacs[3];
		downFacs[0] = 2;
		downFacs[1] = 2;
		downFacs[2] = 1;
		ImgProcessor::imgDownSampleMax(img1Dptr, dendrite1Dptr, dims, downFacs);
		delete[] img1Dptr;
		img1Dptr = dendrite1Dptr;
		dendrite1Dptr = nullptr;

		dims[0] = dims[0] / 2;
		dims[1] = dims[1] / 2;
	}
	
	this->imgSlices.clear();
	ImgProcessor::imgStackSlicer(img1Dptr, this->imgSlices, dims);
	registeredImg inputRegisteredImg;
	inputRegisteredImg.imgAlias = "currBlockSlices";
	inputRegisteredImg.dims[0] = dims[0];
	inputRegisteredImg.dims[1] = dims[1];
	inputRegisteredImg.dims[2] = dims[2];
	delete[] img1Dptr;

	int sliceNum = 0;
	for (vector<vector<unsigned char>>::iterator sliceIt = this->imgSlices.begin(); sliceIt != this->imgSlices.end(); ++sliceIt)
	{
		++sliceNum;
		string sliceName;
		if (sliceNum / 10 == 0) sliceName = "000" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 100 == 0) sliceName = "00" + to_string(sliceNum) + ".tif";
		else if (sliceNum / 1000 == 0) sliceName = "0" + to_string(sliceNum) + ".tif";
		else sliceName = to_string(sliceNum) + ".tif";

		unsigned char* slicePtr = new unsigned char[dims[0] * dims[1]];
		for (int i = 0; i < sliceIt->size(); ++i) slicePtr[i] = sliceIt->at(i);
		myImg1DPtr my1Dslice(new unsigned char[dims[0] * dims[1]]);
		memcpy(my1Dslice.get(), slicePtr, sliceIt->size());
		inputRegisteredImg.slicePtrs.insert({ sliceName, my1Dslice });
		delete[] slicePtr;
	}

	this->fragTraceImgManager.imgDatabase.clear();
	this->fragTraceImgManager.imgDatabase.insert({ inputRegisteredImg.imgAlias, inputRegisteredImg });

	this->adaImgName.clear();
	this->histThreImgName.clear();
	cout << " -- Image slice preparation done." << endl;
	cout << "Image acquisition done. Start fragment tracing.." << endl;
}

void FragTraceManager::imgProcPipe_wholeBlock()
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

	if (this->ada) this->adaThre("currBlockSlices", dims, this->adaImgName);
	
	if (this->cutoffIntensity != 0)
	{
		this->simpleThre(this->adaImgName, dims, "ada_cutoff");
		if (this->gammaCorrection)
		{
			this->gammaCorrect("ada_cutoff", dims, "gammaCorrected");
			this->histThreImg3D("gammaCorrected", dims, this->histThreImgName);
			this->mask2swc(this->histThreImgName, "blobTree");
		}
		else 
		{
			if (this->mode == dendriticTree)
			{
				this->mask2swc("ada_cutoff", "blobTree");
			}
			else
			{
				this->histThreImg3D("ada_cutoff", dims, this->histThreImgName);
				this->mask2swc(this->histThreImgName, "blobTree");
			}
		}
	}
	else
	{
		if (this->gammaCorrection)
		{
			this->gammaCorrect(this->adaImgName, dims, "gammaCorrected");
			this->histThreImg3D("gammaCorrected", dims, this->histThreImgName);
			this->mask2swc(this->histThreImgName, "blobTree");
		}
		else
		{
			this->histThreImg3D(this->adaImgName, dims, this->histThreImgName);
			this->mask2swc(this->histThreImgName, "blobTree");
		}
	}

	NeuronTree denScaleBackTree, finalOutputTree;
	if (this->mode == wholeBlock_axon)
	{
		cout << endl << "Finishing up processing objects.." << endl << " number of objects processed: ";
		vector<NeuronTree> objTrees;
		NeuronTree objSkeletonTree;
		NeuronTree finalCentroidTree;

// ------- using omp to speed up skeletonization process ------- //
#pragma omp parallel num_threads(this->numProcs)
		{
			for (vector<connectedComponent>::iterator it = this->signalBlobs.begin(); it != this->signalBlobs.end(); ++it)
			{
				if (it->size < voxelCount) continue;

				if (int(it - this->signalBlobs.begin()) % 500 == 0) cout << int(it - this->signalBlobs.begin()) << " ";
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

				NeuronTree MSTtree = this->fragTraceTreeManager.SWC2MSTtree(centroidTree);
				profiledTree profiledMSTtree(MSTtree);
				//profiledTree smoothedTree = NeuronStructExplorer::spikeRemove(profiledMSTtree);
				objTrees.push_back(profiledMSTtree.tree);
			}
		}
// ------------------------------------------------------------- //

		QString finalCentroidTreeNameQ = this->finalSaveRootQ + "/finalCentroidTree.swc";
		writeSWC_file(finalCentroidTreeNameQ, finalCentroidTree);
		cout << endl;

		objSkeletonTree = NeuronStructUtil::swcCombine(objTrees);
		profiledTree objSkeletonProfiledTree(objSkeletonTree);
		this->fragTraceTreeManager.treeDataBase.insert({ "objSkeleton", objSkeletonProfiledTree });
		QString skeletonTreeNameQ = this->finalSaveRootQ + "/skeletonTree.swc";
		writeSWC_file(skeletonTreeNameQ, objSkeletonTree);

		NeuronTree MSTbranchBreakTree;
		MSTbranchBreakTree = NeuronStructExplorer::MSTbranchBreak(objSkeletonProfiledTree);
		profiledTree objBranchBreakTree(MSTbranchBreakTree);
		this->fragTraceTreeManager.treeDataBase.insert({ "objBranchBreakTree", objBranchBreakTree });
		QString branchBreakTreeName = this->finalSaveRootQ + "/branchBreakTree.swc";
		//writeSWC_file(branchBreakTreeName, objBranchBreakTree.tree);

		profiledTree downSampledProfiledTree = this->fragTraceTreeManager.treeDownSample(objBranchBreakTree, 2);
		QString downSampledTreeName = this->finalSaveRootQ + "/downSampledTreeTest.swc";
		writeSWC_file(downSampledTreeName, downSampledProfiledTree.tree);

		profiledTree newIteredConnectedTree = this->fragTraceTreeManager.itered_connectLongNeurite(downSampledProfiledTree, 5);

		if (this->minNodeNum > 0) finalOutputTree = NeuronStructExplorer::singleDotRemove(newIteredConnectedTree.tree, this->minNodeNum);
		else finalOutputTree = newIteredConnectedTree.tree;
	} 
// ===============================================================================================================================================================//
	else if (this->mode == dendriticTree)
	{
		connectedComponent dendriteComponent = *this->signalBlobs.begin();
		for (vector<connectedComponent>::iterator compIt = this->signalBlobs.begin() + 1; compIt != this->signalBlobs.end(); ++compIt)
			if (compIt->size > dendriteComponent.size) dendriteComponent = *compIt;

		NeuronTree centroidTree;
// ------- using omp to speed up skeletonization process ------- //
#pragma omp parallel num_threads(this->numProcs)
		{
			boost::container::flat_set<deque<float>> sectionalCentroids = this->fragTraceImgAnalyzer.getSectionalCentroids(dendriteComponent);
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
		}
// ------------------------------------------------------------- //

		NeuronTree MSTtree = this->fragTraceTreeManager.SWC2MSTtree(centroidTree);
		profiledTree profiledMSTtree(MSTtree);
		profiledTree MSTdownSampledTree = this->fragTraceTreeManager.treeDownSample(profiledMSTtree, 10);
		profiledTree MSTdownSampledNoSpikeTree = this->fragTraceTreeManager.spikeRemove(MSTdownSampledTree);
		profiledTree MSTDnNoSpikeBranchBreak = NeuronStructExplorer::MSTbranchBreak(MSTdownSampledNoSpikeTree);
		profiledTree somaHollowedTree = NeuronStructExplorer::treeHollow(MSTDnNoSpikeBranchBreak, 64, 64, 128, 5);
		for (int hollowi = 0; hollowi < this->blankXs.size(); ++hollowi)
		{
			cout << this->blankXs[hollowi] << " " << this->blankYs[hollowi] << " " << this->blankZs[hollowi] << " " << this->blankRadius[hollowi] << endl;
			profiledTree customHollowedTree = NeuronStructExplorer::treeHollow(somaHollowedTree, this->blankXs.at(hollowi), this->blankYs.at(hollowi), this->blankZs.at(hollowi), this->blankRadius.at(hollowi));
			somaHollowedTree = customHollowedTree;
		}
		//profiledTree iteredConnectedTree = this->fragTraceTreeManager.itered_connectLongNeurite(somaHollowedTree, 5);

		NeuronTree floatingExcludedTree;
		if (this->minNodeNum > 0) floatingExcludedTree = NeuronStructExplorer::singleDotRemove(somaHollowedTree, this->minNodeNum);
		else floatingExcludedTree = somaHollowedTree.tree;

		denScaleBackTree = NeuronStructUtil::swcScale(floatingExcludedTree, 2, 2, 1);
		finalOutputTree = denScaleBackTree;
	}

	for (QList<NeuronSWC>::iterator nodeIt = finalOutputTree.listNeuron.begin(); nodeIt != finalOutputTree.listNeuron.end(); ++nodeIt)
		nodeIt->type = 16;

	if (this->finalSaveRootQ != "")
	{
		QString localSWCFullName = this->finalSaveRootQ + "/currBlock.swc";
		writeSWC_file(localSWCFullName, finalOutputTree);
	}
	
	emit emitTracedTree(finalOutputTree);
}

void FragTraceManager::adaThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName)
{
	registeredImg adaSlices;
	adaSlices.imgAlias = outputRegImgName;
	adaSlices.dims[0] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[0];
	adaSlices.dims[1] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[1];
	adaSlices.dims[2] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = adaSlices.dims[0];
	sliceDims[1] = adaSlices.dims[1];
	sliceDims[2] = 1;
	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleAdaThre(sliceIt->second.get(), my1Dslice.get(), sliceDims, this->simpleAdaStepsize, this->simpleAdaRate);
		adaSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	this->fragTraceImgManager.imgDatabase.insert({ adaSlices.imgAlias, adaSlices });

	if (this->saveAdaResults)
	{
		QString saveRootQ = this->simpleAdaSaveDirQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->simpleAdaStepsize)) + "_" + QString::fromStdString(to_string(this->simpleAdaRate));
		this->saveIntermediateResult(outputRegImgName, saveRootQ, dims);
	}
}

void FragTraceManager::simpleThre(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName)
{
	registeredImg adaSlices;
	adaSlices.imgAlias = outputRegImgName;
	adaSlices.dims[0] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[0];
	adaSlices.dims[1] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[1];
	adaSlices.dims[2] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = adaSlices.dims[0];
	sliceDims[1] = adaSlices.dims[1];
	sliceDims[2] = 1;
	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleThresh(sliceIt->second.get(), my1Dslice.get(), sliceDims, this->cutoffIntensity);
		adaSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	this->fragTraceImgManager.imgDatabase.insert({ adaSlices.imgAlias, adaSlices });

	if (this->saveAdaResults)
	{
		QString saveRootQ = this->simpleAdaSaveDirQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->cutoffIntensity));
		this->saveIntermediateResult(outputRegImgName, saveRootQ, dims);
	}
}

void FragTraceManager::gammaCorrect(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName)
{
	if (this->fragTraceImgManager.imgDatabase.find(inputRegImgName) == this->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "No source image found. Do nothing and return.";
	}

	int imgDims[3];
	imgDims[0] = dims[0];
	imgDims[1] = dims[1];
	imgDims[2] = dims[2];
	registeredImg regGamma;
	regGamma.imgAlias = outputRegImgName;
	regGamma.dims[0] = imgDims[0];
	regGamma.dims[1] = imgDims[1];
	regGamma.dims[2] = imgDims[2];
	imgDims[2] = 1;
	cout << this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.size() << endl;
	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		map<int, size_t> histList = ImgProcessor::histQuickList(sliceIt->second.get(), imgDims);
		
		int cutoffIntensity = -1;
		for (int binI = 1; binI < 254; ++binI)
		{
			if (histList[binI] > histList[binI - 1] && histList[binI] < histList[binI + 1])
			{
				cutoffIntensity = binI;
				break;
			}
		}
		
		myImg1DPtr my1Dslice(new unsigned char[imgDims[0] * imgDims[1]]);
		if (cutoffIntensity == -1)
		{
			for (int pixi = 0; pixi < imgDims[0] * imgDims[1]; ++pixi) my1Dslice.get()[pixi] = 0;
			regGamma.slicePtrs.insert({ sliceIt->first, my1Dslice });
		}
		else
		{
			ImgProcessor::stepped_gammaCorrection(sliceIt->second.get(), my1Dslice.get(), imgDims, cutoffIntensity);
			regGamma.slicePtrs.insert({ sliceIt->first, my1Dslice });
		}
	}
	this->fragTraceImgManager.imgDatabase.insert({ regGamma.imgAlias, regGamma });

	if (this->saveAdaResults)
	{
		QString saveRootQ = this->finalSaveRootQ + "\\testFolder1\\gamma";
		if (!QDir(saveRootQ).exists()) QDir().mkpath(saveRootQ);
		this->saveIntermediateResult(outputRegImgName, saveRootQ, dims);
	}
}

void FragTraceManager::histThreImg(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName)
{
	if (this->fragTraceImgManager.imgDatabase.find(inputRegImgName) == this->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "No source image found. Do nothing and return.";
	}

	registeredImg histThreSlices;
	histThreSlices.imgAlias = outputRegImgName;
	histThreSlices.dims[0] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[0];
	histThreSlices.dims[1] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[1];
	histThreSlices.dims[2] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = histThreSlices.dims[0];
	sliceDims[1] = histThreSlices.dims[1];
	sliceDims[2] = 1;

	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		map<string, float> imgStats = ImgProcessor::getBasicStats_no0(sliceIt->second.get(), sliceDims);
		ImgProcessor::simpleThresh(sliceIt->second.get(), my1Dslice.get(), sliceDims, int(floor(imgStats.at("mean") + this->stdFold * imgStats.at("std"))));
		histThreSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	this->fragTraceImgManager.imgDatabase.insert({ histThreSlices.imgAlias, histThreSlices });

	if (this->saveHistThreResults)
	{
		QString saveRootQ = this->histThreSaveDirQ + "\\" + QString::fromStdString(outputRegImgName) + "_std" + QString::fromStdString(to_string(this->stdFold));
		this->saveIntermediateResult(outputRegImgName, saveRootQ, dims);
	}
}

void FragTraceManager::histThreImg3D(const string inputRegImgName, V3DLONG dims[], const string outputRegImgName)
{
	if (this->fragTraceImgManager.imgDatabase.find(inputRegImgName) == this->fragTraceImgManager.imgDatabase.end())
	{
		cerr << "No source image found. Do nothing and return.";
	}

	registeredImg histThreSlices;
	histThreSlices.imgAlias = outputRegImgName;
	histThreSlices.dims[0] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[0];
	histThreSlices.dims[1] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[1];
	histThreSlices.dims[2] = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = histThreSlices.dims[0];
	sliceDims[1] = histThreSlices.dims[1];
	sliceDims[2] = 1;
	map<int, size_t> binMap3D;
	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		map<int, size_t> sliceHistMap = ImgProcessor::histQuickList(sliceIt->second.get(), sliceDims);
		for (map<int, size_t>::iterator binIt = sliceHistMap.begin(); binIt != sliceHistMap.end(); ++binIt)
		{
			if (binMap3D.find(binIt->first) == binMap3D.end()) binMap3D.insert(*binIt);
			else binMap3D[binIt->first] = binMap3D[binIt->first] + binIt->second;
		}
	}
	map<string, float> histList3D = ImgProcessor::getBasicStats_no0_fromHist(binMap3D);

	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != this->fragTraceImgManager.imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleThresh(sliceIt->second.get(), my1Dslice.get(), sliceDims, int(floor(histList3D.at("mean") + this->stdFold * histList3D.at("std"))));
		histThreSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	this->fragTraceImgManager.imgDatabase.insert({ histThreSlices.imgAlias, histThreSlices });

	if (this->saveHistThreResults)
	{
		QString saveRootQ = this->histThreSaveDirQ + "\\" + QString::fromStdString(outputRegImgName) + "_std" + QString::fromStdString(to_string(this->stdFold));
		this->saveIntermediateResult(outputRegImgName, saveRootQ, dims);
	}
}

void FragTraceManager::mask2swc(const string inputImgName, string outputTreeName)
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
	ImgAnalyzer* myImgAnalyzerPtr = new ImgAnalyzer;
	this->signalBlobs = myImgAnalyzerPtr->findSignalBlobs(slice2DVector, sliceDims, 3, mipPtr);
	NeuronTree blob3Dtree = NeuronStructUtil::blobs2tree(this->signalBlobs, true);
	if (this->finalSaveRootQ != "")
	{
		QString blobTreeFullFilenameQ = this->finalSaveRootQ + "\\blob.swc";
		writeSWC_file(blobTreeFullFilenameQ, blob3Dtree);
	}
	profiledTree profiledSigTree(blob3Dtree);
	this->fragTraceTreeManager.treeDataBase.insert({ outputTreeName, profiledSigTree });

	// ----------- Releasing memory ------------
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
	// ------- END of [Releasing memory] -------
}

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

profiledTree FragTraceManager::segConnectAmongTrees(const profiledTree& inputProfiledTree, float distThreshold)
{
	profiledTree tmpTree = inputProfiledTree; 
	profiledTree outputProfiledTree = this->fragTraceTreeManager.itered_connectLongNeurite(tmpTree, distThreshold);
	
	bool typeAssigned = false;
	int assignedType;
	for (map<int, segUnit>::iterator segIt = outputProfiledTree.segs.begin(); segIt != outputProfiledTree.segs.end(); ++segIt)
	{
		for (QList<NeuronSWC>::iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
		{
			if (nodeIt->type != 7)
			{
				typeAssigned = true;
				assignedType = nodeIt->type;
				break;
			}
		}

		if (typeAssigned)
		{
			cout << "existing segment found" << endl;
			for (QList<NeuronSWC>::iterator nodeIt = segIt->second.nodes.begin(); nodeIt != segIt->second.nodes.end(); ++nodeIt)
			{
				nodeIt->type = assignedType;
				outputProfiledTree.tree.listNeuron[outputProfiledTree.node2LocMap.at(nodeIt->n)].type = assignedType;
			}
			typeAssigned = false;
			assignedType = 7;
		}
	}

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

	QString combinedTreeFullName = this->finalSaveRootQ + "/combinedTree.swc";
	writeSWC_file(combinedTreeFullName, outputProfiledTree.tree);

	return outputProfiledTree;
}