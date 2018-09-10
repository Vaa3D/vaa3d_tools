#include <iostream>
#include <string>
#include <iterator>
#include <set>
#include <cmath>

#include <boost\filesystem.hpp>
#include <boost\container\flat_set.hpp>
#include <boost\container\flat_map.hpp>

#include "basic_4dimage.h"
#include "NeuronStructUtilities.h"
#include "ImgProcessor.h"

using namespace boost;

NeuronTree NeuronStructUtil::swcRegister(NeuronTree& inputTree, const NeuronTree& refTree)
{
	double xShift, yShift, zShift;
	double xScale, yScale, zScale;

	double xmin = 10000, ymin = 10000, zmin = 10000;
	double xmax = 0, ymax = 0, zmax = 0;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->x < xmin) xmin = it->x;
		if (it->x > xmax) xmax = it->x;
		if (it->y < ymin) ymin = it->y;
		if (it->y > ymax) ymax = it->y;
		if (it->z < zmin) zmin = it->z;
		if (it->z > zmax) zmax = it->z;
	}
	double refXmin = 10000, refYmin = 10000, refZmin = 10000;
	double refXmax = 0, refYmax = 0, refZmax = 0;
	for (QList<NeuronSWC>::const_iterator refIt = refTree.listNeuron.begin(); refIt != refTree.listNeuron.end(); ++refIt)
	{
		if (refIt->x < refXmin) refXmin = refIt->x;
		if (refIt->x > refXmax) refXmax = refIt->x;
		if (refIt->y < refYmin) refYmin = refIt->y;
		if (refIt->y > refYmax) refYmax = refIt->y;
		if (refIt->z < refZmin) refZmin = refIt->z;
		if (refIt->z > refZmax) refZmax = refIt->z;
	}

	xScale = (refXmax - refXmin) / (xmax - xmin);
	yScale = (refYmax - refYmin) / (ymax - ymin);
	zScale = (refZmax - refZmin) / (zmax - zmin);
	xShift = refXmin - xmin * xScale;
	yShift = refYmin - ymin * yScale;
	zShift = refZmin - zmin * zScale;

	NeuronTree outputTree;
	for (int i = 0; i < inputTree.listNeuron.size(); ++i)
	{
		NeuronSWC newNode = inputTree.listNeuron.at(i);
		newNode.x = newNode.x * xScale + xShift;
		newNode.y = newNode.y * yScale + yShift;
		newNode.z = newNode.z * zScale + zShift;
		outputTree.listNeuron.push_back(newNode);
	}

	return outputTree;
}

