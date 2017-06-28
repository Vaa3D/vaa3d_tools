#include "../../zhi/IVSCC_sort_swc/openSWCDialog.h"
#include <vector>
#include "Tip_marker_finder_plugin.h"
#include <v3d_interface.h>
#include "v3d_message.h"
#include "tip_marker_func.h"
#include <vector>
#include <iostream>
#include "../../../released_plugins/v3d_plugins/gsdt/common_dialog.h"
#include "ui_TipMarkerFinderUI.h"


using namespace std;

const QString title = QObject::tr("Border Marker Finder");

TipMarkerFinderUI::TipMarkerFinderUI(QWidget* parent, V3DPluginCallback2* callback, int menu) :
    QDialog(parent),
    ui(new Ui::TipMarkerFinderUI)
{
    ui->setupUi(this);
	if (menu == 2) 
	{
		ui->lineEdit_3->setEnabled(false);
		ui->lineEdit_4->setEnabled(false);
	}

	this->show();
}

TipMarkerFinderUI::~TipMarkerFinderUI()
{
    delete ui;
}

void TipMarkerFinderUI::filePath()
{
	SWCfileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the SWC file to look into"),
                                               "", QObject::tr("Supported file (*.swc *.eswc)"
                                               ";;Neuron structure	(*.swc)"
                                               ";;Extended neuron structure (*.eswc)"));

	ui->lineEdit->setText(SWCfileName);
}

bool TipMarkerFinderUI::okClicked()
{
	inputs << ui->lineEdit->text();
	inputs << ui->lineEdit_3->text();
	inputs << ui->lineEdit_4->text();

	QString test = inputs[1];
	bool done;
	if (test.isEmpty()) done = TipProcessor(inputs, 2);
	else done = TipProcessor(inputs, 1);
	qDebug() << inputs;
	
	accept();
	
	return true;
}

bool TipProcessor(QStringList input, int menu)
{
		NeuronTree nt = readSWC_file(input[0]);

		QVector<QVector<V3DLONG> > childs;
		V3DLONG neuronNum = nt.listNeuron.size();
		childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			V3DLONG par = nt.listNeuron[i].pn;
			if (par<0) continue;
			childs[nt.hashNeuron.value(par)].push_back(i);
		}

		QList<NeuronSWC> list = nt.listNeuron;
		QList<ImageMarker> bifur_marker;

		if (menu == 1)
		{
			int top = input[1].toInt();
			int bottom = input[2].toInt();
			int z_coord;
			for (int i=0;i<list.size();i++)
			{
				ImageMarker t;

				if (nt.listNeuron.at(i).type == 1) 
				{
					t.x = nt.listNeuron.at(i).x;
					t.y = nt.listNeuron.at(i).y;
					t.z = nt.listNeuron.at(i).z;
					t.type = nt.listNeuron.at(i).type;
					bifur_marker.append(t);
					cout << nt.listNeuron.at(i).type << endl;
				}

				if (childs[i].size() == 0)
				{
					if (nt.listNeuron.at(i).type == 3 || nt.listNeuron.at(i).type == 4)
					{
						z_coord = int(nt.listNeuron.at(i).z);
						if ((z_coord >= bottom) || (z_coord <= top))
						{
							t.x = nt.listNeuron.at(i).x;
							t.y = nt.listNeuron.at(i).y;
							t.z = nt.listNeuron.at(i).z;
							t.type = nt.listNeuron.at(i).type;
							bifur_marker.append(t);
							cout << nt.listNeuron.at(i).type << endl;
						}
					}
				}
			}
		}
		else if (menu == 2)
		{
			int z_coord;
			for (int i=0;i<list.size();i++)
			{
				ImageMarker t;

				if (nt.listNeuron.at(i).type == 1) 
				{
					t.x = nt.listNeuron.at(i).x;
					t.y = nt.listNeuron.at(i).y;
					t.z = nt.listNeuron.at(i).z;
					t.type = nt.listNeuron.at(i).type;
					bifur_marker.append(t);
					cout << nt.listNeuron.at(i).type << endl;
				}

				if (childs[i].size() == 0)
				{
					t.x = nt.listNeuron.at(i).x;
					t.y = nt.listNeuron.at(i).y;
					t.z = nt.listNeuron.at(i).z;
					t.type = nt.listNeuron.at(i).type;
					bifur_marker.append(t);
				}
			}
		}

		QString fileSaveName = input[0];
		QString markerfileName = fileSaveName+QString("_TipMarker.marker");
		writeMarker_file(markerfileName, bifur_marker);
		QString FinishMsg = QString("A marker file [") + markerfileName + QString("] has been generated.");
		v3d_msg(FinishMsg);
        
		return true;
}




