#include <iostream>

#include "ImgProcessor.h"

using namespace std;

ImgProcessor::ImgProcessor(MIPOrientation MIP_Direction)
{
	this->MIPDirection = MIP_Direction;
}

void ImgProcessor::maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[], int xDim, int yDim, int zDim)
{
	if (this->MIPDirection == Mxy) // need to check in which direction to generate MIP? no template implementation for now
	{
		for (int yi = 0; yi < yDim; ++yi)
		{
			for (int xi = 0; xi < xDim; ++xi)
			{
				short int maxVal = 0;
				for (int zi = 0; zi < zDim; ++zi)
				{
					short int curValue = inputVOIPtr[xDim*yDim*zi + xDim*yi + xi];
					if (curValue > maxVal) maxVal = curValue;
				}
				OutputImage2DPtr[xDim*yi + xi] = (unsigned char)(maxVal);
			}
		}
	}
}