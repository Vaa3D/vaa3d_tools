#include <ctime>

#include "ImgAnalyzer.h"

vector<connectedComponent> ImgAnalyzer::findConnectedComponent(vector<unsigned char**> inputSlicesVector, int dims[])
{
	vector<connectedComponent> connList;

	unsigned char* maxIP1D = new unsigned char[dims[0] * dims[1]];
	for (int i = 0; i < dims[0] * dims[1]; ++i) maxIP1D[i] = 0;
	for (vector<unsigned char**>::iterator it = inputSlicesVector.begin(); it != inputSlicesVector.end(); ++it)
	{
		unsigned char* currSlice1D = new unsigned char[dims[0] * dims[1]];
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

		if (currSlice1D) { delete[] currSlice1D; currSlice1D = 0; }
	}
	cout << " MIP done." << endl;
	string testName = "Z:/test.tif";
	const char* testNameC = testName.c_str();
	V3DLONG saveDims[4];
	saveDims[0] = dims[0];
	saveDims[1] = dims[1];
	saveDims[2] = 1;
	saveDims[3] = 1;
	ImgManager::saveimage_wrapper(testNameC, maxIP1D, saveDims, 1);

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

	if (inputSlicesVector.size() == 1)
	{
		int islandCount = 0;
		for (set<vector<int> >::iterator it = whitePixAddress.begin(); it != whitePixAddress.end(); ++it)
		{
			int connectedCount = 0;
			for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
			{
				for (multimap<int, vector<int> >::iterator coordIt = connIt->coords.begin(); coordIt != connIt->coords.end(); ++coordIt)
				{
					if ((it->at(0) - 1 == coordIt->second.at(0) || it->at(0) + 1 == coordIt->second.at(0)) && (it->at(1) - 1 == coordIt->second.at(1) || it->at(1) + 1 == coordIt->second.at(1)))
					{
						vector<int> newCoord(2);
						newCoord[0] = it->at(0);
						newCoord[1] = it->at(1);
						connIt->coords.insert(pair<int, vector<int> >(0, newCoord));
						++connectedCount;
						break;
					}
				}
			}

			if (connectedCount == 0)
			{
				++islandCount;
				connectedComponent newIsland;
				newIsland.islandNum = islandCount;
				newIsland.coords.insert(pair<int, vector<int> >(0, *it));
				connList.push_back(newIsland);
			}
		}

		vector<vector<connectedComponent>::iterator> pos;
		for (vector<connectedComponent>::iterator it = connList.begin(); it != connList.end(); ++it)
		{
			for (vector<connectedComponent>::iterator checkIt = it + 1; checkIt != connList.end(); ++checkIt)
			{
				if (it->coords == checkIt->coords)
				{
					if (find(pos.begin(), pos.end(), checkIt) == pos.end()) pos.push_back(checkIt);
				}
			}
		}
		for (vector<vector<connectedComponent>::iterator>::iterator eraseIt = pos.begin(); eraseIt != pos.end(); ++eraseIt) connList.erase(*eraseIt);

		return connList;
	}
	else
	{
		int islandCount = 0;
		cout << "white pixel number: " << whitePixAddress.size() << endl;
		for (vector<unsigned char**>::iterator sliceIt = inputSlicesVector.begin(); sliceIt != inputSlicesVector.end(); ++sliceIt)
		{
			int sliceNum = int(sliceIt - inputSlicesVector.begin());
			cout << "  processing slice " << sliceNum << ".." << endl;
			for (set<vector<int> >::iterator mipIt = whitePixAddress.begin(); mipIt != whitePixAddress.end(); ++mipIt)
			{
				if ((*sliceIt)[mipIt->at(0)][mipIt->at(1)] > 0)
				{
					//cout << "coordinates: (" << mipIt->at(0) << ", " << mipIt->at(1) << ")" << endl;
					bool connected = false;
					for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
					{
						pair<multimap<int, vector<int> >::iterator, multimap<int, vector<int> >::iterator> lastSliceRange = connIt->coords.equal_range(sliceNum - 1);
						pair<multimap<int, vector<int> >::iterator, multimap<int, vector<int> >::iterator> currSliceRange = connIt->coords.equal_range(sliceNum);
						for (multimap<int, vector<int> >::iterator rangeIt = lastSliceRange.first; rangeIt != lastSliceRange.second; ++rangeIt)
						{
							if (rangeIt->second.at(0) >= mipIt->at(0) - 1 && rangeIt->second.at(0) <= mipIt->at(0) + 1 && 
								rangeIt->second.at(1) >= mipIt->at(1) - 1 && rangeIt->second.at(1) <= mipIt->at(1) + 1)
							{ 
								bool duplicate = false;
								for (multimap<int, vector<int> >::iterator dupCheckIt = currSliceRange.first; dupCheckIt != currSliceRange.second; ++dupCheckIt)
								{
									if (dupCheckIt->second.at(0) == mipIt->at(0) && dupCheckIt->second.at(1) == mipIt->at(1) && dupCheckIt->second.at(2) == sliceNum)
									{
										duplicate = true;
										break;
									}
								}

								if (!duplicate)
								{
									vector<int> newCoord(3);
									newCoord[0] = mipIt->at(0);
									newCoord[1] = mipIt->at(1);
									newCoord[2] = sliceNum;
									connIt->coords.insert(pair<int, vector<int> >(sliceNum, newCoord));
									connected = true;
									break;
								}
								else break;

								/*cout << connIt->islandNum << ":" << endl;
								for (multimap<int, vector<int> >::iterator currCoordIt = connIt->coords.begin(); currCoordIt != connIt->coords.end(); ++currCoordIt)
									cout << " [" << currCoordIt->second.at(0) << ", " << currCoordIt->second.at(1) << ", " << currCoordIt->second.at(2) << "] ";

								cout << endl;*/
							}
						}
						for (multimap<int, vector<int> >::iterator rangeIt = currSliceRange.first; rangeIt != currSliceRange.second; ++rangeIt)
						{
							if (rangeIt->second.at(0) >= mipIt->at(0) - 1 && rangeIt->second.at(0) <= mipIt->at(0) + 1 && 
								rangeIt->second.at(1) >= mipIt->at(1) - 1 && rangeIt->second.at(1) <= mipIt->at(1) + 1)
							{
								bool duplicate = false;
								for (multimap<int, vector<int> >::iterator dupCheckIt = currSliceRange.first; dupCheckIt != currSliceRange.second; ++dupCheckIt)
								{
									if (dupCheckIt->second.at(0) == mipIt->at(0) && dupCheckIt->second.at(1) == mipIt->at(1) && dupCheckIt->second.at(2) == sliceNum)
									{
										duplicate = true;
										break;
									}
								}

								if (!duplicate)
								{
									vector<int> newCoord(3);
									newCoord[0] = mipIt->at(0);
									newCoord[1] = mipIt->at(1);
									newCoord[2] = sliceNum;
									connIt->coords.insert(pair<int, vector<int> >(sliceNum, newCoord));
									connected = true;
									break;
								}
								else break;

								/*cout << connIt->islandNum << ":" << endl;
								for (multimap<int, vector<int> >::iterator currCoordIt = connIt->coords.begin(); currCoordIt != connIt->coords.end(); ++currCoordIt)
									cout << " [" << currCoordIt->second.at(0) << "," << currCoordIt->second.at(1) << "," << currCoordIt->second.at(2) << "] ";

								cout << endl;*/
							}
						}
					}

					if (!connected)
					{
						++islandCount;
						connectedComponent newIsland;
						newIsland.islandNum = islandCount;
						vector<int> newCoord(3);
						newCoord[0] = mipIt->at(0);
						newCoord[1] = mipIt->at(1);
						newCoord[2] = sliceNum;
						newIsland.coords.insert(pair<int, vector<int> >(sliceNum, newCoord));
						connList.push_back(newIsland);
					}
				}
			}
		}

		bool merge = true;
		int merging1, merging2;
		while (merge)
		{
			for (vector<connectedComponent>::iterator it = connList.begin(); it != connList.end() - 1; ++it)
			{
				for (multimap<int, vector<int> >::iterator compIt = it->coords.begin(); compIt != it->coords.end(); ++compIt)
				{
					for (vector<connectedComponent>::iterator checkIt = it + 1; checkIt != connList.end(); ++checkIt)
					{
						for (multimap<int, vector<int> >::iterator checkCompIt = checkIt->coords.begin(); checkCompIt != checkIt->coords.end(); ++checkCompIt)
						{
							if (compIt->second.at(0) >= checkCompIt->second.at(0) - 1 && compIt->second.at(0) <= checkCompIt->second.at(0) + 1 &&
								compIt->second.at(1) >= checkCompIt->second.at(1) - 1 && compIt->second.at(1) <= checkCompIt->second.at(1) + 1 &&
								compIt->second.at(2) >= checkCompIt->second.at(2) - 1 && compIt->second.at(2) <= checkCompIt->second.at(2) + 1)
							{
								for (multimap<int, vector<int> >::iterator mergeIt = checkIt->coords.begin(); mergeIt != checkIt->coords.end(); ++mergeIt)
									it->coords.insert(pair<int, vector<int> >(mergeIt->first, mergeIt->second));

								merging1 = it->islandNum;
								merging2 = checkIt->islandNum;
								connList.erase(checkIt);
								goto MERGE_CHECKPOINT;
							}
						}
					}
				}
			}
			merge = false;

		MERGE_CHECKPOINT:
			cout << "merging component " << merging1 << " and " << merging2 << endl;
		}

		/*for (vector<connectedComponent>::iterator finalIt = connList.begin(); finalIt != connList.end(); ++finalIt)
		{
			for (multimap<int, vector<int> >::iterator finalCoordsIt = finalIt->coords.begin(); finalCoordsIt != finalIt->coords.end(); ++finalCoordsIt)
				finalIt->finalCoords.insert(pair<int, vector<int> >(finalCoordsIt->first, finalCoordsIt->second));

			finalIt->size = finalIt->finalCoords.size();
		}*/

		return connList;
	}
}