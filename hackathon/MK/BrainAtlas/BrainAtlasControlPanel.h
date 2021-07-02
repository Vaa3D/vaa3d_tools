#ifndef BRAINATLASCONTROLPANEL_H
#define BRAINATLASCONTROLPANEL_H

#include "ui_brainRegionListTest.h"
#include "BrainScanner.h"

#include "integratedDataStructures.h"

#include "v3d_interface.h"

const bool DEBUG = false;;

using namespace integratedDataStructures;

class BrainAtlasControlPanel : public QDialog
{
	Q_OBJECT

public:
	BrainAtlasControlPanel(QWidget* parent, V3DPluginCallback2* callback);
	~BrainAtlasControlPanel() { this->thisCallback->setBrainAtlasStatus(this->cur3DViewer, false); }

	boost::container::flat_map<string, brainRegion> regionMap;
	boost::container::flat_map<string, int> regionTypeMap;
	boost::container::flat_map<string, NeuronTree> regionTreeMap;
	set<string> loadedRegions;
	map<string, int> region2ObjManagerIndexMap;
	map<string, int> region2UIindexMap;
	map<string, bool> surfaceStatus;

	Ui::Dialog* currentUIptr;

	v3dhandle curWin;
	V3dR_MainWindow* cur3DViewer;

	NeuronTree convertRegion2tree(string regionName);

	void hideRegionFromMouseClick(string regionName);
	void scanInvolvedRegions(vector<float> coord);
	
	void cleanUpRegionRecords();

public slots:
	void regionSelected(int row, int col);
	void browseFolder();
	void scanSomaOKclicked();

private:
	V3DPluginCallback2* thisCallback;
	BrainScanner myScanner;

	void regionTypeMapGen(QProgressDialog* iniProgressBarPtr);
};

#endif