vector<connectedComponent> NeuronStructUtil::swc2signal2DBlobs(const NeuronTree& inputTree)
{
	// -- Finds signal blobs "slice by slice" from input NeuronTree. Each slice is independent to one another.
	// -- Therefore, the same real blobs in 3D are consists of certain amount of 2D "blob slices" produced by this method. 
	// -- Each 2D blob slice accounts for 1 ImgAnalyzer::connectedComponent.

	vector<NeuronSWC> allNodes;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it) allNodes.push_back(*it);

	vector<connectedComponent> connComps2D;
	int islandCount = 0;
	cout << "number of SWC nodes processed: ";
	for (vector<NeuronSWC>::iterator nodeIt = allNodes.begin(); nodeIt != allNodes.end(); ++nodeIt)
	{
		if (int(nodeIt - allNodes.begin()) % 10000 == 0) cout << int(nodeIt - allNodes.begin()) << " ";
		for (vector<connectedComponent>::iterator connIt = connComps2D.begin(); connIt != connComps2D.end(); ++connIt)
		{
			if (connIt->coordSets.empty()) continue;
			else if (int(nodeIt->z) == connIt->coordSets.begin()->first)
			{
				for (set<vector<int> >::iterator dotIt = connIt->coordSets[int(nodeIt->z)].begin(); dotIt != connIt->coordSets[int(nodeIt->z)].end(); ++dotIt)
				{
					if (int(nodeIt->x) <= dotIt->at(0) + 1 && int(nodeIt->x) >= dotIt->at(0) - 1 &&
						int(nodeIt->y) <= dotIt->at(1) + 1 && int(nodeIt->y) >= dotIt->at(1) - 1)
					{
						vector<int> newCoord(3);
						newCoord[0] = int(nodeIt->x);
						newCoord[1] = int(nodeIt->y);
						newCoord[2] = int(nodeIt->z);
						connIt->coordSets[newCoord[2]].insert(newCoord);
						connIt->size = connIt->size + 1;

						if (newCoord[0] < connIt->xMin) connIt->xMin = newCoord[0];
						else if (newCoord[0] > connIt->xMax) connIt->xMax = newCoord[0];

						if (newCoord[1] < connIt->yMin) connIt->yMin = newCoord[1];
						else if (newCoord[1] > connIt->yMax) connIt->yMax = newCoord[1];

						goto NODE_INSERTED;
					}
				}
			}
		}

		{
			++islandCount;
			connectedComponent newIsland;
			newIsland.islandNum = islandCount;
			vector<int> newCoord(3);
			newCoord[0] = int(nodeIt->x);
			newCoord[1] = int(nodeIt->y);
			newCoord[2] = int(nodeIt->z);
			set<vector<int> > coordSet;
			coordSet.insert(newCoord);
			newIsland.coordSets.insert(pair<int, set<vector<int> > >(newCoord[2], coordSet));
			newIsland.xMax = newCoord[0];
			newIsland.xMin = newCoord[0];
			newIsland.yMax = newCoord[1];
			newIsland.yMin = newCoord[1];
			newIsland.zMin = newCoord[2];
			newIsland.zMax = newCoord[2];
			newIsland.size = 1;
			connComps2D.push_back(newIsland);
		}

	NODE_INSERTED:
		continue;
	}
	cout << endl << endl;

	vector<float> center(3);
	for (vector<connectedComponent>::iterator it = connComps2D.begin(); it != connComps2D.end(); ++it)
		ImgAnalyzer::ChebyshevCenter_connComp(*it);

	return connComps2D;
}

vector<connectedComponent> NeuronStructUtil::swc2signal3DBlobs(const NeuronTree& inputTree)
{
	// -- This method is a wrapper of NeuronStructUtil::swc2signal2DBlobs and NeuronStructUtil::merge2DConnComponent.
	// -- It produces 3D signal blobs by calling the two swc2signal2DBlobs and merge2DConnComponent sequentially.

	vector<connectedComponent> inputConnCompList = this->swc2signal2DBlobs(inputTree);	
	vector<connectedComponent> outputConnCompList = this->merge2DConnComponent(inputConnCompList);

	return outputConnCompList;
}

