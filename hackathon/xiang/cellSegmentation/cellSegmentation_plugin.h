/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 :by Xiang Li (lindbergh.li@gmail.com);
 */
 
#pragma region "headers and constant defination"
#ifndef __CELLSEGMENTATION_PLUGIN_H__
#define __CELLSEGMENTATION_PLUGIN_H__
#include <QtGui>
#include <v3d_interface.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <string>
#include "v3d_message.h"
#include "cellSegmentation_plugin.h"
#include <vector>
#include <cassert>
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <basic_landmark.h>
#include "compute_win_pca.h"
#include "convert_type2uint8.h"
using namespace std;
const int const_length_histogram = 256;
const double const_max_voxelValue = 255;
const int const_count_neighbors = 26; //27 directions -1;
const double default_threshold_global = 10; //a small enough value for the last resort;
const int default_threshold_regionSize = 8; //cube of 2 voxel length;
const double default_multiplier_uThreshold_regionSize = 0.1; 
const double const_infinitesimal = 0.000000001;
#define INF 1E9
#define NINF -1E9
#define PI 3.14159265
enum enum_shape_t {sphere, cube};
#pragma endregion

#pragma region "dialogDefineExemplar" 
class dialogDefineExemplar:public QDialog
{
	Q_OBJECT
public:
	dialogDefineExemplar(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent, int int_channelDim)
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
			QComboBox_channel_selection->setCurrentIndex(0);
		}
		QGroupBox *QGroupBox_channel_main = new QGroupBox("Color channel");
		QGroupBox_channel_main->setStyle(new QWindowsStyle());
		QGridLayout *QGridLayout_channel_main = new QGridLayout();
		QGroupBox_channel_main->setStyle(new QWindowsStyle());
		QGridLayout_channel_main->addWidget(QComboBox_channel_selection, 1,1,1,1);
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
		QGridLayout_intensity_main->addWidget(QLabel_intensity_thresholdType, 1,1,1,1);
		QGridLayout_intensity_main->addWidget(QComboBox_intensity_thresholdType, 1,2,1,1);
		QDoubleSpinBox_intensity_value = new QDoubleSpinBox();
		QDoubleSpinBox_intensity_value->setEnabled(false);
		QDoubleSpinBox_intensity_value->setMaximum(255);
		QDoubleSpinBox_intensity_value->setMinimum(0);
		QGridLayout_intensity_main->addWidget(QDoubleSpinBox_intensity_value, 1,3,1,1);
		QLabel* QLabel_intensity_valueChangeRatio = new QLabel(QObject::tr("Max value change ratio:"));
		QLineEdit_intenstiy_valueChangeRatio = new QLineEdit ("0.1", QWidget_parent);
		QGridLayout_intensity_main->addWidget(QLabel_intensity_valueChangeRatio, 2, 1, 1, 1);
		QGridLayout_intensity_main->addWidget(QLineEdit_intenstiy_valueChangeRatio, 2, 2, 1, 1);
		QGroupBox_intensity_main->setLayout(QGridLayout_intensity_main);
		//control;
		QPushButton *QPushButton_control_start = new QPushButton(QObject::tr("Define exemplar"));
		QPushButton *QPushButton_control_close = new QPushButton(QObject::tr("Close"));
		QWidget* QWidget_control_bar = new QWidget();
		QGridLayout* QGridLayout_control_bar = new QGridLayout();
		QGridLayout_control_bar->addWidget(QPushButton_control_start,1,1,1,1);
		QGridLayout_control_bar->addWidget(QPushButton_control_close,1,2,1,1);
		QWidget_control_bar->setLayout(QGridLayout_control_bar);
		//main pandel;
		QGridLayout *QGridLayout_main = new QGridLayout();
		QGridLayout_main->addWidget(QGroupBox_channel_main);
		QGridLayout_main->addWidget(QGroupBox_intensity_main);
		QGridLayout_main->addWidget(QWidget_control_bar);
		setLayout(QGridLayout_main);
		setWindowTitle(QString("Exemplar Defination"));
		//event evoking;
		connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
		connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
		connect(QComboBox_intensity_thresholdType, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		update();
	}
	~dialogDefineExemplar(){}
	QComboBox* QComboBox_intensity_thresholdType;
	QDoubleSpinBox* QDoubleSpinBox_intensity_value;
	QLineEdit* QLineEdit_intenstiy_valueChangeRatio;
	double intensity_threshold_global;
	double intensity_threshold_valueChangeRatio;
	QComboBox* QComboBox_channel_selection;
	V3DLONG channel_idx_selection;
	public slots:
		void update()
		{
			if(QComboBox_intensity_thresholdType->currentIndex() == 1)
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
			intensity_threshold_valueChangeRatio = this->QLineEdit_intenstiy_valueChangeRatio->text().toDouble();
			accept();
		}
};
#pragma endregion

#pragma  region "dialogPropagateExemplar" 
class dialogPropagateExemplar:public QDialog
{
	Q_OBJECT
public:
	dialogPropagateExemplar(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
	{
		//intensity;
		QGroupBox* QGroupBox_intensity_main = new QGroupBox("Histogram Correlation");
		QGridLayout* QGridLayout_intensity_main = new QGridLayout();
		QLineEdit_intensity_histoCorrelation = new QLineEdit ("0.0", QWidget_parent);
		QLineEdit_intensity_histoCorrelation->setEnabled(false);
		QGridLayout_intensity_main->addWidget(QLineEdit_intensity_histoCorrelation, 1, 1, 1, 1);
		QGroupBox_intensity_main->setLayout(QGridLayout_intensity_main);
		//shape;
		QGroupBox *QGroupBox_shape = new QGroupBox("Geometry stat");
		QGridLayout *QGridLayout_shape = new QGridLayout();
		QRadioButton_shape_sphere = new QRadioButton("sphere-like", QWidget_parent);
		QRadioButton_shape_sphere->setChecked(true);
		QRadioButton_shape_cube = new QRadioButton("cube-like", QWidget_parent);
		QRadioButton_shape_cube->setChecked(false);
		QGridLayout_shape->addWidget(QRadioButton_shape_sphere, 1, 1, 1, 1);
		QGridLayout_shape->addWidget(QRadioButton_shape_cube, 1, 2, 1, 1);
		QLabel* QLabel_shape_delta = new QLabel(QObject::tr("max anisotropic\ndeviation:"));
		QLineEdit_Shape_delta = new QLineEdit("2.5", QWidget_parent);
		QGridLayout_shape->addWidget(QLabel_shape_delta, 1, 3, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_Shape_delta, 1, 4, 1, 1);
		QLabel* QLabel_shape_thresholdRegionSize = new QLabel(QObject::tr("Min region size\nvs. exemplar ratio:"));
		QLineEdit_shape_thresholdRegionSize = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_shape_uThresholdRegionSize = new QLabel(QObject::tr("Max region size\nvs. exemplar ratio:"));
		QLineEdit_shape_uThresholdRegionSize = new QLineEdit("200000", QWidget_parent);
		QGridLayout_shape->addWidget(QLabel_shape_thresholdRegionSize, 2, 1, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_shape_thresholdRegionSize, 2, 2, 1, 1);
		QGridLayout_shape->addWidget(QLabel_shape_uThresholdRegionSize, 2, 3, 1, 1);
		QGridLayout_shape->addWidget(QLineEdit_shape_uThresholdRegionSize, 2, 4, 1, 1);
		QGroupBox_shape->setLayout(QGridLayout_shape);
		//control;
		QPushButton *QPushButton_control_start = new QPushButton(QObject::tr("Propagate"));
		QPushButton *QPushButton_control_close = new QPushButton(QObject::tr("Close"));
		QWidget* QWidget_control_bar = new QWidget();
		QGridLayout* QGridLayout_control_bar = new QGridLayout();
		QGridLayout_control_bar->addWidget(QPushButton_control_start,1,1,1,1);
		QGridLayout_control_bar->addWidget(QPushButton_control_close,1,2,1,1);
		QWidget_control_bar->setLayout(QGridLayout_control_bar);
		//main pandel;
		QGridLayout *QGridLayout_main = new QGridLayout();
		QGridLayout_main->addWidget(QGroupBox_intensity_main);
		QGridLayout_main->addWidget(QGroupBox_shape);
		QGridLayout_main->addWidget(QWidget_control_bar);
		setLayout(QGridLayout_main);
		setWindowTitle(QString("Exemplar Propagation"));
		//event evoking;
		connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
		connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
		update();
	}
	~dialogPropagateExemplar(){}
	QLineEdit* QLineEdit_Shape_delta;
	QLineEdit* QLineEdit_intensity_histoCorrelation;
	QLineEdit* QLineEdit_shape_thresholdRegionSize;
	QLineEdit* QLineEdit_shape_uThresholdRegionSize;
	QRadioButton* QRadioButton_shape_sphere;
	QRadioButton* QRadioButton_shape_cube;
	enum_shape_t shape_type_selection;
	double intensity_threshold_histoCorr;
	double shape_para_delta;
	double shape_multiplier_thresholdRegionSize;
	double shape_multiplier_uThresholdRegionSize;
	public slots:
		void _slot_start()
		{
			shape_para_delta = this->QLineEdit_Shape_delta->text().toDouble();
			intensity_threshold_histoCorr = this->QLineEdit_intensity_histoCorrelation->text().toDouble();
			shape_multiplier_thresholdRegionSize = this->QLineEdit_shape_thresholdRegionSize->text().toDouble();
			shape_multiplier_uThresholdRegionSize = this->QLineEdit_shape_uThresholdRegionSize->text().toDouble();
			if (this->QRadioButton_shape_sphere->isChecked())
			{
				this->shape_type_selection = sphere;
			}
			else if (this->QRadioButton_shape_cube->isChecked())
			{
				this->shape_type_selection = cube;
			}
			accept();
		}
};
#pragma endregion

#pragma region "dialogFurtherSegmentation" 
class dialogFurtherSegmentation:public QDialog
{
	Q_OBJECT
public:
	dialogFurtherSegmentation(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
	{
		//GVF paramters;
		QLabel* QLabel_GVF_maxIteration = new QLabel(QObject::tr("# of Iteration:"));
		QLineEdit_GVF_maxIteration = new QLineEdit("20", QWidget_parent);
		QLabel* QLabel_GVF_fusionThreshold = new QLabel(QObject::tr("merging threshold:"));
		QLineEdit_GVF_fusionThreshold = new QLineEdit("1", QWidget_parent);
		QLabel* QLabel_GVF_sigma = new QLabel(QObject::tr("sigma:"));
		QLineEdit_GVF_sigma = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_GVF_mu = new QLabel(QObject::tr("mu:"));
		QLineEdit_GVF_mu = new QLineEdit("0.1", QWidget_parent);
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
		QGroupBox_GVF_main->setLayout(QGridLayout_GVF_main);
		//control;
		QPushButton *QPushButton_control_start = new QPushButton(QObject::tr("Segment by GVF"));
		QPushButton *QPushButton_control_close = new QPushButton(QObject::tr("Close"));
		QWidget* QWidget_control_bar = new QWidget();
		QGridLayout* QGridLayout_control_bar = new QGridLayout();
		QGridLayout_control_bar->addWidget(QPushButton_control_start,1,1,1,2);
		QGridLayout_control_bar->addWidget(QPushButton_control_close,1,3,1,2);
		QWidget_control_bar->setLayout(QGridLayout_control_bar);
		//main pandel;
		QGridLayout *QGridLayout_main = new QGridLayout();
		QGridLayout_main->addWidget(QGroupBox_GVF_main);
		QGridLayout_main->addWidget(QWidget_control_bar);
		setLayout(QGridLayout_main);
		setWindowTitle(QString("Further Segmentation"));
		//event evoking;
		connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
		connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
		update();
	}
	~dialogFurtherSegmentation(){}
	QLineEdit* QLineEdit_GVF_maxIteration;
	QLineEdit* QLineEdit_GVF_fusionThreshold;
	QLineEdit* QLineEdit_GVF_sigma;
	QLineEdit* QLineEdit_GVF_mu;
	double GVF_para_maxIteration;
	double GVF_para_mergingThreshold;
	double GVF_para_sigma;
	double GVF_para_mu;
	public slots:
		void _slot_start()
		{
			GVF_para_maxIteration = this->QLineEdit_GVF_maxIteration->text().toDouble();
			GVF_para_mergingThreshold = this->QLineEdit_GVF_fusionThreshold->text().toDouble();
			GVF_para_sigma = this->QLineEdit_GVF_sigma->text().toDouble();
			GVF_para_mu = this->QLineEdit_GVF_mu->text().toDouble();
			accept();
		}
};
#pragma endregion

class cellSegmentation :public QObject, public V3DPluginInterface2_1
{
	public:
	#pragma region "class: class_segmentationMain"
	class class_segmentationMain
	{
		#pragma region "class member"
		public:
			struct double3D
			{
				double x; double y; double z;
				double3D(double _x=0, double _y=0, double _z=0) {x=_x; y=_y; z=_z;}
			};

			struct long3D
			{
				V3DLONG x;
				V3DLONG y;
				V3DLONG z;
				long3D(V3DLONG _x=0, V3DLONG _y=0, V3DLONG _z=0) {x=_x; y=_y; z=_z;}
			};

			//constant
			vector<V3DLONG> poss_neighborRelative;
			vector<double3D> point_neighborRelative;
			vector<vector<V3DLONG> > colors_simpleTable;
			
			//Input or directly derived;
			unsigned char* Image1D_original;
			unsigned char* Image1D_page;
			unsigned char* Image1D_mask;
			unsigned char*** Image3D_page;
			V3DLONG dim_X;
			V3DLONG dim_Y;
			V3DLONG dim_Z;
			V3DLONG size_page;
			V3DLONG offset_channel;
			V3DLONG offset_Z;
			V3DLONG offset_Y;
			int idx_channel;
			int idx_shape;
			vector<double> histo_page;
			V3DLONG count_totalWhiteVoxel;
			double threshold_global;
			double threshold_histoSimilar;
			
			//Exemplar (or learn from it);
			vector<V3DLONG> poss_exemplar;
			vector<vector<V3DLONG> > possVct_exemplarRegion;
			vector<vector<double> > histoVct_exemplarRegion;
			vector<vector<vector<double> > > valueVctVct_exemplarShapeStat;
			double threshold_deltaShapeStat;
			double threshold_regionSize;
			double uThreshold_regionSize;
			double uThreshold_valueChangeRatio;
			double multiplier_thresholdRegionSize;
			double multiplier_uThresholdRegionSize;
			unsigned char* Image1D_exemplar;

			//segmentation;
			vector<vector<V3DLONG> > possVct_segmentationResultOriginal;
			vector<vector<V3DLONG> > possVct_segmentationResultPassed;
			vector<vector<V3DLONG> > possVct_segmentationResultSplitted;
			vector<vector<V3DLONG> > possVct_segmentationResultMerged;
			vector<vector<V3DLONG> > possVct_segmentationResultGVF;
			vector<vector<V3DLONG> > possVct_segmentationSeed;
			unsigned char* Image1D_segmentationResultPassed;
			unsigned char* Image1D_segmentationResultMerged;
			unsigned char* Image1D_segmentationResultOriginal;
			unsigned char* Image1D_segmentationResultGVF;
			LandmarkList LandmarkList_segmentationResultPassed;
			LandmarkList LandmarkList_segmentationResultGVF;
			LandmarkList LandmarkList_segmentationResultMerged;
			vector<V3DLONG> poss_segmentationResultCenterPassed;
			vector<V3DLONG> poss_segmentationResultCenterGVF;
			vector<V3DLONG> poss_segmentationResultCenterMerged;
			vector<double> paras_GVF;
			#pragma endregion
			
