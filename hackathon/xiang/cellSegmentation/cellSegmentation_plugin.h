/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 :by Xiang Li (lindbergh.li@gmail.com);
 */
 
#ifndef __CELLSEGMENTATION_PLUGIN_H__
#define __CELLSEGMENTATION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <string>

enum enum_algorithm_t {regionGrowOnly, GWDT, regionGrowGVF, regionGrowSRS, fusing};
enum enum_shape_t {sphere, cube};

class cellSegmentation :public QObject, public V3DPluginInterface2_1
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

class dialogMain:public QDialog
{
    Q_OBJECT

	public:
    dialogMain(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent, int int_channelDim)
    {
        //channel;
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
		else
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
		QLabel* QLabel_channel_main = new QLabel(QObject::tr("Channel:"));
        QGroupBox *QGroupBox_channel_main = new QGroupBox("Color channel");
        QGroupBox_channel_main->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_channel_main = new QGridLayout();
        QGroupBox_channel_main->setStyle(new QWindowsStyle());
        QGridLayout_channel_main->addWidget(QLabel_channel_main, 1,1,1,1);
        QGridLayout_channel_main->addWidget(QComboBox_channel_selection, 1,2,1,3);
        QGroupBox_channel_main->setLayout(QGridLayout_channel_main);

		//intensity;
		QStringList QStringList_intensity_thresholdType;
		QStringList_intensity_thresholdType << "Calculate from Image" << "User Input";
		QComboBox_intensity_thresholdType = new QComboBox();
		QComboBox_intensity_thresholdType->addItems(QStringList_intensity_thresholdType);
		QLabel* QLabel_intensity_thresholdType = new QLabel(QObject::tr("Threshold:"));
		QGroupBox *QGroupBox_intensity_main = new QGroupBox("Image intensity");
        QGroupBox_intensity_main->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_intensity_main = new QGridLayout();
        QGridLayout_intensity_main->addWidget(QLabel_intensity_thresholdType, 1,1,1,2);
        QGridLayout_intensity_main->addWidget(QComboBox_intensity_thresholdType, 1,2,1,1);
        QDoubleSpinBox_intensity_value = new QDoubleSpinBox();
		QDoubleSpinBox_intensity_value->setEnabled(false);
		QDoubleSpinBox_intensity_value->setMaximum(255);
		QDoubleSpinBox_intensity_value->setMinimum(0);
		QGridLayout_intensity_main->addWidget(QDoubleSpinBox_intensity_value, 1,3,1,1);
		QLabel* QLabel_intensity_histoCorrelation = new QLabel(QObject::tr("Histogram correlation:"));
		QLineEdit_intensity_histoCorrelation = new QLineEdit ("0.0", QWidget_parent);
		QGridLayout_intensity_main->addWidget(QLabel_intensity_histoCorrelation, 2, 1, 1, 1);
		QGridLayout_intensity_main->addWidget(QLineEdit_intensity_histoCorrelation, 2, 2, 1, 1);
		QLabel* QLabel_intensity_valueChangeRatio = new QLabel(QObject::tr("Max value change ratio:"));
		QLineEdit_intenstiy_valueChangeRatio = new QLineEdit ("0.8", QWidget_parent);
		QGridLayout_intensity_main->addWidget(QLabel_intensity_valueChangeRatio, 2, 3, 1, 1);
		QGridLayout_intensity_main->addWidget(QLineEdit_intenstiy_valueChangeRatio, 2, 4, 1, 1);
        QGroupBox_intensity_main->setLayout(QGridLayout_intensity_main);

		//shape;
		QLabel* QLabel_shape_type = new QLabel(QObject::tr("Region shape:"));
		QRadioButton_shape_sphere = new QRadioButton("sphere-like", QWidget_parent);
		QRadioButton_shape_sphere->setChecked(true);
		QRadioButton_shape_cube = new QRadioButton("cube-like", QWidget_parent);
		QRadioButton_shape_cube->setChecked(false);
		QLabel* QLabel_shape_delta = new QLabel(QObject::tr("max anisotropic\ndeviation:"));
		QLineEdit_Shape_delta = new QLineEdit("1", QWidget_parent);
		QGroupBox *QGroupBox_shape = new QGroupBox("Geometry stat");
		QGridLayout *QGridLayout_shape = new QGridLayout();
		QLabel* QLabel_shape_thresholdRegionSize = new QLabel(QObject::tr("Min region size\nvs. exemplar ratio:"));
		QLineEdit_shape_thresholdRegionSize = new QLineEdit("0.05", QWidget_parent);
		QLabel* QLabel_shape_uThresholdRegionSize = new QLabel(QObject::tr("Max region size\nvs. exemplar ratio:"));
		QLineEdit_shape_uThresholdRegionSize = new QLineEdit("20", QWidget_parent);
		QGridLayout_shape->addWidget(QLabel_shape_type, 1, 1, 1, 2);
		QGridLayout_shape->addWidget(QRadioButton_shape_sphere, 1, 2, 1, 1);
		QGridLayout_shape->addWidget(QRadioButton_shape_cube, 1, 3, 1, 1);
		QGridLayout_shape->addWidget(QLabel_shape_delta, 2, 1, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_Shape_delta, 2, 2, 1, 1);
		QGridLayout_shape->addWidget(QLabel_shape_thresholdRegionSize, 3, 1, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_shape_thresholdRegionSize, 3, 2, 1, 1);
		QGridLayout_shape->addWidget(QLabel_shape_uThresholdRegionSize, 3, 3, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_shape_uThresholdRegionSize, 3, 4, 1, 1);
		QGroupBox_shape->setLayout(QGridLayout_shape);

		//algorithm;
		QRadioButton_algorithm_regionGrowOnly = new QRadioButton("regionGrow\nOnly", QWidget_parent);
		QRadioButton_algorithm_regionGrowOnly->setChecked(false);
		QRadioButton_algorithm_GWDT = new QRadioButton("GWDT\n+Boundary detection", QWidget_parent);
		QRadioButton_algorithm_GWDT->setChecked(false);
		QRadioButton_algorithm_GWDT->setEnabled(false);
		QRadioButton_algorithm_regionGrowGVF = new QRadioButton("regionGrow\n+GVF", QWidget_parent);
		QRadioButton_algorithm_regionGrowGVF->setChecked(true);
		QRadioButton_algorithm_regionGrowSRS = new QRadioButton("regionGrow\n+SRS", QWidget_parent);
		QRadioButton_algorithm_regionGrowSRS->setChecked(false);
		QRadioButton_algorithm_fusing = new QRadioButton("Fusing", QWidget_parent);
		QRadioButton_algorithm_fusing->setChecked(false);
		QRadioButton_algorithm_fusing->setEnabled(false);
		QCheckBox_algorithm_debug = new QCheckBox("Debugging", QWidget_parent);
		QCheckBox_algorithm_debug->setChecked(true);
		QGroupBox *QGroupBox_algorithm_main = new QGroupBox("Algorithm selection");
		QGroupBox_algorithm_main->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_algorithm_main = new QGridLayout();
		QGridLayout_algorithm_main->addWidget(QRadioButton_algorithm_regionGrowOnly, 1, 1,1,1);
		QGridLayout_algorithm_main->addWidget(QRadioButton_algorithm_GWDT, 1, 2,1,1);
		QGridLayout_algorithm_main->addWidget(QRadioButton_algorithm_regionGrowGVF, 1, 3,1,1);
		QGridLayout_algorithm_main->addWidget(QRadioButton_algorithm_regionGrowSRS, 1, 4,1,1);
		QGridLayout_algorithm_main->addWidget(QRadioButton_algorithm_fusing, 2, 1, 1, 1);
		QGridLayout_algorithm_main->addWidget(QCheckBox_algorithm_debug, 2, 3, 1, 1);
		QGroupBox_algorithm_main->setLayout(QGridLayout_algorithm_main);
	
		//GVF paramters;
		QLabel* QLabel_GVF_maxIteration = new QLabel(QObject::tr("# of Iteration:"));
		QLineEdit_GVF_maxIteration = new QLineEdit("10", QWidget_parent);
		QLabel* QLabel_GVF_fusionThreshold = new QLabel(QObject::tr("merging threshold:"));
		QLineEdit_GVF_fusionThreshold = new QLineEdit("2", QWidget_parent);
		QLabel* QLabel_GVF_sigma = new QLabel(QObject::tr("sigma:"));
		QLineEdit_GVF_sigma = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_GVF_mu = new QLabel(QObject::tr("mu:"));
		QLineEdit_GVF_mu = new QLineEdit("0.1", QWidget_parent);
		//QLabel* QLabel_GVF_mergingCriteria = new QLabel(QObject::tr("Merging criteria:"));
		//QLineEdit_GVF_mergingCriteria = new QLineEdit("0.3", QWidget_parent);
		QGroupBox *QGroupBox_GVF_main = new QGroupBox("GVF paramters");
		QGroupBox_GVF_main->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_GVF_main = new QGridLayout();
		QGridLayout_GVF_main->addWidget(QLabel_GVF_maxIteration, 1, 1,1,1);
		QGridLayout_GVF_main->addWidget(QLineEdit_GVF_maxIteration, 1, 2,1,1);
		QGridLayout_GVF_main->addWidget(QLabel_GVF_fusionThreshold, 1, 3,1,1);
		QGridLayout_GVF_main->addWidget(QLineEdit_GVF_fusionThreshold, 1, 4,1,1);
		QGridLayout_GVF_main->addWidget(QLabel_GVF_sigma, 2, 1,1,1);
		QGridLayout_GVF_main->addWidget(QLineEdit_GVF_sigma, 2, 2,1,1);
		QGridLayout_GVF_main->addWidget(QLabel_GVF_mu, 2, 3,1,1);
		QGridLayout_GVF_main->addWidget(QLineEdit_GVF_mu, 2, 4,1,1);
		//QGridLayout_GVF_main->addWidget(QLabel_GVF_mergingCriteria, 3, 1,1,1);
		//QGridLayout_GVF_main->addWidget(QLineEdit_GVF_mergingCriteria, 3, 2,1,1);
		QGroupBox_GVF_main->setLayout(QGridLayout_GVF_main);

		//GWDT paramters;
		QLabel* QLabel_GWDT_boundaryThin = new QLabel(QObject::tr("# of erosion:"));
		QLineEdit_GWDT_boundaryThin = new QLineEdit("8", QWidget_parent);
		QLabel* QLabel_GWDT_boundaryConditionLoosenBy = new QLabel(QObject::tr("erosion radius:"));
		QLineEdit_GWDT_boundaryConditionLoosenBy = new QLineEdit("1", QWidget_parent);
		QGroupBox *QGroupBox_GWDTparameter = new QGroupBox("GWDT paramters");
		QGroupBox_GWDTparameter->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_GWDTparameter = new QGridLayout();
		QGridLayout_GWDTparameter->addWidget(QLabel_GWDT_boundaryThin, 1, 1,1,1);
		QGridLayout_GWDTparameter->addWidget(QLineEdit_GWDT_boundaryThin, 1, 2,1,1);
		QGridLayout_GWDTparameter->addWidget(QLabel_GWDT_boundaryConditionLoosenBy, 1, 3,1,1);
		QGridLayout_GWDTparameter->addWidget(QLineEdit_GWDT_boundaryConditionLoosenBy, 1, 4,1,1);
		QGroupBox_GWDTparameter->setLayout(QGridLayout_GWDTparameter);

		//SRS paramters
		QLabel* QLabel_SRS_countSeed = new QLabel(QObject::tr("# of seeds:"));
		QLineEdit_SRS_countSeed = new QLineEdit("6", QWidget_parent);
		QLabel* QLabel_SRS_initialT = new QLabel(QObject::tr("Initial T:"));
		QLineEdit_SRS_initialT = new QLineEdit("1000", QWidget_parent);
		QLabel* QLabel_SRS_minT = new QLabel(QObject::tr("min T:"));
		QLineEdit_SRS_minT = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_SRS_grateRatio = new QLabel(QObject::tr("Grate:"));
		QLineEdit_SRS_grateRatio = new QLineEdit("0.8", QWidget_parent);
		QLabel* QLabel_SRS_maxIteration = new QLabel(QObject::tr("# of iteration:"));
		QLineEdit_SRS_maxIteration = new QLineEdit("30", QWidget_parent);
		QLabel* QLabel_SRS_convergeCriteria = new QLabel(QObject::tr("Converge at:"));
		QLineEdit_SRS_convergeCriteria = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_SRS_initialLambda = new QLabel(QObject::tr("Initial lambda:"));
		QLineEdit_SRS_initialLambda = new QLineEdit("100", QWidget_parent);
		QGridLayout *QGridLayout_SRSparameter = new QGridLayout();
		QLabel* QLabel_SRS_mergingCriteria = new QLabel(QObject::tr("Merging criteria:"));
		QLineEdit_SRS_mergingCriteria = new QLineEdit("0.5", QWidget_parent);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_initialT, 1, 1,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_initialT, 1, 2,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_minT, 1, 3,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_minT, 1, 4,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_grateRatio, 2, 1,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_grateRatio, 2, 2,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_maxIteration, 2, 3,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_maxIteration, 2, 4,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_convergeCriteria, 3, 1,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_convergeCriteria, 3, 2,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_initialLambda, 3, 3,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_initialLambda, 3, 4,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_mergingCriteria, 4, 1,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_mergingCriteria, 4, 2,1,1);
		QGridLayout_SRSparameter->addWidget(QLabel_SRS_countSeed, 4, 3,1,1);
		QGridLayout_SRSparameter->addWidget(QLineEdit_SRS_countSeed, 4, 4,1,1);
		QGroupBox *QGroupBox_SRSparameter = new QGroupBox("SRS Paramters");
		QGroupBox_SRSparameter->setStyle(new QWindowsStyle());
		QGroupBox_SRSparameter->setLayout(QGridLayout_SRSparameter);

