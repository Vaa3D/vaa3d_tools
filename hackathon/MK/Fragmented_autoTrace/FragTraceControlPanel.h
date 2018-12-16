#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_interface.h"

#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"

class FragTraceControlPanel : public QDialog
{
	Q_OBJECT

public:
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback, bool showMenu = true);
	~FragTraceControlPanel();

	QString saveSWCFullName;
	QString adaSaveRoot;
	QString histMaskRoot;

	NeuronTree tracedTree;
	void scaleTracedTree();

signals:
	void switchOnSegPipe();

public slots:
	void imgFmtChecked(bool checked);
	void nestedChecks(bool checked);
	void saveSegStepsResultChecked(bool checked);
	void saveSettingsClicked();
	void traceButtonClicked();
	void browseSavePathClicked();

	void catchTracedTree(NeuronTree tracedTree) { this->tracedTree = tracedTree; }

private:
	V3DPluginCallback2* thisCallback;
	Ui::FragmentedTraceUI* uiPtr;

	FragTraceManager* traceManagerPtr;
};


#endif