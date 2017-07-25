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
#include"s2UI.h"
#include <qstringlist.h>
#include "basic_landmark.h"

/* 
  This class aims to simulate real microscope based on image stack.
  All operations related to this simulator is considered as 'offline mode.'
*/

using namespace iim;

class scopeSimulator : public QWidget
{
	Q_OBJECT

	friend class s2Controller;
	friend class S2UI;

public:
	scopeSimulator() {};
	~scopeSimulator() {};
	scopeSimulator(QStringList initParam);
	
	void testFunc(int);
	void imageLocationSetUp();


private:
	float cubeSize, overlap;
	int bkgThres;
	VirtualVolume* data1d;
	LocationSimple seedLocation;

	S2Controller simController;
	S2Controller simPosMon;
	
	S2Parameter simScopeParameter; // This contains locations and tile size.
	QString croppedImageFileName;

	void imageCrop();
	void imageSave();
	
};

#endif