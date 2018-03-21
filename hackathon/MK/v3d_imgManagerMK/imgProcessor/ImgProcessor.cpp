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

