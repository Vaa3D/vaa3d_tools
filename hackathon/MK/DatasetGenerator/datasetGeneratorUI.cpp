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
	ui->checkBox_11->setEnabled(false);
	ui->checkBox_12->setEnabled(false);
	ui->checkBox_13->setEnabled(false);

	procSteps = new QStringListModel(this);
	procSteps3D = new QStringListModel(this);
	listViewSteps = new QStandardItemModel(this);
	listViewSteps3D = new QStandardItemModel(this);
	this->procItems << "crop" << "Maximum Intensity Projection" << "Central Slice";
	procSteps->setStringList(this->procItems);
	procSteps3D->setStringList(this->procItems);
	ui->comboBox->setModel(procSteps);
	ui->comboBox_2->setModel(procSteps);
	ui->listView->setModel(listViewSteps);
	ui->listView_2->setModel(listViewSteps3D);

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
	else if (pushButtonName == "pushButton_7")
	{
		if (ui->checkBox_14->isChecked() && ui->checkBox_11->isChecked())
		{
			ui->groupBox->setChecked(true);
			ui->lineEdit_16->setText(wholePath);
		}
		ui->lineEdit_15->setText(wholePath);
	}
	else if (pushButtonName == "pushButton_9")
	{
		if (ui->checkBox_14->isChecked() && ui->checkBox_11->isChecked())
		{
			ui->groupBox->setChecked(true);
			ui->lineEdit_16->setText(wholePath);
		}
		ui->lineEdit_17->setText(wholePath);
	}
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
			ui->groupBox_3->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_3")
		{
			ui->checkBox_4->setEnabled(false);
			ui->checkBox_5->setEnabled(true);
			ui->checkBox_6->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_6")
		{
			ui->groupBox_3->setEnabled(false);
			ui->checkBox_7->setEnabled(false);
			ui->checkBox_8->setEnabled(false);
			ui->checkBox_9->setEnabled(false);
		}
		else if (checkBoxName == "checkBox_5")
		{
			ui->groupBox_3->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_10")
		{
			ui->checkBox_11->setEnabled(true);
			ui->checkBox_12->setEnabled(true);
			ui->checkBox_13->setEnabled(true);
		}
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
		else if (checkBoxName == "checkBox_6")
		{
			ui->groupBox_3->setEnabled(true);
			ui->checkBox_7->setEnabled(true);
			ui->checkBox_8->setEnabled(true);
			ui->checkBox_9->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_10")
		{
			ui->checkBox_11->setEnabled(false);
			ui->checkBox_12->setEnabled(false);
			ui->checkBox_13->setEnabled(false);
		}
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
		else if (checkBoxName == "checkBox_2")
		{
			ui->checkBox_3->setChecked(false);
			ui->checkBox_6->setChecked(false);
		}
		else if (checkBoxName == "checkBox_3") ui->checkBox_2->setChecked(false);
		else if (checkBoxName == "groupBox_6") ui->groupBox_7->setChecked(false);
		else if (checkBoxName == "groupBox_7") ui->groupBox_6->setChecked(false);
		else if (checkBoxName == "checkBox_11")
		{
			ui->checkBox_12->setChecked(false);
			ui->checkBox_13->setChecked(false);
		}
		else if (checkBoxName == "checkBox_12")
		{
			ui->checkBox_11->setChecked(false);
			ui->checkBox_13->setChecked(false);
		}
		else if (checkBoxName == "checkBox_13")
		{
			ui->checkBox_11->setChecked(false);
			ui->checkBox_12->setChecked(false);
		}
	}
}

void DatasetGeneratorUI::preprocessingEdit()
{
	QObject* emitter = sender();
	QString emitterName = emitter->objectName();
	QString currItemName;
	if (emitterName == "pushButton") currItemName = ui->comboBox->currentText();
	else if (emitterName == "pushButton_4") currItemName = ui->comboBox_2->currentText();
	int listViewCount = ui->listView->model()->rowCount();
	int listViewCount3D = ui->listView_2->model()->rowCount();
	if (emitterName == "pushButton")
	{		
		QStandardItem* newItem = new QStandardItem(currItemName);
		newItem->setFlags(newItem->flags() ^ Qt::ItemIsDropEnabled);
		listViewSteps->appendRow(newItem);
	}
	else if (emitterName == "pushButton_5")
	{
		QModelIndexList selectedItems = ui->listView->selectionModel()->selectedRows();
		int rowNum = selectedItems.begin()->row();
		listViewSteps->removeRow(rowNum);
	}
	else if (emitterName == "pushButton_4")
	{
		QStandardItem* newItem = new QStandardItem(currItemName);
		newItem->setFlags(newItem->flags() ^ Qt::ItemIsDropEnabled);
		listViewSteps3D->appendRow(newItem);
	}
	else if (emitterName == "pushButton_6")
	{
		QModelIndexList selectedItems = ui->listView_2->selectionModel()->selectedRows();
		int rowNum = selectedItems.begin()->row();
		listViewSteps3D->removeRow(rowNum);
	}
}

