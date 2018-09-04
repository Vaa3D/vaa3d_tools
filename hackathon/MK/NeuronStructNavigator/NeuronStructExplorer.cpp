#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"

using namespace std;

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

NeuronTree NeuronStructExplorer::SWC2MSTtree(NeuronTree const& inputTreePtr)
{
	NeuronTree MSTtrees;
	undirectedGraph graph(inputTreePtr.listNeuron.size());
	for (int i = 0; i < inputTreePtr.listNeuron.size(); ++i)
	{
			
		float x1, y1, z1;
		x1 = inputTreePtr.listNeuron.at(i).x;
		y1 = inputTreePtr.listNeuron.at(i).y;
		z1 = inputTreePtr.listNeuron.at(i).z;
		for (int j = 0; j < inputTreePtr.listNeuron.size(); ++j)
		{
			float x2, y2, z2;
			x2 = inputTreePtr.listNeuron.at(j).x;
			y2 = inputTreePtr.listNeuron.at(j).y;
			z2 = inputTreePtr.listNeuron.at(j).z;

			double Vedge = sqrt(double(x1 - x2) * double(x1 - x2) + double(y1 - y2) * double(y1 - y2) + double(z1 - z2) * double(z1 - z2));
			pair<undirectedGraph::edge_descriptor, bool> edgeQuery = boost::edge(i, j, graph);
			if (!edgeQuery.second && i != j) boost::add_edge(i, j, lastVoted(i, weights(Vedge)), graph);
		}
	}

	vector <boost::graph_traits<undirectedGraph>::vertex_descriptor > p(num_vertices(graph));
	boost::prim_minimum_spanning_tree(graph, &p[0]);
	NeuronTree MSTtree;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
		
	for (size_t ii = 0; ii != p.size(); ++ii)
	{
		int pn;
		if (p[ii] == ii) pn = -1;
		else pn = p[ii] + 1;

		NeuronSWC S;
		S.n = ii + 1;
		S.type = 7;
		S.x = inputTreePtr.listNeuron.at(ii).x;
		S.y = inputTreePtr.listNeuron.at(ii).y;
		S.z = inputTreePtr.listNeuron.at(ii).z;
		S.r = 1;
		S.pn = pn;
		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size() - 1);
	}
	MSTtree.listNeuron = listNeuron;
	MSTtree.hashNeuron = hashNeuron;

	return MSTtree;
}

NeuronTree NeuronStructExplorer::MSTtreeTrim(const NeuronTree& inputTree)
{
	vector<vector<size_t> > childs; // location and its children, also in location.
	size_t neuronNum = inputTree.listNeuron.size();
	childs = vector<vector<size_t> >(neuronNum, vector<size_t>());
	size_t* flag = new size_t[neuronNum];
	for (size_t i = 0; i < neuronNum; ++i)
	{
		flag[i] = 1;
		size_t par = inputTree.listNeuron[i].pn;
		if (par < 0) continue;
		
		childs[inputTree.hashNeuron.value(par)].push_back(i);
	}

	NeuronTree outputTree;
	outputTree.listNeuron = inputTree.listNeuron;
	/*for (QList<NeuronSWC>::iterator nodeIt = outputTree.listNeuron.begin(); nodeIt != outputTree.listNeuron.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1)
		{
			int length = 0;

		}
	}*/

	for (vector<vector<size_t> >::iterator it = childs.begin(); it != childs.end(); ++it)
	{
		if (it->size() >= 2)
		{
			for (vector<size_t>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
			{
				outputTree.listNeuron[*it2].parent = -1;
			}
		}
	}

	return outputTree;
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
	// -- Determing false positives out of the detected structure by the threshold of specified distance to the manual structure.
	// -- if (the shortest distance of node i from detectedTree to manualTree > distThreshold) node i is deemed false positive.

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
	// -- False negavies are selected out of the manual structure if none of detected nodes are close enough to them.
	// -- if (the shortest distance of node i from manualTree to detectedTree > distThreshold) node i is deemed false negative.

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

			if (thisNodeDist > distThreshold)
			{
				vector<float> FNcoords;
				FNcoords.push_back(it->x);
				FNcoords.push_back(it->y);
				FNcoords.push_back(it->z);
				this->FNsList.push_back(FNcoords);
				FNcoords.clear();

				break;
			}

			++checkIt;
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

void NeuronStructExplorer::shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum)
{
	// -- Compute the PDF and CDF of shortest distance of detected nodes to manual nodes.

	float interval = float(upperBound) / float(binNum);
	vector<float> shortestDists;
	for (QList<NeuronSWC>::iterator it1 = inputTreePtr1->listNeuron.begin(); it1 != inputTreePtr1->listNeuron.end(); ++it1)
	{
		float minDist = 10000;
		for (QList<NeuronSWC>::iterator it2 = inputTreePtr2->listNeuron.begin(); it2 != inputTreePtr2->listNeuron.end(); ++it2)
		{
			float xSquare = ((it2->x) - (it1->x)) * ((it2->x) - (it1->x));
			float ySquare = ((it2->y) - (it1->y)) * ((it2->y) - (it1->y));
			float zSquare = ((it2->z) - (it1->z)) * ((it2->z) - (it1->z));
			float currDist = sqrtf(xSquare + ySquare + zSquare);

			if (currDist <= minDist) minDist = currDist;
		}
		shortestDists.push_back(minDist);
	}

	for (int i = 0; i < binNum; ++i)
	{
		this->nodeDistPDF[i] = 0;
		this->nodeDistPDF[i] = 0;
	}
	for (vector<float>::iterator distIt = shortestDists.begin(); distIt != shortestDists.end(); ++distIt)
	{
		int binNo = (*distIt) / interval;
		++(this->nodeDistPDF[binNo]);
	}
	this->nodeDistCDF[0] = this->nodeDistPDF[0];
	for (int i = 1; i < binNum; ++i) this->nodeDistCDF[i] = this->nodeDistCDF[i - 1] + nodeDistPDF[i];
}

void NeuronStructExplorer::segmentDecompose(NeuronTree* inputTreePtr)
{
	// -- This function used get_link_map and decompose in v_neuronswc.cpp to get segment hierarchical information.

	this->segmentList.clear();
	this->segmentList = NeuronTree__2__V_NeuronSWC_list(*inputTreePtr);
}