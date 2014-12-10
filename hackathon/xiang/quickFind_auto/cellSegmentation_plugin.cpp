/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */

#pragma region "includes and function defination"
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
Q_EXPORT_PLUGIN2(cellSegmentation, cellSegmentation);
bool interface_run(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent);
void visualizationImage1D(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QString string_windowName);
void visualizationImage1D(V3DLONG* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QString string_windowName);
void landmarkList2file(char* _file_input, LandmarkList _landmarkList_input);
void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
#pragma endregion

#pragma region "class: class_segmentationMain"
class class_segmentationMain
{
	#pragma region "class member"
	public:
		//constant
		vector<V3DLONG> possR_neighbor;
		vector<vector<V3DLONG> > xyzsR_neighbor;
		vector<vector<V3DLONG> > colors_simple;
		vector<vector<V3DLONG> > possVct_seed;
		V3DLONG uThreshold_regionSizeGlobal;
		//Input or directly derived;
		unsigned char* image1D_page;
		V3DLONG* image1D_mask;
		unsigned char*** image3D_page;
		V3DLONG dim_X;
		V3DLONG dim_Y;
		V3DLONG dim_Z;
		V3DLONG size_page;
		V3DLONG size_page3;
		V3DLONG offset_channel;
		V3DLONG offset_Z;
		V3DLONG offset_Y;
		int idx_channel;
		QString name_currentWindow;
		//detection;
		vector<vector<V3DLONG> > possVct_result;
		//unsigned char* image1Dc_result;
		LandmarkList landmarkList_result;
		vector<V3DLONG> centers_result;
		#pragma endregion
		
	class_segmentationMain() {}
	~class_segmentationMain()
	{
		memory_free_int1D(this->image1D_mask);
		memory_free_uchar1D(this->image1D_page);
		//memory_free_uchar1D(this->image1Dc_result);
		memory_free_uchar3D(this->image3D_page, this->dim_Z, this->dim_Y);
	}

