//q_warp_affine_tps_dialogs.h
//by Lei Qu
//2010-03-22

#ifndef __Q_WARP_AFFINE_TPS_DIALOGS_H__
#define __Q_WARP_AFFINE_TPS_DIALOGS_H__

#include <v3d_interface.h>
#include <QDialog>

class QRadioButton;
class QLineEdit;
class QCheckBox;
class QGroupBox;


class WarpAffineTpsDialog_pts : public QDialog
{
	Q_OBJECT

public:
	WarpAffineTpsDialog_pts(QWidget *parent);

private slots:
	void _slots_openFileDlg_tar_ctl();
	void _slots_openFileDlg_sub_ctl();
	void _slots_openFileDlg_sub_warp();
	void _slots_openFileDlg_matchind();
	void _slots_openFileDlg_sub2tar_affine();
	void _slots_openFileDlg_sub2tar_affine_tps();
	void _slots_openHelpDlg();

public:
	//input
	QLineEdit *m_pLineEdit_filepath_tar_ctl;
	QLineEdit *m_pLineEdit_filepath_sub_ctl;
	QLineEdit *m_pLineEdit_filepath_sub_warp;
	QLineEdit *m_pLineEdit_filepath_matchind;
	QCheckBox *m_pCheckBox_scaleradius;
	//output
	QLineEdit *m_pLineEdit_filepath_sub2tar_affine;
	QLineEdit *m_pLineEdit_filepath_sub2tar_affine_tps;
};

class WarpAffineTpsDialog_img : public QDialog
{
	Q_OBJECT

public:
	WarpAffineTpsDialog_img(QWidget *parent);

private slots:
	void _slots_openFileDlg_tar_ctl();
	void _slots_openFileDlg_sub_ctl();
	void _slots_openFileDlg_sub_warp();
	void _slots_openFileDlg_sub2tar_affine();
	void _slots_openFileDlg_sub2tar_affine_tps();

public:
	//input
	QLineEdit *m_pLineEdit_filepath_tar_ctl;
	QLineEdit *m_pLineEdit_filepath_sub_ctl;
	QLineEdit *m_pLineEdit_filepath_sub_warp;
	//output
	QLineEdit *m_pLineEdit_filepath_sub2tar_affine;
	QLineEdit *m_pLineEdit_filepath_sub2tar_tps;
};

#endif
