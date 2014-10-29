/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 
#pragma region "headers and constant defination"
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
using namespace std;
const int const_length_histogram = 256;
const double const_max_voxeValue = 255;
const int const_count_neighbors = 26; //27 directions -1;
const double default_threshold_global = 10; //a small enough value for the last resort;
const double default_uThreshold_valueChangeRatio = 0.5; //how much the value can change (relative to the mean value of current region) between current voxel and its neighbor, during seed-based region growing; //this value has been tuned with the sample image;
const double const_max_regionSizeToPageSizeRatio = 0.125; //0.5*0.5*0.5 if a region with voxel count than this #% of the image, an error would be given;
const double const_multiplier_thresholdRegionSize = 0.025;  //it would be a user-specified value in the future !!!;
const int default_threshold_regionSize = 27; //cube of 3 voxel length;
const int const_count_sphereSection = 12;
const int const_max_preGenerateSphereRadius = 50;
const double const_multiplier_bandWidthToDiagnal = 0.75;
const double const_max_sphereGrowMissToTotalRatio = 0.001;
const double const_infinitesimal = 0.000000001;
#define INF 1E9
#define NINF -1E9
#define PI 3.14159265
#pragma endregion

#pragma region "UI-related functions and function defination"
Q_EXPORT_PLUGIN2(cellSegmentation, cellSegmentation);

bool segmentationInterface(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent);
void visualizationImage1D(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName);


QStringList cellSegmentation::menulist() const
{
    return QStringList()
        <<tr("Cell segmentation")
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
    if (menu_name == tr("Cell segmentation"))
	{
        segmentationInterface(V3DPluginCallback2_currentCallback,QWidget_parent);
    }
	else
	{
        v3d_msg(tr("Segmenting neurons;"
            "by Xiang Li (lindbergh.li@gmail.com);"
			));
	}
}

bool cellSegmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & V3DPluginCallback2_currentCallback,  QWidget * QWidget_parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("cellsegmentation"))
	{
		
	}
	else if (func_name == tr("help"))
	{
		
	}
	else
	{
		return false;
	}
	return true;
}
#pragma endregion

#pragma region "class: class_segmentationMain"
class class_segmentationMain
{
	#pragma region "class member"
	public:
		typedef struct 
		{
			double x;
			double y;
			double z;
		} double3D;

		typedef struct 
		{
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
		} int3D;

		struct HeapElem
		{
			int    heap_id;
			V3DLONG   img_ind;
			double value;
			HeapElem(V3DLONG _ind, double _value)
			{
				heap_id = -1;
				img_ind = _ind;
				value = _value;
			}
		};

		struct HeapElemX : public HeapElem
		{
			V3DLONG prev_ind;  // previous img ind
			HeapElemX(V3DLONG _ind, double _value) : HeapElem(_ind, _value){prev_ind = -1;}
		};

		//constant
		vector<V3DLONG> const_vctRelativeIndex_neighbors;
		
		//Input or directly derived;
		enum_algorithm_t type_algorithm;
		bool is_debugging;
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
		bool is_success;
		V3DLONG count_totalWhiteVoxel;
		double threshold_global;
		double threshold_histoSimilar;
		
		//Exemplar (or learn from it);
		LandmarkList LandmarkList_exemplar;
		vector<V3DLONG> poss_exemplar;
		vector<vector<V3DLONG> > possVct_exemplarRegion;
		vector<vector<V3DLONG> > boundBoxVct_exemplarRegion;
		vector<vector<double> > histoVct_exemplarRegion;
		vector<vector<vector<double> > > valueVctVct_exemplarShapeStat;
		double threshold_deltaShapeStat;
		double threshold_regionSize;
		double uThreshold_valueChangeRatio;

		//segmentation;
		vector<vector<V3DLONG> > possVct_segmentationResultOriginal;
		vector<vector<V3DLONG> > possVct_segmentationResultPassed;
		vector<vector<V3DLONG> > possVct_segmentationResultSplitted;
		vector<vector<V3DLONG> > possVct_segmentationResultSmall;
		vector<vector<V3DLONG> > list_seedList;
		unsigned char* Image1D_segmentationResultPassed;
		unsigned char* Image1D_segmentationResultSplitted;
		unsigned char* Image1D_segmentationResultSmall;
		LandmarkList LandmarkList_segmentationResult;
		vector<V3DLONG> poss_segmentationResultCenter;

		//GVF
		vector<double> vct_GVFparamters;

		//GWDT
		int count_boundaryThinIteration;
		double value_boudaryConditionLoosesBy;
		#pragma endregion

