/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 :by Xiang Li (lindbergh.li@gmail.com);
 */
 
#ifndef __CELLSEGMENTATION_PLUGIN_H__
#define __CELLSEGMENTATION_PLUGIN_H__

#pragma region "includes and constants"
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
const double default_threshold_global = 15; //a small enough value for the last resort;
const int default_threshold_regionSize = 8; //cube of 2 voxel length;
const double const_infinitesimal = 0.000000001;
#define INF 1E9
#define NINF -1E9
#define PI 3.14159265
enum enum_shape_t {sphere, cube};
#pragma endregion

#pragma region "dialogInitialization" 
class dialogRun:public QDialog
{
	Q_OBJECT
public:
	dialogRun(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent, int int_channelDim)
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
		//shape;
		QGroupBox *QGroupBox_shape_main = new QGroupBox("Geometry stat");
		QGridLayout *QGridLayout_shape_main = new QGridLayout();
		QRadioButton_shape_sphere = new QRadioButton("sphere-like", QWidget_parent);
		QRadioButton_shape_sphere->setChecked(true);
		QRadioButton_shape_cube = new QRadioButton("cube-like", QWidget_parent);
		QRadioButton_shape_cube->setChecked(false);
		QGridLayout_shape_main->addWidget(QRadioButton_shape_sphere, 1, 1, 1, 1);
		QGridLayout_shape_main->addWidget(QRadioButton_shape_cube, 1, 2, 1, 1);
		QLabel* QLabel_shape_delta = new QLabel(QObject::tr("max anisotropic\ndeviation:"));
		QLineEdit_Shape_delta = new QLineEdit("1", QWidget_parent);
		QGridLayout_shape_main->addWidget(QLabel_shape_delta, 1, 3, 1, 1);
		QGridLayout_shape_main->addWidget(QLineEdit_Shape_delta, 1, 4, 1, 1);
		QLabel* QLabel_shape_thresholdRegionSize = new QLabel(QObject::tr("Min region size\nvs. exemplar ratio:"));
		QLineEdit_shape_thresholdRegionSize = new QLineEdit("0.1", QWidget_parent);
		QLabel* QLabel_shape_uThresholdRegionSize = new QLabel(QObject::tr("Max region size\nvs. exemplar ratio:"));
		QLineEdit_shape_uThresholdRegionSize = new QLineEdit("20", QWidget_parent);
		QGridLayout_shape_main->addWidget(QLabel_shape_thresholdRegionSize, 2, 1, 1, 1);
		QGridLayout_shape_main->addWidget(QLineEdit_shape_thresholdRegionSize, 2, 2, 1, 1);
		QGridLayout_shape_main->addWidget(QLabel_shape_uThresholdRegionSize, 2, 3, 1, 1);
		QGridLayout_shape_main->addWidget(QLineEdit_shape_uThresholdRegionSize, 2, 4, 1, 1);
		QGroupBox_shape_main->setLayout(QGridLayout_shape_main);
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
		QGridLayout_main->addWidget(QGroupBox_channel_main);
		QGridLayout_main->addWidget(QGroupBox_shape_main);
		QGridLayout_main->addWidget(QWidget_control_bar);
		setLayout(QGridLayout_main);
		setWindowTitle(QString("cellSegmentation: quickFind"));
		//event evoking;
		connect(QPushButton_control_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
		connect(QPushButton_control_close, SIGNAL(clicked()), this, SLOT(reject()));
		update();
	}
	~dialogRun(){}
	QComboBox* QComboBox_channel_selection;
	V3DLONG channel_idx_selection;
	QLineEdit* QLineEdit_Shape_delta;
	QLineEdit* QLineEdit_shape_thresholdRegionSize;
	QLineEdit* QLineEdit_shape_uThresholdRegionSize;
	QRadioButton* QRadioButton_shape_sphere;
	QRadioButton* QRadioButton_shape_cube;
	enum_shape_t shape_type_selection;
	double shape_para_delta;
	double shape_multiplier_thresholdRegionSize;
	double shape_multiplier_uThresholdRegionSize;

