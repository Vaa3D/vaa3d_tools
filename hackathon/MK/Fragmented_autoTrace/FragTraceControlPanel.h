#ifndef _FRAGTRACECONTROLPANEL_H_
#define _FRAGTRACECONTROLPANEL_H_

#include "v3d_interface.h"

#include "ui_fragmentedTraceUI.h"
#include "FragTraceManager.h"

class FragTraceControlPanel : public QDialog
{
	Q_OBJECT

public:
	FragTraceControlPanel();
	FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback);
	~FragTraceControlPanel();

public slots:
	void saveSettingsClicked();
	void traceButtonClicked();

private:
	Ui::FragmentedTraceUI* uiPtr;

};






#endif