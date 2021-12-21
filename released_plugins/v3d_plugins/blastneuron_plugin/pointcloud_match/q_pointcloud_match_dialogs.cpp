// q_pointcloud_match_dialog.cpp
// provide the dialogs class definition
//by Lei Qu
//2009-11-10
// seperated from plugin_pointcloud_match.cpp by Lei Qu on 2010-01-23

#include <QtGui>
#include "q_pointcloud_match_dialogs.h"


//------------------------------------------------------------------------------------------------------------------------------------
// Choose2FileDialog_marker
Choose2FileDialog::Choose2FileDialog(QWidget *parent):QDialog(parent)
{
	_pLineEdit_filepath_1=new QLineEdit(QObject::tr("choose file 1 here"));
	_pLineEdit_filepath_2=new QLineEdit(QObject::tr("choose file 2 here"));
	_pLineEdit_filepath_1->setFixedWidth(300);
	_pLineEdit_filepath_2->setFixedWidth(300);

	QPushButton *pPushButton_openFileDialog_1	=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_2	=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_ok					=new QPushButton("OK");									pPushButton_ok->setDefault(true);
	QPushButton *pPushButton_cancel				=new QPushButton("Cancel");

	connect(pPushButton_openFileDialog_1,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_1()));
	connect(pPushButton_openFileDialog_2,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_2()));
	connect(pPushButton_ok,	    		 SIGNAL(clicked()),this,SLOT(accept()));
	connect(pPushButton_cancel, 		 SIGNAL(clicked()),this,SLOT(reject()));

	QGroupBox *pGroupBox_filechoose_1=new QGroupBox(parent);
	QGroupBox *pGroupBox_filechoose_2=new QGroupBox(parent);
	pGroupBox_filechoose_1->setTitle(QObject::tr("Choose file 1:"));
	pGroupBox_filechoose_2->setTitle(QObject::tr("Choose file 2:"));

	QHBoxLayout *pLayout_filechoose_1=new QHBoxLayout(pGroupBox_filechoose_1);
	pLayout_filechoose_1->addWidget(_pLineEdit_filepath_1);
	pLayout_filechoose_1->addWidget(pPushButton_openFileDialog_1);
	QHBoxLayout *pLayout_filechoose_2=new QHBoxLayout(pGroupBox_filechoose_2);
	pLayout_filechoose_2->addWidget(_pLineEdit_filepath_2);
	pLayout_filechoose_2->addWidget(pPushButton_openFileDialog_2);

	QHBoxLayout *pLayout_okcancel=new QHBoxLayout;
	pLayout_okcancel->addWidget(pPushButton_ok);
	pLayout_okcancel->addWidget(pPushButton_cancel);

	QVBoxLayout *pLayout_main=new QVBoxLayout;
	pLayout_main->addWidget(pGroupBox_filechoose_1);
	pLayout_main->addWidget(pGroupBox_filechoose_2);
	pLayout_main->addLayout(pLayout_okcancel);
	pLayout_main->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Choose two files for matching"));
	setLayout(pLayout_main);
}
void Choose2FileDialog::_slots_openFileDlg_1()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose file 1"));
	d.setNameFilter("Point cloud file (*.marker *.swc *.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		_pLineEdit_filepath_1->setText(selectedFile);
	}
}
void Choose2FileDialog::_slots_openFileDlg_2()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose file 2"));
	d.setNameFilter("Point cloud file (*.marker *.swc *.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		_pLineEdit_filepath_2->setText(selectedFile);
	}
}
QString Choose2FileDialog::m_getFilename_1()
{
	return _pLineEdit_filepath_1->text();
}
QString Choose2FileDialog::m_getFilename_2()
{
	return _pLineEdit_filepath_2->text();
}


