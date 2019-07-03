#ifndef BRAINATLASMANAGER_H
#define BRAINATLASMANAGER_H

#include "v3d_interface.h"

#include "integratedDataStructures.h"

#include "ui_brainRegionListTest.h"

using namespace integratedDataStructures;

class BrainAtlasManaer : public QDialog
{
	Q_OBJECT

public:
	BrainAtlasManaer(QWidget* parent, V3DPluginCallback2* callback);

	boost::container::flat_map<string, brainRegion> regionMap;
	boost::container::flat_map<string, NeuronTree> regionTreeMap;
	set<string> loadedRegions;

	Ui::Dialog* regionListUI;
	//bool eventFilter(QObject* obj, QMouseEvent* mouseEvent);
	
	v3dhandle curWin;
	V3dR_MainWindow* cur3DViewer;

public slots:
	void scanCheckBoxes_list();

public slots:
	//void displayRegion();

private: 
	V3DPluginCallback2* thisCallback;
};


#endif