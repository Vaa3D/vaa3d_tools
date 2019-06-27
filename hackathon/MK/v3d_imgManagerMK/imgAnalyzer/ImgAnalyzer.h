#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string.hpp>

#include "ImgManager.h"
#include "ImgProcessor.h"
#include "integratedDataStructures.h"

using namespace std;
using namespace integratedDataStructures;

class ImgAnalyzer
{	
public:
	ImgAnalyzer();

	enum processName { blobMerging };

	/***************** Image Segmentation/Detection *****************/
	// [findSignalBlobs] finds connected components from a image statck using slice-by-slice approach. All components are stored in the form of ImgAnalyzer::connectedComponent.
	vector<connectedComponent> findSignalBlobs(vector<unsigned char**> inputSlicesVector, int imgDims[], int distThre, unsigned char* maxIP1D = nullptr);
	static inline void ChebyshevCenter_connComp(connectedComponent& inputComp);     // The Chebyshev center will be stored in the input connectedComponent::chebyshevCenter.
	static inline void ChebyshevCenter(set<vector<int>> allCoords, float center[]); // The Chebyshev center will be stored in the input center array point.
	static inline void ChebyshevCenter_connCompList(vector<connectedComponent>& inputCompList);

	myImg1DPtr connectedComponentMask2D(const vector<connectedComponent>& inputComponentList, const int imgDims[]); // Generates 2D mask with input connected component list.
	myImg1DPtr connectedComponentMask3D(const vector<connectedComponent>& inputComponentList, const int imgDims[]); // Generates 3D mask with input connected component list.

private:
	// This method is called by ImgAnalyzer::findSignalBlobs because of its slice-by-slice approach. 
	vector<connectedComponent> merge2DConnComponent(const vector<connectedComponent>& inputConnCompList);
	/****************************************************************/



	/***************** Image Analysis *****************/
public:
	// Identify the centroids of 2D connected components from every sectional plane in x, y, and z direction.
	boost::container::flat_set<deque<float>> getSectionalCentroids(const connectedComponent& inputConnComp);
private:
	// This method is called by ImgAnalyzer::getSectionalCentroids to complete the task.
	boost::container::flat_set<deque<float>> connCompSectionalProc(vector<int>& dim1, vector<int>& dim2, vector<int>& sectionalDim, int secDimStart, int secDimEnd);
	/**************************************************/



public:
	// Depicts skeleton for star-fish-like object with a given starting point (center), using the intensity profiles of those pixels circling the center.
	// This method was aimed to capture dendrites on IVSCC images, but proven to be ineffective due to high image noise level.
	set<vector<int>> somaDendrite_radialDetect2D(unsigned char inputImgPtr[], int xCoord, int yCoord, int imgDims[]);

	// Locate z location for auto-reaced SWC generated based on MIP image.
	static void findZ4swc_maxIntensity(QList<NeuronSWC>& inputNodeList, const registeredImg& inputImg);
	/******************************************************/


	
	/***************** Process Monitoring *****************/
	condition_variable monitorSwitch;
	mutex blobMergingMutex;

	void reportProcess(ImgAnalyzer::processName processName);
	bool blobMergingReport;
	int progressReading;
	/******************************************************/
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

inline void ImgAnalyzer::ChebyshevCenter_connCompList(vector<connectedComponent>& inputCompList)
{
	for (vector<connectedComponent>::iterator it = inputCompList.begin(); it != inputCompList.end(); ++it) 
		ImgAnalyzer::ChebyshevCenter_connComp(*it);
}

#endif