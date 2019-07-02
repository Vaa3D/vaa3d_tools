#ifndef BRAINATLASMANAGER_H
#define BRAINATLASMANAGER_H

#include "v3d_interface.h"

#include "integratedDataStructures.h"

using namespace integratedDataStructures;

class BrainAtlasManaer : public QDialog
{
	Q_OBJECT

public:
	BrainAtlasManaer(QWidget* parent, V3DPluginCallback2* callback);

	boost::container::flat_map<string, brainRegion> regionMap;

private: 
	V3DPluginCallback2* thisCallback;
};


#endif