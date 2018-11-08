#include <ctime>

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
	// All components are stored in the form of ImgAnalyzer::connectedComponent.
	// Each slice is independent to one another. Therefore, the same 3D blobs are consists of certain amount of 2D "blob slices." 
	// Each 2D blob slice accounts for 1 ImgAnalyzer::connectedComponent.

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
			ImgProcessor::imgMax(currSlice1D, maxIP1D, maxIP1D, dims);
		}
		cout << " MIP done." << endl;

		delete[] currSlice1D;
		currSlice1D = nullptr;
	}
	// ------- END [Onlu get into this selection when MIP image is not provided] -------

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
	// ------- END of [Prepare white pixel address book] -------
	
	// -------------------- Finding connected components slice by slice -------------------
	int islandCount = 0;
	cout << "  -- white pixel number: " << whitePixAddress.size() << endl;
	cout << "Processing slices: ";
	for (vector<unsigned char**>::iterator sliceIt = inputSlicesVector.begin(); sliceIt != inputSlicesVector.end(); ++sliceIt)
	{
		int sliceNum = int(sliceIt - inputSlicesVector.begin());
		cout << sliceNum << " ";
		for (set<vector<int>>::iterator mipIt = whitePixAddress.begin(); mipIt != whitePixAddress.end(); ++mipIt)
		{
			if ((*sliceIt)[mipIt->at(0)][mipIt->at(1)] > 0)
			{
				bool connected = false;
				for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
				{
					if (connIt->zMax != sliceNum) continue;
					else
					{
						for (set<vector<int>>::iterator it = connIt->coordSets[sliceNum].begin(); it != connIt->coordSets[sliceNum].end(); ++it)
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