		class_segmentationMain() {}

		#pragma region "control-intialize"
		void control_initialize(unsigned char* _Image1D_original, V3DLONG _dim_X, V3DLONG _dim_Y, V3DLONG _dim_Z , int _idx_channel)
		{
			vector<V3DLONG> color_tmp(3, 0); color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 0;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 0;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 0; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 0;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 0;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 0;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 128; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 128;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 255;
			colors_simpleTable.push_back(color_tmp);
			
			this->Image1D_original = _Image1D_original;
			this->dim_X = _dim_X; this->dim_Y = _dim_Y; this->dim_Z = _dim_Z; this->idx_channel = _idx_channel;
			this->size_page = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*size_page;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
			this->Image1D_page = memory_allocate_uchar1D(this->size_page);
			this->Image1D_mask = memory_allocate_uchar1D(this->size_page);
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_page[i] = Image1D_original[i+offset_channel];
				this->Image1D_mask[i] = const_max_voxelValue; //all available;
			}
			ofstream ofstream_log;
			ofstream_log.open ("log_constructor.txt");
			ofstream_log<<"dim_X: "<<this->dim_X<<", dim_Y: "<<this->dim_Y<<", dim_Z: "<<this->dim_Z<<";"<<endl;
			ofstream_log<<"size_page: "<<this->size_page<<"; "<<"current channel: "<<this->idx_channel<<";"<<endl;
			ofstream_log.close();
			return;
		}
		#pragma  endregion

		#pragma region "control-defineExemplar"
		void control_defineExemplar(double _threshold_global, LandmarkList _LandmarkList_exemplar, double _uThreshold_valueChangeRatio)
		{
			this->uThreshold_valueChangeRatio = _uThreshold_valueChangeRatio;
			this->createNeighborVector();
			this->Image1D_exemplar  = memory_allocate_uchar1D(this->size_page+this->size_page+this->size_page);
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_exemplar[i] = 0;
				this->Image1D_exemplar[i+size_page] = 0;
				this->Image1D_exemplar[i+size_page+size_page] = 0;
			}
			ofstream ofstream_log;
			ofstream_log.open ("log_defineExemplar.txt");
			this->poss_exemplar = landMarkList2IndexList(_LandmarkList_exemplar);
			if (_threshold_global<0) {this->estimateThreshold();} //store to this->threshold_global;
			else {this->threshold_global = _threshold_global;}
			ofstream_log<<"estimateThreshold succeed, threshold_global: "<<this->threshold_global<<";"<<endl;
			this->count_totalWhiteVoxel = this->thresholdForCurrentPage();
			V3DLONG count_removedVoxel = this->removeSingleVoxel(this->Image1D_page, this->size_page);
			ofstream_log<<"removeSingleVoxel succeed, "<<count_removedVoxel<<" single voxel removed;"<<endl;
			ofstream_log<<"count_totalWhiteVoxel: "<<this->count_totalWhiteVoxel<<endl;
			this->possVct_exemplarRegion = this->regionGrowOnPoss(this->poss_exemplar);
			this->possVct2Image1DC(this->possVct_exemplarRegion, this->Image1D_exemplar);
			if (this->possVct_exemplarRegion.empty())
			{
				v3d_msg("Warning: no exemplar regions grown. Program will terminate, please re-select the exemplar(s)!");
				ofstream_log<<"regonGrow for exemplar failed!"<<endl; ofstream_log.close(); return;
			}
			ofstream_log.close();
			return;
		}
		#pragma  endregion

