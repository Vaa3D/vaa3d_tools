#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "NeuronStructExplorer.h"

using namespace std;

NeuronStructExplorer::NeuronStructExplorer(QString inputFileName)
{
	QStringList fileNameParse = inputFileName.split(".");
	if (fileNameParse.back() == "swc")
	{	
		this->neuronFileName = inputFileName;
		NeuronTree inputSWC = readSWC_file(inputFileName);
		this->singleTree = inputSWC;
		this->singleTreePtr = &(this->singleTree);
	}
}

NeuronStructExplorer::NeuronStructExplorer(string inputFileName)
{
	this->neuronFileName = QString::fromStdString(inputFileName);
	QStringList fileNameParse = this->neuronFileName.split(".");
	if (fileNameParse.back() == "swc")
	{
		NeuronTree inputSWC = readSWC_file(this->neuronFileName);
		this->singleTree = inputSWC;
		this->singleTreePtr = &(this->singleTree);
	}
}

void NeuronStructExplorer::detectedPixelStackZProfile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr)
{
	// -- This method creates a z-frequency profile on the xy plane for the image stack.

	for (QList<NeuronSWC>::iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		int xCoord = int(it->x);
		int yCoord = int(it->y);
		int zCoord = int(it->z);
		float prob = it->radius;
		
		string xyKey = to_string(xCoord) + "," + to_string(yCoord);
		this->zProfileMap[xyKey][zCoord] = prob;
	}
}

void NeuronStructExplorer::pixelStackZcleanup(unordered_map<string, unordered_map<int, float>> zProfileMap, NeuronTree* outputTreePtr, int minSecNum, bool max, int threshold)
{
	// -- Using the z-frequency profile created by this->zProfileMap, eliminate redundant nodes in the z direction that share the same x-y coordinates.
	// minSecNum: minimum number of consecutive nodes in z to be processed.
	// max: true  - Keep the nodes with the maximum probablity among the consecutive z nodes.
	//      false - threshold needs to be specified if max == false. Use the threshold instead. 

	outputTreePtr->listNeuron.clear();
	for (unordered_map<string, unordered_map<int, float>>::iterator it = zProfileMap.begin(); it != zProfileMap.end(); ++it)
	{
		QString xyCoordsQ = QString::fromStdString(it->first);
		QStringList xyCoordsParseQ = xyCoordsQ.split(",");
		int xCoord = xyCoordsParseQ[0].toInt();
		int yCoord = xyCoordsParseQ[1].toInt();

		if (it->second.size() < minSecNum)
		{
			for (unordered_map<int, float>::iterator zIt = it->second.begin(); zIt != it->second.end(); ++zIt)
			{
				NeuronSWC remainedNode;
				remainedNode.x = xCoord;
				remainedNode.y = yCoord;
				remainedNode.z = zIt->first;
				remainedNode.type = 2;
				remainedNode.radius = zIt->second;
				remainedNode.parent = -1;
				outputTreePtr->listNeuron.push_back(remainedNode);
			}
		}
		else
		{
			vector<int> zCoords;
			for (unordered_map<int, float>::iterator zIt = it->second.begin(); zIt != it->second.end(); ++zIt) zCoords.push_back(zIt->second);
			sort(zCoords.begin(), zCoords.end());
			int count = 1;
			float maxProb = 0;
			int maxProbZ = 0;
			for (vector<int>::iterator checkIt = zCoords.begin() + 1; checkIt != zCoords.end(); ++checkIt)
			{
				if (*checkIt = *(checkIt - 1) + 1)
				{
					++count;
					if (it->second[*checkIt] >= maxProb)
					{	
						maxProbZ = *checkIt;
						maxProb = it->second[*checkIt];
					}
				}
				else
				{
					NeuronSWC remainedNode;
					remainedNode.x = xCoord;
					remainedNode.y = yCoord;
					remainedNode.z = maxProbZ;
					remainedNode.type = 2;
					remainedNode.radius = maxProb;
					remainedNode.parent = -1;
					outputTreePtr->listNeuron.push_back(remainedNode);

					count = 1;
					maxProbZ = *checkIt;
					maxProb = it->second[*checkIt];
				}
			}
		}
	}
}

void NeuronStructExplorer::falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = detectedTreePtr->listNeuron.begin(); it != detectedTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = manualTreePtr->listNeuron.begin();
		while (checkIt != manualTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			if (thisNodeDist < distThreshold)
			{
				this->processedTree.listNeuron.push_back(*it);
				break;
			}

			++checkIt;
			if (checkIt == manualTreePtr->listNeuron.end())
			{
				vector<float> FPcoords;
				FPcoords.push_back(it->x);
				FPcoords.push_back(it->y);
				FPcoords.push_back(it->z);
				this->FPsList.push_back(FPcoords);
				FPcoords.clear();
			}
		}
	}
}

void NeuronStructExplorer::falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold)
{
	long int nodeCount = 1;
	for (QList<NeuronSWC>::iterator it = manualTreePtr->listNeuron.begin(); it != manualTreePtr->listNeuron.end(); ++it)
	{
		cout << "Scanning detected node " << nodeCount << ".. " << endl;
		++nodeCount;

		QList<NeuronSWC>::iterator checkIt = detectedTreePtr->listNeuron.begin();
		while (checkIt != detectedTreePtr->listNeuron.end())
		{
			float xSquare = (it->x - checkIt->x) * (it->x - checkIt->x);
			float ySquare = (it->y - checkIt->y) * (it->y - checkIt->y);
			float zSquare = (it->z - checkIt->z) * (it->z - checkIt->z);
			float thisNodeDist = sqrtf(xSquare + ySquare + zSquare);

			++checkIt;
			if (checkIt == detectedTreePtr->listNeuron.end())
			{
				vector<float> FNcoords;
				FNcoords.push_back(it->x);
				FNcoords.push_back(it->y);
				FNcoords.push_back(it->z);
				this->FNsList.push_back(FNcoords);
				FNcoords.clear();
			}
		}
	}
}

void NeuronStructExplorer::detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2)
{
	// -- Compute the shortest distance of each node from input structure (inputTreePtr1) to refernce structure (inputTreePtr2) 
	// -- The order of inputTreePtr1 and inputTreePtr2 represent the distances from opposite comparing directions.

	vector<float> minDists;
	long int count = 1;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		++count;
		//cout << "node No. " << count << ".." << endl;
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		minDists.push_back(minDist);
	}

	float distSum = 0;
	for (vector<float>::iterator it = minDists.begin(); it != minDists.end(); ++it) distSum = distSum + *it;
	float distMean = distSum / minDists.size();
	float varSum = 0;
	for (int i = 0; i < minDists.size(); ++i) varSum = varSum + (minDists[i] - distMean) * (minDists[i] - distMean);
	float distVar = varSum / minDists.size();
	float distStd = sqrtf(distVar);
	
	cout << distMean << " " << distStd << endl;
}