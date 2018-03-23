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

void NeuronStructExplorer::swcXYprofile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr)
{
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

void NeuronStructExplorer::swcZcleanup(unordered_map<string, unordered_map<int, float>> zProfileMap, NeuronTree* outputTreePtr, int minSecNum, bool max, int threshold)
{
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

void NeuronStructExplorer::swcDetectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2)
{
	vector<float> minDists;
	long int count = 1;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		++count;
		cout << "node No. " << count << ".." << endl;
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