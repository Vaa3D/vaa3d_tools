// q_pointcloud_match_dialog.h
// provide the dialogs class definition
//by Lei Qu
//2009-11-10
// seperated from plugin_pointcloud_match.h by Lei Qu on 2010-01-23

#ifndef __Q_POINTCLOUD_MATCH_DIALOGS_H__
#define __Q_POINTCLOUD_MATCH_DIALOGS_H__

#include <v3d_interface.h>
#include <QDialog>

class QRadioButton;
class QLineEdit;
class QCheckBox;
class QGroupBox;


//------------------------------------------------------------------------------------------------------------------------------------
class Choose2FileDialog : public QDialog
{
	Q_OBJECT

public:
	Choose2FileDialog(QWidget *parent);
	QString m_getFilename_1();
	QString m_getFilename_2();

private slots:
	void _slots_openFileDlg_1();
	void _slots_openFileDlg_2();

private:
	QLineEdit *_pLineEdit_filepath_1;
	QLineEdit *_pLineEdit_filepath_2;
};


//------------------------------------------------------------------------------------------------------------------------------------
class ChooseFilesDialog_ano : public QDialog
{
	Q_OBJECT

public:
	ChooseFilesDialog_ano(QWidget *parent);
	QString m_getFilename_1();
	QString m_getFilename_2();
	QString m_getOutputDir();

	QCheckBox *m_pCheckBox_save2aposwcfile,*m_pCheckBox_saveinvpB2A2aposwcfile;

private slots:
	void _slots_openFileDialog_1();
	void _slots_openFileDialog_2();
	void _slots_openFileDialog_3();

private:
	QLineEdit *_pLineEdit_filepath_1;
	QLineEdit *_pLineEdit_filepath_2;
	QLineEdit *_pLineEdit_filepath_3;
};


//------------------------------------------------------------------------------------------------------------------------------------
class ParaSettingDialog : public QDialog
{
	Q_OBJECT

public:
	ParaSettingDialog(QWidget *parent);

private slots:
	void _slots_updataDialog();
	void _slots_toggleAdvButton();

public:
	//initial basic
	QRadioButton	*m_pRadioButton_euclidian;
	QRadioButton	*m_pRadioButton_geodesic;
	//initial adv
	QLineEdit 		*m_pLineEdit_NN_weight;
	QLineEdit 		*m_pLineEdit_linear_weight;
	QLineEdit 		*m_pLineEdit_histogram_weight;
	QLineEdit 		*m_pLineEdit_bin;
	QLineEdit 		*m_pLineEdit_K;

	//refine basic
	QGroupBox 		*m_pGroupBox_refinematch;
	QRadioButton 	*m_pRadioButton_affine;
	QRadioButton 	*m_pRadioButton_manifold;

	//refine adv - affine
	QLineEdit 		*m_pLineEdit_affineinvp_weight;
	QLineEdit 		*m_pLineEdit_nransacsampling;

	//refine adv - manifold
	QCheckBox 		*m_pCheckBox_dir;
	QCheckBox 		*m_pCheckBox_dis;
	QCheckBox 		*m_pCheckBox_topcandidate;
	QLineEdit 		*m_pLineEdit_maxiternum;
	QLineEdit 		*m_pLineEdit_neighbornum4dirdisavg;
	QLineEdit 		*m_pLineEdit_topcandidatenum;

private:
	QGroupBox 		*_pGroupBox_initial_adv;
	QGroupBox 		*_pGroupBox_affine_adv;
	QGroupBox 		*_pGroupBox_manifold_adv;

};


#endif
