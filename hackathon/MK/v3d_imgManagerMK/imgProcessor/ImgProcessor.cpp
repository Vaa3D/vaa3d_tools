#include <iostream>

#include "ImgProcessor.h"

using namespace std;

ImgProcessor::ImgProcessor(MIPOrientation MIP_Direction)
{
	this->MIPDirection = MIP_Direction;
}

void ImgProcessor::maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[], long int xDim, long int yDim, long int zDim)
{
	if (this->MIPDirection == Mxy) // need to check in which direction to generate MIP? no template implementation for now
	{
		for (long int yi = 0; yi < yDim; ++yi)
		{
			for (long int xi = 0; xi < xDim; ++xi)
			{
				short int maxVal = 0;
				for (long int zi = 0; zi < zDim; ++zi)
				{
					short int curValue = inputVOIPtr[xDim*yDim*zi + xDim*yi + xi];
					if (curValue > maxVal) maxVal = curValue;
				}
				OutputImage2DPtr[xDim*yi + xi] = (unsigned char)(maxVal);
			}
		}
	}
}

void ImgProcessor::shapeMask2D(int imgDims[2], unsigned char* outputMask1D, int coords[2], int regionDims[2], string shape) // need revision
{
	outputMask1D = new unsigned char[imgDims[0] * imgDims[1]];
	int xlb = coords[0] - regionDims[0] / 2;
	int xhb = coords[0] + regionDims[0] / 2;
	int ylb = coords[1] - regionDims[1] / 2;
	int yhb = coords[2] + regionDims[1] / 2;

	for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputMask1D[i] = 0;
	
	for (int j = ylb - 1; j < yhb; ++j)
	{
		for (int i = xlb - 1; i < xhb; ++i)
		{
			outputMask1D[imgDims[0] * (j - 1) + (i - 1)] = 255;
		}
	}
}
