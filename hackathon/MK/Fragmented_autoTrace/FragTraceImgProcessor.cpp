#include "FragTraceImgProcessor.h"
#include "FragTraceTester.h"

/* =========================== Image Enhancemnet =========================== */
void FragTraceImgProcessor::gammaCorrect(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase) const
{
	registeredImg gammaSlices;
	gammaSlices.imgAlias = outputRegImgName;
	gammaSlices.dims[0] = imgDatabase.at(inputRegImgName).dims[0];
	gammaSlices.dims[1] = imgDatabase.at(inputRegImgName).dims[1];
	gammaSlices.dims[2] = imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = gammaSlices.dims[0];
	sliceDims[1] = gammaSlices.dims[1];
	sliceDims[2] = 1;
	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputRegImgName).slicePtrs.begin();
		sliceIt != imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::stepped_gammaCorrection(sliceIt->second.get(), my1Dslice.get(), sliceDims, 5);
		gammaSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	imgDatabase.insert({ gammaSlices.imgAlias, gammaSlices });

	// ------- Debug ------- //
	/*if (FragTraceTester::isInstantiated())
	{
	QString prefixQ = this->finalSaveRootQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->simpleAdaStepsize)) + "_" + QString::fromStdString(to_string(this->simpleAdaRate));
	FragTraceTester::getInstance()->saveIntermediateImgSlices(outputRegImgName, prefixQ, dims);
	}*/
	// --------------------- //
}

void FragTraceImgProcessor::simpleThre(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const int cutoffIntensity) const
{
	registeredImg adaSlices;
	adaSlices.imgAlias = outputRegImgName;
	adaSlices.dims[0] = imgDatabase.at(inputRegImgName).dims[0];
	adaSlices.dims[1] = imgDatabase.at(inputRegImgName).dims[1];
	adaSlices.dims[2] = imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = adaSlices.dims[0];
	sliceDims[1] = adaSlices.dims[1];
	sliceDims[2] = 1;
	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputRegImgName).slicePtrs.begin(); sliceIt != imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleThresh(sliceIt->second.get(), my1Dslice.get(), sliceDims, cutoffIntensity);
		adaSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	imgDatabase.insert({ adaSlices.imgAlias, adaSlices });

	// ------- Debug ------- //
	/*if (FragTraceTester::isInstantiated())
	{
	QString prefixQ = this->finalSaveRootQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->simpleAdaStepsize)) + "_" + QString::fromStdString(to_string(this->simpleAdaRate));
	FragTraceTester::getInstance()->saveIntermediateImgSlices(outputRegImgName, prefixQ, dims);
	}*/
	// --------------------- //
}

void FragTraceImgProcessor::adaThresholding(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const int stepSize, const int sampleRate) const
{
	registeredImg adaSlices;
	adaSlices.imgAlias = outputRegImgName;
	adaSlices.dims[0] = imgDatabase.at(inputRegImgName).dims[0];
	adaSlices.dims[1] = imgDatabase.at(inputRegImgName).dims[1];
	adaSlices.dims[2] = imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = adaSlices.dims[0];
	sliceDims[1] = adaSlices.dims[1];
	sliceDims[2] = 1;
	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputRegImgName).slicePtrs.begin(); sliceIt != imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleAdaThre(sliceIt->second.get(), my1Dslice.get(), sliceDims, stepSize, sampleRate);
		adaSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	imgDatabase.insert({ adaSlices.imgAlias, adaSlices });

	// ------- Debug ------- //
	/*if (FragTraceTester::isInstantiated())
	{
	QString prefixQ = this->finalSaveRootQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->simpleAdaStepsize)) + "_" + QString::fromStdString(to_string(this->simpleAdaRate));
	FragTraceTester::getInstance()->saveIntermediateImgSlices(outputRegImgName, prefixQ, dims);
	}*/
	// --------------------- //
}