vector<connectedComponent> NeuronStructUtil::merge2DConnComponent(const vector<connectedComponent>& inputConnCompList)
{
	// -- This method finds 3D signal blobs by grouping 2D signal blobs together, which are produced by NeuronStructUtil::swc2signal2DBlobs.
	// -- The approach is consists of 2 stages:
	//		1. Identifying the same 3D blobs slice by slice.
	//		2. Merging 3D blobs that contain the same 2D blobs.

	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";

	vector<connectedComponent> outputConnCompList;

	int zMax = 0;

	// -- I notice that boost's container templates are able to lift up the performace by ~30%.
	boost::container::flat_map<int, boost::container::flat_set<int> > b2Dtob3Dmap;
	b2Dtob3Dmap.clear();
	boost::container::flat_map<int, boost::container::flat_set<int> > b3Dcomps;
	b3Dcomps.clear();
	// ---------------------------------------------------------------------------------------

	// --------- First slice, container initiation --------------
	int sliceBlobCount = 0;
	for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
	{
		if (it->coordSets.begin()->first > zMax) zMax = it->coordSets.begin()->first;

		if (it->coordSets.begin()->first == 0)
		{
			++sliceBlobCount;
			boost::container::flat_set<int> blob3D;
			blob3D.insert(sliceBlobCount);
			b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int> >(it->islandNum, blob3D));
			boost::container::flat_set<int> comps;
			comps.insert(it->islandNum);
			b3Dcomps[sliceBlobCount] = comps;
		}
	}
	// -----------------------------------------------------------

	// ------------------------------------------- Merge 2D blobs from 2 adjacent slices -------------------------------------------
	vector<connectedComponent> currSliceConnComps;
	vector<connectedComponent> preSliceConnComps;
	size_t increasedSize;
	for (int i = 1; i <= zMax; ++i)
	{
		currSliceConnComps.clear();
		preSliceConnComps.clear();

		increasedSize = 0;
		for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
			if (it->coordSets.begin()->first == i) currSliceConnComps.push_back(*it);
		if (currSliceConnComps.empty())
		{
			cout << i << "->0 ";
			continue;
		}

		cout << i << "->";
		for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
			if (it->coordSets.begin()->first == i - 1) preSliceConnComps.push_back(*it);
		if (preSliceConnComps.empty())
		{
			for (vector<connectedComponent>::iterator newCompsIt = currSliceConnComps.begin(); newCompsIt != currSliceConnComps.end(); ++newCompsIt)
			{
				++sliceBlobCount;
				boost::container::flat_set<int> blob3D;
				blob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int> >(newCompsIt->islandNum, blob3D));
				boost::container::flat_set<int> comps;
				comps.insert(newCompsIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
			continue;
		}

		for (vector<connectedComponent>::iterator currIt = currSliceConnComps.begin(); currIt != currSliceConnComps.end(); ++currIt)
		{
			bool merged = false;
			for (vector<connectedComponent>::iterator preIt = preSliceConnComps.begin(); preIt != preSliceConnComps.end(); ++preIt)
			{
				if (currIt->xMin > preIt->xMax + 2 || currIt->xMax < preIt->xMin - 2 ||
					currIt->yMin > preIt->yMax + 2 || currIt->yMax < preIt->yMin - 2) continue;

				for (set<vector<int> >::iterator currDotIt = currIt->coordSets.begin()->second.begin(); currDotIt != currIt->coordSets.begin()->second.end(); ++currDotIt)
				{
					for (set<vector<int> >::iterator preDotIt = preIt->coordSets.begin()->second.begin(); preDotIt != preIt->coordSets.begin()->second.end(); ++preDotIt)
					{
						if (currDotIt->at(0) >= preDotIt->at(0) - 1 && currDotIt->at(0) <= preDotIt->at(0) + 1 &&
							currDotIt->at(1) >= preDotIt->at(1) - 1 && currDotIt->at(1) <= preDotIt->at(1) + 1)
						{
							merged = true;
							boost::container::flat_set<int> asso3Dblob = b2Dtob3Dmap[preIt->islandNum];
							b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int> >(currIt->islandNum, asso3Dblob));
							for (boost::container::flat_set<int>::iterator blob3DIt = asso3Dblob.begin(); blob3DIt != asso3Dblob.end(); ++blob3DIt)
								b3Dcomps[*blob3DIt].insert(currIt->islandNum);

							goto BLOB_MERGED;
						}
					}
				}

				if (!merged) continue;

			BLOB_MERGED:
				merged = true;
			}

			if (!merged)
			{
				++sliceBlobCount;
				boost::container::flat_set<int> newBlob3D;
				newBlob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int> >(currIt->islandNum, newBlob3D));
				boost::container::flat_set<int> comps;
				comps.insert(currIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
		}
		cout << increasedSize << ", ";
	}
	cout << endl << endl;
	cout << "Done merging 2D blobs from every 2 slices." << endl << endl;
	// ---------------------------------------- END of [Merge 2D blobs from 2 adjacent slices] -------------------------------------------

	// ------------------------------------------ Merge 3D blobs --------------------------------------------
	cout << "Now merging 3D blobs.." << endl;
	cout << "-- oroginal 3D blobs number: " << b3Dcomps.size() << endl;
	bool mergeFinish = false;
	int currBaseBlob = 1;
	while (!mergeFinish)
	{
		for (boost::container::flat_map<int, boost::container::flat_set<int> >::iterator checkIt1 = b3Dcomps.begin(); checkIt1 != b3Dcomps.end(); ++checkIt1)
		{
			if (checkIt1->first < currBaseBlob) continue;
			for (boost::container::flat_map<int, boost::container::flat_set<int> >::iterator checkIt2 = checkIt1; checkIt2 != b3Dcomps.end(); ++checkIt2)
			{
				if (checkIt2 == checkIt1) continue;
				for (boost::container::flat_set<int>::iterator member1 = checkIt1->second.begin(); member1 != checkIt1->second.end(); ++member1)
				{
					for (boost::container::flat_set<int>::iterator member2 = checkIt2->second.begin(); member2 != checkIt2->second.end(); ++member2)
					{
						if (*member2 == *member1)
						{
							checkIt1->second.insert(checkIt2->second.begin(), checkIt2->second.end());
							b3Dcomps.erase(checkIt2);
							currBaseBlob = checkIt1->first;
							cout << "  merging blob " << checkIt1->first << " and blob " << checkIt2->first << endl;
							goto MERGED;
						}
					}
				}
			}
		}
		mergeFinish = true;

	MERGED:
		continue;
	}
	cout << "-- new 3D blobs number: " << b3Dcomps.size() << endl;
	// --------------------------------------- END of [Merge 3D blobs] --------------------------------------

	map<int, connectedComponent> compsMap;
	for (vector<connectedComponent>::const_iterator inputIt = inputConnCompList.begin(); inputIt != inputConnCompList.end(); ++inputIt)
		compsMap.insert(pair<int, connectedComponent>(inputIt->islandNum, *inputIt));
	int newLabel = 0;
	for (boost::container::flat_map<int, boost::container::flat_set<int> >::iterator it = b3Dcomps.begin(); it != b3Dcomps.end(); ++it)
	{
		++newLabel;
		connectedComponent newComp;
		newComp.islandNum = newLabel;
		newComp.size = 0;
		newComp.xMax = 0; newComp.xMin = 0;
		newComp.yMax = 0; newComp.yMin = 0;
		newComp.zMax = 0; newComp.zMin = 0;
		for (boost::container::flat_set<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			newComp.coordSets.insert(pair<int, set<vector<int> > >(compsMap[*it2].coordSets.begin()->first, compsMap[*it2].coordSets.begin()->second));
			newComp.xMax = getMax(newComp.xMax, compsMap[*it2].xMax);
			newComp.xMin = getMin(newComp.xMin, compsMap[*it2].xMin);
			newComp.yMax = getMax(newComp.yMax, compsMap[*it2].yMax);
			newComp.yMin = getMin(newComp.yMin, compsMap[*it2].yMin);
			newComp.zMax = getMax(newComp.zMax, compsMap[*it2].zMax);
			newComp.zMin = getMin(newComp.zMin, compsMap[*it2].zMin);
			newComp.size = newComp.size + compsMap[*it2].size;
		}

		outputConnCompList.push_back(newComp);
	}

	return outputConnCompList;
}

