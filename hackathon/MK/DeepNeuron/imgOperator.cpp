#include "imgOperator.h"

imgOperator::imgOperator(Image4DSimple* inputImgPtr)
{
	this->ImgPtr = inputImgPtr->getRawData();
	this->imgX = inputImgPtr->getXDim();
	this->imgY = inputImgPtr->getYDim();
	this->imgZ = inputImgPtr->getZDim();
	this->channel = inputImgPtr->getCDim();

}

void imgOperator::cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
	int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ)
{
	V3DLONG OutputArrayi = 0;
	for (V3DLONG zi = zlb; zi <= zhb; ++zi)
	{
		for (V3DLONG yi = ylb; yi <= yhb; ++yi)
		{
			for (V3DLONG xi = xlb; xi <= xhb; ++xi)
			{
				OutputImagePtr[OutputArrayi] = InputImagePtr[imgX*imgY*(zi - 1) + imgX*(yi - 1) + (xi - 1)];
				++OutputArrayi;
			}
		}
	}
}

void imgOperator::maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
	int xlb, int xhb, int ylb, int yhb, int zlb, int zhb)
{
	int xDim = xhb - xlb + 1;
	int yDim = yhb - ylb + 1;
	int zDim = zhb - zlb + 1;
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