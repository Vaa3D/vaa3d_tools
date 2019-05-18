#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "imgTester.h"
#include "processMonitoringTester.h"

using namespace std;

void ImgTester::sliceImgStack()
{
	ImgManager myManager;
	myManager.inputSingleCaseFullPath = this->inputSingleImgFullName;
	myManager.imgEntry("stack", ImgManager::singleCase);

	int imgDims[3];
	imgDims[0] = myManager.imgDatabase.begin()->second.dims[0];
	imgDims[1] = myManager.imgDatabase.begin()->second.dims[1];
	imgDims[2] = myManager.imgDatabase.begin()->second.dims[2];

	ImgProcessor::imgStackSlicer(myManager.imgDatabase.begin()->second.slicePtrs.begin()->second.get(), this->imgSlices, imgDims);
	V3DLONG saveDims[4];
	saveDims[0] = imgDims[0];
	saveDims[1] = imgDims[1];
	saveDims[2] = 1;
	saveDims[3] = 1;
	
	for (vector<vector<unsigned char>>::iterator sliceIt = this->imgSlices.begin(); sliceIt != this->imgSlices.end(); ++sliceIt)
	{
		unsigned char* slicePtr = new unsigned char[sliceIt->size()];
		for (vector<unsigned char>::iterator pixIt = sliceIt->begin(); pixIt != sliceIt->end(); ++pixIt)
			slicePtr[size_t(pixIt - sliceIt->begin())] = *pixIt;
		
		string fileName;
		if (int(sliceIt - this->imgSlices.begin()) / 10 == 0) fileName = "000" + to_string(int(sliceIt - this->imgSlices.begin())) + ".tif";
		else if (int(sliceIt - this->imgSlices.begin()) / 100 == 0) fileName = "00" + to_string(int(sliceIt - this->imgSlices.begin())) + ".tif";
		else if (int(sliceIt - this->imgSlices.begin()) / 1000 == 0) fileName = "0" + to_string(int(sliceIt - this->imgSlices.begin())) + ".tif";
		else fileName = to_string(int(sliceIt - this->imgSlices.begin())) + ".tif";

		string saveFullName = this->outputImgPath + "\\" + fileName;
		const char* thisSliceSaveNameC = saveFullName.c_str();
		ImgManager::saveimage_wrapper(thisSliceSaveNameC, slicePtr, saveDims, 1);

		delete[] slicePtr;
	}
}

void ImgTester::thre_stats()
{
	ImgManager myManager(QString::fromStdString(this->inputString));
	if (!QString::fromStdString(this->inputString).contains("."))
	{
		this->outputImgPath = this->outputString;
		for (multimap<string, string>::iterator sliceIt = myManager.inputMultiCasesFullPaths.begin(); sliceIt != myManager.inputMultiCasesFullPaths.end(); ++sliceIt)
		{
			myManager.inputSingleCaseFullPath = sliceIt->second;
			myManager.imgEntry(sliceIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(sliceIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(sliceIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
			map<string, float> imgStats = ImgProcessor::getBasicStats_no0(myManager.imgDatabase.at(sliceIt->first).slicePtrs.begin()->second.get(), myManager.imgDatabase.at(sliceIt->first).dims);
			ImgProcessor::simpleThresh(myManager.imgDatabase.at(sliceIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, int(floor(imgStats.at("mean") + imgStats.at("std"))));

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = QString::fromStdString(this->outputImgPath) + "\\" + QString::fromStdString(sliceIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
}

void ImgTester::ada()
{
	ImgManager myManager(QString::fromStdString(this->inputString));
	if (!QString::fromStdString(this->inputString).contains("."))
	{
		this->outputImgPath = this->outputString;
		for (multimap<string, string>::iterator sliceIt = myManager.inputMultiCasesFullPaths.begin(); sliceIt != myManager.inputMultiCasesFullPaths.end(); ++sliceIt)
		{
			myManager.inputSingleCaseFullPath = sliceIt->second;
			myManager.imgEntry(sliceIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(sliceIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(sliceIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
			ImgProcessor::simpleAdaThre(myManager.imgDatabase.at(sliceIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, 5, 3);
			if (this->cutOff != 0)
			{
				for (int i = 0; i < imgDims[0] * imgDims[1]; ++i)
					if (int(outputImgPtr[i]) <= this->cutOff) outputImgPtr[i] = 0;
			}

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = QString::fromStdString(this->outputImgPath) + "\\" + QString::fromStdString(sliceIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}	
}

void ImgTester::gamma()
{
	ImgManager myManager(QString::fromStdString(this->inputString));
	if (!QString::fromStdString(this->inputString).contains("."))
	{
		this->outputImgPath = this->outputString;
		for (multimap<string, string>::iterator sliceIt = myManager.inputMultiCasesFullPaths.begin(); sliceIt != myManager.inputMultiCasesFullPaths.end(); ++sliceIt)
		{
			myManager.inputSingleCaseFullPath = sliceIt->second;
			myManager.imgEntry(sliceIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(sliceIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(sliceIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
			ImgProcessor::stepped_gammaCorrection(myManager.imgDatabase.at(sliceIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, this->gammaCutoff);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = QString::fromStdString(this->outputImgPath) + "\\" + QString::fromStdString(sliceIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
}

void ImgTester::mask2SWC()
{
	ImgManager myManager(QString::fromStdString(this->inputString));
	if (!QString::fromStdString(this->inputString).contains("."))
	{	
		for (multimap<string, string>::iterator sliceIt = myManager.inputMultiCasesFullPaths.begin(); sliceIt != myManager.inputMultiCasesFullPaths.end(); ++sliceIt)
		{
			myManager.inputSingleCaseFullPath = sliceIt->second;
			myManager.imgEntry(sliceIt->first, ImgManager::singleCase);
		}

		int sliceDims[3];
		sliceDims[0] = myManager.imgDatabase.begin()->second.dims[0];
		sliceDims[1] = myManager.imgDatabase.begin()->second.dims[1];
		sliceDims[2] = 1;

		vector<unsigned char**> slice2DVector;
		unsigned char* mipPtr = new unsigned char[sliceDims[0] * sliceDims[1]];
		for (int i = 0; i < sliceDims[0] * sliceDims[1]; ++i) mipPtr[i] = 0;
		for (map<string, registeredImg>::iterator sliceIt = myManager.imgDatabase.begin(); sliceIt != myManager.imgDatabase.end(); ++sliceIt)
		{
			ImgProcessor::imgMax(sliceIt->second.slicePtrs.begin()->second.get(), mipPtr, mipPtr, sliceDims);

			unsigned char** slice2DPtr = new unsigned char*[sliceDims[1]];
			for (int j = 0; j < sliceDims[1]; ++j)
			{
				slice2DPtr[j] = new unsigned char[sliceDims[0]];
				for (int i = 0; i < sliceDims[0]; ++i) slice2DPtr[j][i] = sliceIt->second.slicePtrs.begin()->second.get()[sliceDims[0] * j + i];
			}
			slice2DVector.push_back(slice2DPtr);
		}

		this->signalBlobs.clear();
		ImgAnalyzer myImgAnalyzer;
		myImgAnalyzer.reportProcess(ImgAnalyzer::blobMerging);
		unique_lock<mutex> progressLock(myImgAnalyzer.blobMergingMutex);
		ProcessMonitoringTester myMonitor;
		thread monitorThread(myMonitor, std::ref(myImgAnalyzer));
		this->signalBlobs = myImgAnalyzer.findSignalBlobs(slice2DVector, sliceDims, 3, mipPtr);
		monitorThread.join();

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
	}
}