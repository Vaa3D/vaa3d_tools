#include <iostream>
#include <fstream>
#include <iterator>
#include <cmath>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include <qdir.h>
#include <qfile.h>
#include <qdebug.h>

#include "SegPipe_Controller.h"

using namespace std;

SegPipe_Controller::SegPipe_Controller(QString inputPath, QString outputPath) : inputCaseRootPath(inputPath), outputRootPath(outputPath)
{
	this->caseList.clear();
	QDir inputDir(inputCaseRootPath);
	inputDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	this->caseList = inputDir.entryList();

	if (caseList.empty()) // No directories found.
	{
		inputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->caseList = inputDir.entryList();

		int swcFileCount = 0;
		int tifFileCount = 0;
		for (QStringList::iterator caseCheckIt = this->caseList.begin(); caseCheckIt != this->caseList.end(); ++caseCheckIt)
		{
			if ((*caseCheckIt).contains(".swc")) ++swcFileCount;
			else if ((*caseCheckIt).contains(".tif")) ++tifFileCount;
		}

		if (swcFileCount == this->caseList.size())
		{
			this->inputSWCRootPath = inputPath;
			this->inputCaseRootPath = "";
		}
		else if (tifFileCount == this->caseList.size())
		{
			this->inputSWCRootPath = "";
			this->inputCaseRootPath = inputPath;
		}
	}
	else
	{
		for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
		{
			QString caseFullPath = this->inputCaseRootPath + "/" + *caseIt;
			QString outputCaseFullPath = this->outputRootPath + "/" + *caseIt;
			QDir caseFolder(caseFullPath);
			caseFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
			QStringList caseSlices = caseFolder.entryList();
			if (caseSlices.empty()) cout << "case " << (*caseIt).toStdString() << " is empty. Skip." << endl;

			for (QStringList::iterator sliceIt = caseSlices.begin(); sliceIt != caseSlices.end(); ++sliceIt)
			{
				QString sliceFullPath = caseFullPath + "/" + *sliceIt;
				inputMultiCasesSliceFullPaths.insert(pair<string, string>((*caseIt).toStdString(), sliceFullPath.toStdString()));

				QString outputSliceFullPath = outputCaseFullPath + "/" + *sliceIt;
				outputMultiCasesSliceFullPaths.insert(pair<string, string>((*caseIt).toStdString(), outputSliceFullPath.toStdString()));
			}
		}	
	}

	this->myImgManagerPtr = new ImgManager;
	this->myImgManagerPtr->inputCaseRootPath = this->inputCaseRootPath;
	this->myImgManagerPtr->caseList = this->caseList;
	this->myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;

	this->myImgAnalyzerPtr = new ImgAnalyzer;

	this->myNeuronUtilPtr = new NeuronStructUtil;

	this->myNeuronStructExpPtr = new NeuronStructExplorer;
}

void SegPipe_Controller::singleTaskDispatcher(deque<task> taskList)
{
	// Use this method to dispatch image cases. The idea is to deliver 'subject package' to other methods.
	// The subject package should be a data struct that contains information and/or instruction of how the images should be handled.
	// To be implemented.

	for (deque<task>::iterator it = taskList.begin(); it != taskList.end(); ++it)
	{
		switch (*it)
		{
			case downsample2D:
				break;
			case threshold2D:
				break;
			case bkgThreshold2D:
				break;
			case gammaCorrect2D:
				break;
		}
	}
}

void SegPipe_Controller::sliceDownSample2D(int downFactor, string method)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;
		qDebug() << "INPUT DIRECTORY: " << caseFullPathQ;
		QDir caseDir(caseFullPathQ);
		caseDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		QStringList sliceList = caseDir.entryList();
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		qDebug() << "OUTPUT DIRECTORY: " << saveCaseFullNameQ;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);

		for (QStringList::iterator sliceIt = sliceList.begin(); sliceIt != sliceList.end(); ++sliceIt)
		{
			QString sliceFullNameQ = caseFullPathQ + "/" + *sliceIt;
			string sliceFullName = sliceFullNameQ.toStdString();
			const char* sliceFullNameC = sliceFullName.c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(sliceFullNameC); 
			int dims[3];
			dims[0] = int(slicePtr->getXDim());
			dims[1] = int(slicePtr->getYDim());
			dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			unsigned char* slice1D = new unsigned char[totalbyteSlice];
			memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);

			long newLength = (dims[0] / downFactor) * (dims[1] / downFactor);
			unsigned char* dnSampledSlice = new unsigned char[newLength];
			if (!method.compare("")) ImgProcessor::imgDownSample2D(slice1D, dnSampledSlice, dims, downFactor);
			else if (!method.compare("max")) ImgProcessor::imgDownSample2DMax(slice1D, dnSampledSlice, dims, downFactor);

			V3DLONG Dims[4];
			Dims[0] = dims[0] / downFactor;
			Dims[1] = dims[1] / downFactor;
			Dims[2] = 1;
			Dims[3] = 1;
			

			string saveFullName = this->outputRootPath.toStdString() + "/" + (*caseIt).toStdString() + "/" + (*sliceIt).toStdString();
			const char* saveFullNameC = saveFullName.c_str();
			ImgManager::saveimage_wrapper(saveFullNameC, dnSampledSlice, Dims, 1);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);

			if (slice1D) { delete[] slice1D; slice1D = 0; }
			if (dnSampledSlice) { delete[] dnSampledSlice; dnSampledSlice = 0; }
		}
	}
}