		#pragma region "constructor function"
		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input, bool flag_debugInput, enum_algorithm_t type_algorithmInput, vector<double> vct_GVFparamtersInput, int idx_shapInput, double value_deltaInput, double threshold_histoSimilarInput, int count_boundaryThinIterationInput, double value_boudaryConditionLoosesByInput)
		{
			this->is_success = false;
			this->vct_GVFparamters = vct_GVFparamtersInput;
			this->threshold_deltaShapeStat = value_deltaInput;
			this->type_algorithm = type_algorithmInput;
			this->threshold_histoSimilar = threshold_histoSimilarInput;
			this->is_debugging = flag_debugInput;
			this->idx_shape = idx_shapInput;
			this->count_boundaryThinIteration = count_boundaryThinIterationInput;
			this->value_boudaryConditionLoosesBy = value_boudaryConditionLoosesByInput;
			this->Image1D_original = Image1D_input;
			this->dim_X = int_xDimInput; this->dim_Y = int_yDimInput; this->dim_Z = int_zDimInput; this->idx_channel = int_channelInput;
			this->LandmarkList_exemplar = LandmarkList_input;
			this->size_page = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*size_page;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
			this->uThreshold_valueChangeRatio = default_uThreshold_valueChangeRatio;
			this->threshold_regionSize = default_threshold_regionSize;
			this->createNeighborVector();
			this->Image1D_page = memory_allocate_uchar1D(this->size_page);
			this->Image1D_mask = memory_allocate_uchar1D(this->size_page);
			this->Image1D_segmentationResultPassed = memory_allocate_uchar1D(this->size_page);
			this->Image1D_segmentationResultSplitted = memory_allocate_uchar1D(this->size_page);
			this->Image1D_segmentationResultSmall = memory_allocate_uchar1D(this->size_page);
			this->Image3D_page = memory_allocate_uchar3D(this->dim_Y, this->dim_X, this->dim_Z); //tricky!
			vector<V3DLONG> xyz_i (3, 0);
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_page[i] = Image1D_original[i+offset_channel];
				this->Image1D_mask[i] = const_max_voxeValue; //all available;
				this->Image1D_segmentationResultPassed[i] = 0;
				this->Image1D_segmentationResultSplitted[i] = 0;
				this->Image1D_segmentationResultSmall[i] = 0;
				xyz_i = this->index2Coordinate(i);
				this->Image3D_page[xyz_i[2]][xyz_i[1]][xyz_i[0]] = this->Image1D_page[i];
			}
			ofstream ofstream_log;
			ofstream_log.open ("log_class_segmentationMain.txt");
			ofstream_log<<"dim_X: "<<this->dim_X<<", dim_Y: "<<this->dim_Y<<", dim_Z: "<<this->dim_Z<<";"<<endl;
			ofstream_log<<"size_page: "<<this->size_page<<"; "<<"current channel: "<<this->idx_channel<<";"<<endl;
			this->poss_exemplar = landMarkList2IndexList(this->LandmarkList_exemplar);
			if (double_thresholdInput<0) {this->estimateThreshold();} //store to this->threshold_global;
			else {this->threshold_global = double_thresholdInput;}
			ofstream_log<<"estimateThreshold succeed, threshold_global: "<<this->threshold_global<<";"<<endl;
			this->count_totalWhiteVoxel = this->thresholdForCurrentPage();
			V3DLONG count_removedVoxel = this->removeSingleVoxel(this->Image1D_page, this->size_page);
			ofstream_log<<"removeSingleVoxel succeed, "<<count_removedVoxel<<" single voxel removed;"<<endl;
			ofstream_log<<"count_totalWhiteVoxel: "<<this->count_totalWhiteVoxel<<endl;
			if (this->type_algorithm == regionGrowGWDT)
			{
				if (this->GWDF_wrap())
				{
					this->possVct_segmentationResultPassed.clear();
					memset(Image1D_segmentationResultPassed, 0, this->size_page);
					this->possVct_segmentationResultPassed = this->regionGrowOnVector(poss_segmentationResultCenter, this->Image1D_segmentationResultSplitted);
					this->vctList2Image1D(this->Image1D_segmentationResultPassed, this->possVct_segmentationResultPassed, 1);
					this->poss_segmentationResultCenter.clear();
					for (int i=0;i<this->possVct_segmentationResultPassed.size();i++)
					{
						poss_segmentationResultCenter.push_back(this->getCenterByMean(possVct_segmentationResultPassed[i]));
					}
					this->LandmarkList_segmentationResult = this->indexList2LandMarkList(this->poss_segmentationResultCenter);
					this->is_success = true;
				}
				else
				{
					this->is_success = false;
					ofstream_log.close();
					return;
				}
			}
			else if (this->type_algorithm == regionGrowGVF)
			{
				this->categorizeVoxelsByValue(this->histo_page, this->Image1D_page, this->size_page); //store to this->list_seedList; providing seeds for regionGrowing;
				this->possVct_exemplarRegion = this->regionGrowOnVector(this->poss_exemplar);
				if (this->possVct_exemplarRegion.size()<1)
				{
					v3d_msg("Warning: no exemplar regions grown. Program will terminate, please re-select the exemplar(s)!");
					ofstream_log<<"regonGrow for exemplar failed!"<<endl; ofstream_log.close(); return;
				}
				if (!this->analyzeExemplarRegion())
				{
					ofstream_log<<"analyzeExemplarRegion failed!"<<endl; ofstream_log.close(); return; //failed;
				}
				ofstream_log<<"threshold_deltaShapeStat: "<<this->threshold_deltaShapeStat<<endl;
				ofstream_log<<"threshold_regionSize: "<<this->threshold_regionSize<<endl;
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
				this->possVct_segmentationResultOriginal = this->regionGrowOnVectorList(this->list_seedList);
				this->postAnalysis();
				this->LandmarkList_segmentationResult = this->indexList2LandMarkList(this->poss_segmentationResultCenter);
				this->vctList2Image1D(this->Image1D_segmentationResultPassed, this->possVct_segmentationResultPassed, 1);
				this->vctList2Image1D(this->Image1D_segmentationResultSplitted, this->possVct_segmentationResultSplitted, 1+possVct_segmentationResultPassed.size());
				this->vctList2Image1D(this->possVct_segmentationResultSmall, this->Image1D_segmentationResultSmall, const_max_voxeValue);
				this->is_success = true;
			}
			ofstream_log.close();
			v3d_msg(QString("Segmentation succeed! Totally %1 segments!").arg(this->possVct_segmentationResultPassed.size()+this->possVct_segmentationResultSplitted.size()));
			return;
		}
		#pragma endregion

		#pragma region "regionGrow"
		void postAnalysis()
		{
			ofstream ofstream_log;
			if (this->is_debugging) {ofstream_log.open ("log_postAnalysis.txt");}
			V3DLONG count_region = this->possVct_segmentationResultOriginal.size();
			V3DLONG count_voxel = 0;
			bool is_passedShapeStat = true;
			bool is_passedHisto = true;
			vector<V3DLONG> poss_region;
			vector<V3DLONG> boundBox_region;
			vector<vector<V3DLONG> > possVct_GWDT;
			vector<vector<V3DLONG> > possVct_GVF;
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
			vector<double> histo_region (const_length_histogram, 0);
			vector<V3DLONG> poss_splittedRegion;
			V3DLONG pos_center;
			this->possVct_segmentationResultSplitted.clear();
			double*** double3D_GVF;
			V3DLONG size_radius = 0;
			vector<V3DLONG> xyz_center;
			if (this->is_debugging) {ofstream_log<<"total regions: "<<count_region<<";"<<endl;}
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	
				poss_region = this->possVct_segmentationResultOriginal[idx_region];
				pos_center = this->getCenterByMean(poss_region);
				count_voxel = poss_region.size();
				fill (histo_region.begin(), histo_region.end(), 0);
				for (int idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					pos_voxel = poss_region[idx_voxel];
					value_voxel = this->Image1D_page[pos_voxel];
					histo_region[value_voxel]++;
				}
				for (int i=0;i<const_length_histogram;i++)
				{
					histo_region[i] = histo_region[i] / count_voxel;
				}
				is_passedHisto = false;
				for (int j=0; j<this->histoVct_exemplarRegion.size(); j++)
				{
					if (compareHisto(histo_region, this->histoVct_exemplarRegion[j], this->threshold_histoSimilar))
					{
						is_passedHisto = true;
						break;
					}
				}
				if (!is_passedHisto)
				{
					if (this->is_debugging) {ofstream_log<<" histogram deviates too much from exemplar, removed;"<<endl;}
					continue;
				}
				if (this->is_debugging) {ofstream_log<<endl<<endl<<"idx_region: "<<idx_region<<", count_voxel: "<<count_voxel<<endl;}
				if (count_voxel<this->threshold_regionSize)
				{
					if (this->is_debugging) {ofstream_log<<" too small, removed;"<<endl;}
					this->possVct_segmentationResultSmall.push_back(poss_region);
					continue;
				}
				boundBox_region = this->getBoundBox(poss_region);
				if (this->is_debugging) {ofstream_log<<"min_X: "<<boundBox_region[0]<<", max_X: "<<boundBox_region[1]<<", min_Y: "<<boundBox_region[2]<<", max_Y: "<<boundBox_region[3]<<", min_Z: "<<boundBox_region[4]<<", max_Z: "<<boundBox_region[5]<<endl;}
				size_radius = this->getMinDimension(boundBox_region)/2;
				if (this->is_debugging) {ofstream_log<<"size_radius: "<<size_radius<<endl;}
				if (size_radius<5)
				{
					if (count_voxel<this->threshold_regionSize)
					{
						if (this->is_debugging) {ofstream_log<<" too small, removed;"<<endl;}
						this->possVct_segmentationResultSmall.push_back(poss_region);
						continue;
					}
					else
					{
						this->possVct_segmentationResultPassed.push_back(poss_region); //small but not small enough to be removed;
						this->poss_segmentationResultCenter.push_back(pos_center);
						continue;
					}
				}
				pos_center = this->getCenterByMean(poss_region);
				xyz_center = this->index2Coordinate(pos_center);
				x = V3DLONG(xyz_center[0] + 0.5);
				y = V3DLONG(xyz_center[1] + 0.5);
				z = V3DLONG(xyz_center[2] + 0.5);
				if (this->is_debugging) {ofstream_log<<"pos_center: x("<<x<<"), y("<<y<<"), z("<<z<<");"<<endl;}
				valuesVct_regionShapeStat = this->getShapeStat(x, y, z, size_radius); //consisted of 3 vectors with length 4;
				if (valuesVct_regionShapeStat.empty())
				{
					if (count_voxel<this->threshold_regionSize)
					{
						if (this->is_debugging) {ofstream_log<<" too small, removed;"<<endl;}
						this->possVct_segmentationResultSmall.push_back(poss_region);
						continue;
					}
					else
					{
						this->possVct_segmentationResultPassed.push_back(poss_region); //small but not small enough to be removed;
						this->poss_segmentationResultCenter.push_back(pos_center);
						continue;
					}
				}
				values_PC1 = valuesVct_regionShapeStat[0];
				values_PC2 = valuesVct_regionShapeStat[1];
				values_PC3 = valuesVct_regionShapeStat[2];
				if (this->is_debugging) {ofstream_log<<"values_PC1: "<<values_PC1[0]<<" "<<values_PC1[1]<<" "<<values_PC1[2]<<" "<<values_PC1[3]<<endl;}
				if (this->is_debugging) {ofstream_log<<"values_PC2: "<<values_PC2[0]<<" "<<values_PC2[1]<<" "<<values_PC2[2]<<" "<<values_PC2[3]<<endl;}
				if (this->is_debugging) {ofstream_log<<"values_PC3: "<<values_PC3[0]<<" "<<values_PC3[1]<<" "<<values_PC3[2]<<" "<<values_PC3[3]<<endl;}
				for (int j=0; j<this->valueVctVct_exemplarShapeStat.size(); j++)
				{
					is_passedShapeStat = true;
					for (int m=0; m<4; m++)
					{
						if (fabs(values_PC1[m]-valueVctVct_exemplarShapeStat[j][0][m])>this->threshold_deltaShapeStat)
						{
							is_passedShapeStat = false;
							break;
						}
						if (fabs(values_PC2[m]-valueVctVct_exemplarShapeStat[j][1][m])>this->threshold_deltaShapeStat)
						{
							is_passedShapeStat = false;
							break;
						}
						if (fabs(values_PC3[m]-valueVctVct_exemplarShapeStat[j][2][m])>this->threshold_deltaShapeStat)
						{
							is_passedShapeStat = false;
							break;
						}
					}
					if (is_passedShapeStat)
					{
						break; //no need to check other exemplars;
					}
				}
				if (is_passedShapeStat)
				{
					this->possVct_segmentationResultPassed.push_back(poss_region);
					this->poss_segmentationResultCenter.push_back(pos_center);
					if (this->is_debugging) {ofstream_log<<"shape test passed;"<<endl;}
				}
				else
				{
					if (this->is_debugging) {ofstream_log<<"shape test failed;"<<endl;}
					if (this->type_algorithm == regionGrowOnly)
					{
						this->possVct_segmentationResultPassed.push_back(poss_region); //add it anyway;
					}
					else if (this->type_algorithm == regionGrowGVF)
					{
						V3DLONG size_X = boundBox_region[1]-boundBox_region[0]+1;
						V3DLONG size_Y = boundBox_region[3]-boundBox_region[2]+1;
						V3DLONG size_Z = boundBox_region[5]-boundBox_region[4]+1;
						if (this->is_debugging) {ofstream_log<<"region as input to GVF: size_X("<<size_X<<"), size_Y("<<size_Y<<"), size_Z("<<size_Z<<");"<<endl;}
						double3D_GVF = memory_allocate_double3D(size_X, size_Y, size_Z);
						this->centralizeRegion(poss_region, double3D_GVF, boundBox_region); //fill double3D_GVF with voxels of the given region;
						possVct_GVF = class_segmentationMain::GVF_cellSegmentation(double3D_GVF, size_X, size_Y, size_Z, this->is_debugging, this->vct_GVFparamters);
						if (this->is_debugging) {ofstream_log<<"GVF segmented the region into "<<possVct_GVF.size()<<" sub-regions;"<<endl;}
						for (int i=0;i<possVct_GVF.size();i++)
						{
							if (possVct_GVF[i].size()>=this->threshold_regionSize)
							{
								if (this->is_debugging) {ofstream_log<<"sub-region ("<<i+1<<"), size: "<<possVct_GVF[i].size()<<endl;}
								poss_splittedRegion = deCentralizeRegion(possVct_GVF[i], boundBox_region, this->dim_X, this->dim_Y);
								this->possVct_segmentationResultSplitted.push_back(poss_splittedRegion);
								this->poss_segmentationResultCenter.push_back(this->getCenterByMean(poss_splittedRegion));
							}
							else
							{
								if (this->is_debugging) {ofstream_log<<" too small, removed;"<<endl;}
								poss_splittedRegion = deCentralizeRegion(possVct_GVF[i], boundBox_region, this->dim_X, this->dim_Y);
								this->possVct_segmentationResultSmall.push_back(poss_splittedRegion);
							}
						}
						memory_free_double3D(double3D_GVF, size_Z, size_X);
					}
					else if (this->type_algorithm == regionGrowFusing)
					{
						this->possVct_segmentationResultPassed.push_back(poss_region); //not implemented yet;
					}
				}
			}
			return;
		}

		vector<vector<V3DLONG> > regionGrowOnVectorList(vector<vector<V3DLONG> > vctList_seed)
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
				vctList_tmp = regionGrowOnVector(vctList_seed[idx_region]);
				for (int i=0;i<vctList_tmp.size();i++)
				{
					vct_tmp = vctList_tmp[i];
					if (!vct_tmp.empty()) {vctList_result.push_back(vct_tmp); count_totalCoveredVoxel+=vct_tmp.size();}
				}
			}
			return vctList_result;
		}

		vector<vector<V3DLONG> > regionGrowOnVector(vector<V3DLONG> vct_seed)
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
					vct_tmp = this->regionGrowOnSeed(idx_tmp); //Image1D_mask is updating in this function;
					if (!vct_tmp.empty())	{vctList_result.push_back(vct_tmp);}
				}
			}
			return vctList_result;
		}

		vector<vector<V3DLONG> > regionGrowOnVector(vector<V3DLONG> vct_seed, unsigned char* Image1D_input)
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
					vct_tmp = this->regionGrowOnSeed(idx_tmp, Image1D_input); //Image1D_mask is updating in this function;
					if (vct_tmp.size()>8)	{vctList_result.push_back(vct_tmp);}
				}
			}
			return vctList_result;
		}

		vector<V3DLONG> regionGrowOnSeed(V3DLONG idx_seed)
			//assuming [0-const_max_voxeValue] integer Image1D_page, should also work on binary image (not tested yet);
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
			ofstream ofstream_log;
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_temp;
			V3DLONG idx_current;
			V3DLONG idx_neighbor;
			double double_neightborValue;
			double double_currentValue;
			V3DLONG count_totalVolume = 1;
			double double_seedValue = this->Image1D_page[idx_seed];
			double double_meanValue = double_seedValue;
			double double_sumValue = double_seedValue;
			double_meanValue = double_seedValue;
			vct_temp.push_back(idx_seed);
			vct_result.push_back(idx_seed);
			this->Image1D_mask[idx_seed] = 0; //scooped;
			while (true)
			{
				if (vct_temp.empty()) //growing complete;
				{
					if (this->is_debugging) {ofstream_log.close();}
					return vct_result;
				}
				idx_current = vct_temp.back();
				vct_temp.pop_back();
				double_currentValue = this->Image1D_page[idx_current];
				for (int j=0;j<const_count_neighbors;j++)
				{
					idx_neighbor = idx_current+const_vctRelativeIndex_neighbors[j];
					if (this->checkValidity(idx_neighbor)) //prevent it from going out of bounds;
					{
						if (this->Image1D_mask[idx_neighbor]>0) //available only;
						{
							double_neightborValue = this->Image1D_page[idx_neighbor];
							if ((fabs(double_neightborValue-double_currentValue))<(double_meanValue*this->uThreshold_valueChangeRatio))
							{
								this->Image1D_mask[idx_neighbor] = 0; //scooped;
								vct_temp.push_back(idx_neighbor);
								vct_result.push_back(idx_neighbor);
								double_sumValue += double_neightborValue;
								count_totalVolume++;
								double_meanValue = double_sumValue/count_totalVolume;
							}
						}
					}
				}
			}
		}

		vector<V3DLONG> regionGrowOnSeed(V3DLONG idx_seed, unsigned char* Image1D_input)
			//assuming [0-const_max_voxeValue] integer Image1D_page, should also work on binary image (not tested yet);
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
					idx_neighbor = idx_current+const_vctRelativeIndex_neighbors[j];
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
			double mean_neighborValueChange = 0;
			V3DLONG pos_neighbor = 0;
			this->histoVct_exemplarRegion.clear();
			vector<double> histo_region (const_length_histogram, 0);
			V3DLONG count_neighbor;
			double mean_regionValue = 0;
			double max_ratioValueChagne = -INF;
			double ratio_valueChange = 0;
			V3DLONG size_radius = 0;
			vector<vector<double> > valuesVct_shapeStat;
			vector<V3DLONG> boundBox_region;
			this->valueVctVct_exemplarShapeStat.clear();
			this->threshold_regionSize = INF;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				fill (histo_region.begin(), histo_region.end(), 0);
				poss_region = this->possVct_exemplarRegion[idx_region];
				this->possVct_segmentationResultPassed.push_back(poss_region);
				count_voxel = poss_region.size();
				if (count_voxel>(this->count_totalWhiteVoxel*const_max_regionSizeToPageSizeRatio))
				{
					v3d_msg("Warning: abnormal large exemplar region detected, program will terminate. Please re-select the exemplar(s)!");
					return false;
				}
				if (threshold_regionSize>count_voxel*const_multiplier_thresholdRegionSize) {threshold_regionSize=(count_voxel*const_multiplier_thresholdRegionSize);}
				mean_regionValue=0;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					pos_current = poss_region[idx_voxel];
					if (checkValidity(pos_current))
					{
						value_current = this->Image1D_page[pos_current];
						mean_regionValue += value_current;
						histo_region[value_current]++;
					}
				}
				for (int i=0;i<const_length_histogram;i++)
				{
					histo_region[i] = histo_region[i]/count_voxel;
				}
				this->histoVct_exemplarRegion.push_back(histo_region);
				mean_regionValue = mean_regionValue/count_voxel;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					pos_current = poss_region[idx_voxel];
					if (checkValidity(pos_current))
					{
						value_current = this->Image1D_page[pos_current];
						mean_neighborValueChange = 0;
						count_neighbor = 0;
						max_ratioValueChagne = -INF;
						for (int i=0;i<const_count_neighbors;i++)
						{
							pos_neighbor = pos_current + this->const_vctRelativeIndex_neighbors[i];
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
				}
				if (this->uThreshold_valueChangeRatio<max_ratioValueChagne) {this->uThreshold_valueChangeRatio=max_ratioValueChagne;}
	
				//shape property;
				boundBox_region = this->getBoundBox(poss_region);
				boundBoxVct_exemplarRegion.push_back(boundBox_region);
				size_radius = getMinDimension(boundBox_region)/2;
				if (size_radius < 5) {v3d_msg("Warning: exemplar region too small, please re-select the exemplar(s)!"); return false;}
				V3DLONG pos_center = this->getCenterByMean(poss_region);
				this->poss_segmentationResultCenter.push_back(pos_center);
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
			this->list_seedList.clear();
			vector<V3DLONG> idxs_empty (0,0);
			int int_threshold = (int)this->threshold_global;
			for (int i=int_threshold;i<const_length_histogram;i++)
			{
				this->list_seedList.push_back(idxs_empty);
			}
			int int_valueOffset=0;
			int int_valueTmp=0;
			for (int i=0;i<count_imageSize;i++)
			{
				int_valueTmp = (int)Image1D_input[i];
				int_valueOffset = const_max_voxeValue-int_valueTmp;
				if (int_valueTmp>int_threshold)
				{
					list_seedList[int_valueOffset].push_back(i);
				}
			}
		}

		void estimateThreshold()
		{
			double threshold_Yen;
			double threshold_Ostu;
			this->histo_page = class_segmentationMain::getHistogram(this->Image1D_page, this->size_page);
			threshold_Yen = class_segmentationMain::estimateThresholdYen(histo_page);
			threshold_Ostu = class_segmentationMain::estimateThresholdOtsu(histo_page);
			if (threshold_Ostu<threshold_Yen) {this->threshold_global = threshold_Ostu;}
			else {this->threshold_global = threshold_Yen;}
			if (this->threshold_global < default_threshold_global)
			{
				v3d_msg("Warning: threshold_global estimation failed, will use default threshold_global (10) instead!");
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
			for (ih = 0; ih < 256; ih++ ) 
				total+=histo_input[ih];
			for (ih = 0; ih < 256; ih++ )
				norm_histo[ih] = (double)histo_input[ih]/total;
			P1[0]=norm_histo[0];
			for (ih = 1; ih < 256; ih++ )
				P1[ih]= P1[ih-1] + norm_histo[ih];
			P1_sq[0]=norm_histo[0]*norm_histo[0];
			for (ih = 1; ih < 256; ih++ )
				P1_sq[ih]= P1_sq[ih-1] + norm_histo[ih] * norm_histo[ih];
			P2_sq[255] = 0.0;
			for ( ih = 254; ih >= 0; ih-- )
				P2_sq[ih] = P2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];
			value_threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < 256; it++ ) {
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
			for (ih = 1; ih < 256; ih++ )
				P1[ih]= P1[ih-1] + histo_input[ih];
			P1_sq[0]=histo_input[0]*histo_input[0];
			for (ih = 1; ih < 256; ih++ )
				P1_sq[ih]= P1_sq[ih-1] + histo_input[ih] * histo_input[ih];
			P2_sq[255] = 0.0;
			for ( ih = 254; ih >= 0; ih-- )
				P2_sq[ih] = P2_sq[ih + 1] + histo_input[ih + 1] * histo_input[ih + 1];
			value_threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < 256; it++ ) {
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
			for(i=0;i<256;i++)
			{
				s += histo_input[i];
			}
			for(i=0;i<256;i++)
			{
				histo_input[i] = histo_input[i]/s;
			}
			ut = 0.0;
			for(i=0;i<256;i++)
			{
				ut = ut + i*histo_input[i];
			}
			uk = 0.0; wk = 0.0; max = 0.0;
			for(i=0;i<256;i++)
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
			for(i=0;i<256;i++)
			{
				ut = ut + i*histo_input[i];
			}
			uk = 0.0; wk = 0.0; max = 0.0;
			for(i=0;i<256;i++)
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
					Image1D_mask[i] = const_max_voxeValue; //available;
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

		#pragma region "geometry transform"
		static vector<V3DLONG> getOffset(const V3DLONG dim_X, const V3DLONG dim_Y, const V3DLONG dim_Z) //
		{
			vector<V3DLONG> size_result (2, 0);
			size_result[0] = dim_X*dim_Y;
			size_result[1] = dim_X;
			return size_result;
		}
		#pragma endregion

		#pragma region "conversion"
		static vector<vector<V3DLONG> > Image1DToIdxsVct(unsigned char* Image1D_input, const V3DLONG size_page)
		{
			V3DLONG value_voxel = 0;
			V3DLONG pos_voxel = 0;
			vector<V3DLONG> values_used (0, 0);
			int idx_used;
			vector<vector<V3DLONG> > idxsVct_result;
			vector<V3DLONG> idxs_empty(0, 0);
			int count_value = 0;
			for (int i=0;i<size_page;i++)
			{
				pos_voxel = Image1D_input[i];
				value_voxel = (int)Image1D_input[pos_voxel];
				if (value_voxel>0)
				{
					idx_used = vctContains(values_used, value_voxel);
					if (idx_used<0)
					{
						values_used.push_back(value_voxel);
						idxsVct_result.push_back(idxs_empty);
						idxsVct_result[count_value].push_back(pos_voxel);
						count_value++;
					}
					else
					{
						idxsVct_result[idx_used].push_back(pos_voxel);
					}
				}
			}
			return idxsVct_result;
		}

		static vector<vector<V3DLONG> > Image3D2IdxsVct(int*** Image3D_input, const int size_X, const int size_Y, const int size_Z)
		{
			V3DLONG value_voxel = 0;
			V3DLONG pos_voxel= 0;
			vector<V3DLONG> values_used (0, 0);
			int idx_used;
			vector<vector<V3DLONG> > idxsVct_result;
			vector<V3DLONG> idxs_empty(0, 0);
			int count_value = 0;
			for (int z=0;z<size_Z;z++)
			{
				for (int x=0;x<size_X;x++)
				{
					for (int y=0;y<size_Y;y++)
					{
						value_voxel = (int)Image3D_input[z][x][y];
						if (value_voxel>0)
						{
							idx_used = vctContains(values_used, value_voxel);
							pos_voxel= coordinate2Index(x, y, z, size_X, size_X*size_Y);
							if (idx_used<0)
							{
								values_used.push_back(value_voxel);
								idxsVct_result.push_back(idxs_empty);
								idxsVct_result[count_value].push_back(pos_voxel);
								count_value++;
							}
							else
							{
								idxsVct_result[idx_used].push_back(pos_voxel);
							}
						}
					}
				}
			}
			return idxsVct_result;
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

		vector<vector<V3DLONG> > mergeVectorList(vector<vector<V3DLONG> > vctList_input1, vector<vector<V3DLONG> > vctList_input2) //vctList_input2 will be appended to vctList_input1;
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
						idx_neighbor = i+const_vctRelativeIndex_neighbors[j];
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
			return z*offset_Z+y*offset_Y+x;
		}

		static V3DLONG coordinate2Index(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG offset_Y, V3DLONG offset_Z)
		{
			return z*offset_Z+y*offset_Y+x;
		}

		void createNeighborVector()
		{
			this->const_vctRelativeIndex_neighbors.clear();
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
							this->const_vctRelativeIndex_neighbors.push_back(z*this->offset_Z+y*this->offset_Y+x);
						}
					}
				}
			}
		}

		void vctList2Image1D(unsigned char* Image1D_input, vector<vector<V3DLONG> > vctList_input, int value_base)
		{
			memset(Image1D_input, 0, this->size_page);
			for (int i=0;i<vctList_input.size();i++)
			{
				vct2Image1D(Image1D_input, vctList_input[i], i+value_base);
			}
		}

		void vctList2Image1D(vector<vector<V3DLONG> > vctList_input, unsigned char* Image1D_input, int value_fill)
		{
			memset(Image1D_input, 0, this->size_page);
			for (int i=0;i<vctList_input.size();i++)
			{
				vct2Image1D(Image1D_input, vctList_input[i], value_fill);
			}
		}

		void vct2Image1D(unsigned char* Image1D_input, vector<V3DLONG> vct_input, V3DLONG int_value)
		{
			for (int i=0;i<vct_input.size();i++)
			{
				if (this->checkValidity(vct_input[i]))
				{
					Image1D_input[vct_input[i]] = int_value;
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

		void centralizeRegion(const vector<V3DLONG> vct_input, double*** double3D_output, const vector<V3DLONG> vct_boundBox)
		{
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_coordinate (0, 0);
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG count_voxel = vct_input.size();
			V3DLONG min_X = vct_boundBox[0];
			V3DLONG min_Y = vct_boundBox[2];
			V3DLONG min_Z = vct_boundBox[4];
			V3DLONG size_X = vct_boundBox[1] - vct_boundBox[0];
			V3DLONG size_Y = vct_boundBox[3] - vct_boundBox[2];
			V3DLONG size_Z = vct_boundBox[5] - vct_boundBox[4];
			for (z=0;z<size_Z;z++)
			{
				for (y=0;y<size_Y;y++)
				{
					for (x=0;x<size_X;x++)
					{
						double3D_output[z][x][y] = 0;
					}
				}
			}
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				idx_tmp = vct_input[idx_voxel];
				vct_coordinate = this->index2Coordinate(idx_tmp);
				x = vct_coordinate[0] - min_X;
				y = vct_coordinate[1] - min_Y;
				z = vct_coordinate[2] - min_Z;
				double3D_output[z][x][y] = (double) this->Image1D_page[idx_tmp];
			}
			return;
		}

		static vector<V3DLONG> deCentralizeRegion(vector<V3DLONG> vct_input, vector<V3DLONG> vct_boundBox, V3DLONG dim_X, V3DLONG dim_Y)
		{
			vector<V3DLONG> vct_result;
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_coordinate (0, 0);
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG count_voxel = vct_input.size();
			V3DLONG min_X =  vct_boundBox[0];
			V3DLONG min_Y =  vct_boundBox[2];
			V3DLONG min_Z =  vct_boundBox[4];
			V3DLONG size_X = vct_boundBox[1] - vct_boundBox[0] + 1;
			V3DLONG size_Y = vct_boundBox[3] - vct_boundBox[2] + 1;
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				idx_tmp = vct_input[idx_voxel];
				vct_coordinate = class_segmentationMain::index2Coordinate(idx_tmp, size_X, size_X*size_Y);
				x = vct_coordinate[0] + min_X;
				y = vct_coordinate[1] + min_Y;
				z = vct_coordinate[2] + min_Z;
				vct_result.push_back(class_segmentationMain::coordinate2Index(x, y, z, dim_X, dim_X*dim_Y));
			}
			return vct_result;
		}

		V3DLONG getCenterByMean(V3DLONG pos_input1, V3DLONG pos_input2)
		{
			vector<V3DLONG> xyz_voxel1 = this->index2Coordinate(pos_input1);
			vector<V3DLONG> xyz_voxe2 = this->index2Coordinate(pos_input2);
			V3DLONG x1 = xyz_voxel1[0];
			V3DLONG y1 = xyz_voxel1[1];
			V3DLONG z1 = xyz_voxel1[2];
			V3DLONG x2 = xyz_voxe2[0];
			V3DLONG y2 = xyz_voxe2[1];
			V3DLONG z2 = xyz_voxe2[2];
			V3DLONG x3 = (x1+x2)/2;
			V3DLONG y3 = (y1+y2)/2;
			V3DLONG z3 = (z1+z2)/2;
			return this->coordinate2Index(x3, y3, z3);
		}

		V3DLONG getCenterByMedian(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> vct_coordinate;
			V3DLONG idx_result;
			vector<V3DLONG> vct_x;
			vector<V3DLONG> vct_y;
			vector<V3DLONG> vct_z;
			V3DLONG count_voxel = vct_input.size(); 
			V3DLONG idx_median = count_voxel/2;
			for (int i=0;i<count_voxel;i++)
			{
				vct_coordinate = this->index2Coordinate(vct_input[i]);
				vct_x.push_back(vct_coordinate[0]);
				vct_y.push_back(vct_coordinate[1]);
				vct_z.push_back(vct_coordinate[2]);
			}
			sort(vct_x.begin(), vct_x.end());
			sort(vct_y.begin(), vct_y.end());
			sort(vct_z.begin(), vct_z.end());
			idx_result = this->coordinate2Index(vct_x[idx_median], vct_y[idx_median], vct_z[idx_median]);
			return this->getNearestVoxel(vct_input, idx_result);
		}

		V3DLONG getCenterByMean(V3DLONG sum_X, V3DLONG sum_Y, V3DLONG sum_Z)
		{
			V3DLONG pos_result = this->coordinate2Index(sum_X/2, sum_Y/2, sum_Z/2);
			return pos_result;
		}

		V3DLONG getCenterByMean(vector<V3DLONG> poss_input)
		{
			vector<V3DLONG> xyz_voxel;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			double sum_X=0;
			double sum_Y=0;
			double sum_Z=0;
			V3DLONG count_voxel = poss_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				xyz_voxel = this->index2Coordinate(poss_input[i]);
				x=xyz_voxel[0];
				y=xyz_voxel[1];
				z=xyz_voxel[2];
				sum_X += (double)x;
				sum_Y += (double)y;
				sum_Z += (double)z;
			}
			double result = this->coordinate2Index(sum_X/count_voxel, sum_Y/count_voxel, sum_Z/count_voxel);
			return result;
		}


		V3DLONG getNearestVoxel(vector<V3DLONG> vct_input, V3DLONG idx_input)
		{
			double min_distance = INF;
			double tmp_distance = 0;
			V3DLONG idx_result = -1;
			V3DLONG count_voxel = vct_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				tmp_distance = this->getEuclideanDistance(idx_input, vct_input[i]);
				if (tmp_distance<min_distance)
				{
					min_distance = tmp_distance;
					idx_result = vct_input[i];
				}
			}
			return idx_result;
		}

		V3DLONG getCenterByMass(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> vct_coordinate;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			double sum_X=0;
			double sum_Y=0;
			double sum_Z=0;
			double sum_mass = 0;
			double value_tmp = 0;
			V3DLONG count_voxel = vct_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				vct_coordinate = this->index2Coordinate(vct_input[i]);
				value_tmp = this->Image1D_page[vct_input[i]];
				x=vct_coordinate[0];
				y=vct_coordinate[1];
				z=vct_coordinate[2];
				sum_X += (double)x*value_tmp;
				sum_Y += (double)y*value_tmp;
				sum_Z += (double)z*value_tmp;
				sum_mass += value_tmp;
			}
			double result = this->coordinate2Index(sum_X/sum_mass, sum_Y/sum_mass, sum_Z/sum_mass);
			return result;
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

		static int3D ***memory_allocate_int3D3(const int i_size, const int j_size, const int k_size)
		{
			int3D ***ptr_result;
			int i,k;
			ptr_result=(int3D ***) calloc(k_size,sizeof(int3D **));
			for(k=0;k<k_size;k++)
			{
				ptr_result[k]=(int3D **) calloc(i_size,sizeof(int3D *));
			}
			for(k=0;k<k_size;k++)
			{
				for(i=0;i<i_size;i++)
				{
					ptr_result[k][i]=(int3D *) calloc(j_size,sizeof(int3D));
				}
			}
			return(ptr_result);
		}

		static void memory_free_int3D3(int3D ***ptr_input, const int k_size, const int i_size)
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
		vector<vector<V3DLONG> >  GVF_cellSegmentation(double *** Image3D_input, const int dim_X, const int dim_Y, const int dim_Z, bool is_debugging, vector<double> vct_GVFparamters)
		{
			ofstream ofstream_log;
			if (is_debugging) {ofstream_log.open ("log_GVF.txt");}
			int para_maxIteration = vct_GVFparamters[0];
			int para_fusionThreshold = vct_GVFparamters[1];
			double para_sigma = vct_GVFparamters[2];
			double para_mu = vct_GVFparamters[3];
			double3D ***Image3D3_u;
			double3D ***Image3D3_u_normalized;
			double3D ***Image3D3_f;
			double3D ***Image3D3_gradient;
			Image3D3_gradient = memory_allocate_double3D3(dim_X, dim_Y, dim_Z);
			int3D ***Image3D3_mode;
			int ***Image3D_label;
			int count_page = dim_X*dim_Y*dim_Z;
			int smoothIteration = (int)(3*para_sigma + 0.5);
			if (is_debugging) {ofstream_log<<"initialization passed!"<<endl;}
			smooth_GVFkernal(Image3D_input, smoothIteration, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"smoothing passed!"<<endl;}
			GVF_getGradient(Image3D_input, Image3D3_gradient, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"get gradient passed!"<<endl;}
			Image3D3_f = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			Image3D3_u = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			GVF_initialize(Image3D3_gradient, Image3D3_f, Image3D3_u, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"f/u initialization passed!"<<endl;}
			memory_free_double3D3(Image3D3_gradient,dim_Z,dim_X);
			GVF_warp(para_maxIteration, para_mu, Image3D3_u, Image3D3_f, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"warp passed!"<<endl;}
			memory_free_double3D3(Image3D3_f,dim_Z,dim_X);
			Image3D3_u_normalized = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			GVF_normalize(Image3D3_u, Image3D3_u_normalized, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"normalization passed!"<<endl;}
			memory_free_double3D3(Image3D3_u,dim_Z,dim_X);
			Image3D3_mode = memory_allocate_int3D3(dim_X,dim_Y,dim_Z);
			GVF_findMode(Image3D3_u_normalized, Image3D3_mode, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"findMode passed!"<<endl;}
			memory_free_double3D3(Image3D3_u_normalized, dim_Z, dim_X);
			Image3D_label = memory_allocate_int3D(dim_X, dim_Y, dim_Z);
			GVF_fuseMode( Image3D3_mode, Image3D_label, para_fusionThreshold, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"fuseMode passed!"<<endl;}
			memory_free_int3D3(Image3D3_mode,dim_Z,dim_X);
			//unsigned char ***Image3D_edge;
			//Image3D_edge = memory_allocate_uchar3D(dim_X,dim_Y,dim_Z);
			//GVF_majorFilter(Image3D_label, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"major fitler1 passed!"<<endl;}
			//GVF_findEdge(Image3D_label, Image3D_edge, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"find edge passed!"<<endl;}
			GVF_localThresholding(Image3D_input, Image3D_label, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"local thresholding passed!"<<endl;}
			//GVF_majorFilter(Image3D_result, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"major fitler2 passed!"<<endl;}
			vector<vector<V3DLONG> > vctList_result = Image3D2IdxsVct(Image3D_label, dim_X, dim_Y, dim_Z);
			memory_free_int3D(Image3D_label,dim_Z,dim_X);
			if (is_debugging) {ofstream_log<<"all succeed!"<<endl;}
			return vctList_result;
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

		static void GVF_findMode(double3D ***u, int3D ***mode, int x_size, int y_size, int z_size)
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

		static void GVF_fuseMode(int3D ***mode, int ***visit, const double fuse_th, const int x_size, const int y_size, const int z_size)
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

		static void GVF_fill(int x, int y, int z, int label, int3D ***mode, int ***visit, int *modePointCounts, int **indexTable, int fuse_th, int x_size, int y_size, int z_size)
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

		void GVF_localThresholding(double ***Image3D_input, int ***Image3D_label, int dim_X, int dim_Y, int dim_Z)
		{
			V3DLONG x, y, z;
			double threshold_global = 0;
			double *histo_input;
			int label_voxel = 0;
			int value_voxel = 0;
			histo_input = (double *)malloc(sizeof(double)*const_length_histogram);
			for(int i=0;i<const_length_histogram;i++)
			{
				histo_input[i] = 0;
			}
			threshold_global = estimateThresholdYen(histo_input);
			free(histo_input);
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						label_voxel = Image3D_label[z][x][y];
						value_voxel = Image3D_input[z][x][y];
						if((label_voxel>0) && (value_voxel>threshold_global))
						{
							Image3D_label[z][x][y] = label_voxel;
						}
						else
						{
							Image3D_label[z][x][y] = 0;
						}
					}
				}
			}
			return;
		}

		/*void GVF_localThresholding(double ***Image3D_input, int ***Image3D_label, int dim_X, int dim_Y, int dim_Z)
		{
			int x, y, z;
			int idx_label, i;
			int threshold_global = 0;
			int **vctList_histogram;
			double *vct_histogram;
			int *vct_threshold;
			int count_label = 0;
			int tmp_label = 0;
			int tmp_value = 0;
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						tmp_label = Image3D_label[z][x][y];
						if(tmp_label>count_label)
						{
							count_label = tmp_label;
						}
					}
				}
			}
			count_label = count_label + 1;
			vctList_histogram = memory_allocate_int2D(count_label, const_length_histogram);
			vct_histogram = (double *)malloc(sizeof(double)*const_length_histogram);
			for(i=0;i<const_length_histogram;i++)
			{
				vct_histogram[i] = 0;
			}
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						tmp_label = Image3D_label[z][x][y];
						tmp_value = (int)Image3D_input[z][x][y];
						vctList_histogram[tmp_label][tmp_value]++;
						vct_histogram[tmp_value]++;
					}
				}
			}
			threshold_global = estimateThresholdYen(vct_histogram);
			vct_threshold = (int *)malloc(sizeof(int)*(count_label));
			for(idx_label=0;idx_label<count_label;idx_label++)
			{
				for(i=0;i<const_length_histogram;i++)
				{
					vct_histogram[i] = (double)vctList_histogram[idx_label][i];
				}
				vct_threshold[idx_label] = estimateThresholdYen(vct_histogram);
			}
			free(vct_histogram);
			
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						tmp_label = Image3D_label[z][x][y];
						tmp_value = Image3D_input[z][x][y];
						if((tmp_label>0) && (tmp_value>vct_threshold[tmp_label]) && (tmp_value>threshold_global))
						{
							Image3D_label[z][x][y] = tmp_label;
						}
						else
						{
							Image3D_label[z][x][y] = 0;
						}
					}
				}
			}
			memory_free_int2D(vctList_histogram,count_label);
			free(vct_threshold);
			return;
		}*/
		#pragma endregion

		#pragma region "shapeStat"
		vector<vector<double> > getShapeStat(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG value_radius)
		{
			vector<vector<double> > valuesVct_result;
			if (value_radius<5) //minimum radius of 5, for 4 sample steps;
			{
				return valuesVct_result;
			}
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

		#pragma region "heap"
		template <class T>
		class BasicHeap
		{
		public:
			BasicHeap()
			{
				elems.reserve(10000);
			}
			T* delete_min()
			{
				if(elems.empty()) return 0;
				T* min_elem = elems[0];
				if(elems.size()==1) elems.clear();
				else
				{
					elems[0] = elems[elems.size() - 1];
					elems[0]->heap_id = 0;
					elems.erase(elems.begin() + elems.size() - 1);
					down_heap(0);
				}
				return min_elem;
			}
			void insert(T* t)
			{
				elems.push_back(t);
				t->heap_id = elems.size() - 1;
				up_heap(t->heap_id);
			}
			bool empty(){return elems.empty();}
			void adjust(int id, double new_value)
			{
				double old_value = elems[id]->value;
				elems[id]->value = new_value;
				if(new_value < old_value) up_heap(id);
				else if(new_value > old_value) down_heap(id);
			}
		private:
			vector<T*> elems;
			bool swap_heap(int id1, int id2)
			{
				if(id1 < 0 || id1 >= elems.size() || id2 < 0 || id2 >= elems.size()) return false;
				if(id1 == id2) return false;
				int pid = id1 < id2 ? id1 : id2;
				int cid = id1 > id2 ? id1 : id2;
				assert(cid == 2*(pid+1) -1 || cid == 2*(pid+1));
				if(elems[pid]->value <= elems[cid]->value) return false;
				else
				{
					T * tmp = elems[pid];
					elems[pid] = elems[cid];
					elems[cid] = tmp;
					elems[pid]->heap_id = pid;
					elems[cid]->heap_id = cid;
					return true;
				}
			}
			void up_heap(int id)
			{
				int pid = (id+1)/2 - 1;
				if(swap_heap(id, pid)) up_heap(pid);
			}
			void down_heap(int id)
			{
				int cid1 = 2*(id+1) -1;
				int cid2 = 2*(id+1);
				if(cid1 >= elems.size()) return;
				else if(cid1 == elems.size() - 1)
				{
					swap_heap(id, cid1);
				}
				else if(cid1 < elems.size() - 1)
				{
					int cid = elems[cid1]->value < elems[cid2]->value ? cid1 : cid2;
					if(swap_heap(id, cid)) down_heap(cid);
				}
			}
		};
		#pragma endregion

		#pragma region "GWDT"
		bool GWDF_wrap()
		{
			float * phi = 0;
			int cnn_type = 1;
			double z_thickness = 1;
			if (fastmarching_dt(this->Image1D_page, phi, this->dim_X, this->dim_Y, this->dim_Z, cnn_type, threshold_global, z_thickness))
			{
				float min_val = phi[0], max_val = phi[0];
				for(V3DLONG i = 0; i < this->size_page; i++) {if(phi[i] == INF) continue; min_val = MIN(min_val, phi[i]); max_val = MAX(max_val, phi[i]);}
				max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;
				for(V3DLONG i = 0; i < this->size_page; i++)
				{
					if(phi[i] == INF) this->Image1D_segmentationResultSplitted[i] = 0;
					else if(phi[i] ==0) this->Image1D_segmentationResultSplitted[i] = 0;
					else
					{
						this->Image1D_segmentationResultSplitted[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
						this->Image1D_segmentationResultSplitted[i] = MAX(this->Image1D_segmentationResultSplitted[i], 1);
					}
				}
				delete [] phi; phi = 0;
				int count_label = 0;
				double value_current = 0;
				V3DLONG pos_neighbor = 0;
				double value_neighbor = 0;
				bool is_center = true; //local maxima;
				bool is_boundary = false; //local minima;
				this->poss_segmentationResultCenter.clear();
				vector<V3DLONG> vct_boundary;
				vector<V3DLONG> vct_boundaryAll;
				for (V3DLONG i=0;i<this->size_page;i++)
				{
					value_current = this->Image1D_segmentationResultSplitted[i];
					if (value_current > 0) //it is a front voxel;
					{
						is_center = true;
						for (int j=0;j<const_count_neighbors;j++) //look for its neighbor;
						{
							pos_neighbor = i + this->const_vctRelativeIndex_neighbors[j];
							if (this->checkValidity(pos_neighbor)) //preventing it from going out of bounds;
							{
								value_neighbor = this->Image1D_segmentationResultSplitted[pos_neighbor];
								if ((value_neighbor >= value_current))
								{
									is_center = false;
								}
							}
						}
						if (is_center)
						{
							poss_segmentationResultCenter.push_back(i);
						}
					}
				}
				for (int z=0;z<count_boundaryThinIteration;z++)
				{
					vct_boundary.clear();
					for (V3DLONG i=0;i<this->size_page;i++)
					{
						value_current = this->Image1D_segmentationResultSplitted[i];
						if (value_current > 0) //it is a front voxel;
						{
							is_boundary = true;
							for (int j=0;j<const_count_neighbors;j++) //look for its neighbor;
							{
								pos_neighbor = i + this->const_vctRelativeIndex_neighbors[j];
								if (this->checkValidity(pos_neighbor)) //preventing it from going out of bounds;
								{
									value_neighbor = this->Image1D_segmentationResultSplitted[pos_neighbor];
									if ((value_neighbor < (value_current-value_boudaryConditionLoosesBy)))
									{
										if (value_neighbor>0)
										{
											is_boundary = false;
										}
									}
								}
							}
							if (is_boundary)
							{
								vct_boundary.push_back(i);
								vct_boundaryAll.push_back(i);
							}
						}
					}
					for (int i=0;i<vct_boundary.size();i++)
					{
						this->Image1D_segmentationResultSplitted[vct_boundary[i]] = 0;
					}
				}
				this->vct2Image1D(this->Image1D_segmentationResultSmall, vct_boundaryAll, 255);
				return true;
			}
			else
			{
				return false;
			}
		}

		template<class T> bool fastmarching_dt(T * inimg1d, float * &phi, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, int cnn_type = 3, int bkg_thresh = 0, double z_thickness = 1.0)
		{
			enum{ALIVE = -1, TRIAL = 0, FAR = 1};
			V3DLONG tol_sz = sz0 * sz1 * sz2;
			V3DLONG sz01 = sz0 * sz1;
			if(phi == 0) phi = new float[tol_sz];
			char * state = new char[tol_sz];
			for(V3DLONG i = 0; i < tol_sz; i++)
			{
				if(inimg1d[i] <= bkg_thresh)
				{
					phi[i] = inimg1d[i];
					state[i] = ALIVE;
				}
				else
				{
					phi[i] = INF;
					state[i] = FAR;
				}
			}
			BasicHeap<HeapElem> heap;
			map<V3DLONG, HeapElem*> elems;
			{
				V3DLONG i = -1, j = -1, k = -1;
				for(V3DLONG ind = 0; ind < tol_sz; ind++)
				{
					i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
					if(state[ind] == ALIVE)
					{
						for(int kk = -1; kk <= 1; kk++)
						{
							V3DLONG k2 = k+kk;
							if(k2 < 0 || k2 >= sz2) continue;
							for(int jj = -1; jj <= 1; jj++)
							{
								V3DLONG j2 = j+jj;
								if(j2 < 0 || j2 >= sz1) continue;
								for(int ii = -1; ii <=1; ii++)
								{
									V3DLONG i2 = i+ii;
									if(i2 < 0 || i2 >= sz0) continue;
									int offset = ABS(ii) + ABS(jj) + ABS(kk);
									if(offset == 0 || offset > cnn_type) continue;
									V3DLONG ind2 = k2 * sz01 + j2 * sz0 + i2;
									if(state[ind2] == FAR)
									{
										V3DLONG min_ind = ind;
										// get minimum Alive point around ind2
										if(phi[min_ind] > 0.0)
										{
											for(int kkk = -1; kkk <= 1; kkk++)
											{
												V3DLONG k3 = k2 + kkk;
												if(k3 < 0 || k3 >= sz2) continue;
												for(int jjj = -1; jjj <= 1; jjj++)
												{
													V3DLONG j3 = j2 + jjj;
													if(j3 < 0 || j3 >= sz1) continue;
													for(int iii = -1; iii <= 1; iii++)
													{
														V3DLONG i3 = i2 + iii;
														if(i3 < 0 || i3 >= sz0) continue;
														int offset2 = ABS(iii) + ABS(jjj) + ABS(kkk);
														if(offset2 == 0 || offset2 > cnn_type) continue;
														V3DLONG ind3 = k3 * sz01 + j3 * sz0 + i3;
														if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]) min_ind = ind3;
													}
												}
											}
										}
										// over
										phi[ind2] = phi[min_ind] + inimg1d[ind2];
										state[ind2] = TRIAL;
										HeapElem * elem = new HeapElem(ind2, phi[ind2]);
										heap.insert(elem);
										elems[ind2] = elem;
									}
								}
							}
						}
					}
				}
			}
			double process1 = 0;
			while(!heap.empty())
			{
				HeapElem* min_elem = heap.delete_min();
				elems.erase(min_elem->img_ind);
				V3DLONG min_ind = min_elem->img_ind;
				delete min_elem;
				state[min_ind] = ALIVE;
				V3DLONG i = min_ind % sz0;
				V3DLONG j = (min_ind/sz0) % sz1;
				V3DLONG k = (min_ind/sz01) % sz2;
				int w, h, d;
				for(int kk = -1; kk <= 1; kk++)
				{
					d = k+kk;
					if(d < 0 || d >= sz2) continue;
					for(int jj = -1; jj <= 1; jj++)
					{
						h = j+jj;
						if(h < 0 || h >= sz1) continue;
						for(int ii = -1; ii <= 1; ii++)
						{
							w = i+ii;
							if(w < 0 || w >= sz0) continue;
							int offset = ABS(ii) + ABS(jj) + ABS(kk);
							if(offset == 0 || offset > cnn_type) continue;
							double factor = 0.0;
							V3DLONG index = d*sz01 + h*sz0 + w;
							if(state[index] != ALIVE)
							{
								float new_dist = phi[min_ind] + inimg1d[index] * sqrt(ii*ii + jj*jj + kk * kk * z_thickness* z_thickness);
								if(state[index] == FAR)
								{
									phi[index] = new_dist;
									HeapElem * elem = new HeapElem(index, phi[index]);
									heap.insert(elem);
									elems[index] = elem;
									state[index] = TRIAL;
								}
								else if(state[index] == TRIAL)
								{
									if(phi[index] > new_dist)
									{
										phi[index] = new_dist;
										HeapElem * elem = elems[index];
										heap.adjust(elem->heap_id, phi[index]);
									}
								}
							}
						}
					}
				}
			}
			assert(elems.empty());
			if(state) {delete [] state; state = 0;}
			return true;
		}
		#pragma endregion
};
#pragma endregion

