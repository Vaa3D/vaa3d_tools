/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 
#ifndef __CELLSEGMENTATION_PLUGIN_H__
#define __CELLSEGMENTATION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <string>

enum enum_algorithm_t {regionGrowOnly, regionGrowGWDT, regionGrowGVF, regionGrowFusing};
enum enum_shape_t {sphere, cube};

class cellSegmentation : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

class dialogMain: public QDialog
{
    Q_OBJECT

	public:
    dialogMain(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent, int int_channelDim)
    {
        QPushButton *QPushButton_start = new QPushButton(QObject::tr("start segmentation"));
        QPushButton *QPushButton_close = new QPushButton(QObject::tr("close"));

        QStringList QStringList_channel_items;
        if (int_channelDim==1)
		{
			QStringList_channel_items << "1";
		}
        else if (int_channelDim==3)
        {
            QStringList_channel_items << "1 - red";
            QStringList_channel_items << "2 - green";
            QStringList_channel_items << "3 - blue";
        }
        else //for non-RGB int_channel setups;
        {
            for (int i=1; i<=int_channelDim; i++)
            {
                QStringList_channel_items << QString().setNum(i);
            }
        }
        QComboBox_channel_selection = new QComboBox();
		QComboBox_channel_selection->addItems(QStringList_channel_items);
		if (QStringList_channel_items.size()>1)
		{
			QComboBox_channel_selection->setCurrentIndex(1);
		}
        QLabel* QLabel_channel_main = new QLabel(QObject::tr("Channel: "));
        QGroupBox *QGroupBox_channel = new QGroupBox("Channel");
        QGroupBox_channel->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_channel = new QGridLayout();
        QGroupBox_channel->setStyle(new QWindowsStyle());
        QGridLayout_channel->addWidget(QLabel_channel_main, 2,1,1,1);
        QGridLayout_channel->addWidget(QComboBox_channel_selection, 2,2,1,2);
        QGroupBox_channel->setLayout(QGridLayout_channel);

		QStringList QStringList_threshold_items;
		QStringList_threshold_items << "Calculate from Image" << "User Input";
		QComboBox_threshold_type = new QComboBox();
		QComboBox_threshold_type->addItems(QStringList_threshold_items);
		QLabel* QLabel_thresholdType = new QLabel(QObject::tr("Threshold: "));
		QGroupBox *QGroupBox_threshold = new QGroupBox("Threshold");
        QGroupBox_threshold->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_threshold = new QGridLayout();
        QGridLayout_threshold->addWidget(QLabel_thresholdType, 2,1);
        QGridLayout_threshold->addWidget(QComboBox_threshold_type, 2,2);
        QDoubleSpinBox_threshold = new QDoubleSpinBox();
		QDoubleSpinBox_threshold->setEnabled(false);
		QDoubleSpinBox_threshold->setMaximum(255);
		QDoubleSpinBox_threshold->setMinimum(0);
		QGridLayout_threshold->addWidget(QDoubleSpinBox_threshold, 2,3);
		QLabel* QLabel_threshold_thresholdHistoSimilar = new QLabel(QObject::tr("Histogram similarity threshold: "));
		QLineEdit_threshold_thresholdHistoSimilar = new QLineEdit ("0.0", QWidget_parent);
		QGridLayout_threshold->addWidget(QLabel_threshold_thresholdHistoSimilar, 3, 1);
		QGridLayout_threshold->addWidget(QLineEdit_threshold_thresholdHistoSimilar, 3, 2);
        QGroupBox_threshold->setLayout(QGridLayout_threshold);

		QRadioButton_shape_sphere = new QRadioButton("sphere", QWidget_parent);
		QRadioButton_shape_sphere->setChecked(true);
		QRadioButton_shape_cube = new QRadioButton("cube", QWidget_parent);
		QRadioButton_shape_cube->setChecked(false);
		QLabel* QLabel_Shape_delta = new QLabel(QObject::tr("delta for anisotropic threshold: "));
		QLineEdit_Shape_delta = new QLineEdit("1", QWidget_parent);
		QGroupBox *QGroupBox_shape = new QGroupBox("shape");
		QGridLayout *QGridLayout_shape = new QGridLayout();
		QGridLayout_shape->addWidget(QRadioButton_shape_sphere, 1, 1);
		QGridLayout_shape->addWidget(QRadioButton_shape_cube, 1, 2);
		QGridLayout_shape->addWidget(QLabel_Shape_delta, 1, 3);
		QGridLayout_shape->addWidget(QLineEdit_Shape_delta, 1, 4);
		QGroupBox_shape->setLayout(QGridLayout_shape);

		QRadioButton_algorithm_regionGrowingOnly = new QRadioButton("regionGrowing only", QWidget_parent);
		QRadioButton_algorithm_regionGrowingOnly->setChecked(false);
		QRadioButton_algorithm_regionGrowingGWDT = new QRadioButton("GWDT+boundaryDetection", QWidget_parent);
		QRadioButton_algorithm_regionGrowingGWDT->setChecked(false);
		QRadioButton_algorithm_regionGrowingGVF = new QRadioButton("regionGrowing+GVF", QWidget_parent);
		QRadioButton_algorithm_regionGrowingGVF->setChecked(true);
		QRadioButton_algorithm_regionGrowFusing = new QRadioButton("fusing", QWidget_parent);
		QRadioButton_algorithm_regionGrowFusing->setChecked(false);
		QCheckBox_algorithm_debug = new QCheckBox("Debugging Mode", QWidget_parent);
		QCheckBox_algorithm_debug->setChecked(true);
		QGroupBox *QGroupBox_algorithm = new QGroupBox("Algorithm");
		QGroupBox_algorithm->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_algorithm = new QGridLayout();
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowingOnly, 1, 1);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowingGWDT, 1, 2);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowingGVF, 2, 1);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowFusing, 2, 2);
		QGridLayout_algorithm->addWidget(QCheckBox_algorithm_debug, 3, 1);
		QGroupBox_algorithm->setLayout(QGridLayout_algorithm);
	
		QLabel* QLabel_GVF_matIteration = new QLabel(QObject::tr("Max iteration: "));
		QLineEdit_GVF_maxIteration = new QLineEdit("15", QWidget_parent);
		QLabel* QLabel_GVF_fusionThreshold = new QLabel(QObject::tr("Fusion double_threshold: "));
		QLineEdit_GVF_fusionThreshold = new QLineEdit("2", QWidget_parent);
		QLabel* QLabel_GVF_sigma = new QLabel(QObject::tr("sigma: "));
		QLineEdit_GVF_sigma = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_GVF_mu = new QLabel(QObject::tr("mu: "));
		QLineEdit_GVF_mu = new QLineEdit("0.1", QWidget_parent);
		QGroupBox *QGroupBox_GVFparamter = new QGroupBox("GVF paramters");
		QGroupBox_GVFparamter->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_GVFparamter = new QGridLayout();
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_matIteration, 1, 1);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_maxIteration, 1, 2);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_fusionThreshold, 1, 3);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_fusionThreshold, 1, 4);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_sigma, 2, 1);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_sigma, 2, 2);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_mu, 2, 3);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_mu, 2, 4);
		QGroupBox_GVFparamter->setLayout(QGridLayout_GVFparamter);

		QLabel* QLabel_GWDT_boundaryThin = new QLabel(QObject::tr("Boundary detection iteration: "));
		QLineEdit_GWDT_boundaryThin = new QLineEdit("8", QWidget_parent);
		QLabel* QLabel_GWDT_boundaryConditionLoosenBy = new QLabel(QObject::tr("Boundary detection criteria loosen by: "));
		QLineEdit_GWDT_boundaryConditionLoosenBy = new QLineEdit("1", QWidget_parent);
		QGroupBox *QGroupBox_GWDTparameter = new QGroupBox("GWDT paramters");
		QGroupBox_GWDTparameter->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_GWDTparameter = new QGridLayout();
		QGridLayout_GWDTparameter->addWidget(QLabel_GWDT_boundaryThin, 1, 1);
		QGridLayout_GWDTparameter->addWidget(QLineEdit_GWDT_boundaryThin, 1, 2);
		QGridLayout_GWDTparameter->addWidget(QLabel_GWDT_boundaryConditionLoosenBy, 1, 3);
		QGridLayout_GWDTparameter->addWidget(QLineEdit_GWDT_boundaryConditionLoosenBy, 1, 4);
		QGroupBox_GWDTparameter->setLayout(QGridLayout_GWDTparameter);

        QWidget* QWidget_bottomBar = new QWidget();
        QGridLayout* QGridLayout_bottomBar = new QGridLayout();
        QGridLayout_bottomBar->addWidget(QPushButton_start,1,1);
        QGridLayout_bottomBar->addWidget(QPushButton_close,1,2);
        QWidget_bottomBar->setLayout(QGridLayout_bottomBar);

        QGridLayout *QGridLayout_main = new QGridLayout();
        QGridLayout_main->addWidget(QGroupBox_threshold);
        QGridLayout_main->addWidget(QGroupBox_channel);
		QGridLayout_main->addWidget(QGroupBox_shape);
        QGridLayout_main->addWidget(QGroupBox_algorithm);
		QGridLayout_main->addWidget(QGroupBox_GVFparamter);
		QGridLayout_main->addWidget(QGroupBox_GWDTparameter);
		QGridLayout_main->addWidget(QWidget_bottomBar);


        setLayout(QGridLayout_main);
        setWindowTitle(QString("Cell Segmentation"));

        connect(QPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
        connect(QPushButton_close, SIGNAL(clicked()), this, SLOT(reject()));
        connect(QComboBox_threshold_type, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

        update();
    }

    ~dialogMain(){}

	public:
    QComboBox* QComboBox_channel_selection;
    QComboBox* QComboBox_threshold_type;
    QDoubleSpinBox* QDoubleSpinBox_threshold;
	QCheckBox* QCheckBox_algorithm_debug;
	QRadioButton* QRadioButton_algorithm_regionGrowingOnly;
	QRadioButton* QRadioButton_algorithm_regionGrowingGWDT;
	QRadioButton* QRadioButton_algorithm_regionGrowingGVF;
	QRadioButton* QRadioButton_algorithm_regionGrowFusing;
	QRadioButton* QRadioButton_shape_sphere;
	QRadioButton* QRadioButton_shape_cube;
	QLineEdit* QLineEdit_GVF_maxIteration;
	QLineEdit* QLineEdit_GVF_fusionThreshold;
	QLineEdit* QLineEdit_GVF_sigma;
	QLineEdit* QLineEdit_GVF_mu;
	QLineEdit* QLineEdit_Shape_delta;
	QLineEdit* QLineEdit_threshold_thresholdHistoSimilar;
	QLineEdit* QLineEdit_GWDT_boundaryThin;
	QLineEdit* QLineEdit_GWDT_boundaryConditionLoosenBy;
	V3DLONG int_channel;
    int int_thresholdType;
    double double_threshold;
	enum_algorithm_t type_algorithm;
	enum_shape_t type_shape;
	bool flag_debug;
	double double_GVF_maxIteration;
	double double_GVF_fusionThreshold;
	double double_GVF_sigma;
	double double_GVF_mu;
	double value_Shape_delta;
	double value_threshold_thresholdHistoSimilar;
	int count_boundaryThinIteration;
	double value_boudaryConditionLoosesBy;

	public slots:
    void update()
    {
        int_thresholdType = QComboBox_threshold_type->currentIndex();

        if(int_thresholdType == 1)
        {
            QDoubleSpinBox_threshold->setEnabled(true);
        }
        else
        {
            QDoubleSpinBox_threshold->setEnabled(false);
        }
    }
    void _slot_start()
    {
        int_channel = QComboBox_channel_selection->currentIndex() + 1;
        double_threshold = -1;
        if (QDoubleSpinBox_threshold->isEnabled())
        {
            double_threshold = QDoubleSpinBox_threshold->text().toDouble();
        }
        else
		{
			double_threshold = -1; //calculate automatically;
		}
		this->flag_debug = QCheckBox_algorithm_debug->isChecked();
		if (this->QRadioButton_algorithm_regionGrowingOnly->isChecked())
		{
			this->type_algorithm = regionGrowOnly;
		}
		if (this->QRadioButton_algorithm_regionGrowingGWDT->isChecked())
		{
			this->type_algorithm = regionGrowGWDT;
		}
		else if (this->QRadioButton_algorithm_regionGrowingGVF->isChecked())
		{
			this->type_algorithm = regionGrowGVF;
		}
		else if (this->QRadioButton_algorithm_regionGrowFusing->isChecked())
		{
			this->type_algorithm = regionGrowFusing;
		}
		else
		{
			this->type_algorithm = regionGrowOnly;
		}
		double_GVF_maxIteration = this->QLineEdit_GVF_maxIteration->text().toDouble();
		double_GVF_fusionThreshold = this->QLineEdit_GVF_fusionThreshold->text().toDouble();
		double_GVF_sigma = this->QLineEdit_GVF_sigma->text().toDouble();
		double_GVF_mu = this->QLineEdit_GVF_mu->text().toDouble();
		value_Shape_delta = this->QLineEdit_Shape_delta->text().toDouble();
		value_threshold_thresholdHistoSimilar = this->QLineEdit_threshold_thresholdHistoSimilar->text().toDouble();
		if (this->QRadioButton_shape_sphere->isChecked())
		{
			this->type_shape = sphere;
		}
		else if (this->QRadioButton_shape_cube->isChecked())
		{
			this->type_shape = cube;
		}
		count_boundaryThinIteration = this->QLineEdit_GWDT_boundaryThin->text().toInt();
		value_boudaryConditionLoosesBy = this->QLineEdit_GWDT_boundaryConditionLoosenBy->text().toDouble();
        accept();
    }
};
#endif