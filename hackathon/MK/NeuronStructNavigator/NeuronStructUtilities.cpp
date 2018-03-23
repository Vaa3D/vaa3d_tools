#include <iostream>
#include <list>
#include <deque>
#include <string>
#include <iterator>
#include <ctime>

#include <boost\filesystem.hpp>

#include "basic_4dimage.h"
#include "NeuronStructUtilities.h"
#include "AnalysisExplorer.h"

using namespace std;
using namespace boost;

void swcSlicer(NeuronTree* inputTreePtr, vector<NeuronTree>* outputTreesPtr, int thickness)
{
	// -- Dissemble SWC files into "slices." Each outputSWC file represents only 1 z slice.

	QList<NeuronSWC> inputList = inputTreePtr->listNeuron;
	int zMax = 0;
	ptrdiff_t thicknessPtrDiff = ptrdiff_t(thickness); // Determining how many z sections to be included in 1 single slice.
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		it->z = round(it->z);
		if (it->z >= zMax) zMax = it->z;
	}

	QList<NeuronTree> slicedTrees; // Determining number of sliced trees in the list.
	for (int i = 0; i < zMax; ++i)
	{
		NeuronTree nt;
		slicedTrees.push_back(nt);
	}

	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		NeuronSWC currNode = *it;
		ptrdiff_t sliceNo = ptrdiff_t(it->z);
		(slicedTrees.begin() + sliceNo - 1)->listNeuron.push_back(currNode); // SWC starts with 1.
		float currZ = currNode.z;

		// -- Project +/- thickness slices onto the same plane, making sure the tube can be connected accross planes. -- //
		vector<ptrdiff_t> sectionNums; 
		for (ptrdiff_t ptri = 1; ptri <= thicknessPtrDiff; ++ptri)
		{
			ptrdiff_t minusDiff = sliceNo - ptri;
			ptrdiff_t plusDiff = sliceNo + ptri;

			if (minusDiff < 0) continue;
			else sectionNums.push_back(minusDiff);

			if (plusDiff > ptrdiff_t(zMax)) continue;
			else sectionNums.push_back(plusDiff);
		}
		for (vector<ptrdiff_t>::iterator ptrIt = sectionNums.begin(); ptrIt != sectionNums.end(); ++ptrIt)
		{
			//cout << "current node z:" << currNode.z << " " << *ptrIt << "| ";
			NeuronSWC newNode = currNode;
			newNode.z = float(*ptrIt);
			(slicedTrees.begin() + *ptrIt - 1)->listNeuron.push_back(newNode);
		}
		//cout << endl;

		sectionNums.clear();
		// ------------------------------------------------------------------------------------------------------------- //
	}

	for (QList<NeuronTree>::iterator it = slicedTrees.begin(); it != slicedTrees.end(); ++it)
		outputTreesPtr->push_back(*it);
}

void swcSliceAssembler(string swcPath)
{
	// This function only puts [SLICED SWC files] together without touching any topological structures.
	// z coordinates are adjusted based on the slice number specified in each SWC file name.

	NeuronTree outputTree;
	for (filesystem::directory_iterator itr(swcPath); itr != filesystem::directory_iterator(); ++itr)
	{
		string fileName = itr->path().filename().string();

		string fileExtCheck = fileName.substr(fileName.length() - 3, 3);
		if (fileExtCheck.compare("swc") != 0) continue;

		string sliceNumString = fileName.substr(1, 5);
		int sliceNum = stoi(sliceNumString);

		string currentSWCfullName = swcPath + "\\" + fileName;
		QString currentSWCfullNameQ = QString::fromStdString(currentSWCfullName);
		NeuronTree currentTree = readSWC_file(currentSWCfullNameQ);
		for (QList<NeuronSWC>::iterator nodeIt = currentTree.listNeuron.begin(); nodeIt != currentTree.listNeuron.end(); ++nodeIt)
		{
			nodeIt->z = sliceNum;
			outputTree.listNeuron.push_back(*nodeIt);
		}
	}

	QString outputFileName = QString::fromStdString(swcPath) + "\\assembledSWC.swc";
	writeSWC_file(outputFileName, outputTree);
}

