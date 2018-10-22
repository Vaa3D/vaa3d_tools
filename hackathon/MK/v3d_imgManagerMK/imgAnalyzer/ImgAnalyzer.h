#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cmath>

using namespace std;

struct morphStructElement
{
	std::string eleShape;
	int xLength, yLength;

	morphStructElement();
	morphStructElement(string shape);
	morphStructElement(string shape, int length1, int length2);

	vector<vector<int>> structEle2D;
	vector<vector<vector<int>>> structEle3D;
};

struct connectedComponent
{
	int islandNum;
	map<int, set<vector<int>>> coordSets;
	int xMax, xMin, yMax, yMin, zMax, zMin;
	int size;
	float ChebyshevCenter[3];
};

class ImgAnalyzer
{
public:
	vector<connectedComponent> findSignalBlobs_2Dcombine(vector<unsigned char**> inputSlicesVector, int imgDims[], unsigned char* maxIP1D = nullptr);

	static inline void ChebyshevCenter_connComp(connectedComponent& inputComp);
	static inline void ChebyshevCenter(set<vector<int>> allCoords, float center[]);
};

inline void ImgAnalyzer::ChebyshevCenter(set<vector<int>> allCoords, float center[])
{
	float lengthSum = 1000000;
	for (set<vector<int>>::iterator allCoordIt = allCoords.begin(); allCoordIt != allCoords.end(); ++allCoordIt)
	{
		float currLengthSum = 0;
		for (set<vector<int>>::iterator checkCoordIt = allCoords.begin(); checkCoordIt != allCoords.end(); ++checkCoordIt)
		{
			float length = sqrt(float((checkCoordIt->at(0) - allCoordIt->at(0)) * (checkCoordIt->at(0) - allCoordIt->at(0)) +
									  (checkCoordIt->at(1) - allCoordIt->at(1)) * (checkCoordIt->at(1) - allCoordIt->at(1)) +
									  (checkCoordIt->at(2) - allCoordIt->at(2)) * (checkCoordIt->at(2) - allCoordIt->at(2))));

			currLengthSum += length;
		}

		if (currLengthSum < lengthSum)
		{
			center[0] = allCoordIt->at(0);
			center[1] = allCoordIt->at(1);
			center[2] = allCoordIt->at(2);
			lengthSum = currLengthSum;
		}
	}

	//cout << center[0] << " " << center[1] << " " << center[2] << endl;
}

inline void ImgAnalyzer::ChebyshevCenter_connComp(connectedComponent& inputComp)
{
	set<vector<int>> allCoords;
	for (map<int, set<vector<int>>>::iterator sliceIt = inputComp.coordSets.begin(); sliceIt != inputComp.coordSets.end(); ++sliceIt)
		allCoords.insert(sliceIt->second.begin(), sliceIt->second.end());

	float center[3];
	ImgAnalyzer::ChebyshevCenter(allCoords, center);

	inputComp.ChebyshevCenter[0] = center[0];
	inputComp.ChebyshevCenter[1] = center[1];
	inputComp.ChebyshevCenter[2] = center[2];
	
	//cout << inputComp.ChebyshevCenter[0] << " " << inputComp.ChebyshevCenter[1] << " " << inputComp.ChebyshevCenter[2] << endl;
}

#endif