#include <ctime>

#include "ImgAnalyzer.h"

// ======================================= Image Segmentation ======================================= //
vector<connectedComponent> ImgAnalyzer::findSignalBlobs_2Dcombine(vector<unsigned char**> inputSlicesVector, int dims[], unsigned char maxIP1D[])
{
	// -- For the simplicity when specifying an element in the slice, I decided to use 2D array to represent each slice: vector<unsigned char**>.

	vector<connectedComponent> connList;
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
				for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
				{
					if (connIt->zMax != sliceNum) continue;
					else
					{
						for (set<vector<int>>::iterator it = connIt->coordSets[sliceNum].begin(); it != connIt->coordSets[sliceNum].end(); ++it)
						{
						/******* IMPORATANT NOTE: Vaa3D coord system's x and y are flipped from TIF. Needs to switch x and when assigning x and y to swc from tif image! *******/
							if (it->at(0) >= mipIt->at(1) - 1 && it->at(0) <= mipIt->at(1) + 1 &&
								it->at(1) >= mipIt->at(0) - 1 && it->at(1) <= mipIt->at(0) + 1) // using 8-connectivity
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