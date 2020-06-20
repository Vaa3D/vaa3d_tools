#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_compile_full.h"
#include "v3d_interface.h"
#include "INeuronAssembler.h"
#include "IPMain4NeuronAssembler.h"

#include "FragTracer_Define.h"
#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"
#include "FragmentEditor.h"

class FragTraceControlPanel : public QDialog, public IPMain4NeuronAssembler
{
	Q_OBJECT
	Q_INTERFACES(IPMain4NeuronAssembler)

	friend class FragmentedAutoTracePlugin;
	friend class FragTraceTester;

public:
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback);
	~FragTraceControlPanel();
	map<string, float> paramsFromUI;

// ======= Saving path for results / intermediate results ======= //
	QString saveSWCFullName;
// ============================================================== //


/* ======= Result and Scaling Functions ======= */ 
	NeuronTree tracedTree;
	map<string, NeuronTree> tracedTrees;
	map<int, string> scalingRatioMap;
/* ============================================ */


/* ======= Terafly Communicating Methods ======= */
	virtual void getNAVersionNum();

	virtual void updateCViewerPortal();

	virtual bool markerMonitorStatus() { return this->uiPtr->groupBox_15->isChecked(); }
	virtual void sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList);

	virtual void eraserSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);
	virtual void connectSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap);
	virtual void exitNAeditingMode();
/* ============================================= */


public slots:
/* ================== User Interface Buttons =================== */
	// ------- Configuration ------- //
	void imgFmtChecked(bool checked);
	void nestedChecks(bool checked);
	void multiSomaTraceChecked(bool checked);
	void saveSettingsClicked();
	void browseSavePathClicked();
	// ---------------------------- //

	// ------- Post Editing ------- //
	void eraseButtonClicked();
	void connectButtonClicked();
	// ---------------------------- //
/* ======= END of [User Interface Configuration Buttons] ======= */


// ***************************************************************************** //
	void traceButtonClicked(); // ==> THIS IS WHERE THE TRACING PROCESS STARTS
// ***************************************************************************** //


// -------------- Receive and send tree between FragTraceManager -------------- //
	void catchTracedTree(NeuronTree tracedTree) { this->tracedTree = tracedTree; }
	void sendExistingNeuronTree(NeuronTree& existingTree_in_Manager) { existingTree_in_Manager = thisCallback->getSWCTeraFly(); }
// ---------------------------------------------------------------------------- //


private:
/* ============== Member Class Pointers ============== */
	V3DPluginCallback2* thisCallback; // DO NOT DELETE! -> created and sent from v3d_plugin_loader.
	INeuronAssembler* CViewerPortal;  // DO NOT DELETE! -> This is the base class interface of CViewer!
	Ui::FragmentedTraceUI* uiPtr;
	FragTraceManager* traceManagerPtr;
	FragmentEditor* fragEditorPtr;
/* =================================================== */


/* =============== Additional Widget =============== */
	QDoubleSpinBox* doubleSpinBox;
	QStandardItemModel* somaListViewer;
/* ================================================= */


/* ============== Marker Detection ============== */
	int surType;
	QList<ImageMarker> updatedMarkerList;
	QList<ImageMarker> selectedMarkerList;
	QList<ImageMarker> selectedLocalMarkerList;
	map<int, ImageMarker> somaMap;
	map<int, ImageMarker> localSomaMap;
	map<int, ImageMarker> localAxonMarkerMap;
	map<int, string> somaDisplayNameMap;
	void updateMarkerMonitor();
/* ============================================== */


/* =============== Parameter Collecting Functions =============== */
	void pa_imgEnhancement();
	void pa_maskGeneration();
	void pa_objFilter();
	void pa_objBasedMST();
	void pa_axonContinuous();
/* =========== END of [Parameter Collecting Functions] ========== */


/* ======= Tracing Volume Preparation ======= */
	// Partial volume tracing is achieved by talking to tf::PluginInterface through V3d_PluginLoader with v3d_interface's virtual [getPartialVolumeCoords],
	// so that it can be directly accessed through [thisCalback] from [teraflyTracePrep].
	bool volumeAdjusted;
	int* volumeAdjustedCoords; // local coordinates of [displaying image boudaries], eg, 1 ~ 256, etc
	int* globalCoords;         // global coordinates of [displaying image boundaries] in whole brain scale, currently not used.
	int* displayingDims;

	void teraflyTracePrep(workMode mode); // Image preparation; NOTE: FragTraceManager is created here!
	void sendImgParams();
/* ========================================== */


private slots:	
	void refreshSomaCoords();


private:
	void fillUpParamsForm(); // This is for future parameter learning project.
};


#endif