void swcCrop(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float xlb, float xhb, float ylb, float yhb, float zlb, float zhb)
{
	if (zlb == 0 && zhb == 0)
	{
		for (QList<NeuronSWC>::iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
		{
			if (it->x < xlb || it->x > xhb || it->y < ylb || it->y > yhb) continue;
			else
			{
				NeuronSWC newNode;
				newNode.x = it->x - (xlb - 1);
				newNode.y = it->y - (ylb - 1);
				newNode.z = it->z;
				newNode.type = it->type;
				newNode.n = it->n;
				newNode.parent = it->parent;
				outputTreePtr->listNeuron.push_back(newNode);
			}
		}
	}
}

void swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength)
{
	float yMiddle = float(yLength + 1) / 2;
	for (QList<NeuronSWC>::const_iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		NeuronSWC flippedNode = *it;
		if (it->y > yMiddle) flippedNode.y = (yMiddle - (it->y - yMiddle));
		else if (it->y < yMiddle) flippedNode.y = (yMiddle + (yMiddle - it->y));
	}
}

void swcDownSampleZ(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int factor)
{
	// -- Downsample swc in z dimension with the given factor.

	QList<NeuronSWC> inputList = inputTreePtr->listNeuron;
	outputTreePtr->listNeuron.clear();
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
		if (int(it->z) % factor == 0) outputTreePtr->listNeuron.push_back(*it);
}

void detectedSWCprobFilter(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float threshold)
{
	// -- Filter detected nodes with the given threshold. 
	// -- Currently SWC dtat structure only has [radius] member that could be used as auto-detection output probability.
	
	QList<NeuronSWC> inputList = inputTreePtr->listNeuron;
	outputTreePtr->listNeuron.clear();
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		if (it->radius < threshold) continue;
		else outputTreePtr->listNeuron.push_back(*it);
	}
}

/* =================================== Volumetric SWC sampling methods =================================== */
void sigNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float ratio, float distance) 
{
	// -- Randomly generate signal patches within given distance range
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the radius allowed with SWC node centered

	cout << "target signal patch number: " << int(inputTreePtr->listNeuron.size() * ratio) << endl;
	int nodeCount = 0;
	for (QList<NeuronSWC>::iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		it->parent = -1;
		it->type = 2;
		outputTreePtr->listNeuron.push_back(*it);
		int foldCount = 2;
		while (foldCount <= ratio)
		{
			int randNumX = rand() % int(distance * 2) + int(it->x - distance);
			int randNumY = rand() % int(distance * 2) + int(it->y - distance);
			int randNumZ = rand() % int(distance * 2) + int(it->z - distance);

			++nodeCount;
			if (nodeCount % 10000 == 0) cout << nodeCount << " signal nodes generated." << endl;

			NeuronSWC newNode;
			newNode.x = randNumX;
			newNode.y = randNumY;
			newNode.z = randNumZ;
			newNode.type = 2;
			newNode.radius = 1;
			newNode.parent = -1;
			outputTreePtr->listNeuron.push_back(newNode);

			++foldCount;
		}
	}
}