		//control;
		QPushButton *QPushButton_control_start = new QPushButton(QObject::tr("Start Segmentation"));
		QPushButton *QPushButton_control_close = new QPushButton(QObject::tr("Close"));
		QWidget* QWidget_control_bar = new QWidget();
        QGridLayout* QGridLayout_control_bar = new QGridLayout();
        QGridLayout_control_bar->addWidget(QPushButton_control_start,1,1,1,2);
        QGridLayout_control_bar->addWidget(QPushButton_control_close,1,3,1,2);
        QWidget_control_bar->setLayout(QGridLayout_control_bar);

        //main pandel;
		QGridLayout *QGridLayout_main = new QGridLayout();
        QGridLayout_main->addWidget(QGroupBox_intensity_main);
        QGridLayout_main->addWidget(QGroupBox_channel_main);
		QGridLayout_main->addWidget(QGroupBox_shape);
        QGridLayout_main->addWidget(QGroupBox_algorithm_main);
		QGridLayout_main->addWidget(QGroupBox_GVF_main);
		QGridLayout_main->addWidget(QGroupBox_GWDTparameter);
		QGridLayout_main->addWidget(QGroupBox_SRSparameter);
		QGridLayout_main->addWidget(QWidget_control_bar);
        setLayout(QGridLayout_main);
        setWindowTitle(QString("Cell Segmentation"));