void SegPipe_Controller::sliceThre(float threPercentile)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);
		qDebug() << *caseIt;

		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			unsigned char* threSlice = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(sliceIt->second.get(), myImgManagerPtr->imgDatabase.begin()->second.dims);
			int topBracket = 0;
			float pixCount = histMap[0];
			for (int bracketI = 1; bracketI < 255; ++bracketI)
			{
				pixCount = pixCount + float(histMap[bracketI]);
				if (pixCount / float(myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]) >= threPercentile)
				{
					topBracket = bracketI;
					break;
				}
			}
			ImgProcessor::simpleThresh(sliceIt->second.get(), threSlice, myImgManagerPtr->imgDatabase.begin()->second.dims, topBracket);

			V3DLONG Dims[4];
			Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
			Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string sliceSaveFullName = saveFullPathRoot + "/" + sliceIt->first;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, threSlice, Dims, 1);

			if (threSlice) { delete[] threSlice; threSlice = 0; }
		}
	}
}

void SegPipe_Controller::threshold3D(float threPercentile)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);
		int zSliceNum = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.size();
		size_t totalPixel = size_t(zSliceNum) * size_t(myImgManagerPtr->imgDatabase.begin()->second.dims[0]) * size_t(myImgManagerPtr->imgDatabase.begin()->second.dims[1]);
		cout << "total pixel num: " << totalPixel << endl;
			 
		map<int, map<int, size_t>> histMapAllSlice;
		int sliceCount = 0;
		cout << "processing slice: ";
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			map<int, size_t> histMap = ImgProcessor::histQuickList(sliceIt->second.get(), myImgManagerPtr->imgDatabase.begin()->second.dims);
			histMapAllSlice.insert(pair<int, map<int, size_t>>(sliceCount, histMap));
			++sliceCount;
			cout << sliceCount << " ";
		}
		cout << endl;

		float currPercentile = 0;
		int threshedPixelCount = 0;
		int currThre = 256;
		while (currPercentile < (1 - threPercentile))
		{
			--currThre;
			cout << "current threshold: " << currThre;
			for (map<int, map<int, size_t>>::iterator sliceHistIt = histMapAllSlice.begin(); sliceHistIt != histMapAllSlice.end(); ++sliceHistIt)
			{
				if (sliceHistIt->second.find(currThre) != sliceHistIt->second.end()) threshedPixelCount = threshedPixelCount + sliceHistIt->second[currThre];
			}

			currPercentile = float(threshedPixelCount) / float(totalPixel);
			cout << " thresholded pixel number: " << threshedPixelCount << " " << currPercentile << endl;
		}

		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			unsigned char* slice1DPtr = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			ImgProcessor::simpleThresh(sliceIt->second.get(), slice1DPtr, myImgManagerPtr->imgDatabase.begin()->second.dims, currThre);

			V3DLONG Dims[4];
			Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
			Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string sliceSaveFullName = saveFullPathRoot + "/" + sliceIt->first;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, slice1DPtr, Dims, 1);

			delete[] slice1DPtr;
		}
	}
}

void SegPipe_Controller::sliceBkgThre()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			unsigned char* threSlice = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(sliceIt->second.get(), myImgManagerPtr->imgDatabase.begin()->second.dims);
			int thre = 0;
			int previousI = 0;
			for (map<int, size_t>::iterator it = histMap.begin(); it != histMap.end(); ++it)
			{
				if (it->first - previousI > 1)
				{
					thre = it->first;
					break;
				}
				previousI = it->first;
			}
			cout << "  slice " << sliceIt->first << " cut off threshold: " << thre << endl;
			ImgProcessor::simpleThresh(sliceIt->second.get(), threSlice, myImgManagerPtr->imgDatabase.begin()->second.dims, thre);

			V3DLONG Dims[4];
			Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
			Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string sliceSaveFullName = saveFullPathRoot + "/" + sliceIt->first;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, threSlice, Dims, 1);

			if (threSlice) { delete[] threSlice; threSlice = 0; }
		}
	}
}

void SegPipe_Controller::makeMIPimgs()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);

		unsigned char* MIPptr = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
		for (size_t j = 0; j < myImgManagerPtr->imgDatabase.begin()->second.dims[1]; ++j)
			for (size_t i = 0; i < myImgManagerPtr->imgDatabase.begin()->second.dims[0]; ++i) MIPptr[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * j + i] = 0;	
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			for (size_t j = 0; j < myImgManagerPtr->imgDatabase.begin()->second.dims[1]; ++j)
			{
				for (size_t i = 0; i < myImgManagerPtr->imgDatabase.begin()->second.dims[0]; ++i)
				{
					unsigned char pixValue = getMax(MIPptr[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * j + i], sliceIt->second[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * j + i]);
					MIPptr[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * j + i] = pixValue;
				}
			}
		}

		V3DLONG Dims[4];
		Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
		Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
		Dims[2] = 1;
		Dims[3] = 1;
		string sliceSaveFullName = (this->outputRootPath).toStdString() + "/" + (*caseIt).toStdString() + ".tif";
		const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
		ImgManager::saveimage_wrapper(sliceSaveFullNameC, MIPptr, Dims, 1);

		delete[] MIPptr;
	}
}