		#pragma region "control-propagateExemplar"
		void control_propagateExemplar(int _idx_shape, double _threshold_deltaShapeStat, double _threshold_histoSimilar, double _multiplier_thresholdRegionSize, double _multiplier_uThresholdRegionSize)
		{
			ofstream ofstream_log;
			ofstream_log.open ("log_propagateExemplar.txt");
			this->idx_shape = _idx_shape;
			this->threshold_deltaShapeStat = _threshold_deltaShapeStat;
			this->threshold_histoSimilar = _threshold_histoSimilar;
			this->multiplier_thresholdRegionSize = _multiplier_thresholdRegionSize;
			this->multiplier_uThresholdRegionSize = _multiplier_uThresholdRegionSize;
			this->Image1D_segmentationResultPassed = memory_allocate_uchar1D(this->size_page+this->size_page+this->size_page);
			this->Image1D_segmentationResultOriginal = memory_allocate_uchar1D(this->size_page+this->size_page+this->size_page);
			this->Image3D_page = memory_allocate_uchar3D(this->dim_Y, this->dim_X, this->dim_Z); //tricky!
			vector<V3DLONG> xyz_i (3, 0);
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_segmentationResultOriginal[i] = 0;
				this->Image1D_segmentationResultOriginal[i+size_page] = 0;
				this->Image1D_segmentationResultOriginal[i+size_page+size_page] = 0;
				this->Image1D_segmentationResultPassed[i] = 0;
				this->Image1D_segmentationResultPassed[i+size_page] = 0;
				this->Image1D_segmentationResultPassed[i+size_page+size_page] = 0;
				xyz_i = this->index2Coordinate(i);
				this->Image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = this->Image1D_page[i];
			}
			if (!this->analyzeExemplarRegion())
			{
				v3d_msg("Warning: analyzeExemplarRegion failed. Program will terminate, please re-select the exemplar(s)!");
				ofstream_log<<"analyzeExemplarRegion failed!"<<endl; ofstream_log.close(); return;
			}
			ofstream_log<<"threshold_deltaShapeStat: "<<this->threshold_deltaShapeStat<<endl;
			ofstream_log<<"threshold_regionSize: "<<this->threshold_regionSize<<endl;
			ofstream_log<<"uThreshold_regionSize: "<<this->uThreshold_regionSize<<endl;
			ofstream_log<<"uThreshold_valueChangeRatio: "<<this->uThreshold_valueChangeRatio<<endl;
			for (int i=0;i<this->possVct_exemplarRegion.size();i++)
			{
				ofstream_log<<" exemplar region"<<(i+1)<<", voxel count: "<<this->possVct_exemplarRegion[i].size()<<endl;
				ofstream_log<<" exemplar region"<<(i+1)<<", PC1: "<<endl;
				for (int j=0;j<4;j++)
				{
					ofstream_log<<" "<<this->valueVctVct_exemplarShapeStat[i][0][j];
				}
				ofstream_log<<endl;
				ofstream_log<<" exemplar region"<<(i+1)<<", PC2: "<<endl;
				for (int j=0;j<4;j++)
				{
					ofstream_log<<" "<<this->valueVctVct_exemplarShapeStat[i][1][j];
				}
				ofstream_log<<endl;
				ofstream_log<<" exemplar region"<<(i+1)<<", PC3: "<<endl;
				for (int j=0;j<4;j++)
				{
					ofstream_log<<" "<<this->valueVctVct_exemplarShapeStat[i][2][j];
				}
				ofstream_log<<endl;
			}
			this->categorizeVoxelsByValue(this->histo_page, this->Image1D_page, this->size_page);
			this->possVct_segmentationResultOriginal = this->regionGrowOnPossVector(this->possVct_segmentationSeed);
			this->possVct2Image1DC(this->possVct_segmentationResultOriginal, this->Image1D_segmentationResultOriginal);
			V3DLONG count_region = this->possVct_segmentationResultOriginal.size();
			V3DLONG count_voxel = 0;
			bool is_passedShapeStat = true;
			//bool is_passedHisto = true;
			vector<V3DLONG> poss_region;
			vector<V3DLONG> boundBox_region;
			vector<vector<double> > valuesVct_regionShapeStat;
			vector<double> values_PC1;
			vector<double> values_PC2;
			vector<double> values_PC3;
			double corr_shapeStat1 = 0;
			double corr_shapeStat2 = 0;
			double corr_shapeStat3 = 0;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			V3DLONG pos_voxel;
			int value_voxel;
			int count_label;
			//vector<double> histo_region (const_length_histogram, 0);
			V3DLONG pos_center;
			this->possVct_segmentationResultSplitted.clear();
			this->possVct_segmentationResultPassed.clear();
			this->poss_segmentationResultCenterPassed.clear();
			V3DLONG size_radius = 0;
			double value_anisotropy;
			vector<V3DLONG> xyz_center;
			ofstream_log<<"total regions: "<<count_region<<";"<<endl;
			cout<<"total regions: "<<count_region<<endl;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	
				cout<<"analyzing region["<<idx_region<<"];"<<endl;
				poss_region = this->possVct_segmentationResultOriginal[idx_region];
				count_voxel = poss_region.size();
				ofstream_log<<endl<<endl<<"idx_region("<<idx_region<<"), count_voxel: "<<count_voxel<<endl;
				//fill (histo_region.begin(), histo_region.end(), 0);
				//for (int idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				//{
				//	pos_voxel = poss_region[idx_voxel];
				//	value_voxel = this->Image1D_page[pos_voxel];
				//	//histo_region[value_voxel]++;
				//}
				/*for (int i=0;i<const_length_histogram;i++)
				{
					histo_region[i] = histo_region[i] / count_voxel;
				}*/
				//is_passedHisto = false;
				/*for (int j=0; j<this->histoVct_exemplarRegion.size(); j++)
				{
					if (compareHisto(histo_region, this->histoVct_exemplarRegion[j], this->threshold_histoSimilar))
					{
						is_passedHisto = true;
						break;
					}
				}
				if (!is_passedHisto)
				{
					{ofstream_log<<" histogram deviates too much from exemplar, removed;"<<endl;}
					continue;
				}*/
				if (count_voxel<this->threshold_regionSize)
				{ofstream_log<<" too small, removed;"<<endl; continue; }
				boundBox_region = this->getBoundBox(poss_region);
				ofstream_log<<" min_X: "<<boundBox_region[0]<<", max_X: "<<boundBox_region[1]<<", min_Y: "<<boundBox_region[2]<<", max_Y: "<<boundBox_region[3]<<", min_Z: "<<boundBox_region[4]<<", max_Z: "<<boundBox_region[5]<<endl;
				size_radius = this->getMinDimension(boundBox_region)/2;
				ofstream_log<<" size_radius: "<<size_radius<<endl;
				pos_center = this->getCenterByMass(poss_region);
				xyz_center = this->index2Coordinate(pos_center);
				x = V3DLONG(xyz_center[0] + 0.5);
				y = V3DLONG(xyz_center[1] + 0.5);
				z = V3DLONG(xyz_center[2] + 0.5);
				{ofstream_log<<"pos_center: x("<<x<<"), y("<<y<<"), z("<<z<<");"<<endl;}
				valuesVct_regionShapeStat = this->getShapeStat(x, y, z, size_radius); //consisted of 3 vectors with length 4;
				if (valuesVct_regionShapeStat.empty())
				{
					if (count_voxel<this->threshold_regionSize)
					{ ofstream_log<<" too small, removed;"<<endl; continue; }
					else
					{
						this->possVct_segmentationResultPassed.push_back(poss_region); //small but not small enough to be removed;
						this->poss_segmentationResultCenterPassed.push_back(pos_center);
						continue;
					}
				}
				values_PC1 = valuesVct_regionShapeStat[0];
				values_PC2 = valuesVct_regionShapeStat[1];
				values_PC3 = valuesVct_regionShapeStat[2];
				ofstream_log<<" values_PC1: "<<values_PC1[0]<<" "<<values_PC1[1]<<" "<<values_PC1[2]<<" "<<values_PC1[3]<<endl;
				ofstream_log<<" values_PC2: "<<values_PC2[0]<<" "<<values_PC2[1]<<" "<<values_PC2[2]<<" "<<values_PC2[3]<<endl;
				ofstream_log<<" values_PC3: "<<values_PC3[0]<<" "<<values_PC3[1]<<" "<<values_PC3[2]<<" "<<values_PC3[3]<<endl;
				for (int j=0; j<this->valueVctVct_exemplarShapeStat.size(); j++)
				{
					is_passedShapeStat = true;
					for (int m=0; m<4; m++)
					{
						value_anisotropy = valueVctVct_exemplarShapeStat[j][0][m];
						if (fabs(values_PC1[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
						{is_passedShapeStat = false; break;}
						value_anisotropy = valueVctVct_exemplarShapeStat[j][1][m];
						if (fabs(values_PC2[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
						{is_passedShapeStat = false; break;}
						value_anisotropy = valueVctVct_exemplarShapeStat[j][2][m];
						if (fabs(values_PC3[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
						{is_passedShapeStat = false; break;}
					}
					if (is_passedShapeStat) //no need to check other exemplars;
					{ break; }
				}
				if (is_passedShapeStat)
				{
					this->possVct_segmentationResultPassed.push_back(poss_region);
					this->poss_segmentationResultCenterPassed.push_back(pos_center);
					{ofstream_log<<" shape test passed;"<<endl;}
					cout<<"   region["<<idx_region<<"] passed shape test;"<<endl;
				}
				else
				{
					ofstream_log<<" shape test failed;"<<endl;
					cout<<"   region["<<idx_region<<"] failed shape test;"<<endl;
					if (count_voxel > this->uThreshold_regionSize)
					{
						//irregular and too big, removed;
						ofstream_log<<" too big, removed;"<<endl;
						cout<<"   region["<<idx_region<<"] too big, removed;"<<endl;
					}
					else
					{
						ofstream_log<<" added to possVct_segmentationResultSplitted;"<<endl;
						this->possVct_segmentationResultSplitted.push_back(poss_region);
					}
				}
			}
			this->possVct2Image1DC(this->possVct_segmentationResultPassed, this->Image1D_segmentationResultPassed);
			this->LandmarkList_segmentationResultPassed = this->indexList2LandMarkList(this->poss_segmentationResultCenterPassed);
			ofstream_log.close();
			return;
		}
		#pragma endregion

		#pragma region "control-furtherSegmentation"
		void control_furtherSegmentation(vector<double> _paras_GVF)
		{
			this->paras_GVF = _paras_GVF;
			this->Image1D_segmentationResultMerged = memory_allocate_uchar1D(this->size_page+this->size_page+this->size_page);
			this->Image1D_segmentationResultGVF = memory_allocate_uchar1D(this->size_page+this->size_page+this->size_page);
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_segmentationResultGVF[i] = 0;
				this->Image1D_segmentationResultGVF[i+size_page] = 0;
				this->Image1D_segmentationResultGVF[i+size_page+size_page] = 0;
				this->Image1D_segmentationResultMerged[i] = this->Image1D_original[i+offset_channel];
				this->Image1D_segmentationResultMerged[i+size_page] = this->Image1D_original[i+offset_channel];
				this->Image1D_segmentationResultMerged[i+size_page+size_page] = this->Image1D_original[i+offset_channel];
			}
			ofstream ofstream_log;
			ofstream_log.open ("log_furtherSegmentation.txt");
			V3DLONG count_region = this->possVct_segmentationResultSplitted.size();
			V3DLONG count_voxel = 0;
			vector<V3DLONG> poss_region;
			vector<V3DLONG> boundBox_region;
			vector<vector<V3DLONG> > possVct_GVF;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			V3DLONG pos_voxel;
			int value_voxel;
			int count_label;
			vector<V3DLONG> poss_splittedRegion;
			V3DLONG pos_center;
			this->possVct_segmentationResultGVF.clear();
			this->possVct_segmentationResultMerged.clear();
			this->poss_segmentationResultCenterGVF.clear();
			this->poss_segmentationResultCenterMerged.clear();
			vector<V3DLONG> xyz_center;
			{ofstream_log<<"total regions for GVF segmentation: "<<count_region<<";"<<endl;}
			cout<<"total regions for GVF segmentation: "<<count_region<<endl;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	
				cout<<" GVF analyzing region["<<idx_region<<"];"<<endl;
				poss_region = this->possVct_segmentationResultSplitted[idx_region];
				boundBox_region = this->getBoundBox(poss_region);
				V3DLONG size_X = boundBox_region[1]-boundBox_region[0]+1;
				V3DLONG size_Y = boundBox_region[3]-boundBox_region[2]+1;
				V3DLONG size_Z = boundBox_region[5]-boundBox_region[4]+1;
				V3DLONG min_X = boundBox_region[0];
				V3DLONG min_Y = boundBox_region[2];
				V3DLONG min_Z = boundBox_region[4];
				ofstream_log<<" region as input to GVF: size_X("<<size_X<<"), size_Y("<<size_Y<<"), size_Z("<<size_Z<<");"<<endl;
				double*** double3D_GVF= memory_allocate_double3D(size_X, size_Y, size_Z);
				int*** int3D_label = memory_allocate_int3D(size_X, size_Y, size_Z);
				this->centralizeRegion(poss_region, size_X, size_Y, size_Z, min_X, min_Y, min_Z, double3D_GVF);
				{ofstream_log<<" centralizeRegion() passed;"<<endl;}
				vector<V3DLONG> poss_centerGVF;
				int count_label = class_segmentationMain::GVF_cellSegmentation(double3D_GVF, size_X, size_Y, size_Z, this->paras_GVF, int3D_label);
				{ofstream_log<<" GVF_cellSegmentation() passed;"<<endl;}
				possVct_GVF = int3D2possVct(int3D_label, count_label, double3D_GVF, 
					size_X, size_Y, size_Z, min_X, min_Y, min_Z, this->offset_Y, this->offset_Z, 
					poss_centerGVF);
				{ofstream_log<<" int3D2possVct() passed, GVF segmented the region into "<<possVct_GVF.size()<<" sub-regions;"<<endl;}
				if (possVct_GVF.size()>0)
				{
					for (int i=0;i<possVct_GVF.size();i++)
					{
						count_voxel = possVct_GVF[i].size();
						ofstream_log<<" sub-region ("<<i+1<<"), size: "<<count_voxel<<endl;
						if (count_voxel < this->threshold_regionSize)
						{
							ofstream_log<<" too small, removed;"<<endl;
						}
						else
						{
							this->possVct_segmentationResultGVF.push_back(possVct_GVF[i]);
							this->poss_segmentationResultCenterGVF.push_back(poss_centerGVF[i]);
						}
					}
				}
				else
				{
					this->possVct_segmentationResultGVF.push_back(poss_region);
					this->poss_segmentationResultCenterGVF.push_back(pos_center);
				}
				memory_free_double3D(double3D_GVF, size_Z, size_X);
				memory_free_int3D(int3D_label, size_Z, size_X);
				possVct_GVF.clear();
			}
			ofstream_log<<"post-processing done;"<<endl;
			this->possVct_segmentationResultMerged = this->mergePossVector(this->possVct_exemplarRegion, this->possVct_segmentationResultPassed);
			this->possVct_segmentationResultMerged = this->mergePossVector(this->possVct_segmentationResultMerged, this->possVct_segmentationResultGVF);
			this->poss_segmentationResultCenterMerged = this->mergePoss(this->poss_exemplar, this->poss_segmentationResultCenterPassed);
			this->poss_segmentationResultCenterMerged = this->mergePoss(this->poss_segmentationResultCenterMerged, this->poss_segmentationResultCenterGVF);
			this->possVct2Image1DC(this->possVct_segmentationResultGVF, this->Image1D_segmentationResultGVF);
			this->possVct2Image1DC(this->possVct_segmentationResultMerged, this->Image1D_segmentationResultMerged);
			this->possVct2Image1DC(this->possVct_segmentationResultGVF, this->Image1D_segmentationResultOriginal);
			this->LandmarkList_segmentationResultGVF = this->indexList2LandMarkList(this->poss_segmentationResultCenterGVF);
			this->LandmarkList_segmentationResultMerged = this->indexList2LandMarkList(this->poss_segmentationResultCenterMerged);
			ofstream_log.close();
			return;
		}
		#pragma endregion

		#pragma region "regionGrow"
		vector<vector<V3DLONG> > regionGrowOnPossVector(vector<vector<V3DLONG> > vctList_seed)
		{
			V3DLONG idx_tmp;
			vector<vector<V3DLONG> > vctList_result;
			vector<vector<V3DLONG> > vctList_tmp;
			V3DLONG count_seedList = vctList_seed.size();
			vector<V3DLONG> vct_tmp;
			V3DLONG count_totalCoveredVoxel = 0;
			V3DLONG count_seed = 0;
			for (V3DLONG idx_region=0;idx_region<count_seedList;idx_region++)
			{
				vctList_tmp = regionGrowOnPoss(vctList_seed[idx_region]);
				for (int i=0;i<vctList_tmp.size();i++)
				{
					vct_tmp = vctList_tmp[i];
					if (!vct_tmp.empty()) {vctList_result.push_back(vct_tmp); count_totalCoveredVoxel+=vct_tmp.size();}
				}
			}
			return vctList_result;
		}

			vector<vector<V3DLONG> > regionGrowOnPoss(vector<V3DLONG> vct_seed)
			{
				V3DLONG idx_tmp;
				vector<vector<V3DLONG> > vctList_result;
				vector<V3DLONG> vct_tmp;
				V3DLONG count_seed = vct_seed.size();
				for (V3DLONG idx_seed=0;idx_seed<count_seed;idx_seed++)
				{
					idx_tmp = vct_seed[idx_seed];
					if (this->Image1D_mask[idx_tmp]>0)
					{
						vct_tmp = this->regionGrowOnPos(idx_tmp); //Image1D_mask is updating in this function;
						if (vct_tmp.size()>this->threshold_regionSize)	{vctList_result.push_back(vct_tmp);}
					}
				}
				return vctList_result;
			}

			vector<vector<V3DLONG> > regionGrowOnPoss(vector<V3DLONG> vct_seed, unsigned char* Image1D_input)
			{
				V3DLONG idx_tmp;
				vector<vector<V3DLONG> > vctList_result;
				vector<V3DLONG> vct_tmp;
				V3DLONG count_seed = vct_seed.size();
				for (V3DLONG idx_seed=0;idx_seed<count_seed;idx_seed++)
				{
					idx_tmp = vct_seed[idx_seed];
					if (this->Image1D_mask[idx_tmp]>0)
					{
						vct_tmp = this->regionGrowOnPos(idx_tmp, Image1D_input); //Image1D_mask is updating in this function;
						if (vct_tmp.size()>this->threshold_regionSize)	{vctList_result.push_back(vct_tmp);}
					}
				}
				return vctList_result;
			}

			vector<V3DLONG> regionGrowOnPos(V3DLONG idx_seed)
			{
				vector<V3DLONG> vct_empty (0, 0);
				if (!this->checkValidity(idx_seed)) //unlikely, but just in case;
				{
					return vct_empty;
				}
				if (this->Image1D_mask[idx_seed]<1) //very likely;
				{
					return vct_empty;
				}
				vector<V3DLONG> poss_result;
				vector<V3DLONG> poss_growing;
				vector<V3DLONG> xyz_current;
				V3DLONG pos_current;
				V3DLONG pos_neighbor;
				double value_neighbor;
				double value_current;
				V3DLONG count_totalVolume = 1;
				double value_seed = this->Image1D_page[idx_seed];
				double mean_region = value_seed;
				double sum_region = value_seed;
				mean_region = value_seed;
				poss_growing.push_back(idx_seed);
				poss_result.push_back(idx_seed);
				this->Image1D_mask[idx_seed] = 0; //scooped;
				while (true)
				{
					if (poss_growing.empty()) //growing complete;
					{
						return poss_result;
					}
					pos_current = poss_growing.back();
					poss_growing.pop_back();
					value_current = this->Image1D_page[pos_current];
					xyz_current = this->index2Coordinate(pos_current);
					for (int j=0;j<const_count_neighbors;j++)
					{
						if (((xyz_current[0]+point_neighborRelative[j].x)<0)||((xyz_current[0]+point_neighborRelative[j].x)>=this->dim_X)||((xyz_current[1]+point_neighborRelative[j].y)<0) || ((xyz_current[1]+point_neighborRelative[j].y)>=this->dim_Y)||((xyz_current[2]+point_neighborRelative[j].z)<0) || ((xyz_current[2]+point_neighborRelative[j].z)>=this->dim_Z))
						{
							//invalide anyway;
						}
						else
						{
							pos_neighbor = pos_current+poss_neighborRelative[j];

							if (this->checkValidity(pos_neighbor)) //prevent it from going out of bounds;
							{
								if (this->Image1D_mask[pos_neighbor]>0) //available only;
								{
									value_neighbor = this->Image1D_page[pos_neighbor];
									if ((fabs(value_neighbor-value_current))<(mean_region*this->uThreshold_valueChangeRatio))
									{
										this->Image1D_mask[pos_neighbor] = 0; //scooped;
										poss_growing.push_back(pos_neighbor);
										poss_result.push_back(pos_neighbor);
										sum_region += value_neighbor;
										count_totalVolume++;
										mean_region = sum_region/count_totalVolume; //update mean everytime a new voxel is added;
									}
								}
							}
						}
						
					}
				}
			}

			vector<V3DLONG> regionGrowOnPos(V3DLONG idx_seed, unsigned char* Image1D_input)
				//assuming [0-const_max_voxelValue] integer Image1D_page, should also work on binary image (not tested yet);
			{
				vector<V3DLONG> idxs_empty (0, 0);
				if (!this->checkValidity(idx_seed)) //unlikely, but just in case;
				{
					return idxs_empty;
				}
				if (this->Image1D_mask[idx_seed]<1) //very likely;
				{
					return idxs_empty;
				}
				vector<V3DLONG> vct_result;
				vector<V3DLONG> vct_temp;
				V3DLONG idx_current;
				V3DLONG idx_neighbor;
				double double_neightborValue;
				double double_currentValue;
				double double_seedValue = Image1D_input[idx_seed];
				vct_temp.push_back(idx_seed);
				vct_result.push_back(idx_seed);
				this->Image1D_mask[idx_seed] = 0; //scooped;
				while (true)
				{
					if (vct_temp.empty()) //growing complete;
					{
						return vct_result;
					}
					idx_current = vct_temp.back();
					vct_temp.pop_back();
					double_currentValue = Image1D_input[idx_current];
					for (int j=0;j<const_count_neighbors;j++)
					{
						idx_neighbor = idx_current+poss_neighborRelative[j];
						if (this->checkValidity(idx_neighbor)) //prevent it from going out of bounds;
						{
							if (this->Image1D_mask[idx_neighbor]>0) //available only;
							{
								double_neightborValue = Image1D_input[idx_neighbor];
								if (double_neightborValue>0)
								{
									this->Image1D_mask[idx_neighbor] = 0; //scooped;
									vct_temp.push_back(idx_neighbor);
									vct_result.push_back(idx_neighbor);
								}
							}
						}
					}
				}
			}
			#pragma endregion		

		#pragma region "exemplar" 
		bool analyzeExemplarRegion()
		{
			V3DLONG pos_current = 0;
			int value_current=0;
			vector<V3DLONG> poss_region;
			V3DLONG count_region = this->possVct_exemplarRegion.size();
			V3DLONG count_voxel = 0;
			//double mean_neighborValueChange = 0;
			//V3DLONG pos_neighbor = 0;
			//this->histoVct_exemplarRegion.clear();
			//vector<double> histo_region (const_length_histogram, 0);
			//V3DLONG count_neighbor;
			//double mean_regionValue = 0;
			//double max_ratioValueChagne = -INF;
			//double ratio_valueChange = 0;
			V3DLONG size_radius = 0;
			vector<vector<double> > valuesVct_shapeStat;
			vector<V3DLONG> boundBox_region;
			this->valueVctVct_exemplarShapeStat.clear();
			this->threshold_regionSize = INF;
			this->uThreshold_regionSize = -INF;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				cout<<"exemplar region: "<<(idx_region+1)<<endl;
				//fill (histo_region.begin(), histo_region.end(), 0);
				poss_region = this->possVct_exemplarRegion[idx_region];
				count_voxel = poss_region.size();
				if (this->threshold_regionSize>count_voxel*this->multiplier_thresholdRegionSize) {this->threshold_regionSize=(count_voxel*this->multiplier_thresholdRegionSize);}
				if (this->uThreshold_regionSize<count_voxel*this->multiplier_uThresholdRegionSize) {this->uThreshold_regionSize=(count_voxel*this->multiplier_uThresholdRegionSize);}
				//mean_regionValue=0;
				/*for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					pos_current = poss_region[idx_voxel];
					if (checkValidity(pos_current))
					{
						value_current = this->Image1D_page[pos_current];
						mean_regionValue += value_current;
						histo_region[value_current]++;
					}
				}*/
				/*for (int i=0;i<const_length_histogram;i++)
				{
					histo_region[i] = histo_region[i]/count_voxel;
				}*/
				//this->histoVct_exemplarRegion.push_back(histo_region);
				//mean_regionValue = mean_regionValue/count_voxel;
				/*for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					cout<<"   idx_voxel: "<<(idx_voxel+1)<<endl;
					pos_current = poss_region[idx_voxel];
					if (checkValidity(pos_current))
					{
						value_current = this->Image1D_page[pos_current];
						mean_neighborValueChange = 0;
						count_neighbor = 0;
						max_ratioValueChagne = -INF;
						for (int i=0;i<const_count_neighbors;i++)
						{
							pos_neighbor = pos_current + this->poss_neighborRelative[i];
							if (checkValidity(pos_neighbor))
							{
								if (vctContains(poss_region, pos_neighbor)>-1)
								{
									mean_neighborValueChange += fabs((double)(value_current-this->Image1D_page[pos_neighbor]));
									count_neighbor++;
								}
							}
						}
						if (count_neighbor>0)
						{
							mean_neighborValueChange = mean_neighborValueChange/count_neighbor;
							ratio_valueChange = mean_neighborValueChange/mean_regionValue;
							if (max_ratioValueChagne<ratio_valueChange)
							{
								max_ratioValueChagne = ratio_valueChange;
							}
						}
					}
				}*/
				//if (this->uThreshold_valueChangeRatio<max_ratioValueChagne) {this->uThreshold_valueChangeRatio=max_ratioValueChagne;}
	
				//shape property;
				boundBox_region = this->getBoundBox(poss_region);
				size_radius = getMinDimension(boundBox_region)/2;
				V3DLONG pos_center = this->getCenterByMass(poss_region);
				vector<V3DLONG> xyz_center = this->index2Coordinate(pos_center);
				valuesVct_shapeStat = this->getShapeStat(xyz_center[0], xyz_center[1], xyz_center[2], size_radius);
				this->valueVctVct_exemplarShapeStat.push_back(valuesVct_shapeStat);
			}
			if (this->threshold_regionSize<default_threshold_regionSize)
			{
				this->threshold_regionSize = default_threshold_regionSize;
			}
			return true;
		}
		#pragma endregion
		
		#pragma region "threshold estimation"
		void categorizeVoxelsByValue(vector<double> vct_histoInput, unsigned char* Image1D_input, V3DLONG count_imageSize)
			//will only consider voxels with value higher than threshold_global;
		{
			this->possVct_segmentationSeed.clear();
			vector<V3DLONG> idxs_empty (0,0);
			int int_threshold = (int)this->threshold_global;
			for (int i=int_threshold;i<const_length_histogram;i++)
			{
				this->possVct_segmentationSeed.push_back(idxs_empty);
			}
			int int_valueOffset=0;
			int int_valueTmp=0;
			for (int i=0;i<count_imageSize;i++)
			{
				int_valueTmp = (int)Image1D_input[i];
				int_valueOffset = const_max_voxelValue-int_valueTmp;
				if (int_valueTmp>int_threshold)
				{
					possVct_segmentationSeed[int_valueOffset].push_back(i);
				}
			}
		}

		void estimateThreshold()
		{
			double threshold_Yen;
			double threshold_Ostu;
			this->histo_page = class_segmentationMain::getHistogram(this->Image1D_page, this->size_page);
			threshold_Yen = class_segmentationMain::estimateThresholdYen(histo_page);
			//threshold_Ostu = class_segmentationMain::estimateThresholdOtsu(histo_page);
			//if (threshold_Ostu<threshold_Yen) {this->threshold_global = threshold_Ostu;}
			//else {this->threshold_global = threshold_Yen;}
			this->threshold_global = threshold_Yen;
			
			if (this->threshold_global < default_threshold_global)
			{
				v3d_msg("Warning: threshold estimation failed, will use default threshold (10) instead!");
				this->threshold_global = default_threshold_global;
			}
		}

		static int estimateThresholdYen(double* histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;

			int value_threshold;
			int ih, it;
			double crit;
			double max_crit;
			double* norm_histo = new double[const_length_histogram];
			double* P1 = new double[const_length_histogram];
			double* P1_sq = new double[const_length_histogram]; 
			double* P2_sq = new double[const_length_histogram]; 
			int total =0;
			for (ih = 0; ih < const_length_histogram; ih++ ) 
				total+=histo_input[ih];
			for (ih = 0; ih < const_length_histogram; ih++ )
				norm_histo[ih] = (double)histo_input[ih]/total;
			P1[0]=norm_histo[0];
			for (ih = 1; ih < const_length_histogram; ih++ )
				P1[ih]= P1[ih-1] + norm_histo[ih];
			P1_sq[0]=norm_histo[0]*norm_histo[0];
			for (ih = 1; ih < const_length_histogram; ih++ )
				P1_sq[ih]= P1_sq[ih-1] + norm_histo[ih] * norm_histo[ih];
			P2_sq[const_length_histogram-1] = 0.0;
			for ( ih = const_max_voxelValue-1; ih >= 0; ih-- )
				P2_sq[ih] = P2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];
			value_threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < const_length_histogram; it++ ) {
				crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
				if ( crit > max_crit ) {
					max_crit = crit;
					value_threshold = it;
				}
			}
			return value_threshold;
		}

		static int estimateThresholdYen(vector<double> histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;
			
			int value_threshold;
			int ih, it;
			double crit;
			double max_crit;
			double* P1 = new double[const_length_histogram];
			double* P1_sq = new double[const_length_histogram]; 
			double* P2_sq = new double[const_length_histogram]; 
			P1[0]=histo_input[0];
			for (ih = 1; ih < const_length_histogram; ih++ )
				P1[ih]= P1[ih-1] + histo_input[ih];
			P1_sq[0]=histo_input[0]*histo_input[0];
			for (ih = 1; ih < const_length_histogram; ih++ )
				P1_sq[ih]= P1_sq[ih-1] + histo_input[ih] * histo_input[ih];
			P2_sq[const_length_histogram-1] = 0.0;
			for ( ih = const_max_voxelValue-1; ih >= 0; ih-- )
				P2_sq[ih] = P2_sq[ih + 1] + histo_input[ih + 1] * histo_input[ih + 1];
			value_threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < const_length_histogram; it++ ) {
				crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
				if ( crit > max_crit ) {
					max_crit = crit;
					value_threshold = it;
				}
			}
			return value_threshold;
		}

		static int estimateThresholdOtsu(double* histo_input)
		{
			int i, value_threshold;
			float s,ut,uk,wk,max;
			float b,q;
			s = 0.0;
			for(i=0;i<const_length_histogram;i++)
			{
				s += histo_input[i];
			}
			for(i=0;i<const_length_histogram;i++)
			{
				histo_input[i] = histo_input[i]/s;
			}
			ut = 0.0;
			for(i=0;i<const_length_histogram;i++)
			{
				ut = ut + i*histo_input[i];
			}
			uk = 0.0; wk = 0.0; max = 0.0;
			for(i=0;i<const_length_histogram;i++)
			{
				wk = wk + histo_input[i];
				uk = uk + i*histo_input[i];
				b = ut * wk - uk;
				q = wk * (1.0-wk);
				if(q<const_infinitesimal) continue;
				b = b*b/q;
				if(b > max)
				{
					max = b;
					value_threshold = i;
				}
			}
			return value_threshold;
		}

		static int estimateThresholdOtsu(vector<double> histo_input)
		{
			int i, value_threshold;
			float s,ut,uk,wk,max;
			float b,q;
			ut = 0.0;
			for(i=0;i<const_length_histogram;i++)
			{
				ut = ut + i*histo_input[i];
			}
			uk = 0.0; wk = 0.0; max = 0.0;
			for(i=0;i<const_length_histogram;i++)
			{
				wk = wk + histo_input[i];
				uk = uk + i*histo_input[i];
				b = ut * wk - uk;
				q = wk * (1.0-wk);
				if(q<const_infinitesimal) continue;
				b = b*b/q;
				if(b > max)
				{
					max = b;
					value_threshold = i;
				}
			}
			return value_threshold;
		}

		static vector<double> getHistogram(unsigned char* ImageID_input, V3DLONG length_input)
		{
			vector<double> histo_result (const_length_histogram, 0);
			V3DLONG value_voxel;
			for (V3DLONG i=0;i<length_input;i++)
			{
				value_voxel = (int)ImageID_input[i];
				histo_result[value_voxel] = histo_result[value_voxel]+1;
			}
			for (int i=0;i<const_length_histogram;i++)
			{
				histo_result[i] = histo_result[i]/length_input;
			}
			return histo_result;
		}

		vector<double> getHistogram(vector<V3DLONG> poss_input)
		{
			vector<double> histo_result (const_length_histogram, 0);
			V3DLONG value_voxel;
			V3DLONG count_voxel = poss_input.size();
			for (V3DLONG i=0;i<count_voxel;i++)
			{
				value_voxel = (int)this->Image1D_page[poss_input[i]];
				histo_result[value_voxel] = histo_result[value_voxel]+1;
			}
			for (int i=0;i<const_length_histogram;i++)
			{
				histo_result[i] = histo_result[i]/count_voxel;
			}
			return histo_result;
		}
		
		V3DLONG thresholdForCurrentPage()
		{
			V3DLONG count_totalWhite = 0;
			for(V3DLONG i=0; i<this->size_page; i++)
			{	
				if ((double)Image1D_page[i]>this->threshold_global)
				{
					//do nothing to Image1D_page;
					count_totalWhite = count_totalWhite + 1;
					Image1D_mask[i] = const_max_voxelValue; //available;
				}
				else
				{
					Image1D_page[i] = 0;
					Image1D_mask[i] = 0; //invalid;
				}
			}
			return count_totalWhite;
		}

		bool compareHisto(vector<double> histo_input1, vector<double> histo_input2, double threshold_similar)
		{
			bool is_similar = false;
			double value_similar = this->getCorrelation(histo_input1, histo_input2);
			if (value_similar>=threshold_similar)
			{
				is_similar = true;
			}
			return is_similar;
		}
		#pragma endregion

		#pragma region "utility functions"
		static void Image3D2Image1D(int*** Image3D_input, unsigned char* Image1D_output, const int size_X, const int size_Y, const int size_Z)
		{
			int tmp_value = 0;
			int tmp_idx = 0;
			for (int z=0;z<size_Z;z++)
			{
				for (int x=0;x<size_X;x++)
				{
					for (int y=0;y<size_Y;y++)
					{
						tmp_value = Image3D_input[z][x][y];
						tmp_idx = class_segmentationMain::coordinate2Index(x, y, z, size_X, size_X*size_Y);
						Image1D_output[tmp_idx] = tmp_value;
					}
				}
			}
			return ;
		}

		void Image1D2Image3D(const unsigned char* Image1D_input, double*** Image3D_output, const int dim_X, const int dim_Y, const int dim_Z)
		{
			vector<V3DLONG> vct_coordinate;
			int count_page = dim_X*dim_Y*dim_Z;
			for (int i=0;i<count_page;i++)
			{
				vct_coordinate = index2Coordinate(i);
				Image3D_output[vct_coordinate[2]][vct_coordinate[0]][vct_coordinate[1]] = Image1D_input[i];
			}
			return ;
		}

		bool checkValidity(V3DLONG idx_input)
		{
			if ((idx_input>=0)&&(idx_input<this->size_page))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		vector<vector<V3DLONG> > mergePossVector(vector<vector<V3DLONG> > vctList_input1, vector<vector<V3DLONG> > vctList_input2) //vctList_input2 will be appended to vctList_input1;
		{
			vector<vector<V3DLONG> > vctList_result = vctList_input1;
			vector<V3DLONG> vct_tmp;
			V3DLONG count_region2 = vctList_input2.size();
			for (int i=0;i<count_region2;i++)
			{
				vct_tmp = vctList_input2[i];
				vctList_result.push_back(vct_tmp);
			}
			return vctList_result;
		}

		vector<V3DLONG> mergePoss(vector<V3DLONG> poss_input1, vector<V3DLONG> poss_input2) 
		{
			vector<V3DLONG> poss_result = poss_input1;
			V3DLONG count_pos = poss_input2.size();
			for (int i=0;i<count_pos;i++)
			{
				poss_result.push_back(poss_input2[i]);
			}
			return poss_result;
		}

		int removeSingleVoxel(unsigned char* Image1D_input, V3DLONG count_imageSize)
		{
			bool flag_remove = true;
			double double_currentValue = 0;
			double double_neightborValue = 0;
			V3DLONG idx_neighbor = 0;
			int count_removedVoxel = 0;
			for (int i=0;i<count_imageSize;i++)
			{
				double_currentValue = Image1D_input[i];
				if (double_currentValue>this->threshold_global) //valid voxel;
				{
					flag_remove = true;
					for (int j=0;j<const_count_neighbors;j++)
					{
						idx_neighbor = i+poss_neighborRelative[j];
						double_neightborValue = Image1D_input[idx_neighbor];
						if(this->checkValidity(idx_neighbor)) //prevent it from going out of bounds;
						{
							double_neightborValue = Image1D_input[idx_neighbor];
							if (double_neightborValue>this->threshold_global)
							{
								flag_remove = false; //it has neighbor;
								break;
							}
						}
					}
					if (flag_remove)
					{
						Image1D_input[i] = 0; //remove it;
						count_removedVoxel++;
						this->count_totalWhiteVoxel--;
					}
				}
			}
			return count_removedVoxel;
		}

		LandmarkList indexList2LandMarkList(vector<V3DLONG> vct_index)
		{
			LandmarkList LandmarkList_result;
			LocationSimple Landmark_tmp;
			for (int i=0;i<vct_index.size();i++)
			{
				Landmark_tmp=index2LandMark(vct_index[i]);
				LandmarkList_result.push_back(Landmark_tmp);
			}
			return LandmarkList_result;
		}

		LocationSimple index2LandMark(V3DLONG idx_Input)
		{
			vector<V3DLONG> vct_coordinate = index2Coordinate(idx_Input);
			V3DLONG x=vct_coordinate[0];
			V3DLONG y=vct_coordinate[1];
			V3DLONG z=vct_coordinate[2];
			LocationSimple LocationSimple_result (x, y, z);
			return LocationSimple_result;
		}

		vector<V3DLONG> landMarkList2IndexList(LandmarkList LandmarkList_input)
		{
			vector<V3DLONG> vct_result;
			for (V3DLONG idx_input=0;idx_input<LandmarkList_input.count();idx_input++)
			{
				vct_result.push_back(landMark2Index(LandmarkList_input.at(idx_input)));
			}
			return vct_result;
		}

		V3DLONG landMark2Index(LocationSimple Landmark_input)
		{
			float x=0;
			float y=0;
			float z=0;
			Landmark_input.getCoord(x, y, z);
			return (coordinate2Index(x-1, y-1, z-1));
		}

		vector<V3DLONG> index2Coordinate(V3DLONG idx)
		{
			vector<V3DLONG> vct_result (3, -1);
			vct_result[2] = floor((double)idx/(double)offset_Z);
			vct_result[1] = floor((double)(idx-vct_result[2]*offset_Z)/(double)offset_Y);
			vct_result[0] = idx- vct_result[2]*offset_Z-vct_result[1]*offset_Y;
			return vct_result;
		}

		static vector<V3DLONG> index2Coordinate(V3DLONG idx, V3DLONG offset_Y, V3DLONG offset_Z)
		{
			vector<V3DLONG> vct_result (3, -1);
			vct_result[2] = floor((double)idx/(double)offset_Z);
			vct_result[1] = floor((double)(idx-vct_result[2]*offset_Z)/(double)offset_Y);
			vct_result[0] = idx- vct_result[2]*offset_Z-vct_result[1]*offset_Y;
			return vct_result;
		}

		V3DLONG coordinate2Index(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return z*this->offset_Z+y*this->offset_Y+x;
		}

		static V3DLONG coordinate2Index(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG offset_Y, V3DLONG offset_Z)
		{
			return z*offset_Z+y*offset_Y+x;
		}

		void createNeighborVector()
		{
			this->poss_neighborRelative.clear();
			double3D point_neighbor;
			for (V3DLONG z=-1;z<=1;z++)
			{
				for (V3DLONG y=-1;y<=1;y++)
				{
					for (V3DLONG x=-1;x<=1;x++)
					{
						if (x==0&&y==0&&z==0)
						{
							//that's itself;
						}
						else
						{
							this->poss_neighborRelative.push_back(z*this->offset_Z+y*this->offset_Y+x);
							point_neighbor.x=x; point_neighbor.y=y; point_neighbor.z=z;
							this->point_neighborRelative.push_back(point_neighbor);
						}
					}
				}
			}
		}

		void possVct2Image1DC(vector<vector<V3DLONG> > possVct_input, unsigned char* Image1D_input)
		{
			vector<V3DLONG> color_input (3, 0);
			for (int i=0;i<possVct_input.size();i++)
			{
				int idx_color = i%12;
				color_input[0] = colors_simpleTable[idx_color][0];
				color_input[1] = colors_simpleTable[idx_color][1];
				color_input[2] = colors_simpleTable[idx_color][2];
				poss2Image1DC(Image1D_input, possVct_input[i], color_input);
			}
		}

		void poss2Image1DC(unsigned char* Image1D_input, vector<V3DLONG> poss_input, vector<V3DLONG> color_input)
		{
			for (int i=0;i<poss_input.size();i++)
			{
				if (this->checkValidity(poss_input[i]))
				{
					Image1D_input[poss_input[i]] = color_input[0];
					Image1D_input[poss_input[i]+this->size_page] = color_input[1];
					Image1D_input[poss_input[i]+this->size_page+this->size_page] = color_input[2];
				}
			}
		}

		static void neuronTree2LandmarkList(const NeuronTree & NeuronTree_input, LandmarkList & LandmarkList_output)
		{
			LocationSimple LocationSimple_temp(0,0,0);
			for (V3DLONG i=0;i<NeuronTree_input.listNeuron.size();i++)
			{
				LocationSimple_temp.x = NeuronTree_input.listNeuron.at(i).x;
				LocationSimple_temp.y = NeuronTree_input.listNeuron.at(i).y;
				LocationSimple_temp.z = NeuronTree_input.listNeuron.at(i).z;
				LandmarkList_output.append(LocationSimple_temp);
			}
		}

		static V3DLONG vctContains(vector<V3DLONG> vct_input, V3DLONG idx_input)
		{
			for (int i=0;i<vct_input.size();i++)
			{
				if (vct_input[i] == idx_input)
				{
					return i;
				}
			}
			return -1;
		}
		#pragma endregion

		#pragma region "sorting and comparison"
		double getMax(vector<double> values_input)
		{
			double max_result = -INF;
			for (std::vector<double>::iterator it = values_input.begin() ; it != values_input.end(); ++it)
			{
				if (max_result<*it) {max_result = *it;}
			}
			return max_result;
		}

		double getMin(vector<double> values_input)
		{
			double min_result = -INF;
			for (std::vector<double>::iterator it = values_input.begin() ; it != values_input.end(); ++it)
			{
				if (min_result<*it) {min_result = *it;}
			}
			return min_result;
		}
		
		#pragma endregion

		#pragma region "geometry property"
		static vector<V3DLONG> getOffset(const V3DLONG dim_X, const V3DLONG dim_Y, const V3DLONG dim_Z)
		{
			vector<V3DLONG> size_result (2, 0);
			size_result[0] = dim_X*dim_Y;
			size_result[1] = dim_X;
			return size_result;
		}

		double getEuclideanDistance(V3DLONG idx_input1, V3DLONG idx_input2)
		{
			double result = 0;
			if((idx_input1==INF)||(idx_input2==INF)) {return INF;}
			vector<V3DLONG> vct_xyz1 = this->index2Coordinate(idx_input1);
			vector<V3DLONG> vct_xyz2 = this->index2Coordinate(idx_input2);
			result+=(vct_xyz1[0]-vct_xyz2[0])*(vct_xyz1[0]-vct_xyz2[0]);
			result+=(vct_xyz1[1]-vct_xyz2[1])*(vct_xyz1[1]-vct_xyz2[1]);
			result+=(vct_xyz1[2]-vct_xyz2[2])*(vct_xyz1[2]-vct_xyz2[2]);
			result = sqrt(result);
			return result;
		}

		static double getEuclideanDistance(double3D point_input1, double3D point_input2)
		{
			double result = 0;
			result+=(point_input1.x-point_input2.x)*(point_input1.x-point_input2.x);
			result+=(point_input1.y-point_input2.y)*(point_input1.y-point_input2.y);
			result+=(point_input1.z-point_input2.z)*(point_input1.z-point_input2.z);
			result = sqrt(result);
			return result;
		}

		void centralizeRegion(const vector<V3DLONG> poss_input, const V3DLONG size_X, const V3DLONG size_Y, const V3DLONG size_Z, 
			const V3DLONG min_X, const V3DLONG min_Y, const V3DLONG min_Z, unsigned char* Image1D_output)
		{
			V3DLONG pos_voxel = 0;
			V3DLONG pos_centralized = 0;
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			vector<V3DLONG> xyz_voxel (0, 0);
			V3DLONG count_voxel = poss_input.size();
			V3DLONG size_region = size_X*size_Y*size_Z;
			for (int i=0;i<size_region;i++)
			{
				Image1D_output[i] = 0;
			}
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				pos_voxel = poss_input[idx_voxel];
				xyz_voxel = this->index2Coordinate(pos_voxel);
				x = xyz_voxel[0] - min_X;
				y = xyz_voxel[1] - min_Y;
				z = xyz_voxel[2] - min_Z;
				pos_centralized = class_segmentationMain::coordinate2Index(x, y, z, size_X, size_X*size_Y);
				Image1D_output[pos_centralized] = (int) this->Image1D_page[pos_voxel];
			}
			return;
		}

		void centralizeRegion(const vector<V3DLONG> poss_input, const V3DLONG size_X, const V3DLONG size_Y, const V3DLONG size_Z, 
			const V3DLONG min_X, const V3DLONG min_Y, const V3DLONG min_Z, double*** Image3D_output)
		{
			V3DLONG pos_voxel = 0;
			vector<V3DLONG> xyz_voxel (0, 0);
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG count_voxel = poss_input.size();
			for (z=0;z<size_Z;z++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (x=0;x<size_X;x++)
					{
						Image3D_output[z][x][y] = 0;
					}
				}
			}
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				pos_voxel = poss_input[idx_voxel];
				xyz_voxel = this->index2Coordinate(pos_voxel);
				x = xyz_voxel[0] - min_X;
				y = xyz_voxel[1] - min_Y;
				z = xyz_voxel[2] - min_Z;
				Image3D_output[z][x][y] = (double) this->Image1D_page[pos_voxel];
			}
			return;
		}