//------------------------------------------------------------------------------------------------------------------------------------
ChooseFilesDialog_ano::ChooseFilesDialog_ano(QWidget *parent):QDialog(parent)
{
	_pLineEdit_filepath_1=new QLineEdit(QObject::tr("*.marker,*.swc,*.apo"));
	_pLineEdit_filepath_2=new QLineEdit(QObject::tr("*.ano"));
	_pLineEdit_filepath_3=new QLineEdit(QObject::tr(""));
	_pLineEdit_filepath_1->setFixedWidth(400);
	_pLineEdit_filepath_2->setFixedWidth(400);
	_pLineEdit_filepath_3->setFixedWidth(400);

	QPushButton *button1=new QPushButton(QObject::tr("..."));
	QPushButton *button2=new QPushButton(QObject::tr("..."));
	QPushButton *button3=new QPushButton(QObject::tr("..."));
	QPushButton *ok=new QPushButton("Run batch matching");	ok->setDefault(true);
	QPushButton *cancel=new QPushButton("Cancel");

	m_pCheckBox_save2aposwcfile=new QCheckBox(QObject::tr("save colorized and lined matched pairs to swc and apo files"));
	m_pCheckBox_saveinvpB2A2aposwcfile=new QCheckBox(QObject::tr("save (2->1) inverse projected point set to apo and swc file"));
	m_pCheckBox_save2aposwcfile->setChecked(true);

	connect(button1,SIGNAL(clicked()),this,SLOT(_slots_openFileDialog_1()));
	connect(button2,SIGNAL(clicked()),this,SLOT(_slots_openFileDialog_2()));
	connect(button3,SIGNAL(clicked()),this,SLOT(_slots_openFileDialog_3()));
	connect(ok,	    SIGNAL(clicked()),this,SLOT(accept()));
	connect(cancel, SIGNAL(clicked()),this,SLOT(reject()));

	QGroupBox *fileChooseGroup_1=new QGroupBox(parent);
	QGroupBox *fileChooseGroup_2=new QGroupBox(parent);
	QGroupBox *fileChooseGroup_3=new QGroupBox(parent);
	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	fileChooseGroup_1->setTitle(QObject::tr("Choose target file:"));
	fileChooseGroup_2->setTitle(QObject::tr("Choose subject ano file:"));
	fileChooseGroup_3->setTitle(QObject::tr("Choose output directory:"));
	saveOptionGroup->setTitle(QObject::tr("Save option"));

	QHBoxLayout *fileChooseLayout_1=new QHBoxLayout(fileChooseGroup_1);
	fileChooseLayout_1->addWidget(_pLineEdit_filepath_1);
	fileChooseLayout_1->addWidget(button1);
	QHBoxLayout *fileChooseLayout_2=new QHBoxLayout(fileChooseGroup_2);
	fileChooseLayout_2->addWidget(_pLineEdit_filepath_2);
	fileChooseLayout_2->addWidget(button2);
	QHBoxLayout *fileChooseLayout_3=new QHBoxLayout(fileChooseGroup_3);
	fileChooseLayout_3->addWidget(_pLineEdit_filepath_3);
	fileChooseLayout_3->addWidget(button3);
	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(m_pCheckBox_save2aposwcfile);
	saveOptionLayout->addWidget(m_pCheckBox_saveinvpB2A2aposwcfile);

	QHBoxLayout *okcancelLayout=new QHBoxLayout;
	okcancelLayout->addWidget(ok);
	okcancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(fileChooseGroup_1);
	mainLayout->addWidget(fileChooseGroup_2);
	mainLayout->addWidget(fileChooseGroup_3);
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addLayout(okcancelLayout);
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Choose input files and output dir for batch-matching"));
	setLayout(mainLayout);
}
void ChooseFilesDialog_ano::_slots_openFileDialog_1()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Open target file (*.marker,*.swc,*.apo)"));
	d.setNameFilter(".marker,.swc,.apo file (*.marker *.swc *.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		_pLineEdit_filepath_1->setText(selectedFile);
	}
}
void ChooseFilesDialog_ano::_slots_openFileDialog_2()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Open subject ano file (*.ano)"));
	d.setNameFilter("ANO file (*.ano)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		_pLineEdit_filepath_2->setText(selectedFile);
	}
}
void ChooseFilesDialog_ano::_slots_openFileDialog_3()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose output directory"));
	d.setFileMode(QFileDialog::Directory);
	if(d.exec())
	{
		QString selectedDir=(d.selectedFiles())[0];
		_pLineEdit_filepath_3->setText(selectedDir);
	}
}
QString ChooseFilesDialog_ano::m_getFilename_1()
{
	return _pLineEdit_filepath_1->text();
}
QString ChooseFilesDialog_ano::m_getFilename_2()
{
	return _pLineEdit_filepath_2->text();
}
QString ChooseFilesDialog_ano::m_getOutputDir()
{
	return _pLineEdit_filepath_3->text();
}