		//event evoking;
        connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
        connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
        connect(QComboBox_intensity_thresholdType, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

        update();
    }

    ~dialogMain(){}

	public:
    QComboBox* QComboBox_channel_selection;
    QComboBox* QComboBox_intensity_thresholdType;
    QDoubleSpinBox* QDoubleSpinBox_intensity_value;
	QCheckBox* QCheckBox_algorithm_debug;
	QRadioButton* QRadioButton_algorithm_regionGrowOnly;
	QRadioButton* QRadioButton_algorithm_GWDT;
	QRadioButton* QRadioButton_algorithm_regionGrowGVF;
	QRadioButton* QRadioButton_algorithm_fusing;
	QRadioButton* QRadioButton_algorithm_regionGrowSRS;
	QRadioButton* QRadioButton_shape_sphere;
	QRadioButton* QRadioButton_shape_cube;
	QLineEdit* QLineEdit_GVF_maxIteration;
	QLineEdit* QLineEdit_GVF_fusionThreshold;
	QLineEdit* QLineEdit_GVF_sigma;
	QLineEdit* QLineEdit_GVF_mu;
	//QLineEdit* QLineEdit_GVF_mergingCriteria;
	QLineEdit* QLineEdit_Shape_delta;
	QLineEdit* QLineEdit_intensity_histoCorrelation;
	QLineEdit* QLineEdit_GWDT_boundaryThin;
	QLineEdit* QLineEdit_GWDT_boundaryConditionLoosenBy;
	QLineEdit* QLineEdit_shape_thresholdRegionSize;
	QLineEdit* QLineEdit_shape_uThresholdRegionSize;
	QLineEdit* QLineEdit_intenstiy_valueChangeRatio;
	QLineEdit* QLineEdit_SRS_initialT;
	QLineEdit* QLineEdit_SRS_minT;
	QLineEdit* QLineEdit_SRS_grateRatio;
	QLineEdit* QLineEdit_SRS_maxIteration;
	QLineEdit* QLineEdit_SRS_convergeCriteria;
	QLineEdit* QLineEdit_SRS_initialLambda;
	QLineEdit* QLineEdit_SRS_mergingCriteria;
	QLineEdit* QLineEdit_SRS_countSeed;
	V3DLONG channel_idx_selection;
    int intensity_threshold_type;
	double intensity_threshold_histoCorr;
    double intensity_threshold_global;
	enum_algorithm_t algorithm_type_selection;
	enum_shape_t shape_type_selection;
	bool algorithm_is_debug;
	double GVF_para_maxIteration;
	double GVF_para_mergingThreshold;
	double GVF_para_sigma;
	double GVF_para_mu;
	//double GVF_para_mergingCriteria;
	double shape_para_delta;
	int GWDT_para_boundaryThinIteration;
	double GWDT_para_boundaryCriteria;
	double SRS_para_initialT;
	double SRS_para_minT;
	double SRS_para_grateRatio;
	double SRS_para_maxIteration;
	double SRS_para_convergeCriteria;
	double SRS_para_initialLambda;
	double SRS_para_mergingCriteria;
	int SRS_para_countSeed;
	double shape_multiplier_thresholdRegionSize;
	double shape_multiplier_uThresholdRegionSize;
	double intensity_threshold_valueChangeRatio;

