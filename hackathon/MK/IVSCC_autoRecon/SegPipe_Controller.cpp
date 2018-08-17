#include <iostream>
#include <iterator>

#include <qdir.h>
#include <qdebug.h>

#include "SegPipe_Controller.h"
#include "ImgProcessor.h"
#include "ImgManager.h"

using namespace std;

SegPipe_Controller::SegPipe_Controller(QString inputPath, QString outputPath) : inputCaseRootPath(inputPath), outputRootPath(outputPath)
{
	this->caseList.clear();
	QDir inputDir(inputCaseRootPath);
	inputDir.setFilter(QDir::Dirs);
	this->caseList = inputDir.entryList();

	if (caseList.empty())
	{
		inputContent = singleCase;
		inputDir.setFilter(QDir::Files);
		QStringList sliceList = inputDir.entryList();
		if (sliceList.empty())
		{
			cerr << "Empty folder. Do nothing and return." << endl;
			return;
		}
		else
		{
			for (QStringList::iterator sliceIt = sliceList.begin(); sliceIt != sliceList.end(); ++sliceIt)
			{
				QString sliceFullPathQ = this->inputCaseRootPath + "/" + *sliceIt;
				string sliceFullPath = sliceFullPathQ.toStdString();
				inputSingleCaseSliceFullPaths.push_back(sliceFullPath);

				QString outputSliceFullPathQ = this->outputRootPath + "/" + *sliceIt;
				string outputSliceFullPath = outputSliceFullPathQ.toStdString();
				outputSingleCaseSliceFullPaths.push_back(outputSliceFullPath);
			}
		}
	}
	else
	{
		inputContent = multipleCase;
		for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
		{
			QString caseFullPath = this->inputCaseRootPath + "/" + *caseIt;
			QString outputCaseFullPath = this->outputRootPath + "/" + *caseIt;
			QDir caseFolder(caseFullPath);
			caseFolder.setFilter(QDir::Files);
			QStringList caseSlices = caseFolder.entryList();
			if (*caseIt == "." || *caseIt == "..") continue;
			else if (caseSlices.empty()) cout << "case " << (*caseIt).toStdString() << " is empty. Skip." << endl;

			for (QStringList::iterator sliceIt = caseSlices.begin(); sliceIt != caseSlices.end(); ++sliceIt)
			{
				QString sliceFullPath = caseFullPath + "/" + *sliceIt;
				inputMultiCasesSliceFullPaths.insert(pair<string, string>((*caseIt).toStdString(), sliceFullPath.toStdString()));

				QString outputSliceFullPath = outputCaseFullPath + "/" + *sliceIt;
				outputMultiCasesSliceFullPaths.insert(pair<string, string>((*caseIt).toStdString(), outputSliceFullPath.toStdString()));
			}
		}
	}
}

void SegPipe_Controller::sliceDownSample2D(int downFactor, string method)
{
	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		if (*caseIt == "." || *caseIt == "..") continue;

		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;
		QDir caseDir(caseFullPathQ);
		caseDir.setFilter(QDir::Files);
		QStringList sliceList = caseDir.entryList();
		QString saveCaseFullNameQ = this->outputRootPath + "/" + *caseIt;
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
		QString saveCaseFullNameQ = this->outputRootPath + "/simple_thresholded/" + *caseIt;
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
		QString saveCaseFullNameQ = this->outputRootPath + "/bkg_thresholded/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

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
			unsigned char* threSlice = new unsigned char[dims[0] * dims[1]];
			map<int, size_t> histMap = ImgProcessor::histQuickList(slice1D, dims);
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
			cout << "slice No=" << sliceCount << ": " << thre << endl << endl;
			ImgProcessor::simpleThresh(slice1D, threSlice, dims, thre);

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

void SegPipe_Controller::adaSliceGammaCorrect()
{
	if (this->inputContent == multipleCase)
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


map<QString, NeuronSWC> SegPipe_Controller::findSoma()
{
	map<QString, NeuronSWC> somaList;

	for (QStringList::iterator caseIt = this->caseList.begin(); caseIt != this->caseList.end(); ++caseIt)
	{
		QString saveCaseFullNameQ = this->outputRootPath + "/soma/" + *caseIt;
		if (!QDir(saveCaseFullNameQ).exists()) QDir().mkpath(saveCaseFullNameQ);
		string saveFullPathRoot = saveCaseFullNameQ.toStdString();

		pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range;
		range = this->inputMultiCasesSliceFullPaths.equal_range((*caseIt).toStdString());
		QString caseFullPathQ = this->inputCaseRootPath + "/" + *caseIt;
		vector<unsigned char**> slice2DVector;

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
			unsigned char* soma1D = new unsigned char[dims[0] * dims[1]];
			int threStart = 250;
			ImgProcessor::simpleThresh(slice1D, soma1D, dims, threStart);
			int pixCount = 0;
			for (int i = 0; i < dims[0] * dims[1]; ++i) if (soma1D[i] > 0) ++pixCount;
			if (pixCount <= 50)
			{
				while (pixCount <= 50)
				{
					threStart -= 5;
					ImgProcessor::simpleThresh(slice1D, soma1D, dims, threStart);
					pixCount = 0;
					for (int i = 0; i < dims[0] * dims[1]; ++i) if (soma1D[i] > 0) ++pixCount;
				}
			}

			V3DLONG Dims[4];
			Dims[0] = dims[0];
			Dims[1] = dims[1];
			Dims[2] = 1;
			Dims[3] = 1;

			string fileName = sliceIt->second.substr(sliceIt->second.length() - 9, 9);
			string sliceSaveFullName = saveFullPathRoot + "/" + fileName;
			const char* sliceSaveFullNameC = sliceSaveFullName.c_str();
			ImgManager::saveimage_wrapper(sliceSaveFullNameC, soma1D, Dims, 1);

			slicePtr->~Image4DSimple();
			operator delete(slicePtr);
			if (slice1D) { delete[] slice1D; slice1D = 0; }
			if (soma1D) { delete[] soma1D; soma1D = 0; }
		}
	}

	return somaList;
}

void SegPipe_Controller::somaNeighborhoodThin()
{

}
