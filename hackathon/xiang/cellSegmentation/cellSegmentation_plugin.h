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
		QStringList_threshold_items << "Calculate from Markers" << "User Input";
		QComboBox_threshold_type = new QComboBox();
		QComboBox_threshold_type->addItems(QStringList_threshold_items);
		QLabel* QLabel_thresholdType = new QLabel(QObject::tr("Threshold: "));
		QGroupBox *QGroupBox_threshold = new QGroupBox("Threshold");
        QGroupBox_threshold->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_threshold = new QGridLayout();
        QGroupBox_threshold->setStyle(new QWindowsStyle());
        QGridLayout_threshold->addWidget(QLabel_thresholdType, 2,1);
        QGridLayout_threshold->addWidget(QComboBox_threshold_type, 2,2);
        QDoubleSpinBox_threshold = new QDoubleSpinBox();
        QGridLayout_threshold->addWidget(QDoubleSpinBox_threshold, 2,3);
        QDoubleSpinBox_threshold->setEnabled(false);
        QDoubleSpinBox_threshold->setMaximum(65535);
		QDoubleSpinBox_threshold->setMinimum(-65535);
        QGroupBox_threshold->setLayout(QGridLayout_threshold);

		QRadioButton_algorithm_regionGrowing = new QRadioButton("regionGrowing", QWidget_parent);
		QRadioButton_algorithm_regionGrowing->setChecked(false);
		QRadioButton_algorithm_regionGrowing->setEnabled(false);
		QRadioButton_algorithm_regionGrowingMeanShift = new QRadioButton("regionGrowing+meanShift", QWidget_parent);
		QRadioButton_algorithm_regionGrowingMeanShift->setChecked(false);
		QRadioButton_algorithm_regionGrowingMeanShift->setEnabled(false);
		QRadioButton_algorithm_GVF = new QRadioButton("GVF", QWidget_parent);
		QRadioButton_algorithm_GVF->setChecked(false);
		QRadioButton_algorithm_GVF->setEnabled(false);
		QRadioButton_algorithm_fusing = new QRadioButton("regionGrowing+GVF", QWidget_parent);
		QRadioButton_algorithm_fusing->setChecked(true);
		QCheckBox_algorithm_debug = new QCheckBox("Debugging Mode", QWidget_parent);
		QCheckBox_algorithm_debug->setChecked(false);
		QRadioButton_algorithm_regionGrowing = new QRadioButton("regionGrowing", QWidget_parent);
		QRadioButton_algorithm_regionGrowing->setChecked(false);
		QRadioButton_algorithm_regionGrowingMeanShift = new QRadioButton("regionGrowing+meanShift", QWidget_parent);
		QRadioButton_algorithm_regionGrowingMeanShift->setChecked(false);
		QRadioButton_algorithm_GVF = new QRadioButton("GFV", QWidget_parent);
		QRadioButton_algorithm_GVF->setChecked(false);
		QRadioButton_algorithm_GVF->setEnabled(false);
		QRadioButton_algorithm_fusing = new QRadioButton("Fusing (of the two)", QWidget_parent);
		QRadioButton_algorithm_fusing->setChecked(true);
		QCheckBox_algorithm_debug = new QCheckBox("Debugging Mode", QWidget_parent);
		QCheckBox_algorithm_debug->setChecked(false);
		QGroupBox *QGroupBox_algorithm = new QGroupBox("Algorithm");
		QGroupBox_algorithm->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_algorithm = new QGridLayout();
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowing, 1, 1);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_regionGrowingMeanShift, 1, 2);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_GVF, 2, 1);
		QGridLayout_algorithm->addWidget(QRadioButton_algorithm_fusing, 2, 2);
		QGridLayout_algorithm->addWidget(QCheckBox_algorithm_debug, 3, 1);
		QGroupBox_algorithm->setLayout(QGridLayout_algorithm);
	
		QLabel* QLabel_GVF_matIteration = new QLabel(QObject::tr("Max iteration: "));
		QLineEdit_GVF_maxIteration = new QLineEdit("5", QWidget_parent);
		QLabel* QLabel_GVF_fusionThreshold = new QLabel(QObject::tr("Fusion double_threshold: "));
		QLineEdit_GVF_fusionThreshold = new QLineEdit("3", QWidget_parent);
		QLabel* QLabel_GVF_sigma = new QLabel(QObject::tr("sigma: "));
		QLineEdit_GVF_sigma = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_GVF_lambda = new QLabel(QObject::tr("lambda: "));
		QLineEdit_GVF_lambda = new QLineEdit("0.2", QWidget_parent);
		QLabel* QLabel_GVF_mu = new QLabel(QObject::tr("mu: "));
		QLineEdit_GVF_mu = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_GVF_omega = new QLabel(QObject::tr("omega: "));
		QLineEdit_GVF_omega = new QLineEdit("1.0", QWidget_parent);
		QGroupBox *QGroupBox_GVFparamter = new QGroupBox("GVF paramters");
		QGroupBox_GVFparamter->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_GVFparamter = new QGridLayout();
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_matIteration, 1, 1);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_maxIteration, 1, 2);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_fusionThreshold, 1, 3);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_fusionThreshold, 1, 4);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_sigma, 2, 1);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_sigma, 2, 2);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_lambda, 2, 3);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_lambda, 2, 4);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_mu, 3, 1);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_mu, 3, 2);
		QGridLayout_GVFparamter->addWidget(QLabel_GVF_omega, 3, 3);
		QGridLayout_GVFparamter->addWidget(QLineEdit_GVF_omega, 3, 4);
		QGroupBox_GVFparamter->setLayout(QGridLayout_GVFparamter);

        QWidget* QWidget_bottomBar = new QWidget();
        QGridLayout* QGridLayout_bottomBar = new QGridLayout();
        QGridLayout_bottomBar->addWidget(QPushButton_start,1,1);
        QGridLayout_bottomBar->addWidget(QPushButton_close,1,2);
        QWidget_bottomBar->setLayout(QGridLayout_bottomBar);

        QGridLayout *QGridLayout_main = new QGridLayout();
        QGridLayout_main->addWidget(QGroupBox_threshold);
        QGridLayout_main->addWidget(QGroupBox_channel);
        QGridLayout_main->addWidget(QGroupBox_algorithm);
		QGridLayout_main->addWidget(QGroupBox_GVFparamter);
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
	QRadioButton* QRadioButton_algorithm_regionGrowing;
	QRadioButton* QRadioButton_algorithm_regionGrowingMeanShift;
	QRadioButton* QRadioButton_algorithm_GVF;
	QRadioButton* QRadioButton_algorithm_fusing;
	QLineEdit* QLineEdit_GVF_maxIteration;
	QLineEdit* QLineEdit_GVF_fusionThreshold;
	QLineEdit* QLineEdit_GVF_sigma;
	QLineEdit* QLineEdit_GVF_lambda;
	QLineEdit* QLineEdit_GVF_mu;
	QLineEdit* QLineEdit_GVF_omega;
    V3DLONG int_channel;
    int int_thresholdType;
    double double_threshold;
	int idx_algorithm;
	bool flag_debug;
	double double_GVF_maxIteration;
	double double_GVF_fusionThreshold;
	double double_GVF_sigma;
	double double_GVF_lambda;
	double double_GVF_mu;
	double double_GVF_omega;


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
		if (this->QRadioButton_algorithm_regionGrowing->isChecked())
		{
			this->idx_algorithm = 1;
		}
		if (this->QRadioButton_algorithm_regionGrowingMeanShift->isChecked())
		{
			this->idx_algorithm = 2;
		}
		else if (this->QRadioButton_algorithm_GVF->isChecked())
		{
			this->idx_algorithm = 3;
		}
		else if (this->QRadioButton_algorithm_fusing->isChecked())
		{
			this->idx_algorithm = 4;
		}
		else
		{
			this->idx_algorithm = 1;
		}
		double_GVF_maxIteration = this->QLineEdit_GVF_maxIteration->text().toDouble();
		double_GVF_fusionThreshold = this->QLineEdit_GVF_fusionThreshold->text().toDouble();
		double_GVF_sigma = this->QLineEdit_GVF_sigma->text().toDouble();
		double_GVF_lambda = this->QLineEdit_GVF_lambda->text().toDouble();
		double_GVF_mu = this->QLineEdit_GVF_mu->text().toDouble();
		double_GVF_omega = this->QLineEdit_GVF_omega->text().toDouble();
        accept();
    }
};


#endif

