#include <ctime>

#include <boost\container\flat_set.hpp>
#include <boost\container\map.hpp>
#include <boost\container\flat_map.hpp>

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"

morphStructElement::morphStructElement() : eleShape("square"), xLength(3), yLength(3)
{
	vector<int> array1(3, 1);
	this->structEle2D.push_back(array1);
	this->structEle2D.push_back(array1);
	this->structEle2D.push_back(array1);
}

morphStructElement::morphStructElement(string shape) : eleShape(shape)
{
	if (this->eleShape.compare("square") == 0)
	{
		vector<int> array1(3, 1);
		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array1);
	}
	else if (this->eleShape.compare("circle") == 0)
	{
		this->xLength = 7;
		this->yLength = 7;

		vector<int> array1(7, 1);
		vector<int> array2(7, 1);
		vector<int> array3(7, 1);
		array1.at(0) = 0; array1.at(1) = 0; array1.at(5) = 0; array1.at(6) = 0;
		array2.at(0) = 0; array2.at(6) = 0;

		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array2);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array2);
		this->structEle2D.push_back(array1);
	}
}

vector<connectedComponent> ImgAnalyzer::findSignalBlobs_2Dcombine(vector<unsigned char**> inputSlicesVector, int dims[], unsigned char maxIP1D[])
{
	// Finds connected components from a image statck using slice-by-slice approach.
	// All components are sotred in the form of ImgAnalyzer::connectedComponent.
	// Each slice is independent to one another. Therefore, the same 3D blobs are consists of certain amount of 2D "blob slices." 
	// Each 2D blob slice accounts for 1 ImgAnalyzer::connectedComponent.

	// --- for TESTING purpose ------
	string testName = "Z:/IVSCC_mouse_inhibitory_442_swcROIcropped_lumps/test.tif";
	const char* testNameC = testName.c_str();
	V3DLONG testDims[4];
	testDims[0] = dims[0];
	testDims[1] = dims[1];
	testDims[2] = dims[2];
	testDims[3] = 1;
	// ------------------------------

	vector<connectedComponent> connList;
	cout << "slice dimension: " << dims[0] << " " << dims[1] << endl;

	// --------- Only enter this selection block when MIP image is not provided ---------
	if (maxIP1D == nullptr) 
	{
		unsigned char* maxIP1D = new unsigned char[dims[0] * dims[1]];
		unsigned char* currSlice1D = new unsigned char[dims[0] * dims[1]];	
		for (int i = 0; i < dims[0] * dims[1]; ++i)
		{
			maxIP1D[i] = 0;
			currSlice1D[i] = 0;
		}
		cout << "No maximum intensity projection image provided, preparing MIP now.. " << endl;
		for (vector<unsigned char**>::iterator it = inputSlicesVector.begin(); it != inputSlicesVector.end(); ++it)
		{
			cout << ptrdiff_t(it - inputSlicesVector.begin() + 1) << " ";
			size_t currSliceI = 0;
			for (int j = 0; j < dims[1]; ++j)
			{
				for (int i = 0; i < dims[0]; ++i)
				{
					++currSliceI;
					currSlice1D[currSliceI] = (*it)[j][i];
				}
			}
			ImgProcessor::imageMax(currSlice1D, maxIP1D, maxIP1D, dims);
		}
		cout << " MIP done." << endl;

		delete[] currSlice1D;
		currSlice1D = nullptr;
	}
	// ------- END [Onlu get into this selection when MIP image is not provided] -------

	// ----------- Prepare white pixel address book ------------
	set<vector<int> > whitePixAddress;
	unsigned char** maxIP2D = new unsigned char*[dims[1]];
	for (int j = 0; j < dims[1]; ++j)
	{
		maxIP2D[j] = new unsigned char[dims[0]];
		vector<int> coord(2);
		for (int i = 0; i < dims[0]; ++i)
		{
			maxIP2D[j][i] = maxIP1D[dims[0] * j + i];
			coord[0] = j;
			coord[1] = i;
			if (maxIP2D[j][i] > 0) whitePixAddress.insert(coord);
		}
	}
	// ------- END of [Prepare white pixel address book] -------
	
	// -------------------- Finding connected components slice by slice -------------------
	int islandCount = 0;
	cout << "  -- white pixel number: " << whitePixAddress.size() << endl;
	cout << "Processing slices: ";
	for (vector<unsigned char**>::iterator sliceIt = inputSlicesVector.begin(); sliceIt != inputSlicesVector.end(); ++sliceIt)
	{
		int sliceNum = int(sliceIt - inputSlicesVector.begin());
		cout << sliceNum << " ";
		for (set<vector<int> >::iterator mipIt = whitePixAddress.begin(); mipIt != whitePixAddress.end(); ++mipIt)
		{
			if ((*sliceIt)[mipIt->at(0)][mipIt->at(1)] > 0)
			{
				bool connected = false;
				for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
				{
					if (connIt->zMax != sliceNum) continue;
					else
					{
						for (set<vector<int> >::iterator it = connIt->coordSets[sliceNum].begin(); it != connIt->coordSets[sliceNum].end(); ++it)
						{
							if (it->at(0) >= mipIt->at(0) - 1 && it->at(0) <= mipIt->at(0) + 1 &&
								it->at(1) >= mipIt->at(1) - 1 && it->at(1) <= mipIt->at(1) + 1)
							{
								vector<int> newCoord(3);
								newCoord[0] = mipIt->at(0);
								newCoord[1] = mipIt->at(1);
								newCoord[2] = sliceNum;
								connIt->coordSets[sliceNum].insert(newCoord);

								if (newCoord[0] < connIt->xMin) connIt->xMin = newCoord[0];
								else if (newCoord[0] > connIt->xMax) connIt->xMax = newCoord[0];

								if (newCoord[1] < connIt->yMin) connIt->yMin = newCoord[1];
								else if (newCoord[1] > connIt->yMax) connIt->yMax = newCoord[1];

								connIt->zMin = sliceNum;
								connIt->zMax = sliceNum;

								connected = true;
								goto SIGNAL_VECTOR_INSERTED;
							}
						}
					}
				}

				if (!connected) // -- new connected component identified
				{
					++islandCount;
					connectedComponent newIsland;
					newIsland.islandNum = islandCount;
					vector<int> newCoord(3);
					newCoord[0] = mipIt->at(0);
					newCoord[1] = mipIt->at(1);
					newCoord[2] = sliceNum;
					set<vector<int> > coordSet;
					coordSet.insert(newCoord);
					newIsland.coordSets.insert(pair<int, set<vector<int> > >(sliceNum, coordSet));
					newIsland.xMax = newCoord[0];
					newIsland.xMin = newCoord[0];
					newIsland.yMax = newCoord[1];
					newIsland.yMin = newCoord[1];
					newIsland.zMin = sliceNum;
					newIsland.zMax = sliceNum;
					connList.push_back(newIsland);
					continue;
				}

			SIGNAL_VECTOR_INSERTED:
				continue;
			}
		}
	}
	cout << endl;
	// ------------------ END of [Finding connected components slice by slice] -----------------

	return connList;
}

