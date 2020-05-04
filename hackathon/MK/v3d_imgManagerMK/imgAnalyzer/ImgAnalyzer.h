#ifndef IMGANALYZER_H
#define IMGANALYZER_H

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/algorithm/string.hpp>

#include "ImgProcessor.h"

using namespace integratedDataStructures;

class ImgAnalyzer
{	
public:
	ImgAnalyzer() : blobMergingReport(false) {};

	enum processName { blobMerging };

	/***************** Image Segmentation/Detection *****************/
	// [findSignalBlobs] finds connected components from a image statck using slice-by-slice approach. All components are stored in the form of ImgAnalyzer::connectedComponent.
	vector<connectedComponent> findSignalBlobs(const vector<unsigned char**>& inputSlicesVector, const int imgDims[], const int distThre, unsigned char* maxIP1D = nullptr);
	
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
	boost::container::flat_set<deque<float>> connCompSectionalProc(const vector<int>& dim1, const vector<int>& dim2, const vector<int>& sectionalDim, const int secDimStart, const int secDimEnd);
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

#endif