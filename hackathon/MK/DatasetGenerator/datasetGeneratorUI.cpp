#include "ui_DatasetGenerator.h"
#include "datasetGeneratorUI.h"

using namespace std;

DatasetGeneratorUI::DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback): QDialog(parent), ui(new Ui::DatasetGeneratorUI)
{
	ui->setupUi(this);


	this->show();
}

