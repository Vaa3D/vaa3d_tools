#ifndef DATASETGENERATORUI_H
#define DATASETGENERATORUI_H

#include "Dataset_Generator_plugin.h"
#include "ui_DatasetGenerator.h"
#include <v3d_interface.h>
#include "Operator.h"

namespace Ui 
{
	class DatasetGeneratorUI;
} 

class DatasetGeneratorUI : public QDialog
{

	Q_OBJECT
	friend class Operator;

public:	
	DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback);
	~DatasetGeneratorUI();

	Operator DatasetOperator;

	QStringList procItems2D;
	QStringList procItems3D;
	QStringList selectedProcItems;

public slots:
	void selectClicked();
	void checkboxToggled(bool);
	void exclusiveToggle(bool);
	void associativeToggle(bool);
	void preprocessingEdit();
	void okClicked();

private:
	Ui::DatasetGeneratorUI* ui;
	QDirModel* dirModel;
	QStringListModel* procSteps;
	QStringListModel* procSteps3D;
	QStandardItemModel* listViewSteps;
	QStandardItemModel* listViewSteps3D;

private slots:
	void progressBarUpdater(QString taskName, int percentage);
	


};




#endif