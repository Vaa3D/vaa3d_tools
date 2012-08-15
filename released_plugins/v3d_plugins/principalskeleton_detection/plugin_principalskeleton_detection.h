//principal skeleton detection plugin
//by Lei Qu
//2009-12-29

#ifndef __PLUGIN_PRINCIPALSKELETON_DETECTION_H__
#define __PLUGIN_PRINCIPALSKELETON_DETECTION_H__

#include <v3d_interface.h>
#include <QDialog>

class QLabel;
class QLineEdit;
class QCheckBox;

//------------------------------------------------------------------------------------------------------------------------------------
class PrincipalSkeletonDetectionPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

//------------------------------------------------------------------------------------------------------------------------------------
//paradialog for principal skeleton detection
class ParaDialog_PSDetection : public QDialog
{
	Q_OBJECT

public:
	ParaDialog_PSDetection(QWidget *parent);
	QString getFilename_mak_ini();
	QString getFilename_domain();

private slots:
	void openFileDialog_mak_ini();
	void openFileDialog_domain();

public:
	QLineEdit *filePathLineEdit_mak_ini;
	QLineEdit *filePathLineEdit_domain;

	QLineEdit *refChannelLineEdit;
	QLineEdit *stopThreshLineEdit;
	QLineEdit *maxIterLineEdit;
	QLineEdit *foregroundRatioLineEdit;
	QLineEdit *mophologyRadiusLineEdit;
	QLineEdit *iniskezoomfactorLineEdit;
	QLineEdit *baseimageMethodLineEdit;
};

//------------------------------------------------------------------------------------------------------------------------------------
//paradialog for principal skeleton based image warping
class ParaDialog_PSWarping : public QDialog
{
	Q_OBJECT

public:
	ParaDialog_PSWarping(QWidget *parent);

private slots:
	void openFileDialog_img_sub();
	void openFileDialog_img_tar();
	void openFileDialog_mak_sub();
	void openFileDialog_mak_tar();
	void openFileDialog_domain();

public:
	QLineEdit *filePathLineEdit_img_sub;
	QLineEdit *filePathLineEdit_img_tar;
	QLineEdit *filePathLineEdit_mak_sub;
	QLineEdit *filePathLineEdit_mak_tar;
	QLineEdit *filePathLineEdit_domain;
};

#endif
