#ifndef DATASETGENERATORUI_H
#define DATASETGENERATORUI_H

#include "Dataset_Generator_plugin.h"
#include "ui_DatasetGenerator.h"

namespace Ui 
{
	class DatasetGeneratorUI;
} 

class DatasetGeneratorUI: public QDialog
{
	

public:
	DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback);
	~DatasetGeneratorUI() {};


private:
	Ui::DatasetGeneratorUI* ui;

};






#endif