#include <iostream>

#include "ImgProcessor.h"

using namespace std;

morphStructElement::morphStructElement() : eleShape("square"), xLength(3), yLength(3)
{
	vector<int> array1(3, 1);
	this->structEle2D.push_back(array1);
	this->structEle2D.push_back(array1);
	this->structEle2D.push_back(array1);
}

morphStructElement::morphStructElement(string shape) : eleShape(shape)
{
	if (this->eleShape.compare("square") == 0)
	{
		vector<int> array1(3, 1);
		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array1);
	}
	else if (this->eleShape.compare("circle") == 0)
	{
		this->xLength = 7;
		this->yLength = 7;

		vector<int> array1(7, 1);
		vector<int> array2(7, 1);
		vector<int> array3(7, 1);
		array1.at(0) = 0; array1.at(1) = 0; array1.at(5) = 0; array1.at(6) = 0;
		array2.at(0) = 0; array2.at(6) = 0;

		this->structEle2D.push_back(array1);
		this->structEle2D.push_back(array2);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array3);
		this->structEle2D.push_back(array2);
		this->structEle2D.push_back(array1);
	}
}

morphStructElement::morphStructElement(string shape, int length1, int length2) : eleShape(shape), xLength(length1), yLength(length2)
{

}

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