#pragma region "Segmentation interface"
bool segmentationInterface(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
	//get current image;
    v3dhandle v3dhandle_currentWindow = V3DPluginCallback2_currentCallback.currentImageWindow();
    //cancels if no image loaded;
    if (!v3dhandle_currentWindow)
    {
        v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		return false;
    }
    //try to pull the data as an Image4DSimple (3D+channel);
    Image4DSimple* Image4DSimple_current = V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
	if (!Image4DSimple_current)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		return false;
	}
	V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
	if (count_totalBytes < 1)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		return false;
	}
	ImagePixelType ImagePixelType_current = Image4DSimple_current->getDatatype();
	if(ImagePixelType_current != V3D_UINT8)
	{
		v3d_msg("Currently this program only support 8-bit data, program canceled!");
		return false;
	}
	//sets data into 1D array, note: size is count_totalVoxelsCurrent;
    unsigned char* Image1D_current = Image4DSimple_current->getRawData();
	//get the currenlty-opened file name;
    QString QString_fileNameCurrent = V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
    //defining dimensions;
    V3DLONG dim_X = Image4DSimple_current->getXDim();
    V3DLONG dim_Y = Image4DSimple_current->getYDim();
    V3DLONG dim_Z = Image4DSimple_current->getZDim();
    V3DLONG int_channelDim = Image4DSimple_current->getCDim();
	//get current LandmarkList (one way of defining exemplars);
    LandmarkList LandmarkList_userDefined = V3DPluginCallback2_currentCallback.getLandmark(v3dhandle_currentWindow);
	V3DLONG count_userDefinedLandmarkList = LandmarkList_userDefined.count();
    //get current NeuronTreeList (i.e. SWC list, another way of defining exemplars);
    QList<NeuronTree> * SWCList_current = V3DPluginCallback2_currentCallback.getHandleNeuronTrees_3DGlobalViewer(v3dhandle_currentWindow);
	V3DLONG count_SWCList = 0;
	if (!SWCList_current)
	{
		//no SWC loaded, do nothing to avoid pointer to NULL;
	}
	else
	{
		count_SWCList = SWCList_current->count();
	}
    //check landmark availability;
	LandmarkList LandmarkList_current;
	V3DLONG count_currentLandmarkList = -1;
	if ((count_SWCList<1) && (count_userDefinedLandmarkList<1)) //Both of the lists are empty;
    {
        v3d_msg("You have not defined any landmarks or swc structure to run the segmenation, program canceled!");
        return false;
    }
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList>0)) //Both of the lists are not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
	}
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1)) //Only SWCList_current is not empty;
	{
		class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
	}
	if (count_userDefinedLandmarkList>0) //Only LandmarkList_userDefined is not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		count_currentLandmarkList = LandmarkList_current.count();
	}
	//open dialogMain1 window, get paramters;
    dialogMain dialogMain1(V3DPluginCallback2_currentCallback, QWidget_parent, int_channelDim);
    if (dialogMain1.exec()!=QDialog::Accepted)
    {
        return false;
    }
    V3DLONG idx_channel = dialogMain1.int_channel;
    if (dialogMain1.double_threshold>const_max_voxeValue)
	{
		v3d_msg("Please provide a valid threshold_global, program canceled!"); 
		return false;
	}
	int idx_shape;
	if (dialogMain1.type_shape == sphere) {idx_shape = 1;}
	else if (dialogMain1.type_shape == cube) {idx_shape = 0;}
	vector<double> vct_GVFparamters (4, 0);
	vct_GVFparamters[0] = dialogMain1.double_GVF_maxIteration;
	vct_GVFparamters[1] = dialogMain1.double_GVF_fusionThreshold;
	vct_GVFparamters[2] = dialogMain1.double_GVF_sigma;
	vct_GVFparamters[3] = dialogMain1.double_GVF_mu;
	class_segmentationMain segmentationMain1(dialogMain1.double_threshold, Image1D_current, dim_X, dim_Y, dim_Z, idx_channel, LandmarkList_current, dialogMain1.flag_debug, dialogMain1.type_algorithm, vct_GVFparamters, idx_shape, dialogMain1.value_Shape_delta, dialogMain1.value_threshold_thresholdHistoSimilar, dialogMain1.count_boundaryThinIteration, dialogMain1.value_boudaryConditionLoosesBy);
	if (!segmentationMain1.is_success)
	{
		v3d_msg("warning:initialization of segmentationMain failed, program will terminate!");
		return false;
	}
	visualizationImage1D(segmentationMain1.Image1D_segmentationResultPassed,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result (exemplar-like shape)");
	visualizationImage1D(segmentationMain1.Image1D_segmentationResultSplitted,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result (splitted from irregular shapes)");
	visualizationImage1D(segmentationMain1.Image1D_segmentationResultSmall,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result (very small shapes)");
	V3DPluginCallback2_currentCallback.setLandmark(v3dhandle_currentWindow, segmentationMain1.LandmarkList_segmentationResult);
	V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_currentWindow);
    return true;
}

void visualizationImage1D(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
{
	Image4DSimple Image4DSimple_temp;
	Image4DSimple_temp.setData((unsigned char*)Image1D_input, dim_X, dim_Y, dim_Z, int_channelDim, V3D_UINT8);
	v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
	V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
	V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
	V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
}
#pragma endregion