//q_paradialog_straighten.cpp
//by Lei Qu
//2010-08-11

#include <QtGui>
#include "q_paradialog_straighten.h"


ParaDialog::ParaDialog(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog(callback);
}

void ParaDialog::IniDialog(V3DPluginCallback &callback)
{
	//image and atlas import
	h_wndlist=callback.getImageWindowList();
	QStringList items;
	for(int i=0;i<h_wndlist.size();i++) items<<callback.getImageName(h_wndlist[i]);
	comboBox_imgv3d->addItems(items);
	//visualization
	this->checkBox_showV3D_pts->setCheckState(Qt::Unchecked);
	//skeleton detection
	this->lineEdit_refchannel->setText("1");
	this->lineEdit_downsampleratio->setText("4.0");
	this->lineEdit_ctlpts_num->setText("10");
	this->lineEdit_radius_openclose->setText("0");
	this->lineEdit_fgthresh_xy->setText("3.0");
	this->lineEdit_fgthresh_xz->setText("0.0");
	//straighten
	this->lineEdit_radius_cuttingplane->setText("100");

	connect(pushButton_imgfile,SIGNAL(clicked()),this,SLOT(_slots_openImgDlg()));
	connect(pushButton_markerfile,SIGNAL(clicked()),this,SLOT(_slots_openMarkerDlg()));
	connect(pushButton_strimg_output,SIGNAL(clicked()),this,SLOT(_slots_saveStrImgDlg()));
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
		QString output=selectedFile+QString("_str.raw");
		this->lineEdit_strimg_output->setText(output);
	}
}
void ParaDialog::_slots_openMarkerDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose marker file"));
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_markerfile->setText(selectedFile);
	}
}
void ParaDialog::_slots_saveStrImgDlg()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Save straightened image "));
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setNameFilter("Image file (*.tif *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_strimg_output->setText(selectedFile);
	}
}
