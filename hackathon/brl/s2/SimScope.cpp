#include "SimScope.h"
#include <iostream>
#include <math.h>

using namespace std;

void scopeSimulator::paramShotFromController(LocationSimple nextLoc, float x, float y)
{
	seedLocation = nextLoc;
	seedX = x;
	seedY = y;
}

void scopeSimulator::fakeScopeCrop()
{
    wholeImgDim[0] = this->data1d->getDIM_H();
    wholeImgDim[1] = this->data1d->getDIM_V();
    wholeImgDim[2] = this->data1d->getDIM_D();
    wholeImgDim[3] = this->data1d->getDIM_C();

	if (int(cubeSize)%2 == 0)
	{
		tileOriginX = int(floor(seedX - cubeSize/2));
		tileXstart = tileOriginX;
		tileXend = tileOriginX + cubeSize;
		tileOriginY = int(floor(seedY - cubeSize/2));
		tileYstart = tileOriginY;
		tileYend = tileOriginY + cubeSize;
		cubeDim[0] = cubeSize + 1;
		cubeDim[1] = cubeSize + 1;
		cubeDim[2] = wholeImgDim[2];
		cubeDim[3] = wholeImgDim[3];
	}
	else 
	{
		tileOriginX = int(floor(seedX - (cubeSize-1)/2));
		tileXstart = tileOriginX;
		tileXend = tileOriginX + (cubeSize-1);
		tileOriginY = int(floor(seedY - (cubeSize-1)/2));
		tileYstart = tileOriginY;
		tileYend = tileOriginY + (cubeSize-1);
		cubeDim[0] = cubeSize;
		cubeDim[1] = cubeSize;
		cubeDim[2] = wholeImgDim[2];
		cubeDim[3] = wholeImgDim[3];
	}
	//cout << tileXstart << " " << tileXend << " " << tileYstart << " " << tileYend << endl;

	unsigned char* cubePtr = new unsigned char[cubeDim[0]*cubeDim[1]*cubeDim[2]];
	cout << "cube size: " << cubeDim[0]*cubeDim[1]*cubeDim[2] << endl;
	cube1d = data1d->loadSubvolume_to_UINT8(tileYstart, tileYend, tileXstart, tileXend, 0, wholeImgDim[2]);
	QString saveName = "testCube.v3draw";
    cubeFileName = saveName.toAscii();

	emit signalUIsaveCube();
}