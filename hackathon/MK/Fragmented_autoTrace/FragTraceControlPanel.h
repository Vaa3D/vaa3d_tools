#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_interface.h"

#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"

#define MAINVERSION_NUM 0
#define SUBVERSION_NUM 8
#define PATCHVERSION_NUM 2

class FragTraceControlPanel : public QDialog
{
	Q_OBJECT

public:
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback);
	~FragTraceControlPanel(); // not sure if this is needed => [delete] can only be used witn array pointer.

	// ------- Saving path for results / intermediate results ------- //
	QString saveSWCFullName;
	QString adaSaveRoot;
	QString histMaskRoot;
	// -------------------------------------------------------------- //



	/********* Result and Scaling Functions *********/ 
	NeuronTree tracedTree;
	void scaleTracedTree();
	NeuronTree treeScaleBack(const NeuronTree& inputTree);
	/************************************************/

	map<string, float> paramsFromUI;

	vector<string> blankAreas; // will be abandoned

signals:
	void switchOnSegPipe(); // currently not in action

public slots:
    /************* User Interface Configuration Buttons **************/
	void imgFmtChecked(bool checked);
	void nestedChecks(bool checked);
	void saveSegStepsResultChecked(bool checked);
	void saveSettingsClicked();
	void browseSavePathClicked();
	void blankAreaClicked();
    /********* END of [User Interface Configuration Buttons] *********/

	// ====================================================================== //
	void traceButtonClicked(); // ==> THIS IS WHERE THE TRACING PROCESS STARTS
	// ====================================================================== //

	void catchTracedTree(NeuronTree tracedTree) { this->tracedTree = tracedTree; }

private:
	V3DPluginCallback2* thisCallback;
	Ui::FragmentedTraceUI* uiPtr;

	FragTraceManager* traceManagerPtr;

	/***************** Additional Widget *****************/
	QDoubleSpinBox* doubleSpinBox;
	QStandardItemModel* listViewBlankAreas;
	/*****************************************************/



	/***************** Parameter Collecting Functions *****************/
	void pa_imgEnhancement();
	void pa_maskGeneration();
	void pa_objFilter();
	void pa_objBasedMST();
	void pa_postElongation();
	/************* END of [Parameter Collecting Functions] ************/



	/* ======= Tracing Volume Preparation ======= */
	// Partial volume tracing is achieved by talking to tf::PluginInterface through V3d_PluginLoader with v3d_interface's virtual [getPartialVolumeCoords],
	// so that it can be directly accessed through [thisCalback] from [teraflyTracePrep].
	bool volumeAdjusted;
	int* volumeAdjustedCoords;
	int* globalCoords;
	int* displayingDims;

	void teraflyTracePrep(workMode mode);
	/* ========================================== */

	

	void fillUpParamsForm(); // This is for future parameter learning project.

	void blankArea(); // will be abandoned in newer version
};


#endif