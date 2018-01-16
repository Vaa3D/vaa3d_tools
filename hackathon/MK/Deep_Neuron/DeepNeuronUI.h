#ifndef DEEPNEURONUI_H
#define DEEPNEURONUI_H

#include "v3d_interface.h"
#include "ui_DeepNeuronForm.h"

namespace Ui
{
	class DeepNeuronDialog;
}

class DeepNeuronUI : public QDialog
{
	Q_OBJECT

public:
	DeepNeuronUI(QWidget* parent, V3DPluginCallback2* callback);
	//~DeepNeuronUI();

public slots:
	void uiCall();
	void okClicked();

private:
	Ui::DeepNeuronDialog* ui;
	V3DPluginCallback2* mainCallBack;
};

#endif