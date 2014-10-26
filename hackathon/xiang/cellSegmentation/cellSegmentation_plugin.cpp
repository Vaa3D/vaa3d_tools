/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 
#pragma region "headers and constant defination"
#include "v3d_message.h"
#include "cellSegmentation_plugin.h"
#include <vector>
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <basic_landmark.h>
using namespace std;
const int const_length_histogram = 256;
const double const_max_voxeValue = 255;
const int const_count_neighbors = 26; //27 directions -1;
const double const_threshold_default = 10; //a small enough value for the last resort;
const double const_max_valueRatioChange_default = 0.5; //how much the value can change (relative to the mean value of current region) between current voxel and its neighbor, during seed-based region growing; //this value has been tuned with the sample image;
const double const_max_regionSizeToPageSizeRatio = 0.125; //0.5*0.5*0.5 if a region with voxel count than this #% of the image, an error would be given;
const double const_multiplier_regionSizeLowerBound = 0.025;  //it would be a user-specified value in the future !!!;
const int const_min_regionSize = 10; //cubic of 3 voxel length;
const int const_count_sphereSection = 12;
const int const_max_preGenerateSphereRadius = 50;
const double const_multiplier_bandWidthToDiagnal = 0.75;
const double const_max_sphereGrowMissToTotalRatio = 0.001;
const int const_max_GVF_iteration = 5;
const double const_infinitesimal = 0.000000001;
const int const_int_GVF_majorFilterIteration = 3;
const double const_threshold_GVF_fution = 3;
const double const_threshold_GVF_sigma = 0.1;
const double const_threshold_GVF_lambda = 0.2;
const double const_threshold_GVF_mu = 0.1;
const double const_threshold_GVF_omega = 1.0;
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

		//constant
		vector<V3DLONG> const_vctRelativeIndex_neighbors;
		
		//Input or directly derived;
		int idx_algorithm;
		bool is_debugging;
		double threshold;
		unsigned char* Image1D_original;
		unsigned char *Image1D_page;
		unsigned char *Image1D_mask;
		V3DLONG dim_X;
		V3DLONG dim_Y;
		V3DLONG dim_Z;
		V3DLONG size_page;
		V3DLONG offset_channel;
		V3DLONG offset_Z;
		V3DLONG offset_Y;
		int idx_channel;
		vector<double> histo_main;
		bool is_success;
		V3DLONG count_totalWhiteVoxel;
		
		//Exemplar (or learn from it);
		LandmarkList LandmarkList_exemplar;
		vector<V3DLONG> vct_exemplar;
		vector<V3DLONG> vc_exemplarSphereSize;
		unsigned char *Image1D_exemplarRegion;
		vector<vector<V3DLONG> > vctList_exemplarRegion;
		double upperBound_valueRatioChange;
		double lowerBound_sphereToSizeRatio;
		double lowerBound_regionSize;
		
		//segmentation;
		vector<vector<V3DLONG> > vctList_segmentationResult;
		vector<vector<V3DLONG> > vctList_potentialSeedList;
		vector<vector<V3DLONG> > vctList_preGeneratedSphere;
		unsigned char* Image1D_segmentationResult;
		LandmarkList LandmarkList_segmentationResult;
		vector<V3DLONG> vct_segmentationResultCenter;
		#pragma endregion

		#pragma region "constructor function"
		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input, bool flag_debugInput, int idx_algorithmInput)
		{
			this->is_success = false;
			this->idx_algorithm = idx_algorithmInput;
			this->is_debugging = flag_debugInput;
			this->Image1D_original = Image1D_input;
			this->dim_X = int_xDimInput; this->dim_Y = int_yDimInput; this->dim_Z = int_zDimInput; this->idx_channel = int_channelInput;
			this->LandmarkList_exemplar = LandmarkList_input;
			this->size_page = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*size_page;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
			this->lowerBound_regionSize = const_min_regionSize; //prevent ultra-small exemplar region growing;
			this->upperBound_valueRatioChange = const_max_valueRatioChange_default;
			this->createNeighborVector();
			this->createSphereTemplate();
			this->Image1D_page = new unsigned char [this->size_page];
			this->Image1D_mask = new unsigned char [this->size_page];
			this->Image1D_exemplarRegion = new unsigned char [this->size_page];
			this->Image1D_segmentationResult =  new unsigned char [this->size_page];
			for (V3DLONG i=0;i<size_page;i++)
			{	
				this->Image1D_page[i] = Image1D_original[i+offset_channel];
				this->Image1D_mask[i] = const_max_voxeValue; //all available;
				this->Image1D_exemplarRegion[i] = 0;
				this->Image1D_segmentationResult[i] = 0;
			}
			ofstream ofstream_log;
			ofstream_log.open ("log_class_segmentationMain.txt");
			ofstream_log<<"dim_X:"<<this->dim_X<<", dim_Y:"<<this->dim_Y<<", dim_Z:"<<this->dim_Z<<";"<<endl;
			ofstream_log<<"size_page:"<<this->size_page<<";"<<endl;
			ofstream_log<<"current channel:"<<this->idx_channel<<";"<<endl;
			if (this->idx_algorithm == 1)
			{
				#pragma region "regionGrow only"
				this->estimateThreshold(); //store to this->threshold;
				this->count_totalWhiteVoxel = this->thresholdForCurrentPage();
				this->removeSingleVoxel(this->Image1D_page, this->size_page);
				this->categorizeVoxelsByValue(this->histo_main, this->Image1D_page, this->size_page); //store to this->vctList_potentialSeedList; providing seeds for regionGrowing;
				this->vctList_segmentationResult = this->regionGrowOnVectorList(this->vctList_potentialSeedList);
				this->is_success = true;
				this->visualizeRegionGrow();
				ofstream_log.close();
				#pragma endregion
			}
			else if (this->idx_algorithm == 2 || this->idx_algorithm == 4)
			{
				#pragma region "regionGrow+meanShift"
				this->vct_exemplar = landMarkList2IndexList(this->LandmarkList_exemplar);
				this->estimateThreshold(); //store to this->threshold;
				ofstream_log<<"estimateThreshold succeed, global threshold value: "<<this->threshold<<";"<<endl;
				this->count_totalWhiteVoxel = this->thresholdForCurrentPage();
				V3DLONG count_removedVoxel = this->removeSingleVoxel(this->Image1D_page, this->size_page);
				ofstream_log<<"removeSingleVoxel succeed, "<<count_removedVoxel<<" single voxel removed;"<<endl;
				ofstream_log<<"count_totalWhiteVoxel: "<<count_totalWhiteVoxel<<endl;
				this->categorizeVoxelsByValue(this->histo_main, this->Image1D_page, this->size_page); //store to this->vctList_potentialSeedList; providing seeds for regionGrowing;
				this->vctList_exemplarRegion = this->regionGrowOnVector(this->vct_exemplar);
				if (this->vctList_exemplarRegion.size()<1)
				{
					v3d_msg("Warning: no exemplar regions grown. Program will terminate, please re-select the exemplar(s)!");
					ofstream_log<<"regonGrow for exemplar failed!"<<endl; ofstream_log.close(); return;
				}
				this->vctList2Image1D(this->Image1D_exemplarRegion, this->vctList_exemplarRegion);
				if (!this->analyzeExemplarRegion())
				{
					ofstream_log<<"analyzeExemplarRegion failed!"<<endl; ofstream_log.close(); return; //failed;
				}
				ofstream_log<<"lowerBound_sphereToSizeRatio: "<<this->lowerBound_sphereToSizeRatio<<";"<<endl;
				ofstream_log<<"upperBound_valueRatioChange: "<<this->upperBound_valueRatioChange<<";"<<endl;
				ofstream_log<<"lowerBound_regionSize: "<<this->lowerBound_regionSize<<";"<<endl;
				for (int i=0;i<this->vctList_exemplarRegion.size();i++)
				{
					ofstream_log<<" exemplar region"<<(i+1)<<", voxel count: "<<this->vctList_exemplarRegion[i].size()<<endl;
					ofstream_log<<" exemplar region"<<(i+1)<<", sphere size: "<<this->vc_exemplarSphereSize[i]<<endl;
				}
				this->vctList_segmentationResult = this->regionGrowOnVectorList(this->vctList_potentialSeedList);
				this->vctList_segmentationResult = postAnalysis(this->vctList_segmentationResult);
				this->vctList_segmentationResult = mergeVectorList(this->vctList_segmentationResult, this->vctList_exemplarRegion);
				ofstream_log<<"segmentation succeed, totally "<<this->vctList_segmentationResult.size()<<" regions;"<<endl;
				this->visualizeRegionGrow();
				this->is_success = true;
				ofstream_log.close();
				#pragma endregion
			}
			else if (this->idx_algorithm == 3)
			{
				#pragma region "GVF only"
				double *** Image3D_page;
				Image3D_page = class_segmentationMain::memory_allocate_double3D(this->dim_X, this->dim_Y, this->dim_Z);
				Image1D2Image3D(this->Image1D_page, Image3D_page, this->dim_X, this->dim_Y, this->dim_Z);
				ofstream_log<<"GVF variable preparation succeed!"<<endl;
				this->vctList_segmentationResult = class_segmentationMain::GVF_cellSegmentation(Image3D_page, this->dim_X, this->dim_Y, this->dim_Z);
				ofstream_log<<"GVF result conversion succeed!"<<endl;
				this->is_success = true;
				ofstream_log.close();
				#pragma endregion
			}
			v3d_msg(QString("Segmentation succeed! Totally %1 segments!").arg(this->vctList_segmentationResult.size()));
			return;
		}
		#pragma endregion

		#pragma region "regionGrow"
		vector<vector<V3DLONG> > postAnalysis(vector<vector<V3DLONG> > vctList_regionInput)
		{
			ofstream ofstream_log;
			if (this->is_debugging) {ofstream_log.open ("log_postAnalysis.txt");}
			V3DLONG idx_region = 0;
			V3DLONG idx_page;
			V3DLONG idx_voxel = 0;
			V3DLONG count_region = vctList_regionInput.size();
			V3DLONG count_voxel = 0;
			vector<V3DLONG> vct_voxelTmp;
			vector<vector<V3DLONG> > vctList_regionResult;
			vector<V3DLONG> vct_boundBoxTmp;
			vector<vector<V3DLONG> > vctList_tmp;
			vector<vector<V3DLONG> > vctList_meanShift;
			vector<vector<V3DLONG> > vctList_GVF;
			vector<V3DLONG> vct_sphereGrowResult;
			vector<V3DLONG> vct_tmp;
			V3DLONG count_sphereVoxel = 0;
			V3DLONG idx_center;
			double*** double3D_ImageTmp;
			if (this->is_debugging) {ofstream_log<<"total regions: "<<count_region<<";"<<endl;}
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	
				vct_voxelTmp = vctList_regionInput[idx_region];
				count_voxel = vct_voxelTmp.size();
				if (count_voxel<const_min_regionSize)
				{
					break; //too small, removed;
				}
				if (this->is_debugging) {ofstream_log<<"idx_region: "<<idx_region<<", count_voxel: "<<count_voxel<<endl;}
				idx_center = this->getCenterByMedian(vct_voxelTmp);
				vct2Image1D(this->Image1D_mask, vct_voxelTmp, const_max_voxeValue); //reset mask;
				vct_sphereGrowResult = this->sphereGrowOnSeed(idx_center);
				count_sphereVoxel = vct_sphereGrowResult[0];
				vct2Image1D(this->Image1D_mask, vct_voxelTmp, 0); //set mask;
				if (((double)count_sphereVoxel/count_voxel)>this->lowerBound_sphereToSizeRatio)
				{
					vctList_regionResult.push_back(vct_voxelTmp);
					if (this->is_debugging) {ofstream_log<<"spheric test passed;"<<endl;}
				}
				else
				{
					if (this->is_debugging) {ofstream_log<<"spheric test failed;"<<endl;}
					if (this->idx_algorithm == 2)
					{
						vctList_meanShift = menashiftClustering(vct_voxelTmp, const_multiplier_bandWidthToDiagnal);
						for (int i=0;i<vctList_meanShift.size();i++)
						{
							vct_tmp = vctList_meanShift[i];
							if (vct_tmp.size()>=this->lowerBound_regionSize)
							{
								vctList_regionResult.push_back(vct_tmp);
							}
						}
					}
					else if (this->idx_algorithm == 4)
					{
						//vctList_meanShift = menashiftClustering(vct_voxelTmp, const_multiplier_bandWidthToDiagnal);
						vct_boundBoxTmp = this->getBoundBox(vct_voxelTmp);
						V3DLONG size_X = vct_boundBoxTmp[1]-vct_boundBoxTmp[0]+1;
						V3DLONG size_Y = vct_boundBoxTmp[3]-vct_boundBoxTmp[2]+1;
						V3DLONG size_Z = vct_boundBoxTmp[5]-vct_boundBoxTmp[4]+1;
						if (this->is_debugging) {ofstream_log<<"region as input to GVF: size_X("<<size_X<<"), size_Y("<<size_Y<<"), size_Z("<<size_Z<<");"<<endl;}
						//double3D_ImageTmp = memory_allocate_double3D(size_X+8, size_Y+8, size_Z+8);
						double3D_ImageTmp = memory_allocate_double3D(size_X, size_Y, size_Z);
						centralizeRegion(vct_voxelTmp, double3D_ImageTmp, vct_boundBoxTmp);
						vctList_GVF = class_segmentationMain::GVF_cellSegmentation(double3D_ImageTmp, size_X, size_Y, size_Z, this->is_debugging);
						if (this->is_debugging) {ofstream_log<<"GVF segmented the region into "<<vctList_GVF.size()<<" sub-regions;"<<endl;}
						for (int i=0;i<vctList_GVF.size();i++)
						{
							if (vctList_GVF[i].size()>this->lowerBound_regionSize)
							{
								if (this->is_debugging) {ofstream_log<<"sub-region ("<<i+1<<"), size: "<<vctList_GVF[i].size()<<endl;}
								vct_tmp = deCentralizeRegion(vctList_GVF[i], vct_boundBoxTmp, this->dim_X, this->dim_Y);
								vctList_regionResult.push_back(vct_tmp);
							}
						}
						if (this->is_debugging) {ofstream_log<<"sub-region(s) pushingback succeed;"<<endl;}
						memory_free_double3D(double3D_ImageTmp, size_Z, size_X);
					}
				}
			}
			return vctList_regionResult;
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

		vector<V3DLONG> regionGrowOnSeed(V3DLONG idx_seed)
			//assuming [0-const_max_voxeValue] integer Image1D_page, should also work on binary image (not tested yet);
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
							if ((fabs(double_neightborValue-double_currentValue))<(double_meanValue*this->upperBound_valueRatioChange))
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

		V3DLONG getLocalCenter(V3DLONG idx_input)
		{
			V3DLONG idx_result = idx_input;
			
			return idx_result;
		}

		vector<V3DLONG> sphereGrowOnSeed(V3DLONG idx_seed)
		{
			V3DLONG idx_radius = 0;
			vector<V3DLONG> vct_sphere;
			vector<V3DLONG> vct_result (2, 0);
			V3DLONG count_missed = 0;
			V3DLONG count_total = 0;
			double double_missedRatio = 0;
			ofstream ofstream_log;
			V3DLONG idx_tmp;
			if (!this->checkValidity(idx_seed)) //unlikely, but just in case;
			{
				return vct_result;
			}
			if (this->Image1D_mask[idx_seed]<1) //very likely;
			{
				return vct_result;
			}
			//grow a sphere around it;
			while (true)
			{
				if (idx_radius>=(const_max_preGenerateSphereRadius))
				{
					break;
				}
				vct_sphere = this->vctList_preGeneratedSphere[idx_radius]; //note it's starting from radius of 1;
				for (int i=0;i<vct_sphere.size();i++)
				{
					idx_tmp = idx_seed + vct_sphere[i];
					if (checkValidity(idx_tmp))
					{
						if (this->Image1D_mask[idx_tmp]<1)
						{
							count_missed++;
						}
						count_total++;
					}
				}
				double_missedRatio = double(count_missed)/(double)count_total;
				if (double_missedRatio>const_max_sphereGrowMissToTotalRatio)
				{
					break;
				}
				idx_radius++;
			}
			vct_result[0] = count_total;
			vct_result[1] = idx_radius;
			return vct_result;
		}

		void visualizeRegionGrow()
		{
			V3DLONG count_region = this->vctList_segmentationResult.size();
			V3DLONG count_voxel = 0;
			V3DLONG idx_tmp;
			V3DLONG idx_center;
			this->vct_segmentationResultCenter.clear();
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	 
				count_voxel = this->vctList_segmentationResult[idx_region].size();
				idx_center = this->getCenterByMedian(this->vctList_segmentationResult[idx_region]);
				this->vct_segmentationResultCenter.push_back(idx_center);
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = this->vctList_segmentationResult[idx_region][idx_voxel];
					if (this->checkValidity(idx_tmp))
					{
						this->Image1D_segmentationResult[idx_tmp] = (idx_region+1);
					}
				}
			}
			this->LandmarkList_segmentationResult = indexList2LandMarkList(this->vct_segmentationResultCenter);
		}
		
		#pragma endregion		

		#pragma region "exemplar" 
		bool analyzeExemplarRegion()
		{
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_tmp;
			V3DLONG count_region = this->vctList_exemplarRegion.size();
			V3DLONG count_voxel = 0;
			double mean_neighborValueChangeTmp = 0;
			V3DLONG idx_neighborTmp = 0;
			double value_tmp=0;
			this->upperBound_valueRatioChange = -INF;
			this->lowerBound_sphereToSizeRatio = INF;
			V3DLONG count_neighborTmp;
			double mean_regionValue;
			this->lowerBound_regionSize = INF;
			vector<V3DLONG> vct_sphereGrowResult (2, 0);
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				vct_tmp = this->vctList_exemplarRegion[idx_region];
				count_voxel = vct_tmp.size();
				if (count_voxel>(this->count_totalWhiteVoxel*const_max_regionSizeToPageSizeRatio))
				{
					v3d_msg("Warning: abnormal large exemplar region detected, program will terminate. Please re-select the exemplar(s)!");
					return false;
				}
				if (this->lowerBound_regionSize>count_voxel)
				{
					lowerBound_regionSize = count_voxel;
				}
				mean_regionValue=0;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vct_tmp[idx_voxel];
					value_tmp = this->Image1D_page[idx_tmp];
					mean_regionValue += value_tmp;
				}
				mean_regionValue = mean_regionValue/count_voxel;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vct_tmp[idx_voxel];
					value_tmp = this->Image1D_page[idx_tmp];
					mean_neighborValueChangeTmp = 0;
					count_neighborTmp = 0;
					for (int i=0;i<const_count_neighbors;i++)
					{
						idx_neighborTmp = idx_tmp + this->const_vctRelativeIndex_neighbors[i];
						if (checkValidity(idx_neighborTmp))
						{
							if (vctContains(vct_tmp, idx_neighborTmp)>-1)
							{
								mean_neighborValueChangeTmp += fabs(value_tmp-this->Image1D_page[idx_neighborTmp]);
								count_neighborTmp++;
							}
						}
					}
					if (count_neighborTmp>0)
					{
						mean_neighborValueChangeTmp = mean_neighborValueChangeTmp / count_neighborTmp;
						if (this->upperBound_valueRatioChange<(mean_neighborValueChangeTmp/mean_regionValue))
						{
							this->upperBound_valueRatioChange = mean_neighborValueChangeTmp/mean_regionValue;
						}
					}
				}
	
				//sphere property;
				V3DLONG idx_center = this->getCenterByMedian(vct_tmp);
				vct2Image1D(this->Image1D_mask, vct_tmp, const_max_voxeValue); //reset mask;
				vct_sphereGrowResult= this->sphereGrowOnSeed(idx_center);
				vct2Image1D(this->Image1D_mask, vct_tmp, 0); //set mask;
				V3DLONG count_total = vct_sphereGrowResult[0];
				this->vc_exemplarSphereSize.push_back(vct_sphereGrowResult[1]);
				if (this->lowerBound_sphereToSizeRatio>((double)count_total/count_voxel))
				{
					this->lowerBound_sphereToSizeRatio = ((double)count_total/count_voxel);
				}
			}
			lowerBound_regionSize = lowerBound_regionSize*const_multiplier_regionSizeLowerBound;
			return true;
		}
		#pragma endregion
		
		#pragma region "locate voxels in image with top voxel value (i.e. intensity)"
		void categorizeVoxelsByValue(vector<double> vct_histoInput, unsigned char* Image1D_input, V3DLONG count_imageSize)
		//will only consider voxels with value higher than threshold;
		{
			this->vctList_potentialSeedList.clear();
			vector<V3DLONG> vct_empty (0,0);
			int int_threshold = (int)this->threshold;
			for (int i=(int_threshold+1);i<const_length_histogram;i++)
			{
				this->vctList_potentialSeedList.push_back(vct_empty);
			}
			int int_valueOffset=0;
			int int_valueTmp=0;
			for (int i=0;i<count_imageSize;i++)
			{
				int_valueTmp = (int)Image1D_input[i];
				int_valueOffset = const_max_voxeValue-int_valueTmp;
				if (int_valueTmp>int_threshold)
				{
					vctList_potentialSeedList[int_valueOffset].push_back(i);
				}
			}
		}
		#pragma endregion
		
		#pragma region "threshold estimation"
		void estimateThreshold()
		{
			this->histo_main = class_segmentationMain::getHistogramFromImage1D(this->Image1D_page, this->size_page);

			this->threshold = class_segmentationMain::estimateThresholdYen(histo_main);
			if (this->threshold < const_threshold_default)
			{
				v3d_msg("Warning: threshold estimation failed, will use default threshold (15) instead!");
				this->threshold = const_threshold_default;
			}
		}

		static int estimateThresholdYen(double* histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;

			int threshold;
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
			threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < 256; it++ ) {
				crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
				if ( crit > max_crit ) {
					max_crit = crit;
					threshold = it;
				}
			}
			return threshold;
		}

		static int estimateThresholdYen(vector<double> histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;
			
			int threshold;
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
			threshold = -1;
			max_crit = NINF;
			for ( it = 0; it < 256; it++ ) {
				crit = -1.0 * (( P1_sq[it] * P2_sq[it] )> 0.0? log( P1_sq[it] * P2_sq[it]):0.0) +  2 * ( ( P1[it] * ( 1.0 - P1[it] ) )>0.0? log(  P1[it] * ( 1.0 - P1[it] ) ): 0.0);
				if ( crit > max_crit ) {
					max_crit = crit;
					threshold = it;
				}
			}
			return threshold;
		}

		static int estimateThresholdOtsu(double* histo_input)
		{
			int i, threshold;
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
					threshold = i;
				}
			}
			return threshold;
		}

		static vector<double> getHistogramFromImage1D(unsigned char* ImageID_input, V3DLONG count_inputImageVectorLength)
		{
			vector<double> vct_result (const_length_histogram, 0);
			V3DLONG int_valueTmp;
			for (V3DLONG i=0;i<count_inputImageVectorLength;i++)
			{
				int_valueTmp = (int)ImageID_input[i];
				if (int_valueTmp>0 && int_valueTmp<const_length_histogram)
				{
					vct_result[int_valueTmp] = vct_result[int_valueTmp]+1;
				}
			}
			return vct_result;
		}
		
		V3DLONG thresholdForCurrentPage()
		{
			V3DLONG count_totalWhite = 0;
			for(V3DLONG i=0; i<this->size_page; i++)
			{	
				if ((double)Image1D_page[i]>this->threshold)
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
		#pragma endregion

		#pragma region "sphere"
		void createSphereTemplate()
		{
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			vector<V3DLONG> vct_tmp;
			vector<V3DLONG> vct_result;
			double theta;
			double phi;
			V3DLONG count_loop;
			
			for (V3DLONG r=1; r<=const_max_preGenerateSphereRadius; r++)
			{
				count_loop = r*const_count_sphereSection;
				vct_tmp.clear();
				vct_result.clear();
				for (V3DLONG i=0; i<count_loop; i++)
				{
					theta = (2*PI*i)/count_loop;
					for (V3DLONG j=0; j<count_loop; j++)
					{
						phi = (PI*j)/count_loop;
						x = r*cos(theta)*sin(phi);
						y = r*sin(theta)*sin(phi);
						z = r*cos(phi);
						vct_tmp.push_back(this->coordinate2Index(x, y, z));
					}
				}
				sort (vct_tmp.begin(), vct_tmp.end()); //sort r-th sphere shell and remove duplicate voxels;
				for (int i=1;i<vct_tmp.size();i++)
				{
					if(vct_tmp[i]!=vct_tmp[i-1])
					{
						vct_result.push_back(vct_tmp[i]);
					}
				}
				this->vctList_preGeneratedSphere.push_back(vct_tmp);
			}
		}
		#pragma endregion

		#pragma region "utility functions"
		static vector<V3DLONG> getOffset(const int dim_X, const int dim_Y, const int dim_Z)
		{
			vector<V3DLONG> vct_result (2, 0);
			vct_result[0] = dim_X*dim_Y;
			vct_result[1] = dim_X;
			return vct_result;
		}

		static vector<vector<V3DLONG> > Image3D2vctList(int*** Image3D_input, const int size_X, const int size_Y, const int size_Z)
		{
			V3DLONG tmp_value = 0;
			V3DLONG tmp_idx = 0;
			vector<V3DLONG> vct_valuesUsed (0, 0);
			int idx_valuePosition;
			vector<vector<V3DLONG> > vctList_result;
			vector<V3DLONG> vct_empty(0, 0);
			int count_value = 0;
			for (int z=0;z<size_Z;z++)
			{
				for (int x=0;x<size_X;x++)
				{
					for (int y=0;y<size_Y;y++)
					{
						tmp_value = (int)Image3D_input[z][x][y];
						if (tmp_value>1)
						{
							idx_valuePosition = vctContains(vct_valuesUsed, tmp_value);
							tmp_idx = coordinate2Index(x, y, z, size_X, size_X*size_Y);
							if (idx_valuePosition<0)
							{
								vct_valuesUsed.push_back(tmp_value);
								vctList_result.push_back(vct_empty);
								vctList_result[count_value].push_back(tmp_idx);
								count_value++;
							}
							else
							{
								vctList_result[idx_valuePosition].push_back(tmp_idx);
							}
						}
					}
				}
			}
			return vctList_result;
		}

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

		V3DLONG getCenterByMean(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> vct_coordinate;
			V3DLONG idx_result;
			double xSum=0;
			double ySum=0;
			double zSum=0;
			V3DLONG count_voxel = vct_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				vct_coordinate = this->index2Coordinate(vct_input[i]);
				xSum += vct_coordinate[0];
				ySum += vct_coordinate[1];
				zSum += vct_coordinate[2];
			}
			idx_result = this->coordinate2Index((V3DLONG)xSum/count_voxel, (V3DLONG)ySum/count_voxel,(V3DLONG) zSum/count_voxel);
			return this->getNearestVoxel(vct_input, idx_result);
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

		V3DLONG getMassCenter(vector<V3DLONG> vct_input)
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

		vector<V3DLONG> getBoundBox(vector<V3DLONG> vct_input) //output: xMin, xMax, yMin, yMax, zMin, zMax;
		{
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG xMax = -INF;
			V3DLONG yMax = -INF;
			V3DLONG zMax = -INF;
			V3DLONG xMin = INF;
			V3DLONG yMin = INF;
			V3DLONG zMin = INF;
			V3DLONG count_voxel = vct_input.size();
			vector<V3DLONG> vct_coordinate (3, 0);
			vector<V3DLONG> vct_result (6, 0);
			V3DLONG idx_tmp;
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				idx_tmp = vct_input[idx_voxel];
				vct_coordinate = this->index2Coordinate(idx_tmp);
				x = vct_coordinate[0];
				y = vct_coordinate[1];
				z = vct_coordinate[2];
				if (x>xMax) {xMax=x;}
				if (y>yMax) {yMax=y;}
				if (z>zMax) {zMax=z;}
				if (x<xMin) {xMin=x;}
				if (y<yMin) {yMin=y;}
				if (z<zMin) {zMin=z;}
			}
			vct_result[0]=xMin;
			vct_result[1]=xMax;
			vct_result[2]=yMin;
			vct_result[3]=yMax;
			vct_result[4]=zMin;
			vct_result[5]=zMax;
			return vct_result;
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
				if (double_currentValue>this->threshold) //valid voxel;
				{
					flag_remove = true;
					for (int j=0;j<const_count_neighbors;j++)
					{
						idx_neighbor = i+const_vctRelativeIndex_neighbors[j];
						double_neightborValue = Image1D_input[idx_neighbor];
						if(this->checkValidity(idx_neighbor)) //prevent it from going out of bounds;
						{
							double_neightborValue = Image1D_input[idx_neighbor];
							if (double_neightborValue>this->threshold)
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

		void vctList2Image1D(unsigned char* Image1D_input, vector<vector<V3DLONG> > vctList_input)
		{
			memset(Image1D_input, 0, this->size_page);
			for (int i=0;i<vctList_input.size();i++)
			{
				for (int j=0;j<vctList_input[i].size();j++)
				{
					Image1D_input[vctList_input[i][j]] = this->Image1D_page[vctList_input[i][j]];
				}
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

		void centralizeRegion(const vector<V3DLONG> vct_input, double*** double3D_output, const vector<V3DLONG> vct_boundBox)
		{
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_coordinate (0, 0);
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG count_voxel = vct_input.size();
			/*V3DLONG min_X =  vct_boundBox[0] - 4;
			V3DLONG min_Y =  vct_boundBox[2] - 4;
			V3DLONG min_Z =  vct_boundBox[4] - 4;*/
			V3DLONG min_X = vct_boundBox[0];
			V3DLONG min_Y = vct_boundBox[2];
			V3DLONG min_Z = vct_boundBox[4];
			V3DLONG size_X = vct_boundBox[1] - vct_boundBox[0];
			V3DLONG size_Y = vct_boundBox[3] - vct_boundBox[2];
			V3DLONG size_Z = vct_boundBox[5] - vct_boundBox[4];
			for (z=0;z<size_Z;z++)
			{
				for (x=0;x<size_X;x++)
				{
					for (y=0;y<size_Y;y++)
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
		#pragma endregion

		#pragma region "meanshiftClustering"
		vector<vector<V3DLONG> > menashiftClustering(const vector<V3DLONG> vct_input, const double double_bandwidthRatio)
		{
			ofstream ofstream_log;
			if (this->is_debugging) {ofstream_log.open ("log_meanshiftClustering.txt");}
			V3DLONG count_voxel = vct_input.size();
			vector<V3DLONG> vct_boundingBox = this->getBoundBox(vct_input);
			double double_boxDiagonalLength = pow(double(vct_boundingBox[1]-vct_boundingBox[0])/2, 2)+pow(double(vct_boundingBox[3]-vct_boundingBox[2])/2, 2)+pow(double(vct_boundingBox[5]-vct_boundingBox[4])/2, 2);
			double_boxDiagonalLength = sqrt(double_boxDiagonalLength);
			double double_bandwidth = double_bandwidthRatio*double_boxDiagonalLength;
			if (this->is_debugging) {ofstream_log<<"total voxel:"<<count_voxel<<endl;}
			if (this->is_debugging) {ofstream_log<<"boxDiagonalLength:"<<double_boxDiagonalLength<<endl;}
			double double_stopThresh = 1e-3*double_bandwidth; //convergence;
			V3DLONG idx_mean;
			vector<V3DLONG> vct_xyz1;
			vector<V3DLONG> vct_xyz2;
			V3DLONG idx_meanOld;
			vector<double> vct_distTmp (count_voxel, 0);
			vector<vector<V3DLONG> > vctList_cluster;
			vector<vector<V3DLONG> > vctList_result;
			vector<V3DLONG> vct_tmp;
			vector<bool> vct_visited (count_voxel, false);
			vector<double> vct_voteTmp (count_voxel, 0);
			vector<vector<double> > vctList_vote;
			vector<V3DLONG> vct_seed = vct_input;
			vector<V3DLONG> vct_clusterCenter;
			vector<V3DLONG> vct_empty(0, 0);
			V3DLONG idx_merge = 0;
			V3DLONG count_cluster = 0;
			V3DLONG idx_rand = 0;
			double double_volumeTmp1 = 0;
			double double_volumeTmp2 = 0;
			if (this->is_debugging) {ofstream_log<<"initialization passed!"<<endl;}
			while (!vct_seed.empty())
			{
				if (this->is_debugging) {ofstream_log<<"calculation started!"<<endl;}
				idx_mean = vct_seed[0];
				if (this->is_debugging) {ofstream_log<<"idx_mean:"<<idx_mean<<endl;}
				if (this->is_debugging) {ofstream_log<<"fill passed!"<<endl;}
				fill(vct_voteTmp.begin(), vct_voteTmp.end(), 0);
				vct_tmp.clear();
				while (true)
				{
					for (V3DLONG i=0;i<count_voxel;i++)
					{
						vct_distTmp[i] = this->getEuclideanDistance(idx_mean, vct_input[i]);
					}
					if (this->is_debugging) {ofstream_log<<"getEuclideanDistance passed!"<<endl;}
					for (V3DLONG i=0;i<count_voxel;i++)
					{
						if (vct_distTmp[i]<double_bandwidth)
						{
							if (vct_voteTmp[i]<1)
							{vct_tmp.push_back(vct_input[i]);}
							vct_visited[i] = true;
							vct_voteTmp[i] = vct_voteTmp[i] + (double_boxDiagonalLength-vct_distTmp[i])/double_boxDiagonalLength;
						}
					}
					if (vct_tmp.empty())
					{
						break;
					}
					if (this->is_debugging) {ofstream_log<<"vct_tmp.size():"<<vct_tmp.size()<<endl;}
					idx_meanOld = idx_mean;
					idx_mean = this->getCenterByMean(vct_tmp);
					if (this->is_debugging) {ofstream_log<<"idx_mean(new):"<<idx_mean<<endl;}
					if (this->getEuclideanDistance(idx_mean, idx_meanOld)<double_stopThresh)
					{
						if (this->is_debugging) {ofstream_log<<"converged!"<<endl;}
						//possible merge;
						idx_merge = -1;
						for (int j=0;j<count_cluster;j++)
						{
							if (this->getEuclideanDistance(vct_clusterCenter[j], idx_mean)<(double_bandwidth/2))
							{
								idx_merge = j;
								break;
							}
						}
						if (idx_merge > 0)
						{
							if (this->is_debugging) {ofstream_log<<"merging start!"<<endl;}
							for (int k=0;k<vct_tmp.size();k++)
							{
								if (vctContains(vctList_cluster[idx_merge], vct_tmp[k])<0)
								{
									vctList_cluster[idx_merge].push_back(vct_tmp[k]);
								}
							}
							vct_clusterCenter[idx_merge] = this->getCenterByMean(vctList_cluster[idx_merge]) ;
							for (int k=0;k<count_voxel;k++)
							{
								vctList_vote[idx_merge][k] = vctList_vote[idx_merge][k] + vct_voteTmp[k];
							}
							if (this->is_debugging) {ofstream_log<<"merging succeed!"<<endl;}
						}
						else
						{
							if (this->is_debugging) {ofstream_log<<"creating start!"<<endl;}
							vctList_cluster.push_back(vct_tmp);
							vct_clusterCenter.push_back(idx_mean);
							vctList_vote.push_back(vct_voteTmp);
							count_cluster++;
							if (this->is_debugging) {ofstream_log<<"creating succeed"<<endl;}
						}
						break;
					}
				}
				vct_seed.clear();
				for (int i=0;i<count_voxel;i++)
				{
					if (!vct_visited[i])
					{
						vct_seed.push_back(vct_input[i]);
					}
				}
				if (this->is_debugging) {ofstream_log<<"vct_seed.size():"<<vct_seed.size()<<endl;}
			}
			//final merging;
			for (int i=0;i<count_cluster;i++)
			{
				for (int j=0;j<count_cluster;j++)
				{
					if (i!=j)
					{
						if (this->getEuclideanDistance(vct_clusterCenter[i], vct_clusterCenter[j])<(double_bandwidth/4))
						{
							//merge them;
							for (int k=0;k<vctList_cluster[j].size();k++)
							{
								if (vctContains(vctList_cluster[i], vctList_cluster[j][k])<0)
								{
									vctList_cluster[i].push_back(vctList_cluster[j][k]);
								}
							}
							vct_clusterCenter[i] = this->getCenterByMean(vctList_cluster[i]) ;
							for (int k=0;k<count_voxel;k++)
							{
								vctList_vote[i][k] = vctList_vote[i][k] + vctList_vote[j][k];
							}
							//destroy j-th cluster;
							vct_clusterCenter[j] = INF;
							fill(vctList_vote[j].begin(),vctList_vote[j].end(),0);
						}
					}

				}
			}
			for (int i=0;i<count_cluster;i++)
			{
				vctList_result.push_back(vct_empty);
			}
			int max_vote;
			int idx_vote;
			if (this->is_debugging) {ofstream_log<<"summary vote start!"<<endl;}
			for (int i=0;i<count_voxel;i++)
			{
				max_vote = -INF;
				for (int j=0;j<count_cluster;j++)
				{
					if (vctList_vote[j][i]>max_vote)
					{
						max_vote = vctList_vote[j][i];
						idx_vote = j;
					}
				}
				vctList_result[idx_vote].push_back(vct_input[i]);
			}
			if (this->is_debugging) {ofstream_log<<"succeed!"<<endl;}
			return vctList_result;
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
		#pragma endregion

		#pragma region "GVF"
		vector<vector<V3DLONG> >  GVF_cellSegmentation(double *** Image3D_input, const int dim_X, const int dim_Y, const int dim_Z, bool is_debugging)
		{
			ofstream ofstream_log;
			if (is_debugging) {ofstream_log.open ("log_GVF.txt");}
			int para_fusionThreshold = const_threshold_GVF_fution;
			double para_sigma = const_threshold_GVF_sigma;
			double para_lambda = const_threshold_GVF_lambda;
			double para_mu=const_threshold_GVF_mu;
			double para_omega=const_threshold_GVF_omega;
			double3D ***Image3D3_u;
			double3D ***Image3D3_u_normalized;
			double3D ***Image3D3_f;
			double3D ***Image3D3_gradient;
			//int *** Image3D_result;
			//Image3D_result = memory_allocate_int3D(dim_X,dim_Y,dim_Z);
			Image3D3_gradient = memory_allocate_double3D3(dim_X, dim_Y, dim_Z);
			int3D ***Image3D3_mode;
			int ***Image3D_label;
			int count_page = dim_X*dim_Y*dim_Z;
			//unsigned char ***Image3D_edge;
			int smoothIteration = (int)(3*para_omega + 0.5);
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
			GVF_warp(const_max_GVF_iteration, para_mu, Image3D3_u, Image3D3_f, dim_X, dim_Y, dim_Z);
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
			//Image3D_edge = memory_allocate_uchar3D(dim_X,dim_Y,dim_Z);
			//GVF_majorFilter(Image3D_label, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"major fitler1 passed!"<<endl;}
			//GVF_findEdge(Image3D_label, Image3D_edge, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"find edge passed!"<<endl;}
			GVF_localThresholding(Image3D_input, Image3D_label, dim_X, dim_Y, dim_Z);
			if (is_debugging) {ofstream_log<<"local thresholding passed!"<<endl;}
			//GVF_majorFilter(Image3D_result, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//if (is_debugging) {ofstream_log<<"major fitler2 passed!"<<endl;}
			vector<vector<V3DLONG> > vctList_result = Image3D2vctList(Image3D_label, dim_X, dim_Y, dim_Z);
			memory_free_int3D(Image3D_label,dim_Z,dim_X);
			if (is_debugging) {ofstream_log<<"all succeed!"<<endl;}
			return vctList_result;
		}

		vector<vector<V3DLONG> >  GVF_cellSegmentation(double *** Image3D_input, const int dim_X, const int dim_Y, const int dim_Z)
		{
			ofstream ofstream_log;
			ofstream_log.open ("log_GVF.txt");
			int para_fusionThreshold = 3;
			double para_sigma = 0.1;
			double para_lambda = 0.2;
			double para_mu=0.1;
			double para_omega=1.0;
			double para_globalCoefficient = 0.9;
			double3D ***Image3D3_u;
			double3D ***Image3D3_u_normalized;
			double3D ***Image3D3_f;
			double3D ***Image3D3_gradient;
			int *** Image3D_result;
			Image3D_result = memory_allocate_int3D(dim_X,dim_Y,dim_Z);
			Image3D3_gradient = memory_allocate_double3D3(dim_X, dim_Y, dim_Z);
			int3D ***Image3D3_mode;
			int ***Image3D_visited;
			int count_page = dim_X*dim_Y*dim_Z;
			unsigned char ***Image3D_edge;
			int smoothIteration = (int)(3*para_omega + 0.5);
			ofstream_log<<"initialization passed!"<<endl;
			smooth_GVFkernal(Image3D_input, smoothIteration, dim_X, dim_Y, dim_Z);
			ofstream_log<<"smoothing passed!"<<endl;
			GVF_getGradient(Image3D_input, Image3D3_gradient, dim_X, dim_Y, dim_Z);
			ofstream_log<<"get gradient passed!"<<endl;
			Image3D3_f = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			Image3D3_u = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			GVF_initialize(Image3D3_gradient, Image3D3_f, Image3D3_u, dim_X, dim_Y, dim_Z);
			ofstream_log<<"f/u initialization passed!"<<endl;
			memory_free_double3D3(Image3D3_gradient,dim_Z,dim_X);
			GVF_warp(const_max_GVF_iteration, para_mu, Image3D3_u, Image3D3_f, dim_X, dim_Y, dim_Z);
			ofstream_log<<"warp passed!"<<endl;
			memory_free_double3D3(Image3D3_f,dim_Z,dim_X);
			Image3D3_u_normalized = memory_allocate_double3D3(dim_X,dim_Y,dim_Z);
			GVF_normalize(Image3D3_u, Image3D3_u_normalized, dim_X, dim_Y, dim_Z);
			ofstream_log<<"normalization passed!"<<endl;
			memory_free_double3D3(Image3D3_u,dim_Z,dim_X);
			Image3D3_mode = memory_allocate_int3D3(dim_X,dim_Y,dim_Z);
			GVF_findMode(Image3D3_u_normalized, Image3D3_mode, dim_X, dim_Y, dim_Z);
			ofstream_log<<"findMode passed!"<<endl;
			memory_free_double3D3(Image3D3_u_normalized, dim_Z, dim_X);
			Image3D_visited = memory_allocate_int3D(dim_X, dim_Y, dim_Z);
			GVF_fuseMode( Image3D3_mode, Image3D_visited, para_fusionThreshold, dim_X, dim_Y, dim_Z);
			ofstream_log<<"fuseMode passed!"<<endl;
			memory_free_int3D3(Image3D3_mode,dim_Z,dim_X);
			Image3D_edge = memory_allocate_uchar3D(dim_X,dim_Y,dim_Z);
			//GVF_majorFilter(Image3D_visited, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//ofstream_log<<"major fitler1 passed!"<<endl;
			//GVF_findEdge(Image3D_visited, Image3D_edge, dim_X, dim_Y, dim_Z);
			//ofstream_log<<"find edge passed!"<<endl;
			//GVF_localThresholding(Image3D_input, Image3D_visited, Image3D_result, dim_X, dim_Y, dim_Z);
			//ofstream_log<<"local thresholding passed!"<<endl;
			//GVF_majorFilter(Image3D_result, const_int_GVF_majorFilterIteration, dim_X, dim_Y, dim_Z);
			//ofstream_log<<"major fitler2 passed!"<<endl;
			vector<vector<V3DLONG> > vctList_result = Image3D2vctList(Image3D_result, dim_X, dim_Y, dim_Z);
			memory_free_int3D(Image3D_visited,dim_Z,dim_X);
			ofstream_log<<"all succeed!"<<endl;
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
			double threshold = 0.3, threshold2 = 0.1;
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
								u[nextz][nextx][nexty].z*u[nnextz][nnextx][nnexty].z>=threshold \
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
			int x, y, z;
			int idx_label, i;
			int threshold_global = 0;
			double *vct_histogram;
			vct_histogram = (double *)malloc(sizeof(double)*256);
			for(i=0;i<256;i++)
			{
				vct_histogram[i] = 0;
			}
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						if(Image3D_label[z][x][y]>0)
						{
							vct_histogram[(int)Image3D_input[z][x][y]]++;
						}
					}
				}
			}
			threshold_global = class_segmentationMain::estimateThresholdYen(vct_histogram);
			for(z=0;z<dim_Z;z++)
			{
				for(x=0;x<dim_X;x++)
				{
					for(y=0;y<dim_Y;y++)
					{
						if((Image3D_label[z][x][y]>=0) && (Image3D_input[z][x][y]>=threshold_global))
						{
							Image3D_label[z][x][y] =  Image3D_label[z][x][y]+1;
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
		#pragma endregion
		//class defination ended;
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
    if (dialogMain1.threshold>const_max_voxeValue)
	{
		v3d_msg("Please provide a valid threshold, program canceled!"); 
		return false;
	}
	class_segmentationMain segmentationMain1(dialogMain1.threshold, Image1D_current, dim_X, dim_Y, dim_Z, idx_channel, LandmarkList_current, dialogMain1.flag_debug, dialogMain1.idx_algorithm);
	if (!segmentationMain1.is_success)
	{
		v3d_msg("warning:initialization of segmentationMain failed, program will terminate!");
		return false;
	}
	//visualizationImage1D(segmentationMain1.Image1D_exemplarRegion,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Exemplar Regions");
	visualizationImage1D(segmentationMain1.Image1D_segmentationResult,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result");
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