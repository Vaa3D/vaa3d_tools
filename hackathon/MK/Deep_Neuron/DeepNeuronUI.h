#ifndef DEEPNEURONUI_H
#define DEEPNEURONUI_H

#include <qsettings.h>

#include "v3d_interface.h"
#include "ui_DeepNeuronForm.h"
#include "imgOperator.h"
#include "DLOperator.h"

namespace Ui
{
	class DeepNeuronDialog;
}

class DeepNeuronUI : public QDialog
{
	Q_OBJECT

public:
	DeepNeuronUI(QWidget* parent, V3DPluginCallback2* callback);
	~DeepNeuronUI();

	LandmarkList markerList;
	QString deployDisplay;
	QString modelDisplay;
	QString meanDisplay;

	QString imageName;
	Image4DSimple* curImg4DPtr;

public slots:
	void okClicked();
	void filePath();
	void closeClicked();

	void progressBarUpdater(QString taskName, int percentage);

private:
	Ui::DeepNeuronDialog* ui;
	V3DPluginCallback2* mainCallBack;

	imgOperator* curImgProcessor;
	DLOperator* curImgDLProcessor;

	QString QSettingFileName;
	void loadSettings();
};

#endif