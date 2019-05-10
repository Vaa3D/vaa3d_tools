#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_interface.h"

#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"

#define MAINVERSION_NUM 0
#define SUBVERSION_NUM 4
#define PATCHVERSION_NUM 0

class FragTraceControlPanel : public QDialog
{
	Q_OBJECT

public:
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback, bool showMenu = true);
	~FragTraceControlPanel();

	QString saveSWCFullName;
	QString adaSaveRoot;
	QString histMaskRoot;

	vector<string> blankAreas;

	NeuronTree tracedTree;
	void scaleTracedTree();
	NeuronTree treeScaleBack(const NeuronTree& inputTree);

	map<string, float> paramsFromUI;

signals:
	void switchOnSegPipe();

public slots:
	void imgFmtChecked(bool checked);
	void nestedChecks(bool checked);
	void saveSegStepsResultChecked(bool checked);
	void saveSettingsClicked();
	void traceButtonClicked();
	void browseSavePathClicked();
	void blankAreaClicked();

	void catchTracedTree(NeuronTree tracedTree) { this->tracedTree = tracedTree; }

private:
	QDoubleSpinBox* doubleSpinBox;
	QStandardItemModel* listViewBlankAreas;

	V3DPluginCallback2* thisCallback;
	Ui::FragmentedTraceUI* uiPtr;

	FragTraceManager* traceManagerPtr;

	void fillUpParamsForm();
};


#endif