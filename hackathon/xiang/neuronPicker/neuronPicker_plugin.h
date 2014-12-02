/* neuronPicker_plugin.h
 * 
 * 2014-12-01 : by Xiang Li, Hanbo Chen
 */
 
#ifndef __NEURONPICKER_PLUGIN_H__
#define __NEURONPICKER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#pragma region "dialogRun" 
class dialogRun:public QDialog
{
	Q_OBJECT
public:
	dialogRun(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
	{
		//region definition;
		QGroupBox *QGroupBox_region_main = new QGroupBox("Region definition parameters");
		QGridLayout *QGridLayout_region_main = new QGridLayout();
		QLabel* QLabel_region_colorBandwidth = new QLabel(QObject::tr("Color filtering bandwidth:"));
		QLineEdit_region_colorBandwidth = new QLineEdit("100", QWidget_parent);
		QLabel* QLabel_region_gapSize = new QLabel(QObject::tr("Max gap size:"));
		QLineEdit_region_gapSize = new QLineEdit("10", QWidget_parent);
		QGridLayout_region_main->addWidget(QLabel_region_colorBandwidth, 1, 1, 1, 1);
		QGridLayout_region_main->addWidget(QLineEdit_region_colorBandwidth, 1, 2, 1, 1);
		QGridLayout_region_main->addWidget(QLabel_region_gapSize, 2, 1, 1, 1);
		QGridLayout_region_main->addWidget(QLineEdit_region_gapSize, 2, 2, 1, 1);
		QGroupBox_region_main->setLayout(QGridLayout_region_main);
		//control;
		QPushButton *QPushButton_control_start = new QPushButton(QObject::tr("Run"));
		QPushButton *QPushButton_control_close = new QPushButton(QObject::tr("Close"));
		QWidget* QWidget_control_bar = new QWidget();
		QGridLayout* QGridLayout_control_bar = new QGridLayout();
		QGridLayout_control_bar->addWidget(QPushButton_control_start,1,1,1,1);
		QGridLayout_control_bar->addWidget(QPushButton_control_close,1,2,1,1);
		QWidget_control_bar->setLayout(QGridLayout_control_bar);
		//main panel;
		QGridLayout *QGridLayout_main = new QGridLayout();
		QGridLayout_main->addWidget(QGroupBox_region_main);
		QGridLayout_main->addWidget(QWidget_control_bar);
		setLayout(QGridLayout_main);
		setWindowTitle(QString("neuronPicker"));
		//event evoking;
		connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
		connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
	}
	~dialogRun(){}
	QLineEdit* QLineEdit_region_colorBandwidth;
	QLineEdit* QLineEdit_region_gapSize;
	double bandWidth_color;
	V3DLONG size_gap;
	public slots:
		void _slot_start()
		{
			bandWidth_color=QLineEdit_region_colorBandwidth->text().toDouble();
			size_gap=QLineEdit_region_gapSize->text().toUInt();
			accept();
		}
};
#pragma endregion

class neuronPicker : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	bool interface_run(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent);

	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

