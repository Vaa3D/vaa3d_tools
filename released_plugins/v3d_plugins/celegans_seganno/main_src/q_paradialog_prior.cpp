//q_paradialog_prior.cpp
//by Lei Qu
//2010-09-28

#include <QtGui>
#include "q_paradialog_prior.h"


ParaDialog::ParaDialog(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog(callback);
}

void ParaDialog::IniDialog(V3DPluginCallback &callback)
{
	//read settings
	QSettings settings("V3D plugin","atlasguided_seganno");
	QString qs_atlasinput_last=settings.value("atlas_input").toString();
	QString qs_atlasoutput_last=settings.value("atlas_output").toString();
	this->lineEdit_atlasfile->setText(qs_atlasinput_last);
	this->lineEdit_atlas_output->setText(qs_atlasoutput_last);

	//image and atlas import
	h_wndlist=callback.getImageWindowList();
	QStringList items;
	for(int i=0;i<h_wndlist.size();i++) items<<callback.getImageName(h_wndlist[i]);
	comboBox_imgv3d->addItems(items);
	//align paras
	this->checkBox_wormupsidedown->setCheckState(Qt::Unchecked);
	this->lineEdit_refchannel->setText("0");
	this->lineEdit_downsampleratio->setText("2.0");
	this->lineEdit_temp->setText("0.2");
	this->lineEdit_fgthresh->setText("3");

	connect(pushButton_atlasfile,SIGNAL(clicked()),this,SLOT(_slots_openAtlasDlg()));
	connect(pushButton_atlas_output,SIGNAL(clicked()),this,SLOT(_slots_saveAtlasDlg()));
}

void ParaDialog::_slots_openAtlasDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose atlas file"));
	d.setNameFilter("Atlas file (*.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_atlasfile->setText(selectedFile);

		//save last opened file name
		QSettings settings("V3D plugin","atlasguided_seganno");
		settings.setValue("atlas_input",selectedFile);
	}
}
void ParaDialog::_slots_saveAtlasDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose atlas output "));
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setNameFilter("Atlas file (*.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_atlas_output->setText(selectedFile);

		//save last opened file name
		QSettings settings("V3D plugin","atlasguided_seganno");
		settings.setValue("atlas_output",selectedFile);
	}
}
