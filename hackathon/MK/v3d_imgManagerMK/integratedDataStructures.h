#ifndef INTEGRATEDDATASTRUCTURES_H
#define INTEGRATEDDATASTRUCTURES_H

#include <iostream>
#include <vector>
#include <set>
#include <map>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/shared_array.hpp>

#include "qstring.h"

#include "basic_surf_objs.h"
#include "v3d_basicdatatype.h"

using namespace std;

namespace integratedDataStructures
{
	using myImg1DPtr = boost::shared_array<unsigned char>; // --> Since GNU 4.8 hasn't adopted C++11 standard (Linux Vaa3D), 
														   //     I decided to use boost's shared pointer instead of C++11's std::shared_ptr.
	using myImg1DfloatPtr = boost::shared_array<float>;

	/***************** Structuering Element for 2D Morphological Operations *****************/
	struct morphStructElement2D
	{
		// The default constructor sets disk shape with both axes length = 5.
		// The constructor also takes different lengths for the 2 axes if oval or rectangular structuring element is desired.

		enum shape { disk };

		morphStructElement2D(shape structEleShape = morphStructElement2D::disk, int length = 5);
		morphStructElement2D(string shape, int length = 5);
		morphStructElement2D(string shape, int xLength, int yLength);
		~morphStructElement2D();

		string eleShape;
		shape structEleShape;
		int xLength, yLength, radius;

		unsigned char* structElePtr;
		inline void printOutStructEle(); // Prints out the element row by row.
	};
	/****************************************************************************************/

	struct connectedComponent
	{
		int islandNum;
		map<int, set<vector<int>>> coordSets;  // The key is the number of slice. If there is only 1 slice, there will be only one pair<int, set<vector<int>>> in the map.
											   // This data member will be revised to be boost's associate container for efficiency purposes.
		boost::container::flat_map<int, boost::container::flat_set<vector<int>>> surfaceCoordSets;
		boost::container::flat_set<vector<int>> xyProjection;
		boost::container::flat_set<vector<int>> yzProjection;
		boost::container::flat_set<vector<int>> xzProjection;
		int xMax, xMin, yMax, yMin, zMax, zMin;
		int size;
		float ChebyshevCenter[3];

		void getConnCompSurface();
		void getXYZprojections();
	};

	inline void ChebyshevCenter_connComp(connectedComponent& inputComp);     // The Chebyshev center will be stored in the input connectedComponent::chebyshevCenter.
	inline void ChebyshevCenter(set<vector<int>> allCoords, float center[]); // The Chebyshev center will be stored in the input center array point.
	inline void ChebyshevCenter_connCompList(vector<connectedComponent>& inputCompList);
	inline vector<int> getSliceBoundaries(boost::container::flat_set<vector<int>> inputSliceCoordSet);

	struct brainRegion
	{
		string name;
		vector<connectedComponent> regionBodies;

		void writeBrainRegion_file(string saveFileName);
		void readBrainRegion_file(string inputFileName);
	};

	struct registeredImg
	{
		string imgAlias;
		QString imgCaseRootQ;

		void createBlankImg(const int imgDims[]);
		map<string, myImg1DPtr> slicePtrs;
		map<string, myImg1DfloatPtr> floatSlicePtrs;

		map<int, size_t> histMap;
		map<int, double> histMap_log10;
		void getHistMap_no0();
		void getHistMap_no0_log10();

		int dims[4];
		ImagePixelType dataType;
	};
}

inline void integratedDataStructures::morphStructElement2D::printOutStructEle()
{
	for (int j = 0; j < this->radius * 2 + 1; ++j)
	{
		for (int i = 0; i < this->radius * 2 + 1; ++i)
			cout << int(this->structElePtr[(this->radius * 2 + 1)*j + i]) << " ";
		cout << endl;
	}
}	

void integratedDataStructures::ChebyshevCenter(set<vector<int>> allCoords, float center[])
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

inline void integratedDataStructures::ChebyshevCenter_connComp(connectedComponent& inputComp)
{
	set<vector<int>> allCoords;
	for (map<int, set<vector<int>>>::iterator sliceIt = inputComp.coordSets.begin(); sliceIt != inputComp.coordSets.end(); ++sliceIt)
		allCoords.insert(sliceIt->second.begin(), sliceIt->second.end());

	float center[3];
	integratedDataStructures::ChebyshevCenter(allCoords, center);

	inputComp.ChebyshevCenter[0] = center[0];
	inputComp.ChebyshevCenter[1] = center[1];
	inputComp.ChebyshevCenter[2] = center[2];

	//cout << inputComp.ChebyshevCenter[0] << " " << inputComp.ChebyshevCenter[1] << " " << inputComp.ChebyshevCenter[2] << endl;
}

inline void integratedDataStructures::ChebyshevCenter_connCompList(vector<connectedComponent>& inputCompList)
{
	for (vector<connectedComponent>::iterator it = inputCompList.begin(); it != inputCompList.end(); ++it)
		integratedDataStructures::ChebyshevCenter_connComp(*it);
}

inline vector<int> integratedDataStructures::getSliceBoundaries(boost::container::flat_set<vector<int>> inputSliceCoordSet)
{
	int xMax = 0, yMax = 0;
	int xMin = 100000, yMin = 100000;
	for (boost::container::flat_set<vector<int>>::iterator it = inputSliceCoordSet.begin(); it != inputSliceCoordSet.end(); ++it)
	{
		if (it->at(0) > xMax) xMax = it->at(0);
		if (it->at(0) < xMin) xMin = it->at(0);
		if (it->at(1) > yMax) yMax = it->at(1);
		if (it->at(1) < yMin) yMin = it->at(1);
	}

	vector<int> outputVec = { xMin, xMax, yMin, yMax };
	return outputVec;
}

#endif