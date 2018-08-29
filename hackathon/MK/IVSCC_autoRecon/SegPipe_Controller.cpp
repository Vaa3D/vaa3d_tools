#include <iostream>
#include <iterator>
#include <cmath>
#include <unordered_map>

#include <qdir.h>
#include <qfile.h>
#include <qdebug.h>

#include "SegPipe_Controller.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

using namespace std;

SegPipe_Controller::SegPipe_Controller(QString inputPath, QString outputPath) : inputCaseRootPath(inputPath), outputRootPath(outputPath)
{
	this->caseList.clear();
	QDir inputDir(inputCaseRootPath);
	inputDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	this->caseList = inputDir.entryList();

	if (caseList.empty())
	{
		inputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->caseList = inputDir.entryList();
		this->inputSWCRootPath = inputPath;
		this->inputCaseRootPath = "";
	}
	else
	{
		inputContent = multipleCase;
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

		this->myImgManagerPtr = new ImgManager;
		this->myImgManagerPtr->inputCaseRootPath = this->inputCaseRootPath;
		this->myImgManagerPtr->caseList = this->caseList;
		this->myImgManagerPtr->inputMultiCasesSliceFullPaths = this->inputMultiCasesSliceFullPaths;
	}
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

void SegPipe_Controller::sliceThre(float thre)
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
			unsigned char* threSlice = new unsigned char[dims[0] * dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(slice1D, dims);
			int topBracket = 0;
			float pixCount = histMap[0];
			for (int bracketI = 1; bracketI < 255; ++bracketI)
			{
				pixCount = pixCount + float(histMap[bracketI]);
				if (pixCount / float(dims[0] * dims[1]) >= thre)
				{
					topBracket = bracketI;
					break;
				}
			}
			ImgProcessor::simpleThresh(slice1D, threSlice, dims, topBracket);

			V3DLONG Dims[4];
			Dims[0] = dims[0];
			Dims[1] = dims[1];
			Dims[2] = 1;
			Dims[3] = 1;

			string fileName = sliceIt->second.substr(sliceIt->second.length() - 9, 9);
			string sliceSaveFullName = saveFullPathRoot + "/" + fileName;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, threSlice, Dims, 1);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
			if (threSlice) { delete[] threSlice; threSlice = 0; }
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
		myImgManagerPtr->imgManager_regisImg(*caseIt, slices);
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

void SegPipe_Controller::findSomaMass()
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

		int massSize = 0;
		int startIntensity = 255;
		while (massSize <= 27)
		{
			startIntensity -= 5;
			int pixCount = 0;
			int dims[3];
			for (multimap<string, string>::iterator sliceIt = range.first; sliceIt != range.second; ++sliceIt)
			{
				const char* inputFullPathC = (sliceIt->second).c_str();
				Image4DSimple* slicePtr = new Image4DSimple;
				slicePtr->loadImage(inputFullPathC);
				dims[0] = int(slicePtr->getXDim());
				dims[1] = int(slicePtr->getYDim());
				dims[2] = 1;
				long int totalbyteSlice = slicePtr->getTotalBytes();
				unsigned char* slice1D = new unsigned char[totalbyteSlice];
				memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);

				unsigned char* somaThreSlice1D = new unsigned char[totalbyteSlice];
				ImgProcessor::simpleThresh(slice1D, somaThreSlice1D, dims, startIntensity);
				for (int i = 0; i < dims[0] * dims[1]; ++i)
				{
					if (somaThreSlice1D[i] > 0) ++pixCount;
				}

				V3DLONG Dims[4];
				Dims[0] = dims[0];
				Dims[1] = dims[1];
				Dims[2] = 1;
				Dims[3] = 1;

				string fileName = sliceIt->second.substr(sliceIt->second.length() - 9, 9);
				string sliceSaveFullName = saveFullPathRoot + "/" + fileName;
				const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
				ImgManager::saveimage_wrapper(sliceSaveFullNameC, somaThreSlice1D, Dims, 1);

				slicePtr->~Image4DSimple();
				operator delete(slicePtr);
				if (slice1D) { delete[] slice1D; slice1D = 0; }
				if (somaThreSlice1D) { delete[] somaThreSlice1D; somaThreSlice1D = 0; }
			}
			massSize = massSize + pixCount;
		}
	}
}