NeuronTree NeuronStructUtil::swcIdentityCompare(const NeuronTree& subjectTree, const NeuronTree& refTree, float radius, float distThre)
{
	map<string, vector<NeuronSWC> > gridSWCmap; // better use vector instead of set here, as set by default sorts the elements.
												// This can cause complication if the element is a data struct.

	for (QList<NeuronSWC>::const_iterator refIt = refTree.listNeuron.begin(); refIt != refTree.listNeuron.end(); ++refIt)
	{
		string xLabel = to_string(int((refIt->x) / (radius * 2)));
		string yLabel = to_string(int((refIt->y) / (radius * 2)));
		string zLabel = to_string(int((refIt->z) / (radius * 2 / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		if (gridSWCmap.find(keyLabel) != gridSWCmap.end()) gridSWCmap[keyLabel].push_back(*refIt);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*refIt);
			gridSWCmap.insert(pair<string, vector<NeuronSWC> >(keyLabel, newSet));
		}
	}

	NeuronTree outputTree;
	NeuronTree refConfinedFilteredTree;
	for (QList<NeuronSWC>::const_iterator suIt = subjectTree.listNeuron.begin(); suIt != subjectTree.listNeuron.end(); ++suIt)
	{
		string xLabel = to_string(int((suIt->x) / (radius * 2)));
		string yLabel = to_string(int((suIt->y) / (radius * 2)));
		string zLabel = to_string(int((suIt->z) / (radius * 2 / zRATIO)));
		string keyLabel = xLabel + "_" + yLabel + "_" + zLabel;
		
		if (gridSWCmap.find(keyLabel) != gridSWCmap.end())
		{
			bool identified = false;
			for (vector<NeuronSWC>::iterator nodeIt = gridSWCmap[keyLabel].begin(); nodeIt != gridSWCmap[keyLabel].end(); ++nodeIt)
			{
				float dist = sqrt((nodeIt->x - suIt->x) * (nodeIt->x - suIt->x) + (nodeIt->y - suIt->y) * (nodeIt->y - suIt->y) +
					zRATIO * zRATIO * (nodeIt->z - suIt->z) * (nodeIt->z - suIt->z));
				
				if (dist <= distThre)
				{
					outputTree.listNeuron.push_back(*suIt);
					(outputTree.listNeuron.end() - 1)->type = 2;
					identified = true;
					break;
				}
			}

			if (!identified)
			{
				outputTree.listNeuron.push_back(*suIt);
				(outputTree.listNeuron.end() - 1)->type = 3;
			}
		}
		else refConfinedFilteredTree.listNeuron.push_back(*suIt);
	}

	return outputTree;
}

NeuronTree NeuronStructUtil::swcZclenUP(const NeuronTree& inputTree, float zThre)
{
	map<string, vector<NeuronSWC> > xyLabeledNodeMap;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		string xLabel = to_string(int((it->x)));
		string yLabel = to_string(int((it->y)));
		string zLabel = to_string(int((it->z)));
		string labelKey = xLabel + "_" + yLabel;
		if (xyLabeledNodeMap.find(labelKey) != xyLabeledNodeMap.end()) xyLabeledNodeMap[labelKey].push_back(*it);
		else
		{
			vector<NeuronSWC> newSet;
			newSet.push_back(*it);
			xyLabeledNodeMap.insert(pair<string, vector<NeuronSWC> >(labelKey, newSet));
		}
	}

	NeuronTree outputTree;
	for (map<string, vector<NeuronSWC> >::iterator it = xyLabeledNodeMap.begin(); it != xyLabeledNodeMap.end(); ++it)
	{
		boost::container::flat_map<float, NeuronSWC> zSortedNodes; // boost::container::flat_map is vector based, so it can do pointer arithmetic.
		if (it->second.size() == 1)
		{
			outputTree.listNeuron.push_back(*(it->second.begin()));
			continue;
		}
		else
		{
			for (vector<NeuronSWC>::iterator nodeIt = it->second.begin(); nodeIt != it->second.end(); ++nodeIt)
				zSortedNodes.insert(pair<float, NeuronSWC>(nodeIt->z, *nodeIt));

			boost::container::flat_map<float, NeuronSWC>::iterator startIt = zSortedNodes.begin();
			for (boost::container::flat_map<float, NeuronSWC>::iterator zIt = zSortedNodes.begin() + 1; zIt != zSortedNodes.end(); ++zIt)
			{
				if (zIt->second.z - (zIt - 1)->second.z > zThre)
				{
					ptrdiff_t steps = zIt - startIt;
					ptrdiff_t selectedLoc = ptrdiff_t(int(steps) / 2);
					outputTree.listNeuron.push_back((startIt + selectedLoc)->second);
					startIt = zIt;
				}
			}
			if (startIt == zSortedNodes.begin())
			{
				ptrdiff_t selectedLoc = ptrdiff_t(int((zSortedNodes.end() - zSortedNodes.begin()) / 2));
				outputTree.listNeuron.push_back((startIt + selectedLoc)->second);
			}
		}
	}

	return outputTree;
}

