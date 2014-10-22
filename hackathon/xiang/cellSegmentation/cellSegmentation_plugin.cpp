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
const bool const_flag_debugMode = false;
const int const_length_histogram = 256;
const double const_max_voxeValue = 255;
const int const_count_neighbors = 26; //27 directions -1;
const double const_threshold_default = 15; //a small enough value for the last resort;
const double const_max_valueRatioChange_default = 0.5; //how much the value can change (relative to the mean value of current region) between current voxel and its neighbor, during seed-based region growing;
const double const_max_regionDimensionToPageDimension = 0.5; //if a region with any its dimension larger than this #% of the image, an error would be given;
const double const_multiplier_regionVolumeLowerBound = 0.001;  //it would be a user-specified value in the future !!!;
const double const_multiplier_earlyStop = 0.95;  //it would be a user-specified value in the future !!!;
const int const_min_regionSize = 8; //2*2*2, when a region is considered as non-acceptable small;
const int const_count_sphereSection = 12;
const int const_max_preGenerateSphereRadius = 30;
const double const_multiplier_bandWidthToDiagnal = 0.8;
const double const_max_sphereGrowMissToTotalRatio = 0.01;
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
	public:
		//constant
		vector<V3DLONG> const_vctRelativeIndex_neighbors;
		
		//Input or directly derived;
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
		vector<int> histo_main;
		bool is_success;
		V3DLONG count_totalWhiteVoxel;
		
		//Exemplar (or learn from it);
		LandmarkList LandmarkList_exemplar;
		vector<V3DLONG> vct_exemplar;
		unsigned char *Image1D_exemplarRegion;
		vector<vector<V3DLONG> > vctList_exemplarRegion;
		double max_exemplarRegionVolume;
		double min_exemplarRegionVolume;
		vector<vector<V3DLONG> > vctList_exemplarRegionBoudingBox;
		double upperBound_valueRatioChange;
		double upperBound_sphereToRegionVolumeRatio;
		
		//regonGrow;
		double lowerBound_regionVolume;
		vector<vector<V3DLONG> > vctList_segmentationResult;
		vector<vector<V3DLONG> > vctList_potentialSeedList;
		vector<vector<V3DLONG> > vctList_preGeneratedSphere;
		unsigned char* Image1D_segmentationResult;
		LandmarkList LandmarkList_segmentationResult;
		vector<V3DLONG> vct_segmentationResultCenter;

		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input, bool flag_debugInput)
		{
			this->is_success = false;
			this->is_debugging = flag_debugInput;
			this->Image1D_original = Image1D_input;
			this->dim_X = int_xDimInput; this->dim_Y = int_yDimInput; this->dim_Z = int_zDimInput; this->idx_channel = int_channelInput;
			this->LandmarkList_exemplar = LandmarkList_input;
			this->size_page = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*size_page;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
			this->lowerBound_regionVolume = const_min_regionSize; //prevent ultra-small exemplar region growing;
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
			this->vct_exemplar = landMarkList2IndexList(this->LandmarkList_exemplar);
			ofstream_log<<"estimateThreshold started!"<<endl;
			this->estimateThreshold(); //store to this->threshold;
			ofstream_log<<"estimateThreshold succeed!"<<endl;
			ofstream_log<<"thresholdForCurrentPage started!"<<endl;
			this->count_totalWhiteVoxel = this->thresholdForCurrentPage();
			ofstream_log<<"thresholdForCurrentPage succeed!"<<endl;
			ofstream_log<<"removeSingleVoxel started!"<<endl;
			this->removeSingleVoxel(this->Image1D_page, this->size_page);
			ofstream_log<<"removeSingleVoxel succeed!"<<endl;
			ofstream_log<<"categorizeVoxelsByValue started!"<<endl;
			this->categorizeVoxelsByValue(this->histo_main, this->Image1D_page, this->size_page); //store to this->vctList_potentialSeedList; providing seeds for regionGrowing;
			ofstream_log<<"categorizeVoxelsByValue succeed!"<<endl;
			ofstream_log<<"regonGrow for exemplar started!"<<endl;
			this->vctList_exemplarRegion = this->regionGrowOnVector(this->vct_exemplar);
			if (this->vctList_exemplarRegion.size()<1)
			{
				v3d_msg("Warning: no exemplar regions grown. Program will terminate, please re-select the exemplar(s)!");
				ofstream_log<<"regonGrow for exemplar failed!"<<endl; ofstream_log.close();
				return;
			}
			vctList2Image1D(this->Image1D_exemplarRegion, this->vctList_exemplarRegion);
			ofstream_log<<"regonGrow for exemplar succeed!"<<endl;
			ofstream_log<<"analyzeExemplarRegion started!"<<endl;
			if (!this->analyzeExemplarRegion())
			{
				if (this->is_debugging) {ofstream_log<<"analyzeExemplarRegion failed!"<<endl; ofstream_log.close();}
				return; //failed;
			}
			ofstream_log<<"analyzeExemplarRegion succeed!"<<endl;
			ofstream_log<<"regonGrow for whole image started!"<<endl;
			this->vctList_segmentationResult = this->regionGrowOnVectorList(this->vctList_potentialSeedList);
			ofstream_log<<"regonGrow for whole image succeed!"<<endl;
			ofstream_log<<"postAnalysis for whole image started!"<<endl;
			this->vctList_segmentationResult = postAnalysis(this->vctList_segmentationResult);
			ofstream_log<<"postAnalysis for whole image succeed!"<<endl;
			ofstream_log<<"merge results started!"<<endl;
			this->vctList_segmentationResult = mergeVectorList(this->vctList_segmentationResult, this->vctList_exemplarRegion);
			ofstream_log<<"merge results succeed!"<<endl;
			this->visualizeRegionGrow();
			this->is_success = true;
			ofstream_log<<"all succeed!"<<endl; ofstream_log.close();
			return;
		}

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
			vector<V3DLONG> vct_clusterCenterTmp;
			V3DLONG idx_clusterCenterTmp;
			V3DLONG count_clusterCenterTmp;
			vector<V3DLONG> vct_growSphereTmp;
			V3DLONG idx_center;
			double double_volume;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{	
				vct_voxelTmp = vctList_regionInput[idx_region];
				vct_boundBoxTmp = this->getBoundBox(vct_voxelTmp);
				double_volume = (vct_boundBoxTmp[1]-vct_boundBoxTmp[0])*(vct_boundBoxTmp[3]-vct_boundBoxTmp[2])*(vct_boundBoxTmp[5]-vct_boundBoxTmp[4]);
				if (double_volume<this->lowerBound_regionVolume)
				{
					//too small, removed;
					break;
				}
				count_voxel = vct_voxelTmp.size();
				if (this->is_debugging) {ofstream_log<<"count_voxel: "<<count_voxel<<endl;}
				idx_center = this->getMassCenter(vct_voxelTmp);
				vct_growSphereTmp = this->sphereGrowOnSeed(idx_center);
				if (((double)vct_growSphereTmp.size()/double_volume)>this->upperBound_sphereToRegionVolumeRatio)
				{
					//passed, need not further segmentation;
					vctList_regionResult.push_back(vct_voxelTmp);
					this->vct_segmentationResultCenter.push_back(idx_center);
				}
				else
				{
					vct_clusterCenterTmp = menashiftClustering(vct_voxelTmp, const_multiplier_bandWidthToDiagnal);
					count_clusterCenterTmp = vct_clusterCenterTmp.size();
					if (this->is_debugging) {ofstream_log<<"count_clusterCenterTmp:"<<count_clusterCenterTmp<<endl;}
					if (count_clusterCenterTmp<1)
					{
						//error?
						vctList_regionResult.push_back(vct_growSphereTmp);
						this->vct_segmentationResultCenter.push_back(idx_center);
					}
					else
					{
						//reset mask;
						vct2Image1D(this->Image1D_mask, vct_voxelTmp, const_max_voxeValue);
						for (int i=0;i<count_clusterCenterTmp;i++)
						{
							idx_clusterCenterTmp = vct_clusterCenterTmp[i];
							vct_growSphereTmp = this->sphereGrowOnSeed(idx_clusterCenterTmp);
							if (!vct_growSphereTmp.empty())
							{
								vctList_regionResult.push_back(vct_growSphereTmp);
								this->vct_segmentationResultCenter.push_back(idx_clusterCenterTmp);
							}
						}
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
				if (count_totalCoveredVoxel>this->count_totalWhiteVoxel*const_multiplier_earlyStop)
				{
					break;
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
							if ((abs(double_neightborValue-double_currentValue))<(double_meanValue*this->upperBound_valueRatioChange))
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

		vector<vector<V3DLONG> > sphereGrowOnVectorList(vector<vector<V3DLONG> > vctList_seed)
		{
			V3DLONG idx_tmp;
			vector<vector<V3DLONG> > vctList_result;
			vector<vector<V3DLONG> > vctList_tmp;
			V3DLONG count_seedList = vctList_seed.size();
			vector<V3DLONG> vct_tmp;
			V3DLONG count_totalCoveredVoxel = 0;
			V3DLONG count_seed = 0;
			for (V3DLONG idx_seedList=0;idx_seedList<count_seedList;idx_seedList++)
			{
				vctList_tmp = this->sphereGrowOnVector(vctList_seed[idx_seedList]);
				for (int i=0;i<vctList_tmp.size();i++)
				{
					vct_tmp = vctList_tmp[i];
					if (!vct_tmp.empty())	{vctList_result.push_back(vct_tmp);}
					count_totalCoveredVoxel += vct_tmp.size();
				}
				if (count_totalCoveredVoxel>this->count_totalWhiteVoxel*const_multiplier_earlyStop)
				{
					break;
				}
			}
			return vctList_result;
		}

		vector<vector<V3DLONG> > sphereGrowOnVector(vector<V3DLONG> vct_seed)
		{
			V3DLONG idx_tmp;
			vector<vector<V3DLONG> > vctList_result;
			vector<V3DLONG> vct_tmp;
			V3DLONG count_totalCoveredVoxel = 0;
			V3DLONG count_seed = vct_seed.size();
			for (V3DLONG idx_seed=0;idx_seed<count_seed;idx_seed++)
			{
				idx_tmp = vct_seed[idx_seed];
				//locate a better position for idx_tmp;
				//idx_tmp = getLocalCenter(idx_tmp);
				if (checkValidity(idx_tmp))
				{
					if (this->Image1D_mask[idx_tmp]>0)
					{
						vct_tmp = this->sphereGrowOnSeed(idx_tmp); //Image1D_mask is updating in this function;
						if (!vct_tmp.empty())	{vctList_result.push_back(vct_tmp);}
						count_totalCoveredVoxel += vct_tmp.size();
					}
				}
			}
			return vctList_result;
		}

		V3DLONG getLocalCenter(V3DLONG idx_input)
		{
			V3DLONG idx_result = idx_input;
			
			return idx_result;
		}

		vector<V3DLONG> sphereGrowOnSeed(V3DLONG idx_seed)
		{
			ofstream ofstream_log;
			if (this->is_debugging) {ofstream_log.open ("log_sphereGrowOnSeed.txt");}
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_empty (0, 0);
			V3DLONG idx_tmp;
			if (!this->checkValidity(idx_seed)) //unlikely, but just in case;
			{
				return vct_empty;
			}
			if (this->Image1D_mask[idx_seed]<1) //very likely;
			{
				return vct_empty;
			}
			//grow a sphere around it;
			V3DLONG idx_radius = 0;
			vector<V3DLONG> vct_sphere;
			V3DLONG count_missed = 0;
			V3DLONG count_total = 0;
			double double_missedRatio = 0;
			while (true)
			{
				count_missed = 0;
				count_total = 0;
				if (idx_radius>(const_max_preGenerateSphereRadius))
				{
					break;
				}
				vct_sphere = this->vctList_preGeneratedSphere[idx_radius];
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
					double_missedRatio = double(count_missed)/(double)count_total;
				}
				if (double_missedRatio>const_max_sphereGrowMissToTotalRatio)
				{
					break;
				}
				idx_radius++;
			}
			if (this->is_debugging) {ofstream_log<<"idx_radius: "<<idx_radius<<endl;}
			vct_sphere = this->vctList_preGeneratedSphere[idx_radius];
			if (this->is_debugging) {ofstream_log<<"vct_sphere.size(): "<<vct_sphere.size()<<endl;}
			for (int i=0;i<vct_sphere.size();i++)
			{
				idx_tmp = idx_seed + vct_sphere[i];
				if (checkValidity(idx_tmp))
				{
					if (this->Image1D_mask[idx_tmp]>0)
					{
						vct_result.push_back(idx_tmp);
						this->Image1D_mask[idx_tmp]=0; //scooped
					}
				}
			}
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
				idx_center = this->getCenter(this->vctList_segmentationResult[idx_region]);
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
			this->max_exemplarRegionVolume = -INF;
			this->min_exemplarRegionVolume = INF;
			this->vctList_exemplarRegionBoudingBox.clear();
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG xMax = -INF;
			V3DLONG yMax = -INF;
			V3DLONG zMax = -INF;
			V3DLONG xMin = INF;
			V3DLONG yMin = INF;
			V3DLONG zMin = INF;
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_xyz;
			vector<V3DLONG> vct_boundingBox (6, 0);
			vector<V3DLONG> vct_tmp;
			double double_volumeTmp = 0; 
			V3DLONG count_region = this->vctList_exemplarRegion.size();
			V3DLONG count_voxel = 0;
			double mean_neighborValueChangeTmp = 0;
			V3DLONG idx_neighborTmp = 0;
			double value_tmp=0;
			this->upperBound_valueRatioChange = -INF;
			this->upperBound_sphereToRegionVolumeRatio = INF;
			V3DLONG count_neighborTmp;
			double mean_regionValue;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				xMax = -INF;
				yMax = -INF;
				zMax = -INF;
				xMin = INF;
				yMin = INF;
				zMin = INF;
				vct_tmp = this->vctList_exemplarRegion[idx_region];
				count_voxel = vct_tmp.size();
				mean_regionValue=0;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vct_tmp[idx_voxel];
					value_tmp = this->Image1D_page[idx_tmp];
					mean_regionValue += value_tmp;
					vct_xyz = this->index2Coordinate(idx_tmp);
					x = vct_xyz[0];
					y = vct_xyz[1];
					z = vct_xyz[2];
					if (x>xMax) {xMax=x;}
					if (y>yMax) {yMax=y;}
					if (z>zMax) {zMax=z;}
					if (x<xMin) {xMin=x;}
					if (y<yMin) {yMin=y;}
					if (z<zMin) {zMin=z;}
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
								mean_neighborValueChangeTmp += abs(value_tmp-this->Image1D_page[idx_neighborTmp]);
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
				if (((xMax-xMin)>const_max_regionDimensionToPageDimension*this->dim_X) || ((yMax-yMin)>const_max_regionDimensionToPageDimension*this->dim_Y) || ((zMax-zMin)>const_max_regionDimensionToPageDimension*this->dim_Z))
				{
					v3d_msg("Warning: abnormal large exemplar region detected, program will terminate. Please re-select the exemplar(s)!");
					return false;
				}
				vct_boundingBox[0]=xMin;
				vct_boundingBox[1]=xMax;
				vct_boundingBox[2]=yMin;
				vct_boundingBox[3]=yMax;
				vct_boundingBox[4]=zMin;
				vct_boundingBox[5]=zMax;
				this->vctList_exemplarRegionBoudingBox.push_back(vct_boundingBox);
				double_volumeTmp = (xMax-xMin)*(yMax-yMin)*(zMax-zMin);
				if (double_volumeTmp>this->max_exemplarRegionVolume) {this->max_exemplarRegionVolume=double_volumeTmp;}
				if (double_volumeTmp<this->min_exemplarRegionVolume) {this->min_exemplarRegionVolume=double_volumeTmp;}
				
				//sphere property;
				V3DLONG idx_center = this->getCenter(vct_tmp);
				V3DLONG idx_radius = 0;
				V3DLONG count_missed = 0;
				V3DLONG count_total = 0;
				V3DLONG count_hit = 0;
				double double_missedRatio = 0;
				V3DLONG idx_position = 0;
				vector<bool> flag_visited (count_voxel, false);
				vector<V3DLONG> vct_sphere;
				while (true)
				{
					count_missed = 0;
					count_total = 0;
					count_hit = 0;
					vct_sphere = this->vctList_preGeneratedSphere[idx_radius];
					fill(flag_visited.begin(), flag_visited.end(), false);
					for (int i=0;i<vct_sphere.size();i++)
					{
						idx_tmp = idx_center + vct_sphere[i];
						if (checkValidity(idx_tmp))
						{
							idx_position = this->vctContains(vct_tmp, idx_tmp);
							if (idx_position>-1)
							{
								if (!flag_visited[idx_position])
								{
									flag_visited[idx_position]=true;
									count_hit++;
								}
							}
							else
							{
								count_missed++;
							}
							count_total++;
						}
						double_missedRatio = double(count_missed)/(double)count_total;
					}
					if ((count_hit >= count_voxel)||(double_missedRatio>const_max_sphereGrowMissToTotalRatio))
					{
						break;
					}
					idx_radius++;
					if (idx_radius>=const_max_preGenerateSphereRadius) break;
				}
				//this->double_sphereGrowingTolerateLevel += double_missedRatio;
				vct_sphere = this->vctList_preGeneratedSphere[idx_radius];
				if (this->upperBound_sphereToRegionVolumeRatio>((double)vct_sphere.size()/double_volumeTmp))
				{
					this->upperBound_sphereToRegionVolumeRatio = ((double)vct_sphere.size()/double_volumeTmp);
				}
			}
			//this->double_sphereGrowingTolerateLevel = this->double_sphereGrowingTolerateLevel/count_region;
			this->lowerBound_regionVolume =this->min_exemplarRegionVolume*const_multiplier_regionVolumeLowerBound;
			if (this->lowerBound_regionVolume < const_min_regionSize)
			{
				this->lowerBound_regionVolume = const_min_regionSize;
			}
			return true;
		}
		#pragma endregion
		
		#pragma region "locate voxels in image with top voxel value (i.e. intensity)"
		void categorizeVoxelsByValue(vector<int> vct_histoInput, unsigned char* Image1D_input, V3DLONG count_imageSize)
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
		
		#pragma region "methods to estimate threshold"
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

		static int estimateThresholdYen(vector<int> histo_input)
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

		static vector<int> getHistogramFromImage1D(unsigned char* ImageID_input, V3DLONG count_inputImageVectorLength)
		{
			vector<int> vct_result (const_length_histogram, 0);
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
			vector<V3DLONG> vct_tmp;
			for (int r=0;r<=const_max_preGenerateSphereRadius;r++)
			{
				vector<V3DLONG> vct_tmp = this->generateCentralizedSphere(r);
				this->vctList_preGeneratedSphere.push_back(vct_tmp);
			}
		}

		vector<V3DLONG> generateCentralizedSphere(V3DLONG int_radius)
		{
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_tmp;
			double theta;
			double phi;
			V3DLONG count_loop;
			for (V3DLONG r=1; r<=int_radius; r++)
			{
				count_loop = r*const_count_sphereSection;
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
			}
			//sort the result and remove duplicate voxels;
			sort (vct_tmp.begin(), vct_tmp.end());
			for (int i=1;i<vct_tmp.size();i++)
			{
				if(vct_tmp[i]!=vct_tmp[i-1])
				{
					vct_result.push_back(vct_tmp[i]);
				}
			}
			return vct_result;
		}
		#pragma endregion

		#pragma region "utility functions"
		V3DLONG getCenter(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> vct_xyz;
			V3DLONG x;
			V3DLONG y;
			V3DLONG z;
			double xSum=0;
			double ySum=0;
			double zSum=0;
			V3DLONG count_voxel = vct_input.size();
			for (int i=0;i<count_voxel;i++)
			{
				vct_xyz = this->index2Coordinate(vct_input[i]);
				x=vct_xyz[0];
				y=vct_xyz[1];
				z=vct_xyz[2];
				xSum += x;
				ySum += y;
				zSum += z;
			}
			return this->coordinate2Index((V3DLONG)xSum/count_voxel, (V3DLONG)ySum/count_voxel,(V3DLONG) zSum/count_voxel);
		}

		V3DLONG getMassCenter(vector<V3DLONG> vct_input)
		{
			vector<V3DLONG> vct_xyz;
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
				vct_xyz = this->index2Coordinate(vct_input[i]);
				value_tmp = this->Image1D_page[i];
				x=vct_xyz[0];
				y=vct_xyz[1];
				z=vct_xyz[2];
				sum_X += x*value_tmp;
				sum_Y += y*value_tmp;
				sum_Z += z*value_tmp;
				sum_mass += value_tmp;
			}
			return this->coordinate2Index(sum_X/sum_mass, sum_Y/sum_mass, sum_Z/sum_mass);
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
			vector<V3DLONG> vct_xyz (3, 0);
			vector<V3DLONG> vct_result (6, 0);
			V3DLONG idx_tmp;
			for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
			{
				idx_tmp = vct_input[idx_voxel];
				vct_xyz = this->index2Coordinate(idx_tmp);
				x = vct_xyz[0];
				y = vct_xyz[1];
				z = vct_xyz[2];
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

		void removeSingleVoxel(unsigned char* Image1D_input, V3DLONG count_imageSize)
		{
			bool flag_remove = true;
			double double_currentValue = 0;
			double double_neightborValue = 0;
			V3DLONG idx_neighbor = 0;
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
					}
				}
			}
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
			vector<V3DLONG> vct_xyz = index2Coordinate(idx_Input);
			V3DLONG x=vct_xyz[0];
			V3DLONG y=vct_xyz[1];
			V3DLONG z=vct_xyz[2];
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

		V3DLONG coordinate2Index(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return z*offset_Z+y*offset_Y+x;
		}

		double getValueAtCoordiante(unsigned char* Image1D_input, V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return ((double)Image1D_input[coordinate2Index(x, y, z)]);
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

		V3DLONG vctContains(vector<V3DLONG> vct_input, V3DLONG idx_input)
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
		vector<V3DLONG> menashiftClustering(const vector<V3DLONG> vct_input, const double double_bandwidthRatio)
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
			vector<V3DLONG> vct_clusterCenter;
			vector<bool> vct_visited (count_voxel, false);
			V3DLONG idx_mean;
			vector<V3DLONG> vct_xyz1;
			vector<V3DLONG> vct_xyz2;
			V3DLONG idx_meanOld;
			vector<double> vct_distTmp (count_voxel, 0);
			vector<V3DLONG> vct_includeTmp;
			vector<V3DLONG> vct_seed = vct_input;
			V3DLONG idx_merge = 0;
			V3DLONG count_cluster = 0;
			V3DLONG idx_rand = 0;
			if (this->is_debugging) {ofstream_log<<"initialization passed!"<<endl;}
			while (!vct_seed.empty())
			{
				if (this->is_debugging) {ofstream_log<<"calculation started!"<<endl;}
				idx_rand = rand()%vct_seed.size();
				if (this->is_debugging) {ofstream_log<<"idx_rand:"<<idx_rand<<endl;}
				idx_mean = vct_seed[idx_rand];
				if (this->is_debugging) {ofstream_log<<"idx_mean:"<<idx_mean<<endl;}
				vct_includeTmp.clear();
				if (this->is_debugging) {ofstream_log<<"fill passed!"<<endl;}
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
							vct_includeTmp.push_back(vct_input[i]);
							vct_visited[i] = true;
						}
					}
					if (this->is_debugging) {ofstream_log<<"get vote passed!"<<endl;}
					if (vct_includeTmp.size()<1)
					{
						break;
					}
					if (this->is_debugging) {ofstream_log<<"vct_includeTmp.size():"<<vct_includeTmp.size()<<endl;}
					idx_meanOld = idx_mean;
					idx_mean = this->getCenter(vct_includeTmp);
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
							vct_xyz1 = this->index2Coordinate(vct_clusterCenter[idx_merge]);
							vct_xyz2 = this->index2Coordinate(idx_mean);
							vct_xyz1[0] = 0.5*(vct_xyz1[0]+vct_xyz2[0]);
							vct_xyz1[1] = 0.5*(vct_xyz1[1]+vct_xyz2[1]);
							vct_xyz1[2] = 0.5*(vct_xyz1[2]+vct_xyz2[2]);
							vct_clusterCenter[idx_merge] = this->coordinate2Index(vct_xyz1[0], vct_xyz1[1], vct_xyz1[2]);
							if (this->is_debugging) {ofstream_log<<"merging succeed!"<<endl;}
						}
						else
						{
							if (this->is_debugging) {ofstream_log<<"creating start!"<<endl;}
							vct_clusterCenter.push_back(idx_mean);
							if (this->is_debugging) {ofstream_log<<"creating succeed"<<endl;}
						}
						count_cluster = vct_clusterCenter.size();
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
			if (this->is_debugging) {ofstream_log<<"succeed!"<<endl;}
			return vct_clusterCenter;
		}
		#pragma endregion
};
#pragma endregion

