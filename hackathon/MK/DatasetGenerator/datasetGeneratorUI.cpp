#include "ui_DatasetGenerator.h"
#include "datasetGeneratorUI.h"
#include <qabstractitemview.h>
#include <qitemselectionmodel.h>
#include <qfilesystemmodel.h>
#include <iostream>

using namespace std;

DatasetGeneratorUI::DatasetGeneratorUI(QWidget* parent, V3DPluginCallback2* callback): QDialog(parent), ui(new Ui::DatasetGeneratorUI)
{
	ui->setupUi(this);
	
	this->dirModel = new QDirModel(this);
	this->dirModel->setReadOnly(false);
	ui->treeView->setModel(this->dirModel);
	ui->treeView->hideColumn(1);
	ui->treeView->hideColumn(2);
	ui->treeView->hideColumn(3);
	ui->treeView->setCurrentIndex(this->dirModel->index(QDir::currentPath()));

	ui->checkBox_4->setEnabled(false);
	ui->checkBox_5->setEnabled(false);
	ui->checkBox_6->setEnabled(false);

	procSteps = new QStringListModel(this);
	listViewSteps = new QStringListModel(this);
	this->procItems << "crop" << "Maximum Intensity Projection" << "Central Slice";
	procSteps->setStringList(this->procItems);
	listViewSteps->setStringList(this->selectedProcItems);
	ui->comboBox->setModel(procSteps);
	ui->listView->setModel(listViewSteps);

	this->show();
}

DatasetGeneratorUI::~DatasetGeneratorUI()
{
	delete ui;
}

void DatasetGeneratorUI::selectClicked()
{
	QModelIndexList indexList = ui->treeView->selectionModel()->selectedIndexes();
	QModelIndex selectedEntry = *(indexList.begin());
	QString wholePath = this->dirModel->fileInfo(selectedEntry).absoluteFilePath();

	QObject* signalSender = sender();
	QString pushButtonName = signalSender->objectName();
	if (pushButtonName == "pushButton_2") ui->lineEdit_7->setText(wholePath);
	else if (pushButtonName == "pushButton_3") ui->lineEdit_8->setText(wholePath);
}

void DatasetGeneratorUI::checkboxToggled(bool checked)
{
	QObject* signalSender = sender();
	QString checkBoxName = signalSender->objectName();
	
	if (checked == true)
	{
		if (checkBoxName == "checkBox_2")
		{
			ui->checkBox_5->setEnabled(false);
			ui->checkBox_6->setEnabled(false);
			ui->checkBox_4->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_3")
		{
			ui->checkBox_4->setEnabled(false);
			ui->checkBox_5->setEnabled(true);
			ui->checkBox_6->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_6") ui->groupBox_3->setEnabled(false);
	}
	else
	{
		if (checkBoxName == "checkBox_2")
		{
			ui->checkBox_3->setEnabled(true);
			ui->checkBox_5->setEnabled(true);
			ui->checkBox_6->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_3")
		{
			ui->checkBox_2->setEnabled(true);
			ui->checkBox_4->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_6") ui->groupBox_3->setEnabled(true);
	}
}

void DatasetGeneratorUI::exclusiveToggle(bool checked)
{
	QObject* signalSender = sender();
	QString checkBoxName = signalSender->objectName();

	if (checked == true)
	{
		if (checkBoxName == "checkBox_5") ui->checkBox_6->setChecked(false);
		else if (checkBoxName == "checkBox_6") ui->checkBox_5->setChecked(false);
		else if (checkBoxName == "groupBox_4") ui->groupBox_3->setChecked(false);
		else if (checkBoxName == "groupBox_3") ui->groupBox_4->setChecked(false);
		else if (checkBoxName == "checkBox_2") ui->checkBox_3->setChecked(false);
		else if (checkBoxName == "checkBox_3") ui->checkBox_2->setChecked(false);
	}
}

void DatasetGeneratorUI::preprocessingEdit()
{
	QString currItemName = ui->comboBox->currentText();
	//qDebug() << currItemName;

	QObject* emitter = sender();
	QString emitterName = emitter->objectName();
	int listViewCount = ui->listView->model()->rowCount();
	if (emitterName == "pushButton")
	{
		listViewSteps->insertRow(listViewCount);
		QModelIndex newEntry = listViewSteps->index(listViewCount);
		listViewSteps->setData(newEntry, currItemName);
		ui->listView->setCurrentIndex(newEntry);
		
		

		
	}
	else if (emitterName == "pushButton_4")
	{


	}
	else if (emitterName == "pushButton_5")
	{

	}
}

