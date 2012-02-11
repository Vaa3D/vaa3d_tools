//q_warp_affine_tps_dialogs.h
//by Lei Qu
//2010-03-22

#include <QtGui>
#include "q_warp_affine_tps_dialogs.h"


//------------------------------------------------------------------------------------------------------------------------------------
WarpAffineTpsDialog_pts::WarpAffineTpsDialog_pts(QWidget *parent):QDialog(parent)
{
	m_pLineEdit_filepath_tar_ctl=new QLineEdit(QObject::tr("choose target  control .marker file here"));
	m_pLineEdit_filepath_sub_ctl=new QLineEdit(QObject::tr("choose subject control .marker file here"));
	m_pLineEdit_filepath_sub_warp=new QLineEdit(QObject::tr("choose to be warped pointset file here (.swc .marker)"));
	m_pLineEdit_filepath_matchind=new QLineEdit(QObject::tr("choose matched-pairs index file here [optional]"));
	m_pLineEdit_filepath_tar_ctl->setFixedWidth(400);
	m_pLineEdit_filepath_sub_ctl->setFixedWidth(400);
	m_pLineEdit_filepath_sub_warp->setFixedWidth(400);
	m_pLineEdit_filepath_matchind->setFixedWidth(400);
	m_pLineEdit_filepath_sub2tar_affine=new QLineEdit(QObject::tr("assign affine warped output file here [optional]"));
	m_pLineEdit_filepath_sub2tar_affine_tps=new QLineEdit(QObject::tr("assign tps warped output file here [optional]"));
	m_pLineEdit_filepath_sub2tar_affine->setFixedWidth(400);
	m_pLineEdit_filepath_sub2tar_affine_tps->setFixedWidth(400);

	QPushButton *pPushButton_openFileDialog_tar_ctl				=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub_ctl				=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub_warp			=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_matchind			=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub2tar_affine		=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub2tar_affine_tps	=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_ok									=new QPushButton("OK");						pPushButton_ok->setDefault(true);
	QPushButton *pPushButton_cancel								=new QPushButton("Cancel");
	QPushButton *pPushButton_help								=new QPushButton("Help");

	m_pCheckBox_scaleradius=new QCheckBox(QObject::tr("scale swc nodes and markers radius accordingly"));	m_pCheckBox_scaleradius->setChecked(true);

	connect(pPushButton_openFileDialog_tar_ctl,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_tar_ctl()));
	connect(pPushButton_openFileDialog_sub_ctl,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub_ctl()));
	connect(pPushButton_openFileDialog_sub_warp,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub_warp()));
	connect(pPushButton_openFileDialog_matchind,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_matchind()));
	connect(pPushButton_openFileDialog_sub2tar_affine,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub2tar_affine()));
	connect(pPushButton_openFileDialog_sub2tar_affine_tps,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub2tar_affine_tps()));
	connect(pPushButton_ok,	    		SIGNAL(clicked()),this,SLOT(accept()));
	connect(pPushButton_cancel, 		SIGNAL(clicked()),this,SLOT(reject()));
	connect(pPushButton_help, 		 	SIGNAL(clicked()),this,SLOT(_slots_openHelpDlg()));

	QGroupBox *pGroupBox_input=new QGroupBox(parent);
	QGroupBox *pGroupBox_output=new QGroupBox(parent);
	pGroupBox_input->setTitle(QObject::tr("Input filenames:"));
	pGroupBox_output->setTitle(QObject::tr("Output filenames:"));

	QGridLayout *pLayout_input=new QGridLayout();
	pLayout_input->addWidget(m_pLineEdit_filepath_tar_ctl,1,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_tar_ctl,1,2);
	pLayout_input->addWidget(m_pLineEdit_filepath_sub_ctl,2,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_sub_ctl,2,2);
	pLayout_input->addWidget(m_pLineEdit_filepath_sub_warp,3,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_sub_warp,3,2);
	pLayout_input->addWidget(m_pLineEdit_filepath_matchind,4,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_matchind,4,2);
	pLayout_input->addWidget(m_pCheckBox_scaleradius,5,1);
	pGroupBox_input->setLayout(pLayout_input);

	QGridLayout *pLayout_output=new QGridLayout();
	pLayout_output->addWidget(m_pLineEdit_filepath_sub2tar_affine,1,1);
	pLayout_output->addWidget(pPushButton_openFileDialog_sub2tar_affine,1,2);
	pLayout_output->addWidget(m_pLineEdit_filepath_sub2tar_affine_tps,2,1);
	pLayout_output->addWidget(pPushButton_openFileDialog_sub2tar_affine_tps,2,2);
	pGroupBox_output->setLayout(pLayout_output);

	QHBoxLayout *pLayout_okcancel=new QHBoxLayout;
	pLayout_okcancel->addWidget(pPushButton_ok);
	pLayout_okcancel->addWidget(pPushButton_cancel);
	pLayout_okcancel->addWidget(pPushButton_help);

	QVBoxLayout *pLayout_main=new QVBoxLayout;
	pLayout_main->addWidget(pGroupBox_input);
	pLayout_main->addWidget(pGroupBox_output);
	pLayout_main->addLayout(pLayout_okcancel);
	pLayout_main->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Assign parameters and input/output filenames"));
	setLayout(pLayout_main);
}

