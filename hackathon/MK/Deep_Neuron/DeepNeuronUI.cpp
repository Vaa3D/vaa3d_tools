#include <iostream>

#include <QString>
#include <QStringList>
#include <QTimer>

#include "ui_DeepNeuronForm.h"
#include "DeepNeuronUI.h"

using namespace std;

DeepNeuronUI::DeepNeuronUI(QWidget* parent, V3DPluginCallback2* callback) : QDialog(parent), ui(new Ui::DeepNeuronDialog)
{
	ui->setupUi(this);
	this->mainCallBack = callback;

	QSettings settings("Allen Institute", "Deep Neuron");
	QString savedDeploy = settings.value("deploy file").toString();
	QString savedModel = settings.value("trained model file").toString();
	QString savedMean = settings.value("database mean file").toString();
	ui->lineEdit->setText(savedDeploy);
	ui->lineEdit_2->setText(savedModel);
	ui->lineEdit_3->setText(savedMean);

	if (savedDeploy == "") ui->lineEdit->setText(".prototxt");
	if (savedModel == "") ui->lineEdit_2->setText(".caffemodel");
	if (savedMean == "") ui->lineEdit_3->setText(".binarayproto");
}

DeepNeuronUI::~DeepNeuronUI()
{
	delete ui;
}

void DeepNeuronUI::progressBarUpdater(QString taskName, int percentage)
{
	ui->progressBar->setFormat(taskName + QString::number(percentage) + "%");
	ui->progressBar->setValue(percentage);
}

void DeepNeuronUI::okClicked()
{
	v3dhandle curwin = this->mainCallBack->currentImageWindow();
	if (!curwin)
	{
		QMessageBox::information(0, "", "No image window found.");
		return;
	}
	else
	{
		this->curImg4DPtr = mainCallBack->getImage(curwin);
		if (!curImg4DPtr)
		{
			QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
			return;
		}
		qDebug() << mainCallBack->getImageName(curwin);
		this->curImgProcessor = new imgOperator(this->curImg4DPtr);
	}

	this->curImgDLProcessor = new DLOperator(this->curImgProcessor);
	connect(curImgDLProcessor, SIGNAL(progressBarReporter(QString, int)), this, SLOT(progressBarUpdater(QString, int)));

	if (ui->lineEdit->text() != ".prototxt")
	{
		this->deployDisplay = ui->lineEdit->text();
		this->curImgDLProcessor->deployName = ui->lineEdit->text().toStdString();
	}
	else this->curImgDLProcessor->deployName = deployDisplay.toStdString();

	if (ui->lineEdit_2->text() != ".caffemodel")
	{
		this->modelDisplay = ui->lineEdit_2->text();
		this->curImgDLProcessor->modelName = ui->lineEdit_2->text().toStdString();
	}
	else this->curImgDLProcessor->modelName = modelDisplay.toStdString();

	if (ui->lineEdit_3->text() != ".binaryproto")
	{
		this->meanDisplay = ui->lineEdit_3->text();
		this->curImgDLProcessor->meanName = ui->lineEdit_3->text().toStdString();
	}
	else this->curImgDLProcessor->meanName = meanDisplay.toStdString();

	if (this->deployDisplay == "")
	{
		QMessageBox::information(0, "", "Deploying file is not chosen.");
		return;
	}
	if (this->modelDisplay == "")
	{
		QMessageBox::information(0, "", "Trained model file is not chosen.");
		return;
	}
	if (this->meanDisplay == "")
	{
		QMessageBox::information(0, "", "Image mean file is not chosen.");
		return;
	}
	
	this->curImgDLProcessor->predictSWCstroke(this->mainCallBack, curwin);
}

void DeepNeuronUI::closeClicked()
{
	if (ui->checkBox->isChecked())
	{
		QSettings settings("Allen Institute", "Deep Neuron");
		settings.setValue("deploy file", ui->lineEdit->text());
		settings.setValue("trained model file", ui->lineEdit_2->text());
		settings.setValue("database mean file", ui->lineEdit_3->text());
	}
	this->~DeepNeuronUI();
}

void DeepNeuronUI::filePath()
{
	QObject* emitter = sender();
	QString emitterName = emitter->objectName();

	if (emitterName == "pushButton_3")
	{
		QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "", QObject::tr("Deploying file (*.prototxt)"));
		if (!(fileName == "")) ui->lineEdit->setText(".prototxt");

		ui->lineEdit->setText(fileName);
		this->deployDisplay = fileName;
	}
	else if (emitterName == "pushButton_4")
	{
		QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "", QObject::tr("Trained model file (*.caffemodel)"));
		if (!(fileName == "")) ui->lineEdit_2->setText(".caffemodel");

		ui->lineEdit_2->setText(fileName);
		this->modelDisplay = fileName;		
	}
	else if (emitterName == "pushButton_5")
	{
		QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("open Network Model Files"), "", QObject::tr("Image mean file (*.binaryproto)"));
		if (!(fileName == "")) ui->lineEdit_3->setText(".binaryproto");

		ui->lineEdit_3->setText(fileName);
		this->meanDisplay = fileName;		
	}
}