void SegPipe_Controller::findConnComponent()
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		cout << "Processing case " << (*caseIt).toStdString() << ":" << endl;
		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;
		qDebug() << caseFullPathQ;
		vector<unsigned char**> slice2DVector;

		QString swcSaveFullNameQ = this->outputRootPath + "/connComponents/" + *caseIt + ".swc";
		QFile swcFileCheck(swcSaveFullNameQ);
		if (swcFileCheck.exists())
		{
			cerr << "This case is complete. Skip " << (*caseIt).toStdString() << endl;
			continue;
		}

		int dims[3];
		for (multimap<string, string>::iterator sliceIt = range.first; sliceIt != range.second; ++sliceIt)
		{
			const char* inputFullPathC = (sliceIt->second).c_str();
			Image4DSimple* slicePtr = new Image4DSimple;
			slicePtr->loadImage(inputFullPathC);
			dims[0] = int(slicePtr->getXDim());
			dims[1] = int(slicePtr->getYDim());
			dims[2] = 1;
			long int totalbyteSlice = slicePtr->getTotalBytes();
			unsigned char* slice1D = new unsigned char[totalbyteSlice];
			memcpy(slice1D, slicePtr->getRawData(), totalbyteSlice);

			unsigned char** slice2DPtr = new unsigned char*[dims[1]];
			for (int j = 0; j < dims[1]; ++j)
			{
				slice2DPtr[j] = new unsigned char[dims[0]];
				for (int i = 0; i < dims[0]; ++i) slice2DPtr[j][i] = slice1D[dims[0] * j + i];
			}
			slice2DVector.push_back(slice2DPtr);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
		}
		cout << "slice preparation done." << endl;

		this->connComponents.clear();
		this->connComponents = ImgAnalyzer::findConnectedComponent(slice2DVector, dims);
		connectedComponent soma;
		QList<NeuronSWC> allSigs;
		for (vector<connectedComponent>::iterator connIt = this->connComponents.begin(); connIt != this->connComponents.end(); ++connIt)
		{
			for (map<int, set<vector<int> > >::iterator sliceSizeIt = connIt->coordSets.begin(); sliceSizeIt != connIt->coordSets.end(); ++sliceSizeIt)
			{
				for (set<vector<int> >::iterator nodeIt = sliceSizeIt->second.begin(); nodeIt != sliceSizeIt->second.end(); ++nodeIt)
				{
					NeuronSWC sig;
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

		this->getChebyshevCenters(*caseIt);
	}
}

void SegPipe_Controller::getChebyshevCenters(QString caseNum)
{
	this->centers.clear();
	for (vector<connectedComponent>::iterator it = this->connComponents.begin(); it != this->connComponents.end(); ++it)
	{
		vector<float> center = ImgAnalyzer::ChebyshevCenter(*it);

		NeuronSWC centerNode;
		centerNode.x = center[1];
		centerNode.y = center[0];
		centerNode.z = center[2];
		centerNode.type = 2;
		centerNode.parent = -1;
		this->centers.push_back(centerNode);
	}

	NeuronTree centerTree;
	centerTree.listNeuron = this->centers;
	QString swcSaveFullNameQ = this->outputRootPath + "/centers/" + caseNum + ".swc";
	writeSWC_file(swcSaveFullNameQ, centerTree);
	
}

void SegPipe_Controller::somaNeighborhoodThin()
{

}

void SegPipe_Controller::swc_imgCrop()
{
	QString shiftedSWC_saveRootQ = this->outputRootPath + "/../shiftedSWC/";

	this->myImgManagerPtr = new ImgManager;
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
		myImgManagerPtr->imgManager_regisImg(*caseIt, slices);

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

		NeuronTree newTree;
		for (QList<NeuronSWC>::iterator nodeIt = currCaseTree.listNeuron.begin(); nodeIt != currCaseTree.listNeuron.end(); ++nodeIt)
		{
			NeuronSWC newNode = *nodeIt;
			newNode.x = (nodeIt->x / 4) - xlb;
			newNode.y = (nodeIt->y / 4) - ylb;
			newNode.z = (nodeIt->z / 2);
			newTree.listNeuron.push_back(newNode);
		}
		QString newTreeName = shiftedSWC_saveRootQ + *caseIt + ".swc";
		writeSWC_file(newTreeName, newTree);

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
	operator delete(myImgManagerPtr);
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
		NeuronTree MSTtree = NeuronStructExplorer::SWC2MSTtree(currTree);

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, MSTtree);
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
		NeuronStructExplorer::MSTtreeCut(currTree, 6, 15);

		QString outputSWCFullPath = this->outputRootPath + "/" + *caseIt;
		writeSWC_file(outputSWCFullPath, currTree);
	}
}

void SegPipe_Controller::getTiledMST()
{
	float xyLength = 30;
	float zLength = 5;
	map<string, QList<NeuronSWC> > tiledSWCmap;
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
			int tileZlabel = int(floor(it->z / zLength));
			string swcTileKey = to_string(tileXlabel) + "_" + to_string(tileYlabel) + "_" + to_string(tileZlabel);
			if (tiledSWCmap.insert(pair<string, QList<NeuronSWC> >(swcTileKey, tileSWCList)).first != tiledSWCmap.end()) tiledSWCmap[swcTileKey].push_back(*it);
		}
		cout << "tiledSWCmap size = " << tiledSWCmap.size() << endl;

		NeuronTree assembledTree;
		//NeuronTree testTree;
		for (map<string, QList<NeuronSWC> >::iterator it = tiledSWCmap.begin(); it != tiledSWCmap.end(); ++it)
		{
			NeuronTree tileTree;
			tileTree.listNeuron = it->second;
			NeuronTree tileMSTtree = NeuronStructExplorer::SWC2MSTtree(tileTree);

			int currnodeNum = assembledTree.listNeuron.size();
			//if (currnodeNum > 50) break;	
			for (QList<NeuronSWC>::iterator nodeIt = tileMSTtree.listNeuron.begin(); nodeIt != tileMSTtree.listNeuron.end(); ++nodeIt)
			{
				nodeIt->n = nodeIt->n + currnodeNum;
				if (nodeIt->parent != -1)
				{	
					nodeIt->parent = nodeIt->parent + currnodeNum;
					//cout << "  " << nodeIt->parent << " " << currnodeNum << endl;
				}

				//if (currnodeNum >= 2900 && currnodeNum <= 3000) testTree.listNeuron.push_back(*nodeIt);
				//cout << nodeIt->n << " " << nodeIt->parent << endl;
				assembledTree.listNeuron.push_back(*nodeIt);
			}
		}

		QString outputSWCPath = this->outputRootPath + "/" + *caseIt; 
		//writeSWC_file(outputSWCPath, assembledTree);
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
		QString outputSWCPath = this->outputRootPath + "/" + *caseIt;\
		writeSWC_file(outputSWCPath, scaledTree);
	}
}