void WarpAffineTpsDialog_pts::_slots_openFileDlg_tar_ctl()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose target control marker file"));
	d.setNameFilter("marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_tar_ctl->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openFileDlg_sub_ctl()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose subject control marker file"));
	d.setNameFilter("marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub_ctl->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openFileDlg_sub_warp()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose to be warped subject file"));
	d.setNameFilter("pointset file (*.marker *.swc *.apo)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub_warp->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openFileDlg_matchind()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose matched-pairs index file"));
	d.setNameFilter("match index file (*.match)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_matchind->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openFileDlg_sub2tar_affine()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose affine warped output filename"));
//	d.setNameFilter("image stack file (*.tif *.lsm *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub2tar_affine->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openFileDlg_sub2tar_affine_tps()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose affine+tps warped output filename (*.swc)"));
//	d.setNameFilter("apo file (*.tif *.lsm *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub2tar_affine_tps->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_pts::_slots_openHelpDlg()
{
	QString qs_helptext="This plugin can warp given pointset(.marker or .swc) by best aligning the given control points.<br>";
	qs_helptext.append("<br>");
	qs_helptext.append("There are two \"warp\" methods: Affine transform and Thin-Plate-Spline (TPS) warp:<br>");
	qs_helptext.append("(1). Affine transform tries to align the control points using least square fitting. It can not gaurantee all the subject control points be exactly mapped to the position of target. This method is usually used for global alignment.<br>");
	qs_helptext.append("(2). TPS warp can guarantee all the subject control points are exactly mapped to the corresponding target positions. In order to minimize the distortion, an affine transform is often used as the pre-processor.<br>");
	qs_helptext.append("<br>");
	qs_helptext.append("Input filenames (from up to down):<br>");
	qs_helptext.append("(1). the marker file that contains the target control points in different rows.<br>");
	qs_helptext.append("(2). the marker file that contains the subject control points in different rows.<br>");
	qs_helptext.append("(3). the pointset file that contains the subject points to be warped.<br>");
	qs_helptext.append("(4). the point-matching file that indicates the matching relationship between the subject pointset and target pointset. If user dont assign this file, we assume the points in subject and target file are well ordered, i.e. the i-th point (row) in the subject file corresponds to the i-th point (row) in the target file.<br>");
	qs_helptext.append("-----the format of point-matching file consists of N rows, each is a correspondence. In each row there are two numbers seperated by ':'. The first number indicates the index of subject point and the second one indicates the corresponding target point index.<br>");
	qs_helptext.append("<br>");
	qs_helptext.append("Output filenames (from up to down):<br>");
	qs_helptext.append("(1). the output file name that contains only affine transformed pointset. <br>");
	qs_helptext.append("(2). the output file name that contains affine+TPS warped pointset. If a user don't specify this file, only the affine transform is used. <br>");
	qs_helptext.append("Note: the output file format(suffix) should be same as input one(to be warped). <br>");

	QTextEdit *matchResultText=new QTextEdit(qs_helptext);
	matchResultText->setReadOnly(true);
	matchResultText->setFontPointSize(12);

	QPushButton *close=new QPushButton("Close");

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(matchResultText);
	mainLayout->addWidget(close);


	QDialog help_DLG(this);
	help_DLG.setWindowTitle(QObject::tr("Help"));
	help_DLG.setLayout(mainLayout);
	help_DLG.resize(500,500);
	help_DLG.connect(close,  SIGNAL(clicked()),&help_DLG,SLOT(accept()));

	help_DLG.exec();

}

