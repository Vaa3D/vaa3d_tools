#include "SimScope.h"
#include <iostream>
#include <math.h>
#include <qtimer.h>
#include "v3d_basicdatatype.h"

using namespace std;

void SimScope::hookThingsUp()
{
	connect(this, SIGNAL(pullSwitch(bool)), this, SLOT(fakeScopeSwitch(bool)));
	connect(this, SIGNAL(transmitKick()), this, SLOT(gotKicked()));
}

void SimScope::configFakeScope(QStringList initialParam)
{
	acqCycleNum = 0;
	this->wholeStack = VirtualVolume::instance(initialParam[0].toStdString().c_str());
	QList<ImageMarker> inputSeed = readMarker_file(initialParam[1]);
	float x = inputSeed[0].x - 1;
	float y = inputSeed[0].y - 1;
	float z = inputSeed[0].z - 1;
	this->location.x = x;
	this->location.y = y;
	this->location.z = z;
	this->zCoord = int(floor(z));
	this->cubeSize = initialParam[2].toInt();
	this->overlap = initialParam[3].toFloat();
	this->bkgThres = initialParam[4].toInt();
	this->zSecNum = initialParam[5].toInt();
	this->hookThingsUp();

	this->wholeImgDim[0] = this->wholeStack->getDIM_H();
	this->wholeImgDim[1] = this->wholeStack->getDIM_V();
	this->wholeImgDim[2] = this->wholeStack->getDIM_D();
	this->wholeImgDim[3] = this->wholeStack->getDIM_C();

	float tileLocX, tileLocY;
	if (int(this->cubeSize)%2 == 0)
	{
		tileLocX = x - this->cubeSize/2;
		tileLocY = y - this->cubeSize/2;
	}
	else 
	{
		tileLocX = x - (this->cubeSize-1)/2;
		tileLocY = y - (this->cubeSize-1)/2;
	}

	LocationSimple startLoc;
	startLoc.x = x;
	startLoc.y = y;
	startLoc.z = z;
	startLoc.ev_pc1 = cubeSize + 1;
	startLoc.ev_pc2 = cubeSize + 1;
	//cout << "  --> SEED: " << endl;
	//cout << startLoc.x << " " << startLoc.y << " " << startLoc.z << endl;

	this->initFakeScopeParams();
	emit notifyConfigReady(startLoc, tileLocX, tileLocY);
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
	S2SimParameterMap.insert(8, S2Parameter("micronsPerPixelX", "-gts micronsPerPixel XAxis", 1.0,"","float")); // fixed as 1
	S2SimParameterMap.insert(9, S2Parameter("micronsPerPixelY", "-gts micronsPerPixel YAxis", 1.0)); // fixed as 1
	S2SimParameterMap.insert(10, S2Parameter("pixelsPerLine", "-gts pixelsPerLine")); // changing
	S2SimParameterMap.insert(11, S2Parameter("linesPerFrame", "-gts linesPerFrame")); // changing
	S2SimParameterMap.insert(12, S2Parameter("opticalZoom", "-gts opticalZoom"));
	S2SimParameterMap.insert(13, S2Parameter("micronROISizeX", "", 0.0, "", "floatderived"));
	S2SimParameterMap.insert(14, S2Parameter("micronROISizeY", "", 0.0, "", "floatderived"));
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
	if (int(cubeSize)%2 == 0)
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
		tileOriginX = x - (cubeSize/2);
		tileXstart = int(floor(tileOriginX));
		tileXend = tileXstart + cubeSize;
		tileOriginY = y - (cubeSize/2);
		tileYstart = int(floor(tileOriginY));
		tileYend = tileOriginY + cubeSize;

		cubeDim[0] = cubeSize + 1;
		cubeDim[1] = cubeSize + 1;
		if (zSecNum > 0) cubeDim[2] = zSecNum + 1;
		else cubeDim[2] = wholeImgDim[2];
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
		if (zSecNum > 0) cubeDim[2] = zSecNum + 1;
		else cubeDim[2] = wholeImgDim[2];
		cubeDim[3] = wholeImgDim[3];
	}
	//cout << tileXstart << " " << tileXend << " " << tileYstart << " " << tileYend << endl;
}

