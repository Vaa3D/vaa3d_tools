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
#include <cstring>

/* 
  This class aims to simulate real microscope based on image stack.
  All operations related to this simulator is considered as 'offline mode.'
*/

using namespace iim;

class ScopeSimulator : public QObject
{
	Q_OBJECT

	friend class s2Controller;
	friend class S2UI;

public:
	ScopeSimulator() {};
	~ScopeSimulator() {};
	
	V3DLONG cubeSize;
	int bkgThres;
	float overlap;
	LocationSimple location;

	int testi;

public slots:
	void paramShotFromController(LocationSimple, float, float);
	void fakeScopeCrop();
	
private:
	VirtualVolume* data1d;
	V3DLONG wholeImgDim[4];
	V3DLONG cubeDim[4];
	unsigned char* cube1d;
	float tileOriginX, tileOriginY;
	V3DLONG tileXstart, tileXend, tileYstart, tileYend;
	const char* cubeFileName;

	S2Parameter simScopeParameter; // This contains locations and tile size.
	QMap<int, S2Parameter> S2SimParameterMap;
	int simMaxParams;

	void initFakeScopeParams();

private slots:


signals:
	void testFunc(int);
	void signalUIsaveCube();
	void callMyPosMon(V3DLONG, V3DLONG);
	
};

#endif