	public slots:
	void _slot_start()
	{
		channel_idx_selection = QComboBox_channel_selection->currentIndex() + 1;
		shape_para_delta = this->QLineEdit_Shape_delta->text().toDouble();
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
			unsigned char* Image1D_page;
			unsigned char* Image1D_mask;
			unsigned char*** Image3D_page;
			V3DLONG dim_X;
			V3DLONG dim_Y;
			V3DLONG dim_Z;
			V3DLONG size_page;
			V3DLONG size_page3;
			V3DLONG offset_channel;
			V3DLONG offset_Z;
			V3DLONG offset_Y;
			int idx_channel;
			int idx_shape;
			QString name_currentWindow;
			
			//Exemplar (or learn from it);
			vector<V3DLONG> poss_exemplar;
			vector<vector<V3DLONG> > possVct_exemplarRegion;
			vector<vector<vector<double> > > valueVctVct_exemplarShapeStat;
			vector<V3DLONG> thresholds_voxelValue;
			vector<V3DLONG> thresholds_regionSize;
			vector<V3DLONG> uThresholds_regionSize;
			vector<V3DLONG> thresholds_radius;
			double threshold_deltaShapeStat;
			double multiplier_thresholdRegionSize;
			double multiplier_uThresholdRegionSize;
			unsigned char* Image1D_exemplar;

			//segmentation;
			vector<vector<V3DLONG> > possVct_segmentationResult;
			//vector<vector<V3DLONG> > possVct_segmentationResultMerged;
			vector<vector<V3DLONG> > possVct_seed;
			unsigned char* Image1D_segmentationResult;
			LandmarkList LandmarkList_segmentationResult;
			LandmarkList LandmarkList_exemplar;
			vector<V3DLONG> poss_segmentationResultCenter;
			//vector<V3DLONG> poss_segmentationResultCenterMerged;
			#pragma endregion
			
		class_segmentationMain() {}
		~class_segmentationMain() {}

		#pragma region "control-run"
		bool control_run(unsigned char* _Image1D_original, V3DLONG _dim_X, V3DLONG _dim_Y, V3DLONG _dim_Z ,
			int _idx_channel, LandmarkList _LandmarkList_exemplar, int _idx_shape, double _threshold_deltaShapeStat,
			double _multiplier_thresholdRegionSize, double _multiplier_uThresholdRegionSize, QString _name_currentWindow)
		{
			if (this->poss_exemplar.size()<1)
			{
				this->dim_X = _dim_X; this->dim_Y = _dim_Y; this->dim_Z = _dim_Z; this->idx_channel = _idx_channel;
				this->size_page = dim_X*dim_Y*dim_Z;
				this->size_page3 = this->size_page+this->size_page+this->size_page;
				this->offset_channel = (idx_channel-1)*size_page;
				this->offset_Z = dim_X*dim_Y;
				this->offset_Y = dim_X;
				this->Image1D_page = memory_allocate_uchar1D(this->size_page);
				this->Image1D_mask = memory_allocate_uchar1D(this->size_page);
				this->Image3D_page = memory_allocate_uchar3D(this->dim_Y, this->dim_X, this->dim_Z); //tricky!
				this->Image1D_segmentationResult = memory_allocate_uchar1D(this->size_page3);
				this->Image1D_exemplar = memory_allocate_uchar1D(this->size_page3);
				for (V3DLONG i=0;i<this->size_page;i++)
				{	
					this->Image1D_page[i] = _Image1D_original[i+offset_channel];
					vector<V3DLONG> xyz_i = this->index2Coordinate(i);
					this->Image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = this->Image1D_page[i];
					this->Image1D_segmentationResult[i] = this->Image1D_page[i];
					this->Image1D_segmentationResult[i+size_page] = this->Image1D_page[i];
					this->Image1D_segmentationResult[i+size_page+size_page] = this->Image1D_page[i];
				}
				memset(this->Image1D_mask, const_max_voxelValue, this->size_page);
				this->idx_shape = _idx_shape;
				this->categorizeVoxelsByValue();
				this->threshold_deltaShapeStat = _threshold_deltaShapeStat;
				this->multiplier_thresholdRegionSize = _multiplier_thresholdRegionSize;
				this->multiplier_uThresholdRegionSize = _multiplier_uThresholdRegionSize;
				this->name_currentWindow = _name_currentWindow;
			}
			//this->possVct_segmentationResult.clear();
			this->LandmarkList_exemplar = _LandmarkList_exemplar;
			this->initializeConstants();
			this->poss_exemplar.clear();
			this->possVct_exemplarRegion.clear();
			this->thresholds_voxelValue.clear();
			this->thresholds_radius.clear();
			this->valueVctVct_exemplarShapeStat.clear();
			this->thresholds_regionSize.clear();
			this->uThresholds_regionSize.clear();
			this->poss_exemplar = landMarkList2IndexList(_LandmarkList_exemplar);
			V3DLONG count_exemplar = this->poss_exemplar.size();
			vector<V3DLONG> poss_exemplarNew;
			for (V3DLONG idx_exemplar=0;idx_exemplar<count_exemplar;idx_exemplar++)
			{
				V3DLONG pos_exemplar = this->poss_exemplar[idx_exemplar];
				if (this->Image1D_mask[pos_exemplar]<1) {continue;}
				V3DLONG value_exemplar = this->Image1D_page[pos_exemplar];
				V3DLONG count_step = (value_exemplar-default_threshold_global);
				V3DLONG pos_massCenterOld = -1;
				V3DLONG pos_massCenterNew = 0;
				vector<V3DLONG> poss_exemplarRegionNew;
				vector<V3DLONG> poss_exemplarRegionOld;
				V3DLONG idx_step = 0;
				for (idx_step=0;idx_step<count_step;idx_step++)
				{
					V3DLONG threshold_exemplarRegion = value_exemplar-idx_step;
					poss_exemplarRegionNew=this->regionGrowOnPos(pos_exemplar, threshold_exemplarRegion, this->size_page/1000, this->Image1D_mask);
					if (poss_exemplarRegionNew.size()<default_threshold_regionSize) {break; }
					this->poss2Image1D(poss_exemplarRegionNew, this->Image1D_mask, const_max_voxelValue);
					pos_massCenterNew = this->getCenterByMass(poss_exemplarRegionNew);
					double value_centerMovement = this->getEuclideanDistance2(pos_massCenterOld, pos_massCenterNew);
					if (value_centerMovement>1)	{break;}
					pos_massCenterOld = pos_massCenterNew;
					poss_exemplarRegionOld = poss_exemplarRegionNew;
				}
				if (idx_step<1) {continue; } //failed;
				if (poss_exemplarRegionOld.size()<default_threshold_regionSize) {continue; } //failed;
				this->poss2Image1D(poss_exemplarRegionOld, this->Image1D_mask, 0);
				this->possVct_exemplarRegion.push_back(poss_exemplarRegionOld);
				poss_exemplarNew.push_back(pos_massCenterOld);
				this->thresholds_voxelValue.push_back(this->getMin(poss_exemplarRegionOld));
				V3DLONG count_voxel = poss_exemplarRegionOld.size();
				V3DLONG size_upper = count_voxel*this->multiplier_uThresholdRegionSize;
				V3DLONG size_lower = count_voxel*this->multiplier_thresholdRegionSize;
				if (size_lower<default_threshold_regionSize) {size_lower=default_threshold_regionSize;}
				this->thresholds_regionSize.push_back(size_lower);
				this->uThresholds_regionSize.push_back(size_upper);
				//shape property;
				vector<V3DLONG> boundBox_exemplarRegion = this->getBoundBox(poss_exemplarRegionOld);
				V3DLONG radius_exemplarRegion = getMinDimension(boundBox_exemplarRegion)/2;
				this->thresholds_radius.push_back(radius_exemplarRegion);
				vector<V3DLONG> xyz_exemplarRegionCenter = this->index2Coordinate(pos_massCenterOld);
				vector<vector<double> > valuesVct_shapeStatExemplarRegion = this->getShapeStat(xyz_exemplarRegionCenter[0], xyz_exemplarRegionCenter[1], xyz_exemplarRegionCenter[2], radius_exemplarRegion);
				this->valueVctVct_exemplarShapeStat.push_back(valuesVct_shapeStatExemplarRegion);
			}
			if (this->possVct_exemplarRegion.empty())
			{
				return false;
			}
			this->poss_exemplar.clear();
			this->poss_exemplar = poss_exemplarNew;
			this->LandmarkList_exemplar = this->poss2LandMarkList(poss_exemplar);
			count_exemplar = poss_exemplar.size();
			memset(this->Image1D_exemplar, 0, this->size_page3);
			this->possVct2Image1DC(this->possVct_exemplarRegion, this->Image1D_exemplar);
			vector<V3DLONG> mapping_exemplar = this->sort(this->thresholds_voxelValue); // in ascending order;
			V3DLONG count_seedCategory = this->possVct_seed.size();
			unsigned char ** masks_page = this->memory_allocate_uchar2D(count_exemplar, this->size_page);
			for (V3DLONG idx_exemplar=0;idx_exemplar<count_exemplar;idx_exemplar++)
			{
				memset(masks_page[idx_exemplar], const_max_voxelValue, this->size_page);
				for (V3DLONG i=0;i<this->size_page;i++)
				{
					if (this->Image1D_mask[i]<1) {masks_page[idx_exemplar][i]=0;}
				}
			}
			for (V3DLONG idx_seedCategoy=0;idx_seedCategoy<count_seedCategory;idx_seedCategoy++)
			{
				V3DLONG count_seed = this->possVct_seed[idx_seedCategoy].size();
				cout<<"at value: "<<(255-idx_seedCategoy)<<", totally: "<<count_seed<<" seeds;"<<endl;
				for (V3DLONG idx_seed=0;idx_seed<count_seed;idx_seed++)
				{
					V3DLONG pos_seed = this->possVct_seed[idx_seedCategoy][idx_seed];
					for (V3DLONG idx_exemplar=0;idx_exemplar<count_exemplar;idx_exemplar++)
					{
						if (masks_page[idx_exemplar][pos_seed]<1) {continue;}
						V3DLONG value_seed = this->Image1D_page[pos_seed];
						V3DLONG idx_exemplarMapped = mapping_exemplar[idx_exemplar];
						V3DLONG threshold_voxelValue = this->thresholds_voxelValue[idx_exemplarMapped];
						V3DLONG threshold_regionSize = this->thresholds_regionSize[idx_exemplarMapped];
						if (value_seed<threshold_voxelValue) {break;}
						V3DLONG uThreshold_regionSize = this->uThresholds_regionSize[idx_exemplarMapped];
						V3DLONG threshold_radius = this->thresholds_radius[idx_exemplarMapped];
						vector<V3DLONG> poss_region = this->regionGrowOnPos(pos_seed, threshold_voxelValue, uThreshold_regionSize, masks_page[idx_exemplar]);
						V3DLONG count_voxel = poss_region.size();
						if (count_voxel>uThreshold_regionSize) {continue; }
						else if (count_voxel<threshold_regionSize) {break; }
						vector<V3DLONG> boundBox_region = this->getBoundBox(poss_region);
						V3DLONG size_radius = this->getMinDimension(boundBox_region)/2;
						if (size_radius<(threshold_radius*this->multiplier_thresholdRegionSize)) {break;}
						else if (size_radius>(threshold_radius*this->multiplier_uThresholdRegionSize)) {continue;}
						V3DLONG pos_center = this->getCenterByMass(poss_region);
						vector<V3DLONG> xyz_center = this->index2Coordinate(pos_center);
						V3DLONG x = V3DLONG(xyz_center[0] + 0.5); V3DLONG y = V3DLONG(xyz_center[1] + 0.5); V3DLONG z = V3DLONG(xyz_center[2] + 0.5);
						vector<vector<double> > valuesVct_regionShapeStat = this->getShapeStat(x, y, z, size_radius); //consisted of 3 vectors with length 4;
						if (valuesVct_regionShapeStat.empty()) {	break;}
						vector<double> values_PC1 = valuesVct_regionShapeStat[0]; vector<double> values_PC2 = valuesVct_regionShapeStat[1]; vector<double> values_PC3 = valuesVct_regionShapeStat[2];
						bool is_passedShapeTest = true;
						for (int m=0; m<4; m++)
						{
							double value_anisotropy = valueVctVct_exemplarShapeStat[idx_exemplarMapped][0][m];
							if (fabs(values_PC1[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
							{is_passedShapeTest = false; break;}
							value_anisotropy = valueVctVct_exemplarShapeStat[idx_exemplarMapped][1][m];
							if (fabs(values_PC2[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
							{is_passedShapeTest = false; break;}
							value_anisotropy = valueVctVct_exemplarShapeStat[idx_exemplarMapped][2][m];
							if (fabs(values_PC3[m]-value_anisotropy)>(this->threshold_deltaShapeStat*value_anisotropy))
							{is_passedShapeTest = false; break;}
						}
						if (is_passedShapeTest)
						{
							this->possVct_segmentationResult.push_back(poss_region);
							this->poss_segmentationResultCenter.push_back(pos_center);
							for (V3DLONG idx_exemplar=0;idx_exemplar<count_exemplar;idx_exemplar++)
							{
								this->poss2Image1D(poss_region, masks_page[idx_exemplar], 0);
							}
							for (V3DLONG i=0;i<count_voxel;i++)
							{
								vector<V3DLONG> xyz_i = this->index2Coordinate(poss_region[i]);
								this->Image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
							}
							break;
						}
					}
				}
			}
			memset(this->Image1D_mask, const_max_voxelValue, this->size_page);
			this->possVct_segmentationResult = this->mergePossVector(this->possVct_exemplarRegion, this->possVct_segmentationResult);
			this->possVct2Image1D(this->possVct_segmentationResult, this->Image1D_mask, 0);
			this->poss_segmentationResultCenter = this->mergePoss(this->poss_exemplar, this->poss_segmentationResultCenter);
			this->LandmarkList_segmentationResult = this->poss2LandMarkList(this->poss_segmentationResultCenter);
			this->possVct2Image1DC(this->possVct_segmentationResult, this->Image1D_segmentationResult);
			this->memory_free_uchar2D(masks_page, count_exemplar);
			return true;
		}
		#pragma endregion

		#pragma region "regionGrow"
		void categorizeVoxelsByValue() //will only consider voxels with value higher than threshold_global;
		{
			this->possVct_seed.clear();
			vector<V3DLONG> poss_empty (0,0);
			for (V3DLONG i=default_threshold_global;i<const_length_histogram;i++)
			{
				this->possVct_seed.push_back(poss_empty);
			}
			for (V3DLONG i=0;i<this->size_page;i++)
			{
				V3DLONG value_i = this->Image1D_page[i];
				if (value_i>default_threshold_global)
				{
					V3DLONG offset_i = const_max_voxelValue-value_i;
					this->possVct_seed[offset_i].push_back(i);
				}
			}
		}

		vector<V3DLONG> regionGrowOnPos(V3DLONG _pos_seed, double _threshold_voxelValue, 
			V3DLONG _uThreshold_regionSize, unsigned char* _mask_input)
		{
			vector<V3DLONG> poss_result;
			vector<V3DLONG> poss_growing;
			poss_growing.push_back(_pos_seed);
			poss_result.push_back(_pos_seed);
			V3DLONG count_voxel = 1;
			_mask_input[_pos_seed] = 0; //scooped;
			while (true)
			{
				if (poss_growing.empty()) //growing complete;
				{
					return poss_result;
				}
				V3DLONG pos_current = poss_growing.back();
				poss_growing.pop_back();
				vector<V3DLONG> xyz_current = this->index2Coordinate(pos_current);
				for (int j=0;j<const_count_neighbors;j++)
				{
					if (((xyz_current[0]+point_neighborRelative[j].x)<0)||((xyz_current[0]+point_neighborRelative[j].x)>=this->dim_X)||((xyz_current[1]+point_neighborRelative[j].y)<0) || ((xyz_current[1]+point_neighborRelative[j].y)>=this->dim_Y)||((xyz_current[2]+point_neighborRelative[j].z)<0) || ((xyz_current[2]+point_neighborRelative[j].z)>=this->dim_Z))
					{
						//invalide anyway;
					}
					else
					{
						V3DLONG pos_neighbor = pos_current+poss_neighborRelative[j];
						if (this->checkValidity(pos_neighbor)) //prevent it from going out of bounds;
						{
							if (_mask_input[pos_neighbor]>0) //available only;
							{
								V3DLONG value_neighbor = this->Image1D_page[pos_neighbor];
								if (value_neighbor>_threshold_voxelValue)
								{
									_mask_input[pos_neighbor] = 0; //scooped;
									poss_growing.push_back(pos_neighbor);
									poss_result.push_back(pos_neighbor);
									count_voxel++;
									if (count_voxel>(_uThreshold_regionSize)) //too large, +2 here so it won't pass the size filter later;
									{
										return poss_result;
									}
								}
							}
						}
					}
				}
			}
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

		LandmarkList poss2LandMarkList(vector<V3DLONG> vct_index)
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

		void initializeConstants()
		{
			this->poss_neighborRelative.clear();
			this->point_neighborRelative.clear();
			this->colors_simpleTable.clear();

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

			vector<V3DLONG> color_tmp(3, 0); color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 0;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 0;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 0; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 0;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 0;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 0;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 0; color_tmp[1] = 128; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 128;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
			color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 255;
			this->colors_simpleTable.push_back(color_tmp);
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

		void poss2Image1D(vector<V3DLONG> poss_input, unsigned char* Image1D_input, V3DLONG value_input)
		{
			V3DLONG size_input = poss_input.size();
			for (int i=0;i<size_input;i++) {Image1D_input[poss_input[i]]=value_input; }
		}

		void poss2Image1D(vector<V3DLONG> poss_input, V3DLONG* Image1D_input, V3DLONG value_input)
		{
			V3DLONG size_input = poss_input.size();
			for (int i=0;i<size_input;i++) {Image1D_input[poss_input[i]]=value_input; }
		}

		void possVct2Image1D(vector<vector<V3DLONG> > possVct_input, unsigned char* Image1D_input, V3DLONG value_input)
		{
			V3DLONG count_region = possVct_input.size();
			for (V3DLONG i=0;i<count_region;i++) {poss2Image1D(possVct_input[i], Image1D_input, value_input);}
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

		double getMax(vector<V3DLONG> poss_input)
		{
			double max_result = -INF;
			V3DLONG count_input = poss_input.size();
			for (V3DLONG i=0;i<count_input;i++)
			{
				double value_i = this->Image1D_page[poss_input[i]];
				if (max_result<value_i) {max_result=value_i;}
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

		double getMin(vector<V3DLONG> poss_input)
		{
			double min_result = INF;
			V3DLONG count_input = poss_input.size();
			for (V3DLONG i=0;i<count_input;i++)
			{
				double value_i = this->Image1D_page[poss_input[i]];
				if (min_result>value_i) {min_result=value_i;}
			}
			return min_result;
		}

		vector<V3DLONG> sort(vector<V3DLONG> values_input)
		{
			V3DLONG count_input = values_input.size();
			vector<V3DLONG> mapping_result;
			for (V3DLONG i=0;i<count_input;i++)
			{
				V3DLONG value_i = values_input[i];
				V3DLONG count_greater = 0;
				for (V3DLONG j=0;j<count_input;j++)
				{
					V3DLONG value_j = values_input[j];
					if (value_i<value_j) {count_greater++;}
					if ((value_i==value_j)&&(j>i)) {count_greater++;}
				}
				mapping_result.push_back(count_greater);
			}
			return mapping_result;
		}

		void swap (V3DLONG& x, V3DLONG& y)
		{
			V3DLONG tmp = x;	x = y; y = tmp;
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

		double getEuclideanDistance2(V3DLONG pos_input1, V3DLONG pos_input2)
		{
			if ((pos_input1<0)||(pos_input2<0)) {return 0;}
			double result = 0;
			vector<V3DLONG> vct_xyz1 = this->index2Coordinate(pos_input1);
			vector<V3DLONG> vct_xyz2 = this->index2Coordinate(pos_input2);
			result+=(vct_xyz1[0]-vct_xyz2[0])*(vct_xyz1[0]-vct_xyz2[0]);
			result+=(vct_xyz1[1]-vct_xyz2[1])*(vct_xyz1[1]-vct_xyz2[1]);
			result+=(vct_xyz1[2]-vct_xyz2[2])*(vct_xyz1[2]-vct_xyz2[2]);
			return result;
		}

		double getEuclideanDistance(V3DLONG pos_input1, V3DLONG pos_input2)
		{
			if ((pos_input1<0)||(pos_input2<0)) {return 0;}
			double result = 0;
			vector<V3DLONG> vct_xyz1 = this->index2Coordinate(pos_input1);
			vector<V3DLONG> vct_xyz2 = this->index2Coordinate(pos_input2);
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
			if (count_voxel<1) {return -1;}
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
			const double min_centerDistance, vector<V3DLONG> &poss_center)
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
				if (sums_mass[i]>0)
				{
					possVct_result.push_back(possVct_resultWithEmpty[i]);
					poss_center.push_back(class_segmentationMain::coordinate2Index(mean_center[i].x+min_X, mean_center[i].y+min_Y, mean_center[i].z+min_Z, offset_Yglobal, offset_Zglobal));
				}
			}
			return possVct_result;
		}

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

		static unsigned char **memory_allocate_uchar2D(const V3DLONG i_size,const V3DLONG j_size)
		{
			unsigned char **ptr_result;
			V3DLONG i;
			ptr_result=(unsigned char **) calloc(i_size,sizeof(unsigned char *));
			for(i=0;i<i_size;i++)
			{
				ptr_result[i]=(unsigned char *) calloc(j_size, sizeof(unsigned char));
			}
			return(ptr_result);
		}

		static void memory_free_uchar2D(unsigned char **ptr_input, const V3DLONG i_size)
		{
			V3DLONG i;
			for(i=0;i<i_size;i++)
				free(ptr_input[i]);
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

		static V3DLONG *memory_allocate_int1D(const V3DLONG i_size)
		{
			V3DLONG *ptr_result;
			ptr_result = (V3DLONG *) calloc(i_size, sizeof(V3DLONG));
			return(ptr_result);
		}

		static void memory_free_int1D(int *ptr_input, const int i_size)
		{
			free(ptr_input);
		}

		static void memory_free_double2D(bool **ptr_input, const int i_size)
		{
			int i;

			for(i=0;i<i_size;i++)
				free(ptr_input[i]);
			free(ptr_input);
		}

		static unsigned char *memory_allocate_uchar1D(const int i_size)
		{
			unsigned char *ptr_result;
			ptr_result = (unsigned char *) calloc(i_size, sizeof(unsigned char));
			return(ptr_result);
		}

		static void memory_free_uchar1D(unsigned char *ptr_input)
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

		#pragma region "smoothing and filtering"
		void filter_Median(V3DLONG _count_medianFilterRadius)
		{
			if (_count_medianFilterRadius < 1) {return;}
			unsigned char *arr;
			int ii,jj;
			int size = (2*_count_medianFilterRadius+1)*(2*_count_medianFilterRadius+1)*(2*_count_medianFilterRadius+1);
			arr = new unsigned char[size];
			unsigned char *Image1D_output = memory_allocate_uchar1D(this->size_page);
			for(V3DLONG iz = 0; iz < this->dim_Z; iz++)
			{
				cout<<"\r median filter, "<<(double)(iz + 1)*100/this->dim_Z<<"% completed;"<<flush;
				V3DLONG offsetk = iz*this->offset_Z;
				for(V3DLONG iy = 0; iy < this->dim_Y; iy++)
				{
					V3DLONG offsetj = iy*this->offset_Y;
					for(V3DLONG ix = 0; ix < this->dim_X; ix++)
					{
						V3DLONG xb = ix-_count_medianFilterRadius; if(xb<0) xb = 0;
						V3DLONG xe = ix+_count_medianFilterRadius; if(xe>=this->dim_X-1) xe = this->dim_X-1;
						V3DLONG yb = iy-_count_medianFilterRadius; if(yb<0) yb = 0;
						V3DLONG ye = iy+_count_medianFilterRadius; if(ye>=this->dim_Y-1) ye = this->dim_Y-1;
						V3DLONG zb = iz-_count_medianFilterRadius; if(zb<0) zb = 0;
						V3DLONG ze = iz+_count_medianFilterRadius; if(ze>=this->dim_Z-1) ze = this->dim_Z-1;
						ii = 0;
						for(V3DLONG k=zb; k<=ze; k++)
						{
							V3DLONG offsetkl = k*this->offset_Z;
							for(V3DLONG j=yb; j<=ye; j++)
							{
								V3DLONG offsetjl = j*this->offset_Y;
								for(V3DLONG i=xb; i<=xe; i++)
								{
									unsigned char dataval = this->Image1D_page[offsetkl + offsetjl + i];
									arr[ii] = dataval;
									if (ii>0)
									{
										jj = ii;
										while(jj > 0 && arr[jj-1]>arr[jj])
										{
											unsigned char tmp = arr[jj];
											arr[jj] = arr[jj-1];
											arr[jj-1] = tmp;
											jj--;
										}
									}
									ii++;
								}
							}
						}
						V3DLONG index_pim = offsetk + offsetj + ix;
						Image1D_output[index_pim] = arr[int(0.5*ii)+1];
					}
				}
			}
			for (V3DLONG i=0;i<this->size_page;i++)
			{
				this->Image1D_page[i] = Image1D_output[i];
			}
			delete [] arr;
		}

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

		void smooth_GVFkernal(V3DLONG count_smoothRadius)
		{
			unsigned char* Image1D_update = memory_allocate_uchar1D(this->size_page);
			V3DLONG pos_voxel;
			V3DLONG pos_neighbor1;
			V3DLONG pos_neighbor2;
			V3DLONG pos_neighbor3;
			V3DLONG pos_neighbor4;
			V3DLONG pos_neighbor5;
			V3DLONG pos_neighbor6;
			for(int i=0;i<count_smoothRadius;i++)
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
							Image1D_update[pos_voxel] = 0.4*this->Image1D_page[pos_voxel]+0.1*(this->Image1D_page[pos_neighbor1]+this->Image1D_page[pos_neighbor2]+this->Image1D_page[pos_neighbor3]+this->Image1D_page[pos_neighbor4]+this->Image1D_page[pos_neighbor5]+this->Image1D_page[pos_neighbor6]);
						}
					}
				}
				for (int i=0;i<this->size_page;i++)
				{
					this->Image1D_page[i] = Image1D_update[i];
				}
			}
			return;
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
	Q_OBJECT Q_INTERFACES(V3DPluginInterface2_1);
	float getPluginVersion() const {return 1.1f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

	#pragma region "interface"
	bool interface_run(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent)
	{
		v3dhandle v3dhandle_currentWindow = _V3DPluginCallback2_currentCallback.currentImageWindow();
		if (!v3dhandle_currentWindow) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		Image4DSimple* Image4DSimple_current = _V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
		if (!Image4DSimple_current) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
		if (count_totalBytes<1) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
		unsigned char* Image1D_current = Image4DSimple_current->getRawData();
		QString name_currentWindow = _V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
		V3DLONG dim_X = Image4DSimple_current->getXDim(); V3DLONG dim_Y = Image4DSimple_current->getYDim();
		V3DLONG dim_Z = Image4DSimple_current->getZDim(); V3DLONG dim_C = Image4DSimple_current->getCDim();
		V3DLONG size_image = dim_X*dim_Y*dim_Z*dim_C;
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
		dialogRun dialogRun1(_V3DPluginCallback2_currentCallback, _QWidget_parent, dim_C);
		bool is_success = false;
		if (this->class_segmentationMain1.possVct_segmentationResult.size()<1)
		{
			if (dialogRun1.exec()!=QDialog::Accepted) {return false;}
			int idx_shape; //get shape paramters;
			if (dialogRun1.shape_type_selection == sphere) {idx_shape = 1;}
			else if (dialogRun1.shape_type_selection == cube) {idx_shape = 0;}
			is_success = this->class_segmentationMain1.control_run(Image1D_current, dim_X, dim_Y, dim_Z, dialogRun1.channel_idx_selection, LandmarkList_current,
				idx_shape, dialogRun1.shape_para_delta, dialogRun1.shape_multiplier_thresholdRegionSize, dialogRun1.shape_multiplier_uThresholdRegionSize, name_currentWindow);
		}
		else
		{
			is_success = this->class_segmentationMain1.control_run(this->class_segmentationMain1.Image1D_page, this->class_segmentationMain1.dim_X, 
				this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, this->class_segmentationMain1.idx_channel, 
				LandmarkList_current, this->class_segmentationMain1.idx_shape, 
				this->class_segmentationMain1.threshold_deltaShapeStat, this->class_segmentationMain1.multiplier_thresholdRegionSize,
				this->class_segmentationMain1.multiplier_uThresholdRegionSize, this->class_segmentationMain1.name_currentWindow);
		}
		if (is_success)
		{
			visualizationImage1D(this->class_segmentationMain1.Image1D_exemplar, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Exemplar");
			visualizationImage1D(this->class_segmentationMain1.Image1D_segmentationResult, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, "Result");
			//visualizationImage1D(this->class_segmentationMain1.Image1D_mask, this->class_segmentationMain1.dim_X, this->class_segmentationMain1.dim_Y, this->class_segmentationMain1.dim_Z, 1, _V3DPluginCallback2_currentCallback, "Mask");
			v3dhandleList v3dhandleList_current = _V3DPluginCallback2_currentCallback.getImageWindowList();
			V3DLONG count_v3dhandle = v3dhandleList_current.size();
			QString name_result = "Result";
			QString name_exemplar = "Exemplar";
			for (V3DLONG i=0;i<count_v3dhandle;i++)
			{
				if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(this->class_segmentationMain1.name_currentWindow))
				{
					_V3DPluginCallback2_currentCallback.setLandmark(v3dhandleList_current[i], this->class_segmentationMain1.LandmarkList_segmentationResult);
					_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				}
				if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(name_result))
				{
					//_V3DPluginCallback2_currentCallback.setLandmark(v3dhandleList_current[i], this->class_segmentationMain1.LandmarkList_exemplar);
					//_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				}
				if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(name_exemplar))
				{
					_V3DPluginCallback2_currentCallback.setLandmark(v3dhandleList_current[i], this->class_segmentationMain1.LandmarkList_exemplar);
					_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				}
			}
			return true;
		}
		else
		{
			QString name_result = "Result";
			v3dhandleList v3dhandleList_current = _V3DPluginCallback2_currentCallback.getImageWindowList();
			V3DLONG count_v3dhandle = v3dhandleList_current.size();
			for (V3DLONG i=0;i<count_v3dhandle;i++)
			{
				if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(name_result))
				{
					LandmarkList LandmarkList_empty;
					_V3DPluginCallback2_currentCallback.setLandmark(v3dhandleList_current[i], LandmarkList_empty);
					_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				}
			}
			v3d_msg("Warning: no exemplar defined, please re-select the exemplars!");
			return false;
		}
	}

	void visualizationImage1D(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QString string_windowName)
	{
		V3DLONG size_page = dim_X*dim_Y*dim_Z*dim_C;
		unsigned char* Image1D_tmp = class_segmentationMain::memory_allocate_uchar1D(size_page);
		for (V3DLONG i=0;i<size_page;i++)
		{
			Image1D_tmp[i] = Image1D_input[i];
		}
		Image4DSimple Image4DSimple_temp;
		Image4DSimple_temp.setData(Image1D_tmp, dim_X, dim_Y, dim_Z, dim_C, V3D_UINT8);

		v3dhandleList v3dhandleList_current = _V3DPluginCallback2_currentCallback.getImageWindowList();
		V3DLONG count_v3dhandle = v3dhandleList_current.size();
		bool is_found = false;
		for (V3DLONG i=0;i<count_v3dhandle;i++)
		{
			if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(string_windowName))
			{
				_V3DPluginCallback2_currentCallback.setImage(v3dhandleList_current[i], &Image4DSimple_temp);
				_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				is_found=true;
				break;
			}
		}
		if (!is_found)
		{
			v3dhandle v3dhandle_main = _V3DPluginCallback2_currentCallback.newImageWindow();
			_V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
			_V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
			_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
		}
	}
	#pragma endregion
};



#endif