#ifndef BRAINATLASMANAGER_H
#define BRAINATLASMANAGER_H

#include "v3d_interface.h"

#include "integratedDataStructures.h"

#include "ui_brainRegionListTest.h"
#include "BrgScanner.h"

using namespace integratedDataStructures;

class BrainAtlasManager : public QDialog
{
	Q_OBJECT

public:
	BrainAtlasManager(QWidget* parent, V3DPluginCallback2* callback);
	~BrainAtlasManager();

	boost::container::flat_map<string, brainRegion> regionMap;
	boost::container::flat_map<string, NeuronTree> regionTreeMap;
	set<string> loadedRegions;
	map<string, int> surface2indexMap;
	map<string, int> region2UIindexMap;
	map<string, bool> surfaceStatus;

	Ui::Dialog* regionListUI;
	//bool eventFilter(QObject* obj, QMouseEvent* mouseEvent);
	
	v3dhandle curWin;
	V3dR_MainWindow* cur3DViewer;

	vector<NeuronTree> loadedTree;

	void rightClickBrgShow(QString inputCoordKey);

public slots:
    void regionSelected(int row, int col, bool rightClick = false);
	//void scanCheckBoxes_list();
	void neuronInvolvedRegionClicked();

private: 
	V3DPluginCallback2* thisCallback;
	BrgScanner brgScanner;
};


#endif