#include "FragTraceManager.h"

FragTraceManager::FragTraceManager(const Image4DSimple* inputImg4DSimplePtr, bool slices)
{
	int dims[3];
	dims[0] = inputImg4DSimplePtr->getXDim();
	dims[1] = inputImg4DSimplePtr->getYDim();
	dims[2] = inputImg4DSimplePtr->getZDim();
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
}

void FragTraceManager::imgProcPipe_wholeBlock()
{
	cout << "number of slices: " << this->fragTraceImgManager.imgDatabase.begin()->second.slicePtrs.size() << endl;
	V3DLONG dims[4];
	dims[0] = this->fragTraceImgManager.imgDatabase.begin()->second.dims[0];
	dims[1] = this->fragTraceImgManager.imgDatabase.begin()->second.dims[1];
	dims[2] = 1;
	dims[3] = 1;
	this->fragTraceImgManager.imgDatabase.begin()->second.dims[2] = 1;
	string saveRoot = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\testFolder\\";

	for (map<string, myImg1DPtr>::iterator sliceIt = this->fragTraceImgManager.imgDatabase.begin()->second.slicePtrs.begin(); sliceIt != this->fragTraceImgManager.imgDatabase.begin()->second.slicePtrs.end(); ++sliceIt)
	{
		//cout << sliceIt->first << endl;
		unsigned char* adaSlicePtr = new unsigned char[dims[0] * dims[1]];
		
		ImgProcessor::simpleAdaThre(sliceIt->second.get(), adaSlicePtr, this->fragTraceImgManager.imgDatabase.begin()->second.dims, 5, 3);

		string saveFullPath = saveRoot + sliceIt->first;
		const char* saveFullPathC = saveFullPath.c_str();
		this->fragTraceImgManager.saveimage_wrapper(saveFullPathC, adaSlicePtr, dims, 1);

		delete[] adaSlicePtr;
	}
	
}