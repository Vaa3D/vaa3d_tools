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

	ui->lineEdit->setText(".prototxt");
	ui->lineEdit_2->setText(".caffemodel");
	ui->lineEdit_3->setText(".binaryproto");
}

void DeepNeuronUI::uiCall()
{
	this->show();
	this->setWindowModality(Qt::NonModal);
	this->setVisible(true);
}

void DeepNeuronUI::okClicked()
{
	cout << "haha" << endl;
}