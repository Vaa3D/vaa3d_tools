#include "FragTraceImgProcessor.h"
#include "FragTraceTester.h"

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