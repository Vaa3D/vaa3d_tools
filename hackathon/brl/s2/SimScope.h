#ifndef SIMSCOPE_H
#define SIMSCOPE_H

#include <QWidget>
#include <QThread>
#include <QMap>
#include "v3d_interface.h"
#include "s2Controller.h"
#include "VirtualVolume.h"
#include <QList>
#include <QString>
#include "basic_surf_objs.h"
#include <QDir>
#include "Tiff3DMngr.h"
#include <qregexp.h>
#include <math.h>
#include <qstringlist.h>
#include "basic_landmark.h"

/* 
  This class aims to simulate real microscope based on image stack.
  All operations related to this simulator is considered as 'offline mode.'
*/

using namespace iim;

class scopeSimulator : public QObject
{
	Q_OBJECT

	friend class s2Controller;
	friend class S2UI;

public:
	scopeSimulator() {};
	~scopeSimulator() {};
	
	float cubeSize, overlap;
	int bkgThres;
	float seedX, seedY;
	LocationSimple seedLocation;
	long int tileOriginX, tileOriginY;

	void testFunc(int);
	void imageLocationSetUp();

public slots:
	void paramShotFromController(LocationSimple, float, float);
	
	
private:
	V3DPluginCallback2* S2UIcb;

	VirtualVolume* data1d;
	V3DLONG wholeImgDim[4];
	V3DLONG cubeDim[4];
	unsigned char* cube1d;
	V3DLONG tileXstart, tileXend, tileYstart, tileYend;

	S2Parameter simScopeParameter; // This contains locations and tile size.
	QMap<int, S2Parameter> S2SimParameterMap;
	QString croppedImageFileName;

	

	void fakeScopeCrop();
	void imageSave();

private slots:

signals:
	void signalUIsaveCube(const char*, unsigned char*, V3DLONG[]);
	
	
};

#endif