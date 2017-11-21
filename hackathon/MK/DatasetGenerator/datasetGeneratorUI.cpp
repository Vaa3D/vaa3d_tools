#include "ui_DatasetGenerator.h"
#include "datasetGeneratorUI.h"
#include "Dataset_Generator_plugin.h"

#include <iostream>

#include <qabstractitemview.h>
#include <qitemselectionmodel.h>
#include <qfilesystemmodel.h>
#include <queue>

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
	ui->lineEdit_20->setText("0.1");

	connect(&DatasetOperator, SIGNAL(progressBarReporter(QString, int)), this, SLOT(progressBarUpdater(QString, int)));

	this->show();
}

DatasetGeneratorUI::~DatasetGeneratorUI()
{
	delete ui;
}

void DatasetGeneratorUI::progressBarUpdater(QString taskName, int percentage)
{
	ui->progressBar->setAlignment(Qt::AlignCenter);
	ui->progressBar->setFormat(taskName + QString::number(percentage) + "%");
	ui->progressBar->setValue(percentage);
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
	else if (pushButtonName == "pushButton_10")
	{
		ui->lineEdit_19->setText(wholePath);
		ui->lineEdit_21->setText(wholePath);
	}
	else if (pushButtonName == "pushButton_14")
	{
		ui->lineEdit_22->setText(wholePath);
	}
	else if (pushButtonName == "pushButton_15")
	{
		ui->lineEdit_24->setText(wholePath);
	}
	else if (pushButtonName == "pushButton_12")
	{
		wholePath = wholePath + "/";
		ui->lineEdit_21->setText(wholePath);
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
		else if (checkBoxName == "groupBox")
		{
			ui->pushButton_8->setEnabled(true);
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
		else if (checkBoxName == "groupBox")
		{
			ui->pushButton_8->setEnabled(false);
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

void DatasetGeneratorUI::associativeToggle(bool checked)
{
	QObject* signalSender = sender();
	QString checkBoxName = signalSender->objectName();

	if (checked == true)
	{
		if (checkBoxName == "checkBox_14")
		{
			if (ui->checkBox_11->isChecked()) ui->groupBox->setChecked(true);
		}
	}
	else
	{
		if (checkBoxName == "checkBox_14")
		{
			if (ui->checkBox_11->isChecked()) ui->groupBox_3->setChecked(false);
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
		newItem->setFlags(newItem->flags() ^ Qt::ItemIsDropEnabled); // XOR for allowing drop feature but not overwrite
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
		newItem->setFlags(newItem->flags() ^ Qt::ItemIsDropEnabled); // XOR for allowing drop feature but not overwrite
		listViewSteps3D->appendRow(newItem);
	}
	else if (emitterName == "pushButton_6")
	{
		QModelIndexList selectedItems = ui->listView_2->selectionModel()->selectedRows();
		int rowNum = selectedItems.begin()->row();
		listViewSteps3D->removeRow(rowNum);
	}
}

void DatasetGeneratorUI::okClicked()
{
	if (ui->groupBox_2->isChecked()) // Create list from existing list.
	{
		taskFromUI newTask;
		newTask.createList = true;
		newTask.createPatch = false;
		newTask.createPatchNList = false;

		QString sourceQString = ui->lineEdit_19->text();
		QString destQString = ui->lineEdit_21->text();
		QString percentageString = ui->lineEdit_20->text();
		newTask.source = sourceQString.toStdString();
		newTask.outputFileName = destQString.toStdString();
		newTask.subsetRatio = percentageString.toDouble();
		newTask.listOp = subset;
		
		DatasetOperator.taskQueu.push(newTask);
		DatasetOperator.taskQueuDispatcher();
	}
	else if (ui->groupBox_8->isChecked())
	{
		taskFromUI newTask;
		newTask.createList = true;
		newTask.createPatch = false;
		newTask.createPatchNList = false;

		QString sourceQString = ui->lineEdit_22->text();
		QString destQString = ui->lineEdit_24->text();
		QString foldNumString = ui->lineEdit_23->text();
		newTask.source = sourceQString.toStdString();
		newTask.outputDirName = destQString.toStdString();
		newTask.foldNum = foldNumString.toInt();
		newTask.listOp = crossVal;

		DatasetOperator.taskQueu.push(newTask);
		DatasetOperator.taskQueuDispatcher();
	}
	else if (ui->checkBox_2->isChecked() && ui->checkBox_10->isChecked()) // Create patches based on neuronstructure file.
	{
		if (!ui->groupBox_6->isChecked() && !ui->groupBox_7->isChecked())
		{ }
		
		taskFromUI newTask;
		newTask.createList = false;
		QString imageSource = ui->lineEdit_7->text();
		QString destImageDir = ui->lineEdit_8->text();
		QString neuronStructFile = ui->lineEdit_15->text();
		newTask.source = imageSource.toStdString();
		newTask.outputDirName = destImageDir.toStdString();
		newTask.neuronStrucFileName = neuronStructFile.toStdString();
		if (ui->checkBox_14->isChecked()) // Create list for the patches at the same time.
		{

		}
		else  // List for the patches will not be created.
		{
			if (ui->checkBox_4->isChecked()) // From terafly
			{
			}
			else // From non-terafly image stack
			{
				newTask.createPatch = true;
				newTask.createPatchNList = false;
				if (ui->groupBox_4->isChecked())
				{
					newTask.patchOp = stackTo2D;
					
					for (int i = 0; i < listViewSteps->rowCount(); ++i)
					{
						QStandardItem* thisItem = listViewSteps->item(i);
						newTask.opSequence.push_back(thisItem->text());
					}
					if (ui->checkBox_7->isChecked()) newTask.dimSelection = xy;
					else if (ui->checkBox_8->isChecked()) newTask.dimSelection = yz;
					else if (ui->checkBox_9->isChecked()) newTask.dimSelection = xz;
					newTask.sideX = ui->lineEdit_9->text().toInt();
					newTask.sideX = ui->lineEdit_10->text().toInt();
					newTask.sideX = ui->lineEdit_11->text().toInt();

					DatasetOperator.taskQueu.push(newTask);
					DatasetOperator.taskQueuDispatcher();
				}
			}
		}
	}
}