//------------------------------------------------------------------------------------------------------------------------------------
WarpAffineTpsDialog_img::WarpAffineTpsDialog_img(QWidget *parent):QDialog(parent)
{
	m_pLineEdit_filepath_tar_ctl=new QLineEdit(QObject::tr("choose target  control .marker file here"));
	m_pLineEdit_filepath_sub_ctl=new QLineEdit(QObject::tr("choose subject control .marker file here"));
	m_pLineEdit_filepath_sub_warp=new QLineEdit(QObject::tr("choose to be warped image here (.raw .tif)"));
	m_pLineEdit_filepath_tar_ctl->setFixedWidth(400);
	m_pLineEdit_filepath_sub_ctl->setFixedWidth(400);
	m_pLineEdit_filepath_sub_warp->setFixedWidth(400);
	m_pLineEdit_filepath_sub2tar_affine=new QLineEdit(QObject::tr("assign affine warped output image here [optional]"));
	m_pLineEdit_filepath_sub2tar_tps=new QLineEdit(QObject::tr("assign affine+tps warped output image here [optional]"));
	m_pLineEdit_filepath_sub2tar_affine->setFixedWidth(400);
	m_pLineEdit_filepath_sub2tar_tps->setFixedWidth(400);

	QPushButton *pPushButton_openFileDialog_tar_ctl				=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub_ctl				=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub_warp			=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub2tar_affine		=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_openFileDialog_sub2tar_affine_tps	=new QPushButton(QObject::tr("..."));
	QPushButton *pPushButton_ok									=new QPushButton("OK");						pPushButton_ok->setDefault(true);
	QPushButton *pPushButton_cancel								=new QPushButton("Cancel");

	connect(pPushButton_openFileDialog_tar_ctl,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_tar_ctl()));
	connect(pPushButton_openFileDialog_sub_ctl,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub_ctl()));
	connect(pPushButton_openFileDialog_sub_warp,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub_warp()));
	connect(pPushButton_openFileDialog_sub2tar_affine,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub2tar_affine()));
	connect(pPushButton_openFileDialog_sub2tar_affine_tps,SIGNAL(clicked()),this,SLOT(_slots_openFileDlg_sub2tar_affine_tps()));
	connect(pPushButton_ok,	    		SIGNAL(clicked()),this,SLOT(accept()));
	connect(pPushButton_cancel, 		SIGNAL(clicked()),this,SLOT(reject()));

	QGroupBox *pGroupBox_input=new QGroupBox(parent);
	QGroupBox *pGroupBox_output=new QGroupBox(parent);
	pGroupBox_input->setTitle(QObject::tr("Input filenames:"));
	pGroupBox_output->setTitle(QObject::tr("Output filenames:"));

	QGridLayout *pLayout_input=new QGridLayout();
	pLayout_input->addWidget(m_pLineEdit_filepath_tar_ctl,1,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_tar_ctl,1,2);
	pLayout_input->addWidget(m_pLineEdit_filepath_sub_ctl,2,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_sub_ctl,2,2);
	pLayout_input->addWidget(m_pLineEdit_filepath_sub_warp,3,1);
	pLayout_input->addWidget(pPushButton_openFileDialog_sub_warp,3,2);
	pGroupBox_input->setLayout(pLayout_input);

	QGridLayout *pLayout_output=new QGridLayout();
	pLayout_output->addWidget(m_pLineEdit_filepath_sub2tar_affine,1,1);
	pLayout_output->addWidget(pPushButton_openFileDialog_sub2tar_affine,1,2);
	pLayout_output->addWidget(m_pLineEdit_filepath_sub2tar_tps,2,1);
	pLayout_output->addWidget(pPushButton_openFileDialog_sub2tar_affine_tps,2,2);
	pGroupBox_output->setLayout(pLayout_output);

	QHBoxLayout *pLayout_okcancel=new QHBoxLayout;
	pLayout_okcancel->addWidget(pPushButton_ok);
	pLayout_okcancel->addWidget(pPushButton_cancel);

	QVBoxLayout *pLayout_main=new QVBoxLayout;
	pLayout_main->addWidget(pGroupBox_input);
	pLayout_main->addWidget(pGroupBox_output);
	pLayout_main->addLayout(pLayout_okcancel);
	pLayout_main->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Assign parameters and input/output filenames"));
	setLayout(pLayout_main);
}

void WarpAffineTpsDialog_img::_slots_openFileDlg_tar_ctl()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose target control marker file"));
	d.setNameFilter("marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_tar_ctl->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_img::_slots_openFileDlg_sub_ctl()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose subject control marker file"));
	d.setNameFilter("marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub_ctl->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_img::_slots_openFileDlg_sub_warp()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Choose to be warped subject image"));
	d.setNameFilter("image file (*.raw *.tif)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub_warp->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_img::_slots_openFileDlg_sub2tar_affine()
{
	QFileDialog d(this);
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setWindowTitle(tr("Choose affine warped output filename"));
//	d.setNameFilter("image stack file (*.tif *.lsm *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub2tar_affine->setText(selectedFile);
	}
}
void WarpAffineTpsDialog_img::_slots_openFileDlg_sub2tar_affine_tps()
{
	QFileDialog d(this);
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setWindowTitle(tr("Choose affine+tps warped output filename (*.swc)"));
//	d.setNameFilter("apo file (*.tif *.lsm *.raw)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		m_pLineEdit_filepath_sub2tar_tps->setText(selectedFile);
	}
}