void bkgNode_Gen(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, long int dims[], float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance
	// dims:     image stack dimension
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the forbidden distance from each SWC node

	QList<NeuronSWC> neuronList = inputTreePtr->listNeuron;
	int targetBkgNodeNum = int(neuronList.size() * ratio);
	cout << targetBkgNodeNum << " targeted bkg nodes to ge generated." << endl;
	int bkgNodeCount = 0;
	while (bkgNodeCount <= targetBkgNodeNum)
	{
		int randNumX = rand() % (dims[0] - 20) + 10;
		int randNumY = rand() % (dims[1] - 20) + 10;
		int randNumZ = rand() % dims[2];

		bool flag = false;
		for (QList<NeuronSWC>::iterator it = neuronList.begin(); it != neuronList.end(); ++it)
		{
			float distSqr;
			float diffx = float(randNumX) - it->x;
			float diffy = float(randNumY) - it->y;
			float diffz = float(randNumZ) - it->z;
			distSqr = diffx * diffx + diffy * diffy + diffz * diffz;

			if (distSqr <= distance * distance)
			{
				flag = true;
				break;
			}
		}

		if (flag == false)
		{
			++bkgNodeCount;
			if (bkgNodeCount % 10000 == 0) cout << bkgNodeCount << " bkg nodes generated." << endl;

			NeuronSWC newBkgNode;
			newBkgNode.x = randNumX;
			newBkgNode.y = randNumY;
			newBkgNode.z = randNumZ;
			newBkgNode.type = 3;
			newBkgNode.radius = 1;
			newBkgNode.parent = -1;
			outputTreePtr->listNeuron.push_back(newBkgNode);
		}
	}
}

void bkgNode_Gen_somaArea(NeuronTree* intputTreePtr, NeuronTree* outputTreePtr, int xLength, int yLength, int zLength, float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance. This method aims to reinforce the background recognition near soma area.
	// xLength, yLength, zLength: decide the range to apply with soma centered
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the forbidden distance from each SWC node

	NeuronSWC somaNode;
	for (QList<NeuronSWC>::iterator it = intputTreePtr->listNeuron.begin(); it != intputTreePtr->listNeuron.end(); ++it)
	{
		if (it->parent == -1)
		{
			somaNode = *it;
			break;
		}
	}
	float xlb = somaNode.x - float(xLength);
	float xhb = somaNode.x + float(xLength);
	float ylb = somaNode.y - float(yLength);
	float yhb = somaNode.y + float(yLength);
	float zlb = somaNode.z - float(zLength);
	float zhb = somaNode.z + float(zLength);

	list<NeuronSWC> confinedNodes;
	for (QList<NeuronSWC>::iterator it = intputTreePtr->listNeuron.begin(); it != intputTreePtr->listNeuron.end(); ++it)
		if (xlb <= it->x && xhb >= it->x && ylb <= it->y && yhb >= it->y && zlb <= it->z && zhb >= it->z) confinedNodes.push_back(*it);
	
	int targetBkgNodeNum = int(float(xLength) * float(yLength) * float(zLength) * ratio);
	cout << targetBkgNodeNum << " targeted bkg nodes to ge generated." << endl;
	int bkgNodeCount = 0;
	while (bkgNodeCount <= targetBkgNodeNum)
	{
		int randNumX = rand() % int(xhb - xlb + 1) + int(xlb);
		int randNumY = rand() % int(yhb - ylb + 1) + int(ylb);
		int randNumZ = rand() % int(zhb - zlb + 1) + int(zlb);

		bool flag = false;
		for (list<NeuronSWC>::iterator it = confinedNodes.begin(); it != confinedNodes.end(); ++it)
		{
			float distSqr;
			float diffx = float(randNumX) - it->x;
			float diffy = float(randNumY) - it->y;
			float diffz = float(randNumZ) - it->z;
			distSqr = diffx * diffx + diffy * diffy + diffz * diffz;

			if (distSqr <= distance * distance)
			{
				flag = true;
				break;
			}
		}

		if (flag == false)
		{
			++bkgNodeCount;
			if (bkgNodeCount % 10000 == 0) cout << bkgNodeCount << " bkg nodes generated within the soma area." << endl;

			NeuronSWC newBkgNode;
			newBkgNode.x = randNumX;
			newBkgNode.y = randNumY;
			newBkgNode.z = randNumZ;
			newBkgNode.type = 3;
			newBkgNode.radius = 1;
			newBkgNode.parent = -1;
			outputTreePtr->listNeuron.push_back(newBkgNode);
		}
	}
}
/* =================================== Volumetric SWC sampling methods =================================== */