	public slots:
    void update()
    {
        intensity_threshold_type = QComboBox_intensity_thresholdType->currentIndex();

        if(intensity_threshold_type == 1)
        {
            QDoubleSpinBox_intensity_value->setEnabled(true);
        }
        else
        {
            QDoubleSpinBox_intensity_value->setEnabled(false);
        }
    }
    void _slot_start()
    {
        channel_idx_selection = QComboBox_channel_selection->currentIndex() + 1;
        intensity_threshold_global = -1;
        if (QDoubleSpinBox_intensity_value->isEnabled())
        {
            intensity_threshold_global = QDoubleSpinBox_intensity_value->text().toDouble();
        }
        else
		{
			intensity_threshold_global = -1; //calculate automatically;
		}
		this->algorithm_is_debug = QCheckBox_algorithm_debug->isChecked();
		if (this->QRadioButton_algorithm_regionGrowOnly->isChecked())
		{
			this->algorithm_type_selection = regionGrowOnly;
		}
		if (this->QRadioButton_algorithm_GWDT->isChecked())
		{
			this->algorithm_type_selection = GWDT;
		}
		else if (this->QRadioButton_algorithm_regionGrowGVF->isChecked())
		{
			this->algorithm_type_selection = regionGrowGVF;
		}
		else if (this->QRadioButton_algorithm_fusing->isChecked())
		{
			this->algorithm_type_selection = fusing;
		}
		else if (this->QRadioButton_algorithm_regionGrowSRS->isChecked())
		{
			this->algorithm_type_selection = regionGrowSRS;
		}
		else
		{
			this->algorithm_type_selection = regionGrowOnly;
		}
		GVF_para_maxIteration = this->QLineEdit_GVF_maxIteration->text().toDouble();
		GVF_para_mergingThreshold = this->QLineEdit_GVF_fusionThreshold->text().toDouble();
		GVF_para_sigma = this->QLineEdit_GVF_sigma->text().toDouble();
		GVF_para_mu = this->QLineEdit_GVF_mu->text().toDouble();
		//GVF_para_mergingCriteria = this->QLineEdit_GVF_mergingCriteria->text().toDouble();
		shape_para_delta = this->QLineEdit_Shape_delta->text().toDouble();
		intensity_threshold_histoCorr = this->QLineEdit_intensity_histoCorrelation->text().toDouble();
		if (this->QRadioButton_shape_sphere->isChecked())
		{
			this->shape_type_selection = sphere;
		}
		else if (this->QRadioButton_shape_cube->isChecked())
		{
			this->shape_type_selection = cube;
		}
		GWDT_para_boundaryThinIteration = this->QLineEdit_GWDT_boundaryThin->text().toInt();
		GWDT_para_boundaryCriteria = this->QLineEdit_GWDT_boundaryConditionLoosenBy->text().toDouble();
		shape_multiplier_thresholdRegionSize = this->QLineEdit_shape_thresholdRegionSize->text().toDouble();
		shape_multiplier_uThresholdRegionSize = this->QLineEdit_shape_uThresholdRegionSize->text().toDouble();
		intensity_threshold_valueChangeRatio = this->QLineEdit_intenstiy_valueChangeRatio->text().toDouble();
		SRS_para_initialT = this->QLineEdit_SRS_initialT->text().toDouble();
		SRS_para_minT = this->QLineEdit_SRS_minT->text().toDouble();
		SRS_para_grateRatio = this->QLineEdit_SRS_grateRatio->text().toDouble();
		SRS_para_maxIteration = this->QLineEdit_SRS_maxIteration->text().toDouble();
		SRS_para_convergeCriteria = this->QLineEdit_SRS_convergeCriteria->text().toDouble();
		SRS_para_initialLambda = this->QLineEdit_SRS_initialLambda->text().toDouble();
		SRS_para_mergingCriteria = this->QLineEdit_SRS_mergingCriteria->text().toDouble();
		SRS_para_countSeed = this->QLineEdit_SRS_countSeed->text().toInt();
        accept();
    }
};
#endif