void SimScope::fakeScopeCrop()
{
	unsigned char* cube1d = new unsigned char[cubeDim[0]*cubeDim[1]*cubeDim[2]];
	if (zSecNum > 0) 
		cube1d = wholeStack->loadSubvolume_to_UINT8(tileYstart, tileYend+1, tileXstart, tileXend+1, (zCoord-zSecNum/2), ((zCoord+zSecNum/2)+1));
    else 
		cube1d = wholeStack->loadSubvolume_to_UINT8(tileYstart, tileYend+1, tileXstart, tileXend+1, 0, wholeImgDim[2]);

	//cout << wholeStack->getDIM_D() << " " << wholeStack->getDIM_H() << " " << wholeStack->getDIM_V() << endl;
	//cout << "cubeDim: " << cubeDim[0] << " " << cubeDim[1] << " " << cubeDim[2] << " " << cubeDim[3] << endl;
	QString folder = savingPath + "/";
	QDir().mkpath(folder);
	QString sliceFolder = folder + "tile_" + QString::number(acqCycleNum);
	QDir().mkpath(sliceFolder);
	
	// -- Check if the cube is cropped correctly --
	QString cubeTest = folder + "cube_" + QString::number(acqCycleNum) + ".v3draw";
	qDebug() << " fakeScope: image stack name = " << cubeTest;
	const char* cubeName = cubeTest.toStdString().c_str(); //--> This is really weird, that separate conversion would result in garbage code. Qt bug?
	simple_saveimage_wrapper(*S2UIcb, cubeTest.toStdString().c_str(), cube1d, cubeDim, 1);

	updatedOriginX = tileOriginX;
	updatedOriginY = tileOriginY;
	updatedXstart = tileXstart;
	updatedXend = tileXend;
	updatedYstart = tileYstart;
	updatedYend = tileYend;

	Image4DSimple* cube4D = new Image4DSimple;
	Image4DSimple outputImg;
	ImagePixelType pixelType = V3D_UINT8;
	
	cube4D->setData(cube1d, cubeDim[0], cubeDim[1], cubeDim[2], cubeDim[3], pixelType);
	int x = round(updatedOriginX);
	int y = round(updatedOriginY);
	QString tileX = QString::number(x);
	QString tileY = QString::number(y);
	QString filePrefix = sliceFolder + "/ZSeries-";
	
	qDebug() << "  fakeScope: lastImageName = " << lastImgName;
	//system("pause");
	save_z_slices(*S2UIcb, cube4D, 0, 1, cubeDim[2]-1, filePrefix);

	S2SimParameterMap[7].setCurrentString(lastImgName);
	if(cube1d) {delete []cube1d; cube1d = 0;}
}

void SimScope::updateS2ParamMap()
{
	// Note: S2SimParameterMap is only needed in the SimScope->myPosMon route.
	qDebug() << "fakeScope: lastImgName: " << lastImgName;
	S2Parameter newX("stageX", "-gmp X 0");
	newX.setCurrentValue(updatedOriginX);
	S2SimParameterMap[5] = newX;
	S2Parameter newY("stageY", "-gmp Y 0");
	newY.setCurrentValue(updatedOriginY);
	S2SimParameterMap[6] = newY;
	++acqCycleNum;
}

void SimScope::fakeScopeSwitch(bool pull)
{
	if (pull == true) 
	{
		cout << "  ========= fakeScope on ========" << endl;
		isRunning = true;
		S2MapEmitter();
		emit transmitKick();
	}
	else if (pull == false) 
	{
		cout << "  ========= fakeScope off ========" << endl << endl;
		isRunning = false;
	}
}

void SimScope::gotKicked()
{
	if (isRunning == true)
	{
		fakeScopeCrop();
		updateS2ParamMap();
		emit pullSwitch(false);
	}
}

void SimScope::S2MapEmitter()
{
	emit reportToMyPosMon(S2SimParameterMap);
	if (isRunning == true) QTimer::singleShot(50, this, SLOT(S2MapEmitter()));
}

bool SimScope::save_z_slices(V3DPluginCallback2& callback, Image4DSimple* subject, V3DLONG startnum, V3DLONG increment, V3DLONG endnum, QString filenameprefix)
{
    if (!subject || !subject->valid()) return false;

    V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
    V3DLONG sz3 = subject->getCDim();

    //copy data
    V3DLONG sz2_new = ceil(double(endnum-startnum+1) / increment);

    Image4DSimple outImage;

    outImage.createBlankImage(sz0, sz1, 1, sz3, subject->getDatatype());
    if (!outImage.valid()) return false;

    QString curfile = filenameprefix;
    V3DLONG k=0, c, pagesz;
    for (V3DLONG i=startnum, k=0; i<=endnum; i+=increment, ++k)
    {
        switch (subject->getDatatype())
        {
        case V3D_UINT8: 
            pagesz = sz0 * sz1 * subject->getUnitBytes();
            for (c=0; c<sz3; c++)
            {
                //printf("c=%d i=%d k=%d\n", c, i, k);
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".tif");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));

            break;

        case V3D_UINT16:
            pagesz = sz0 * sz1 * subject->getUnitBytes();
            for (c=0; c<sz3; c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));

            break;

        case V3D_FLOAT32:
            pagesz = sz0 * sz1 * subject->getUnitBytes();
            for (c=0; c<sz3; c++)
            {
                unsigned char *dst = outImage.getRawDataAtChannel(c);
                unsigned char *src = subject->getRawDataAtChannel(c) + i*pagesz;
                memcpy(dst, src, pagesz);
            }
            curfile = gen_file_name(filenameprefix, k, sz2_new, ".v3draw");
            callback.saveImage(&outImage, (char *)qPrintable(curfile));
            break;

        default: v3d_msg("You should never see this. The data is not returned meaningfully. Check your data and code.");
            return false;
            break;
        }
    }

    return true;
}

QString SimScope::gen_file_name(QString prefixstr, V3DLONG k, V3DLONG maxn, QString extstr)
{
    QString ks=""; ks.setNum(k); 
	if (k < 10) ks.prepend("0000");
    else if (k < 100) ks.prepend("000");
    else if (k < 1000) ks.prepend("00");
    else if (k < 10000) ks.prepend("0");

	ks = ks + "_Cycle00001_Ch2_000001.ome";
	lastImgName = prefixstr + ks + extstr;
    return (prefixstr + ks + extstr);
}


