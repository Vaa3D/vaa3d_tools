//q_paradialog_stranno.cpp
//by Lei Qu
//2010-11-04

#include <QtGui>
#include "q_paradialog_stranno.h"


CParaDialog_stranno::CParaDialog_stranno(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog(callback);
}
CParaDialog_stranno::~CParaDialog_stranno()
{
	//save setting
	//file io
	QSettings settings("V3D plugin","atlasguided_stranno");
	settings.setValue("atlas_input",this->lineEdit_atlasfile->text());
	settings.setValue("celloi_input",this->lineEdit_celloifile->text());
	settings.setValue("atlas_output",this->lineEdit_atlas_output->text());
	settings.setValue("seglabel_output",this->lineEdit_seglabel_output->text());
	//visualization
	settings.setValue("show_pts",this->checkBox_showatlas->isChecked());
	settings.setValue("show_seg",this->checkBox_showsegmentation->isChecked());
	//paras
	settings.setValue("refchannel",this->lineEdit_refchannel->text());
	settings.setValue("downsampleratio",this->lineEdit_downsampleratio->text());
	settings.setValue("fgthresh",this->lineEdit_fgthresh->text());
	settings.setValue("temp_ini",this->lineEdit_temp_ini->text());
	settings.setValue("temp_min",this->lineEdit_temp_min->text());
	settings.setValue("annealingrate",this->lineEdit_annealingrate->text());
	settings.setValue("niter_pertemp",this->lineEdit_niter_pertemp->text());
}

void CParaDialog_stranno::IniDialog(V3DPluginCallback &callback)
{
	//read settings
	QSettings settings("V3D plugin","atlasguided_stranno");
	//file io
	this->lineEdit_atlasfile->setText(settings.value("atlas_input").toString());
	this->lineEdit_celloifile->setText(settings.value("celloi_input").toString());
	this->lineEdit_atlas_output->setText(settings.value("atlas_output").toString());
	this->lineEdit_seglabel_output->setText(settings.value("seglabel_output").toString());
	//visualization
	if(settings.value("show_pts",0).toBool()) this->checkBox_showatlas->setCheckState(Qt::Checked);
	else              	                     this->checkBox_showatlas->setCheckState(Qt::Unchecked);
	if(settings.value("show_seg",0).toBool()) this->checkBox_showsegmentation->setCheckState(Qt::Checked);
	else              	                     this->checkBox_showsegmentation->setCheckState(Qt::Unchecked);
	//paras
	this->lineEdit_refchannel->setText(settings.value("refchannel",2).toString());
	this->lineEdit_downsampleratio->setText(settings.value("downsampleratio",4).toString());
	this->lineEdit_fgthresh->setText(settings.value("fgthresh",3).toString());
	this->lineEdit_temp_ini->setText(settings.value("temp_ini",20).toString());
	this->lineEdit_temp_min->setText(settings.value("temp_min",0.2).toString());
	this->lineEdit_annealingrate->setText(settings.value("annealingrate",0.95).toString());
	this->lineEdit_niter_pertemp->setText(settings.value("niter_pertemp",1).toString());

	//get image list from v3d
	h_wndlist=callback.getImageWindowList();
	QStringList items;
	for(int i=0;i<h_wndlist.size();i++) items<<callback.getImageName(h_wndlist[i]);
	comboBox_imgv3d->addItems(items);

	connect(pushButton_atlasfile,SIGNAL(clicked()),this,SLOT(_slots_openAtlasDlg()));
	connect(pushButton_celloifile,SIGNAL(clicked()),this,SLOT(_slots_openCelloiDlg()));
	connect(pushButton_celloifile_2,SIGNAL(clicked()),this,SLOT(_slots_openCelloiDlg_2()));
	connect(pushButton_atlas_output,SIGNAL(clicked()),this,SLOT(_slots_saveAtlasDlg()));
	connect(pushButton_seglabel_output,SIGNAL(clicked()),this,SLOT(_slots_saveSeglabelDlg()));
}

void CParaDialog_stranno::_slots_openAtlasDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose atlas file"));
	d.setNameFilter("Atlas file (*.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_atlasfile->setText(selectedFile);
	}
}
void CParaDialog_stranno::_slots_openCelloiDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose interesting cell file"));
	d.setNameFilter("Txt file (*.txt)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_celloifile->setText(selectedFile);
	}
}
void CParaDialog_stranno::_slots_openCelloiDlg_2()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose secondary interesting cell file"));
	d.setNameFilter("Txt file (*.txt)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_celloifile_2->setText(selectedFile);
	}
}
void CParaDialog_stranno::_slots_saveAtlasDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose atlas output "));
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setNameFilter("Atlas file (*.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_atlas_output->setText(selectedFile);
	}
}
void CParaDialog_stranno::_slots_saveSeglabelDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose seg-label image output "));
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setNameFilter("Image file (*.raw *.tif *.lsm)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_seglabel_output->setText(selectedFile);
	}
}
