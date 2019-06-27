#ifndef INTEGRATEDDATASTRUCTURES_H
#define INTEGRATEDDATASTRUCTURES_H

#include <vector>
#include <set>
#include <map>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/shared_array.hpp>

#include "qstring.h"

#include "v3d_basicdatatype.h"

using namespace std;

namespace integratedDataStructures
{
	typedef boost::shared_array<unsigned char> myImg1DPtr; // --> Since GNU 4.8 hasn't adopted C++11 standard (Linux Vaa3D), 
	//     I decided to use boost's shared pointer instead of C++11's std::shared_ptr.
	typedef boost::shared_array<float> myImg1DfloatPtr;

	struct connectedComponent
	{
		int islandNum;
		map<int, set<vector<int>>> coordSets;  // The key is the number of slice. If there is only 1 slice, there will be only one pair<int, set<vector<int>>> in the map.
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

#endif