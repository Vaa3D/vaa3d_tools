#include "SimScope.h"
#include <iostream>
#include <math.h>
#include <qtimer.h>

using namespace std;

void SimScope::hookThingsUp()
{
	connect(this, SIGNAL(pullSwitch(bool)), this, SLOT(fakeScopeSwitch(bool)));
	//connect(this, SIGNAL(reportToMyPosMon(QMAP<int, S2Parameter>)), this, SLOT(constantReport(QMap<int, SParameter>)));
	connect(this, SIGNAL(transmitKick()), this, SLOT(gotKicked()));
}

void SimScope::initFakeScopeParams()
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

	float frameDim = float(cubeSize);
	if (int(cubeSize&2) == 0)
	{
		S2SimParameterMap[10].setCurrentValue(frameDim + 1);
		S2SimParameterMap[11].setCurrentValue(frameDim + 1);
	}
	else
	{
		S2SimParameterMap[10].setCurrentValue(frameDim);
		S2SimParameterMap[11].setCurrentValue(frameDim);
	}

    simMaxParams = S2SimParameterMap.keys().last()+1;
}

void SimScope::paramShotFromController(LocationSimple nextLoc, float x, float y)
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

	//pullSwitch(true);
}

void SimScope::fakeScopeCrop()
{
	unsigned char* cubePtr = new unsigned char[cubeDim[0]*cubeDim[1]*cubeDim[2]];
	cout << "cube size: " << cubeDim[0]*cubeDim[1]*cubeDim[2] << endl;
	cube1d = this->data1d->loadSubvolume_to_UINT8(tileYstart, tileYend, tileXstart, tileXend, 0, wholeImgDim[2]-1);
	QString num = QString::number(testi);
	QString saveName = "testCube" + num + ".v3draw";
    cubeFileName = saveName.toAscii();

	updatedOriginX = tileOriginX;
	updatedOriginY = tileOriginY;
	updatedXstart = tileXstart;
	updatedXend = tileXend;
	updatedYstart = tileYstart;
	updatedYend = tileYend;

	simple_saveimage_wrapper(*S2UIcb, cubeFileName, cube1d, cubeDim, 1);
	//for (size_t i=0; i<1000; ++i) qDebug() << cube1d[i];
}

void SimScope::updateS2ParamMap()
{
	// Note: S2SimParameterMap is only needed in the SimScope->myPosMon route.
	S2Parameter newX("stageX", "-gmp X 0");
	newX.setCurrentValue(updatedOriginX);
	S2SimParameterMap[5] = newX;
	S2Parameter newY("stageY", "-gmp y 0");
	newX.setCurrentValue(updatedOriginY);
	S2SimParameterMap[6] = newY;
}

//infrastructure to start and stop simscope
// slot that sets isRunning =False 
// slot startSimscope that sets isRunning =True and calls runSimScope
// new attribute bool isRunning
// new signal called currentS2ParamterMap

void SimScope::fakeScopeSwitch(bool pull)
{
	
	if (pull == true) 
	{
		isRunning = true;
		emit transmitKick();
		//QTimer::singleShot(50, this, SLOT(constantReport(S2SimParameterMap)));
	}
	else if (pull == false) isRunning = false;
}

void SimScope::gotKicked()
{
	if (isRunning == true)
	{
		fakeScopeCrop();
		updateS2ParamMap();
		constantReport(S2SimParameterMap);
		emit pullSwitch(false);
	}
}

void SimScope::constantReport(QMap<int, S2Parameter> S2SimParameterMap)
{
	cout << "report sent?" << endl;
	emit reportToMyPosMon(S2SimParameterMap);
	
}
