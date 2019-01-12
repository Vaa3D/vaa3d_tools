#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <cmath>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string.hpp>

#include "ImgManager.h"
#include "ImgProcessor.h"

using namespace std;

struct connectedComponent
{
	int islandNum;
	map<int, set<vector<int>>> coordSets;  // The key is the number of slice. If there is only 1 slice, there will be only one pair<int, set<vector<int>>> in the map.
	int xMax, xMin, yMax, yMin, zMax, zMin;
	int size;
	float ChebyshevCenter[3];
};

class ImgAnalyzer
{
public:
	/***************** Image Segmentation *****************/

	// [findSignalBlobs] finds connected components from a image statck using slice-by-slice approach. All components are stored in the form of ImgAnalyzer::connectedComponent.
	vector<connectedComponent> findSignalBlobs(vector<unsigned char**> inputSlicesVector, int imgDims[], int distThre, unsigned char* maxIP1D = nullptr);

	// This method is called by ImgAnalyzer::merge2DConnComponent. Standalone use hasn't been tested yest.
	static vector<connectedComponent> merge2DConnComponent(const vector<connectedComponent>& inputConnCompList);

	boost::container::flat_set<deque<float>> getSectionalCentroids(const connectedComponent& inputConnComp);
private:
	boost::container::flat_set<deque<float>> ImgAnalyzer::connCompSectionalProc(vector<int>& dim1, vector<int>& dim2, vector<int>& sectionalDim, int secDimStart, int secDimEnd);

public:
	// Depicts skeleton for star-fish-like object with a given starting point (center), using the intensity profiles of those pixels circling the center.
	// This method was aimed to capture dendrites on IVSCC images, but proven to be ineffective due to high image noise level.
	set<vector<int>> somaDendrite_radialDetect2D(unsigned char inputImgPtr[], int xCoord, int yCoord, int imgDims[]);

	myImg1DPtr connectedComponentMask2D(const vector<connectedComponent>& inputComponentList, const int imgDims[]);
	myImg1DPtr connectedComponentMask3D(const vector<connectedComponent>& inputComponentList, const int imgDims[]);
	/******************************************************/

	static void findZ4swc_maxIntensity(QList<NeuronSWC>& inputNodeList, const registeredImg& inputImg);

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