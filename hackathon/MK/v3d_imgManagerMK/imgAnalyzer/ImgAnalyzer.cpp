#include <ctime>

#include "ImgAnalyzer.h"

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
	/*string testName = "Z:/test.tif";
	const char* testNameC = testName.c_str();
	V3DLONG saveDims[4];
	saveDims[0] = dims[0];
	saveDims[1] = dims[1];
	saveDims[2] = 1;
	saveDims[3] = 1;
	ImgManager::saveimage_wrapper(testNameC, maxIP1D, saveDims, 1);*/

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
					if (mipIt->at(0) + 2 < connIt->xMin && mipIt->at(0) - 2 > connIt->xMax &&
						mipIt->at(1) + 2 < connIt->yMin && mipIt->at(1) - 2 > connIt->yMax &&
						connIt->zMin > sliceNum + 2 && connIt->zMax < sliceNum - 2) goto NEW_SIGNAL_VECTOR;
					else
					{
						for (set<vector<int> >::iterator it1 = connIt->coordSets[sliceNum - 1].begin(); it1 != connIt->coordSets[sliceNum - 1].end(); ++it1)
						{
							if (it1->at(0) >= mipIt->at(0) - 1 && it1->at(0) <= mipIt->at(0) + 1 &&
								it1->at(1) >= mipIt->at(1) - 1 && it1->at(1) <= mipIt->at(1) + 1)
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

								connIt->zMax = sliceNum;

								connected = true;
								goto SIGNAL_VECTOR_INSERTED;
							}
						}
						for (set<vector<int> >::iterator it2 = connIt->coordSets[sliceNum].begin(); it2 != connIt->coordSets[sliceNum].end(); ++it2)
						{
							if (it2->at(0) >= mipIt->at(0) - 1 && it2->at(0) <= mipIt->at(0) + 1 &&
								it2->at(1) >= mipIt->at(1) - 1 && it2->at(1) <= mipIt->at(1) + 1)
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

								connIt->zMax = sliceNum;

								connected = true;
								goto SIGNAL_VECTOR_INSERTED;
							}
						}
					}
				}

			NEW_SIGNAL_VECTOR:
				{
					if (!connected)
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
				}

			SIGNAL_VECTOR_INSERTED:
				continue;
			}
		}
	}
	cout << endl;

	return connList;
}

void ImgAnalyzer::mergeConnComponent(vector<connectedComponent>& inputConnCompList)
{
	bool merge = true;
	int merging1, merging2;
	while (merge)
	{
		for (vector<connectedComponent>::iterator it = inputConnCompList.begin(); it != inputConnCompList.end() - 1; ++it)
		{
			for (map<int, set<vector<int> > >::iterator compIt = it->coordSets.begin(); compIt != it->coordSets.end(); ++compIt)
			{
				for (set<vector<int> >::iterator compSliceIt = compIt->second.begin(); compSliceIt != compIt->second.end(); ++compSliceIt)
				{
					for (vector<connectedComponent>::iterator checkIt = it + 1; checkIt != inputConnCompList.end(); ++checkIt)
					{
						if (checkIt->xMin > it->xMax + 2 || checkIt->xMax < it->xMin - 2 ||
							checkIt->yMin > it->yMax + 2 || checkIt->yMax < it->yMin - 2 ||
							checkIt->zMin > it->zMax + 2 || checkIt->zMax < it->zMin - 2) continue;

						for (map<int, set<vector<int> > >::iterator checkSetIt = checkIt->coordSets.begin(); checkSetIt != checkIt->coordSets.end(); ++checkSetIt)
						{
							for (set<vector<int> >::iterator checkSliceIt = checkSetIt->second.begin(); checkSliceIt != checkSetIt->second.end(); ++checkSliceIt)
							{
								if (compSliceIt->at(0) >= checkSliceIt->at(0) - 1 && compSliceIt->at(0) <= checkSliceIt->at(0) + 1 &&
									compSliceIt->at(1) >= checkSliceIt->at(1) - 1 && compSliceIt->at(1) <= checkSliceIt->at(1) + 1 &&
									compSliceIt->at(2) >= checkSliceIt->at(2) - 1 && compSliceIt->at(2) <= checkSliceIt->at(2) + 1)
								{
									for (map<int, set<vector<int> > >::iterator addSetIt = checkIt->coordSets.begin(); addSetIt != checkIt->coordSets.end(); ++addSetIt)
									{
										for (set<vector<int> >::iterator addVectorIt = addSetIt->second.begin(); addVectorIt != addSetIt->second.end(); ++addVectorIt)
											it->coordSets[addSetIt->first].insert(*addVectorIt);

										merging1 = it->islandNum;
										merging2 = checkIt->islandNum;
										inputConnCompList.erase(checkIt);
										goto MERGE_CHECKPOINT;
									}
								}
							}
						}
					}
				}
			}
		}
		merge = false;

	MERGE_CHECKPOINT:
		cout << "merging components " << merging1 << " and " << merging2 << "    component number left: " << inputConnCompList.size() << endl;
	}
}