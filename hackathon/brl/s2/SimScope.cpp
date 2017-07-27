#include "SimScope.h"
#include <iostream>
#include <math.h>

using namespace std;

void ScopeSimulator::paramShotFromController(LocationSimple nextLoc, float x, float y)
{
	location = nextLoc;

	if (int(cubeSize)%2 == 0)
	{
		tileOriginX = x;
		tileXstart = int(floor(tileOriginX));
		tileXend = tileXstart + cubeSize;
		tileOriginY = y;
		tileYstart = int(floor(tileOriginY));
		tileYend = tileOriginY + cubeSize;
		cubeDim[0] = cubeSize + 1;
		cubeDim[1] = cubeSize + 1;
		cubeDim[2] = wholeImgDim[2];
		cubeDim[3] = wholeImgDim[3];
	}
	else 
	{
		tileOriginX = x - (cubeSize-1)/2;
		tileXstart = int(floor(tileOriginX));
		tileXend = tileXstart + (cubeSize-1);
		tileOriginY = y - (cubeSize-1)/2;
		tileYstart = int(floor(tileOriginY));
		tileYend = tileYstart + (cubeSize-1);
		cubeDim[0] = cubeSize;
		cubeDim[1] = cubeSize;
		cubeDim[2] = wholeImgDim[2];
		cubeDim[3] = wholeImgDim[3];
	}
	//cout << tileXstart << " " << tileXend << " " << tileYstart << " " << tileYend << endl;
}

void ScopeSimulator::fakeScopeCrop()
{
	unsigned char* cubePtr = new unsigned char[cubeDim[0]*cubeDim[1]*cubeDim[2]];
	cout << "cube size: " << cubeDim[0]*cubeDim[1]*cubeDim[2] << endl;
	cube1d = data1d->loadSubvolume_to_UINT8(tileYstart, tileYend, tileXstart, tileXend, 0, wholeImgDim[2]);
	QString num = QString::number(testi);
	QString saveName = "testCube" + num + ".v3draw";
    cubeFileName = saveName.toAscii();

	tileXstart += 50;
	tileXend += 50;
	tileYstart += 50;
	tileYend += 50;

	emit signalUIsaveCube();
	emit callMyPosMon(tileXstart, tileYstart);
}

void ScopeSimulator::initFakeScopeParams()
{
    S2Parameter tPara = S2Parameter("currentMode", "-gts activeMode", 0.0, "", "string");
    S2SimParameterMap.insert(0, tPara);
    S2SimParameterMap.insert(1, S2Parameter("galvoXVolts", "-gts currentScanCenter XAxis")) ;
    S2SimParameterMap.insert(2, S2Parameter("galvoYVolts", "-gts currentScanCenter YAxis")) ;
    S2SimParameterMap.insert(3, S2Parameter("piezoZ", "-gmp Z 1")) ;
    S2SimParameterMap.insert(4, S2Parameter("stepperZ", "-gmp Z 0")) ;
    S2SimParameterMap.insert(5, S2Parameter("stageX", "-gmp X 0")) ; // changing
    S2SimParameterMap.insert(6, S2Parameter("stageY", "-gmp Y 0")) ; // changing
    S2SimParameterMap.insert(7, S2Parameter("last image", "-gts recentAcquisitions", 0.0, "", "list")); // changing
    S2SimParameterMap.insert(8, S2Parameter("micronsPerPixelX", "-gts micronsPerPixel XAxis")); // fixed as 1
    S2SimParameterMap.insert(9, S2Parameter("micronsPerPixelY", "-gts micronsPerPixel YAxis")); // fixed as 1
    S2SimParameterMap.insert(10, S2Parameter("pixelsPerLine", "-gts pixelsPerLine")); // changing
    S2SimParameterMap.insert(11, S2Parameter("linesPerFrame", "-gts linesPerFrame")); // changing
    S2SimParameterMap.insert(12, S2Parameter("opticalZoom", "-gts opticalZoom"));
    S2SimParameterMap.insert(13, S2Parameter("micronROISizeX", "", 0.0, "", "floatderived"));
    S2SimParameterMap.insert(14, S2Parameter("micronROISizeY", "", 0, "", "floatderived"));
    S2SimParameterMap.insert(15, S2Parameter("maxVoltsX", "-gts maxVoltage XAxis ", 0.0, "float"));
    S2SimParameterMap.insert(16, S2Parameter("minVoltsX", "-gts minVoltage XAxis ", 0.0, "float"));

    S2SimParameterMap.insert(17, S2Parameter("micronsPerVolt", "", 0.0, "", "floatderived"));
    S2SimParameterMap.insert(18, S2Parameter("galvoXmicrons", "", 0.0, "", "floatderived"));
    S2SimParameterMap.insert(19, S2Parameter("galvoYmicrons", "", 0.0, "", "floatderived"));
    S2SimParameterMap.insert(20, S2Parameter("resonantX", "-gts currentPanLocationX", 0.0, "float"));
    S2SimParameterMap.insert(21, S2Parameter("maxVoltsY", "-gts maxVoltage YAxis ", 0.0, "float"));
    S2SimParameterMap.insert(22, S2Parameter("minVoltsY", "-gts minVoltage YAxis ", 0.0, "float"));
    S2SimParameterMap.insert(23, S2Parameter("micronsPerVoltY", "", 0.0, "", "floatderived"));

    simMaxParams = S2SimParameterMap.keys().last()+1;
}