//q_paradialog_rigidaffine.cpp
//by Lei Qu
//2011-04-11

#include <QtGui>
#include "q_paradialog_rigidaffine.h"


CParaDialog_rigidaffine::CParaDialog_rigidaffine(V3DPluginCallback &callback,QWidget *parent):QDialog(parent)
{
	setupUi(this);
	IniDialog();
}
CParaDialog_rigidaffine::~CParaDialog_rigidaffine()
{
	QSettings settings("V3D plugin","image_registration_rigidaffine");
	settings.setValue("img_tar",this->lineEdit_img_tar->text());
	settings.setValue("img_sub",this->m_qsSelectedFiles_sub);
	settings.setValue("img_sub2tar",this->lineEdit_img_sub2tar->text());
	settings.setValue("swc_grid",this->lineEdit_swc_grid->text());
	settings.setValue("refchannel",this->lineEdit_refchannel->text());
	settings.setValue("downsampleratio",this->lineEdit_downsampleratio->text());
	settings.setValue("histomatch",this->checkBox_histogrammatching->isChecked());
	settings.setValue("alignedges",this->checkBox_alignedges->isChecked());
	settings.setValue("gau_smooth",this->groupBox_Gaussian->isChecked());
	settings.setValue("gau_radius",this->lineEdit_Gau_radius->text());
	settings.setValue("gau_sigma",this->lineEdit_Gau_sigma->text());

	settings.setValue("iter_max",this->lineEdit_iter_max->text());
	settings.setValue("step_inimulti",this->lineEdit_step_multiplyfactor->text());
	settings.setValue("step_annealing",this->lineEdit_step_annealingrate->text());
	settings.setValue("step_min",this->lineEdit_step_min->text());
}

void CParaDialog_rigidaffine::IniDialog()
{
	QSettings settings("V3D plugin","image_registration_rigidaffine");
	this->lineEdit_img_tar->setText(settings.value("img_tar").toString());
	this->m_qsSelectedFiles_sub=settings.value("img_sub").toStringList();
	this->lineEdit_img_sub2tar->setText(settings.value("img_sub2tar").toString());
	this->lineEdit_swc_grid->setText(settings.value("swc_grid").toString());
	QString selectedFiles;
	for(long i=0;i<m_qsSelectedFiles_sub.size();i++)
		selectedFiles=selectedFiles+";"+m_qsSelectedFiles_sub[i];
	this->lineEdit_img_sub->setText(selectedFiles);
	this->lineEdit_refchannel->setText(settings.value("refchannel",1).toString());
	this->lineEdit_downsampleratio->setText(settings.value("downsampleratio",4).toString());
	this->checkBox_histogrammatching->setChecked(settings.value("histomatch",1).toBool());
	this->checkBox_alignedges->setChecked(settings.value("alignedges",0).toBool());
	this->groupBox_Gaussian->setChecked(settings.value("gau_smooth",1).toBool());
	this->lineEdit_Gau_radius->setText(settings.value("gau_radius",2).toString());
	this->lineEdit_Gau_sigma->setText(settings.value("gau_sigma",0.5).toString());
	this->lineEdit_iter_max->setText(settings.value("iter_max",500).toString());
	this->lineEdit_step_multiplyfactor->setText(settings.value("step_inimulti",5).toString());
	this->lineEdit_step_annealingrate->setText(settings.value("step_annealing",0.95).toString());
	this->lineEdit_step_min->setText(settings.value("step_min",0.01).toString());

	this->lineEdit_refchannel->setValidator(new QIntValidator(1,4,this));

	connect(pushButton_img_tar,SIGNAL(clicked()),this,SLOT(_slots_openDlg_tar()));
	connect(pushButton_img_sub,SIGNAL(clicked()),this,SLOT(_slots_openDlg_sub()));
	connect(pushButton_img_sub2tar,SIGNAL(clicked()),this,SLOT(_slots_saveDlg_sub2tar()));
	connect(pushButton_swc_grid,SIGNAL(clicked()),this,SLOT(_slots_saveDlg_grid()));
}

void CParaDialog_rigidaffine::_slots_openDlg_tar()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose target image file"));
	d.setAcceptMode(QFileDialog::AcceptOpen);
	d.setNameFilter("Image file (*.tif *.raw *.lsm *.v3draw *.vaa3draw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_img_tar->setText(selectedFile);
	}
}
void CParaDialog_rigidaffine::_slots_openDlg_sub()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose subject image file"));
	d.setAcceptMode(QFileDialog::AcceptOpen);
	d.setFileMode(QFileDialog::ExistingFiles);
	d.setNameFilter("Image file (*.tif *.raw *.lsm *.v3draw *.vaa3draw)");
	if(d.exec())
	{
		m_qsSelectedFiles_sub=d.selectedFiles();
		QString selectedFiles;
		for(long i=0;i<m_qsSelectedFiles_sub.size();i++)
			selectedFiles=selectedFiles+";"+m_qsSelectedFiles_sub[i];
		this->lineEdit_img_sub->setText(selectedFiles);
	}
}
void CParaDialog_rigidaffine::_slots_saveDlg_sub2tar()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose warpped image output "));
	d.setAcceptMode(QFileDialog::AcceptSave);
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_img_sub2tar->setText(selectedFile);
	}
}
void CParaDialog_rigidaffine::_slots_saveDlg_grid()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose warpped grid output (*.swc)"));
	d.setAcceptMode(QFileDialog::AcceptSave);
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		this->lineEdit_swc_grid->setText(selectedFile);
	}
}
