#include "SimScope.h"
#include <iostream>
#include "VirtualVolume.h"

using namespace std;

scopeSimulator::scopeSimulator(QStringList inputParams)
{
	//for (QStringList::iterator it=inputParams.begin(); it!=inputParams.end(); ++it) qDebug() << *it << " ";
	this->data1d = VirtualVolume::instance(inputParams[0].toStdString().c_str());
	this->cubeSize = inputParams[2].toFloat();
	this->overlap = inputParams[3].toFloat();
	this->bkgThres = inputParams[4].toInt();
	QList<ImageMarker> inputSeed = readMarker_file(inputParams[1]);
	this->seedLocation.init();
	this->seedLocation.x = inputSeed[0].x;
	this->seedLocation.y = inputSeed[0].y;
	this->seedLocation.z = inputSeed[0].z;

	
}

void scopeSimulator::testFunc(int num) 
{
	cout << "test " << num << endl;
}