void NeuronStructUtil::swcSlicer(const NeuronTree& inputTree, vector<NeuronTree>& outputTrees, int thickness)
{
	// -- Dissemble SWC files into "slices." Each outputSWC file represents only 1 z slice.
	// thickness * 2 + 1 = the number of consecutive z slices for one SWC node to appear. This is for the purpose producing continous masks.

	QList<NeuronSWC> inputList = inputTree.listNeuron;
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
		outputTrees.push_back(*it);
}

void NeuronStructUtil::swcSliceAssembler(string swcPath)
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

void NeuronStructUtil::swcDownSampleZ(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, int factor)
{
	// -- Downsample swc in z dimension with the given factor.

	QList<NeuronSWC> inputList = inputTreePtr->listNeuron;
	outputTreePtr->listNeuron.clear();
	for (QList<NeuronSWC>::iterator it = inputList.begin(); it != inputList.end(); ++it)
		if (int(it->z) % factor == 0) outputTreePtr->listNeuron.push_back(*it);
}

void NeuronStructUtil::detectedSWCprobFilter(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr, float threshold)
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
void NeuronStructUtil::sigNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, float ratio, float distance)
{
	// -- Randomly generate signal patches within given distance range
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the radius allowed with SWC node centered

	cout << "target signal patch number: " << int(inputTree.listNeuron.size() * ratio) << endl;
	int nodeCount = 0;
	for (QList<NeuronSWC>::const_iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		outputTree.listNeuron.push_back(*it);
		(outputTree.listNeuron.end() - 1)->parent = -1;
		(outputTree.listNeuron.end() - 1)->type = 2;
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
			outputTree.listNeuron.push_back(newNode);

			++foldCount;
		}
	}
}

