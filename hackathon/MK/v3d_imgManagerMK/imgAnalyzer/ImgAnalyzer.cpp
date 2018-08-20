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
				for (set<vector<int> >::iterator coordIt = connIt->coords.begin(); coordIt != connIt->coords.end(); ++coordIt)
				{
					if ((it->at(0) - 1 == coordIt->at(0) || it->at(0) + 1 == coordIt->at(0)) && (it->at(1) - 1 == coordIt->at(1) || it->at(1) + 1 == coordIt->at(1)))
					{
						vector<int> newCoord(2);
						newCoord[0] = it->at(0);
						newCoord[1] = it->at(1);
						connIt->coords.insert(newCoord);
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
				newIsland.coords.insert(*it);
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
			cout << "  processing slice " << sliceNum + 1 << ".." << endl;
			for (set<vector<int> >::iterator mipIt = whitePixAddress.begin(); mipIt != whitePixAddress.end(); ++mipIt)
			{
				if ((*sliceIt)[mipIt->at(0)][mipIt->at(1)] > 0)
				{
					//cout << "coordinates: (" << mipIt->at(0) << ", " << mipIt->at(1) << ")" << endl;
					int connectedCount = 0;
					for (vector<connectedComponent>::iterator connIt = connList.begin(); connIt != connList.end(); ++connIt)
					{
						for (set<vector<int> >::iterator coordIt = connIt->coords.begin(); coordIt != connIt->coords.end(); ++coordIt)
						{
							if (coordIt->at(2) == sliceNum - 1)
							{
								if ((mipIt->at(0) - 1 == coordIt->at(0) || mipIt->at(0) + 1 == coordIt->at(0)) && (mipIt->at(1) - 1 == coordIt->at(1) || mipIt->at(1) + 1 == coordIt->at(1)))
								{
									vector<int> newCoord(3);
									newCoord[0] = mipIt->at(0);
									newCoord[1] = mipIt->at(1);
									newCoord[2] = sliceNum;
									connIt->coords.insert(newCoord);
									++connectedCount;
									break;
								}
							}
						}
					}

					if (connectedCount == 0)
					{
						++islandCount;
						connectedComponent newIsland;
						newIsland.islandNum = islandCount;
						vector<int> newCoord(3);
						newCoord[0] = mipIt->at(0);
						newCoord[1] = mipIt->at(1);
						newCoord[2] = sliceNum;
						newIsland.coords.insert(newCoord);
						connList.push_back(newIsland);
					}
				}
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
}