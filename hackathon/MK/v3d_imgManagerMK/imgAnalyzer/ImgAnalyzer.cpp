#include <ctime>

#include "ImgAnalyzer.h"

// ======================================= Image Segmentation ======================================= //
vector<connectedComponent> ImgAnalyzer::findSignalBlobs(vector<unsigned char**> inputSlicesVector, int dims[], int distThre, unsigned char maxIP1D[])
{
	// -- For the simplicity when specifying an element in the slice, I decided to use 2D array to represent each slice: vector<unsigned char**>.

	vector<connectedComponent> connList2D;
	cout << endl << "Identifying 2D signal blobs.. \n slice dimension: " << dims[0] << " " << dims[1] << endl;
	bool MIPprovided = true;

	// --------- Enter this selection block only when MIP image is not provided ---------
	if (maxIP1D == nullptr) 
	{
		MIPprovided = false;
		cout << "No maximum intensity projection image provided, preparing MIP now.. " << endl;

		maxIP1D = new unsigned char[dims[0] * dims[1]];
		unsigned char* currSlice1D = new unsigned char[dims[0] * dims[1]];	
		for (int i = 0; i < dims[0] * dims[1]; ++i)
		{
			maxIP1D[i] = 0;
			currSlice1D[i] = 0;
		}
		
		cout << "scanning slices.. ";
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
			ImgProcessor::imgMax(currSlice1D, maxIP1D, maxIP1D, dims);
		}
		cout << " MIP done." << endl;

		delete[] currSlice1D;
		currSlice1D = nullptr;

	}

	/***************** testing block *****************/
	/*V3DLONG mipDims[4];
	mipDims[0] = dims[0];
	mipDims[1] = dims[1];
	mipDims[2] = 1;
	mipDims[3] = 1;
	string testSaveName = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\testMIP.tif";
	const char* testSaveNameC = testSaveName.c_str();
	ImgManager::saveimage_wrapper(testSaveNameC, maxIP1D, mipDims, 1);*/
	/************ END of [testing block] ************/

	// ------- END [Enter this selection block only when MIP image is not provided] -------

	// ----------- Prepare white pixel address book ------------
	set<vector<int>> whitePixAddress;
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
	// ------- END of [Prepare white pixel address book] ------
	
	// -------------------- Finding connected components slice by slice -------------------
	int islandCount = 0;
	cout << "  -- white pixel number: " << whitePixAddress.size() << endl << endl;
	cout << "Processing each slice to identify connected components: ";
	for (vector<unsigned char**>::iterator sliceIt = inputSlicesVector.begin(); sliceIt != inputSlicesVector.end(); ++sliceIt)
	{
		int sliceNum = int(sliceIt - inputSlicesVector.begin());
		cout << sliceNum << " ";
		for (set<vector<int>>::iterator mipIt = whitePixAddress.begin(); mipIt != whitePixAddress.end(); ++mipIt)
		{
			if ((*sliceIt)[mipIt->at(0)][mipIt->at(1)] > 0) // use mip image to narraow down search region for every slice.
			{
				bool connected = false;
				for (vector<connectedComponent>::iterator connIt = connList2D.begin(); connIt != connList2D.end(); ++connIt)
				{
					if (connIt->zMax != sliceNum) continue; // skip those components that don't extend in z direction anymore
					else
					{
						for (set<vector<int>>::iterator it = connIt->coordSets.at(sliceNum).begin(); it != connIt->coordSets.at(sliceNum).end(); ++it)
						{
						/******* IMPORATANT NOTE: Vaa3D coord system's x and y are flipped from TIF. Needs to switch x and when assigning x and y to swc from tif image! *******/
							if (it->at(0) >= mipIt->at(1) - distThre && it->at(0) <= mipIt->at(1) + distThre &&
								it->at(1) >= mipIt->at(0) - distThre && it->at(1) <= mipIt->at(0) + distThre) // using 8-connectivity
							{
								vector<int> newCoord(3);
								newCoord[0] = mipIt->at(1);
								newCoord[1] = mipIt->at(0);
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
					newCoord[0] = mipIt->at(1);
					newCoord[1] = mipIt->at(0);
					newCoord[2] = sliceNum;
					set<vector<int>> coordSet;
					coordSet.insert(newCoord);
					newIsland.coordSets.insert(pair<int, set<vector<int>>>(sliceNum, coordSet));
					newIsland.xMax = newCoord[0];
					newIsland.xMin = newCoord[0];
					newIsland.yMax = newCoord[1];
					newIsland.yMin = newCoord[1];
					newIsland.zMin = sliceNum;
					newIsland.zMax = sliceNum;
					connList2D.push_back(newIsland);
					continue;
				}

			SIGNAL_VECTOR_INSERTED:
				continue;
			}
		}
	}
	cout << endl;
	// ------------------ END of [Finding connected components slice by slice] -----------------

	// ------- Merge 2D blobs into 3D -------
	vector<connectedComponent> connList = ImgAnalyzer::merge2DConnComponent(connList2D);

	return connList;
}

vector<connectedComponent> ImgAnalyzer::merge2DConnComponent(const vector<connectedComponent>& inputConnCompList)
{
	// -- This method finds 3D signal blobs by grouping 2D signal blobs together, which are generated by ImgAnalyzer::findSignalBlobs.
	// -- This method is typically called by ImgAnalyzer::findSignalBlobs when identifying 3D blobs from 2D ones.
	// -- The approach is consists of 2 stages:
	//		1. Identifying the same 3D blobs slice by slice.
	//		2. Merging 3D blobs that contain the same 2D blobs.

	cout << "Merging 2D signal blobs.." << endl;
	cout << "-- processing slice ";

	vector<connectedComponent> outputConnCompList;

	int zMax = 0;

	// -- I notice that boost's container templates are able to lift up the performace by ~30%.
	boost::container::flat_map<int, boost::container::flat_set<int>> b2Dtob3Dmap;
	b2Dtob3Dmap.clear();
	boost::container::flat_map<int, boost::container::flat_set<int>> b3Dcomps;  // a map from 3D connected components to all of its associated 2D connected components
	b3Dcomps.clear();
	// ---------------------------------------------------------------------------------------

	// --------- First slice, container initialization --------------
	int sliceBlobCount = 0;
	for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
	{
		if (it->coordSets.begin()->first > zMax) zMax = it->coordSets.begin()->first;

		if (it->coordSets.begin()->first == 0) // 1st slice connected components profile initialization
		{
			++sliceBlobCount;
			boost::container::flat_set<int> blob3D;
			blob3D.insert(sliceBlobCount);
			b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(it->islandNum, blob3D));
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
			if (it->coordSets.begin()->first == i) currSliceConnComps.push_back(*it); // collect all connected components from the current slice
		if (currSliceConnComps.empty())
		{
			cout << i << "->0 ";
			continue;
		}

		cout << i << "->";
		for (vector<connectedComponent>::const_iterator it = inputConnCompList.begin(); it != inputConnCompList.end(); ++it)
			if (it->coordSets.begin()->first == i - 1) preSliceConnComps.push_back(*it);  // collect all connected components from the previous slice
		if (preSliceConnComps.empty())
		{
			// If the previous slice is empty, all 2D components found in the current slice will be part of new 3D components.
			for (vector<connectedComponent>::iterator newCompsIt = currSliceConnComps.begin(); newCompsIt != currSliceConnComps.end(); ++newCompsIt)
			{
				++sliceBlobCount;
				boost::container::flat_set<int> blob3D;
				blob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(newCompsIt->islandNum, blob3D));
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
				// First, use component boundaries to quickly exclude those pixels that can't be connected to any existing components.
				// And then create new components for these pixels.
				if (currIt->xMin > preIt->xMax + 2 || currIt->xMax < preIt->xMin - 2 ||
					currIt->yMin > preIt->yMax + 2 || currIt->yMax < preIt->yMin - 2) continue;

				for (set<vector<int>>::iterator currDotIt = currIt->coordSets.begin()->second.begin(); currDotIt != currIt->coordSets.begin()->second.end(); ++currDotIt)
				{
					for (set<vector<int>>::iterator preDotIt = preIt->coordSets.begin()->second.begin(); preDotIt != preIt->coordSets.begin()->second.end(); ++preDotIt)
					{
						if (currDotIt->at(0) >= preDotIt->at(0) - 1 && currDotIt->at(0) <= preDotIt->at(0) + 1 &&
							currDotIt->at(1) >= preDotIt->at(1) - 1 && currDotIt->at(1) <= preDotIt->at(1) + 1)
						{
							merged = true;
							// Find out to which 3D component the 2D component connected to the pixel belong.    
							boost::container::flat_set<int> asso3Dblob = b2Dtob3Dmap[preIt->islandNum];

							// Register the component of the pixel in the current slice to b2Dtob3Dmap.
							b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(currIt->islandNum, asso3Dblob));

							// Add a new entry of newly identified 2D component that is connected to the existing 3D component to b3Dcomps.
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

			if (!merged) // All 2D blobs in the current slice fail to find its associated 3D blobs. Create new 3D blobs for them here.
			{
				++sliceBlobCount;
				boost::container::flat_set<int> newBlob3D;
				newBlob3D.insert(sliceBlobCount);
				b2Dtob3Dmap.insert(pair<int, boost::container::flat_set<int>>(currIt->islandNum, newBlob3D));
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
	// Merge any 3D blobs if any of them share the same 2D blob members.
	cout << "Now merging 3D blobs.." << endl;
	cout << " -- oroginal 3D blobs number: " << b3Dcomps.size() << endl;
	bool mergeFinish = false;
	int currBaseBlob = 1;
	while (!mergeFinish)
	{
		for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator checkIt1 = b3Dcomps.begin(); checkIt1 != b3Dcomps.end(); ++checkIt1)
		{
			if (checkIt1->first < currBaseBlob) continue;
			for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator checkIt2 = checkIt1 + 1; checkIt2 != b3Dcomps.end(); ++checkIt2)
			{
				//if (checkIt2 == checkIt1) continue;
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
	cout << " -- new 3D blobs number: " << b3Dcomps.size() << endl;
	// --------------------------------------- END of [Merge 3D blobs] --------------------------------------

	// ------------------------------------- Create 3D connected component data -------------------------------------
	map<int, connectedComponent> compsMap;
	for (vector<connectedComponent>::const_iterator inputIt = inputConnCompList.begin(); inputIt != inputConnCompList.end(); ++inputIt)
		compsMap.insert(pair<int, connectedComponent>(inputIt->islandNum, *inputIt));
	int newLabel = 0;
	for (boost::container::flat_map<int, boost::container::flat_set<int>>::iterator it = b3Dcomps.begin(); it != b3Dcomps.end(); ++it)
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
			// A 3D connected component may contain different 2D components from the same slice.
			if (newComp.coordSets.find(compsMap.at(*it2).coordSets.begin()->first) != newComp.coordSets.end())
			{
				for (set<vector<int>>::iterator it3 = compsMap.at(*it2).coordSets.begin()->second.begin();
					it3 != compsMap.at(*it2).coordSets.begin()->second.end(); ++it3)
					newComp.coordSets.at(compsMap.at(*it2).coordSets.begin()->first).insert(*it3);

				newComp.xMax = getMax(newComp.xMax, compsMap.at(*it2).xMax);
				newComp.xMin = getMin(newComp.xMin, compsMap.at(*it2).xMin);
				newComp.yMax = getMax(newComp.yMax, compsMap.at(*it2).yMax);
				newComp.yMin = getMin(newComp.yMin, compsMap.at(*it2).yMin);
				newComp.zMax = getMax(newComp.zMax, compsMap.at(*it2).zMax);
				newComp.zMin = getMin(newComp.zMin, compsMap.at(*it2).zMin);
				newComp.size = newComp.size + compsMap.at(*it2).size;
			}
			else
			{
				newComp.coordSets.insert(pair<int, set<vector<int>>>(compsMap.at(*it2).coordSets.begin()->first, compsMap.at(*it2).coordSets.begin()->second));
				newComp.xMax = getMax(newComp.xMax, compsMap.at(*it2).xMax);
				newComp.xMin = getMin(newComp.xMin, compsMap.at(*it2).xMin);
				newComp.yMax = getMax(newComp.yMax, compsMap.at(*it2).yMax);
				newComp.yMin = getMin(newComp.yMin, compsMap.at(*it2).yMin);
				newComp.zMax = getMax(newComp.zMax, compsMap.at(*it2).zMax);
				newComp.zMin = getMin(newComp.zMin, compsMap.at(*it2).zMin);
				newComp.size = newComp.size + compsMap.at(*it2).size;
			}
		}

		outputConnCompList.push_back(newComp);
	}
	// --------------------------------- END of [Create 3D connected component data] ---------------------------------

	return outputConnCompList;
}

set<vector<int>> ImgAnalyzer::somaDendrite_radialDetect2D(unsigned char inputImgPtr[], int xCoord, int yCoord, int imgDims[])
{
	set<vector<int>> dendriteSigSet;

	float zeroCount = 0;	
	float zeroPortion = 0;
	vector<int> intensitySeries;
	vector<vector<int>> coords;
	int round = 1;
	do
	{
		int startCoordX = xCoord - round;
		int startCoordY = yCoord - round;	
		vector<int> coord(2);
		coord[0] = startCoordX;
		coord[1] = startCoordY;
		coords.push_back(coord);
		intensitySeries.push_back(int(ImgProcessor::getPixValue2D(inputImgPtr, imgDims, startCoordX, startCoordY)));

		int displace = round * 2;
		for (int i = 1; i <= displace; ++i)
		{
			intensitySeries.push_back(int(ImgProcessor::getPixValue2D(inputImgPtr, imgDims, ++startCoordX, startCoordY)));
			vector<int> coord(2);
			coord[0] = startCoordX;
			coord[1] = startCoordY;
			coords.push_back(coord);
		}
		for (int i = 1; i <= displace; ++i)
		{
			intensitySeries.push_back(int(ImgProcessor::getPixValue2D(inputImgPtr, imgDims, startCoordX, ++startCoordY)));
			vector<int> coord(2);
			coord[0] = startCoordX;
			coord[1] = startCoordY;
			coords.push_back(coord);
		}
		for (int i = displace; i >= 1; --i)
		{
			intensitySeries.push_back(int(ImgProcessor::getPixValue2D(inputImgPtr, imgDims, --startCoordX, startCoordY)));
			vector<int> coord(2);
			coord[0] = startCoordX;
			coord[1] = startCoordY;
			coords.push_back(coord);
		}
		for (int i = displace; i >= 2; --i)
		{
			intensitySeries.push_back(int(ImgProcessor::getPixValue2D(inputImgPtr, imgDims, startCoordX, --startCoordY)));
			vector<int> coord(2);
			coord[0] = startCoordX;
			coord[1] = startCoordY;
			coords.push_back(coord);
		}
		--startCoordY;

		set<vector<int>> tempDendriteSigSet;
		for (int index = 1; index < intensitySeries.size() - 1; ++index)
		{
			if (intensitySeries.at(index) == 0) ++zeroCount;
			if (intensitySeries.at(index) > intensitySeries.at(index - 1) && intensitySeries.at(index) > intensitySeries.at(index + 1))
			{
				vector<int> coord(2);
				coord[0] = coords.at(index).at(0);
				coord[1] = coords.at(index).at(1);
				tempDendriteSigSet.insert(coord);
			}
		}

		zeroPortion = zeroCount / float(intensitySeries.size());
		if (zeroPortion > 0.5) break;
		else
		{
			dendriteSigSet.insert(tempDendriteSigSet.begin(), tempDendriteSigSet.end());
			intensitySeries.clear();
			coords.clear();
			tempDendriteSigSet.clear();
			zeroCount = 0;
		}
		++round;

		if (round > 30) break;

	} while (zeroPortion < 0.5);

	return dendriteSigSet;
}

myImg1DPtr ImgAnalyzer::connectedComponentMask2D(const vector<connectedComponent>& inputComponentList, const int imgDims[])
{
	myImg1DPtr output1Dptr(new unsigned char[imgDims[0] * imgDims[1]]);
	for (size_t i = 0; i < imgDims[0] * imgDims[1]; ++i) output1Dptr.get()[i] = 0;

	for (vector<connectedComponent>::const_iterator compIt = inputComponentList.begin(); compIt != inputComponentList.end(); ++compIt)
	{
		for (map<int, set<vector<int>>>::const_iterator sliceIt = compIt->coordSets.begin(); sliceIt != compIt->coordSets.end(); ++sliceIt)
		{
			for (set<vector<int>>::const_iterator pointIt = sliceIt->second.begin(); pointIt != sliceIt->second.end(); ++pointIt)
				output1Dptr.get()[imgDims[0] * pointIt->at(1) + pointIt->at(0)] = 255;
		}
	}

	return output1Dptr;
}

myImg1DPtr ImgAnalyzer::connectedComponentMask3D(const vector<connectedComponent>& inputComponentList, const int imgDims[])
{
	myImg1DPtr output1Dptr(new unsigned char[imgDims[0] * imgDims[1] * imgDims[2]]);
	for (size_t i = 0; i < imgDims[0] * imgDims[1] * imgDims[2]; ++i) output1Dptr.get()[i] = 0;

	for (vector<connectedComponent>::const_iterator compIt = inputComponentList.begin(); compIt != inputComponentList.end(); ++compIt)
	{
		for (map<int, set<vector<int>>>::const_iterator sliceIt = compIt->coordSets.begin(); sliceIt != compIt->coordSets.end(); ++sliceIt)
		{
			for (set<vector<int>>::const_iterator pointIt = sliceIt->second.begin(); pointIt != sliceIt->second.end(); ++pointIt)
				output1Dptr.get()[imgDims[0] * imgDims[1] * pointIt->at(2) + imgDims[0] * pointIt->at(1) + pointIt->at(0)] = 255;
		}
	}

	return output1Dptr;
}
// ===================================== END of [Image Segmentation] ===================================== //

void ImgAnalyzer::findZ4swc_maxIntensity(QList<NeuronSWC>& inputNodeList, const registeredImg& inputImg)
{
	for (QList<NeuronSWC>::iterator nodeIt = inputNodeList.begin(); nodeIt != inputNodeList.end(); ++nodeIt)
	{
		int intensity = 0;
		int imgDims[3];
		int sliceCount = 0;
		int currSliceIntensity;
		imgDims[0] = inputImg.dims[0];
		imgDims[1] = inputImg.dims[1];
		imgDims[2] = 1;
		for (map<string, myImg1DPtr>::const_iterator sliceIt = inputImg.slicePtrs.begin(); sliceIt != inputImg.slicePtrs.end(); ++sliceIt)
		{
			++sliceCount;
			currSliceIntensity = int(ImgProcessor::getPixValue2D(sliceIt->second.get(), imgDims, int(nodeIt->x), int(nodeIt->y)));
			if (currSliceIntensity > intensity)
			{
				intensity = currSliceIntensity;
				nodeIt->z = sliceCount;
			}
		}
	}
}