void SegPipe_Controller::makeDescentSkeletons()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QStringList inputNameParse = (*caseIt).split(".");
		QString caseNum = *inputNameParse.begin();

		qDebug() << this->outputRootPath << " " << this->outputRootPath2 << endl;
		QString saveCaseFullNameQ = this->outputRootPath + "/" + caseNum;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		QString saveSkeFullNameQ = this->outputRootPath2 + "/" + caseNum;
		if (!QDir(saveSkeFullNameQ).exists()) QDir().mkpath(saveSkeFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl << endl;
			continue;
		}
		string saveSkeFullPathRoot = saveSkeFullNameQ.toStdString();

		QString inputFileFullPath = this->inputCaseRootPath + "/" + *caseIt;
		myImgManagerPtr->inputSingleCaseSingleSliceFullPath = inputFileFullPath.toStdString();
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::singleCase_singleSlice);

		map<int, size_t> histMap = ImgProcessor::histQuickList(myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin()->second.get(), myImgManagerPtr->imgDatabase.begin()->second.dims);
		histMap.erase(histMap.begin());
		for (map<int, size_t>::iterator histIt = histMap.begin(); histIt != histMap.end(); ++histIt)
		{
			unsigned char* newThreSlice1D = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			ImgProcessor::simpleThresh(myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin()->second.get(), newThreSlice1D, myImgManagerPtr->imgDatabase.begin()->second.dims, histIt->first);
			V3DLONG Dims[4];
			Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
			Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string threshold = to_string(histIt->first);
			string newSliceName = saveFullPathRoot + "/" + caseNum.toStdString() + "_" + threshold + ".tif";
			const char* newSliceNameC = newSliceName.c_str();
			ImgManager::saveimage_wrapper(newSliceNameC, newThreSlice1D, Dims, 1);

			unsigned char* newThreSliceSke1D = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			ImgProcessor::skeleton2D(newThreSlice1D, newThreSliceSke1D, myImgManagerPtr->imgDatabase.begin()->second.dims);
			string newSkeletonSliceName = saveSkeFullPathRoot + "/" + caseNum.toStdString() + "_" + threshold + "skeleton.tif";
			const char* newSkeletonSliceNameC = newSkeletonSliceName.c_str();
			ImgManager::saveimage_wrapper(newSkeletonSliceNameC, newThreSliceSke1D, Dims, 1);

			delete[] newThreSlice1D;
			delete[] newThreSliceSke1D;
		}

		myImgManagerPtr->imgDatabase.clear();
	}
}

void SegPipe_Controller::getSomaBlendedImgs()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);

		string sliceName;
		string threshold;
		for (map<string, myImg1DPtr>::reverse_iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.rbegin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.rend(); ++sliceIt)
		{
			for (size_t i = 0; i < myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]; ++i)
				if (sliceIt->second.get()[i] > 0)
				{
					sliceName = sliceIt->first;
					vector<string> sliceItParse;
					boost::split(sliceItParse, sliceName, boost::is_any_of("_"));
					vector<string> threParse;
					boost::split(threParse, sliceItParse.back(), boost::is_any_of("skeleton"));
					threshold = threParse.front();
					goto HIGHEST_THRESHOLD;
				}
		}

	HIGHEST_THRESHOLD:
		QString mipFullNameQ = this->inputCaseRootPath2 + "/" + *caseIt + ".tif";
		QString mipAlias = *caseIt + "_mip";
		string mipFullName = mipFullNameQ.toStdString();
		myImgManagerPtr->inputSingleCaseSingleSliceFullPath = mipFullName;
		myImgManagerPtr->imgEntry(mipAlias, ImgManager::singleCase_singleSlice);

		unsigned char* blendingPtr = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1] * 2];
		vector<unsigned char*> blending;
		blending.push_back(myImgManagerPtr->imgDatabase.at((*caseIt).toStdString()).slicePtrs.at(sliceName).get());
		blending.push_back(myImgManagerPtr->imgDatabase.at(mipAlias.toStdString()).slicePtrs.begin()->second.get());
		ImgManager::imgsBlend(blending, blendingPtr, myImgManagerPtr->imgDatabase.begin()->second.dims);
		
		V3DLONG Dims[4];
		Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
		Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
		Dims[2] = 1;
		Dims[3] = 2;
		string saveFullName = this->outputRootPath.toStdString() + "/" + (*caseIt).toStdString() + "_" + threshold + ".tif";
		const char* saveFullNameC = saveFullName.c_str();
		ImgManager::saveimage_wrapper(saveFullNameC, blendingPtr, Dims, 1);

		myImgManagerPtr->imgDatabase.clear();
		delete[] blendingPtr;
	}
}

void SegPipe_Controller::sliceGammaCorrect()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		qDebug() << saveCaseFullNameQ;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		for (multimap<string, string>::iterator sliceIt = range.first; sliceIt != range.second; ++sliceIt)
		{
			const char* inputFullPathC = (sliceIt->second).c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(inputFullPathC);
			int dims[3];
			dims[0] = int(slicePtr->getXDim());
			dims[1] = int(slicePtr->getYDim());
			dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			unsigned char* slice1D = new unsigned char[totalbyteSlice];
			memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);
			unsigned char* gammaSlice = new unsigned char[dims[0] * dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(slice1D, dims);
			int gammaStart = 0;
			for (int histI = 1; histI < histMap.size(); ++histI)
			{
				if (histMap[histI] <= histMap[histI - 1] && histMap[histI] <= histMap[histI + 1])
				{
					gammaStart = histI;
					break;
				}
			}
			ImgProcessor::gammaCorrect_eqSeriesFactor(slice1D, gammaSlice, dims, gammaStart);

			V3DLONG Dims[4];
			Dims[0] = dims[0];
			Dims[1] = dims[1];
			Dims[2] = 1;
			Dims[3] = 1;

			string fileName = sliceIt->second.substr(sliceIt->second.length() - 9, 9);
			string sliceSaveFullName = saveFullPathRoot + "/" + fileName;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, gammaSlice, Dims, 1);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
			if (gammaSlice) { delete[] gammaSlice; gammaSlice = 0; }
		}
	}
}

