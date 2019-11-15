#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <thread>
#include <iterator>
#include <map>
#include <string>
#include <omp.h>

#include <boost\filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include "SWCtester.h"
#include "imgTester.h"
#include "processMonitoringTester.h"
#include "FeatureExtractor.h"
#include "integratedDataTypes.h"
#include "TreeGrower.h"

#include "C:\Vaa3D_2013_Qt486\vaa3d_tools\released_plugins\v3d_plugins\sort_neuron_swc\sort_swc.h"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{
	/********* specify function *********/
	//const char* funcNameC = argv[1];
	//string funcName(funcNameC);
	
	vector<string> paras;
	/*for (int i = 2; i < argc; ++i)
	{
		const char* paraC = argv[i];
		string paraString(paraC);
		paras.push_back(paraString);
	}*/

	string funcName = "spikeRemove_spikeRoots";
	/************************************/

	ImgTester myImgTester;
	NeuronStructUtil myNeuronStructUtil;
	if (!funcName.compare("swcID"))
	{
		string refSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped\\319215569.swc";
		string subjSWCname = "H:\\IVSCC_mouse_inhibitory_442_swcROIcropped_centroids3D\\319215569.swc";
		QString refSWCnameQ = QString::fromStdString(refSWCname);
		QString subjSWCnameQ = QString::fromStdString(subjSWCname);
		NeuronTree refTree = readSWC_file(refSWCnameQ);
		NeuronTree subjTree = readSWC_file(subjSWCnameQ);
		NeuronTree outputTree = NeuronStructExplorer::swcIdentityCompare(subjTree, refTree, 50, 20);
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
	else if (!funcName.compare("swc_typeFilter"))
	{
		QString inputSWCName = QString::fromStdString(paras.at(0));
		NeuronTree inputTree = readSWC_file(inputSWCName);
		map<int, QList<NeuronSWC>> swcTypeMap = NeuronStructUtil::swcSplitByType(inputTree);
		NeuronTree outputTree;
		outputTree.listNeuron = swcTypeMap.at(stoi(paras.at(2)));
		writeSWC_file(QString::fromStdString(paras.at(1)), outputTree);
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
	else if (!funcName.compare("spikeRemove_spikeRoots"))
	{
		//QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		QString inputSWCFullNameQ = "C:\\Users\\hsienchik\\Desktop\\spikeTest\\beforeSpikeRemove.swc";
		NeuronTree beforeSpikeRemoveTree = readSWC_file(inputSWCFullNameQ);
		profiledTree spikedProfiledTree(beforeSpikeRemoveTree);
		profiledTree spikeRemovedProfiledTree = TreeGrower::itered_spikeRemoval(spikedProfiledTree);
		//QString outputSWCfullName = QString::fromStdString(paras.at(1));
		QString outputSWCfullName = "C:\\Users\\hsienchik\\Desktop\\spikeTest\\afterSpikeRemove.swc";
		writeSWC_file(outputSWCfullName, spikeRemovedProfiledTree.tree);

		cout << " -- spike root number:" << spikeRemovedProfiledTree.spikeRootIDs.size() << endl;
		for (boost::container::flat_set<int>::iterator it = spikeRemovedProfiledTree.spikeRootIDs.begin(); it != spikeRemovedProfiledTree.spikeRootIDs.end(); ++it)
		{
			if (spikeRemovedProfiledTree.node2LocMap.find(*it) != spikeRemovedProfiledTree.node2LocMap.end() && spikeRemovedProfiledTree.node2childLocMap.find(*it) != spikeRemovedProfiledTree.node2childLocMap.end())
			{
				cout << *it << ": ";
				if (spikeRemovedProfiledTree.tree.listNeuron.at(spikeRemovedProfiledTree.node2LocMap.at(*it)).parent != -1 && spikeRemovedProfiledTree.node2childLocMap.at(*it).size() == 1)
				{
					cout << spikeRemovedProfiledTree.tree.listNeuron.at(spikeRemovedProfiledTree.node2LocMap.at(*it)).parent << " " << *spikeRemovedProfiledTree.node2childLocMap.at(*it).begin();
					NeuronSWC angularNode = spikeRemovedProfiledTree.tree.listNeuron.at(spikeRemovedProfiledTree.node2LocMap.at(*it));
					NeuronSWC endNode1 = spikeRemovedProfiledTree.tree.listNeuron.at(spikeRemovedProfiledTree.node2LocMap.at(angularNode.parent));
					NeuronSWC endNode2 = spikeRemovedProfiledTree.tree.listNeuron.at(*spikeRemovedProfiledTree.node2childLocMap.at(*it).begin());

					float angle = NeuronGeoGrapher::get3nodesFormingAngle<float>(angularNode, endNode1, endNode2);
					if (angle <= 0.5)
					{
						spikeRemovedProfiledTree.tree.listNeuron[spikeRemovedProfiledTree.node2LocMap.at(*it)].x = (endNode1.x + endNode2.x) / 2;
						spikeRemovedProfiledTree.tree.listNeuron[spikeRemovedProfiledTree.node2LocMap.at(*it)].y = (endNode1.y + endNode2.y) / 2;
						spikeRemovedProfiledTree.tree.listNeuron[spikeRemovedProfiledTree.node2LocMap.at(*it)].z = (endNode1.z + endNode2.z) / 2;
					}
				}
				cout << endl;
			}
		}
		profiledTreeReInit(spikeRemovedProfiledTree);
		
		//QString outputSWCStrightenfullName = QString::fromStdString(paras.at(2));
		QString spikeRootStrightFullName = "C:\\Users\\hsienchik\\Desktop\\spikeTest\\spikeStright.swc";
		writeSWC_file(spikeRootStrightFullName, spikeRemovedProfiledTree.tree);
	}
	else if (!funcName.compare("centroidTree2MST"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree centroidTree = readSWC_file(inputSWCFullNameQ);
		NeuronTree MSTtree = TreeGrower::SWC2MSTtree_boost(centroidTree);
		QString saveNameQ = QString::fromStdString(paras.at(1));
		writeSWC_file(saveNameQ, MSTtree);
	}
	else if (!funcName.compare("downSampleTest"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree inputTree = readSWC_file(inputSWCFullNameQ);
		profiledTree testTree(inputTree);
		profiledTree outputTree = NeuronStructUtil::treeDownSample(testTree, 10);
		profiledTree finalTree = TreeGrower::spikeRemoval(outputTree);
		writeSWC_file(QString::fromStdString(paras.at(1)), finalTree.tree);
	}
	else if (!funcName.compare("polarTest"))
	{
		float originX = stof(paras.at(0));
		float originY = stof(paras.at(1));
		float originZ = stof(paras.at(2));
		float radius = stof(paras.at(3));
		float density = stof(paras.at(4));
		vector<float> originVec = { originX, originY, originZ };

		NeuronTree sphere = NeuronStructUtil::sphereRandNodes(radius, originX, originY, originZ, density);
		SWCtester mySWCtester;
		QList<NeuronSWC> outputList = mySWCtester.polarCoordShellPeeling(sphere.listNeuron, originVec, radius);
		NeuronTree outputTree;
		outputTree.listNeuron = outputList;

		writeSWC_file(QString::fromStdString(paras.at(5)), outputTree);
	}
	else if (!funcName.compare("polarTest_angleHor"))
	{
		float originX = stof(paras.at(0));
		float originY = stof(paras.at(1));
		float originZ = stof(paras.at(2));
		float radius = stof(paras.at(3));
		float density = stof(paras.at(4));
		vector<float> originVec = { originX, originY, originZ };

		NeuronTree sphere = NeuronStructUtil::sphereRandNodes(radius, originX, originY, originZ, density);
		SWCtester mySWCtester;
		QList<NeuronSWC> outputList = mySWCtester.polarCoordAngle_horizontal(sphere.listNeuron, originVec, radius);
		NeuronTree outputTree;
		outputTree.listNeuron = outputList;

		writeSWC_file(QString::fromStdString(paras.at(5)), outputTree);
	}
	else if (!funcName.compare("polarTest_angleVer"))
	{
		float originX = stof(paras.at(0));
		float originY = stof(paras.at(1));
		float originZ = stof(paras.at(2));
		float radius = stof(paras.at(3));
		float density = stof(paras.at(4));
		vector<float> originVec = { originX, originY, originZ };

		NeuronTree sphere = NeuronStructUtil::sphereRandNodes(radius, originX, originY, originZ, density);
		SWCtester mySWCtester;
		QList<NeuronSWC> outputList = mySWCtester.polarCoordAngle_vertical(sphere.listNeuron, originVec, radius);
		NeuronTree outputTree;
		outputTree.listNeuron = outputList;

		writeSWC_file(QString::fromStdString(paras.at(5)), outputTree);
	}
	else if (!funcName.compare("mask2SWC"))
	{
		myImgTester.inputString = paras.at(0);
		myImgTester.outputString = paras.at(1);
		myImgTester.progressPercentage = 101;
		//ProcessMonitoringTester myMonitor;
		//thread monitorThread(myMonitor, std::ref(myImgTester));
		myImgTester.mask2SWC();
		//monitorThread.join();
		
		NeuronTree dendriteTree = NeuronStructUtil::blobs2tree(myImgTester.signalBlobs, true);
		QString blobTreeFullNameQ = QString::fromStdString(paras.at(1));
		writeSWC_file(blobTreeFullNameQ, dendriteTree);
	}
	else if (!funcName.compare("polarRadiusShell"))
	{	
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		//QString inputSWCFullNameQ = "C:\\Users\\hsienchik\\Desktop\\blob_dendrite.swc";
		NeuronTree inputBlobTree = readSWC_file(inputSWCFullNameQ);
		vector<polarNeuronSWC> polarNodeList;
		vector<int> origin = { 128, 128, 128 };
		NeuronGeoGrapher::nodeList2polarNodeList(inputBlobTree.listNeuron, polarNodeList, origin);
		boost::container::flat_map<double, boost::container::flat_set<int>> shellRadiusMap = NeuronGeoGrapher::getShellByRadius_loc(polarNodeList);

		map<double, NeuronTree> radius2NeuronTreeMap;
		for (boost::container::flat_map<double, boost::container::flat_set<int>>::iterator it = shellRadiusMap.begin(); it != shellRadiusMap.end(); ++it)
		{
			NeuronTree currShellTree;
			for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				NeuronSWC newNode = NeuronGeoGrapher::polar2CartesianNode(polarNodeList.at(*it2));
				currShellTree.listNeuron.push_back(newNode);
			}

			radius2NeuronTreeMap.insert({ it->first, currShellTree });
		}

		map<double, vector<connectedComponent>> radius2ConnMap;
		NeuronTree finalTree;
		NeuronTree finalCentroidTree;
		for (map<double, NeuronTree>::iterator it = radius2NeuronTreeMap.begin(); it != radius2NeuronTreeMap.end(); ++it)
		{
			vector<connectedComponent> currConnCompList = NeuronStructUtil::swc2signal3DBlobs(it->second);
			radius2ConnMap.insert({ it->first, currConnCompList });

			NeuronTree currBlobTree = NeuronStructUtil::blobs2tree(currConnCompList);
			for (QList<NeuronSWC>::iterator it3 = currBlobTree.listNeuron.begin(); it3 != currBlobTree.listNeuron.end(); ++it3) it3->type = int(it->first);
			finalTree.listNeuron.append(currBlobTree.listNeuron);

			for (vector<connectedComponent>::iterator it2 = currConnCompList.begin(); it2 != currConnCompList.end(); ++it2)
			{
				ChebyshevCenter_connComp(*it2);
				NeuronSWC newNode;
				newNode.x = it2->ChebyshevCenter[0];
				newNode.y = it2->ChebyshevCenter[1];
				newNode.z = it2->ChebyshevCenter[2];
				newNode.type = int(it->first);
				newNode.parent = -1;
				finalCentroidTree.listNeuron.push_back(newNode);
			}
		}
	
		writeSWC_file(QString::fromStdString(paras.at(1)), finalCentroidTree);
	}
	else if (!funcName.compare("centroidShellTest"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		//QString inputSWCFullNameQ = "C:\\Users\\hsienchik\\Desktop\\blob_dendrite.swc";
		NeuronTree inputBlobTree = readSWC_file(inputSWCFullNameQ);
		vector<polarNeuronSWC> polarNodeList;
		vector<int> origin = { 68, 64, 130 };
		NeuronGeoGrapher::nodeList2polarNodeList(inputBlobTree.listNeuron, polarNodeList, origin);
		boost::container::flat_map<double, boost::container::flat_set<int>> shellRadiusMap = NeuronGeoGrapher::getShellByRadius_loc(polarNodeList);
		map<double, NeuronTree> radius2NeuronTreeMap;

		NeuronTree outputTree;
		for (boost::container::flat_map<double, boost::container::flat_set<int>>::iterator it = shellRadiusMap.begin(); it != shellRadiusMap.end(); ++it)
		{
			NeuronTree currShellTree;
			for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				NeuronSWC newNode = NeuronGeoGrapher::polar2CartesianNode(polarNodeList.at(*it2));
				newNode.type = it->first;
				currShellTree.listNeuron.push_back(newNode);

				outputTree.listNeuron.push_back(newNode);
			}

			radius2NeuronTreeMap.insert({ it->first, currShellTree });
		}

		writeSWC_file(QString::fromStdString(paras.at(1)), outputTree);
	}
	else if (!funcName.compare("denTree"))
	{
		//QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		QString inputSWCFullNameQ = "C:\\Users\\hsienchik\\Desktop\\blob_dendrite.swc";
		NeuronTree inputBlobTree = readSWC_file(inputSWCFullNameQ);
		vector<polarNeuronSWC> polarNodeList;
		vector<int> origin = { 68, 64, 130 };
		NeuronGeoGrapher::nodeList2polarNodeList(inputBlobTree.listNeuron, polarNodeList, origin);
		boost::container::flat_map<double, boost::container::flat_set<int>> shellRadiusMap = NeuronGeoGrapher::getShellByRadius_loc(polarNodeList);

		TreeGrower myTreeGrower;
		myTreeGrower.polarNodeList = polarNodeList;
		myTreeGrower.radiusShellMap_loc = shellRadiusMap;
		myTreeGrower.dendriticTree_shellCentroid();
		
		/*for (boost::container::flat_map<double, vector<connectedComponent>>::iterator it = myTreeGrower.radius2shellConnCompMap.begin(); it != myTreeGrower.radius2shellConnCompMap.end(); ++it)
		{
			NeuronTree outputTree;
			int connCount = 0;
			for (vector<connectedComponent>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				for (map<int, set<vector<int>>>::iterator it3 = it2->coordSets.begin(); it3 != it2->coordSets.end(); ++it3)
				{
					for (set<vector<int>>::iterator it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
					{
						NeuronSWC newNode;
						newNode.x = it4->at(0);
						newNode.y = it4->at(1);
						newNode.z = it4->at(2);
						newNode.type = connCount;
						newNode.parent = -1;
						outputTree.listNeuron.push_back(newNode);
					}
				}
				++connCount;
			}
			
			QString saveName = QString::fromStdString(to_string(it->first)) + ".swc";
			QString saveFullName = "C:\\Users\\hsienchik\\Desktop\\shell\\" + saveName;
			writeSWC_file(saveName, outputTree);

			outputTree.listNeuron.clear();
		}*/

		//writeSWC_file(QString::fromStdString(paras.at(1)), myTreeGrower.treeDataBase.at("dendriticProfiledTree").tree);
		QString saveName = "C:\\Users\\hsienchik\\Desktop\\denDebug.swc";
		writeSWC_file(saveName, myTreeGrower.treeDataBase.at("dendriticProfiledTree").tree);
	}
	else if (!funcName.compare("MSTrelatedTest"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree inputCentroidTree = readSWC_file(inputSWCFullNameQ);
		profiledTree profiledMSTtree(inputCentroidTree);

		NeuronTree tiledMSTTree = TreeGrower::SWC2MSTtreeTiled_boost(inputCentroidTree, 32, 8);

		//profiledTree MSTdownSampledTree = myNeuronStructExplorer.treeDownSample(profiledMSTtree, 10);
		//profiledTree MSTdownSampledNoSpikeTree = NeuronStructExplorer::spikeRemove(MSTdownSampledTree);
		//profiledTree MSTDnNoSpikeBranchBreak = NeuronStructExplorer::MSTbranchBreak(MSTdownSampledNoSpikeTree);

		//profiledTree somaHollowedTree = NeuronStructExplorer::treeHollow(MSTDnNoSpikeBranchBreak, 64, 64, 128, 5);

		writeSWC_file(QString::fromStdString(paras.at(1)), tiledMSTTree);
	}
	else if (!funcName.compare("getCentroidTree"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		NeuronTree inputBlobTree = readSWC_file(inputSWCFullNameQ);
		NeuronStructUtil myUtil;
		ImgAnalyzer myAnalyzer;
		vector<connectedComponent> blobsComps = NeuronStructUtil::swc2signal3DBlobs(inputBlobTree);
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
	else if (!funcName.compare("segPair"))
	{
		QString inputSWCFullNameQ = QString::fromStdString(paras.at(0));
		QString saveNameQ = QString::fromStdString(paras.at(1));
		NeuronTree inputTree = readSWC_file(inputSWCFullNameQ);

		TreeGrower newGrowingTree;
		profiledTree inputProfiledTree(inputTree);
		profiledTree testTree = newGrowingTree.itered_connectSegsWithinClusters(inputProfiledTree, 5);
		writeSWC_file("C:\\Users\\hsienchik\\Desktop\\connect.swc", testTree.tree);
		newGrowingTree.getSegHeadTailClusters(inputProfiledTree);
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
	else if (!funcName.compare("generateBrainRegionSWC_f"))
	{
		ifstream inputFile("C:\\Users\\hsienchik\\Desktop\\CCF\\idValue2regionName.txt");
		string line;
		string buffer;
		vector<string> lineSplit;
		map<float, string> idValue2nameMap;
		boost::container::flat_set<float> valueSet;
		map<float, int> nonRecogNum;
		map<float, int> idValue2assignedMap;
		int assignedI = 0;
		if (inputFile.is_open())
		{
			while (getline(inputFile, line))
			{
				stringstream ss(line);
				while (ss >> buffer) lineSplit.push_back(buffer);

				++assignedI;
				int typeValue = assignedI % 20;
				idValue2assignedMap.insert({ stof(lineSplit.at(0)), typeValue });
				idValue2nameMap.insert({ stof(lineSplit.at(0)), lineSplit.at(1) });
				//cout << lineSplit.at(1) << " " << lineSplit.at(2) << " " << lineSplit.at(3) << " " << lineSplit.at(4) << endl;
				valueSet.insert(stof(lineSplit.at(0)));

				lineSplit.clear();
			}
		}

		map<string, NeuronTree> name2treeMap;
		map<string, float> name2idValueMap;
		for (map<float, string>::iterator it = idValue2nameMap.begin(); it != idValue2nameMap.end(); ++it)
		{
			NeuronTree newTree;
			name2treeMap.insert({ it->second, newTree });
			name2idValueMap.insert({ it->second, it->first });
			cout << it->second << " " << it->first << endl;
		}

		ImgManager myManager;
		int imgDims[3];
		myManager.inputSingleCaseFullPath = "C:\\Users\\hsienchik\\Desktop\\CCF\\annotation_25.v3draw";
		myManager.imgEntry("CCFstack", ImgManager::singleCase);
		cout << myManager.imgDatabase.at("CCFstack").dataType << endl;
		imgDims[0] = myManager.imgDatabase.at("CCFstack").dims[0];
		imgDims[1] = myManager.imgDatabase.at("CCFstack").dims[1];
		imgDims[2] = myManager.imgDatabase.at("CCFstack").dims[2];
		cout << imgDims[0] << " " << imgDims[1] << " " << imgDims[2] << endl;

		boost::container::flat_set<float> nonRecogValues;
		map<float, NeuronTree> nonRecogValueTrees;
		map<float, int> nonRecogValue2typeMap;
		map<int, int> slice0map;
		int nonRecgType = 0;
		for (int zi = 1; zi <= imgDims[2]; ++zi)
		{
			cout << zi << " ";
			slice0map.insert({ zi, 0 });
			for (int yi = 1; yi <= imgDims[1]; ++yi)
			{
				for (int xi = 1; xi <= imgDims[0]; ++xi)
				{
					float value = ImgProcessor::getPixValue(myManager.imgDatabase.at("CCFstack").floatSlicePtrs.begin()->second.get(), imgDims, xi, yi, zi);
					//int intValue = int(floor(value));

					if (value == 0)
					{
						slice0map.at(zi) += 1;
					}
					else
					{
						string regionName = idValue2nameMap.at(value);
						NeuronSWC newNode;
						newNode.x = xi - 1;
						newNode.y = yi - 1;
						newNode.z = zi - 1;
						newNode.parent = -1;
						newNode.type = idValue2assignedMap.at(value);
						name2treeMap.at(regionName).listNeuron.push_back(newNode);
					}
				}
			}
		}

		QString savingRoot = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regions\\";
		for (map<string, NeuronTree>::iterator it = name2treeMap.begin(); it != name2treeMap.end(); ++it)
		{
			QString saveFullName = savingRoot + QString::fromStdString(it->first) + "-" + QString::fromStdString(to_string(name2idValueMap.at(it->first))) + ".swc";
			qDebug() << saveFullName;
			writeSWC_file(saveFullName, it->second);
		}

		for (map<float, int>::iterator it = nonRecogNum.begin(); it != nonRecogNum.end(); ++it) cout << it->first << " " << it->second << endl;
		//QString savingRoot = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regions\\";
		for (map<float, NeuronTree>::iterator it = nonRecogValueTrees.begin(); it != nonRecogValueTrees.end(); ++it)
		{
			QString saveFullName = savingRoot + QString::fromStdString(to_string(it->first)) + ".swc";
			qDebug() << saveFullName;
			writeSWC_file(saveFullName, it->second);
		}

		for (map<int, int>::iterator it = slice0map.begin(); it != slice0map.end(); ++it)
			cout << it->first << " " << it->second << endl;
		cout << endl;
	}
	else if (!funcName.compare("swcSurfs"))
	{
		string inputFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regions";
		QString saveFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces\\";
		//string destFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\empty_brain_regions\\";
		for (filesystem::directory_iterator fileIt(inputFolderName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileName = fileIt->path().string();
			string swcName = fileIt->path().filename().string();
			cout << fileName << " " << swcName << endl;
			QString fileNameQ = QString::fromStdString(fileName);
			NeuronTree nt = readSWC_file(fileNameQ);

			NeuronTree outputTree;
			vector<connectedComponent> compList = NeuronStructUtil::swc2signal3DBlobs(nt);
			for (vector<connectedComponent>::iterator it = compList.begin(); it != compList.end(); ++it)
			{
				it->getConnCompSurface();
				for (boost::container::flat_map<int, boost::container::flat_set<vector<int>>>::iterator it1 = it->surfaceCoordSets.begin(); it1 != it->surfaceCoordSets.end(); ++it1)
				{
					for (boost::container::flat_set<vector<int>>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
					{
						NeuronSWC newNode;
						newNode.x = it2->at(0);
						newNode.y = it2->at(1);
						newNode.z = it2->at(2);
						newNode.parent = -1;
						newNode.type = nt.listNeuron.begin()->type;
						outputTree.listNeuron.push_back(newNode);
					}
				}
			}

			QString outputFullName = saveFolderName + QString::fromStdString(swcName);
			qDebug() << outputFullName;
			writeSWC_file(outputFullName, outputTree);
		}
	}
	else if (!funcName.compare("rename"))
	{
		string inputFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces\\";
		//string destFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\empty_brain_regions\\";
		for (filesystem::directory_iterator fileIt(inputFolderName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileName = fileIt->path().string();
			string swcName = fileIt->path().filename().string();
			vector<string> inputs;
			boost::split(inputs, swcName, boost::is_any_of("-"));
			string newFileName;

			if (inputs.size() == 2) newFileName = inputs.at(0);
			else
			{
				for (vector<string>::iterator it = inputs.begin(); it != inputs.end() - 2; ++it) newFileName = *it + "-";
				newFileName = newFileName + *(inputs.end() - 2);
			}
			newFileName = newFileName + ".swc";
			string destName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces\\";
			destName = destName + newFileName;
			cout << destName << endl;
			filesystem::rename(fileName, destName);
		}
	}
	else if (!funcName.compare("surfReadTest"))
	{
		clock_t startTime = clock();
		string inputFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces";
		QString saveFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces\\";
		//string destFolderName = "C:\\Users\\hsienchik\\Desktop\\CCF\\empty_brain_regions\\";
		ofstream outputFile("C:\\Users\\hsienchik\\Desktop\\CCF\\regionBoundaries.txt");
		for (filesystem::directory_iterator fileIt(inputFolderName); fileIt != filesystem::directory_iterator(); ++fileIt)
		{
			string fileName = fileIt->path().string();
			string swcName = fileIt->path().filename().string();
			cout << fileName << " " << swcName << endl;
			QString fileNameQ = QString::fromStdString(fileName);
			NeuronTree nt = readSWC_file(fileNameQ);
			vector<string> inputs;
			boost::split(inputs, swcName, boost::is_any_of("."));

			brainRegion thisRegion;
			thisRegion.name = inputs.at(0);
			thisRegion.regionBodies = NeuronStructUtil::swc2signal3DBlobs(nt);

			outputFile << thisRegion.name << "\t";
			for (vector<connectedComponent>::iterator it = thisRegion.regionBodies.begin(); it != thisRegion.regionBodies.end(); ++it)
				outputFile << it->xMin << "\t" << it->xMax << "\t" << it->yMin << "\t" << it->yMax << "\t" << it->zMin << "\t" << it->zMax << "\t";
			outputFile << endl;
		}
		double duration = (clock() - startTime) / double(CLOCKS_PER_SEC);
		cout << "time elapsed: " << duration << " sec" << endl;
	}
	else if (!funcName.compare("allBrgsReadingTime"))
	{
		string inputPath = "C:\\Users\\King Mars\\Desktop\\CCF\\brgs";
		clock_t start = clock();
		for (filesystem::directory_iterator it(inputPath); it != filesystem::directory_iterator(); ++it)
		{
			string fileFullName = it->path().string();
			cout << fileFullName << endl;
			brainRegion region;
			region.readBrainRegion_file(fileFullName);
		}
		double duration = (clock() - start) / double(CLOCKS_PER_SEC);
		cout << duration << " secs" << endl;
		cout << double(13) / double(18) * duration << endl;
	}
	else if (!funcName.compare("regionReadTest"))
	{
		clock_t startTime = clock();
		ifstream inputFile("C:\\Users\\King Mars\\Desktop\\CCF\\regionBoundaries.txt");

		QString inputSWCName = "C:\\Users\\King Mars\\Desktop\\CCF\\17302_00110_s_affine_jba.swc";
		NeuronTree inputTree = readSWC_file(inputSWCName);
		vector<int> swcBounds = NeuronStructUtil::getSWCboundary<int>(inputTree);
		string line;
		string buffer;
		vector<string> lineSplit;
		map<string, vector<vector<int>>> regionBoundsMap;
		if (inputFile.is_open())
		{
			while (getline(inputFile, line))
			{
				stringstream ss(line);
				while (ss >> buffer) lineSplit.push_back(buffer);

				string name = lineSplit.at(0);
				vector<vector<int>> boundaries;
				int count = 0;
				vector<int> currBounds;
				for (vector<string>::iterator it = lineSplit.begin() + 1; it != lineSplit.end(); ++it)
				{
					currBounds.push_back(stoi(*it));
					++count;
					if (count % 6 == 0)
					{
						boundaries.push_back(currBounds);
						count = 0;
						currBounds.clear();
					}
				}

				//for (vector<int>::iterator it1 = swcBounds.begin(); it1 != swcBounds.end(); ++it1) cout << *it1 << " ";
				//cout << endl;
				//cout << lineSplit.at(0) << ": ";
				for (vector<vector<int>>::iterator it = boundaries.begin(); it != boundaries.end(); ++it)
				{
					//cout << it->at(0) << " " << it->at(1) << " " << it->at(2) << " " << it->at(3) << " " << it->at(4) << " " << it->at(5) << endl;

					if (swcBounds.at(0) < it->at(1) && swcBounds.at(1) > it->at(0) &&
						swcBounds.at(2) < it->at(3) && swcBounds.at(3) > it->at(2) &&
						swcBounds.at(4) < it->at(5) && swcBounds.at(5) > it->at(4))
					{
						regionBoundsMap.insert({ lineSplit.at(0), boundaries });
						break;
					}
					else continue;
				}
				//cout << endl;
				//for (vector<string>::iterator it = lineSplit.begin(); it != lineSplit.end(); ++it) cout << *it << " ";
				//cout << endl;
				lineSplit.clear();
				boundaries.clear();
			}
		}
		for (map<string, vector<vector<int>>>::iterator it = regionBoundsMap.begin(); it != regionBoundsMap.end(); ++it) cout << it->first << " ";
		cout << endl;
		cout << "candidate regions number: " << regionBoundsMap.size() << endl;
		//std::system("pause");

		set<string> regions;
		vector<brainRegion> regionList;
		string inputRegionFolder = "C:\\Users\\King Mars\\Desktop\\CCF\\brgs\\";

		for (map<string, vector<vector<int>>>::iterator it = regionBoundsMap.begin(); it != regionBoundsMap.end(); ++it)
		{
			string inputRegionFullName = inputRegionFolder + it->first + ".brg";
			cout << it->first << endl;
			brainRegion region;
			region.name = it->first;
			region.readBrainRegion_file(inputRegionFullName);
			for (vector<connectedComponent>::iterator compIt = region.regionBodies.begin(); compIt != region.regionBodies.end(); ++compIt)
			{
				//cout << compIt->xMax << " " << compIt->xMin << " " << compIt->yMax << " " << compIt->yMin << " " << compIt->zMax << " " << compIt->zMin << endl;
				//std::system("pause");

				for (QList<NeuronSWC>::iterator nodeIt = inputTree.listNeuron.begin(); nodeIt < inputTree.listNeuron.end() - 10; nodeIt = nodeIt + 10)
				{
					int nodeX = int(nodeIt->x);
					int nodeY = int(nodeIt->y);
					int nodeZ = int(nodeIt->z);
					//cout << nodeX << " " << nodeY << " " << nodeZ << endl;
					if (nodeX > compIt->xMin && nodeX < compIt->xMax &&
						nodeY > compIt->yMin && nodeY < compIt->yMax &&
						nodeZ > compIt->zMin && nodeZ < compIt->zMax)
					{
						vector<int> xyVec = { int(nodeIt->x), int(nodeIt->y) };
						vector<int> xzVec = { int(nodeIt->x), int(nodeIt->z) };
						vector<int> yzVec = { int(nodeIt->y), int(nodeIt->z) };

						if (compIt->xyProjection.find(xyVec) != compIt->xyProjection.end() && compIt->xzProjection.find(xzVec) != compIt->xzProjection.end() && compIt->yzProjection.find(yzVec) != compIt->yzProjection.end())
						{
							//cout << " " << nodeX << " " << nodeY << " " << nodeZ << endl;
							//regions.insert(it->first);
							regionList.push_back(region);
							goto REGION_FOUND;
						}
					}
				}
			}

		REGION_FOUND:
			continue;
		}
		cout << "--" << endl;


		QList<NeuronSWC> copiedList = inputTree.listNeuron;
		for (vector<brainRegion>::iterator it = regionList.begin(); it != regionList.end(); ++it)
		{
			cout << it->name << ": ";
			for (vector<connectedComponent>::iterator compIt = it->regionBodies.begin(); compIt != it->regionBodies.end(); ++compIt)
			{
				for (QList<NeuronSWC>::iterator nodeIt = copiedList.begin(); nodeIt < copiedList.end() - 10; nodeIt = nodeIt + 10)
				{
					int nodeZ = int(nodeIt->z);
					if (compIt->surfaceCoordSets.find(nodeZ) != compIt->surfaceCoordSets.end())
					{
						int nodeX = int(nodeIt->x);
						int nodeY = int(nodeIt->y);			
						vector<int> sliceBound = getSliceBoundaries(compIt->surfaceCoordSets.at(nodeZ));
						//cout << sliceBound.at(0) << " " << sliceBound.at(1) << " " << sliceBound.at(2) << " " << sliceBound.at(3) << endl;
						
						int test = 0;
						for (int stepi = nodeY; stepi <= 320; ++stepi)
						{
							vector<int> stepVec = { nodeX, stepi, nodeZ };
							if (compIt->surfaceCoordSets.at(nodeZ).find(stepVec) != compIt->surfaceCoordSets.at(nodeZ).end())
							{
								//cout << "Group " << int(compIt - it->regionBodies.begin()) + 1 << " " << compIt->xMin << " " << compIt->xMax << " " << compIt->yMin << " " << compIt->yMax << endl;
								//cout << "   " << nodeX << " " << nodeY << " " << nodeZ << endl;
								++test;
								break;
							}
						}

						for (int stepi = nodeY; stepi >= 1; --stepi)
						{
							vector<int> stepVec = { nodeX, stepi, nodeZ };
							if (compIt->surfaceCoordSets.at(nodeZ).find(stepVec) != compIt->surfaceCoordSets.at(nodeZ).end())
							{
								//cout << "Group " << int(compIt - it->regionBodies.begin()) + 1 << " " << compIt->xMin << " " << compIt->xMax << " " << compIt->yMin << " " << compIt->yMax << endl;
								//cout << "   " << nodeX << " " << nodeY << " " << nodeZ << endl;
								++test;
								break;
							}
						}

						if (test == 2)
						{
							regions.insert(it->name);
							copiedList.erase(nodeIt);
							goto FINAL_REGION;
						}
					}
				}
			}

		FINAL_REGION:
			cout << endl;
			continue;
		}
		
		for (set<string>::iterator it = regions.begin(); it != regions.end(); ++it) cout << *it << " ";
		cout << endl;

		double duration = (clock() - startTime) / double(CLOCKS_PER_SEC);
		cout << "time elapsed: " << duration << " sec" << endl;
	}
	else if (!funcName.compare("brgFileTest"))
	{
		QString inputFile = "C:\\Users\\King Mars\\Desktop\\CCF\\brain_regionSurfaces\\GPe.swc";
		NeuronTree nt = readSWC_file(inputFile);
		vector<connectedComponent> regionCompList = NeuronStructUtil::swc2signal3DBlobs(nt);
		brainRegion GPe;
		GPe.name = "GPe";
		GPe.regionBodies = regionCompList;
		
		string outputFileName = "C:\\Users\\King Mars\\Desktop\\CCF\\GPe.brg";
		GPe.writeBrainRegion_file(outputFileName);
	}
	else if (!funcName.compare("brgFileReadTest"))
	{
		string inputFileName = "C:\\Users\\King Mars\\Desktop\\CCF\\GPe.brg";
		brainRegion GPe;
		GPe.readBrainRegion_file(inputFileName);
		cout << GPe.name << endl;
		cout << GPe.regionBodies.size() << " " << GPe.regionBodies.at(0).surfaceCoordSets.size() << " " << GPe.regionBodies.at(1).surfaceCoordSets.size() << endl;
	}
	else if (!funcName.compare("brgs"))
	{
		string inputFolder = "C:\\Users\\hsienchik\\Desktop\\CCF\\brain_regionSurfaces";
		string saveFolder = "C:\\Users\\hsienchik\\Desktop\\CCF\\brgs\\";
		for (filesystem::directory_iterator it(inputFolder); it != filesystem::directory_iterator(); ++it)
		{
			string fileFullName = it->path().string();
			string fileName = it->path().filename().string();
			vector<string> nameSplit;
			boost::split(nameSplit, fileName, boost::is_any_of("."));
			string regionName = nameSplit.at(0);
			QString inputSWCName = QString::fromStdString(fileFullName);
			string saveFullName = saveFolder + regionName + ".brg";

			NeuronTree inputTree = readSWC_file(inputSWCName);
			brainRegion thisRegion;
			thisRegion.name = regionName;
			thisRegion.regionBodies = NeuronStructUtil::swc2signal3DBlobs(inputTree);
			thisRegion.writeBrainRegion_file(saveFullName);
		}
	}
	else if (!funcName.compare("surfTest"))
	{
		QString inputFile = "C:\\Users\\hsienchik\\Desktop\\CCF\\AHN-88.000000.swc";
		NeuronTree inputTree = readSWC_file(inputFile);

		vector<connectedComponent> compList = NeuronStructUtil::swc2signal3DBlobs(inputTree);
		cout << "component number: " << compList.size() << endl;

		NeuronTree surfTree;
		for (vector<connectedComponent>::iterator it = compList.begin(); it != compList.end(); ++it)
		{
			it->getConnCompSurface();
			for (boost::container::flat_map<int, boost::container::flat_set<vector<int>>>::iterator it1 = it->surfaceCoordSets.begin(); it1 != it->surfaceCoordSets.end(); ++it1)
			{
				for (boost::container::flat_set<vector<int>>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
				{
					NeuronSWC newNode;
					newNode.x = it2->at(0);
					newNode.y = it2->at(1);
					newNode.z = it2->at(2);
					newNode.parent = -1;
					newNode.type = inputTree.listNeuron.begin()->type;
					surfTree.listNeuron.push_back(newNode);
				}
			}
		}
		
		QString saveFileName = "C:\\Users\\hsienchik\\Desktop\\CCF\\AHN-88.000000_surf.swc";
		writeSWC_file(saveFileName, surfTree);
	}
	// ---------------------------------------------------------------------------------------------------------------------------------------- //
	else if (!funcName.compare("swc2mask"))
	{
		//QString inputSWCNameQ = QString::fromStdString(paras.at(2));
		QString inputSWCNameQ = "H:\\fMOST_fragment_tracing\\testCase1\\blob.swc";
		NeuronTree inputTree = readSWC_file(inputSWCNameQ);

		NeuronStructUtil util;
		vector<connectedComponent> blobs = NeuronStructUtil::swc2signal3DBlobs(inputTree);

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
		map<string, float> swcStats = NeuronStructExplorer::selfNodeDist(inputTree1.listNeuron);
		
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
			NeuronStructUtil::treeUpSample(profiledNt, outputProfiledTree);
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
		vector<int> bounds = NeuronStructUtil::getSWCboundary<int>(refTree);
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

	return 0;
}