void FragTraceImgProcessor::histThreImg3D(const string inputRegImgName, const string outputRegImgName, map<string, registeredImg>& imgDatabase, const float stdFold) const
{
	if (imgDatabase.find(inputRegImgName) == imgDatabase.end())
	{
		cerr << "No source image found. Do nothing and return.";
	}

	registeredImg histThreSlices;
	histThreSlices.imgAlias = outputRegImgName;
	histThreSlices.dims[0] = imgDatabase.at(inputRegImgName).dims[0];
	histThreSlices.dims[1] = imgDatabase.at(inputRegImgName).dims[1];
	histThreSlices.dims[2] = imgDatabase.at(inputRegImgName).dims[2];

	int sliceDims[3];
	sliceDims[0] = histThreSlices.dims[0];
	sliceDims[1] = histThreSlices.dims[1];
	sliceDims[2] = 1;
	map<int, size_t> binMap3D;
	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputRegImgName).slicePtrs.begin(); sliceIt != imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		map<int, size_t> sliceHistMap = ImgProcessor::histQuickList(sliceIt->second.get(), sliceDims);
		for (map<int, size_t>::iterator binIt = sliceHistMap.begin(); binIt != sliceHistMap.end(); ++binIt)
		{
			if (binMap3D.find(binIt->first) == binMap3D.end()) binMap3D.insert(*binIt);
			else binMap3D[binIt->first] = binMap3D[binIt->first] + binIt->second;
		}
	}
	map<string, float> histList3D = ImgProcessor::getBasicStats_no0_fromHist(binMap3D);

	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputRegImgName).slicePtrs.begin(); sliceIt != imgDatabase.at(inputRegImgName).slicePtrs.end(); ++sliceIt)
	{
		myImg1DPtr my1Dslice(new unsigned char[sliceDims[0] * sliceDims[1]]);
		ImgProcessor::simpleThresh(sliceIt->second.get(), my1Dslice.get(), sliceDims, int(floor(histList3D.at("mean") + stdFold * histList3D.at("std"))));
		histThreSlices.slicePtrs.insert({ sliceIt->first, my1Dslice });
	}
	imgDatabase.insert({ histThreSlices.imgAlias, histThreSlices });

	// ------- Debug ------- //
	/*if (FragTraceTester::isInstantiated())
	{
	QString prefixQ = this->finalSaveRootQ + "\\" + QString::fromStdString(outputRegImgName) + "_" + QString::fromStdString(to_string(this->simpleAdaStepsize)) + "_" + QString::fromStdString(to_string(this->simpleAdaRate));
	FragTraceTester::getInstance()->saveIntermediateImgSlices(outputRegImgName, prefixQ, dims);
	}*/
	// --------------------- //
}
/* ========================================================================= */


/* =========================== Image Segmentation =========================== */
NeuronTree FragTraceImgProcessor::mask2swc(const string inputImgName, map<string, registeredImg>& imgDatabase, vector<connectedComponent>& signalBlobs)
{
	int sliceDims[3];
	sliceDims[0] = imgDatabase.at(inputImgName).dims[0];
	sliceDims[1] = imgDatabase.at(inputImgName).dims[1];
	sliceDims[2] = 1;

	vector<unsigned char**> slice2DVector;
	unsigned char* mipPtr = new unsigned char[sliceDims[0] * sliceDims[1]];
	for (int i = 0; i < sliceDims[0] * sliceDims[1]; ++i) mipPtr[i] = 0;
	for (map<string, myImg1DPtr>::iterator sliceIt = imgDatabase.at(inputImgName).slicePtrs.begin();
		sliceIt != imgDatabase.at(inputImgName).slicePtrs.end(); ++sliceIt)
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

	signalBlobs.clear();
	signalBlobs = this->findSignalBlobs(slice2DVector, sliceDims, 3, mipPtr);

	NeuronTree blob3Dtree = NeuronStructUtil::blobs2tree(signalBlobs, true);

	QString blobTreeFullFilenameQ = this->blobTreeSavePathQ + "\\blob.swc";
	//writeSWC_file(blobTreeFullFilenameQ, blob3Dtree);

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

	return blob3Dtree;
}
/* ======================= END of [Image Segmentation] ====================== */