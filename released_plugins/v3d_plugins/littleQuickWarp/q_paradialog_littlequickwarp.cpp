//q_paradialog_littlequickwarp.cpp
//by Lei Qu
//2011-04-11

#include <QtGui>
#include "q_paradialog_littlequickwarp.h"


CParaDialog_littlequickwarp::CParaDialog_littlequickwarp(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog();
}
CParaDialog_littlequickwarp::~CParaDialog_littlequickwarp()
{
	QSettings settings("V3D plugin","littlequickwarp");

	settings.setValue("img_sub",this->lineEdit_img_sub->text());
	settings.setValue("img_warp",this->lineEdit_img_warp->text());
	settings.setValue("marker_sub",this->lineEdit_marker_sub->text());
	settings.setValue("marker_tar",this->lineEdit_marker_tar->text());

	settings.setValue("groupBox_resize",this->groupBox_resize->isChecked());
	settings.setValue("xdim",this->lineEdit_Xdim->text());
	settings.setValue("ydim",this->lineEdit_Ydim->text());
	settings.setValue("zdim",this->lineEdit_Zdim->text());

	settings.setValue("dopadding",this->checkBox_padding->isChecked());
	settings.setValue("interpmethod",this->radioButton_interp_linear->isChecked());
}

void CParaDialog_littlequickwarp::IniDialog()
{
	//read settings
	QSettings settings("V3D plugin","littlequickwarp");
	this->lineEdit_img_sub->setText(settings.value("img_sub").toString());
	this->lineEdit_img_warp->setText(settings.value("img_warp").toString());
	this->lineEdit_marker_sub->setText(settings.value("marker_sub").toString());
	this->lineEdit_marker_tar->setText(settings.value("marker_tar").toString());

	this->groupBox_resize->setChecked(settings.value("groupBox_resize",0).toBool());
	this->lineEdit_Xdim->setText(settings.value("xdim",0).toString());
	this->lineEdit_Ydim->setText(settings.value("ydim",0).toString());
	this->lineEdit_Zdim->setText(settings.value("zdim",0).toString());

	this->checkBox_padding->setChecked(settings.value("dopadding",0).toBool());
	this->radioButton_interp_nn->setChecked(!(settings.value("interpmethod",0).toBool()));
	this->radioButton_interp_linear->setChecked(settings.value("interpmethod",1).toBool());

	connect(pushButton_img_sub,SIGNAL(clicked()),this,SLOT(_slots_openDlg_img_sub()));
	connect(pushButton_img_warp,SIGNAL(clicked()),this,SLOT(_slots_openDlg_img_warp()));
	connect(pushButton_marker_sub,SIGNAL(clicked()),this,SLOT(_slots_openDlg_marker_sub()));
	connect(pushButton_marker_tar,SIGNAL(clicked()),this,SLOT(_slots_openDlg_marker_tar()));
}

void CParaDialog_littlequickwarp::_slots_openDlg_img_sub()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose subject image file"));
	d.setAcceptMode(QFileDialog::AcceptOpen);
	d.setFileMode(QFileDialog::ExistingFiles);
	d.setNameFilter("Image file (*.tiff *.tif *.raw *.lsm *.v3draw *.vaa3draw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_img_sub->setText(selectedFile);
	}
}
void CParaDialog_littlequickwarp::_slots_openDlg_marker_sub()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose subject marker file"));
	d.setAcceptMode(QFileDialog::AcceptOpen);
	d.setFileMode(QFileDialog::ExistingFiles);
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_marker_sub->setText(selectedFile);
	}
}
void CParaDialog_littlequickwarp::_slots_openDlg_marker_tar()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose target marker file"));
	d.setAcceptMode(QFileDialog::AcceptOpen);
	d.setFileMode(QFileDialog::ExistingFiles);
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_marker_tar->setText(selectedFile);
	}
}
void CParaDialog_littlequickwarp::_slots_openDlg_img_warp()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose warpped image output (*.tif *.raw *.lsm *.v3draw *.vaa3draw)"));
	d.setAcceptMode(QFileDialog::AcceptSave);
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_img_warp->setText(selectedFile);
	}
}