void NeuronStructUtil::bkgNode_Gen(const NeuronTree& inputTree, NeuronTree& outputTree, int dims[], float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance
	// dims:     image stack dimension
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the forbidden distance from each SWC node

	QList<NeuronSWC> neuronList = inputTree.listNeuron;
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
			outputTree.listNeuron.push_back(newBkgNode);
		}
	}
}

void NeuronStructUtil::bkgNode_Gen_somaArea(const NeuronTree& intputTree, NeuronTree& outputTree, int xLength, int yLength, int zLength, float ratio, float distance)
{
	// -- Randomly generate background patches away from the forbidden distance. This method aims to reinforce the background recognition near soma area.
	// xLength, yLength, zLength: decide the range to apply with soma centered
	// ratio:    the ratio of targeted number of patches to the number of nodes in the inputTree
	// distance: the forbidden distance from each SWC node

	NeuronSWC somaNode;
	for (QList<NeuronSWC>::const_iterator it = intputTree.listNeuron.begin(); it != intputTree.listNeuron.end(); ++it)
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
	for (QList<NeuronSWC>::const_iterator it = intputTree.listNeuron.begin(); it != intputTree.listNeuron.end(); ++it)
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
			outputTree.listNeuron.push_back(newBkgNode);
		}
	}
}
/* =================================== Volumetric SWC sampling methods =================================== */