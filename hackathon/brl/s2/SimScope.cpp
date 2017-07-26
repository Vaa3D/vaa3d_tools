#include "SimScope.h"
#include <iostream>
#include <math.h>

using namespace std;

void scopeSimulator::testFunc(int num) 
{
	cout << "test " << num << endl;
}

void scopeSimulator::paramShotFromController(LocationSimple nextLoc, float x, float y)
{
	seedLocation = nextLoc;
	seedX = x;
	seedY = y;

	fakeScopeCrop();
}

void scopeSimulator::fakeScopeCrop()
{
    wholeImgDim[0] = this->data1d->getDIM_H();
    wholeImgDim[1] = this->data1d->getDIM_V();
    wholeImgDim[2] = this->data1d->getDIM_D();
    wholeImgDim[3] = this->data1d->getDIM_C();

	if (int(cubeSize)%2 == 0)
	{
		tileOriginX = long int(floor(seedX - cubeSize/2));
		tileXstart = tileOriginX;
		tileXend = tileOriginX + cubeSize;
		tileOriginY = long int(floor(seedY - cubeSize/2));
		tileYstart = tileOriginY;
		tileYend = tileOriginY + cubeSize;
	}
	else 
	{
		tileOriginX = long int(floor(seedX - (cubeSize-1)/2));
		tileXstart = tileOriginX;
		tileXend = tileOriginX + (cubeSize-1);
		tileOriginY = long int(floor(seedY - (cubeSize-1)/2));
		tileYstart = tileOriginY;
		tileYend = tileOriginY + (cubeSize-1);
	}

	cubeDim[0] = cubeSize + 1;
	cubeDim[1] = cubeSize + 1;
	cubeDim[2] = wholeImgDim[2];
	cubeDim[3] = wholeImgDim[3];

	this->cube1d = this->data1d->loadSubvolume_to_UINT8(tileYstart, tileYend, tileXstart, tileYend, 0, wholeImgDim[2]);
	
	QString saveName = "testCube.v3draw";
    const char* fileName = saveName.toAscii();
	//emit signalUIsaveCube(fileName, cube1d, cubeDim);
	simple_saveimage_wrapper(*S2UIcb, fileName, cube1d, cubeDim, 1);
}