void SegPipe_Controller::sliceReversedGammaCorrect()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/gamma/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		for (multimap<string, string>::iterator sliceIt = range.first; sliceIt != range.second; ++sliceIt)
		{
			const char* inputFullPathC = (sliceIt->second).c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(inputFullPathC);
			int dims[3];
			dims[0] = int(slicePtr->getXDim());
			dims[1] = int(slicePtr->getYDim());
			dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			unsigned char* slice1D = new unsigned char[totalbyteSlice];
			memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);
			unsigned char* gammaSlice = new unsigned char[dims[0] * dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(slice1D, dims);
			int histMin = 100000;
			for (int histI = 1; histI < histMap.size(); ++histI)
			{
				if (histMap[histI] <= histMin)
				{
					histMin = histI;
					break;
				}
			}
			ImgProcessor::reversed_gammaCorrect_eqSeriesFactor(slice1D, gammaSlice, dims, histMin);

			V3DLONG Dims[4];
			Dims[0] = dims[0];
			Dims[1] = dims[1];
			Dims[2] = 1;
			Dims[3] = 1;

			string fileName = sliceIt->second.substr(sliceIt->second.length() - 9, 9);
			string sliceSaveFullName = saveFullPathRoot + "/" + fileName;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, gammaSlice, Dims, 1);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
			if (gammaSlice) { delete[] gammaSlice; gammaSlice = 0; }
		}
	}
}

void SegPipe_Controller::histQuickList()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		int sliceCount = 0;
		for (multimap<string, string>::iterator sliceIt = range.first; sliceIt != range.second; ++sliceIt)
		{
			++sliceCount;
			const char* inputFullPathC = (sliceIt->second).c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(inputFullPathC);
			int dims[3];
			dims[0] = int(slicePtr->getXDim());
			dims[1] = int(slicePtr->getYDim());
			dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			unsigned char* slice1D = new unsigned char[totalbyteSlice];
			memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);
			map<int, size_t> histMap = ImgProcessor::histQuickList(slice1D, dims);
			cout << "slice No: " << sliceCount << endl;
			for (map<int, size_t>::iterator it = histMap.begin(); it != histMap.end(); ++it) cout << it->first << " " << it->second << endl;
			cout << endl;

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
		}
	}
}

void SegPipe_Controller::findSomaMass(int somaSizeThre)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This folder already exists. Skip case: " << (*caseIt).toStdString() << endl;
			continue;
		}
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);

		map<int, map<int, size_t>> histMapAllSlice;
		int sliceCount = 0;
		cout << "processing slice: ";
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			map<int, size_t> histMap = ImgProcessor::histQuickList(sliceIt->second.get(), myImgManagerPtr->imgDatabase.begin()->second.dims);
			histMapAllSlice.insert(pair<int, map<int, size_t>>(sliceCount, histMap));
			++sliceCount;
			cout << sliceCount << " ";
		}
		cout << endl;

		int pixelCount = 0;
		int currThre = 256;
		while (pixelCount <= somaSizeThre)
		{
			--currThre;
			cout << "current threshold: " << currThre;
			for (map<int, map<int, size_t>>::iterator sliceHistIt = histMapAllSlice.begin(); sliceHistIt != histMapAllSlice.end(); ++sliceHistIt)
			{
				if (sliceHistIt->second.find(currThre) != sliceHistIt->second.end()) pixelCount = pixelCount + sliceHistIt->second[currThre];
			}
			cout << " thresholded pixel number: " << pixelCount << endl;
		}

		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
		{
			unsigned char* slice1DPtr = new unsigned char[myImgManagerPtr->imgDatabase.begin()->second.dims[0] * myImgManagerPtr->imgDatabase.begin()->second.dims[1]];
			ImgProcessor::simpleThresh(sliceIt->second.get(), slice1DPtr, myImgManagerPtr->imgDatabase.begin()->second.dims, currThre);

			V3DLONG Dims[4];
			Dims[0] = myImgManagerPtr->imgDatabase.begin()->second.dims[0];
			Dims[1] = myImgManagerPtr->imgDatabase.begin()->second.dims[1];
			Dims[2] = 1;
			Dims[3] = 1;
			string sliceSaveFullName = saveFullPathRoot + "/" + sliceIt->first;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, slice1DPtr, Dims, 1);

			delete[] slice1DPtr;
		}
	}
}

void SegPipe_Controller::findSignalBlobs2D()
{
	myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		cout << "Processing case " << (*caseIt).toStdString() << ":" << endl;
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;
		vector<unsigned char**> slice2DVector;

		QString swcSaveFullNameQ = this->outputRootPath + "/" + *caseIt + ".swc";
		QFile swcFileCheck(swcSaveFullNameQ);
		if (swcFileCheck.exists())
		{
			cerr << "This case is complete. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}

		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);
		int dims[3]; 
		dims[0] = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[0];
		dims[1] = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[1];
		dims[2] = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[2];
		unsigned char* MIP1Dptr = new unsigned char[dims[0] * dims[1]];
		for (int i = 0; i < dims[0] * dims[1]; ++i) MIP1Dptr[i] = 0;
		
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].slicePtrs.end(); ++sliceIt)
		{
			ImgProcessor::imageMax(sliceIt->second.get(), MIP1Dptr, MIP1Dptr, dims);

			unsigned char** slice2DPtr = new unsigned char*[dims[1]];
			for (int j = 0; j < dims[1]; ++j)
			{
				slice2DPtr[j] = new unsigned char[dims[0]];
				for (int i = 0; i < dims[0]; ++i) slice2DPtr[j][i] = sliceIt->second.get()[dims[0] * j + i];
			}
			slice2DVector.push_back(slice2DPtr);
		}
		cout << "slice preparation done." << endl;

		this->signalBlobs.clear();
		ImgAnalyzer* myAnalyzer = new ImgAnalyzer;
		this->signalBlobs = myImgAnalyzerPtr->findSignalBlobs_2Dcombine(slice2DVector, dims, MIP1Dptr);

		// ----------- Releasing memory ------------
		delete[] MIP1Dptr;
		MIP1Dptr = nullptr;
		for (vector<unsigned char**>::iterator slice2DPtrIt = slice2DVector.begin(); slice2DPtrIt != slice2DVector.end(); ++slice2DPtrIt)
		{
			for (int yi = 0; yi < dims[1]; ++yi)
			{
				delete[] (*slice2DPtrIt)[yi];
				(*slice2DPtrIt)[yi] = nullptr;
			}
			delete[] *slice2DPtrIt;
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
		writeSWC_file(swcSaveFullNameQ, sigTree);
		allSigs.clear();
	}
}