#pragma region "Segmentation interface"
bool segmentationInterface(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
	ofstream ofstream_log;
	ofstream_log.open ("log_func_interface_Segmentation.txt");
	//get current image;
    v3dhandle v3dhandle_currentWindow = V3DPluginCallback2_currentCallback.currentImageWindow();
    //cancels if no image loaded;
    if (!v3dhandle_currentWindow)
    {
        v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		ofstream_log.close();
        return false;
    }
    //try to pull the data as an Image4DSimple (3D+channel);
    Image4DSimple* Image4DSimple_current = V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
	if (!Image4DSimple_current)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		ofstream_log.close();
        return false;
	}
	V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
	if (count_totalBytes < 1)
	{
		v3d_msg("You have not loaded any image or the image is corrupted, program canceled!");
		ofstream_log.close();
        return false;
	}
	ImagePixelType ImagePixelType_current = Image4DSimple_current->getDatatype();
	if(ImagePixelType_current != V3D_UINT8)
	{
		v3d_msg("Currently this program only support 8-bit data, program canceled!");
		ofstream_log.close();
		return false;
	}
	//sets data into 1D array, note: size is count_totalVoxelsCurrent;
    unsigned char* Image1D_current = Image4DSimple_current->getRawData();
	ofstream_log<<"Voxel count (by couting bytes): "<<(count_totalBytes/sizeof(Image1D_current[0]))<<endl;
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
		ofstream_log.close();
        return false;
    }
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList>0)) //Both of the lists are not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count:"<<count_currentLandmarkList<<endl;
	}
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1)) //Only SWCList_current is not empty;
	{
		class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count:"<<count_currentLandmarkList<<endl;
	}
	if (count_userDefinedLandmarkList>0) //Only LandmarkList_userDefined is not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count:"<<count_currentLandmarkList<<endl;
	}
	//open dialogMain1 window, get paramters;
    dialogMain dialogMain1(V3DPluginCallback2_currentCallback, QWidget_parent, int_channelDim);
    if (dialogMain1.exec()!=QDialog::Accepted)
    {
		ofstream_log.close();
        return false;
    }
    V3DLONG idx_channel = dialogMain1.int_channel;
    double  threshold = dialogMain1.threshold;
    if (threshold>const_max_voxeValue)
	{
		v3d_msg("Please provide a valid threshold, program canceled!"); 
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"initialization of segmentationMain start!"<<endl;
	class_segmentationMain segmentationMain1(threshold, Image1D_current, dim_X, dim_Y, dim_Z, idx_channel, LandmarkList_current, const_flag_debugMode);
	if (!segmentationMain1.is_success)
	{
		v3d_msg("warning:initialization of segmentationMain failed, program will terminate!");
		ofstream_log<<"segmentation failed!"<<endl;
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"initialization of segmentationMain succeed!"<<endl;
	ofstream_log<<"dim_X:"<<segmentationMain1.dim_X<<", dim_Y:"<<segmentationMain1.dim_Y<<", dim_Z:"<<segmentationMain1.dim_Z<<"!"<<endl;
	ofstream_log<<"size_page:"<<segmentationMain1.size_page<<"!"<<endl;
	ofstream_log<<"current channel:"<<segmentationMain1.idx_channel<<"!"<<endl;
	ofstream_log<<"estimated threshold:"<<segmentationMain1.threshold<<"!"<<endl;
	ofstream_log<<"upperBound_sphereToRegionVolumeRatio:"<<segmentationMain1.upperBound_sphereToRegionVolumeRatio<<"!"<<endl;
	if (segmentationMain1.vct_exemplar.empty())
	{
		ofstream_log<<"no exemplar grown!"<<endl;
	}
	else
	{
		ofstream_log<<"exemplar region sizes:"<<endl;
		for (int i=0;i<segmentationMain1.vctList_exemplarRegion.size();i++)
		{
			ofstream_log<<"	region "<<(i+1)<<":"<<segmentationMain1.vctList_exemplarRegion[i].size()<<"!"<<endl;
		}
		ofstream_log<<"max_exemplarRegionVolume:"<<segmentationMain1.max_exemplarRegionVolume<<"!"<<endl;
		ofstream_log<<"min_exemplarRegionVolume:"<<segmentationMain1.min_exemplarRegionVolume<<"!"<<endl;
		ofstream_log<<"lowerBound_regionVolume:"<<segmentationMain1.lowerBound_regionVolume<<"!"<<endl;
		ofstream_log<<"exemplar shape profile:"<<endl;
		for (int i=0;i<segmentationMain1.vctList_exemplarRegionBoudingBox.size();i++)
		{
			ofstream_log<<"	region "<<(i+1)<<":"<<segmentationMain1.vctList_exemplarRegion[i].size()<<endl;
			ofstream_log<<"xMin:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][0]<<", xMax:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][1]<<"!"<<endl;
			ofstream_log<<"yMin:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][2]<<", yMax:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][3]<<"!"<<endl;
			ofstream_log<<"zMin:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][4]<<", zMax:"<<segmentationMain1.vctList_exemplarRegionBoudingBox[i][5]<<"!"<<endl;
		}
	}
	ofstream_log<<"total regions grown:"<<segmentationMain1.vctList_segmentationResult.size()<<"!"<<endl;
	//visualizationImage1D(segmentationMain1.Image1D_exemplarRegion,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Exemplar Regions");
	visualizationImage1D(segmentationMain1.Image1D_segmentationResult,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result");
	V3DPluginCallback2_currentCallback.setLandmark(v3dhandle_currentWindow, segmentationMain1.LandmarkList_segmentationResult);
	V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_currentWindow);
	v3d_msg(QString("Segmentation succeed! Totally %1 segments!").arg(segmentationMain1.vctList_segmentationResult.size()));
	ofstream_log.close();
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