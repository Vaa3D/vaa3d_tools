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

		if (current_exception) { delete[] currSlice1D; currSlice1D = 0; }
	}
}