void SegPipe_Controller::swc2DsignalBlobsCenter()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		vector<connectedComponent> signalBlobs2D = myNeuronUtilPtr->swc2signal2DBlobs(currTree);
		//cout << signalBlobs2D.size() << endl;

		this->centers.clear();
		for (vector<connectedComponent>::iterator it = signalBlobs2D.begin(); it != signalBlobs2D.end(); ++it)
		{
			if (it->size <= 3) continue;
			ImgAnalyzer::ChebyshevCenter_connComp(*it);
			NeuronSWC centerNode;
			centerNode.n = it->islandNum;
			centerNode.x = it->ChebyshevCenter[0];
			centerNode.y = it->ChebyshevCenter[1];
			centerNode.z = it->ChebyshevCenter[2];
			centerNode.type = 2;
			centerNode.parent = -1;
			this->centers.push_back(centerNode);
		}

		NeuronTree centerTree;
		centerTree.listNeuron = this->centers;
		QString swcSaveFullNameQ = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(swcSaveFullNameQ, centerTree);
	}
}

void SegPipe_Controller::swcSignalBlob3Dcenter()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		vector<connectedComponent> signalBlobs3D = myNeuronUtilPtr->swc2signal3DBlobs(currTree);

		this->centers.clear();
		for (vector<connectedComponent>::iterator it = signalBlobs3D.begin(); it != signalBlobs3D.end(); ++it)
		{
			if (it->size <= 3) continue;

			ImgAnalyzer::ChebyshevCenter_connComp(*it);
			NeuronSWC centerNode;
			centerNode.n = it->islandNum;
			centerNode.x = it->ChebyshevCenter[0];
			centerNode.y = it->ChebyshevCenter[1];
			centerNode.z = it->ChebyshevCenter[2];
			centerNode.type = 2;
			centerNode.parent = -1;
			this->centers.push_back(centerNode);
		}

		NeuronTree centerTree;
		centerTree.listNeuron = this->centers;
		QString swcSaveFullNameQ = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(swcSaveFullNameQ, centerTree);
	}
}

void SegPipe_Controller::getChebyshevCenters(QString caseNum)
{
	this->centers.clear();
	for (vector<connectedComponent>::iterator it = this->signalBlobs.begin(); it != this->signalBlobs.end(); ++it)
	{
		ImgAnalyzer::ChebyshevCenter_connComp(*it);

		NeuronSWC centerNode;
		centerNode.x = it->ChebyshevCenter[1];
		centerNode.y = it->ChebyshevCenter[0];
		centerNode.z = it->ChebyshevCenter[2];
		centerNode.type = 2;
		centerNode.parent = -1;
		this->centers.push_back(centerNode);
	}

	NeuronTree centerTree;
	centerTree.listNeuron = this->centers;
	QString swcSaveFullNameQ = this->outputRootPath + "/centers/" + caseNum + ".swc";
	writeSWC_file(swcSaveFullNameQ, centerTree);
	
}

void SegPipe_Controller::getSomaCandidates(float distThre)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);

		vector<connectedComponent> somaCandidateList;
		if (distThre == 30) somaCandidateList = NeuronStructUtil::swc2clusters_distance(currTree);
		else somaCandidateList = NeuronStructUtil::swc2clusters_distance(currTree, distThre);
		NeuronTree somaOutput;
		int nodeNum = 1;
		for (vector<connectedComponent>::iterator connCompIt = somaCandidateList.begin(); connCompIt != somaCandidateList.end(); ++connCompIt)
		{
			ImgAnalyzer::ChebyshevCenter_connComp(*connCompIt);

			NeuronSWC somaNode;
			somaNode.n = nodeNum;
			somaNode.x = connCompIt->ChebyshevCenter[0];
			somaNode.y = connCompIt->ChebyshevCenter[1];
			somaNode.z = connCompIt->ChebyshevCenter[2];
			somaNode.type = 1;
			somaNode.parent = -1;
			somaOutput.listNeuron.push_back(somaNode);

			++nodeNum;
		}
		QString swcSaveFullNameQ = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(swcSaveFullNameQ, somaOutput);
	}
}

void SegPipe_Controller::getDendriteSkeletonStart()
{

}

void SegPipe_Controller::swcMapBack()
{
	QString scaledSWC_saveRootQ = this->outputSWCRootPath;

	myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		cout << "Processing case " << (*caseIt).toStdString() << ":" << endl;
		
		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);

		QString currInputSWCFile = this->refSWCRootPath + "/" + *caseIt + ".swc";
		NeuronTree currCaseTree = readSWC_file(currInputSWCFile);
		
		vector<int> offSets = ImgManager::retreiveSWCcropDnParam_imgBased(myImgManagerPtr->imgDatabase[(*caseIt).toStdString()], currCaseTree.listNeuron, 4, 4, 2);
		QString targetedTreeFullNameQ = this->inputSWCRootPath + "/" + *caseIt + ".swc";
		NeuronTree targetedTree = readSWC_file(targetedTreeFullNameQ);
		NeuronTree registeredTree;
		registeredTree.listNeuron.clear();
		for (QList<NeuronSWC>::iterator nodeIt = targetedTree.listNeuron.begin(); nodeIt != targetedTree.listNeuron.end(); ++nodeIt)
		{
			NeuronSWC newNode = *nodeIt;
			newNode.x = (nodeIt->x + offSets.at(0)) * 4;
			newNode.y = (nodeIt->y + offSets.at(1)) * 4;
			newNode.z = (nodeIt->z + offSets.at(2)) * 2;
			registeredTree.listNeuron.push_back(newNode);
		}
		QString newTreeName = scaledSWC_saveRootQ + *caseIt + ".swc";
		writeSWC_file(newTreeName, registeredTree);
	}
}

