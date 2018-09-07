#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iterator>

#include <boost\filesystem.hpp>

#include "SWCtester.h"
#include "ImgManager.h"
#include "ImgProcessor.h"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{
	//const char* funcNameC = argv[1];
	//string funcName(funcNameC);
	string funcName = "2DblobMerge";

	if (!funcName.compare("2DblobMerge"))
	{
		//const char* inputSWCnameC = argv[2];
		//string inputSWCname(inputSWCnameC);
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory_442_swcLumps_2Dlabel\\478293723.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);

		SWCtester mySWCtester;
		vector<connectedComponent> outputConnCompList = mySWCtester.connComponent2DmergeTest(inputSWCnameQ);
	}


	//float sum = 0;
	//size_t sliceI = 0;
	//int dims[3];
	//for (filesystem::directory_iterator sliceIt(originalRoot); sliceIt != filesystem::directory_iterator(); ++sliceIt)
	//{
	//	++sliceI;
	//	string sliceName = sliceIt->path().filename().string();
	//	string sliceFullName = originalRoot + "\\" + sliceName;
	//	const char* sliceFullNameC = sliceFullName.c_str();
	//	Image4DSimple* tiffPtr = new Image4DSimple;
	//	tiffPtr->loadImage(sliceFullNameC);
	//	int dims[3];
	//	dims[0] = int(tiffPtr->getXDim());
	//	dims[1] = int(tiffPtr->getYDim());
	//	dims[2] = 1;
	//	long int totalbyteTiff = tiffPtr->getTotalBytes();
	//	unsigned char* tiffSlice1D = new unsigned char[totalbyteTiff];
	//	memcpy(tiffSlice1D, tiffPtr->getRawData(), totalbyteTiff);
	//	
	//	string prefix = sliceName.substr(0, 5);
	//	string adaSliceName = prefix + ".tif";
	//	string adaSliceFullName = adaRoot + "\\" + adaSliceName;
	//	const char* adaSliceFullNameC = adaSliceFullName.c_str();
	//	Image4DSimple* adaTiffPtr = new Image4DSimple;
	//	adaTiffPtr->loadImage(adaSliceFullNameC);
	//	long int totalbyteAdaTiff = adaTiffPtr->getTotalBytes();
	//	unsigned char* adaTiffSlice1D = new unsigned char[totalbyteAdaTiff];
	//	memcpy(adaTiffSlice1D, adaTiffPtr->getRawData(), totalbyteAdaTiff);
	//	
	//	unsigned char* proc1 = new unsigned char[dims[0] * dims[1]];
	//	map<int, size_t> histMap = ImgProcessor::histQuickList(adaTiffSlice1D, dims);
	//	map<int, size_t> histMapOri = ImgProcessor::histQuickList(tiffSlice1D, dims);
	//	//for (map<int, size_t>::iterator it = histMap.begin(); it != histMap.end(); ++it) cout << it->first << " " << it->second << endl;
	//	//cout << endl;
	//	map<string, float>basicStats = ImgProcessor::getBasicStats_no0(adaTiffSlice1D, dims);
	//	int gammaStart = 0;
	//	int topBracket = 0;
	//	float pixCount = histMapOri[0];
	//	for (int histI = 1; histI < histMap.size(); ++histI)
	//	{
	//		if (histMap[histI] <= histMap[histI - 1] && histMap[histI] <= histMap[histI + 1])
	//		{
	//			gammaStart = histI;
	//			break;
	//		}
	//	}
	//	for (int bracketI = 1; bracketI < 255; ++bracketI)
	//	{
	//		pixCount = pixCount + float(histMapOri[bracketI]);
	//		if (pixCount / float(dims[0] * dims[1]) >= 0.999)
	//		{
	//			topBracket = bracketI;
	//			break;
	//		}
	//	}
	//	cout << topBracket << " ";

	//	/*int maxNegDiff = 0;
	//	int cutoffValue;
	//	for (int diffI = 1; diffI <= 255; ++diffI)
	//	{
	//		if (int(histMap[diffI]) - int(histMap[diffI - 1]) <= maxNegDiff)
	//		{
	//			cutoffValue = diffI;
	//			maxNegDiff = int(histMap[diffI]) - int(histMap[diffI - 1]);

	//		}
	//	}*/
	//	//ImgProcessor::gammaCorrect(adaTiffSlice1D, proc1, dims, gammaStart);
	//	//cout << gammaStart << " ";

	//	unsigned char* proc2 = new unsigned char[dims[0] * dims[1]];
	//	ImgProcessor::simpleThresh(tiffSlice1D, proc2, dims, topBracket);

	//	unsigned char* proc3 = new unsigned char[dims[0] * dims[1]];
	//	ImgProcessor::imageMax(proc1, proc2, proc3, dims);

	//	//cout << basicStats["mean"] << " " << basicStats["mean"] + basicStats["std"] << endl;
	//	//for (size_t i = 0; i < dims[0] * dims[1]; ++i)
	//	//{
	//	//	sum = sum + tiffSlice1D[i];
	//	//	if (tiffSlice1D[i] <= basicStats["mean"]) adath[i] = 0;
	//	//	else adath[i] = tiffSlice1D[i];
	//	//}
	//	V3DLONG Dims[4];
	//	Dims[0] = dims[0];
	//	Dims[1] = dims[1];
	//	Dims[2] = 1;
	//	Dims[3] = 1;
	//	string saveFullName = saveRoot + "\\" + sliceName;
	//	//cout << saveFullName << endl;
	//	const char* saveFullNameC = saveFullName.c_str();
	//	ImgManager::saveimage_wrapper(saveFullNameC, proc3, Dims, 1);

	//	tiffPtr->~Image4DSimple();
	//	operator delete(tiffPtr);
	//	adaTiffPtr->~Image4DSimple();
	//	operator delete(adaTiffPtr);

	//	if (tiffSlice1D) { delete[] tiffSlice1D; tiffSlice1D = 0; }
	//	if (adaTiffSlice1D) { delete[] adaTiffSlice1D; adaTiffSlice1D = 0; }
	//	if (proc1) { delete[] proc1; proc1 = 0; }
	//	if (proc2) { delete[] proc2; proc2 = 0; }
	//	if (proc3) { delete[] proc2; proc3 = 0; }
	//}
	//float mean = sum / float(dims[0] * dims[1] * sliceI);

	return 0;
}