vector<connectedComponent> ImgAnalyzer::merge2DConnComponent(const vector<connectedComponent>& inputConnCompList)
{
	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";

	vector<connectedComponent> outputConnCompList;

	int zMax = 0;
	
	boost::container::flat_multimap<int, int> b2Dtob3Dmap;
	b2Dtob3Dmap.clear();
	boost::container::flat_map<int, boost::container::flat_set<int> > b3Dcomps;
	b3Dcomps.clear();

	int sliceBlobCount = 0;
	for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
	{
		if (it->coordSets.begin()->first > zMax) zMax = it->coordSets.begin()->first;

		if (it->coordSets.begin()->first == 0)
		{
			++sliceBlobCount;
			b2Dtob3Dmap.insert(pair<int, int>(it->islandNum, sliceBlobCount));
			boost::container::flat_set<int> comps;
			comps.insert(it->islandNum);
			b3Dcomps[sliceBlobCount] = comps;
		}
	}
	
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
				b2Dtob3Dmap.insert(pair<int, int>(newCompsIt->islandNum, sliceBlobCount));
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
							pair<boost::container::flat_multimap<int, int>::iterator, boost::container::flat_multimap<int, int>::iterator> range = b2Dtob3Dmap.equal_range(preIt->islandNum);
							for (boost::container::flat_multimap<int, int>::iterator rangeIt = range.first; rangeIt != range.second; ++rangeIt)
							{
								if (i == 52) cout << "_" << rangeIt->first << "_" << rangeIt->second << "_" << preIt->islandNum << "_" << currIt->islandNum << "> ";
								if (rangeIt->first != preIt->islandNum) break;
								else
								{
									b2Dtob3Dmap.insert(pair<int, int>(currIt->islandNum, rangeIt->second));
									b3Dcomps[rangeIt->second].insert(currIt->islandNum);
								}
							}
							
							goto BLOB_MERGED;
						}
					}
				}

			BLOB_MERGED:
				merged = true;
				continue;
			}

			if (!merged)
			{
				++sliceBlobCount;
				b2Dtob3Dmap.insert(pair<int, int>(currIt->islandNum, sliceBlobCount));
				boost::container::flat_set<int> comps;
				comps.insert(currIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
		}
		cout << increasedSize << ", ";
	}
	cout << endl << endl;
	cout << "Merging Slice Done." << endl << endl;

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

