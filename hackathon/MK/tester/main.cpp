#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iterator>
#include <map>
#include <string>

#include <boost\filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include "SWCtester.h"
#include "imgTester.h"
#include "FeatureExtractor.h"
#include "integratedDataTypes.h"

#include "C:\Vaa3D_2013_Qt486\vaa3d_tools\released_plugins\v3d_plugins\sort_neuron_swc\sort_swc.h"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{
	/********* specify function *********/
	const char* funcNameC = argv[1];
	string funcName(funcNameC);
	
	vector<string> paras;
	for (int i = 2; i < argc; ++i)
	{
		const char* paraC = argv[i];
		string paraString(paraC);
		paras.push_back(paraString);
	}

	//string funcName = "downSampleTest";
	/************************************/

	ImgTester myImgTester;
	if (!funcName.compare("swcID"))
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
	else if (!funcName.compare("swcScale"))
	{
		QString inputSWCName = QString::fromStdString(paras.at(0));
		NeuronTree inputTree = readSWC_file(inputSWCName);
		NeuronTree outputTree = NeuronStructUtil::swcScale(inputTree, 2, 2, 1);
		QString outputName = QString::fromStdString(paras.at(1));
		writeSWC_file(outputName, outputTree);
	}
	else if (!funcName.compare("gammaTest"))
	{
		myImgTester.inputString = paras.at(0);
		myImgTester.outputString = paras.at(1);
		if (paras.at(2).compare("")) myImgTester.gammaCutoff = stoi(paras.at(2));
		myImgTester.gamma();
	}
	else if (!funcName.compare("inputSWCProperties"))
	{
		QString inputSWCName = QString::fromStdString(paras.at(0));
		NeuronTree inputTree = readSWC_file(inputSWCName);
		profiledTree inputProfiledTree(inputTree);

		cout << inputProfiledTree.segs.size() << endl;

	}
	else if (!funcName.compare("stackSlice"))
	{
		myImgTester.inputSingleImgFullName = paras.at(0);
		myImgTester.outputImgPath = paras.at(1);
		myImgTester.sliceImgStack();
	}
	else if (!funcName.compare("ada"))
	{
		myImgTester.inputString = paras.at(0);
		myImgTester.outputString = paras.at(1);
		if (paras.at(2).compare("")) myImgTester.cutOff = stoi(paras.at(2));
		myImgTester.ada();
	}
	else if (!funcName.compare("threshold_stats"))
	{
		myImgTester.inputString = paras.at(0);
		myImgTester.outputString = paras.at(1);
		myImgTester.thre_stats();
	}
	else if (!funcName.compare("simple_thre"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			//map<string, float> imgStats = ImgProcessor::getBasicStats_no0(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), myManager.imgDatabase.at(caseIt->first).dims);
			ImgProcessor::simpleThresh(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, 10);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("mask2SWC"))
	{
		myImgTester.inputString = paras.at(0);
		myImgTester.outputString = paras.at(1);
		myImgTester.mask2SWC();

		NeuronTree dendriteTree = NeuronStructUtil::blobs2tree(myImgTester.signalBlobs, true);
		QString blobTreeFullNameQ = QString::fromStdString(paras.at(1));
		writeSWC_file(blobTreeFullNameQ, dendriteTree);
	}
	else if (!funcName.compare("getCentroidTree"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree inputBlobTree = readSWC_file(inputSWCFullNameQ);
		NeuronStructUtil myUtil;
		ImgAnalyzer myAnalyzer;
		vector<connectedComponent> blobsComps = myUtil.swc2signal3DBlobs(inputBlobTree);
		NeuronTree finalCentroidTree;
		for (vector<connectedComponent>::iterator it = blobsComps.begin(); it != blobsComps.end(); ++it)
		{
			if (int(it - blobsComps.begin()) % 500 == 0) cout << int(it - blobsComps.begin()) << " ";
			NeuronTree centroidTree;
			boost::container::flat_set<deque<float>> sectionalCentroids = myAnalyzer.getSectionalCentroids(*it);
			for (boost::container::flat_set<deque<float>>::iterator nodeIt = sectionalCentroids.begin(); nodeIt != sectionalCentroids.end(); ++nodeIt)
			{
				NeuronSWC newNode;
				newNode.x = nodeIt->at(0);
				newNode.y = nodeIt->at(1);
				newNode.z = nodeIt->at(2);
				newNode.type = 2;
				newNode.parent = -1;
				centroidTree.listNeuron.push_back(newNode);
			}
			finalCentroidTree.listNeuron.append(centroidTree.listNeuron);
		}
		QString finalCentroidTreeNameQ = QString::fromStdString(paras.at(1));
		writeSWC_file(finalCentroidTreeNameQ, finalCentroidTree);
	}
	else if (!funcName.compare("centroidTree2MST"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree centroidTree = readSWC_file(inputSWCFullNameQ);
		NeuronStructExplorer myExplorer;
		NeuronTree MSTtree = myExplorer.SWC2MSTtree(centroidTree);
		QString saveNameQ = QString::fromStdString(paras.at(1));
		writeSWC_file(saveNameQ, MSTtree);
	}
	else if (!funcName.compare("downSampleTest"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree inputTree = readSWC_file(inputSWCFullNameQ);
		NeuronStructExplorer myExplorer;
		profiledTree testTree(inputTree);
		profiledTree outputTree = myExplorer.treeDownSample(testTree, 10);
		profiledTree finalTree = NeuronStructExplorer::spikeRemove(outputTree);
		writeSWC_file(QString::fromStdString(paras.at(1)), finalTree.tree);
	}
	else if (!funcName.compare("segPair"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		QString saveNameQ = QString::fromStdString(paras.at(1));
		NeuronTree inputTree = readSWC_file(inputSWCFullNameQ);
		
		NeuronStructExplorer myExplorer;
		profiledTree inputProfiledTree(inputTree);
		profiledTree testTree = myExplorer.connectLongNeurite(inputProfiledTree, 5);
		//writeSWC_file("H:\\fMOST_fragment_tracing\\testCase1\\connect1.swc", testTree.tree);
		myExplorer.getSegHeadTailClusters(inputProfiledTree);
		int clusterCount = 1;
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputProfiledTree.segTailClusters.begin(); it != inputProfiledTree.segTailClusters.end(); ++it)
		{
			for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(*inputProfiledTree.segs.at(*it2).tails.begin())].type = it->first % 9;
			}
			++clusterCount;
		}
		clusterCount = 1;
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = inputProfiledTree.segHeadClusters.begin(); it != inputProfiledTree.segHeadClusters.end(); ++it)
		{
			for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				inputProfiledTree.tree.listNeuron[inputProfiledTree.node2LocMap.at(inputProfiledTree.segs.at(*it2).head)].type = it->first % 9;
			}
			++clusterCount;
		}

		profiledTree terminals(inputProfiledTree.tree);
		NeuronTree terminalTree;
		for (QList<NeuronSWC>::iterator it = terminals.tree.listNeuron.begin(); it != terminals.tree.listNeuron.end(); ++it)
		{
			if (it->parent == -1 || terminals.node2childLocMap.find(it->n) == terminals.node2childLocMap.end())
			{
				NeuronSWC newNode = *it;
				newNode.parent = -1;
				terminalTree.listNeuron.push_back(newNode);
			}
		}
		writeSWC_file(saveNameQ, terminalTree);
	}
	else if (!funcName.compare("swc2mask"))
	{
		//QString inputSWCNameQ = QString::fromStdString(paras.at(2));
		QString inputSWCNameQ = "H:\\fMOST_fragment_tracing\\testCase1\\blob.swc";
		NeuronTree inputTree = readSWC_file(inputSWCNameQ);

		NeuronStructUtil util;
		vector<connectedComponent> blobs = util.swc2signal3DBlobs(inputTree);

		ImgAnalyzer myAnalyzer;
		int imgDims[3];
		imgDims[0] = 256;
		imgDims[1] = 256;
		imgDims[2] = 256;
		myImg1DPtr mask1Dptr = myAnalyzer.connectedComponentMask3D(blobs, imgDims);

		V3DLONG saveDims[4];
		saveDims[0] = imgDims[0];
		saveDims[1] = imgDims[1];
		saveDims[2] = imgDims[2];
		saveDims[3] = 1;
		QString saveFileNameQ = "H:\\fMOST_fragment_tracing\\testCase1\\testMask.tif";
		string saveFileName = saveFileNameQ.toStdString();
		const char* saveFileNameC = saveFileName.c_str();
		ImgManager::saveimage_wrapper(saveFileNameC, mask1Dptr.get(), saveDims, 1);
	}
	else if (!funcName.compare("selfDist"))
	{
		ofstream outputTest("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\test.txt");

		string inputSWCname1 = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\diffTree_zCleaned_MST_zRatio_branchBreak_noSpike_elong_noDotSeg2_tile30elong30_tile60elong30_RESULT\\319215569.swc";
		string inputSWCname2 = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\442_swcROIcropped\\319215569.swc";
		QString inputSWCnameQ1 = QString::fromStdString(inputSWCname1);
		QString inputSWCnameQ2 = QString::fromStdString(inputSWCname2);
		NeuronTree inputTree1 = readSWC_file(inputSWCnameQ1);
		NeuronTree inputTree2 = readSWC_file(inputSWCnameQ2);
		map<string, float> swcStats = NeuronStructUtil::selfNodeDist(inputTree1.listNeuron);
		
		outputTest << swcStats["mean"] << " " << swcStats["std"] << endl;
	}
	else if (!funcName.compare("accuracy"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		ofstream outputFile("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\accuracy.txt");
		outputFile << "case num\t" << "rate\t" << "node num" << endl;

		for (filesystem::directory_iterator swcIt(inputPathName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string swcName = swcIt->path().filename().string();
			string swcFullName = inputPathName + "\\" + swcName;
			QString swcFullNameQ = QString::fromStdString(swcFullName);
			NeuronTree nt = readSWC_file(swcFullNameQ);

			int TP = 0;
			int FP = 0;
			for (QList<NeuronSWC>::iterator it = nt.listNeuron.begin(); it != nt.listNeuron.end(); ++it)
			{
				if (it->type == 2) ++TP;
				else if (it->type == 3) ++FP;
			}
			double rate = double(TP) / double(TP + FP);

			string caseNum = swcName.substr(0, 9);
			outputFile << caseNum << "\t" << rate << "\t" << nt.listNeuron.size() << endl;
		}
	}
	else if (!funcName.compare("interAccuracy"))
	{
		const char* inputFolderNameC = argv[2];
		string inputFolderName(inputFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(inputFolderName);

		const char* savePathNameC = argv[3];
		string savePathName(savePathNameC);
		ofstream outputFile(savePathName);
		outputFile << "case num\t" << "avgDist 1-2\t" << "structure diff 1-2\t" << "avgDist 2-1\t" << "structure diff 2-1\t" << "avgDist all\t" << "structure diff all" << endl;

		for (filesystem::directory_iterator swcIt(inputFolderName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string csvName = swcIt->path().filename().string();
			string csvFullName = inputFolderName + "\\" + csvName;
			string caseName = csvName.substr(0, 9);
			//cout << csvFullName << endl;
			ifstream inputCSV(csvFullName);
			string inputLine;
			int count1_2 = 0;
			int count2_1 = 0;
			vector<string> values(7);
			values[0] = caseName;
			while (getline(inputCSV, inputLine))
			{
				if (inputLine.find("from neuron 1 to 2") != string::npos)
				{
					++count1_2;
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();			
					
					if (count1_2 == 1) values[1] = measure;
					else if (count1_2 == 2) values[2] = measure;
				}
				else if (inputLine.find("from neuron 2 to 1") != string::npos)
				{
					++count2_1;
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();

					if (count2_1 == 1) values[3] = measure;
					else if (count2_1 == 2) values[4] = measure;
				}
				else if (inputLine.find("structure-averages") != string::npos)
				{
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();
					values[5] = measure;
				}
				else if (inputLine.find("(average)") != string::npos)
				{
					vector<string> inputs;
					boost::split(inputs, inputLine, boost::is_any_of(" "));
					//for (vector<string>::iterator checkit = inputs.begin(); checkit != inputs.end(); ++checkit) cout << *checkit << " ";
					//cout << inputs.size() << " ";
					string measure = inputs.back();
					values[6] = measure;
				}
			}
			for (vector<string>::iterator it = values.begin(); it != values.end(); ++it) outputFile << *it << "\t";
			outputFile << endl;
			values.clear();
			inputLine.clear();
		}
	}
	else if (!funcName.compare("makeLinkerFile"))
	{
		const char* inputPathNameC = argv[1];
		string inputPathName(inputPathNameC);
		//ofstream outputFile("Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_centroids2D\\interAccuracy.txt");
		//outputFile << "case num\t" << "avgDist\t" << "structure diff" << endl;

		for (filesystem::directory_iterator swcIt(inputPathName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string swcName = swcIt->path().filename().string();
			string swcFullName = inputPathName + "\\" + swcName;
			NeuronStructUtil::linkerFileGen_forSWC(swcFullName);
		}
	}
	else if (!funcName.compare("somaPointsCluster"))
	{
		const char* inputFileNameC = argv[1];
		string inputFileName = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_somaCentroid3D\\472599722.swc";
		//string inputFileName(inputFileNameC);
		QString inputFileNameQ = QString::fromStdString(inputFileName);
		NeuronTree nt = readSWC_file(inputFileNameQ);

		vector<connectedComponent> compList = NeuronStructUtil::swc2clusters_distance(nt, 20);
		for (vector<connectedComponent>::iterator compIt = compList.begin(); compIt != compList.end(); ++compIt)
		{
			cout << "[";
			ImgAnalyzer::ChebyshevCenter_connComp(*compIt);
			for (map<int, set<vector<int>>>::iterator it = compIt->coordSets.begin(); it != compIt->coordSets.end(); ++it)
			{
				for (set<vector<int>>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				{
					cout << it2->at(0) << "_" << it2->at(1) << "_" << it2->at(2) << ", ";
				}
			}
			cout << "] ";
			cout << compIt->ChebyshevCenter[0] << " " << compIt->ChebyshevCenter[1] << " " << compIt->ChebyshevCenter[2] << endl;
		}
	}
	else if (!funcName.compare("multipleSomaCandidates"))
	{
		string inputFileName = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_somaCandidates\\";
		vector<string> fileList;
		for (filesystem::directory_iterator fileIt(inputFileName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileName = fileIt->path().string();
			QString fileNameQ = QString::fromStdString(fileName);
			NeuronTree nt = readSWC_file(fileNameQ);

			if (nt.listNeuron.size() == 2) fileList.push_back(fileName);
		}

		for (vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) cout << *it << endl;
	}
	else if (!funcName.compare("skeleton"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			ImgProcessor::skeleton2D(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
		}
	}
	else if (!funcName.compare("imgClose"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = myManager.imgDatabase.at(caseIt->first).dims[2];
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1] * imgDims[2]];
			morphStructElement2D structEle(morphStructElement2D::disk, 3);
			ImgProcessor::imgClose3D_sliceBySlice(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, structEle);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = imgDims[2];
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
		}
	}
	else if (!funcName.compare("imgOpen"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			morphStructElement2D structEle(morphStructElement2D::disk, 3);
			ImgProcessor::imgOpen2D(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, structEle);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
		}
	}
	else if (!funcName.compare("simpleBlend"))
	{
		const char* folder1C = argv[2];
		string folder1(folder1C);
		const char* folder2C = argv[3];
		string folder2(folder2C);

		for (filesystem::directory_iterator sliceIt(folder1); sliceIt != filesystem::directory_iterator(); ++sliceIt)
		{
			string caseName = sliceIt->path().filename().string();
			string caseNameFullPath = folder1 + "\\" + caseName;
			caseName = caseName.substr(0, 9);
			for (filesystem::directory_iterator sliceIt2(folder2); sliceIt2 != filesystem::directory_iterator(); ++sliceIt2)
			{
				string caseName2 = sliceIt2->path().filename().string();
				string caseName2FullPath = folder2 + "\\" + caseName2;
				caseName2 = caseName2.substr(0, 9);
				if (!caseName2.compare(caseName))
				{
					//cout << caseNameFullPath << endl << caseName2FullPath << endl;
					ImgManager myImgManager;
					myImgManager.inputSingleCaseFullPath = caseNameFullPath;
					myImgManager.imgEntry("img1", ImgManager::singleCase);
					myImgManager.inputSingleCaseFullPath = caseName2FullPath;
					myImgManager.imgEntry("img2", ImgManager::singleCase);

					vector<unsigned char*> blendingPtrs;
					for (map<string, registeredImg>::iterator it = myImgManager.imgDatabase.begin(); it != myImgManager.imgDatabase.end(); ++it)
						blendingPtrs.push_back(it->second.slicePtrs.begin()->second.get());

					unsigned char* imgArray1D = new unsigned char[myImgManager.imgDatabase.begin()->second.dims[0] * myImgManager.imgDatabase.begin()->second.dims[1] * 2];
					ImgManager::imgsBlend(blendingPtrs, imgArray1D, myImgManager.imgDatabase.begin()->second.dims);

					V3DLONG Dims[4];
					Dims[0] = myImgManager.imgDatabase.begin()->second.dims[0];
					Dims[1] = myImgManager.imgDatabase.begin()->second.dims[1];
					Dims[2] = 1;
					Dims[3] = 2;

					const char* saveNameC = argv[4];
					string saveName(saveNameC);
					saveName = saveName + "\\" + caseName2 + ".tif";
					const char* saveFullNameC = saveName.c_str();
					ImgManager::saveimage_wrapper(saveFullNameC, imgArray1D, Dims, 1);

					myImgManager.imgDatabase.clear();
				}
			}
		}
	}
	else if (!funcName.compare("getImgStats"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		ofstream outputFile("C:\\Users\\hsienchik\\Desktop\\Work\\boutonTest\\imgStats.txt");
		outputFile << "case num\tmean\tstd\tmedian" << endl;
		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			outputFile << caseIt->first << "\t";
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);
			map<string, float> imgStats = ImgProcessor::getBasicStats_no0(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), myManager.imgDatabase.at(caseIt->first).dims);
			outputFile << imgStats.at("mean") << "\t" << imgStats.at("std") << "\t" << imgStats.at("median") << endl;
		}
	}
	else if (!funcName.compare("swcSubtract"))
	{
		/*const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);

		const char* swcFolderNameC = argv[3];
		string swcFolderName(swcFolderNameC);
		QString swcFolderNameQ = QString::fromStdString(swcFolderName);

		const char* saveFolderNameC = argv[4];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);*/

		QString folderNameQ = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_BOTH\\combined_noiseRemoved_up5";
		QString swcFolderNameQ = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_BOTH\\1";
		QString saveFolderNameQ = "H:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_BOTH\\combined_noiseRemoved_up5_DENDRITE";

		ImgManager myManager(folderNameQ);
		myManager.outputRootPath = saveFolderNameQ;

		for (QStringList::iterator caseIt = myManager.caseList.begin(); caseIt != myManager.caseList.end(); ++caseIt)
		{
			NeuronTree nt1 = readSWC_file(folderNameQ + "\\" + *caseIt);
			NeuronTree nt2 = readSWC_file(swcFolderNameQ + "\\" + *caseIt);
			NeuronTree outputTree = NeuronStructUtil::swcSubtraction(nt1, nt2, 2);
		
			writeSWC_file(saveFolderNameQ + "\\" + *caseIt, outputTree);
		}
	}
	else if (!funcName.compare("swcUpSample"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		ImgManager myManager(folderNameQ);
		for (QStringList::iterator it = myManager.caseList.begin(); it != myManager.caseList.end(); ++it)
		{
			QString inputSWCfullName = folderNameQ + "\\" + *it;
			NeuronTree nt = readSWC_file(inputSWCfullName);
			profiledTree profiledNt(nt);
			profiledTree outputProfiledTree;
			NeuronStructExplorer::treeUpSample(profiledNt, outputProfiledTree);
			writeSWC_file(saveFolderNameQ + "\\" + *it, outputProfiledTree.tree);
		}
	}
	else if (!funcName.compare("swcCombine"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);

		const char* swcFolderNameC = argv[3];
		string swcFolderName(swcFolderNameC);
		QString swcFolderNameQ = QString::fromStdString(swcFolderName);

		const char* saveFolderNameC = argv[4];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		ImgManager myManager(folderNameQ);
		myManager.outputRootPath = saveFolderNameQ;

		for (QStringList::iterator caseIt = myManager.caseList.begin(); caseIt != myManager.caseList.end(); ++caseIt)
		{
			NeuronTree nt1 = readSWC_file(folderNameQ + "\\" + *caseIt);
			NeuronTree nt2 = readSWC_file(swcFolderNameQ + "\\" + *caseIt);
			vector<NeuronTree> treeVector(2);
			treeVector[0] = nt1;
			treeVector[1] = nt2;
			NeuronTree outputTree = NeuronStructUtil::swcCombine(treeVector);

			writeSWC_file(saveFolderNameQ + "\\" + *caseIt, outputTree);
		}
	}
	else if (!funcName.compare("getPixValue"))
	{
		QString inputSWCNameQ = "Z:\\IVSCC_mouse_inhibitory\\442_swcROIcropped_somaCandidates\\319215569.swc";
		NeuronTree inputTree = readSWC_file(inputSWCNameQ);
		profiledTree inputProfiledTree(inputTree);
		ImgManager myImgManager;
		myImgManager.inputSingleCaseFullPath = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\319215569_histEq.tif";
		myImgManager.imgEntry("inputImg", ImgManager::singleCase);

		for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
		{
			unsigned char value = ImgProcessor::getPixValue(myImgManager.imgDatabase.at("inputImg").slicePtrs.begin()->second.get(), myImgManager.imgDatabase.at("inputImg").dims, int(it->x), int(it->y));
			int valueInt = int(value);
			cout << valueInt << endl;
		}

		int inputX = int(inputTree.listNeuron.at(0).x);
		int inputY = int(inputTree.listNeuron.at(0).y);
		ImgAnalyzer myAnalyzer;
		set<vector<int>> whitePixSet = myAnalyzer.somaDendrite_radialDetect2D(myImgManager.imgDatabase.at("inputImg").slicePtrs.begin()->second.get(), inputX, inputY, myImgManager.imgDatabase.at("inputImg").dims);
		
		unsigned char* dendriteDetect2D = new unsigned char[myImgManager.imgDatabase.begin()->second.dims[0] * myImgManager.imgDatabase.begin()->second.dims[1]];
		for (size_t i = 0; i < myImgManager.imgDatabase.begin()->second.dims[0] * myImgManager.imgDatabase.begin()->second.dims[1]; ++i)
			dendriteDetect2D[i] = 0;

		for (set<vector<int>>::iterator coordIt = whitePixSet.begin(); coordIt != whitePixSet.end(); ++coordIt)
		{
			size_t index = size_t((coordIt->at(1) - 1) * myImgManager.imgDatabase.begin()->second.dims[0]) + size_t(coordIt->at(0));
			dendriteDetect2D[index] = 255;
		}

		V3DLONG saveDims[4];
		saveDims[0] = myImgManager.imgDatabase.begin()->second.dims[0];
		saveDims[1] = myImgManager.imgDatabase.begin()->second.dims[1];
		saveDims[2] = 1;
		saveDims[3] = 1;
		QString saveFileNameQ = "Z:\\IVSCC_mouse_inhibitory\\testOutput\\319215569_detected.tif";
		string saveFileName = saveFileNameQ.toStdString();
		const char* saveFileNameC = saveFileName.c_str();
		ImgManager::saveimage_wrapper(saveFileNameC, dendriteDetect2D, saveDims, 1);

		delete[] dendriteDetect2D;
		myImgManager.imgDatabase.clear();
	}
	else if (!funcName.compare("histEq"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			ImgProcessor::histEqual_unit8(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("imgErode"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		for (multimap<string, string>::iterator caseIt = myManager.inputMultiCasesFullPaths.begin(); caseIt != myManager.inputMultiCasesFullPaths.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = caseIt->second;
			myManager.imgEntry(caseIt->first, ImgManager::singleCase);

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(caseIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(caseIt->first).dims[1];
			imgDims[2] = 1;
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
			morphStructElement2D structEle("circle", 31);
			map<string, float> statsMap = ImgProcessor::getBasicStats_no0(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), imgDims);
			int threshold = int(statsMap.at("mean") - statsMap.at("std"));
			ImgProcessor::conditionalErode2D_imgStats(myManager.imgDatabase.at(caseIt->first).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, structEle, threshold);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = "C:\\Users\\hsienchik\\Desktop\\Work\\boutonTest\\adaThre_mean_conditional_ero31_mean-1std\\" + QString::fromStdString(caseIt->first) + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("imgThre_hist"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (QStringList::iterator caseIt = myManager.caseList.begin(); caseIt != myManager.caseList.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = (folderNameQ + *caseIt + ".tif").toStdString();
			myManager.imgEntry((*caseIt).toStdString(), ImgManager::singleCase);
			myManager.imgDatabase.at((*caseIt).toStdString()).getHistMap_no0_log10();

			double countLog10Min = 10;
			int threshold;
			for (map<int, double>::iterator threIt = myManager.imgDatabase.at((*caseIt).toStdString()).histMap_log10.begin(); threIt != myManager.imgDatabase.at((*caseIt).toStdString()).histMap_log10.end(); ++threIt)
			{
				if (threIt->second < countLog10Min)
				{
					countLog10Min = threIt->second;
					threshold = threIt->first;
				}
			}

			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at((*caseIt).toStdString()).dims[0];
			imgDims[1] = myManager.imgDatabase.at((*caseIt).toStdString()).dims[1];
			imgDims[2] = 1;
			
			unsigned char* outputImgPtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
			ImgProcessor::simpleThresh(myManager.imgDatabase.at((*caseIt).toStdString()).slicePtrs.begin()->second.get(), outputImgPtr, imgDims, threshold);

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			QString saveFileNameQ = saveFolderNameQ + "\\" + *caseIt + ".tif";
			string saveFileName = saveFileNameQ.toStdString();
			const char* saveFileNameC = saveFileName.c_str();
			ImgManager::saveimage_wrapper(saveFileNameC, outputImgPtr, saveDims, 1);

			delete[] outputImgPtr;
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("imgSig2SWC"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);
		ImgManager myManager(folderNameQ);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		for (QStringList::iterator caseIt = myManager.caseList.begin(); caseIt != myManager.caseList.end(); ++caseIt)
		{
			myManager.inputSingleCaseFullPath = (folderNameQ + *caseIt + ".tif").toStdString();
			myManager.imgEntry((*caseIt).toStdString(), ImgManager::singleCase);
			NeuronTree outputTree = ImgManager::imgSignal2SWC(myManager.imgDatabase.at((*caseIt).toStdString()), 3);
			QString saveName = saveFolderNameQ + "\\" + *caseIt + ".swc";
			writeSWC_file(saveName, outputTree);
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("getZ"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);

		const char* swcFolderNameC = argv[3];
		string swcFolderName(swcFolderNameC);
		QString swcFolderNameQ = QString::fromStdString(swcFolderName);

		const char* saveFolderNameC = argv[4];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		ImgManager myManager(folderNameQ);
		myManager.outputRootPath = saveFolderNameQ;

		for (QStringList::iterator caseIt = myManager.caseList.begin(); caseIt != myManager.caseList.end(); ++caseIt)
		{
			NeuronTree nt = readSWC_file(swcFolderNameQ + "\\" + *caseIt + ".swc");
			myManager.imgEntry((*caseIt).toStdString(), ImgManager::slices);
			ImgAnalyzer::findZ4swc_maxIntensity(nt.listNeuron, myManager.imgDatabase.at((*caseIt).toStdString()));

			writeSWC_file(saveFolderNameQ + "\\" + *caseIt + ".swc", nt);
			myManager.imgDatabase.clear();
		}
	}
	else if (!funcName.compare("stackSlice"))
	{
		const char* folderNameC = argv[2];
		string folderName(folderNameC);
		QString folderNameQ = QString::fromStdString(folderName);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		double duration;
		clock_t start = clock();
		ImgManager myManager;
		myManager.inputSingleCaseFullPath = "C:\\Users\\hsienchik\\Desktop\\Work\\boutonTest\\17302AS1Rd.tif";
		myManager.imgEntry("stack", ImgManager::singleCase);
		vector<vector<unsigned char>> imgSlices;
		int imgDims[3];
		imgDims[0] = 900;
		imgDims[1] = 900;
		imgDims[2] = 251;
		ImgProcessor::imgStackSlicer(myManager.imgDatabase.at("stack").slicePtrs.begin()->second.get(), imgSlices, imgDims);

		int sliceCount = 0;
		string saveSliceName;
		for (vector<vector<unsigned char>>::iterator sliceIt = imgSlices.begin(); sliceIt != imgSlices.end(); ++sliceIt)
		{
			++sliceCount;
			unsigned char* slicePtr = new unsigned char[imgDims[0] * imgDims[1]];
			for (size_t i = 0; i < imgDims[0] * imgDims[1]; ++i)
				slicePtr[i] = sliceIt->at(i);
			
			if (sliceCount / 10 == 0) saveSliceName = "00" + to_string(sliceCount) + ".tif";
			else if (sliceCount / 100 == 0) saveSliceName = "0" + to_string(sliceCount) + ".tif";
			else saveSliceName = to_string(sliceCount) + ".tif";

			string saveFullName = saveFolderName + "\\" + saveSliceName;
			const char* saveFullNameC = saveFullName.c_str();

			V3DLONG saveDims[4];
			saveDims[0] = imgDims[0];
			saveDims[1] = imgDims[1];
			saveDims[2] = 1;
			saveDims[3] = 1;
			ImgManager::saveimage_wrapper(saveFullNameC, slicePtr, saveDims, 1);

			delete[] slicePtr;
		}
		duration = (clock() - start) / double(CLOCKS_PER_SEC);
		cout << "time elapsed: " << duration << endl;
	}
	else if (!funcName.compare("swcSlice"))
	{
		const char* swcNameC = argv[2];
		string swcName(swcNameC);
		QString swcNameQ = QString::fromStdString(swcName);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		NeuronTree inputTree = readSWC_file(swcNameQ);
		vector<NeuronTree> outputTrees;
		NeuronStructUtil::swcSlicer(inputTree, outputTrees, 50);

		for (int i = 0; i < outputTrees.size(); ++i)
		{
			QString saveNameQ = saveFolderNameQ + "\\" + QString::fromStdString(to_string(i)) + ".swc";
			writeSWC_file(saveNameQ, outputTrees.at(i));
		}
	}
	else if (!funcName.compare("combineAllswc"))
	{
		const char* swcFolderNameC = argv[2];
		string swcFolderName(swcFolderNameC);
		QString swcFolderNameQ = QString::fromStdString(swcFolderName);

		const char* saveFolderNameC = argv[3];
		string saveFolderName(saveFolderNameC);
		QString saveFolderNameQ = QString::fromStdString(saveFolderName);

		vector<NeuronTree> trees;
		for (filesystem::directory_iterator swcIt(swcFolderName); swcIt != filesystem::directory_iterator(); ++swcIt)
		{
			string swcName = swcIt->path().filename().string();
			string swcFullName = swcIt->path().string();
			QString swcFullNameQ = QString::fromStdString(swcFullName);
			NeuronTree nt = readSWC_file(swcFullNameQ);
			trees.push_back(nt);
		}

		NeuronTree combinedTree = NeuronStructUtil::swcCombine(trees);
		QString saveFullNameQ = swcFolderNameQ + "\\combinedTree.swc";
		writeSWC_file(saveFullNameQ, combinedTree);
	}
	else if (!funcName.compare("swcCrop"))
	{
		const char* targetSWCnameC = argv[2];
		string targetSWCname(targetSWCnameC);
		QString targetSWCnameQ = QString::fromStdString(targetSWCname);

		const char* refSWCnameC = argv[3];
		string refSWCname(refSWCnameC);
		QString refSWCnameQ = QString::fromStdString(refSWCname);

		const char* manualCropNameC = argv[4];
		string manualCropName(manualCropNameC);
		QString manualCropNameQ = QString::fromStdString(manualCropName);

		NeuronTree refTree = readSWC_file(refSWCnameQ);
		NeuronTree targetTree = readSWC_file(targetSWCnameQ);
		vector<int> bounds = NeuronStructUtil::getSWCboundary(refTree);
		NeuronTree outputTree;
		cout << bounds.at(0) << " " << bounds.at(1) << " " << bounds.at(2) << " " << bounds.at(3) << " " << bounds.at(4) << " " << bounds.at(5) << endl;
		NeuronStructUtil::swcCrop(targetTree, outputTree, bounds.at(0), bounds.at(1), bounds.at(2), bounds.at(3), bounds.at(4), bounds.at(5));

		writeSWC_file(manualCropNameQ, outputTree);
	}
	else if (!funcName.compare("3Dhist"))
	{
		const char* inputFolderNameC = argv[2];
		string inputFolderName(inputFolderNameC);
		QString inputFolderNameQ = QString::fromStdString(inputFolderName);

		//const char* outputFolderNameC = argv[3];
		//string outputFolderName(outputFolderNameC);
		//QString outputFolerNameQ = QString::fromStdString(outputFolderName);

		//string outputFileFullName = outputFolderName + "\\histProfile.txt";
		//ofstream outputFile(outputFileFullName.c_str());

		map<int, size_t> hist3Dmap;
		ImgManager myManager(inputFolderNameQ);
		for (multimap<string, string>::iterator sliceIt = myManager.inputMultiCasesFullPaths.begin(); sliceIt != myManager.inputMultiCasesFullPaths.end(); ++sliceIt)
		{
			myManager.inputSingleCaseFullPath = sliceIt->second;
			myManager.imgEntry(sliceIt->first, ImgManager::singleCase);
			int imgDims[3];
			imgDims[0] = myManager.imgDatabase.at(sliceIt->first).dims[0];
			imgDims[1] = myManager.imgDatabase.at(sliceIt->first).dims[1];
			imgDims[2] = 1;
		
			map<int, size_t> binMap = ImgProcessor::histQuickList(myManager.imgDatabase.at(sliceIt->first).slicePtrs.begin()->second.get(), imgDims);
			for (map<int, size_t>::iterator binIt = binMap.begin(); binIt != binMap.end(); ++binIt)
			{
				if (hist3Dmap.find(binIt->first) == hist3Dmap.end()) hist3Dmap.insert(*binIt);
				else hist3Dmap[binIt->first] = hist3Dmap[binIt->first] + binIt->second;
			}
		}

		map<string, float> img3DStats = ImgProcessor::getBasicStats_no0_fromHist(hist3Dmap);
		cout << img3DStats.at("mean") << " " << img3DStats.at("std") << endl;
	}
	else if (!funcName.compare("nodeClusterTest"))
	{
		NeuronStructExplorer myExplorer;
		NeuronTree pre_sphereRandNodes = NeuronStructUtil::nodeSpheresGen(3, 1, 7, 7, 60, 30, 30, 120);
		NeuronTree sphereRandNodes = NeuronStructUtil::swcShift(pre_sphereRandNodes, 8, 5, 5);
		writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\sphereRandNode.swc", sphereRandNodes);
		profiledTree profiledSphereTree(sphereRandNodes, 10);

		myExplorer.getTileBasedSegClusters(profiledSphereTree, 6);
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator tailClusterIt = profiledSphereTree.segTailClusters.begin(); tailClusterIt != profiledSphereTree.segTailClusters.end(); ++tailClusterIt)
		{
			//cout << headClusterIt->first << " ";
			for (boost::container::flat_set<int>::iterator segIDit = tailClusterIt->second.begin(); segIDit != tailClusterIt->second.end(); ++segIDit)
			{
				//cout << profiledRandNodeTree.segs.at(*segIDit).head << " ";
				profiledSphereTree.tree.listNeuron[profiledSphereTree.node2LocMap.at(*profiledSphereTree.segs.at(*segIDit).tails.begin())].type = tailClusterIt->first % 7;
			}
		}
		
		writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\sphere.swc", profiledSphereTree.tree);

		int segTileCount = 1;
		/*cout << profiledSphereTree.segTailClusters.size() << endl;
		for (map<string, vector<int>>::iterator it = profiledSphereTree.segTailMap.begin(); it != profiledSphereTree.segTailMap.end(); ++it)
		{
			//cout << it->first << " " << it->second.size() << endl;
			//cout << " ";
			for (vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				//cout << profiledSphereTree.segs.at(*it2).head << " ";
				profiledSphereTree.tree.listNeuron[profiledSphereTree.node2LocMap.at(*profiledSphereTree.segs.at(*it2).tails.begin())].type = segTileCount % 20;
			}
			//cout << endl;
			++segTileCount;
		}
		writeSWC_file("D:\\Work\\FragTrace\\segTileTest1.swc", profiledSphereTree.tree);*/

		for (map<string, vector<int>>::iterator it = profiledSphereTree.segHeadMap.begin(); it != profiledSphereTree.segHeadMap.end(); ++it)
		{
			//cout << it->first << " " << it->second.size() << endl;
			//cout << " ";
			for (vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				//cout << profiledSphereTree.segs.at(*it2).head << " ";
				profiledSphereTree.tree.listNeuron[profiledSphereTree.node2LocMap.at(profiledSphereTree.segs.at(*it2).head)].type = segTileCount % 20;
			}
			//cout << endl;
			++segTileCount;
		}
		writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\segTileTest1.swc", profiledSphereTree.tree);

		myExplorer.mergeTileBasedSegClusters(profiledSphereTree, 6);
		int clusterCount = 1;
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = profiledSphereTree.segTailClusters.begin(); it != profiledSphereTree.segTailClusters.end(); ++it)
		{
			for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				profiledSphereTree.tree.listNeuron[profiledSphereTree.node2LocMap.at(*profiledSphereTree.segs.at(*it2).tails.begin())].type = clusterCount;
			}
			++clusterCount;
		}

		
		writeSWC_file("C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\segTileTest2.swc", profiledSphereTree.tree);
	}

	return 0;
}


