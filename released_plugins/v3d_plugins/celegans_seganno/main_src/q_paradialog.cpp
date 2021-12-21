//q_paradialog.cpp
//by Lei Qu
//2010-07-31

#include <QtGui>
#include "q_paradialog.h"


ParaDialog::ParaDialog(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog(callback);
}
ParaDialog::~ParaDialog()
{
	//save setting (only path)
	QSettings settings("V3D plugin","atlasguided_stranno");
	settings.setValue("atlas_input",this->lineEdit_atlasfile->text());
	settings.setValue("atlas_output",this->lineEdit_atlas_output->text());
}

void ParaDialog::IniDialog(V3DPluginCallback &callback)
{
	//read settings
	QSettings settings("V3D plugin","atlasguided_stranno");
	QString qs_atlasinput_last=settings.value("atlas_input").toString();
	QString qs_atlasoutput_last=settings.value("atlas_output").toString();
	this->lineEdit_atlasfile->setText(qs_atlasinput_last);
	this->lineEdit_atlas_output->setText(qs_atlasoutput_last);

	//image and atlas import
	h_wndlist=callback.getImageWindowList();
	QStringList items;
	for(int i=0;i<h_wndlist.size();i++) items<<callback.getImageName(h_wndlist[i]);
	comboBox_imgv3d->addItems(items);
	//visualization
	this->checkBox_showV3D_img->setCheckState(Qt::Unchecked);
	this->checkBox_showV3D_pts->setCheckState(Qt::Unchecked);
	//initial align paras
	this->checkBox_wormupsidedown->setCheckState(Qt::Unchecked);
	this->lineEdit_refchannel->setText("1");
	this->lineEdit_downsampleratio->setText("4.0");
	this->lineEdit_temp_ini->setText("5.0");
	this->lineEdit_temp_min->setText("0.2");
	this->lineEdit_annealingrate->setText("0.95");
	this->lineEdit_niter_pertemp->setText("1");
	//refined align paras
	this->checkBox_refinealign_simple->setCheckState(Qt::Checked);
	this->lineEdit_ref_temperature->setText("0.1");
	this->lineEdit_ref_cellradius->setText("8");
	this->lineEdit_ref_minposchange->setText("1.0");
	this->lineEdit_ref_maxiter->setText("100");

	connect(pushButton_imgfile,SIGNAL(clicked()),this,SLOT(_slots_openImgDlg()));
	connect(pushButton_atlasfile,SIGNAL(clicked()),this,SLOT(_slots_openAtlasDlg()));
	connect(pushButton_atlas_output,SIGNAL(clicked()),this,SLOT(_slots_saveAtlasDlg()));
}

void ParaDialog::_slots_openImgDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose image file"));
	d.setNameFilter("Image file (*.tif *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_imfile->setText(selectedFile);
		//set default output filepath
		QString outputAtlas=selectedFile+QString(".apo");
		this->lineEdit_atlas_output->setText(outputAtlas);
	}
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
	}
}
