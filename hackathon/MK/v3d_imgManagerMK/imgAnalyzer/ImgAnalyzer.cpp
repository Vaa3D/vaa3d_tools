#include <set>

#include "ImgAnalyzer.h"

vector<connectedComponent> ImgAnalyzer::findConnectedComponent(vector<unsigned char**> inputSlicesVector, int dims[])
{
	unsigned char* maxIP1D = new unsigned char[dims[0] * dims[1]];
	for (int i = 0; i < dims[0] * dims[1]; ++i) maxIP1D[i] = 0;
	for (vector<unsigned char**>::iterator it = inputSlicesVector.begin(); it != inputSlicesVector.end(); ++it)
	{
		unsigned char* currSlice1D = new unsigned char[dims[0] * dims[1]];
		ImgProcessor::imageMax(currSlice1D, maxIP1D, maxIP1D, dims);

		if (currSlice1D) { delete[] currSlice1D; currSlice1D = 0; }
	}

	set<int[2]> whitePixAddress;
	unsigned char** maxIP2D = new unsigned char*[dims[1]];
	for (int j = 0; j < dims[1]; ++j)
	{
		maxIP2D[j] = new unsigned char[dims[0]];
		for (int i = 0; i < dims[0]; ++i)
		{
			maxIP2D[j][i] = maxIP1D[dims[0] * j + i];
			int whitePixCoord[2];
			whitePixCoord[0] = j;
			whitePixCoord[1] = i;
			if (maxIP2D[j][i] > 0) whitePixAddress.insert(whitePixCoord);
		}
	}

	
}