vector<connectedComponent> ImgAnalyzer::merge2DConnComponent_vecVersion(const vector<connectedComponent>& inputConnCompList)
{
	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";

	vector<connectedComponent> outputConnCompList;

	int zMax = 0;
	boost::container::multimap<int, int> b2Dtob3Dmap;
	b2Dtob3Dmap.clear();
	boost::container::map<int, boost::container::flat_set<int> > b3Dcomps;
	b3Dcomps.clear();

	int sliceBlobCount = 0;
	for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
	{
		if (it->coordSets.begin()->first > zMax) zMax = it->coordSets.begin()->first;

		if (it->coordSets.begin()->first == 0)
		{
			++sliceBlobCount;
			b2Dtob3Dmap.insert(pair<int, int>(it->islandNum, sliceBlobCount));
			boost::container::flat_set<int> comps;
			comps.insert(it->islandNum);
			b3Dcomps[sliceBlobCount] = comps;
		}
	}

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
				b2Dtob3Dmap.insert(pair<int, int>(newCompsIt->islandNum, sliceBlobCount));
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
							pair<boost::container::multimap<int, int>::iterator, boost::container::multimap<int, int>::iterator> range = b2Dtob3Dmap.equal_range(preIt->islandNum);
							for (boost::container::multimap<int, int>::iterator rangeIt = range.first; rangeIt != range.second; ++rangeIt)
							{
								if (rangeIt->first != preIt->islandNum) break;
								b2Dtob3Dmap.insert(pair<int, int>(currIt->islandNum, rangeIt->second));
								b3Dcomps[rangeIt->second].insert(currIt->islandNum);
							}

							goto BLOB_MERGED;
						}
					}
				}

			BLOB_MERGED:
				merged = true;
				continue;
			}

			if (!merged)
			{
				++sliceBlobCount;
				b2Dtob3Dmap.insert(pair<int, int>(currIt->islandNum, sliceBlobCount));
				boost::container::flat_set<int> comps;
				comps.insert(currIt->islandNum);
				b3Dcomps[sliceBlobCount] = comps;
				increasedSize = increasedSize + comps.size();
			}
		}
		cout << increasedSize << ", ";
	}
	cout << endl << endl;
	cout << "Merging Slice Done." << endl << endl;

	cout << "Now merging 3D blobs.." << endl;
	cout << "-- oroginal 3D blobs number: " << b3Dcomps.size() << endl;
	bool mergeFinish = false;
	int currBaseBlob = 1;
	while (!mergeFinish)
	{
		for (boost::container::map<int, boost::container::flat_set<int> >::iterator checkIt1 = b3Dcomps.begin(); checkIt1 != b3Dcomps.end(); ++checkIt1)
		{
			if (checkIt1->first < currBaseBlob) continue;
			for (boost::container::map<int, boost::container::flat_set<int> >::iterator checkIt2 = checkIt1; checkIt2 != b3Dcomps.end(); ++checkIt2)
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

	map<int, connectedComponent> compsMap;
	for (vector<connectedComponent>::const_iterator inputIt = inputConnCompList.begin(); inputIt != inputConnCompList.end(); ++inputIt)
		compsMap.insert(pair<int, connectedComponent>(inputIt->islandNum, *inputIt));
	int newLabel = 0;
	for (boost::container::map<int, boost::container::flat_set<int> >::iterator it = b3Dcomps.begin(); it != b3Dcomps.end(); ++it)
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