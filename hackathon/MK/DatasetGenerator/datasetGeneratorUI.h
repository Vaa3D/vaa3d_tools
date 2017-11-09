#ifndef DATASETGENERATORUI_H
#define DATASETGENERATORUI_H

#include "Dataset_Generator_plugin.h"
#include "ui_DatasetGenerator.h"
#include <v3d_interface.h>

namespace Ui 
{
	class DatasetGeneratorUI;
} 

class DatasetGeneratorUI : public QDialog
{

	Q_OBJECT

public:
	DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback);
	~DatasetGeneratorUI();

	QStringList procItems;

public slots:
	void selectClicked();
	void checkboxToggled(bool);
	void exclusiveToggle(bool);
	void preprocessingEdit();

private:
	Ui::DatasetGeneratorUI* ui;
	QDirModel* dirModel;
	QStringListModel* procSteps;


};






#endif