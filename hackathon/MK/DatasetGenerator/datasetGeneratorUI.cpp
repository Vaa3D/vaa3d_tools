#include "ui_DatasetGenerator.h"
#include "datasetGeneratorUI.h"

using namespace std;

DatasetGeneratorUI::DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback): QDialog(parent), ui(new Ui::DatasetGeneratorUI)
{
	ui->setupUi(this);
	dirModel = new QDirModel(this);
	dirModel->setReadOnly(false);
	ui->treeView->setModel(dirModel);
	ui->treeView->hideColumn(1);
	ui->treeView->hideColumn(2);
	ui->treeView->hideColumn(3);
	ui->treeView->setCurrentIndex(dirModel->index(QDir::currentPath()));

	this->show();
}

