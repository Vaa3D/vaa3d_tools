#ifndef RegistrationDlg_H
#define RegistrationDlg_H

#include <QtGui>
#include <QWidget>
#include <v3d_interface.h>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class RegistrationDlg:public QDialog
{
	Q_OBJECT
public:
	RegistrationDlg();
	void SetCallback(V3DPluginCallback &callback);
	QString getTransformType();
	QString getInterpolateType();
	QString getMetricType();
	QString getOptimizerType();
	void SetFinalParameters(QList<double> list);
public slots:
	void updateOptim(int i);
	void updateIntOptim(int i);
	void updateConfig();
	void Start();
	void Subtract();
private:
	QLabel *TransformLabel;
	QComboBox *TransformMethod;
	QLabel *InterpolateLabel;
	QComboBox *InterpolateMethod;
	QLabel *MetricLabel;
	QComboBox *MetricMethod;
	QLabel *OptimizerLabel;
	QComboBox *OptimizerMethod;

	QLabel *RegTypeLabel;
	QComboBox *RegType;

	QLabel *FinalParameterLabel;
	QListWidget *FinalParaList;
	QLabel *RegSelectLabel;
	QListWidget *RegSelectList;

	QPushButton *StartBtn;
	QPushButton *SubtractBtn;
	QPushButton *ExitBtn;

	QString transform;
	QString interpolator;
	QString metric;
	QString optimizer;
	QString reg_str;

	V3DPluginCallback *callback;

};

#endif