//------------------------------------------------------------------------------------------------------------------------------------
ParaSettingDialog::ParaSettingDialog(QWidget *parent):QDialog(parent)
{
	//------------------------------------------------------------------
	//initial matching basic group
	m_pRadioButton_euclidian=new QRadioButton(tr("Euclidian_distance based"));								m_pRadioButton_euclidian->setChecked(true);
	m_pRadioButton_geodesic =new QRadioButton(tr("Geodesic_distance based"));

	QGroupBox *pGroupBox_initialmatch=new QGroupBox(tr("Initial_matching related:"));
	QVBoxLayout *pLayout_initialmatch=new QVBoxLayout;
	pLayout_initialmatch->addWidget(m_pRadioButton_euclidian);
	pLayout_initialmatch->addWidget(m_pRadioButton_geodesic);
	pGroupBox_initialmatch->setLayout(pLayout_initialmatch);

	connect(m_pRadioButton_geodesic,SIGNAL(toggled(bool)),this,SLOT(_slots_updataDialog()));

	//------------------------------------------------------------------
	//refine matching basic group
	m_pRadioButton_affine=new QRadioButton(tr("Affine based"));												m_pRadioButton_affine->setChecked(true);
	m_pRadioButton_manifold=new QRadioButton(tr("Manifold based"));

	m_pGroupBox_refinematch=new QGroupBox(tr("Refine_matching related:"));									m_pGroupBox_refinematch->setCheckable(true);
	QVBoxLayout *pLayout_refinematch=new QVBoxLayout;
	pLayout_refinematch->addWidget(m_pRadioButton_affine);
	pLayout_refinematch->addWidget(m_pRadioButton_manifold);
	m_pGroupBox_refinematch->setLayout(pLayout_refinematch);

	connect(m_pGroupBox_refinematch,SIGNAL(toggled(bool)),this,SLOT(_slots_updataDialog()));
	connect(m_pRadioButton_affine,SIGNAL(toggled(bool)),this,SLOT(_slots_updataDialog()));

	//------------------------------------------------------------------
	//advanced para setting groups
	//inital_matching adv group
	m_pLineEdit_NN_weight=new QLineEdit(tr("1"));															m_pLineEdit_NN_weight->setFixedWidth(50);
	m_pLineEdit_linear_weight=new QLineEdit(tr("1"));														m_pLineEdit_linear_weight->setFixedWidth(50);
	m_pLineEdit_histogram_weight=new QLineEdit(tr("1"));													m_pLineEdit_histogram_weight->setFixedWidth(50);
	QGroupBox *pGroupBox_voterweight_adv=new QGroupBox(tr("Shape context measure weight:"));
	QFormLayout *pFormLayout_voterweight_adv=new QFormLayout;												pFormLayout_voterweight_adv->setSpacing(1);
	pFormLayout_voterweight_adv->addRow(tr("NN voter weight:"),m_pLineEdit_NN_weight);
	pFormLayout_voterweight_adv->addRow(tr("Linear voter weight:"),m_pLineEdit_linear_weight);
	pFormLayout_voterweight_adv->addRow(tr("Histogram voter weight:"),m_pLineEdit_histogram_weight);
	pGroupBox_voterweight_adv->setLayout(pFormLayout_voterweight_adv);

	m_pLineEdit_bin=new QLineEdit(tr("10"));																m_pLineEdit_bin->setFixedWidth(50);
	m_pLineEdit_K=new QLineEdit(tr("14"));																	m_pLineEdit_K->setFixedWidth(50);
	m_pLineEdit_K->setEnabled(false);
	QFormLayout *pFormLayout_initalothers=new QFormLayout;													pFormLayout_initalothers->setSpacing(1);
	pFormLayout_initalothers->addRow(tr("Histogram bin number:"),m_pLineEdit_bin);
	pFormLayout_initalothers->addRow(tr("Neighbor num 4 Geodis computing:"),m_pLineEdit_K);

	QVBoxLayout *pLayout_initial_adv=new QVBoxLayout;
	pLayout_initial_adv->addWidget(pGroupBox_voterweight_adv);
	pLayout_initial_adv->addStretch();
	pLayout_initial_adv->addLayout(pFormLayout_initalothers);

	_pGroupBox_initial_adv=new QGroupBox(tr("Initial_matching advanced para setting:"));
	_pGroupBox_initial_adv->setVisible(false);
	_pGroupBox_initial_adv->setLayout(pLayout_initial_adv);

	//affine based refine_matching adv group
	m_pLineEdit_affineinvp_weight=new QLineEdit(tr("10"));													m_pLineEdit_affineinvp_weight->setFixedWidth(50);
	m_pLineEdit_nransacsampling=new QLineEdit(tr("2000"));													m_pLineEdit_nransacsampling->setFixedWidth(50);
	QFormLayout *pFormLayout_affine_adv=new QFormLayout;													pFormLayout_affine_adv->setSpacing(1);
	pFormLayout_affine_adv->addRow(tr("Affine invp vote weight:"),m_pLineEdit_affineinvp_weight);
	pFormLayout_affine_adv->addRow(tr("RANSAC sampling times:"),m_pLineEdit_nransacsampling);
	_pGroupBox_affine_adv=new QGroupBox(tr("Affine based refine_matching advanced para setting:"));
	_pGroupBox_affine_adv->setVisible(false);
	_pGroupBox_affine_adv->setLayout(pFormLayout_affine_adv);

	//manifold constrainter choose group
	m_pCheckBox_dir=new QCheckBox(tr("direction of matched pair"));											m_pCheckBox_dir->setChecked(true);
	m_pCheckBox_dis=new QCheckBox(tr("distance of matched pair"));											m_pCheckBox_dis->setChecked(true);
	m_pCheckBox_topcandidate=new QCheckBox(tr("limited in top K candidates"));								m_pCheckBox_topcandidate->setChecked(true);
	QGroupBox *pGroupBox_constraintchoose=new QGroupBox(tr("Choose manifold constrainter:"));
	QVBoxLayout *pFormLayout_constraintchoose=new QVBoxLayout;												pFormLayout_constraintchoose->setSpacing(1);
	pFormLayout_constraintchoose->addWidget(m_pCheckBox_dir);
	pFormLayout_constraintchoose->addWidget(m_pCheckBox_dis);
	pFormLayout_constraintchoose->addWidget(m_pCheckBox_topcandidate);
	pGroupBox_constraintchoose->setLayout(pFormLayout_constraintchoose);

	m_pLineEdit_maxiternum=new QLineEdit(tr("50"));															m_pLineEdit_maxiternum->setFixedWidth(50);
	m_pLineEdit_neighbornum4dirdisavg=new QLineEdit(tr("15"));												m_pLineEdit_neighbornum4dirdisavg->setFixedWidth(50);
	m_pLineEdit_topcandidatenum=new QLineEdit(tr("20"));														m_pLineEdit_topcandidatenum->setFixedWidth(50);
	QFormLayout *pLayout_manifoldothers=new QFormLayout;													pLayout_manifoldothers->setSpacing(1);
	pLayout_manifoldothers->addRow(tr("Maximal optimise iter num:"),m_pLineEdit_maxiternum);
	pLayout_manifoldothers->addRow(tr("Neighbor num 4 Avgdirdis computing:"),m_pLineEdit_neighbornum4dirdisavg);
	pLayout_manifoldothers->addRow(tr("Top voted candidates num:"),m_pLineEdit_topcandidatenum);

	QVBoxLayout *pLayout_manifold_adv=new QVBoxLayout;
	pLayout_manifold_adv->addWidget(pGroupBox_constraintchoose);
	pLayout_manifold_adv->addLayout(pLayout_manifoldothers);

	//manifold based refine_matching adv group
	_pGroupBox_manifold_adv=new QGroupBox(tr("Manifold based refine_matching advanced para setting:"));
	_pGroupBox_manifold_adv->setDisabled(true);
	_pGroupBox_manifold_adv->setVisible(false);
	_pGroupBox_manifold_adv->setLayout(pLayout_manifold_adv);

	//------------------------------------------------------------------
	//main window
	QPushButton *pPushButton_ok=new QPushButton(tr("OK"));
	QPushButton *pPushButton_cancel=new QPushButton(tr("Cancel"));
	QPushButton *pPushButton_advance=new QPushButton(tr("Advanced"));
	connect(pPushButton_ok,	   SIGNAL(clicked()),this,SLOT(accept()));
	connect(pPushButton_cancel,SIGNAL(clicked()),this,SLOT(reject()));
	connect(pPushButton_advance,SIGNAL(clicked()),this,SLOT(_slots_toggleAdvButton()));

	QVBoxLayout *pLayout_okcancel=new QVBoxLayout;
	pLayout_okcancel->addWidget(pPushButton_ok);
	pLayout_okcancel->addWidget(pPushButton_cancel);
	pLayout_okcancel->addStretch();
	pLayout_okcancel->addWidget(pPushButton_advance);

	QGridLayout *pLayout_main=new QGridLayout;
	pLayout_main->setSizeConstraint(QLayout::SetFixedSize);
	pLayout_main->addWidget(pGroupBox_initialmatch,1,1);
	pLayout_main->addWidget(m_pGroupBox_refinematch,1,2);
	pLayout_main->addLayout(pLayout_okcancel,1,3);
	pLayout_main->addWidget(_pGroupBox_initial_adv,2,1);
	pLayout_main->addWidget(_pGroupBox_affine_adv,3,1);
	pLayout_main->addWidget(_pGroupBox_manifold_adv,2,2);

	setLayout(pLayout_main);
	setWindowTitle(QObject::tr("Choose matching parameters"));
}

void ParaSettingDialog::_slots_updataDialog()
{
	m_pLineEdit_K->setEnabled(m_pRadioButton_geodesic->isChecked());

	if(m_pGroupBox_refinematch->isChecked())
	{
		if(m_pRadioButton_manifold->isChecked())
		{
			_pGroupBox_affine_adv->setEnabled(false);
			_pGroupBox_manifold_adv->setEnabled(true);
		}
		else
		{
			_pGroupBox_affine_adv->setEnabled(true);
			_pGroupBox_manifold_adv->setEnabled(false);
		}
	}
	else
	{
		_pGroupBox_affine_adv->setEnabled(false);
		_pGroupBox_manifold_adv->setEnabled(false);
	}
}

void ParaSettingDialog::_slots_toggleAdvButton()
{
	if(_pGroupBox_initial_adv->isVisible())
	{
		_pGroupBox_initial_adv->setVisible(false);
		_pGroupBox_affine_adv->setVisible(false);
		_pGroupBox_manifold_adv->setVisible(false);
	}
	else
	{
		_pGroupBox_initial_adv->setVisible(true);
		_pGroupBox_affine_adv->setVisible(true);
		_pGroupBox_manifold_adv->setVisible(true);
	}
}