void SegPipe_Controller::swc_imgCrop()
{
	QString scaledSWC_saveRootQ = this->outputSWCRootPath;

	myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		cout << "Processing case " << (*caseIt).toStdString() << ":" << endl;
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		else
		{
			cerr << "This case is done. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		string saveCaseFullPathRoot = saveCaseFullNameQ.toStdString();
		myImgManagerPtr->imgDatabase.clear();
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::slices);

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;

		QString currInputSWCFile = this->refSWCRootPath + "/" + *caseIt + ".swc";
		NeuronTree currCaseTree = readSWC_file(currInputSWCFile);
		int xlb = 10000, xhb = 0, ylb = 10000, yhb = 0;
		for (QList<NeuronSWC>::iterator nodeIt = currCaseTree.listNeuron.begin(); nodeIt != currCaseTree.listNeuron.end(); ++nodeIt)
		{
			int thisNodeX = int(round(nodeIt->x / 4));
			int thisNodeY = int(round(nodeIt->y / 4));
			if (thisNodeX < xlb) xlb = thisNodeX;
			if (thisNodeX > xhb) xhb = thisNodeX;
			if (thisNodeY < ylb) ylb = thisNodeY;
			if (thisNodeY > yhb) yhb = thisNodeY;
		}
		xlb -= 10; xhb += 10; ylb -= 10; yhb += 10;
		if (xlb <= 0) xlb = 1;
		if (xhb >= myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[0]) xhb = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[0];
		if (ylb <= 0) ylb = 1;
		if (yhb >= myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[1]) yhb = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[1];
		int newDims[3];
		newDims[0] = xhb - xlb + 1;
		newDims[1] = yhb - ylb + 1;
		newDims[2] = 1;
		cout << "boundries: " << xlb << " " << xhb << " " << ylb << " " << yhb << endl;
		
		if (!this->outputSWCRootPath.isEmpty())
		{
			NeuronTree newTree;
			for (QList<NeuronSWC>::iterator nodeIt = currCaseTree.listNeuron.begin(); nodeIt != currCaseTree.listNeuron.end(); ++nodeIt)
			{
				NeuronSWC newNode = *nodeIt;
				newNode.x = (nodeIt->x / 4) - xlb;
				newNode.y = (nodeIt->y / 4) - ylb;
				newNode.z = (nodeIt->z / 2);
				newTree.listNeuron.push_back(newNode);
			}
			QString newTreeName = scaledSWC_saveRootQ + *caseIt + ".swc";
			writeSWC_file(newTreeName, newTree);
		}

		V3DLONG saveDims[4];
		saveDims[0] = newDims[0];
		saveDims[1] = newDims[1];
		saveDims[2] = 1;
		saveDims[3] = 1;
		
		QString saveImgCasePathQ = this->outputRootPath + "/" + *caseIt + "/";
		if (!QDir(saveImgCasePathQ).exists()) QDir().mkpath(saveImgCasePathQ);
		for (map<string, myImg1DPtr>::iterator sliceIt = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].slicePtrs.begin();
			sliceIt != myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].slicePtrs.end(); ++sliceIt)
		{
			unsigned char* croppedSlice = new unsigned char[newDims[0] * newDims[1]];
			ImgProcessor::cropImg2D(sliceIt->second.get(), croppedSlice, xlb, xhb, ylb, yhb, myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims);

			QString saveFileNameQ = saveImgCasePathQ + QString::fromStdString(sliceIt->first);
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, croppedSlice, saveDims, 1);

			if (croppedSlice) { delete[] croppedSlice; croppedSlice = 0; }
		}
	}
}

void SegPipe_Controller::getMST()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		NeuronTree MSTtree = myNeuronStructExpPtr->SWC2MSTtree(currTree);

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, MSTtree);
	}
}

void SegPipe_Controller::getMST_2Dslices()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}

		QStringList caseParse = (*caseIt).split(".");
		QString caseNum = caseParse[0];
		QString outputSWCcasePath = this->outputRootPath + "/" + caseNum;
		if (!QDir(outputSWCcasePath).exists()) QDir().mkpath(outputSWCcasePath);
		else
		{
			cerr << "This case is done. Skip " << caseNum.toStdString() << endl;
			continue;
		}

		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		float zMax = 0;
		for (QList<NeuronSWC>::iterator zIt = currTree.listNeuron.begin(); zIt != currTree.listNeuron.end(); ++zIt) 
			if (zIt->z > zMax) zMax = zIt->z;

		vector<connectedComponent> blob2D = myNeuronUtilPtr->swc2signal2DBlobs(currTree);
		vector<NeuronTree> MSTsliceTrees;
		NeuronTree thisSliceTree;
		for (int i = 0; i <= int(zMax); ++i)
		{
			thisSliceTree.listNeuron.clear();
			for (vector<connectedComponent>::iterator blobIt = blob2D.begin(); blobIt != blob2D.end(); ++blobIt)
			{
				if (blobIt->coordSets.begin()->first == i)
				{
					NeuronSWC centerSWC;
					centerSWC.n = blobIt->islandNum;
					centerSWC.type = 2;
					centerSWC.x = blobIt->ChebyshevCenter[0];
					centerSWC.y = blobIt->ChebyshevCenter[1];
					centerSWC.z = blobIt->ChebyshevCenter[2];
					centerSWC.parent = -1;
					thisSliceTree.listNeuron.push_back(centerSWC);
				}
			}
			if (thisSliceTree.listNeuron.isEmpty()) continue;
			
			NeuronTree thisSliceMSTtree = myNeuronStructExpPtr->SWC2MSTtree(thisSliceTree);
			NeuronTree outputTree = NeuronStructExplorer::MSTtreeCut(thisSliceMSTtree, 15);
			MSTsliceTrees.push_back(outputTree);
		}
		
		for (vector<NeuronTree>::iterator treeIt = MSTsliceTrees.begin(); treeIt != MSTsliceTrees.end(); ++treeIt)
		{
			QString sliceMSTtreeSaveName = outputSWCcasePath + "/" + QString::number((int(treeIt - MSTsliceTrees.begin()))) + ".swc";
			writeSWC_file(sliceMSTtreeSaveName, *treeIt);
		}
	}
}

