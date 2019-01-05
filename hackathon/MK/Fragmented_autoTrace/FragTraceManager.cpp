#include "FragTraceManager.h"

FragTraceManager::FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, bool slices)
{
	int dims[3];
	dims[0] = inputImg4DSimplePtr->getXDim();
	dims[1] = inputImg4DSimplePtr->getYDim();
	dims[2] = inputImg4DSimplePtr->getZDim();
	cout << " -- Current image block dimensions: " << dims[0] << " " << dims[1] << " " << dims[2] << endl;
	int totalbyte = inputImg4DSimplePtr->getTotalBytes();
	unsigned char* img1Dptr = new unsigned char[dims[0] * dims[1] * dims[2]];
	memcpy(img1Dptr, inputImg4DSimplePtr->getRawData(), totalbyte);
	
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
	imgDims[2] = dims[2];

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
			this->histThreImg3D("ada_cutoff", dims, this->histThreImgName);
			this->mask2swc(this->histThreImgName, "blobTree");
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
	
	this->signalBlobs2D = this->fragTraceTreeUtil.swc2signal2DBlobs(this->fragTraceTreeManager.treeDataBase.at("blobTree").tree);
	this->signalBlobs = this->fragTraceTreeUtil.swc2signal3DBlobs(this->fragTraceTreeManager.treeDataBase.at("blobTree").tree);
	myImg1DPtr blob1DPtr = this->fragTraceImgAnalyzer.connectedComponentMask2D(this->signalBlobs, imgDims);
	string compMaskNameString = this->finalSaveRootQ.toStdString() + "test.tif";
	const char* compMaskNameC = compMaskNameString.c_str();
	this->fragTraceImgManager.saveimage_wrapper(compMaskNameC, blob1DPtr.get(), dims, 1);
	/*cout << "original connected component number: " << this->signalBlobs2D.size();
	int connCompSize = 10000;
	int islandIndex;
	for (vector<connectedComponent>::iterator checkIt = this->signalBlobs2D.begin(); checkIt != this->signalBlobs2D.end(); ++checkIt)
	{
		if (checkIt->size < connCompSize)
		{
			connCompSize = checkIt->size;
			islandIndex = checkIt->islandNum;
		}
	}
	cout << ", smallest component: " << islandIndex << ", size = " << connCompSize << endl;*/
	if (this->smallBlobRemove) this->smallBlobRemoval(this->signalBlobs2D, this->smallBlobThreshold);
	if (this->smallBlobRemove) this->smallBlobRemoval(this->signalBlobs, this->smallBlobThreshold);
	//cout << "connected component number after removing small ones (threshold = " << this->smallBlobThreshold << ", 2D): " << this->signalBlobs2D.size() << endl;
	
	NeuronTree blob2Dtree = NeuronStructUtil::blobs2tree(this->signalBlobs2D, true);
	QString blob2DTreeName = this->finalSaveRootQ + "\\2DblobTree.swc";
	writeSWC_file(blob2DTreeName, blob2Dtree);
	
	NeuronTree blob3Dtree = NeuronStructUtil::blobs2tree(this->signalBlobs, true);
	QString blob3DTreeName = this->finalSaveRootQ + "\\3DblobTree.swc";
	writeSWC_file(blob3DTreeName, blob3Dtree);
	
	this->get2DcentroidsTree(this->signalBlobs2D);
	NeuronTree cleanedUpTree = NeuronStructUtil::swcZclenUP(this->fragTraceTreeManager.treeDataBase.at("centerTree").tree);
	profiledTree profiledCleanedTree(cleanedUpTree);
	this->fragTraceTreeManager.treeDataBase.insert({ "centerTreeCleaned", profiledCleanedTree });
	QString centerCleanedTreeNameQ = this->finalSaveRootQ + "\\centerCleaned.swc";
	writeSWC_file(centerCleanedTreeNameQ, this->fragTraceTreeManager.treeDataBase.at("centerTreeCleaned").tree);
	
	if (this->MST)
	{
		if (this->tiledMST)
		{
			NeuronTree tiledMSTtree = this->fragTraceTreeManager.SWC2MSTtree_tiled(this->fragTraceTreeManager.treeDataBase.at("centerTreeCleaned").tree, float(this->tileLength), float(this->zSectionNum));
			profiledTree profiledTiledMSTtree(tiledMSTtree);
			this->fragTraceTreeManager.treeDataBase.insert({ "MSTtree", profiledTiledMSTtree });
		}
		else
		{
			NeuronTree MSTtree = this->fragTraceTreeManager.SWC2MSTtree(this->fragTraceTreeManager.treeDataBase.at("centerTreeCleaned").tree);
			profiledTree profiledMSTtree(MSTtree);
			this->fragTraceTreeManager.treeDataBase.insert({ "MSTtree", profiledMSTtree });
		}
	}
	QString tiledMSTtreeNameQ = this->finalSaveRootQ + "\\tiledMSTtree.swc";
	writeSWC_file(tiledMSTtreeNameQ, this->fragTraceTreeManager.treeDataBase.at("MSTtree").tree);
	
	NeuronTree noLongSegTree = NeuronStructExplorer::longConnCut(this->fragTraceTreeManager.treeDataBase.at("MSTtree"), this->segLengthLimit);
	profiledTree profiledCleanedCutTree(noLongSegTree);
	this->fragTraceTreeManager.treeDataBase.insert({ "MSTtree_longCut", profiledCleanedCutTree });
	
	NeuronTree noBranchTree = NeuronStructExplorer::MSTbranchBreak(this->fragTraceTreeManager.treeDataBase.at("MSTtree_longCut"));
	profiledTree profiledNoBranchTree(noBranchTree);
	this->fragTraceTreeManager.treeDataBase.insert({ "MST_longCut_noBranch", profiledNoBranchTree });

	profiledTree profiledElongatedTree = this->fragTraceTreeManager.itered_segElongate(this->fragTraceTreeManager.treeDataBase.at("MST_longCut_noBranch"));
	this->fragTraceTreeManager.treeDataBase.insert({ "MSTelongatedTree", profiledElongatedTree });
	QString elongatedTreeNameQ = this->finalSaveRootQ + "\\elongated.swc";
	writeSWC_file(elongatedTreeNameQ, profiledElongatedTree.tree);

	NeuronTree noDotTree = NeuronStructExplorer::singleDotRemove(this->fragTraceTreeManager.treeDataBase.at("MSTelongatedTree"), this->minNodeNum);
	profiledTree profiledCleanedUpTree(noDotTree);
	this->fragTraceTreeManager.treeDataBase.insert({ "MSTelongatedClean", profiledCleanedUpTree });
	QString finalSaveFullName = this->finalSaveRootQ + "\\traced.swc";
	writeSWC_file(finalSaveFullName, this->fragTraceTreeManager.treeDataBase.at("MSTelongatedClean").tree);

	emit emitTracedTree(this->fragTraceTreeManager.treeDataBase.at("MSTelongatedClean").tree);
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
	sliceDims[0] = this->fragTraceImgManager.imgDatabase.at(*(this->imgThreSeq.end() - 1)).dims[0];
	sliceDims[1] = this->fragTraceImgManager.imgDatabase.at(*(this->imgThreSeq.end() - 1)).dims[1];
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
	this->signalBlobs = myImgAnalyzerPtr->findSignalBlobs_2Dcombine(slice2DVector, sliceDims, mipPtr);

	// ----------- Releasing memory ------------
	delete[] mipPtr;
	mipPtr = nullptr;
	for (vector<unsigned char**>::iterator slice2DPtrIt = slice2DVector.begin(); slice2DPtrIt != slice2DVector.end(); ++slice2DPtrIt)
	{
		for (int yi = 0; yi < sliceDims[1]; ++yi)
		{
			delete[](*slice2DPtrIt)[yi];
			(*slice2DPtrIt)[yi] = nullptr;
		}
		delete[] * slice2DPtrIt;
		*slice2DPtrIt = nullptr;
	}
	slice2DVector.clear();
	// ------- END of [Releasing memory] -------

	QList<NeuronSWC> allSigs;
	for (vector<connectedComponent>::iterator connIt = this->signalBlobs.begin(); connIt != this->signalBlobs.end(); ++connIt)
	{
		for (map<int, set<vector<int>>>::iterator sliceSizeIt = connIt->coordSets.begin(); sliceSizeIt != connIt->coordSets.end(); ++sliceSizeIt)
		{
			for (set<vector<int>>::iterator nodeIt = sliceSizeIt->second.begin(); nodeIt != sliceSizeIt->second.end(); ++nodeIt)
			{
				NeuronSWC sig;
				V3DLONG blobID = connIt->islandNum;
				sig.n = blobID;
				sig.x = nodeIt->at(1);
				sig.y = nodeIt->at(0);
				sig.z = sliceSizeIt->first;
				sig.type = 2;
				sig.parent = -1;
				allSigs.push_back(sig);
			}
		}
	}
	NeuronTree sigTree;
	sigTree.listNeuron = allSigs;
	QString blobTreeFullFilenameQ = this->finalSaveRootQ + "\\blob.swc";
	writeSWC_file(blobTreeFullFilenameQ, sigTree);
	allSigs.clear();

	profiledTree profiledSigTree(sigTree);
	this->fragTraceTreeManager.treeDataBase.insert({ outputTreeName, profiledSigTree });
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

