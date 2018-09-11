#include <iostream>
#include <algorithm>
#include <cmath>

#include "basic_4dimage.h"
#include "basic_landmark.h"
#include "neuron_format_converter.h"

#include "NeuronStructExplorer.h"

using namespace std;

topoCharacter::topoCharacter(NeuronSWC centerNode, segUnit* segUnitPtr)
{
	this->segUnitPtr = segUnitPtr;
	
	this->topoCenter = centerNode;
	this->topoCenterPa = segUnitPtr->nodes.at(segUnitPtr->nodeLocMap[this->topoCenter.n]);
	vector<size_t> childLocs = segUnitPtr->childMap[segUnitPtr->nodeLocMap[this->topoCenter.n]];
	for (vector<size_t>::iterator it = childLocs.begin(); it != childLocs.end(); ++it) this->topoCenterImmedChildren.push_back(segUnitPtr->nodes.at(*it));

	if (childLocs.size() == 2)
	{
		double dot = (topoCenterImmedChildren.at(0).x - this->topoCenter.x) * (topoCenterImmedChildren.at(1).x - this->topoCenter.x) +
					 (topoCenterImmedChildren.at(0).y - this->topoCenter.y) * (topoCenterImmedChildren.at(1).y - this->topoCenter.y) +
					 (topoCenterImmedChildren.at(0).z - this->topoCenter.z) * (topoCenterImmedChildren.at(1).z - this->topoCenter.z);

		double sq1 = (topoCenterImmedChildren.at(0).x - this->topoCenter.x) * (topoCenterImmedChildren.at(0).x - this->topoCenter.x) + 
					 (topoCenterImmedChildren.at(0).y - this->topoCenter.y) * (topoCenterImmedChildren.at(0).y - this->topoCenter.y) + 
					 (topoCenterImmedChildren.at(0).z - this->topoCenter.z) * (topoCenterImmedChildren.at(0).z - this->topoCenter.z);

		double sq2 = (topoCenterImmedChildren.at(1).x - this->topoCenter.x) * (topoCenterImmedChildren.at(1).x - this->topoCenter.x) +
					 (topoCenterImmedChildren.at(1).y - this->topoCenter.y) * (topoCenterImmedChildren.at(1).y - this->topoCenter.y) +
					 (topoCenterImmedChildren.at(1).z - this->topoCenter.z) * (topoCenterImmedChildren.at(1).z - this->topoCenter.z);

		this->childAngle = acos(dot / sqrt(sq1*sq2));
		if (isnan(acos(dot / sqrt(sq1*sq2)))) this->childAngle = -1;

		
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
		int par = inputTree.listNeuron[i].pn;
		if (par < 0) continue;
		
		childs[inputTree.hashNeuron.value(par)].push_back(i);
	}

	this->segs.clear();
	for (QList<NeuronSWC>::const_iterator nodeIt = inputTree.listNeuron.begin(); nodeIt != inputTree.listNeuron.end(); ++nodeIt)
	{
		if (nodeIt->parent == -1)
		{
			segUnit newSeg;
			newSeg.nodes.push_back(*nodeIt);
			vector<size_t> childLocs = childs[inputTree.hashNeuron[nodeIt->n]];
			vector<size_t> grandChildLocs;
			int childNum = 0;
			while (childNum != 0)
			{
				grandChildLocs.clear();
				for (vector<size_t>::iterator childIt = childLocs.begin(); childIt != childLocs.end(); ++childIt)
				{
					newSeg.nodes.push_back(inputTree.listNeuron.at(*childIt));
					vector<size_t> thisGrandChildLocs = childs[inputTree.hashNeuron[*childIt]];
					grandChildLocs.insert(childLocs.end(), thisGrandChildLocs.begin(), thisGrandChildLocs.end());
				}
				childNum = grandChildLocs.size();
				
				childLocs = grandChildLocs;
			}
			
			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it) 
				newSeg.nodeLocMap.insert(pair<int, size_t>(it->n, size_t(it - newSeg.nodes.begin())));

			for (QList<NeuronSWC>::iterator it = newSeg.nodes.begin(); it != newSeg.nodes.end(); ++it)
			{
				if (it->parent == -1) continue;
				else
				{
					if (newSeg.childMap.find(newSeg.nodeLocMap[it->parent]) != newSeg.childMap.end())
						newSeg.childMap[newSeg.nodeLocMap[it->parent]].push_back(newSeg.nodeLocMap[it->n]);
					else
					{
						vector<size_t> childSet;
						childSet.push_back(newSeg.nodeLocMap[it->n]);
						newSeg.childMap.insert(pair<size_t, vector<size_t> >(newSeg.nodeLocMap[it->parent], childSet));
					}
				}
			}
			this->segs.push_back(newSeg);
		}
	}

	/*for (vector<segUnit>::iterator segIt = this->segs.begin(); segIt != this->segs.end(); ++segIt)
	{
		for (vector<NeuronSWC>::iterator nodeIt = segIt->nodes.begin(); nodeIt != segIt->nodes.end(); ++nodeIt)
		{

		}
	}

	for (vector<vector<size_t> >::iterator it = childs.begin(); it != childs.end(); ++it)
	{
		if (it->size() >= 2)
		{
			for (vector<size_t>::iterator it2 = it->begin(); it2 != it->end(); ++it2)
			{
				outputTree.listNeuron[*it2].parent = -1;
			}
			outputTree.listNeuron[it->at(1)].parent = -1;
		}
	}

	return outputTree;*/

	/*if (inputSeg.size() <= 3) return;

#define PI 3.1415926
	//long segInfoNodeCount = 0;

	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostMean = 0;
	double mainSqr = ((inputSeg.end() - 1)->x - inputSeg.begin()->x)*((inputSeg.end() - 1)->x - inputSeg.begin()->x) +
		((inputSeg.end() - 1)->y - inputSeg.begin()->y)*((inputSeg.end() - 1)->y - inputSeg.begin()->y) +
		((inputSeg.end() - 1)->z - inputSeg.begin()->z)*((inputSeg.end() - 1)->z - inputSeg.begin()->z);
	double segDist = sqrt(mainSqr);
	double nodeHeadDot, nodeHeadSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistMean = 0;
	//cout << "segment displacement: " << segDist << endl << endl;

	cout << "    -- nodes contained in the segment to be straightned: " << segInfoPtr->nodeCount << endl;
	for (vector<V_NeuronSWC_unit>::iterator check = inputSeg.begin(); check != inputSeg.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;
	vector<nodeInfo> pickedNode;
	for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin() + 1; it != inputSeg.end() - 1; ++it)
	{
		dot = ((it - 1)->x - it->x)*((it + 1)->x - it->x) + ((it - 1)->y - it->y)*((it + 1)->y - it->y) + ((it - 1)->z - it->z)*((it + 1)->z - it->z);
		sq1 = ((it - 1)->x - it->x)*((it - 1)->x - it->x) + ((it - 1)->y - it->y)*((it - 1)->y - it->y) + ((it - 1)->z - it->z)*((it - 1)->z - it->z);
		sq2 = ((it + 1)->x - it->x)*((it + 1)->x - it->x) + ((it + 1)->y - it->y)*((it + 1)->y - it->y) + ((it + 1)->z - it->z)*((it + 1)->z - it->z);
		if (isnan(acos(dot / sqrt(sq1*sq2)))) return;
		radAngle = acos(dot / sqrt(sq1*sq2));
		nodeHeadDot = (it->x - inputSeg.begin()->x)*((inputSeg.end() - 1)->x - inputSeg.begin()->x) +
			(it->y - inputSeg.begin()->y)*((inputSeg.end() - 1)->y - inputSeg.begin()->y) +
			(it->z - inputSeg.begin()->z)*((inputSeg.end() - 1)->z - inputSeg.begin()->z);
		nodeHeadSqr = (it->x - inputSeg.begin()->x)*(it->x - inputSeg.begin()->x) + (it->y - inputSeg.begin()->y)*(it->y - inputSeg.begin()->y) + (it->z - inputSeg.begin()->z)*(it->z - inputSeg.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeHeadDot / sqrt(mainSqr*nodeHeadSqr));
		nodeToMainDist = sqrt(nodeHeadSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistMean = nodeToMainDistMean + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle / PI) << " turning cost:" << (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI) << " " << it->x << " " << it->y << " " << it->z << endl;

		if ((radAngle / PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			nodeInfo sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;

			sharp.segID = it->seg_id;
			sharp.distToMain = nodeToMainDist;
			sharp.sqr1 = sq1; sharp.sqr2 = sq2; sharp.dot = dot;
			sharp.radAngle = radAngle;
			sharp.nodeAddress = it;
			sharp.turnCost = (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI);

			pickedNode.push_back(sharp);
			turnCostMean = turnCostMean + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return;

	nodeToMainDistMean = nodeToMainDistMean / (inputSeg.size() - 2);
	turnCostMean = turnCostMean / pickedNode.size();
	cout << endl << endl << "  ==== start deleting nodes... " << endl;

	int delete_count = 0;
	ptrdiff_t order = 0;
	vector<V_NeuronSWC> preservedSegs;
	for (vector<nodeInfo>::iterator it = pickedNode.begin(); it != pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMain << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMain >= nodeToMainDistMean || it->turnCost >= turnCostMean || it->distToMain >= segDist)
		{
			++delete_count;
			if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->segID].to_be_deleted = true;
			--(segInfoPtr->nodeCount);

			V_NeuronSWC preservedNode;
			preservedNode.row.push_back(*(it->nodeAddress - order));
			preservedNode.row[0].data[6] = -1;
			preservedSegs.push_back(preservedNode);
			cout << "delete [" << (it->nodeAddress - order)->x << " " << (it->nodeAddress - order)->y << " " << (it->nodeAddress - order)->z << "] " << delete_count << endl;
			inputSeg.erase(it->nodeAddress - order);
			++order;
			cout << "----" << endl;
		}
	}

	cout << endl << "  ==== cheking angles... " << endl;

	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin() + 1; it != inputSeg.end() - 1; ++it)
		{
			double dot2 = (it->x - (it - 1)->x)*((it + 1)->x - it->x) + (it->y - (it - 1)->y)*((it + 1)->y - it->y) + (it->z - (it - 1)->z)*((it + 1)->z - it->z);
			double sq1_2 = ((it - 1)->x - it->x)*((it - 1)->x - it->x) + ((it - 1)->y - it->y)*((it - 1)->y - it->y) + ((it - 1)->z - it->z)*((it - 1)->z - it->z);
			double sq2_2 = ((it + 1)->x - it->x)*((it + 1)->x - it->x) + ((it + 1)->y - it->y)*((it + 1)->y - it->y) + ((it + 1)->z - it->z)*((it + 1)->z - it->z);
			if (isnan(acos(dot2 / sqrt(sq1_2*sq2_2)))) break;
			double radAngle_2 = acos(dot2 / sqrt(sq1_2*sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;

			if ((radAngle_2 / PI) * 180 > 75)
			{
				if (sqrt(sq1_2) > (1 / 10)*sqrt(sq2_2))
				{
					--(segInfoPtr->nodeCount);
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((inputSeg.size() - deleteCount2) <= 2)
					{
						--deleteCount2;
						++(segInfoPtr->nodeCount);
						break;
					}
					if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->seg_id].to_be_deleted = true;

					V_NeuronSWC preservedNode;
					preservedNode.row.push_back(*it);
					preservedNode.row[0].data[6] = -1;
					preservedSegs.push_back(preservedNode);
					inputSeg.erase(it);
				}
			}
		}
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);

	size_t label = 1;
	cout << "number of nodes after straightening process: " << inputSeg.size() << " ( segID = " << segInfoPtr->segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (vector<V_NeuronSWC_unit>::iterator it = inputSeg.begin(); it != inputSeg.end(); ++it)
	{
		it->seg_id = segInfoPtr->segID;
		it->data[0] = label;
		it->data[6] = label + 1;
		++label;
		cout << "[" << it->seg_id << ": " << it->x << " " << it->y << " " << it->z << "] ";
	}
	cout << endl;
	(inputSeg.end() - 1)->data[6] = -1;

	if (connectEdit == segmentEdit)
	{
		V_NeuronSWC newSeg;
		newSeg.row = inputSeg;
		curImgPtr->tracedNeuron.seg[segInfoPtr->segID] = newSeg;

		size_t singleNodeCount = 1;
		for (vector<V_NeuronSWC>::iterator nodeIt = preservedSegs.begin(); nodeIt != preservedSegs.end(); ++nodeIt)
		{
			nodeIt->row[0].seg_id = curImgPtr->tracedNeuron.seg.size() + singleNodeCount;
			++singleNodeCount;
			curImgPtr->tracedNeuron.seg.push_back(*nodeIt);
			//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
		}
	}

	return;*/
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