void SegPipe_Controller::cutMST()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		NeuronTree outputTree = NeuronStructExplorer::MSTtreeCut(currTree, 30);

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, outputTree);
	}
}

void SegPipe_Controller::MSTtrim()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree inputTree = readSWC_file(swcFileFullPathQ);
		profiledTree currTree(inputTree);
		/*vector<segUnit> segs = myNeuronStructExpPtr->MSTtreeTrim(currTree.segs);
		NeuronTree outputTree;
		for (vector<segUnit>::iterator it = segs.begin(); it != segs.end(); ++it)
		{
			for (QList<NeuronSWC>::iterator nodeIt = it->nodes.begin(); nodeIt != it->nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, outputTree);*/
	}
}

void SegPipe_Controller::breakMSTbranch()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFileFullPathQ = this->inputSWCRootPath + "/" + *caseIt;
		QFile swcFileCheck(swcFileFullPathQ);
		if (!swcFileCheck.exists())
		{
			cerr << "This case hasn't been generated. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree currTree = readSWC_file(swcFileFullPathQ);
		myNeuronStructExpPtr->treeEntry(currTree, "currTree");
		NeuronTree outputTree = myNeuronStructExpPtr->MSTbranchBreak(myNeuronStructExpPtr->treeDataBase["currTree"], 20);

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, outputTree);

		myNeuronStructExpPtr->treeDataBase.clear();
	}
}

void SegPipe_Controller::getTiledMST()
{
	float xyLength = 40;
	map<string, QList<NeuronSWC>> tiledSWCmap;
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		tiledSWCmap.clear();
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		QList<NeuronSWC> tileSWCList;
		tileSWCList.clear();
		for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		{
			int tileXlabel = int(floor(it->x / xyLength));
			int tileYlabel = int(floor(it->y / xyLength));
			int tileZlabel = int(floor(it->z / (xyLength / zRATIO)));
			string swcTileKey = to_string(tileXlabel) + "_" + to_string(tileYlabel) + "_" + to_string(tileZlabel);
			tiledSWCmap.insert(pair<string, QList<NeuronSWC>>(swcTileKey, tileSWCList));  
			tiledSWCmap[swcTileKey].push_back(*it);
		}
		cout << "tiledSWCmap size = " << tiledSWCmap.size() << endl;

		NeuronTree assembledTree;
		for (map<string, QList<NeuronSWC>>::iterator it = tiledSWCmap.begin(); it != tiledSWCmap.end(); ++it)
		{
			NeuronTree tileTree;
			tileTree.listNeuron = it->second;
			NeuronTree tileMSTtree = myNeuronStructExpPtr->SWC2MSTtree(tileTree);

			int currnodeNum = assembledTree.listNeuron.size();
			for (QList<NeuronSWC>::iterator nodeIt = tileMSTtree.listNeuron.begin(); nodeIt != tileMSTtree.listNeuron.end(); ++nodeIt)
			{
				nodeIt->n = nodeIt->n + currnodeNum;
				if (nodeIt->parent != -1)
				{	
					nodeIt->parent = nodeIt->parent + currnodeNum;
					//cout << "  " << nodeIt->parent << " " << currnodeNum << endl;
				}

				//cout << nodeIt->n << " " << nodeIt->parent << endl;
				assembledTree.listNeuron.push_back(*nodeIt);
			}
		}

		QString outputSWCPath = this->outputRootPath + "/" + *caseIt; 
		writeSWC_file(outputSWCPath, assembledTree);
	}
}

void SegPipe_Controller::swcRegister()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		QString refSWCPath = this->refSWCRootPath + "/" + *caseIt;
		QFile refCheck(refSWCPath);
		if (!refCheck.exists())
		{
			cerr << "No refSWC for this case. Skip  " << (*caseIt).toStdString() << endl;
			continue;
		}
		NeuronTree refTree = readSWC_file(refSWCPath);

		NeuronTree regTree = NeuronStructUtil::swcRegister(inputTree, refTree);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, regTree);
	}
}

void SegPipe_Controller::swcScale(float xScale, float yScale, float zScale)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);

		NeuronTree scaledTree = NeuronStructUtil::swcScale(inputTree, xScale, yScale, zScale);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, scaledTree);
	}
}

