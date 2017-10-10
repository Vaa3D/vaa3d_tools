#include <vector>
#include "IVSCC_scaling_plugin.h"
#include <iostream>
#include "../IVSCC_sort_swc/openSWCDialog.h"
#include "ui_SWC_scaling.h"
#include <qstring.h>

using namespace std;

swcScalingUI::swcScalingUI(QWidget* parent, V3DPluginCallback2* callback) : QDialog(parent), ui(new Ui::swcScalingUI)
{
    ui->setupUi(this);
	this->callback = callback;
	
	ui->lineEdit_3->setText("114.4");
	ui->lineEdit_4->setText("114.4");
	ui->lineEdit_5->setText("280");
	ui->lineEdit_6->setText("280");

	this->show();
}

swcScalingUI::~swcScalingUI()
{
    delete ui;
}

void swcScalingUI::filePath()
{
	OpenSWCDialog* openDlg = new OpenSWCDialog(0, this->callback);
    if (!openDlg->exec()) return;
    this->SWCfileName = openDlg->file_name;
    if(this->SWCfileName == "") 
	{
		cerr << "Not a valid input. Exit with doing nothing." << endl;
		return;
	}

	ui->lineEdit->setText(SWCfileName);
	NeuronTree nt;
	if (SWCfileName.toUpper().endsWith(".SWC") || SWCfileName.toUpper().endsWith(".ESWC")) this->inputNt = openDlg->nt;

	delete openDlg;
}

bool swcScalingUI::okClicked()
{
	inputs << ui->lineEdit_3->text();
	inputs << ui->lineEdit_4->text();
	inputs << ui->lineEdit_5->text();
	inputs << ui->lineEdit_6->text();
	
	accept();
	
	NeuronTree* inputTreePtr = &(this->inputNt);
	scaleSWC(this->inputs, inputTreePtr, this->SWCfileName);

	return true;
}


void scaleSWC(QStringList params, NeuronTree* nt, QString inputSWCName)
{
	if (inputSWCName == "")
	{
		cerr << "Not a valid input. Exit with doing nothing." << endl;
		QString errMsg = "Not a valid input. Exit plugin with doing nothing.";
		v3d_msg(errMsg);
		return;
	}
	qDebug() << inputSWCName << " " << nt->listNeuron.size() << " " << params[0];

	double x_scale = params[0].toDouble();
	double y_scale = params[1].toDouble();
	double z_scale = params[2].toDouble();
	double r_scale = params[3].toDouble();

    NeuronTree nt_scaled;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    NeuronSWC S;
    QList<NeuronSWC> list = nt->listNeuron;

    for(V3DLONG i=0; i<list.size(); ++i)
    {
        S.n =  nt->listNeuron.at(i).n;
        S.x = x_scale * nt->listNeuron.at(i).x;
        S.y = y_scale * nt->listNeuron.at(i).y;
        S.z = z_scale * nt->listNeuron.at(i).z;
        S.r = r_scale * nt->listNeuron.at(i).r;
        S.pn =  nt->listNeuron.at(i).pn;
        S.type =  nt->listNeuron.at(i).type;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }

    nt_scaled.n = -1;
    nt_scaled.on = true;
    nt_scaled.listNeuron = listNeuron;
    nt_scaled.hashNeuron = hashNeuron;

	QString outSWCFileName = inputSWCName + "_scaled.swc";
    writeSWC_file(outSWCFileName, nt_scaled);

	QString FinishMsg = QString("The scaled SWC file [") + outSWCFileName + QString("] has been generated.");
	v3d_msg(FinishMsg);
}