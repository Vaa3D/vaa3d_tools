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

signals:
	void switchOnSegPipe();

public slots:
	void saveSettingsClicked();
	void traceButtonClicked();

private:
	V3DPluginCallback2* thisCallback;

	Ui::FragmentedTraceUI* uiPtr;
	FragTraceManager* traceManager;
};






#endif