void SegPipe_Controller::correctSWC()
{
	QString scaledSWC_saveRootQ = this->outputSWCRootPath;

	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		string inputSliceFullPath = range.first->second;
		myImgManagerPtr->inputSingleCaseSingleSliceFullPath = inputSliceFullPath;
		myImgManagerPtr->imgEntry(*caseIt, ImgManager::singleCase_singleSlice);

		QString currInputSWCFile = this->refSWCRootPath + "/" + *caseIt + ".swc";
		NeuronTree currCaseTree = readSWC_file(currInputSWCFile);
		int xlb = 10000, xhb = 0, ylb = 10000, yhb = 0;
		for (QList<NeuronSWC>::iterator nodeIt = currCaseTree.listNeuron.begin(); nodeIt != currCaseTree.listNeuron.end(); ++nodeIt)
		{
			int thisNodeX = int(round(nodeIt->x / 4));
			int thisNodeY = int(round(nodeIt->y / 4));
			if (thisNodeX < xlb) xlb = thisNodeX;
			if (thisNodeX > xhb) xhb = thisNodeX;
			if (thisNodeY < ylb) ylb = thisNodeY;
			if (thisNodeY > yhb) yhb = thisNodeY;
		}
		xlb -= 10; xhb += 10; ylb -= 10; yhb += 10;
		if (xlb <= 0) xlb = 1;
		if (xhb >= myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[0]) xhb = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[0];
		if (ylb <= 0) ylb = 1;
		if (yhb >= myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[1]) yhb = myImgManagerPtr->imgDatabase[(*caseIt).toStdString()].dims[1];
		int newDims[3];
		newDims[0] = xhb - xlb + 1;
		newDims[1] = yhb - ylb + 1;
		newDims[2] = 1;
		cout << "boundries: " << xlb << " " << xhb << " " << ylb << " " << yhb << endl;

		NeuronTree correctedTree = this->swcBackTrack(4, 4, 2, xlb, ylb, 0, currCaseTree);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt + ".swc";
		writeSWC_file(outputSWCPath, correctedTree);

		myImgManagerPtr->inputSingleCaseSingleSliceFullPath.clear();
	}
}

void SegPipe_Controller::nodeIdentify()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);

		QString refSWCfullPath = this->refSWCRootPath + "/" + *caseIt;
		NeuronTree refTree = readSWC_file(refSWCfullPath);

		NeuronTree diffTree = NeuronStructUtil::swcIdentityCompare(inputTree, refTree, 50, 20);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, diffTree);
	}
}

void SegPipe_Controller::cleanUpzFor2Dcentroids()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);

		NeuronTree cleanedZtree = NeuronStructUtil::swcZclenUP(inputTree);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, cleanedZtree);
	}
}

void SegPipe_Controller::swcSeparate(QString outputRoot2)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);

		NeuronTree sigTree;
		NeuronTree noiseTree;

		for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		{
			NeuronSWC thisNode = *it;
			if (it->type == 2) sigTree.listNeuron.push_back(thisNode);
			else if (it->type == 3) noiseTree.listNeuron.push_back(thisNode);
		}

		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		QString outputSWCPath2 = outputRoot2 + "/" + *caseIt;
		writeSWC_file(outputSWCPath, sigTree);
		writeSWC_file(outputSWCPath2, noiseTree);
	}
}

void SegPipe_Controller::segElongation()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		string treeName = (*caseIt).toStdString();
		treeName = treeName.substr(0, treeName.length() - 3);

		myNeuronStructExpPtr->treeEntry(inputTree, treeName, 80);
		profiledTree elongatedTree = myNeuronStructExpPtr->itered_segElongate(myNeuronStructExpPtr->treeDataBase.begin()->second, 0.30);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, elongatedTree.tree);

		myNeuronStructExpPtr->treeDataBase.clear();
	}
}

void SegPipe_Controller::segTerminalize()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		string treeName = (*caseIt).toStdString();
		treeName = treeName.substr(0, treeName.length() - 3);

		myNeuronStructExpPtr->treeEntry(inputTree, treeName);
		NeuronTree outputTree = NeuronStructExplorer::segTerminalize(myNeuronStructExpPtr->treeDataBase.begin()->second);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, outputTree);

		myNeuronStructExpPtr->treeDataBase.clear();
	}
}

void SegPipe_Controller::dotRemove()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		string treeName = (*caseIt).toStdString();
		treeName = treeName.substr(0, treeName.length() - 3);

		myNeuronStructExpPtr->treeEntry(inputTree, treeName);
		NeuronTree outputTree = NeuronStructExplorer::singleDotRemove(myNeuronStructExpPtr->treeDataBase.begin()->second, 2);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, outputTree);

		myNeuronStructExpPtr->treeDataBase.clear();
	}
}

void SegPipe_Controller::longConnCut()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		string treeName = (*caseIt).toStdString();
		treeName = treeName.substr(0, treeName.length() - 3);

		myNeuronStructExpPtr->treeEntry(inputTree, treeName);
		NeuronTree outputTree = NeuronStructExplorer::longConnCut(myNeuronStructExpPtr->treeDataBase.begin()->second, 100);
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, outputTree);

		myNeuronStructExpPtr->treeDataBase.clear();
	}
}

void SegPipe_Controller::treeUnion()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree MST2ndTree = readSWC_file(swcFullPath);
		profiledTree profiledMST2ndTree(MST2ndTree);

		QString refSWCfullPath = this->refSWCRootPath + "/" + *caseIt;
		NeuronTree baseTree = readSWC_file(refSWCfullPath);
		profiledTree profiledBaseTree(baseTree);

		profiledTree outputProfiledTree = myNeuronStructExpPtr->treeUnion_MSTbased(profiledMST2ndTree, profiledBaseTree);
		NeuronTree outputTree = outputProfiledTree.tree;
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCPath, outputTree);
	}
}

void SegPipe_Controller::treeWithinDist()
{
	string dist2D = "dist2D.txt";
	string dist3D = "dist3D.txt";
	string outputName = outputRootPath.toStdString() + "/" + dist3D;
	ofstream outputFile(outputName.c_str());
	outputFile << "case num\t" << "mean\t" << "std\t" << "median" << endl;
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString swcFullPath = this->inputSWCRootPath + "/" + *caseIt;
		NeuronTree inputTree = readSWC_file(swcFullPath);
		string treeName = (*caseIt).toStdString();
		treeName = treeName.substr(0, treeName.length() - 3);

		map<string, float> swcWithinStats = NeuronStructUtil::selfNodeDist(inputTree.listNeuron);
		outputFile << (*caseIt).toStdString() << "\t" << swcWithinStats["mean"] << "\t" << swcWithinStats["std"] << "\t" << swcWithinStats["median"] << endl;
	}
	outputFile.close();
}