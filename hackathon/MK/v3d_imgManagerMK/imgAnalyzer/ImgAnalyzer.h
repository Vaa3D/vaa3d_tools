#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cmath>

#include <qlist.h>

#include "ImgManager.h"
#include "ImgProcessor.h"

using namespace std;

struct morphStructElement
{
	std::string eleShape;
	int xLength, yLength;

	morphStructElement();
	morphStructElement(string shape);
	morphStructElement(string shape, int length1, int length2);

	vector<vector<int> > structEle2D;
	vector<vector<vector<int> > > structEle3D;
};

struct connectedComponent
{
	int islandNum;
	map<int, set<vector<int> > > coordSets;
	int xMax, xMin, yMax, yMin, zMax, zMin;
	long int size;
	vector<float> ChebyshevCenter;
};

class ImgAnalyzer
{
public:
	static vector<connectedComponent> findConnectedComponent(vector<unsigned char**> inputSlicesVector, int imgDims[]);
	
	static inline vector<float> ChebyshevCenter(connectedComponent inputComp);

private:
	void mergeConnComponent(vector<connectedComponent>& inputConnCompList);
};

#endif

inline vector<float> ImgAnalyzer::ChebyshevCenter(connectedComponent inputComp)
{
	set<vector<int> > allCoords;
	for (map<int, set<vector<int> > >::iterator sliceIt = inputComp.coordSets.begin(); sliceIt != inputComp.coordSets.end(); ++sliceIt)
		allCoords.insert(sliceIt->second.begin(), sliceIt->second.end());
	
	vector<float> center(3);
	float lengthSum = 1000000;
	for (set<vector<int> >::iterator allCoordIt = allCoords.begin(); allCoordIt != allCoords.end(); ++allCoordIt)
	{
		float currLengthSum = 0;
		for (set<vector<int> >::iterator checkCoordIt = allCoords.begin(); checkCoordIt != allCoords.end(); ++checkCoordIt)
		{
			float length = sqrt( (checkCoordIt->at(0) - allCoordIt->at(0)) * (checkCoordIt->at(0) - allCoordIt->at(0)) +
				                 (checkCoordIt->at(1) - allCoordIt->at(1)) * (checkCoordIt->at(1) - allCoordIt->at(1)) +
								 (checkCoordIt->at(2) - allCoordIt->at(2)) * (checkCoordIt->at(2) - allCoordIt->at(2)) );

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
	inputComp.ChebyshevCenter = center;

	return center;
}