		V3DLONG getCenterByMass(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> xyz_voxel;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			double sum_X=0;
			double sum_Y=0;
			double sum_Z=0;
			double sum_mass = 0;
			double value_voxel = 0;
			V3DLONG count_voxel = vct_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				xyz_voxel = this->index2Coordinate(vct_input[i]);
				value_voxel = this->Image1D_page[vct_input[i]];
				x=xyz_voxel[0];
				y=xyz_voxel[1];
				z=xyz_voxel[2];
				sum_X += (double)x*value_voxel;
				sum_Y += (double)y*value_voxel;
				sum_Z += (double)z*value_voxel;
				sum_mass += value_voxel;
			}
			V3DLONG pos_result = this->coordinate2Index(sum_X/sum_mass, sum_Y/sum_mass, sum_Z/sum_mass);
			return pos_result;
		}

		vector<V3DLONG> getBoundBox(vector<V3DLONG> idxs_input) //output: min_X, max_X, min_Y, max_Y, min_Z, max_Z;
		{
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG max_X = -INF;
			V3DLONG max_Y = -INF;
			V3DLONG max_Z = -INF;
			V3DLONG min_X = INF;
			V3DLONG min_Y = INF;
			V3DLONG min_Z = INF;
			V3DLONG count_voxel = idxs_input.size();
			vector<V3DLONG> xyz_voxel (3, 0);
			vector<V3DLONG> values_result (6, 0);
			V3DLONG idx_tmp;
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				idx_tmp = idxs_input[idx_voxel];
				xyz_voxel = this->index2Coordinate(idx_tmp);
				x = xyz_voxel[0];
				y = xyz_voxel[1];
				z = xyz_voxel[2];
				if (x>max_X) {max_X=x;}
				if (y>max_Y) {max_Y=y;}
				if (z>max_Z) {max_Z=z;}
				if (x<min_X) {min_X=x;}
				if (y<min_Y) {min_Y=y;}
				if (z<min_Z) {min_Z=z;}
			}
			values_result[0]=min_X;
			values_result[1]=max_X;
			values_result[2]=min_Y;
			values_result[3]=max_Y;
			values_result[4]=min_Z;
			values_result[5]=max_Z;
			return values_result;
		}

		V3DLONG getMinDimension(vector<V3DLONG> vct_input) //input: min_X, max_X, min_Y, max_Y, min_Z, max_Z;
		{
			V3DLONG size_X = vct_input[1] - vct_input[0];
			V3DLONG size_Y = vct_input[3] - vct_input[2];
			V3DLONG size_Z = vct_input[5] - vct_input[4];
			if (size_X < size_Y) {size_X=size_Y;}
			if (size_X < size_Z) {size_X=size_Z;}
			return size_X;
		}

		static vector<vector<V3DLONG> > int1D2possVct(int* Image1D_label, int count_label, unsigned char* Image1D_image, 
			const V3DLONG size_X, const V3DLONG size_Y, const V3DLONG size_Z, 
			const V3DLONG min_X, const V3DLONG min_Y, const V3DLONG min_Z, const V3DLONG offset_Yglobal, const V3DLONG offset_Zglobal, 
			const double min_centerDistance, vector<V3DLONG> &poss_center, const double threshold_regionSize)
		{
			int label_voxel = 0;
			V3DLONG pos_voxel = 0;
			double value_voxel = 0;
			vector<V3DLONG> vct_empty(0, 0);
			vector<vector<V3DLONG> > possVct_result;
			vector<vector<V3DLONG> > possVct_resultWithEmpty;
			for (int i=0;i<count_label;i++)
			{
				possVct_resultWithEmpty.push_back(vct_empty);
			}
			double3D xyz_zero;
			vector<double3D> mean_center;
			for (int i=0;i<count_label;i++)
			{
				mean_center.push_back(xyz_zero);
			}
			vector<int> idxs_remap (count_label, 0);
			V3DLONG label_remap = 0;
			V3DLONG x=0; V3DLONG y=0; V3DLONG z=0;
			V3DLONG offset_Y = size_X; V3DLONG offset_Z = size_X*size_Y;
			vector<double> sums_mass (count_label, 0);
			for (x=0;x<size_X;x++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (z=0;z<size_Z;z++)
					{
						pos_voxel = class_segmentationMain::coordinate2Index(x, y, z, offset_Y, offset_Z);
						label_voxel = Image1D_label[pos_voxel];
						if (label_voxel > 0)
						{
							label_voxel = label_voxel-1;
							value_voxel = Image1D_image[pos_voxel];
							mean_center[label_voxel].x += value_voxel*x; mean_center[label_voxel].y += value_voxel*y; mean_center[label_voxel].z += value_voxel*z;
							sums_mass[label_voxel] += value_voxel;
						}
					}
				}
			}
			for (int i=0;i<count_label;i++)
			{
				idxs_remap[i] = i;
				if (sums_mass[i]>0) {mean_center[i].x/=sums_mass[i]; mean_center[i].y/=sums_mass[i]; mean_center[i].z/=sums_mass[i];}
			}
			for (int i=0;i<count_label;i++)
			{
				if (sums_mass[i]>0)
				{
					for (int j=(i+1);j<count_label;j++)
					{
						if (sums_mass[j]>0)
						{
							if (class_segmentationMain::getEuclideanDistance(mean_center[i], mean_center[j])<min_centerDistance)
							{
								if (idxs_remap[j] == j)
								{
									idxs_remap[j] = i;
								}
							}
						}
					}
				}
			}
			mean_center.clear();
			for (int i=0;i<count_label;i++)
			{
				mean_center.push_back(xyz_zero);
			}
			fill(sums_mass.begin(), sums_mass.end(), 0);
			for (x=0;x<size_X;x++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (z=0;z<size_Z;z++)
					{
						pos_voxel = class_segmentationMain::coordinate2Index(x, y, z, offset_Y, offset_Z);
						label_voxel = Image1D_label[pos_voxel];
						if (label_voxel > 0)
						{
							label_voxel = label_voxel-1;
							value_voxel = Image1D_image[pos_voxel];
							label_remap = idxs_remap[label_voxel];
							mean_center[label_remap].x += value_voxel*x; mean_center[label_remap].y += value_voxel*y; mean_center[label_remap].z += value_voxel*z;
							sums_mass[label_remap] += value_voxel;
							pos_voxel = class_segmentationMain::coordinate2Index(x+min_X, y+min_Y, z+min_Z, offset_Yglobal, offset_Zglobal);
							possVct_resultWithEmpty[label_remap].push_back(pos_voxel);
						}
					}
				}
			}
			for (int i=0;i<count_label;i++)
			{
				if (sums_mass[i]>0) {mean_center[i].x/=sums_mass[i]; mean_center[i].y/=sums_mass[i]; mean_center[i].z/=sums_mass[i];}
			}
			for (int i=0;i<count_label;i++)
			{
				if ((sums_mass[i]>0) && ((possVct_resultWithEmpty[i].size()>threshold_regionSize)))
				{
					possVct_result.push_back(possVct_resultWithEmpty[i]);
					poss_center.push_back(class_segmentationMain::coordinate2Index(mean_center[i].x+min_X, mean_center[i].y+min_Y, mean_center[i].z+min_Z, offset_Yglobal, offset_Zglobal));
				}
			}
			return possVct_result;
		}

		/*static vector<vector<V3DLONG> > int3D2possVct(int*** int3D_label, int count_label, double*** Image3D_image, 
			const V3DLONG size_X, const V3DLONG size_Y, const V3DLONG size_Z, 
			const V3DLONG min_X, const V3DLONG min_Y, const V3DLONG min_Z, const V3DLONG offset_Yglobal, const V3DLONG offset_Zglobal, 
			const double min_centerDistance, vector<V3DLONG> &poss_center, const double threshold_regionSize)
		{
			int label_voxel = 0;
			V3DLONG pos_voxel = 0;
			double value_voxel = 0;
			vector<V3DLONG> vct_empty(0, 0);
			vector<vector<V3DLONG> > possVct_result;
			vector<vector<V3DLONG> > possVct_resultWithEmpty;
			for (int i=0;i<count_label;i++)
			{
				possVct_resultWithEmpty.push_back(vct_empty);
			}
			double3D xyz_zero;
			vector<double3D> mean_center;
			for (int i=0;i<count_label;i++)
			{
				mean_center.push_back(xyz_zero);
			}
			vector<int> idxs_remap (count_label, 0);
			V3DLONG label_remap = 0;
			V3DLONG x=0; V3DLONG y=0; V3DLONG z=0;
			V3DLONG offset_Y = size_X; V3DLONG offset_Z = size_X*size_Y;
			vector<double> sums_mass (count_label, 0);
			for (x=0;x<size_X;x++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (z=0;z<size_Z;z++)
					{
						label_voxel = int3D_label[z][x][y];
						if (label_voxel > 0)
						{
							label_voxel = label_voxel-1;
							value_voxel = Image3D_image[z][x][y];
							mean_center[label_voxel].x += value_voxel*x; mean_center[label_voxel].y += value_voxel*y; mean_center[label_voxel].z += value_voxel*z;
							sums_mass[label_voxel] += value_voxel;
						}
					}
				}
			}
			for (int i=0;i<count_label;i++)
			{
				idxs_remap[i] = i;
				if (sums_mass[i]>0) {mean_center[i].x/=sums_mass[i]; mean_center[i].y/=sums_mass[i]; mean_center[i].z/=sums_mass[i];}
			}
			for (int i=0;i<count_label;i++)
			{
				if (sums_mass[i]>0)
				{
					for (int j=(i+1);j<count_label;j++)
					{
						if (sums_mass[j]>0)
						{
							if (class_segmentationMain::getEuclideanDistance(mean_center[i], mean_center[j])<min_centerDistance)
							{
								if (idxs_remap[j] == j)
								{
									idxs_remap[j] = i;
								}
							}
						}
					}
				}
			}
			mean_center.clear();
			for (int i=0;i<count_label;i++)
			{
				mean_center.push_back(xyz_zero);
			}
			fill(sums_mass.begin(), sums_mass.end(), 0);
			for (x=0;x<size_X;x++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (z=0;z<size_Z;z++)
					{
						pos_voxel = class_segmentationMain::coordinate2Index(x, y, z, offset_Y, offset_Z);
						label_voxel = int3D_label[z][x][y];
						if (label_voxel > 0)
						{
							label_voxel = label_voxel-1;
							value_voxel = Image3D_image[z][x][y];
							label_remap = idxs_remap[label_voxel];
							mean_center[label_remap].x += value_voxel*x; mean_center[label_remap].y += value_voxel*y; mean_center[label_remap].z += value_voxel*z;
							sums_mass[label_remap] += value_voxel;
							pos_voxel = class_segmentationMain::coordinate2Index(x+min_X, y+min_Y, z+min_Z, offset_Yglobal, offset_Zglobal);
							possVct_resultWithEmpty[label_remap].push_back(pos_voxel);
						}
					}
				}
			}
			for (int i=0;i<count_label;i++)
			{
				if (sums_mass[i]>0) {mean_center[i].x/=sums_mass[i]; mean_center[i].y/=sums_mass[i]; mean_center[i].z/=sums_mass[i];}
			}
			for (int i=0;i<count_label;i++)
			{
				if ((sums_mass[i]>0) && ((possVct_resultWithEmpty[i].size()>threshold_regionSize)))
				{
					possVct_result.push_back(possVct_resultWithEmpty[i]);
					poss_center.push_back(class_segmentationMain::coordinate2Index(mean_center[i].x+min_X, mean_center[i].y+min_Y, mean_center[i].z+min_Z, offset_Yglobal, offset_Zglobal));
				}
			}
			return possVct_result;
		}*/

		static vector<vector<V3DLONG> > int3D2possVct(int*** int3D_label, int count_label, double*** Image3D_image, 
			const V3DLONG size_X, const V3DLONG size_Y, const V3DLONG size_Z, 
			const V3DLONG min_X, const V3DLONG min_Y, const V3DLONG min_Z, const V3DLONG offset_Yglobal, const V3DLONG offset_Zglobal, 
			vector<V3DLONG> &poss_center)
		{
			int label_voxel = 0;
			V3DLONG pos_voxel = 0;
			double value_voxel = 0;
			vector<V3DLONG> vct_empty(0, 0);
			vector<vector<V3DLONG> > possVct_result;
			vector<vector<V3DLONG> > possVct_resultWithEmpty;
			for (int i=0;i<count_label;i++)
			{
				possVct_resultWithEmpty.push_back(vct_empty);
			}
			double3D xyz_zero;
			vector<double3D> mean_center;
			for (int i=0;i<count_label;i++)
			{
				mean_center.push_back(xyz_zero);
			}
			V3DLONG x=0; V3DLONG y=0; V3DLONG z=0;
			V3DLONG offset_Y = size_X; V3DLONG offset_Z = size_X*size_Y;
			vector<double> sums_mass (count_label, 0);
			for (x=0;x<size_X;x++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (z=0;z<size_Z;z++)
					{
						label_voxel = int3D_label[z][x][y];
						if (label_voxel > 0)
						{
							label_voxel = label_voxel-1;
							value_voxel = Image3D_image[z][x][y];
							pos_voxel = class_segmentationMain::coordinate2Index(x+min_X, y+min_Y, z+min_Z, offset_Yglobal, offset_Zglobal);
							possVct_resultWithEmpty[label_voxel].push_back(pos_voxel);
							mean_center[label_voxel].x += value_voxel*x; mean_center[label_voxel].y += value_voxel*y; mean_center[label_voxel].z += value_voxel*z;
							sums_mass[label_voxel] += value_voxel;
						}
					}
				}
			}
			for (int i=0;i<count_label;i++)
			{
				if (sums_mass[i]>0) {mean_center[i].x/=sums_mass[i]; mean_center[i].y/=sums_mass[i]; mean_center[i].z/=sums_mass[i];}
			}
			for (int i=0;i<count_label;i++)
			{
				if ((sums_mass[i]>0) && ((possVct_resultWithEmpty[i].size()>default_threshold_regionSize)))
				{
					possVct_result.push_back(possVct_resultWithEmpty[i]);
					poss_center.push_back(class_segmentationMain::coordinate2Index(mean_center[i].x+min_X, mean_center[i].y+min_Y, mean_center[i].z+min_Z, offset_Yglobal, offset_Zglobal));
				}
			}
			return possVct_result;
		}

		
		#pragma endregion

		#pragma region "math"
		double getCovariance(vector<double> vct_input1, vector<double> vct_input2)
		{
			double xmean = getMean(vct_input1);
			double ymean = getMean(vct_input1);
			double total = 0;
			for(int i = 0; i < vct_input1.size(); i++)
			{
				total += (vct_input1[i] - xmean) * (vct_input2[i] - ymean);
			}
			return total/vct_input1.size();
		}

		double getStdev(vector<double> vct_input)
		{
			double mean = getMean(vct_input);
			double temp = 0;
			for(int i = 0; i < vct_input.size(); i++)
			{
				temp += (vct_input[i] - mean)*(vct_input[i] - mean) ;
			}
			return sqrt(temp/(vct_input.size()-1));
		}

		double getMean(vector<double> vct_input)
		{
			double sum = 0;
			for(int i=0;i<vct_input.size();i++)
			{
				sum += vct_input[i];
			}
			return (sum/vct_input.size());
		}

		double getCorrelation(vector<double> vct_input1, vector<double> vct_input2)
		{
			double double_covariance = getCovariance(vct_input1, vct_input2);
			return double_covariance/((getStdev(vct_input1))*(getStdev(vct_input2)));
		}
		#pragma endregion

		#pragma region "memoryManagement"
		static double3D ***memory_allocate_double3D3(const int i_size, const int j_size, const int k_size)
		{
			double3D ***ptr_result;
			int i,k;
			ptr_result=(double3D ***) calloc(k_size,sizeof(double3D **));
			for(k=0;k<k_size;k++)
			{
				ptr_result[k]=(double3D **) calloc(i_size,sizeof(double3D *));
			}
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					ptr_result[k][i]=(double3D *) calloc(j_size,sizeof(double3D ));
				}
			}
			return(ptr_result);
		}

		static void memory_free_double3D3(double3D ***ptr_input, const int k_size, const int i_size)
		{
			int k,i;
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					free(ptr_input[k][i]);
				}
			}
			for(k=0;k<k_size;k++)
			{
				free(ptr_input[k]);
			}
			free(ptr_input);
		}

		static double ***memory_allocate_double3D(int i_size,int j_size,int k_size)
		{
			double ***ptr_result;
			int i,k;
			ptr_result=(double ***) calloc(k_size,sizeof(double **));
			for(k=0;k<k_size;k++)
			{
				ptr_result[k]=(double **) calloc(i_size,sizeof(double *));
			}
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					ptr_result[k][i]=(double *) calloc(j_size,sizeof(double ));
				}
			}
			return(ptr_result);
		}

		static void memory_free_double3D(double ***ptr_input, const int k_size, const int i_size)
		{
			int k,i;
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					free(ptr_input[k][i]);
				}
			}
			for(k=0;k<k_size;k++)
			{
				free(ptr_input[k]);
			}
			free(ptr_input);
		}

		static unsigned char ***memory_allocate_uchar3D(const int i_size, const int j_size, const int k_size)
		{
			unsigned char ***ptr_result;
			int i,k;
			ptr_result=(unsigned char ***) calloc(k_size,sizeof(unsigned char **));
			for(k=0;k<k_size;k++)
			{
				ptr_result[k]=(unsigned char **) calloc(i_size,sizeof(unsigned char *));
			}
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					ptr_result[k][i]=(unsigned char *) calloc(j_size,sizeof(unsigned char ));
				}
			}
			return(ptr_result);
		}

		static void memory_free_uchar3D(unsigned char ***ptr_input, const int k_size, const int i_size)
		{
			int k,i;
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					free(ptr_input[k][i]);
				}
			}
			for(k=0;k<k_size;k++)
			{
				free(ptr_input[k]);
			}
			free(ptr_input);
		}

		static long3D ***memory_allocate_int3D3(const int i_size, const int j_size, const int k_size)
		{
			long3D ***ptr_result;
			int i,k;
			ptr_result=(long3D ***) calloc(k_size,sizeof(long3D **));
			for(k=0;k<k_size;k++)
			{
				ptr_result[k]=(long3D **) calloc(i_size,sizeof(long3D *));
			}
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					ptr_result[k][i]=(long3D *) calloc(j_size,sizeof(long3D));
				}
			}
			return(ptr_result);
		}

		static void memory_free_int3D3(long3D ***ptr_input, const int k_size, const int i_size)
		{
			int k,i;
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					free(ptr_input[k][i]);
				}
			}
			for(k=0;k<k_size;k++)
			{
				free(ptr_input[k]);
			}
			free(ptr_input);
		}

		static int **memory_allocate_int2D(const int i_size,const int j_size)
		{
			int **ptr_result;
			int i;
			ptr_result=(int **) calloc(i_size,sizeof(int *));
			for(i=0;i<i_size;i++)
			{
				ptr_result[i]=(int *) calloc(j_size,sizeof(int ));
			}
			return(ptr_result);
		}

		static void memory_free_int2D(int **ptr_input, const int i_size)
		{
			int i;

			for(i=0;i<i_size;i++)
				free(ptr_input[i]);
			free(ptr_input);
		}

		static double **memory_allocate_double2D(const int i_size,const int j_size)
		{
			double **ptr_result;
			int i;
			ptr_result=(double **) calloc(i_size,sizeof(double *));
			for(i=0;i<i_size;i++)
			{
				ptr_result[i]=(double *) calloc(j_size,sizeof(double ));
			}
			return(ptr_result);
		}

		static void memory_free_double2D(double **ptr_input, const int i_size)
		{
			int i;

			for(i=0;i<i_size;i++)
				free(ptr_input[i]);
			free(ptr_input);
		}

		static int *memory_allocate_int1D(const int i_size)
		{
			int *ptr_result;
			ptr_result = (int *) calloc(i_size, sizeof(int));
			return(ptr_result);
		}

		static void memory_free_int1D(int *ptr_input, const int i_size)
		{
			free(ptr_input);
		}

		static unsigned char *memory_allocate_uchar1D(const int i_size)
		{
			unsigned char *ptr_result;
			ptr_result = (unsigned char *) calloc(i_size, sizeof(unsigned char));
			return(ptr_result);
		}

		static void memory_free_uchar1D(unsigned char *ptr_input, const int i_size)
		{
			free(ptr_input);
		}

		static int ***memory_allocate_int3D(int i_size,int j_size,int k_size)
		{
			int ***ptr_result;
			int i,k;
			ptr_result=(int ***) calloc(k_size,sizeof(int **));
			for(k=0;k<k_size;k++)
				ptr_result[k]=(int **) calloc(i_size,sizeof(int *));
			for(k=0;k<k_size;k++)
				for(i=0;i<i_size;i++)
					ptr_result[k][i]=(int *) calloc(j_size,sizeof(int ));
			return(ptr_result);
		}

		static void memory_free_int3D(int ***ptr_input,int k_size,int i_size)
		{
			int k,i;
			for(k=0;k<k_size;k++)
				for(i=0;i<i_size;i++)
					free(ptr_input[k][i]);
			for(k=0;k<k_size;k++)
				free(ptr_input[k]);
			free(ptr_input);
		}
		#pragma endregion

		#pragma region "smoothing"
		static void smooth_GVFkernal(double ***Image3D_input, int count_smoothIteration, int dim_X, int dim_Y, int dim_Z)
		{
			int i, x, y, z;
			double ***Image3D_update;
			Image3D_update = memory_allocate_double3D(dim_X, dim_Y, dim_Z);
			for(i=0;i<count_smoothIteration;i++)
			{
				for(z=0;z<dim_Z;z++)
					for(x=0;x<dim_X;x++)
						for(y=0;y<dim_Y;y++)
							Image3D_update[z][x][y] = 0;

				for(z=1;z<dim_Z-1;z++)
					for(x=1;x<dim_X-1;x++)
						for(y=1;y<dim_Y-1;y++)
							Image3D_update[z][x][y] = 0.4*Image3D_input[z][x][y]+0.1*(Image3D_input[z-1][x][y]+Image3D_input[z+1][x][y]+Image3D_input[z][x-1][y]+Image3D_input[z][x+1][y]+Image3D_input[z][x][y-1]+Image3D_input[z][x][y+1]);

				for(z=0;z<dim_Z;z++)
					for(x=0;x<dim_X;x++)
						for(y=0;y<dim_Y;y++)
							Image3D_input[z][x][y] = Image3D_update[z][x][y];
			}
			memory_free_double3D(Image3D_update,dim_Z,dim_X);
			return;
		}

		void smooth_GVFkernal(unsigned char *Image1D_input, int count_smoothIteration)
		{
			unsigned char *Image1D_update;
			Image1D_update = memory_allocate_uchar1D(this->size_page);
			V3DLONG pos_voxel;
			V3DLONG pos_neighbor1;
			V3DLONG pos_neighbor2;
			V3DLONG pos_neighbor3;
			V3DLONG pos_neighbor4;
			V3DLONG pos_neighbor5;
			V3DLONG pos_neighbor6;
			for(int i=0;i<count_smoothIteration;i++)
			{
				memset(Image1D_update, 0, this->size_page);
				for(int z=1;z<this->dim_Z-1;z++)
				{
					for(int x=1;x<this->dim_X-1;x++)
					{
						for(int y=1;y<this->dim_Y-1;y++)
						{
							pos_voxel = this->coordinate2Index(x, y, z);
							pos_neighbor1 = this->coordinate2Index(x, y, z-1);
							pos_neighbor2 = this->coordinate2Index(x, y, z+1);
							pos_neighbor3 = this->coordinate2Index(x, y-1, z);
							pos_neighbor4 = this->coordinate2Index(x, y+1, z);
							pos_neighbor5 = this->coordinate2Index(x-1, y, z);
							pos_neighbor6 = this->coordinate2Index(x+1, y, z);
							Image1D_update[pos_voxel] = 0.4*Image1D_input[pos_voxel]+0.1*(Image1D_input[pos_neighbor1]+Image1D_input[pos_neighbor2]+Image1D_input[pos_neighbor3]+Image1D_input[pos_neighbor4]+Image1D_input[pos_neighbor5]+Image1D_input[pos_neighbor6]);
						}
					}
				}
				for (int i=0;i<this->size_page;i++)
				{
					Image1D_update[i] = Image1D_update[i];
				}
			}
			memory_free_uchar1D(Image1D_update, this->size_page);
			return;
		}
		#pragma endregion

		#pragma region "GVF"
		int GVF_cellSegmentation(double *** Image3D_input, const int size_X, const int size_Y, const int size_Z, vector<double> paras_GVF,
			int*** Image3D_output)
		{
			ofstream ofstream_log;
			{ofstream_log.open ("log_GVF.txt");}
			int para_maxIteration = paras_GVF[0];
			double para_fusionThreshold = paras_GVF[1];
			double para_sigma = paras_GVF[2];
			double para_mu = paras_GVF[3];
			int count_label = 0;
			double3D ***Image3D3_u;
			double3D ***Image3D3_u_normalized;
			double3D ***Image3D3_f;
			double3D ***Image3D3_gradient;
			Image3D3_gradient = memory_allocate_double3D3(size_X, size_Y, size_Z);
			long3D ***Image3D3_mode;
			V3DLONG x, y, z;
			int ***Image3D_label;
			Image3D_label = memory_allocate_int3D(size_X, size_Y, size_Z);
			int count_page = size_X*size_Y*size_Z;
			int smoothIteration = (int)(3*para_sigma + 0.5);
			{ofstream_log<<"initialization passed!"<<endl;}
			smooth_GVFkernal(Image3D_input, smoothIteration, size_X, size_Y, size_Z);
			{ofstream_log<<"smoothing passed!"<<endl;}
			GVF_getGradient(Image3D_input, Image3D3_gradient, size_X, size_Y, size_Z);
			{ofstream_log<<"get gradient passed!"<<endl;}
			Image3D3_f = memory_allocate_double3D3(size_X,size_Y,size_Z);
			Image3D3_u = memory_allocate_double3D3(size_X,size_Y,size_Z);
			GVF_initialize(Image3D3_gradient, Image3D3_f, Image3D3_u, size_X, size_Y, size_Z);
			{ofstream_log<<"f/u initialization passed!"<<endl;}
			memory_free_double3D3(Image3D3_gradient,size_Z,size_X);
			GVF_warp(para_maxIteration, para_mu, Image3D3_u, Image3D3_f, size_X, size_Y, size_Z);
			{ofstream_log<<"warp passed!"<<endl;}
			memory_free_double3D3(Image3D3_f,size_Z,size_X);
			Image3D3_u_normalized = memory_allocate_double3D3(size_X,size_Y,size_Z);
			GVF_normalize(Image3D3_u, Image3D3_u_normalized, size_X, size_Y, size_Z);
			{ofstream_log<<"normalization passed!"<<endl;}
			memory_free_double3D3(Image3D3_u,size_Z,size_X);
			Image3D3_mode = memory_allocate_int3D3(size_X,size_Y,size_Z);
			GVF_findMode(Image3D3_u_normalized, Image3D3_mode, size_X, size_Y, size_Z);
			{ofstream_log<<"findMode passed!"<<endl;}
			memory_free_double3D3(Image3D3_u_normalized, size_Z, size_X);
			GVF_fuseMode( Image3D3_mode, Image3D_label, para_fusionThreshold, size_X, size_Y, size_Z);
			{ofstream_log<<"fuseMode passed!"<<endl;}
			memory_free_int3D3(Image3D3_mode,size_Z,size_X);
			//unsigned char ***Image3D_edge;
			//Image3D_edge = memory_allocate_uchar3D(size_X,size_Y,size_Z);
			//GVF_majorFilter(Image3D_label, const_int_GVF_majorFilterIteration, size_X, size_Y, size_Z);
			//{ofstream_log<<"major fitler1 passed!"<<endl;}
			//GVF_findEdge(Image3D_label, Image3D_edge, size_X, size_Y, size_Z);
			//{ofstream_log<<"find edge passed!"<<endl;}
			count_label = GVF_localThresholding(Image3D_input, Image3D_label, size_X, size_Y, size_Z);
			{ofstream_log<<"local thresholding passed!"<<endl;}
			//GVF_majorFilter(Image3D_result, const_int_GVF_majorFilterIteration, size_X, size_Y, size_Z);
			//{ofstream_log<<"major fitler2 passed!"<<endl;}
			V3DLONG pos_voxel = 0;
			for (z=0;z<size_Z;z++)
			{
				for (x=0;x<size_X;x++)
				{
					for (y=0;y<size_Y;y++)
					{
						Image3D_output[z][x][y] = Image3D_label[z][x][y];
					}
				}
			}
			memory_free_int3D(Image3D_label,size_Z,size_X);
			{ofstream_log<<"all succeed!"<<endl;}
			return count_label;
		}

		static void GVF_getGradient(double ***Image1D_input, double3D ***Image3D_gradient, const int dim_X, const int dim_Y, const int dim_Z)
		{
			int x,y,z;
			double max_gradient,tmp_gradient;
			for(z=0;z<dim_Z;z++)
			for(x=0;x<dim_X;x++)
			for(y=0;y<dim_Y;y++)
			{
				Image3D_gradient[z][x][y].x = 0;
				Image3D_gradient[z][x][y].y = 0;
				Image3D_gradient[z][x][y].z = 0;
			}
			for(z=1;z<dim_Z-1;z++)
			for(x=1;x<dim_X-1;x++)
			for(y=1;y<dim_Y-1;y++)
			{
				Image3D_gradient[z][x][y].x = 0.5*(Image1D_input[z][x+1][y] - Image1D_input[z][x-1][y]); 
				Image3D_gradient[z][x][y].y = 0.5*(Image1D_input[z][x][y+1] - Image1D_input[z][x][y-1]);
				Image3D_gradient[z][x][y].z = 0.5*(Image1D_input[z+1][x][y] - Image1D_input[z-1][x][y]);
			}
			max_gradient = -INF;
			for(z=0;z<dim_Z;z++)
			for(x=0;x<dim_X;x++)
			for(y=0;y<dim_Y;y++)
			{
				tmp_gradient = Image3D_gradient[z][x][y].x*Image3D_gradient[z][x][y].x + Image3D_gradient[z][x][y].y*Image3D_gradient[z][x][y].y + Image3D_gradient[z][x][y].z*Image3D_gradient[z][x][y].z; 
				if(tmp_gradient > max_gradient) {max_gradient = tmp_gradient;}
			}
			max_gradient = sqrt(max_gradient);
			max_gradient = max_gradient + const_infinitesimal;			  
			for(z=0;z<dim_Z;z++)
			for(x=0;x<dim_X;x++)
			for(y=0;y<dim_Y;y++)
			{
				Image3D_gradient[z][x][y].x = Image3D_gradient[z][x][y].x / max_gradient;
				Image3D_gradient[z][x][y].y = Image3D_gradient[z][x][y].y / max_gradient;
				Image3D_gradient[z][x][y].z = Image3D_gradient[z][x][y].z / max_gradient;
				tmp_gradient = sqrt(Image3D_gradient[z][x][y].x*Image3D_gradient[z][x][y].x + Image3D_gradient[z][x][y].y*Image3D_gradient[z][x][y].y + Image3D_gradient[z][x][y].z*Image3D_gradient[z][x][y].z);
			}
			return;
		}

		static void GVF_initialize(double3D ***Image3D_gradient, double3D ***Image3D_f, double3D ***Image3D_u, int dim_X, int dim_Y, int dim_Z)
		{
			int x,y,z;
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						Image3D_f[z][x][y].x = Image3D_gradient[z][x][y].x;
						Image3D_f[z][x][y].y = Image3D_gradient[z][x][y].y;
						Image3D_f[z][x][y].z = Image3D_gradient[z][x][y].z;
						Image3D_u[z][x][y].x = 0.0;
						Image3D_u[z][x][y].y = 0.0;
						Image3D_u[z][x][y].z = 0.0;
					}
				}
			}
			return;
		}

		static int GVF_warp(int ITER, double mu, double3D ***u, double3D ***f, int x_size, int y_size, int z_size)
		{
			int count=0,i,j,k;
			double3D ***tmpu;
			double *mag;
			tmpu = memory_allocate_double3D3(x_size,y_size,z_size);
			mag=(double *)calloc(x_size*y_size*z_size,sizeof(double));
			for(k=0;k<z_size;k++)
				for(i=0;i<x_size;i++)
					for(j=0;j<y_size;j++)
						mag[k*x_size*y_size+i*y_size+j]=f[k][i][j].x*f[k][i][j].x+f[k][i][j].y*f[k][i][j].y+f[k][i][j].z*f[k][i][j].z;
			while(count<ITER)
			{   
				for(k=1;k<z_size-1;k++)
				{
					for(i=1;i<x_size-1;i++)
					{
						for(j=1;j<y_size-1;j++)
						{
							tmpu[k][i][j].x=u[k][i][j].x+mu*(u[k+1][i][j].x+u[k-1][i][j].x+u[k][i+1][j].x+u[k][i-1][j].x+u[k][i][j+1].x+u[k][i][j-1].x-6*u[k][i][j].x)-(u[k][i][j].x-f[k][i][j].x)*mag[k*x_size*y_size+i*y_size+j];
							tmpu[k][i][j].y=u[k][i][j].y+mu*(u[k+1][i][j].y+u[k-1][i][j].y+u[k][i+1][j].y+u[k][i-1][j].y+u[k][i][j+1].y+u[k][i][j-1].y-6*u[k][i][j].y)-(u[k][i][j].y-f[k][i][j].y)*mag[k*x_size*y_size+i*y_size+j];
							tmpu[k][i][j].z=u[k][i][j].z+mu*(u[k+1][i][j].z+u[k-1][i][j].z+u[k][i+1][j].z+u[k][i-1][j].z+u[k][i][j+1].z+u[k][i][j-1].z-6*u[k][i][j].z)-(u[k][i][j].z-f[k][i][j].z)*mag[k*x_size*y_size+i*y_size+j];
						}
					}
				}
				for(k=1;k<z_size-1;k++)
				{
					for(i=1;i<x_size-1;i++)
					{
						for(j=1;j<y_size-1;j++)
						{
							u[k][i][j].x=tmpu[k][i][j].x;
							u[k][i][j].y=tmpu[k][i][j].y;
							u[k][i][j].z=tmpu[k][i][j].z;
						}
					}
				}
				count++;
			} 
			free(mag);
			memory_free_double3D3( tmpu, z_size, x_size);
			return(count);
		} 

		static void GVF_normalize(double3D ***u, double3D ***nu, int x_size, int y_size, int z_size)
		{
			int x,y,z;
			double tempgrad;
			for(z=1;z<z_size-1;z++)
			{
				for(x=1;x<x_size-1;x++)
				{
					for(y=1;y<y_size-1;y++)
					{
						tempgrad = sqrt(u[z][x][y].x*u[z][x][y].x + u[z][x][y].y*u[z][x][y].y + u[z][x][y].z*u[z][x][y].z + const_infinitesimal);
						nu[z][x][y].x = u[z][x][y].x/tempgrad;
						nu[z][x][y].y = u[z][x][y].y/tempgrad;
						nu[z][x][y].z = u[z][x][y].z/tempgrad;
					}
				}
			}
			return;
		}

		static void GVF_findMode(double3D ***u, long3D ***mode, int x_size, int y_size, int z_size)
		{
			int x,y,z;
			int nextx,nexty,nextz,nnextx,nnexty,nnextz;
			int n,count;
			double value_threshold = 0.3, threshold2 = 0.1;
			unsigned char ***visit;
			unsigned char ***edge;
			int **pass;
			visit = memory_allocate_uchar3D(x_size,y_size,z_size);
			edge = memory_allocate_uchar3D(x_size,y_size,z_size);
			int volume = x_size*y_size*z_size;
			pass = memory_allocate_int2D(volume/3, 3);
			for(z=0;z<z_size;z++)
			{
				for(x=0;x<x_size;x++)
				{
					for(y=0;y<y_size;y++)
					{
						visit[z][x][y] = 0;
						edge[z][x][y] = 0;
						mode[z][x][y].x = -1;
						mode[z][x][y].y = -1;
						mode[z][x][y].z = -1;
					}
					for(x=0;x<volume/3;x++)
					{
						pass[x][0] = 0;
						pass[x][1] = 0;
						pass[x][2] = 0;
					}
				}
			}
			for(z=1;z<z_size-1;z++)
			{
				for(x=1;x<x_size-1;x++)
				{
					for(y=1;y<y_size-1;y++)
					{
						if(visit[z][x][y]==0)
						{
							count = -1;
							nextx = x;	
							nexty = y;
							nextz = z;
							if(u[nextz][nextx][nexty].x>=0)
								nnextx = nextx + (int)(u[nextz][nextx][nexty].x + 0.5);
							else
								nnextx = nextx + (int)(u[nextz][nextx][nexty].x - 0.5);
							if(u[nextz][nextx][nexty].y>=0)
								nnexty = nexty + (int)(u[nextz][nextx][nexty].y + 0.5);
							else
								nnexty = nexty + (int)(u[nextz][nextx][nexty].y - 0.5);
							if(u[nextz][nextx][nexty].z>=0)
								nnextz = nextz + (int)(u[nextz][nextx][nexty].z + 0.5);
							else
								nnextz = nextz + (int)(u[nextz][nextx][nexty].z - 0.5);
							while(nnextx>0 && nnexty>0 && nnextz>0 && nnextx<x_size && nnexty<y_size && nnextz<z_size \
								&& u[nextz][nextx][nexty].x*u[nnextz][nnextx][nnexty].x + \
								u[nextz][nextx][nexty].y*u[nnextz][nnextx][nnexty].y + \
								u[nextz][nextx][nexty].z*u[nnextz][nnextx][nnexty].z>=value_threshold \
								&& fabs(u[nextz][nextx][nexty].x) + fabs(u[nextz][nextx][nexty].y) + fabs(u[nextz][nextx][nexty].z) > 0.5 \
								&& (nnextx!=nextx || nnexty!=nexty || nnextz!=nextz))
							{
								count++;
								pass[count][0] = nextx;
								pass[count][1] = nexty;
								pass[count][2] = nextz;
								nextx = nnextx;
								nexty = nnexty;
								nextz = nnextz;
								if(u[nextz][nextx][nexty].x>=0)
									nnextx = nextx + (int)(u[nextz][nextx][nexty].x + 0.5);
								else
									nnextx = nextx + (int)(u[nextz][nextx][nexty].x - 0.5);
								if(u[nextz][nextx][nexty].y>=0)
									nnexty = nexty + (int)(u[nextz][nextx][nexty].y + 0.5);
								else
									nnexty = nexty + (int)(u[nextz][nextx][nexty].y - 0.5);
								if(u[nextz][nextx][nexty].z>=0)
									nnextz = nextz + (int)(u[nextz][nextx][nexty].z + 0.5);
								else
									nnextz = nextz + (int)(u[nextz][nextx][nexty].z - 0.5);
								if (count > 65535)
									break;
							}
							if(nextx>0 && nexty>0 && nextz>0 && nextx<x_size && nexty<y_size && nextz<z_size)
							{
								mode[z][x][y].x = nextx;
								mode[z][x][y].y = nexty;
								mode[z][x][y].z = nextz;
							}
							else
							{
								mode[z][x][y].x = -1;
								mode[z][x][y].y = -1;
								mode[z][x][y].z = -1;
							}
							visit[z][x][y]=1;
							if(count>=0)
							{
								for(n=0;n<=count;n++)
								{
									if(visit[pass[count][2]][pass[n][0]][pass[n][1]]==0)
									{
										mode[pass[count][2]][pass[n][0]][pass[n][1]].x = nextx;
										mode[pass[count][2]][pass[n][0]][pass[n][1]].y = nexty;
										mode[pass[count][2]][pass[n][0]][pass[n][1]].z = nextz;

										visit[pass[count][2]][pass[n][0]][pass[n][1]] = 1;
									}
								}
							}						
						}
					}
				}
			}
			memory_free_int2D(pass,volume/3);
			memory_free_uchar3D(visit,z_size,x_size);
			memory_free_uchar3D(edge,z_size,x_size);
			return;
		}

		static void GVF_fuseMode(long3D ***mode, int ***visit, const double fuse_th, const int x_size, const int y_size, const int z_size)
		{
			int x, y, z;
			int label;
			int **indexTable;
			int *modePointCounts;
			int volume = x_size*y_size*z_size;
			indexTable = memory_allocate_int2D(volume,3);
			modePointCounts = (int *) malloc(sizeof(int)*volume);
			for(z=0;z<z_size;z++)
			{
				for(x=0;x<x_size;x++)
				{
					for(y=0;y<y_size;y++)
					{
						visit[z][x][y] = -1;
					}
				}
			}
			for(x=0;x<volume/3;x++)
			{
				modePointCounts[x] = 0;
				indexTable[x][0] = 0;
				indexTable[x][1] = 0;
				indexTable[x][2] = 0;
			}
			label = -1;
			for(z=0;z<z_size;z++)
			{
				for(x=0;x<x_size;x++)
				{
					for(y=0;y<y_size;y++)
					{
						if(visit[z][x][y]<0)
						{
							visit[z][x][y] = ++label;
							GVF_fill( x, y, z, label, mode, visit, modePointCounts, indexTable, fuse_th, x_size, y_size, z_size);
						}
					}
				}
			}
			free(modePointCounts);
			memory_free_int2D(indexTable,volume);
			return;
		}

		static void GVF_fill(int x, int y, int z, int label, long3D ***mode, int ***visit, int *modePointCounts, int **indexTable, double fuse_th, int x_size, int y_size, int z_size)
		{
			int i, j, k;
			int neighx, neighy, neighz;
			int index=0;
			double mode_dist;
			int neighborsFound;
			indexTable[0][0] = x;
			indexTable[0][1] = y;
			indexTable[0][2] = z;
			modePointCounts[label]++;
			while(1)
			{
				neighborsFound	= 0;
				for(k=-1;k<=1;k++)
					for(i=-1;i<=1;i++)
						for(j=-1;j<=1;j++)
						{
							neighx = x + i;
							neighy = y + j;
							neighz = z + k;
							if((neighx>=0)&&(neighy>=0)&&(neighz>=0) \
								&&(neighx<x_size)&&(neighy<y_size)&&(neighz<z_size) \
								&& visit[neighz][neighx][neighy]<0) 
							{
								mode_dist = sqrt(double((mode[z][x][y].x-mode[neighz][neighx][neighy].x)*(mode[z][x][y].x-mode[neighz][neighx][neighy].x) \
									+ (mode[z][x][y].y-mode[neighz][neighx][neighy].y)*(mode[z][x][y].y-mode[neighz][neighx][neighy].y) \
									+ (mode[z][x][y].z-mode[neighz][neighx][neighy].z)*(mode[z][x][y].z-mode[neighz][neighx][neighy].z)));

								if(mode_dist <= fuse_th)
								{
									visit[neighz][neighx][neighy] = label;

									modePointCounts[label]++;

									++index;
									indexTable[index][0] = neighx;
									indexTable[index][1] = neighy;
									indexTable[index][2] = neighz;

									neighborsFound = 1;
								}
							}
						}
						if(neighborsFound)
						{
							x = indexTable[index][0];
							y = indexTable[index][1];
							z = indexTable[index][2];
						}
						else if (index > 1)
						{
							--index;
							x = indexTable[index][0];
							y = indexTable[index][1];
							z = indexTable[index][2];
						}
						else
							break; 
			}
			return;
		}

		int GVF_localThresholding(double ***Image3D_input, int ***Image3D_label, int size_X, int size_Y, int size_Z)
		{
			int x, y, z;
			int idx_label, i;
			int threshold_global = 0;
			//double **histos_local;
			double *histo_global;
			int *vct_threshold;
			int label_voxel = 0;
			int value_voxel = 0;
			int count_label = 0;
			for(z=0;z<size_Z;z++)
			{
				for(x=0;x<size_X;x++)
				{
					for(y=0;y<size_Y;y++)
					{
						label_voxel = Image3D_label[z][x][y];
						if(label_voxel>count_label) //we are assuming continuous labeling here;
						{
							count_label = label_voxel;
						}
					}
				}
			}
			count_label = count_label + 1;
			cout<<"count_label: "<<count_label<<endl;
			//histos_local = memory_allocate_double2D(count_label, const_length_histogram);
			histo_global = (double *)malloc(sizeof(double)*const_length_histogram);
			for(i=0;i<const_length_histogram;i++)
			{
				histo_global[i] = 0;
			}
			for(z=0;z<size_Z;z++)
			{
				for(x=0;x<size_X;x++)
				{
					for(y=0;y<size_Y;y++)
					{
						//label_voxel = Image3D_label[z][x][y];
						value_voxel = (int)Image3D_input[z][x][y];
						//histos_local[label_voxel][value_voxel]++;
						histo_global[value_voxel]++;
					}
				}
			}
			//threshold_global = estimateThresholdYen(histo_global)*0.5;
			threshold_global = estimateThresholdOtsu(histo_global);
			cout<<"threshold_global: "<<threshold_global<<endl;
			free(histo_global);
			/*vct_threshold = (int *)malloc(sizeof(int)*(count_label));
			for(idx_label=0;idx_label<count_label;idx_label++)
			{
				vct_threshold[idx_label] = estimateThresholdYen(histos_local[idx_label])*0.5;
			}
			memory_free_double2D(histos_local, count_label);*/
			vector<V3DLONG> labels_used (0, 0);
			count_label = 0;
			int idx_usedLabel = 0;
			for(z=0;z<size_Z;z++)
			{
				for(x=0;x<size_X;x++)
				{
					for(y=0;y<size_Y;y++)
					{
						label_voxel = Image3D_label[z][x][y];
						value_voxel = Image3D_input[z][x][y];
						//if((label_voxel>0) && (value_voxel>vct_threshold[label_voxel]) && (value_voxel>threshold_global))
						if((label_voxel>0) && (value_voxel>threshold_global))
						{
							idx_usedLabel = vctContains(labels_used, label_voxel);
							if (idx_usedLabel>0)
							{
								Image3D_label[z][x][y] = idx_usedLabel+1;
							}
							else
							{
								count_label++;
								Image3D_label[z][x][y] = count_label;
								labels_used.push_back(label_voxel);
							}
						}
						else
						{
							Image3D_label[z][x][y] = 0;
						}
					}
				}
			}
			cout<<"thresholding passed;"<<endl;
			//free(vct_threshold);
			return count_label;
		}
		#pragma endregion

		#pragma region "shapeStat"
		vector<vector<double> > getShapeStat(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG value_radius)
		{
			vector<vector<double> > valuesVct_result;
			double value_PC1 = 0;
			double value_PC2 = 0;
			double value_PC3 = 0;
			vector<double> values_PC1;
			vector<double> values_PC2;
			vector<double> values_PC3;
			double size_step = (double)(value_radius-2)/3.0;
			V3DLONG rr=0;
			for (int i=1;i<=4;i++) //make sure the first sample is at rr==1, the last sample at rr==value_radius;
			{
				rr=2+size_step*(i-1);
				compute_win3d_pca(this->Image3D_page, this->dim_X, this->dim_Y, this->dim_Z, x , y, z, rr, rr, rr,value_PC1, value_PC2, value_PC3, this->idx_shape, false);
				values_PC1.push_back(value_PC1/rr);
				values_PC2.push_back(value_PC2/rr);
				values_PC3.push_back(value_PC3/rr);
			}
			valuesVct_result.push_back(values_PC1);
			valuesVct_result.push_back(values_PC2);
			valuesVct_result.push_back(values_PC3);
			/*double value_Lscore = exp( -( (value_PC1-value_PC2)*(value_PC1-value_PC2) + (value_PC2-value_PC3)*(value_PC2-value_PC3) + (value_PC1-value_PC3)*(value_PC1-value_PC3) ) /
				(value_PC1*value_PC1 + value_PC2*value_PC2 + value_PC3*value_PC3) );*/
			/*double value_linear = (value_PC1-value_PC2)/(value_PC1+value_PC2+value_PC3);
			double value_planar = 2.0*(value_PC2-value_PC3)/(value_PC1+value_PC2+value_PC3);
			double value_sphere = 3.0*value_PC3/(value_PC1+value_PC2+value_PC3);*/
			//vct_result.push_back(value_Lscore);
			//vct_result.push_back(value_linear);
			//vct_result.push_back(value_planar);
			//vct_result.push_back(value_sphere);
			return valuesVct_result;
		}
		#pragma endregion
	};
	#pragma endregion
	class_segmentationMain class_segmentationMain1;
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);
	float getPluginVersion() const {return 1.1f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

	bool interface_exemplarDefination(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent)
	{
		v3dhandle v3dhandle_currentWindow = _V3DPluginCallback2_currentCallback.currentImageWindow();
		if (!v3dhandle_currentWindow) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		Image4DSimple* Image4DSimple_current = _V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
		if (!Image4DSimple_current) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
		if (count_totalBytes<1) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		unsigned char* Image1D_current = Image4DSimple_current->getRawData();
		QString QString_fileNameCurrent = _V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
		V3DLONG dim_X = Image4DSimple_current->getXDim(); V3DLONG dim_Y = Image4DSimple_current->getYDim();
		V3DLONG dim_Z = Image4DSimple_current->getZDim(); V3DLONG dim_channel = Image4DSimple_current->getCDim();
		//convert it to UNIT8;
		ImagePixelType ImagePixelType_current = Image4DSimple_current->getDatatype(); V3DLONG size_image = dim_X*dim_Y*dim_Z*dim_channel;
		if(ImagePixelType_current != V3D_UINT8)
		{
			unsigned char* Image1D_convertedToUNIT8 = class_segmentationMain::memory_allocate_uchar1D(size_image);
			if (ImagePixelType_current==V3D_UINT16)
			{
				double min,max;
				if(!rescale_to_0_255_and_copy((unsigned short int *)Image1D_current, size_image, min, max, Image1D_convertedToUNIT8)) {return false;}
			}
			else if(ImagePixelType_current==V3D_FLOAT32)
			{
				double min,max;
				if(!rescale_to_0_255_and_copy((float *)Image1D_current, size_image, min, max, Image1D_convertedToUNIT8)) {return false;}
			}
			else
			{v3d_msg("Warning: unknown voxel data type, program will terminate!"); return false; }
			class_segmentationMain::memory_free_uchar1D(Image1D_current, size_image);
			Image1D_current = Image1D_convertedToUNIT8;
		}
		
		LandmarkList LandmarkList_userDefined = _V3DPluginCallback2_currentCallback.getLandmark(v3dhandle_currentWindow);
		V3DLONG count_userDefinedLandmarkList = LandmarkList_userDefined.count();
		QList<NeuronTree> * SWCList_current = _V3DPluginCallback2_currentCallback.getHandleNeuronTrees_3DGlobalViewer(v3dhandle_currentWindow);
		V3DLONG count_SWCList = 0;
		if (SWCList_current) {count_SWCList = SWCList_current->count();}
		LandmarkList LandmarkList_current;
		V3DLONG count_currentLandmarkList = -1;
		if ((count_SWCList<1) && (count_userDefinedLandmarkList<1)) {v3d_msg("You have not defined any landmarks or swc structure to run the segmenation, program canceled!"); return false;}
		else if ((count_SWCList>0) && (count_userDefinedLandmarkList>0))
		{
			LandmarkList_current = LandmarkList_userDefined;
			class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
			count_currentLandmarkList = LandmarkList_current.count();
		}
		else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1))
		{
			class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
			count_currentLandmarkList = LandmarkList_current.count();
		}
		if (count_userDefinedLandmarkList>0)
		{
			LandmarkList_current = LandmarkList_userDefined;
			count_currentLandmarkList = LandmarkList_current.count();
		}
		dialogDefineExemplar dialogDefineExemplar1(_V3DPluginCallback2_currentCallback, _QWidget_parent, dim_channel);
		if (dialogDefineExemplar1.exec()!=QDialog::Accepted) {return false;}
		this->class_segmentationMain1.control_initialize(Image1D_current, dim_X, dim_Y, dim_Z, dialogDefineExemplar1.channel_idx_selection);
		this->class_segmentationMain1.control_defineExemplar(dialogDefineExemplar1.intensity_threshold_global, LandmarkList_current, dialogDefineExemplar1.intensity_threshold_valueChangeRatio);
		visualizationImage1D(this->class_segmentationMain1.Image1D_exemplar, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Exemplar");
		return true;
	}

	bool interface_exemplarPropagation(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent)
	{
		v3dhandle v3dhandle_currentWindow = _V3DPluginCallback2_currentCallback.currentImageWindow();
		if (!v3dhandle_currentWindow) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		Image4DSimple* Image4DSimple_current = _V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
		if (!Image4DSimple_current) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		dialogPropagateExemplar dialogPropagateExemplar1(_V3DPluginCallback2_currentCallback, _QWidget_parent);
		if (dialogPropagateExemplar1.exec()!=QDialog::Accepted) {return false;}
		//get shape paramters;
		int idx_shape;
		if (dialogPropagateExemplar1.shape_type_selection == sphere) {idx_shape = 1;}
		else if (dialogPropagateExemplar1.shape_type_selection == cube) {idx_shape = 0;}
		this->class_segmentationMain1.control_propagateExemplar(idx_shape, dialogPropagateExemplar1.shape_para_delta, dialogPropagateExemplar1.intensity_threshold_histoCorr, 
			dialogPropagateExemplar1.shape_multiplier_thresholdRegionSize, dialogPropagateExemplar1.shape_multiplier_uThresholdRegionSize);
		visualizationImage1D(this->class_segmentationMain1.Image1D_segmentationResultOriginal, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Segmentation Result (regionGrowing)");
		visualizationImage1D(this->class_segmentationMain1.Image1D_segmentationResultPassed, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Segmentation Result (exemplar-like shape)");
		_V3DPluginCallback2_currentCallback.setLandmark(v3dhandle_currentWindow, this->class_segmentationMain1.LandmarkList_segmentationResultPassed);
		_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_currentWindow);
		return true;
	}

	bool interface_furtherSegmentation(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent)
	{
		v3dhandle v3dhandle_currentWindow = _V3DPluginCallback2_currentCallback.currentImageWindow();
		if (!v3dhandle_currentWindow) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		Image4DSimple* Image4DSimple_current = _V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
		if (!Image4DSimple_current) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		dialogFurtherSegmentation dialogFurtherSegmentation1(_V3DPluginCallback2_currentCallback, _QWidget_parent);
		if (dialogFurtherSegmentation1.exec()!=QDialog::Accepted) {return false;}
		//get GVF paramters;
		vector<double> paras_GVF (4, 0);
		paras_GVF[0] = dialogFurtherSegmentation1.GVF_para_maxIteration;
		paras_GVF[1] = dialogFurtherSegmentation1.GVF_para_mergingThreshold;
		paras_GVF[2] = dialogFurtherSegmentation1.GVF_para_sigma;
		paras_GVF[3] = dialogFurtherSegmentation1.GVF_para_mu;
		this->class_segmentationMain1.control_furtherSegmentation(paras_GVF);
		visualizationImage1D(this->class_segmentationMain1.Image1D_segmentationResultGVF, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Segmentation Result (splitted from irregular shapes)");
		visualizationImage1D(this->class_segmentationMain1.Image1D_segmentationResultMerged, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Segmentation Result (merged)");
		_V3DPluginCallback2_currentCallback.setLandmark(v3dhandle_currentWindow, this->class_segmentationMain1.LandmarkList_segmentationResultMerged);
		_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_currentWindow);
		return true;
	}

	void visualizationImage1D(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int dim_channel, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
	{
		Image4DSimple Image4DSimple_temp;
		Image4DSimple_temp.setData((unsigned char*)Image1D_input, dim_X, dim_Y, dim_Z, dim_channel, V3D_UINT8);
		v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
		V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
		V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
};



#endif