	#pragma region "control-run"
	bool control_run(unsigned char* _Image1D_original, V3DLONG _dim_X, V3DLONG _dim_Y, V3DLONG _dim_Z ,
		int _idx_channel, int _idx_shape, double _delta_shapeStat,
		V3DLONG _count_exemplar, V3DLONG _threshold_intensity, QString _name_currentWindow,
		V3DLONG _uThreshold_centerMovement1, V3DLONG _uThreshold_centerMovement2,
		V3DLONG _threshold_centerDistance2, V3DLONG _threshold_regionSize,
		double _multiplier_uThresholdRegionSizeGlobal, double _multiplier_uThresholdRegionSizeExemplar, 
		V3DPluginCallback2 & _V3DPluginCallback2_currentCallback,  QWidget * _QWidget_parent)
	{
		cout<<"detection starting..."<<endl;
		this->dim_X = _dim_X; this->dim_Y = _dim_Y; this->dim_Z = _dim_Z; this->idx_channel = _idx_channel;
		cout<<"dim_X: "<<dim_X<<" dim_Y: "<<dim_X<<" dim_Z: "<<dim_Z<<endl;
		this->size_page = dim_X*dim_Y*dim_Z;
		this->size_page3 = this->size_page+this->size_page+this->size_page;
		this->offset_channel = (idx_channel-1)*size_page;
		cout<<"offset_channel: "<<offset_channel<<endl;
		this->offset_Z = dim_X*dim_Y;
		this->offset_Y = dim_X;
		this->image1D_page = memory_allocate_uchar1D(this->size_page);
		if (this->image1D_page == NULL) {return false;}
		this->image1D_mask = memory_allocate_int1D(this->size_page);
		if (this->image1D_mask == NULL) {return false;}
		V3DLONG* image1D_maskTmp = memory_allocate_int1D(size_page);
		if (image1D_maskTmp == NULL) {return false;}
		this->image3D_page = memory_allocate_uchar3D(this->dim_Z, this->dim_Y, this->dim_X);
		if (this->image3D_page == NULL) {return false;}
		//this->image1Dc_result = memory_allocate_uchar1D(this->size_page3);
		//if (this->image1Dc_result == NULL) {return false;}
		cout<<"memory allocation finished;"<<endl;
		for (V3DLONG z=0;z<dim_Z;z++)
		{
			for (V3DLONG y=0;y<dim_Y;y++)
			{
				for (V3DLONG x=0;x<dim_X;x++)
				{
					V3DLONG pos_current = this->coordinate2pos(x, y, z);
					this->image1D_page[pos_current] = _Image1D_original[pos_current+offset_channel];
					this->image3D_page[z][y][x] = this->image1D_page[pos_current];
				}
			}
		}
		cout<<"current threshold: "<<_threshold_intensity<<endl;
		this->thresholdForCurrentPage(_threshold_intensity);
		cout<<"thresholding finished;"<<endl;
		this->categorizeVoxelsByIntensity(_threshold_intensity);
		this->name_currentWindow = _name_currentWindow;
		this->initializeConstants();
		cout<<"initializeConstants() finished;"<<endl;
		V3DLONG uThreshold_regionSizeGlobal = this->size_page*_multiplier_uThresholdRegionSizeGlobal;
		V3DLONG count_seedCategory = this->possVct_seed.size();
		V3DLONG count_result = 0;
		V3DLONG count_iteration = 0;
		V3DLONG start_seedCategory=0;
		V3DLONG start_seed=0;
		while (count_iteration<const_max_iteration)
		{
			vector<V3DLONG > thresholds_intensity;
			//vector<V3DLONG > uThresholds_intensity;
			vector<V3DLONG > uThresholds_size;
			V3DLONG counts_learn = 0;
			vector<vector<vector<double> > > shapeStats_exemplar;
			V3DLONG count_exemplar=0;
			count_iteration++;
			cout<<"current iteration: "<<count_iteration<<endl;
			for (V3DLONG idx_seedCategoy=0;idx_seedCategoy<count_seedCategory;idx_seedCategoy++)
			{
				V3DLONG count_seed = this->possVct_seed[idx_seedCategoy].size();
				if (counts_learn>const_max_learnIteration) { break;	}
				//cout<<"current intensity: "<<(const_max_voxelValue-idx_seedCategoy)<<endl;
				if (idx_seedCategoy<start_seedCategory) {continue;}
				start_seedCategory=idx_seedCategoy;
				start_seed=0;
				for (V3DLONG idx_seed=start_seed;idx_seed<count_seed;idx_seed++)
				{
					if (idx_seed<start_seed) {continue;}
					start_seed=idx_seed;
					if (counts_learn>const_max_learnIteration) { break;	}
					if (count_exemplar>=_count_exemplar) {break;}
					V3DLONG pos_seed = this->possVct_seed[idx_seedCategoy][idx_seed];
					//cout<<"pos_seed: "<<pos_seed<<endl;
					if (idx_seed<start_seed) {continue;}
					start_seed=idx_seed;
					if (this->image1D_mask[pos_seed]>0) {continue;}
					
					counts_learn++;
					V3DLONG pos_exemplar=pos_seed;
					V3DLONG intensity_exemplar = this->image1D_page[pos_exemplar];
					//cout<<"intensity_exemplar: "<<intensity_exemplar<<endl;
					V3DLONG count_step = intensity_exemplar-_threshold_intensity;
					V3DLONG center_exemplar = -1;
					vector<V3DLONG> poss_exemplarNew;
					vector<V3DLONG> poss_exemplar;
					//vector<vector<double> > shapeStat_exemplarOld;
					//vector<vector<double> > shapeStat_exemplarNew;
					V3DLONG idx_step = 0;
					//vector<V3DLONG> boundBox_exemplar;
					//V3DLONG radius_exemplar;
					for (idx_step=0;idx_step<count_step;idx_step++)
					{
						V3DLONG threshold_exemplarRegion = intensity_exemplar-idx_step;
						//if (image1D_visited[pos_seed]>=threshold_exemplarRegion) { break; }
						poss_exemplarNew=this->regionGrowOnPos(pos_exemplar, threshold_exemplarRegion, this->image1D_mask, count_result+1);
						V3DLONG size_exemplarNew = poss_exemplarNew.size();
						//cout<<"size_exemplarNew: "<<size_exemplarNew<<endl;
						this->poss2Image1D(poss_exemplarNew, this->image1D_mask, 0);
						if (size_exemplarNew<_threshold_regionSize) {continue; }
						V3DLONG center_new = this->getCenterByMass(poss_exemplarNew);
						//cout<<"center_new: "<<center_new<<endl;
						/*boundBox_exemplar = this->getBoundBox(poss_exemplarNew);
						radius_exemplar = this->getMaxDimension(boundBox_exemplar)/2;
						if (radius_exemplar<(const_shapeStatCount+1))
						{
							continue;
						}*/
						/*else if (radius_exemplar==const_shapeStatCount)
						{
							radius_exemplar = const_shapeStatCount+1;
						}*/
						//vector<V3DLONG> centerXYZ_exemplar = this->pos2xyz(center_new);
						//shapeStat_exemplarNew = this->getShapeStat(centerXYZ_exemplar[0], centerXYZ_exemplar[1], centerXYZ_exemplar[2], radius_exemplar);
						if (center_exemplar>0)
						{
							//bool is_passedShapeTest = true;
							//for (V3DLONG m=0; m<const_shapeStatCount; m++)
							//{
							//	double value_anisotropyNew = shapeStat_exemplarNew[0][m];
							//	double value_anisotropyOld = shapeStat_exemplarOld[0][m];
							//	//if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*min(value_anisotropyOld, value_anisotropyNew)))
							//	if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*value_anisotropyOld, value_anisotropyNew))
							//	{is_passedShapeTest = false; break;}
							//	value_anisotropyNew = shapeStat_exemplarNew[1][m];
							//	value_anisotropyOld = shapeStat_exemplarOld[1][m];
							//	//if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*min(value_anisotropyOld, value_anisotropyNew)))
							//	if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*value_anisotropyOld, value_anisotropyNew))
							//	{is_passedShapeTest = false; break;}
							//	value_anisotropyNew = shapeStat_exemplarNew[2][m];
							//	value_anisotropyOld = shapeStat_exemplarOld[2][m];
							//	//if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*min(value_anisotropyOld, value_anisotropyNew)))
							//	if (fabs(value_anisotropyNew-value_anisotropyOld)>(_delta_shapeStat*value_anisotropyOld, value_anisotropyNew))
							//	{is_passedShapeTest = false; break;}
							//}
							//if (!is_passedShapeTest) {break;}
							double distance_centerMovement1 = this->getEuclideanDistance2(center_new, center_exemplar);
							if (distance_centerMovement1>_uThreshold_centerMovement1) {break;}
							double distance_centerMovement2 = this->getEuclideanDistance2(center_new, pos_exemplar);
							if (distance_centerMovement2>_uThreshold_centerMovement2) {break;}
						}
						if (size_exemplarNew>uThreshold_regionSizeGlobal)
						{
							//pass the center test yet too large;
							for (V3DLONG i=0;i<size_exemplarNew;i++)
							{
								vector<V3DLONG> xyz_i = this->pos2xyz(poss_exemplarNew[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
							}
							this->poss2Image1D(poss_exemplarNew, this->image1D_mask, INF); //removed;
							break;
						}
						//shapeStat_exemplarOld = shapeStat_exemplarNew;
						center_exemplar = center_new;
						poss_exemplar = poss_exemplarNew;
					}
					if (idx_step<1) {continue; } //failed;
					V3DLONG threshold_intensity = intensity_exemplar-idx_step+1;
					//vector<V3DLONG> poss_exemplar = this->regionGrowOnPos(pos_exemplar, threshold_intensity, this->image1D_mask, count_result+1);
					//this->poss2Image1D(poss_exemplarNew, this->image1D_mask, 0);
					V3DLONG size_exemplar = poss_exemplar.size();
					//cout<<"size_exemplar: "<<size_exemplar<<endl;
					if (size_exemplar>uThreshold_regionSizeGlobal)
					{
						continue; //not passed the center test, will not removed from image;
					} //failed;
					if (size_exemplar<_threshold_regionSize) { continue;} //failed;
					vector<V3DLONG> boundBox_exemplar = this->getBoundBox(poss_exemplar);
					V3DLONG radius_exemplar = ceil((double)getMaxDimension(boundBox_exemplar)/2);
					//cout<<"radius_exemplar: "<<radius_exemplar<<endl;
					if (radius_exemplar<(const_shapeStatCount+1)) { continue;} //radius too small, failed;
					vector<V3DLONG> centerXYZ_exemplar = this->pos2xyz(center_exemplar);
					V3DLONG center_geometry=getGeometryCenter(poss_exemplar);
					//cout<<"center_geometry: "<<center_geometry<<endl;
					V3DLONG label_center=this->image1D_mask[center_geometry];
					//cout<<"label_center: "<<label_center<<endl;
					if ((label_center>0)&&(label_center<INF))//donut, fill it;
					{
						this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_exemplar);
						this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
						for (V3DLONG i=0;i<size_exemplar;i++)
						{
							vector<V3DLONG> xyz_i = this->pos2xyz(poss_exemplar[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
						}
						this->poss2Image1D(poss_exemplar, this->image1D_mask, label_center); //merged;
						continue;
					}
					//cout<<"center_exemplar: "<<center_exemplar<<endl;
					label_center=this->image1D_mask[center_exemplar];
					//cout<<"label_center: "<<label_center<<endl;
					if ((label_center>0)&&(label_center<INF))//donut, fill it;
					{
						this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_exemplar);
						this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
						for (V3DLONG i=0;i<size_exemplar;i++)
						{
							vector<V3DLONG> xyz_i = this->pos2xyz(poss_exemplar[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
						}
						this->poss2Image1D(poss_exemplar, this->image1D_mask, label_center); //merged;
						continue;
					}
					//finally, succeed;
					vector<vector<double> > shapeStat_exemplar=this->getShapeStat(centerXYZ_exemplar[0], centerXYZ_exemplar[1], centerXYZ_exemplar[2], radius_exemplar, _idx_shape);
					//double shapeStat_sum = 0;
					//for (V3DLONG i=0;i<3;i++)
					//{
					//	for (V3DLONG m=0;m<const_shapeStatCount;m++)
					//	{
					//		shapeStat_sum += fabs(shapeStat_exemplar[i][m]);
					//	}
					//}
					//if (shapeStat_sum<const_infinitesimal) {continue;} //empty shape stat, failed;
					this->poss2Image1D(poss_exemplarNew, this->image1D_mask, count_result+1);
					for (V3DLONG i=0;i<size_exemplar;i++)
					{
						vector<V3DLONG> xyz_i = this->pos2xyz(poss_exemplar[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
					}
					shapeStats_exemplar.push_back(shapeStat_exemplar);
					V3DLONG uThreshold_size = size_exemplar*_multiplier_uThresholdRegionSizeExemplar;
					uThresholds_size.push_back(uThreshold_size);
					//uThresholds_intensity.push_back(this->getMaxIntensity(poss_exemplar));
					this->possVct_result.push_back(poss_exemplar);
					this->centers_result.push_back(center_exemplar);
					thresholds_intensity.push_back(threshold_intensity);
					count_exemplar++;
					count_result++;
				}
				if (count_exemplar>=_count_exemplar) {break;}
			}
			cout<<"exemplars found in this iteration: "<<count_exemplar<<endl;
			//QString name_result = "Exemplar";
			//visualizationImage1D(this->image1Dc_result, dim_X, dim_Y, dim_Z, 3, _V3DPluginCallback2_currentCallback, name_result);
			if (thresholds_intensity.empty()) {break;} //converged;
			vector<V3DLONG> mapping_exemplar = this->sort(thresholds_intensity); // in ascending order;
			for (V3DLONG idx_exemplar1=0;idx_exemplar1<count_exemplar;idx_exemplar1++)
			{
				V3DLONG idx_exemplarMapped1 = mapping_exemplar[idx_exemplar1];
				V3DLONG threshold_current = thresholds_intensity[idx_exemplarMapped1];
				//cout<<"threshold_current: "<<threshold_current<<endl;
				//V3DLONG uThreshold_current = uThresholds_intensity[idx_exemplarMapped1];
				memcpy(image1D_maskTmp, image1D_mask, size_page*sizeof(V3DLONG));
				V3DLONG uThreshold_size1 = uThresholds_size[idx_exemplarMapped1];
				//cout<<"uThreshold_size1: "<<uThreshold_size1<<endl;
				for (V3DLONG idx_seedCategoy=0;idx_seedCategoy<count_seedCategory;idx_seedCategoy++)
				{
					V3DLONG count_seed = this->possVct_seed[idx_seedCategoy].size();
					for (V3DLONG idx_seed=0;idx_seed<count_seed;idx_seed++)
					{
						V3DLONG pos_seed = this->possVct_seed[idx_seedCategoy][idx_seed];
						if (image1D_mask[pos_seed]>0) {continue;}
						V3DLONG value_seed = this->image1D_page[pos_seed];
						if (value_seed<=threshold_current) { break; }
						//if (value_seed>uThreshold_current) {continue;}
						vector<V3DLONG> poss_propagate = this->regionGrowOnPos(pos_seed, threshold_current,  image1D_maskTmp, 1);
						V3DLONG size_propagate = poss_propagate.size();
						if (size_propagate>uThreshold_regionSizeGlobal)	{ continue; }
						if (size_propagate<_threshold_regionSize) { continue;}
						if (size_propagate>uThreshold_size1) { continue;}
						vector<V3DLONG> boundBox_propagate = this->getBoundBox(poss_propagate);
						V3DLONG radius_propagate = ceil((double)this->getMaxDimension(boundBox_propagate)/2);
						if (radius_propagate<(const_shapeStatCount+1))	{ continue; }
						//cout<<"size_propagate: "<<size_propagate<<endl;
						V3DLONG center_propagate = this->getCenterByMass(poss_propagate);
						V3DLONG label_center=this->image1D_mask[center_propagate];
						//cout<<"center_propagate: "<<center_propagate<<endl;
						//cout<<"label_center: "<<label_center<<endl;
						if ((label_center>0)&&(label_center<INF))//donut, fill it;
						{
							this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_propagate);
							this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
							for (V3DLONG i=0;i<size_propagate;i++)
							{
								vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
							}
							this->poss2Image1D(poss_propagate, this->image1D_mask, label_center); //merged;
						}
						V3DLONG center_geometry=getGeometryCenter(poss_propagate);
						//cout<<"center_geometry: "<<center_geometry<<endl;
						label_center=this->image1D_mask[center_geometry];
						if ((label_center>0)&&(label_center<INF))//donut, fill it;
						{
							this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_propagate);
							this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
							for (V3DLONG i=0;i<size_propagate;i++)
							{
								vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
							}
							this->poss2Image1D(poss_propagate, this->image1D_mask, label_center); //merged;
						}
						vector<V3DLONG> xyz_center = this->pos2xyz(center_propagate);
						vector<vector<double> > shapeStat_propagate = this->getShapeStat(xyz_center[0], xyz_center[1], xyz_center[2], radius_propagate, _idx_shape); //consisted of 3 vectors with length 4;
						if (shapeStat_propagate.empty()) {cout<<"warning: shape stats estimation failed;"<<endl;continue;}
						bool is_passedShapeTest = true;
						for (int m=0; m<const_shapeStatCount; m++)
						{
							double anisotropy_exemplar = fabs(shapeStats_exemplar[idx_exemplarMapped1][0][m]);
							//cout<<"anisotropy_exemplar: "<<anisotropy_exemplar<<endl;
							double anisotropy_propagate = fabs(shapeStat_propagate[0][m]);
							if ((anisotropy_exemplar<const_infinitesimal) && (anisotropy_exemplar>const_infinitesimal))
							{is_passedShapeTest = false; break;}
							if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
							{is_passedShapeTest = false; break;}
							
							anisotropy_exemplar = fabs(shapeStats_exemplar[idx_exemplarMapped1][1][m]);
							//cout<<"anisotropy_exemplar: "<<anisotropy_exemplar<<endl;
							anisotropy_propagate = fabs(shapeStat_propagate[1][m]);
							if ((anisotropy_exemplar<const_infinitesimal) && (anisotropy_exemplar>const_infinitesimal))
							{is_passedShapeTest = false; break;}
							if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
							{is_passedShapeTest = false; break;}

							anisotropy_exemplar = fabs(shapeStats_exemplar[idx_exemplarMapped1][2][m]);
							//cout<<"anisotropy_exemplar: "<<anisotropy_exemplar<<endl;
							anisotropy_propagate = fabs(shapeStat_propagate[2][m]);
							if ((anisotropy_exemplar<const_infinitesimal) && (anisotropy_exemplar>const_infinitesimal))
							{is_passedShapeTest = false; break;}
							if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
							{is_passedShapeTest = false; break;}
						}
						if (is_passedShapeTest)
						{
							this->possVct_result.push_back(poss_propagate); //finally succeed;
							this->centers_result.push_back(center_propagate);
							for (V3DLONG i=0;i<size_propagate;i++)
							{
								vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]);
								this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
							}
							this->poss2Image1D(poss_propagate, this->image1D_mask, count_result+1); //succeed;
							count_result++;
						}
						//for (V3DLONG idx_exemplar2=0;idx_exemplar2<count_exemplar;idx_exemplar2++)
						//{
						//	V3DLONG idx_exemplarMapped2 = mapping_exemplar[idx_exemplar2];
						//	V3DLONG uThreshold_size2 = uThresholds_size[idx_exemplarMapped2];
						//	if (size_propagate>uThreshold_size2) { continue;}
						//	bool is_passedShapeTest = true;
						//	for (int m=0; m<const_shapeStatCount; m++)
						//	{
						//		double anisotropy_exemplar = shapeStats_exemplar[idx_exemplarMapped2][0][m];
						//		double anisotropy_propagate = shapeStat_propagate[0][m];
						//		if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
						//		{is_passedShapeTest = false; break;}
						//		anisotropy_exemplar = shapeStats_exemplar[idx_exemplarMapped2][1][m];
						//		anisotropy_propagate = shapeStat_propagate[1][m];
						//		if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
						//		{is_passedShapeTest = false; break;}
						//		anisotropy_exemplar = shapeStats_exemplar[idx_exemplarMapped2][2][m];
						//		anisotropy_propagate = shapeStat_propagate[2][m];
						//		if (fabs(anisotropy_propagate-anisotropy_exemplar)>(_delta_shapeStat*anisotropy_exemplar))
						//		{is_passedShapeTest = false; break;}
						//	}
						//	if (is_passedShapeTest)
						//	{
						//		V3DLONG label_center=this->image1D_mask[center_propagate];
						//		if ((label_center>0)&&(label_center<INF))//donut, fill it;
						//		{
						//			this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_propagate);
						//			this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
						//			for (V3DLONG i=0;i<size_propagate;i++)
						//			{
						//				vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
						//			}
						//			this->poss2Image1D(poss_propagate, this->image1D_mask, label_center); //merged;
						//			this->poss2Image1D(poss_propagate, image1D_maskTmp, INF); //masked;
						//		}
						//		V3DLONG center_geometry=getGeometryCenter(poss_propagate);
						//		label_center=this->image1D_mask[center_geometry];
						//		if ((label_center>0)&&(label_center<INF))//donut, fill it;
						//		{
						//			this->possVct_result[label_center-1]=this->mergePoss(this->possVct_result[label_center-1], poss_propagate);
						//			this->centers_result[label_center-1]=this->getCenterByMass(this->possVct_result[label_center-1]);
						//			for (V3DLONG i=0;i<size_propagate;i++)
						//			{
						//				vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]); this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
						//			}
						//			this->poss2Image1D(poss_propagate, this->image1D_mask, label_center); //merged;
						//		}
						//		this->possVct_result.push_back(poss_propagate); //finally succeed;
						//		this->centers_result.push_back(center_propagate);
						//		for (V3DLONG i=0;i<size_propagate;i++)
						//		{
						//			vector<V3DLONG> xyz_i = this->pos2xyz(poss_propagate[i]);
						//			this->image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = 0;
						//		}
						//		this->poss2Image1D(poss_propagate, this->image1D_mask, count_result+1); //succeed;
						//		count_result++;
						//		break;
						//	}
						//}
					}
				}
			}
			cout<<"count regions: "<<count_result<<endl;
			if (count_exemplar<_count_exemplar) {break;}
		}
		//post-processing;
		count_result = possVct_result.size();
		for (V3DLONG idx_result=0;idx_result<count_result;idx_result++)
		{
			vector<V3DLONG> poss_result = possVct_result[idx_result];
			vector<V3DLONG> boudnBox_result = getBoundBox(poss_result);
			V3DLONG label_region = getDominantLabelFromBoundBox(boudnBox_result);
			//cout<<"label_region: "<<label_region<<"; "<<"idx_result: "<<idx_result+1<<endl;
			if ((label_region>0) && (label_region!=(idx_result+1))) //donut, fill it;
			{
				this->possVct_result[label_region-1]=this->mergePoss(this->possVct_result[label_region-1], poss_result);
				this->centers_result[label_region-1]=this->getCenterByMass(this->possVct_result[label_region-1]);
				possVct_result[idx_result].clear();
				this->centers_result[idx_result]=-1;
			}
			else if (label_region==0)
			{
				possVct_result[idx_result].clear();
				this->centers_result[idx_result]=-1;
			}
			V3DLONG center1 = this->centers_result[idx_result];
			if (center1>-1)
			{
				for (V3DLONG idx_result2=0;idx_result2<count_result;idx_result2++)
				{
					if (idx_result!=idx_result2)
					{
						V3DLONG center2 = this->centers_result[idx_result2];
						if (center2>-1)
						if (this->getEuclideanDistance2(center2, center1)<_threshold_centerDistance2)
						{
							/*this->possVct_result[idx_result2]=this->mergePoss(this->possVct_result[idx_result2], poss_result);
							this->centers_result[idx_result2]=this->getCenterByMass(this->possVct_result[idx_result2]);*/
							possVct_result[idx_result].clear();
							this->centers_result[idx_result]=-1;
							//cout<<"remove duplicate centers at("<<idx_result<<");"<<endl;
						}
					}
				}
			}
		}
		if (this->possVct_result.empty()) { v3d_msg("cell detection failed;"); return false;	}
		this->landmarkList_result = this->poss2landMarkList(this->centers_result);
		//this->possVct2Image1DC(this->possVct_result, this->image1Dc_result);
		memory_free_int1D(image1D_maskTmp);
		return true;
	}
	#pragma endregion

	#pragma region "regionGrow"
	void categorizeVoxelsByIntensity(double _threshold_intensity) //will only consider voxels with value higher than default_threshold_global;
	{
		this->possVct_seed.clear();
		vector<V3DLONG> poss_empty (0,0);
		for (V3DLONG i=_threshold_intensity;i<const_length_histogram;i++)
		{
			this->possVct_seed.push_back(poss_empty);
		}
		for (V3DLONG i=0;i<this->size_page;i++)
		{
			V3DLONG value_i = this->image1D_page[i];
			if (value_i>_threshold_intensity)
			{
				V3DLONG offset_i = const_max_voxelValue-value_i;
				this->possVct_seed[offset_i].push_back(i);
			}
		}
	}
	vector<V3DLONG> regionGrowOnPos(const V3DLONG _pos_seed, const V3DLONG _threshold_voxelValue, 
		V3DLONG* _mask_input, V3DLONG _identifier_seed)
	{
		vector<V3DLONG> poss_result;
		vector<vector<V3DLONG> > poss_growing;
		if (_mask_input[_pos_seed]>0) {return poss_result;}
		if (this->image1D_page[_pos_seed]<=_threshold_voxelValue) {return poss_result;}
		vector<V3DLONG> xyz_seed = this->pos2xyz(_pos_seed);
		poss_growing.push_back(xyz_seed);
		poss_result.push_back(_pos_seed);
		_mask_input[_pos_seed]=_identifier_seed; //scooped;
		while (true)
		{
			if (poss_growing.empty()) //growing complete;
			{
				return poss_result;
			}
			vector<V3DLONG> xyz_current = poss_growing.back();
			poss_growing.pop_back();
			for (V3DLONG j=0;j<const_count_neighbors;j++)
			{
				vector<V3DLONG> xyz_neighbor (3, 0);
				xyz_neighbor[0]=xyz_current[0]+this->xyzsR_neighbor[j][0];xyz_neighbor[1]=xyz_current[1]+this->xyzsR_neighbor[j][1];xyz_neighbor[2]=xyz_current[2]+this->xyzsR_neighbor[j][2];
				if (isValid(xyz_neighbor))
				{
					V3DLONG pos_current = this->coordinate2pos(xyz_current[0],xyz_current[1],xyz_current[2]);
					V3DLONG pos_neighbor = pos_current+this->possR_neighbor[j];
					if (_mask_input[pos_neighbor]<1) //available only;
					{
						V3DLONG value_neighbor = this->image1D_page[pos_neighbor];
						if (value_neighbor>_threshold_voxelValue)
						{
							_mask_input[pos_neighbor]=_identifier_seed; //scooped;
							poss_growing.push_back(xyz_neighbor);
							poss_result.push_back(pos_neighbor);
						}
					}
				}
			}
		}
	}
	vector<V3DLONG> regionGrowOnPos(const V3DLONG _pos_seed, const V3DLONG _threshold_voxelValue, 
		const V3DLONG _uThreshold_regionSize, V3DLONG* _mask_input, V3DLONG _identifier_seed)
	{
		vector<V3DLONG> poss_result;
		vector<vector<V3DLONG> > poss_growing;
		if (_mask_input[_pos_seed]>0) {return poss_result;}
		if (this->image1D_page[_pos_seed]<=_threshold_voxelValue) {return poss_result;}
		vector<V3DLONG> xyz_seed = this->pos2xyz(_pos_seed);
		poss_growing.push_back(xyz_seed);
		poss_result.push_back(_pos_seed);
		V3DLONG count_pos=1;
		_mask_input[_pos_seed]=_identifier_seed; //scooped;
		while (true)
		{
			if (poss_growing.empty()) //growing complete;
			{
				return poss_result;
			}
			vector<V3DLONG> xyz_current = poss_growing.back();
			poss_growing.pop_back();
			for (V3DLONG j=0;j<const_count_neighbors;j++)
			{
				vector<V3DLONG> xyz_neighbor (3, 0);
				xyz_neighbor[0]=xyz_current[0]+this->xyzsR_neighbor[j][0];xyz_neighbor[1]=xyz_current[1]+this->xyzsR_neighbor[j][1];xyz_neighbor[2]=xyz_current[2]+this->xyzsR_neighbor[j][2];
				if (isValid(xyz_neighbor))
				{
					V3DLONG pos_current = this->coordinate2pos(xyz_current[0],xyz_current[1],xyz_current[2]);
					V3DLONG pos_neighbor = pos_current+this->possR_neighbor[j];
					if (_mask_input[pos_neighbor]<1) //available only;
					{
						V3DLONG value_neighbor = this->image1D_page[pos_neighbor];
						if (value_neighbor>_threshold_voxelValue)
						{
							_mask_input[pos_neighbor]=_identifier_seed; //scooped;
							poss_growing.push_back(xyz_neighbor);
							poss_result.push_back(pos_neighbor);
							count_pos++;
							if (count_pos>_uThreshold_regionSize) {	return poss_result; }//too large;
						}
					}
				}
			}
		}
	}
	vector<V3DLONG> regionGrowOnPos(const V3DLONG _pos_seed, const V3DLONG _threshold_voxelValue, const V3DLONG _uThreshold_voxelValue, 
		const V3DLONG _uThreshold_regionSize, V3DLONG* _mask_input, V3DLONG _identifier_seed)
	{
		vector<V3DLONG> poss_result;
		vector<vector<V3DLONG> > poss_growing;
		if (_mask_input[_pos_seed]>0) {return poss_result;}
		if (this->image1D_page[_pos_seed]<=_threshold_voxelValue) {return poss_result;}
		vector<V3DLONG> xyz_seed = this->pos2xyz(_pos_seed);
		poss_growing.push_back(xyz_seed);
		poss_result.push_back(_pos_seed);
		V3DLONG count_pos=1;
		_mask_input[_pos_seed]=_identifier_seed; //scooped;
		while (true)
		{
			if (poss_growing.empty()) //growing complete;
			{
				return poss_result;
			}
			vector<V3DLONG> xyz_current = poss_growing.back();
			poss_growing.pop_back();
			for (V3DLONG j=0;j<const_count_neighbors;j++)
			{
				vector<V3DLONG> xyz_neighbor (3, 0);
				xyz_neighbor[0]=xyz_current[0]+this->xyzsR_neighbor[j][0];xyz_neighbor[1]=xyz_current[1]+this->xyzsR_neighbor[j][1];xyz_neighbor[2]=xyz_current[2]+this->xyzsR_neighbor[j][2];
				if (isValid(xyz_neighbor))
				{
					V3DLONG pos_current = this->coordinate2pos(xyz_current[0],xyz_current[1],xyz_current[2]);
					V3DLONG pos_neighbor = pos_current+this->possR_neighbor[j];
					if (_mask_input[pos_neighbor]<1) //available only;
					{
						V3DLONG value_neighbor = this->image1D_page[pos_neighbor];
						if ((value_neighbor>_threshold_voxelValue) && (value_neighbor<=_uThreshold_voxelValue))
						{
							_mask_input[pos_neighbor]=_identifier_seed; //scooped;
							poss_growing.push_back(xyz_neighbor);
							poss_result.push_back(pos_neighbor);
							count_pos++;
							if (count_pos>_uThreshold_regionSize) {	return poss_result; }//too large;
						}
					}
				}
			}
		}
	}
	V3DLONG getDominantLabelFromBoundBox(vector<V3DLONG> _boundBox_input)
	{
		vector<V3DLONG> histo_input (100000, 0);
		V3DLONG size_input = (_boundBox_input[1]-_boundBox_input[0])*(_boundBox_input[3]-_boundBox_input[2])*(_boundBox_input[5]-_boundBox_input[4]);
		V3DLONG threshold_dominant1 = size_input*const_criteria_dominant1;
		V3DLONG threshold_dominant2 = size_input*const_criteria_dominant2;
		//cout<<threshold_dominant<<endl;
		for (V3DLONG x=_boundBox_input[0];x<_boundBox_input[1];x++)
		{
			for (V3DLONG y=_boundBox_input[2];y<_boundBox_input[3];y++)
			{
				for (V3DLONG z=_boundBox_input[4];z<_boundBox_input[5];z++)
				{
					V3DLONG pos_current = this->coordinate2pos(x, y, z);
					//cout<<x<<","<<y<<","<<z<<endl;
					if (this->image1D_mask[pos_current]<INF)
					{
						histo_input[this->image1D_mask[pos_current]]++;
					}
					//cout<<histo_input[this->image1D_mask[pos_current]]<<endl;
				}
			}
		}
		for (V3DLONG i=0;i<100000;i++)
		{
			if (i==0)
			{
				if (histo_input[i]>threshold_dominant2) { return i; }
			}
			else
			{
				if (histo_input[i]>threshold_dominant1) { return i; }
			}
		}
		return -1;
	}
	void thresholdForCurrentPage(double _threshold_input)
	{
		for(V3DLONG i=0; i<this->size_page; i++)
		{	
			if (image1D_page[i]>_threshold_input)
			{
				//do nothing to image1D_page;
				this->image1D_mask[i] = 0; //available;
			}
			else
			{
				this->image1D_mask[i] = INF; //invalid;
			}
		}
		return;
	}
	#pragma endregion

	#pragma region "utility functions"
	bool isValid(const vector<V3DLONG> _xyz_input)
	{
		if ((_xyz_input[0]<0)||(_xyz_input[0]>=this->dim_X)||
			(_xyz_input[1]<0)||(_xyz_input[1]>=this->dim_Y)||
			(_xyz_input[2]<0)||(_xyz_input[2]>=this->dim_Z))
		{ return false;	}
		else { return true; }
	}
	vector<V3DLONG> mergePoss(const vector<V3DLONG> _poss_input1, const vector<V3DLONG> _poss_input2) 
	{
		vector<V3DLONG> poss_result = _poss_input1;
		V3DLONG count_pos = _poss_input2.size();
		for (V3DLONG i=0;i<count_pos;i++) { poss_result.push_back(_poss_input2[i]); }
		return poss_result;
	}
	LandmarkList poss2landMarkList(const vector<V3DLONG> _poss_input)
	{
		LandmarkList LandmarkList_result;
		LocationSimple Landmark_tmp;
		for (V3DLONG i=0;i<_poss_input.size();i++)
		{
			if (_poss_input[i]!=-1)
			{
				Landmark_tmp=index2landMark(_poss_input[i]);
				LandmarkList_result.push_back(Landmark_tmp);
			}
		}
		return LandmarkList_result;
	}
	LocationSimple index2landMark(const V3DLONG _pos_input)
	{
		vector<V3DLONG> xyz_input = pos2xyz(_pos_input);
		V3DLONG x=xyz_input[0]+1; V3DLONG y=xyz_input[1]+1; V3DLONG z=xyz_input[2]+1;
		LocationSimple LocationSimple_result (x, y, z);
		return LocationSimple_result;
	}
	vector<V3DLONG> pos2xyz(const V3DLONG _pos_input)
	{
		vector<V3DLONG> xyz_result (3, -1);
		xyz_result[2] = floor((double)_pos_input/(double)this->offset_Z);
		xyz_result[1] = floor((double)(_pos_input-xyz_result[2]*this->offset_Z)/(double)this->offset_Y);
		xyz_result[0] = _pos_input- xyz_result[2]*this->offset_Z-xyz_result[1]*this->offset_Y;
		return xyz_result;
	}
	V3DLONG coordinate2pos(const V3DLONG x, const V3DLONG y, const V3DLONG z)
	{
		return z*this->offset_Z+y*this->offset_Y+x;
	}
	void initializeConstants()
	{
		this->possR_neighbor.clear();
		this->xyzsR_neighbor.clear();
		this->colors_simple.clear();
		vector<V3DLONG> xyz_neighbor (3, -1);;
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
						this->possR_neighbor.push_back(z*this->offset_Z+y*this->offset_Y+x);
						xyz_neighbor[0]=x; xyz_neighbor[1]=y; xyz_neighbor[2]=z;
						this->xyzsR_neighbor.push_back(xyz_neighbor);
					}
				}
			}
		}
		vector<V3DLONG> color_tmp(3, 0); color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 0;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 0;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 0; color_tmp[1] = 0; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 0;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 0; color_tmp[1] = 255; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 255; color_tmp[1] = 0; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 0;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 0;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 0; color_tmp[1] = 128; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 255; color_tmp[1] = 255; color_tmp[2] = 128;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 128; color_tmp[1] = 255; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
		color_tmp[0] = 255; color_tmp[1] = 128; color_tmp[2] = 255;
		this->colors_simple.push_back(color_tmp);
	}
	void possVct2Image1DC(const vector<vector<V3DLONG> > _possVct_input, unsigned char* _Image1D_input)
	{
		vector<V3DLONG> color_poss (3, 0);
		V3DLONG count_poss=_possVct_input.size();
		for (V3DLONG i=0;i<count_poss;i++)
		{
			V3DLONG idx_color = i%12;
			color_poss[0] = colors_simple[idx_color][0];
			color_poss[1] = colors_simple[idx_color][1];
			color_poss[2] = colors_simple[idx_color][2];
			poss2Image1DC(_possVct_input[i], _Image1D_input, color_poss);
		}
	}
	void poss2Image1D(const vector<V3DLONG> _poss_input, unsigned char* _Image1D_input, const V3DLONG _value_input)
	{
		V3DLONG size_poss = _poss_input.size();
		for (V3DLONG i=0;i<size_poss;i++) { _Image1D_input[_poss_input[i]]=_value_input; }
	}
	void poss2Image1D(const vector<V3DLONG> _poss_input, V3DLONG* _Image1D_input, const V3DLONG _value_input)
	{
		V3DLONG size_poss = _poss_input.size();
		for (V3DLONG i=0;i<size_poss;i++) { _Image1D_input[_poss_input[i]]=_value_input; }
	}
	void possVct2Image1D(const vector<vector<V3DLONG> > _possVct_input, unsigned char* _Image1D_input, const V3DLONG _value_input)
	{
		V3DLONG count_poss = _possVct_input.size();
		for (V3DLONG i=0;i<count_poss;i++) {poss2Image1D(_possVct_input[i], _Image1D_input, _value_input);}
	}
	void possVct2Image1D(const vector<vector<V3DLONG> > _possVct_input, V3DLONG* _Image1D_input, const V3DLONG _value_input)
	{
		V3DLONG count_poss = _possVct_input.size();
		for (V3DLONG i=0;i<count_poss;i++) {poss2Image1D(_possVct_input[i], _Image1D_input, _value_input);}
	}
	void poss2Image1DC(const vector<V3DLONG> _poss_input, unsigned char* _Image1D_input, const vector<V3DLONG> _color_input)
	{
		V3DLONG count_pos = _poss_input.size();
		for (V3DLONG i=0;i<count_pos;i++)
		{
			_Image1D_input[_poss_input[i]] = _color_input[0];
			_Image1D_input[_poss_input[i]+this->size_page] = _color_input[1];
			_Image1D_input[_poss_input[i]+this->size_page+this->size_page] = _color_input[2];
		}
	}
	vector<V3DLONG> sort(const vector<V3DLONG> _values_input)
	{
		V3DLONG count_value = _values_input.size();
		vector<V3DLONG> counts_larger;
		vector<V3DLONG> mapping_result (count_value, -1);
		for (V3DLONG i=0;i<count_value;i++)
		{
			V3DLONG value_i = _values_input[i];
			V3DLONG count_larger = 0;
			for (V3DLONG j=0;j<count_value;j++)
			{
				V3DLONG value_j = _values_input[j];
				if (value_i>value_j) {count_larger++;}
				if ((value_i==value_j)&&(j>i)) {count_larger++;}
			}
			counts_larger.push_back(count_larger);
		}
		for (V3DLONG i=0;i<count_value;i++)
		{
			mapping_result[counts_larger[i]]=i;
		}
		return mapping_result;
	}
	V3DLONG getMaxIntensity(const vector<V3DLONG> _poss_input)
	{
		V3DLONG intensity_max = -INF;
		V3DLONG count_input = _poss_input.size();
		for (V3DLONG idx_input=0;idx_input<count_input;idx_input++)
		{
			if (intensity_max<this->image1D_page[_poss_input[idx_input]])
			{
				intensity_max = this->image1D_page[_poss_input[idx_input]];
			}
		}
		return intensity_max;
	}
	#pragma endregion

	#pragma region "geometry property"
	double getEuclideanDistance2(const V3DLONG _pos_input1, const V3DLONG _pos_input2)
	{
		double result = 0;
		vector<V3DLONG> xyz_input1 = this->pos2xyz(_pos_input1);
		vector<V3DLONG> xyz_input2 = this->pos2xyz(_pos_input2);
		if (!isValid(xyz_input1)||!isValid(xyz_input2)) {return -1;}
		result+=(xyz_input1[0]-xyz_input2[0])*(xyz_input1[0]-xyz_input2[0]);
		result+=(xyz_input1[1]-xyz_input2[1])*(xyz_input1[1]-xyz_input2[1]);
		result+=(xyz_input1[2]-xyz_input2[2])*(xyz_input1[2]-xyz_input2[2]);
		return result;
	}
	V3DLONG getGeometryCenter(const vector<V3DLONG> _poss_input)
	{
		vector<V3DLONG> xyz_voxel;
		V3DLONG x; V3DLONG y; V3DLONG z;
		double sum_X=0; double sum_Y=0;	double sum_Z=0;
		V3DLONG count_pos = _poss_input.size();
		if (count_pos<1) {return -1;}
		for (V3DLONG i=0;i<count_pos;i++)
		{
			xyz_voxel = this->pos2xyz(_poss_input[i]);
			x=xyz_voxel[0]; y=xyz_voxel[1]; z=xyz_voxel[2];
			sum_X += (double)x; sum_Y += (double)y; sum_Z += (double)z;
		}
		return (this->coordinate2pos(sum_X/count_pos, sum_Y/count_pos, sum_Z/count_pos));
	}
	V3DLONG getCenterByMass(const vector<V3DLONG> _poss_input)
	{
		vector<V3DLONG> xyz_voxel;
		V3DLONG x; V3DLONG y; V3DLONG z;
		double sum_X=0; double sum_Y=0;	double sum_Z=0;
		double sum_mass = 0;
		V3DLONG count_pos = _poss_input.size();
		if (count_pos<1) {return -1;}
		for (V3DLONG i=0;i<count_pos;i++)
		{
			xyz_voxel = this->pos2xyz(_poss_input[i]);
			double value_voxel = this->image1D_page[_poss_input[i]];
			x=xyz_voxel[0]; y=xyz_voxel[1]; z=xyz_voxel[2];
			sum_X += (double)x*value_voxel; sum_Y += (double)y*value_voxel; sum_Z += (double)z*value_voxel;
			sum_mass += value_voxel;
		}
		return (this->coordinate2pos(sum_X/sum_mass, sum_Y/sum_mass, sum_Z/sum_mass));
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
		V3DLONG count_pos = idxs_input.size();
		vector<V3DLONG> xyz_voxel (3, 0);
		vector<V3DLONG> values_result (6, 0);
		V3DLONG idx_tmp;
		for (V3DLONG idx_voxel=0;idx_voxel<count_pos;idx_voxel++)
		{
			idx_tmp = idxs_input[idx_voxel];
			xyz_voxel = this->pos2xyz(idx_tmp);
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
	V3DLONG getMaxDimension(vector<V3DLONG> vct_input)
	{
		V3DLONG size_X = vct_input[1] - vct_input[0];
		V3DLONG size_Y = vct_input[3] - vct_input[2];
		V3DLONG size_Z = vct_input[5] - vct_input[4];
		return max(max(size_X, size_Y), size_Z);
	}
	#pragma endregion

	#pragma region "memoryManagement"
	static double ***memory_allocate_double3D(V3DLONG i_size,V3DLONG j_size,V3DLONG k_size)
	{
		double ***ptr_result;
		V3DLONG i,k;
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

	static void memory_free_double3D(double ***ptr_input, const V3DLONG k_size, const V3DLONG i_size)
	{
		V3DLONG k,i;
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

	static unsigned char ***memory_allocate_uchar3D(const V3DLONG dim_Z, const V3DLONG dim_Y, const V3DLONG dim_X)
	{
		unsigned char ***ptr_result;
		ptr_result=(unsigned char ***) calloc(dim_Z,sizeof(unsigned char **));
		for(V3DLONG z=0;z<dim_Z;z++)
		{
			ptr_result[z]=(unsigned char **) calloc(dim_Y,sizeof(unsigned char *));
		}
		for(V3DLONG z=0;z<dim_Z;z++)
		{
			for(V3DLONG y=0;y<dim_Y;y++)
			{
				ptr_result[z][y]=(unsigned char *) calloc(dim_X,sizeof(unsigned char ));
			}
		}
		return(ptr_result);
	}

	static void memory_free_uchar3D(unsigned char ***ptr_input, const int dim_Z, const int dim_Y)
	{
		for(V3DLONG z=0;z<dim_Z;z++)
		{
			for(V3DLONG y=0;y<dim_Y;y++)
			{
				free(ptr_input[z][y]);
			}
		}
		for(V3DLONG z=0;z<dim_Z;z++)
		{
			free(ptr_input[z]);
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

	static void memory_free_int1D(V3DLONG *ptr_input)
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

	#pragma region "shapeStat"
	vector<vector<double> > getShapeStat(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG value_radius, V3DLONG _idx_shape)
	{
		vector<vector<double> > valuesVct_result;
		double value_PC1 = 0;
		double value_PC2 = 0;
		double value_PC3 = 0;
		vector<double> values_PC1;
		vector<double> values_PC2;
		vector<double> values_PC3;
		double size_step = (double)(value_radius-2)/(double)(const_shapeStatCount-1);
		V3DLONG rr=0;
		for (int i=1;i<=const_shapeStatCount;i++)
		{
			rr=2+size_step*(i-1);
			getPCA(this->image3D_page, this->dim_X, this->dim_Y, this->dim_Z, x , y, z, rr, rr, rr,value_PC1, value_PC2, value_PC3, _idx_shape, false, false);
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

	template <class T> bool getPCA(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
		V3DLONG x0, V3DLONG y0, V3DLONG z0,
		V3DLONG rx, V3DLONG ry, V3DLONG rz,
		double &pc1, double &pc2, double &pc3, int wintype=0,
		bool b_disp_CoM_etc=true, 	//b_disp_CoM_etc is the display option for center of mass )
		bool b_normalize_score=false) //if the score if normalized with respect to the window size
	{
		if (wintype==0)
			return getPCA_cube(img3d, sx, sy, sz, x0, y0, z0, rx, ry, rz, pc1, pc2, pc3, b_disp_CoM_etc, b_normalize_score);
		else //wintype==1
			return getPCA_sphere(img3d, sx, sy, sz, x0, y0, z0, rx, ry, rz, pc1, pc2, pc3, b_disp_CoM_etc, b_normalize_score);
	}

	template <class T> bool getPCA_sphere(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
		V3DLONG x0, V3DLONG y0, V3DLONG z0,
		V3DLONG rx, V3DLONG ry, V3DLONG rz,
		double &pc1, double &pc2, double &pc3,
		bool b_disp_CoM_etc=true, //b_disp_CoM_etc is the display option for center of mass )
		bool b_normalize_score=false)
	{
		if (!img3d || sx<=0 || sy<=0 || sz<=0 ||
			x0<0 || x0>=sx || y0<0 || y0>=sy || z0<0 || z0>=sz ||
			rx<0 || ry<0 || rz<0)
			return false;

		//get max radius
		V3DLONG maxrr = (rx>ry)?rx:ry; maxrr = (maxrr>rz)?maxrr:rz;

		//get the boundary

		V3DLONG xb=x0-rx; if(xb<0) xb=0; else if (xb>=sx) xb=sx-1;
		V3DLONG xe=x0+rx; if(xe<0) xe=0; else if (xe>=sx) xe=sx-1;
		V3DLONG yb=y0-ry; if(yb<0) yb=0; else if (yb>=sy) yb=sy-1;
		V3DLONG ye=y0+ry; if(ye<0) ye=0; else if (ye>=sy) ye=sy-1;
		V3DLONG zb=z0-rz; if(zb<0) zb=0; else if (zb>=sz) zb=sz-1;
		V3DLONG ze=z0+rz; if(ze<0) ze=0; else if (ze>=sz) ze=sz-1;

		V3DLONG i,j,k;
		double w;

		//first get the center of mass
		double x2, y2, z2;
		double rx2 = double(rx+1)*(rx+1), ry2 = (double)(ry+1)*(ry+1), rz2 = (double)(rz+1)*(rz+1); //+1 because later need to do use it for radius cmp
		double tmpd;
		double xm=0,ym=0,zm=0, s=0, mv=0, n=0;
		for (k=zb;k<=ze;k++)
		{
			z2 = k-z0; z2*=z2;
			for (j=yb;j<=ye;j++)
			{
				y2 = j-y0; y2*=y2;
				tmpd = y2/ry2 + z2/rz2;
				if (tmpd>1.0)
					continue;

				for (i=xb;i<=xe;i++)
				{
					x2 = i-x0; x2*=x2;
					if (x2/rx2 + tmpd > 1.0)
						continue;

					w = double(img3d[k][j][i]);
					xm += w*i;
					ym += w*j;
					zm += w*k;
					s += w;
					n = n+1;
				}
			}
		}
		if (s>0)
		{
			xm /= s; ym /=s; zm /=s;
			mv = s/n;
			//if (b_disp_CoM_etc)
			//{
			//printf("center of mass is (xm, ym, zm) = %5.3f, %5.3f, %5.3f\n",xm,ym,zm);
			//}

		}
		else
		{
			//printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
			return false;
		}

		//get the covariance. Note that the center of mass must be in the ellpsoid

		double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
		double dfx, dfy, dfz;
		for (k=zb;k<=ze;k++)
		{
			z2 = k-z0; z2*=z2;

			dfz = double(k)-zm;
			if (b_normalize_score) dfz /= maxrr;

			for (j=yb;j<=ye;j++)
			{
				y2 = j-y0; y2*=y2;
				tmpd = y2/ry2 + z2/rz2;
				if (tmpd>1.0)
					continue;

				dfy = double(j)-ym;
				if (b_normalize_score) dfy /= maxrr;

				for (i=xb;i<=xe;i++)
				{
					x2 = i-x0; x2*=x2;
					if (x2/rx2 + tmpd > 1.0)
						continue;

					dfx = double(i)-xm;
					if (b_normalize_score) dfx /= maxrr;

					//                w = img3d[k][j][i]; //140128
					w = img3d[k][j][i] - mv;  if (w<0) w=0; //140128 try the new formula

					cc11 += w*dfx*dfx;
					cc12 += w*dfx*dfy;
					cc13 += w*dfx*dfz;
					cc22 += w*dfy*dfy;
					cc23 += w*dfy*dfz;
					cc33 += w*dfz*dfz;
				}
			}
		}

		cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
		//if (b_disp_CoM_etc)
		//printf("convariance value (c11,c12,c13,c22,c23,c33) = %5.3f, %5.3f, %5.3f, %5.3f, %5.3f, %5.3f\n",cc11, cc12, cc13, cc22, cc23, cc33);

		//now get the eigen vectors and eigen values

		try
		{
			//then find the eigen vector
			SymmetricMatrix Cov_Matrix(3);
			Cov_Matrix.Row(1) << cc11;
			Cov_Matrix.Row(2) << cc12 << cc22;
			Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

			DiagonalMatrix DD;
			Matrix VV;
			EigenValues(Cov_Matrix,DD,VV);

			//output the result
			pc1 = DD(3);
			pc2 = DD(2);
			pc3 = DD(1);
		}
		catch (...)
		{
			pc1 = VAL_INVALID;
			pc2 = VAL_INVALID;
			pc3 = VAL_INVALID;
		}

		return true;
	}

	template <class T> bool getPCA_cube(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
		V3DLONG x0, V3DLONG y0, V3DLONG z0,
		V3DLONG rx, V3DLONG ry, V3DLONG rz,
		double &pc1, double &pc2, double &pc3, bool b_disp_CoM_etc=true, //b_disp_CoM_etc is the display option for center of mass
		bool b_normalize_score=false)
	{
		if (!img3d || sx<=0 || sy<=0 || sz<=0 ||
			x0<0 || x0>=sx || y0<0 || y0>=sy || z0<0 || z0>=sz ||
			rx<0 || ry<0 || rz<0)
			return false;

		//get max radius
		V3DLONG maxrr = (rx>ry)?rx:ry; maxrr = (maxrr>rz)?maxrr:rz;

		//get the boundary

		V3DLONG xb=x0-rx; if(xb<0) xb=0; else if (xb>=sx) xb=sx-1;
		V3DLONG xe=x0+rx; if(xe<0) xe=0; else if (xe>=sx) xe=sx-1;
		V3DLONG yb=y0-ry; if(yb<0) yb=0; else if (yb>=sy) yb=sy-1;
		V3DLONG ye=y0+ry; if(ye<0) ye=0; else if (ye>=sy) ye=sy-1;
		V3DLONG zb=z0-rz; if(zb<0) zb=0; else if (zb>=sz) zb=sz-1;
		V3DLONG ze=z0+rz; if(ze<0) ze=0; else if (ze>=sz) ze=sz-1;

		V3DLONG i,j,k;
		double w;

		//first get the center of mass
		double xm=0,ym=0,zm=0, s=0, mv=0;
		for (k=zb;k<=ze;k++)
		{
			for (j=yb;j<=ye;j++)
			{
				for (i=xb;i<=xe;i++)
				{
					w = double(img3d[k][j][i]);
					xm += w*i;
					ym += w*j;
					zm += w*k;
					s += w;
				}
			}
		}

		if (s>0)
		{
			xm /= s; ym /=s; zm /=s;
			mv = s/(double(ze-zb+1)*(ye-yb+1)*(xe-xb+1));
			if (b_disp_CoM_etc)
				printf("center of mass is (xm, ym, zm) = %5.3f, %5.3f, %5.3f\n",xm,ym,zm);
		}
		else
		{
			printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
			return false;
		}

		//get the covariance

		double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
		double dfx, dfy, dfz;
		for (k=zb;k<=ze;k++)
		{
			dfz = double(k)-zm;
			if (b_normalize_score) dfz /= maxrr;
			for (j=yb;j<=ye;j++)
			{
				dfy = double(j)-ym;
				if (b_normalize_score) dfy /= maxrr;
				for (i=xb;i<=xe;i++)
				{
					dfx = double(i)-xm;
					if (b_normalize_score) dfx /= maxrr;

					//                w = img3d[k][j][i]; //140128
					w = img3d[k][j][i] - mv;  if (w<0) w=0; //140128 try the new formula

					cc11 += w*dfx*dfx;
					cc12 += w*dfx*dfy;
					cc13 += w*dfx*dfz;
					cc22 += w*dfy*dfy;
					cc23 += w*dfy*dfz;
					cc33 += w*dfz*dfz;
				}
			}
		}

		cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
		if (b_disp_CoM_etc)
			printf("convariance value (c11,c12,c13,c22,c23,c33) = %5.3f, %5.3f, %5.3f, %5.3f, %5.3f, %5.3f\n",cc11, cc12, cc13, cc22, cc23, cc33);

		//now get the eigen vectors and eigen values

		try
		{
			//then find the eigen vector
			SymmetricMatrix Cov_Matrix(3);
			Cov_Matrix.Row(1) << cc11;
			Cov_Matrix.Row(2) << cc12 << cc22;
			Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

			DiagonalMatrix DD;
			Matrix VV;
			EigenValues(Cov_Matrix,DD,VV);

			//output the result
			pc1 = DD(3);
			pc2 = DD(2);
			pc3 = DD(1);
		}
		catch (...)
		{
			pc1 = VAL_INVALID;
			pc2 = VAL_INVALID;
			pc3 = VAL_INVALID;
		}

		return true;
	}

	#pragma endregion
};
#pragma endregion




QStringList cellSegmentation::menulist() const
{
    return QStringList()
		<<tr("automatic cell detection")
        <<tr("About");
}

QStringList cellSegmentation::funclist() const
{
	return QStringList()
		<<tr("cellsegmentation")
		<<tr("help");
}

void cellSegmentation::domenu(const QString &menu_name, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
	if (menu_name == tr("automatic cell detection"))
	{
        interface_run(V3DPluginCallback2_currentCallback,QWidget_parent);
    }
	else { v3d_msg(tr("Cell segmentation and counting by exemplar matching\nby Xiang Li (lindbergh.li@gmail.com);")); }
}

bool cellSegmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & _V3DPluginCallback2_currentCallback,  QWidget * _QWidget_parent)
{
	if (func_name == tr("run"))
	{
		vector<char*> inparas = *((vector<char*> *)input.at(1).p);
		V3DLONG idx_channel = atoi(inparas.at(0));
		V3DLONG count_exemplar = atoi(inparas.at(1));
		double delta_shapeStat = atof(inparas.at(2));
		V3DLONG threshold_centerMovement1 = atoi(inparas.at(3));
		threshold_centerMovement1 = threshold_centerMovement1*threshold_centerMovement1;
		V3DLONG threshold_centerMovement2 = atoi(inparas.at(4));
		threshold_centerMovement2 = threshold_centerMovement2*threshold_centerMovement2;
		V3DLONG threshold_intensity = atoi(inparas.at(5));
		V3DLONG threshold_centerDistance = atoi(inparas.at(6));
		threshold_centerDistance = threshold_centerDistance*threshold_centerDistance;
		V3DLONG threshold_regionSize = atoi(inparas.at(7));
		double multipler_uThresholdRegionSizeGlobal = atof(inparas.at(8));
		double multipler_uThresholdRegionSizeExemplar = atof(inparas.at(9));
		char * file_input = ((vector<char*> *)(input.at(0).p))->at(0);
		char * file_output = ((vector<char*> *)(output.at(0).p))->at(0);
		cout<<"channel: "<<idx_channel<<endl;
		cout<<"# of exemplars learned per iteration: "<<count_exemplar<<endl;
		cout<<"max anisotropic stat deviation: "<<delta_shapeStat<<endl;
		cout<<"max center movement during exemplar learning: "<<threshold_centerMovement1<<endl;
		cout<<"max movement from seed position during exemplar learning: "<<threshold_centerMovement2<<endl;
		cout<<"threshold for current image: "<<threshold_intensity<<endl;
		cout<<"threshold for center distance (for removing duplicate centers): "<<threshold_centerDistance<<endl;
		cout<<"threshold for region size: "<<threshold_regionSize<<endl;
		cout<<"mutiplier for upper threshold of region size (regarding to image size): "<<multipler_uThresholdRegionSizeGlobal<<endl;
		cout<<"mutiplier for upper threshold of region size (regarding to exemplar size): "<<multipler_uThresholdRegionSizeExemplar<<endl;
		cout<<"input image: "<<file_input<<endl;
		cout<<"output landmark list: "<<file_output<<endl;
		Image4DSimple * image4D_input = _V3DPluginCallback2_currentCallback.loadImage(file_input);
		//V3DLONG type_input = image4D_input->getDatatype();
		V3DLONG dim_X=image4D_input->getXDim();
		V3DLONG dim_Y=image4D_input->getYDim();
		V3DLONG dim_Z=image4D_input->getZDim();
		V3DLONG dim_C=image4D_input->getCDim();
		V3DLONG size_image = dim_X*dim_Y*dim_Z*dim_C;
		if (dim_C==1) { idx_channel=1; }
		unsigned char * image1D_current = image4D_input->getRawData();
		//if (type_input == 2) //V3D_UINT16;
		//{
		//	convert2UINT8((unsigned short*)image1D_current, image1D_current, size_image);
		//}
		//else if(type_input == 4) //V3D_FLOAT32;
		//{
		//	convert2UINT8((float*)image1D_current, image1D_current, size_image);
		//}
		//else
		//{
		//	v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
		//	return false;
		//}
		QString name_result = "Result";
		V3DLONG idx_shape = 1;
		class_segmentationMain class_segmentationMain1;
		if (class_segmentationMain1.control_run(image1D_current, dim_X, dim_Y, dim_Z, idx_channel, 
			idx_shape, delta_shapeStat, count_exemplar, threshold_intensity, file_input,
			threshold_centerMovement1, threshold_centerMovement2, threshold_centerDistance, threshold_regionSize,
			multipler_uThresholdRegionSizeGlobal, multipler_uThresholdRegionSizeExemplar, _V3DPluginCallback2_currentCallback, _QWidget_parent))
		{
			landmarkList2file(file_output, class_segmentationMain1.landmarkList_result);
		}
		else
		{
			v3d_msg("Warning: identification failed;");
			return false;
		}
		cout<<"finished;"<<endl;
	}
	else
	{
		cout<<endl;
		cout<<endl;
		cout<<" usage : vaa3d -x quickFind -f run -i <input filename> -o <output filename -p <idx_channel> <iter> <fusion> <size>"<<endl;
		cout<<" parameters: (default values in parentheses) "<<endl;
		cout<<"idx_channel = channel of input file to use for segmentation (1);"<<endl;
		cout<<"count_exemplar = number of exemplars per iteration (5);"<<endl;
		cout<<"_delta_shapeStat = max anisotrophc deviation (1.5);"<<endl;
		cout<<"threshold_centerMovement1 = max movement from center during learning (2);"<<endl;
		cout<<"threshold_centerMovement2 = max movement from seed during learning (4);"<<endl;
		cout<<"(optional) threshold_intensity = threshold for current image;"<<endl;
	}
	return true;
}

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
	dialogRun dialogRun1(_V3DPluginCallback2_currentCallback, _QWidget_parent, dim_C);
	QString name_result = "Result";
	if (dialogRun1.exec()!=QDialog::Accepted) {return false;}
	int _idx_shape; //get shape paramters;
	if (dialogRun1.shape_type_selection == sphere) {_idx_shape = 1;}
	else if (dialogRun1.shape_type_selection == cube) {_idx_shape = 0;}
	class_segmentationMain class_segmentationMain1;
	if (class_segmentationMain1.control_run(Image1D_current, dim_X, dim_Y, dim_Z, dialogRun1.idx_channel, 
		_idx_shape, dialogRun1.delta_shapeStat, dialogRun1.count_exemplar, dialogRun1.threshold_intensity, name_currentWindow,
		dialogRun1.uThreshold_centerMovement1, dialogRun1.uThreshold_centerMovement2, dialogRun1.threshold_centerDistance,
		dialogRun1.threshold_regionSize, dialogRun1.multiplier_uThresholdRegionSizeGlobal, dialogRun1.multiplier_uThresholdRegionSizeExemplar,
		_V3DPluginCallback2_currentCallback, _QWidget_parent))
	{
		//visualizationImage1D(class_segmentationMain1.image1Dc_result, class_segmentationMain1.dim_X, class_segmentationMain1.dim_Y, class_segmentationMain1.dim_Z, 3, _V3DPluginCallback2_currentCallback, name_result);
		//visualizationImage1D(class_segmentationMain1.image1D_mask, class_segmentationMain1.dim_X, class_segmentationMain1.dim_Y, class_segmentationMain1.dim_Z, 1, _V3DPluginCallback2_currentCallback, QString("mask"));
		v3dhandleList v3dhandleList_current = _V3DPluginCallback2_currentCallback.getImageWindowList();
		V3DLONG count_v3dhandle = v3dhandleList_current.size();
		for (V3DLONG i=0;i<count_v3dhandle;i++)
		{
			if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(class_segmentationMain1.name_currentWindow))
			{
				_V3DPluginCallback2_currentCallback.setLandmark(v3dhandleList_current[i], class_segmentationMain1.landmarkList_result);
				_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
				break;
			}
		}
		return true;
	}
	else
	{
		v3d_msg("Warning: identification failed;");
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

void landmarkList2file(char* _file_input, LandmarkList _landmarkList_input)
{
	FILE * fp = fopen(_file_input, "w");
	if (!fp)
	{
		return;
	}
	fprintf(fp, "#x, y, z, radius, shape, name, comment\n");
	for (int i=0;i<_landmarkList_input.count(); i++)
	{
		fprintf(fp, "%ld,%ld,%ld,%ld,%ld,%s,%s\n",
			V3DLONG(_landmarkList_input.at(i).x), V3DLONG(_landmarkList_input.at(i).y), V3DLONG(_landmarkList_input.at(i).z),
			V3DLONG(_landmarkList_input.at(i).radius), V3DLONG(_landmarkList_input.at(i).shape),
			_landmarkList_input.at(i).name.c_str(), _landmarkList_input.at(i).comments.c_str());
	}
	fclose(fp);
}

void visualizationImage1D(V3DLONG* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QString string_windowName)
{
	V3DLONG size_page = dim_X*dim_Y*dim_Z*dim_C;
	unsigned char* Image1D_tmp = class_segmentationMain::memory_allocate_uchar1D(size_page);
	for (V3DLONG i=0;i<size_page;i++)
	{
		Image1D_tmp[i] = (unsigned char) Image1D_input[i];
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


void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
	unsigned short* pPre = (unsigned short*)pre1d;
	unsigned short max_v=0, min_v = 255;
	for(V3DLONG i=0; i<imsz; i++)
	{
		if(max_v<pPre[i]) max_v = pPre[i];
		if(min_v>pPre[i]) min_v = pPre[i];
	}
	max_v -= min_v;
	if(max_v>0)
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
		}
	}
	else
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) pPre[i];
		}
	}
}
void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
	float* pPre = (float*)pre1d;
	float max_v=0, min_v = 65535;
	for(V3DLONG i=0; i<imsz; i++)
	{
		if(max_v<pPre[i]) max_v = pPre[i];
		if(min_v>pPre[i]) min_v = pPre[i];
	}
	max_v -= min_v;
	if(max_v>0)
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
		}
	}
	else
	{
		for(V3DLONG i=0; i<imsz; i++)
		{
			pPost[i] = (unsigned char) pPre[i];
		}
	}
}
#pragma endregion