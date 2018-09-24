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
	/********* specify function *********/
	//const char* funcNameC = argv[1];
	//string funcName(funcNameC);
	string funcName = "segElongate";
	/************************************/

	if (!funcName.compare("2DblobMerge"))
	{
		//const char* inputSWCnameC = argv[2];
		//string inputSWCname(inputSWCnameC);
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory_442_swcLumps_2Dlabel\\478293723.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);

		SWCtester mySWCtester;
		vector<connectedComponent> outputConnCompList = mySWCtester.connComponent2DmergeTest(inputSWCnameQ);
	}
	else if (!funcName.compare("swcID"))
	{
		string refSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped\\319215569.swc";
		string subjSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped_centroids3D\\319215569.swc";
		QString refSWCnameQ = QString::fromStdString(refSWCname);
		QString subjSWCnameQ = QString::fromStdString(subjSWCname);
		NeuronTree refTree = readSWC_file(refSWCnameQ);
		NeuronTree subjTree = readSWC_file(subjSWCnameQ);
		NeuronTree outputTree = NeuronStructUtil::swcIdentityCompare(subjTree, refTree, 50, 20);
		QString outputSWCname = "H:\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("cleanUpZ"))
	{
		string inputSWCname = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_diffTree\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		NeuronTree outputTree = NeuronStructUtil::swcZclenUP(inputTree);
		QString outputSWCname = "H:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("swcDownSample"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_diffTree_zCleaned\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		NeuronTree outputTree;
		NeuronStructUtil::swcDownSample(inputTree, outputTree, 2, true);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\test.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("morphCheck"))
	{
		NeuronStructExplorer mySWCanalyzer;
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_zCleaned_MSTcut_zRatio\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree currTree(inputTree);
		/*vector<segUnit> segs = mySWCanalyzer.MSTtreeTrim(currTree.segs);
		NeuronTree outputTree;
		for (vector<segUnit>::iterator it = segs.begin(); it != segs.end(); ++it)
		{
			for (QList<NeuronSWC>::iterator nodeIt = it->nodes.begin(); nodeIt != it->nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\testMSTzCutMorph.swc";
		writeSWC_file(outputSWCname, outputTree);*/
	}
	else if (!funcName.compare("profiledTreeTest"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_zCleaned_MSTcut\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		
		profiledTree testTree(inputTree);
		NeuronTree outputTree;
		/*for (vector<segUnit>::iterator it = testTree.segs.begin(); it != testTree.segs.end(); ++it)
		{
			for (QList<NeuronSWC>::iterator nodeIt = it->nodes.begin(); nodeIt != it->nodes.end(); ++nodeIt)
				outputTree.listNeuron.push_back(*nodeIt);
		}
		
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\test2.swc";
		writeSWC_file(outputSWCname, outputTree);*/
	}
	else if (!funcName.compare("MSTtest"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_diffTree_zCleaned\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);

		NeuronStructExplorer mySWCanalyzer;
		NeuronTree outputTree = mySWCanalyzer.SWC2MSTtree(inputTree);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTz.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("MSTcut"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTz.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);

		NeuronTree outputTree = NeuronStructExplorer::MSTtreeCut(inputTree, 20);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput2\\testMSTzCut.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("branchBreak"))
	{
		string inputSWCname = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D_zCleaned_MST_zRatio\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);

		NeuronTree outputTree = NeuronStructExplorer::MSTbranchBreak(inputTree, true);
		QString outputSWCname = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\test1.swc";
		writeSWC_file(outputSWCname, outputTree);
	}
	else if (!funcName.compare("segElongate"))
	{
		string inputSWCname = "H:\\IVSCC_mouse_inhibitory\\testInput\\319215569.swc";
		QString inputSWCnameQ = QString::fromStdString(inputSWCname);
		NeuronTree inputTree = readSWC_file(inputSWCnameQ);
		profiledTree inputProfiledTree(inputTree);

		NeuronStructExplorer mySWCExplorer;
		profiledTree elongatedTree = mySWCExplorer.itered_segElongate(inputProfiledTree);

		QString outputSWCname = "H:\\IVSCC_mouse_inhibitory\\testOutput\\test.swc";
		writeSWC_file(outputSWCname, elongatedTree.tree);
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

	return 0;
}


