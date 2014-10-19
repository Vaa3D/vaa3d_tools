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
#include <time.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <fstream>
#include <stdlib.h>
#include <ctime>
#include <basic_landmark.h>
using namespace std;

const int const_int_histogramLength = 256;
const double const_double_maxVoxelIntensity = 255;
const int const_int_count_neighbors = 26; //27 directions -1;
const int const_int_maximumGrowingLength = 10000;
const int const_int_minimumIntensityConsideredAsNonZero = 2; //as voxel with value of "1" is unreasonable to be considered as foreground;
const double const_double_defaultThreshold = 15; //a small enough value for the last resort;
const double const_double_valueChangeThreshold = 0.8; //how much the value can change (relative to the mean value of current region) between current voxel and its neighbor, during seed-based region growing;
const double const_double_exemplarRegionThresholdLoosenBy = 0.4; //this value times the mean value of current region is the threshold used during seed-based region growing;
const double const_double_fragmentRatioToPageSizeWarning = 0.05; //if a region with volume larger than this #% of the whole image, a warning would be given;
const double const_double_regionFittingTolerateLevel = 0.90; //percentage of the region fit to be considered as "fit";
const double const_double_regionVolumeLowerBoundLoosenBy = 0.25;  //it would be a user-specified value in the future !!!;
const double const_double_regionVolumeUpperBoundLoosenBy = 1.25;  //it would be a user-specified value in the future !!!;
const double const_double_earlyStopCriteria = 0.95;  //it would be a user-specified value in the future !!!;
const int const_int_tinyFragmentCriteria = 8; //2*2*2, when a region is considered as non-acceptable small;

#define INF 1E9
#define NINF -1E9
#define PI 3.14159265

#define UBYTE unsigned char
#define BYTE signed char
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
		V3DLONG vct_neighbors[const_int_count_neighbors];

		//Input or directly derived;
		bool flag_debug;
		double double_threshold;
		unsigned char* Image1D_original;
		unsigned char *Image1D_page;
		unsigned char *Image1D_mask;
		V3DLONG dim_X;
		V3DLONG dim_Y;
		V3DLONG dim_Z;
		V3DLONG count_totalPageSize;
		V3DLONG offset_channel;
		V3DLONG offset_Z;
		V3DLONG offset_Y;
		int idx_channel;
		vector<int> histo_main;
		bool flag_success;
		
		//Exemplar;
		LandmarkList LandmarkList_exemplar;
		vector<V3DLONG> vct_exemplarIndex;
		V3DLONG int_boundaryRegionCriteria; //max number of "black" voxels around the boundary;
		V3DLONG int_boundaryRegionCriteriaEstimation; 
		unsigned char *Image1D_exemplarRegion;
		vector<vector<V3DLONG> > vctList_exemplarRegionIndex;
		//vector<vector<V3DLONG> > vctList_exemplarRegionCentralizedIndex;
		V3DLONG int_maxRegionVolumeByExemplar;
		V3DLONG int_minRegionVolumeByExemplar;
		V3DLONG int_meanRegionVolumeByExemplar;
		
		//regonGrow;
		V3DLONG int_regionVolumeLowerBound;
		V3DLONG int_regionVolumeUpperBound;
		vector<vector<V3DLONG> > vctList_grownRegionIndex;
		vector<vector<V3DLONG> > vctList_potentialSeedListIndex;
		unsigned char* Image1D_segmentation;
		
		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input, bool flag_debugInput)
		{
			this->flag_success = false;
			this->flag_debug = flag_debugInput;
			this->Image1D_original = Image1D_input;
			this->dim_X = int_xDimInput; this->dim_Y = int_yDimInput; this->dim_Z = int_zDimInput; this->idx_channel = int_channelInput;
			this->LandmarkList_exemplar = LandmarkList_input;
			this->count_totalPageSize = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*count_totalPageSize;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
			this->int_boundaryRegionCriteria = -1; //it will be later updated in regonGrowOnExemplar();
			this->int_boundaryRegionCriteriaEstimation = 0;
			this->int_regionVolumeLowerBound = const_int_tinyFragmentCriteria; //prevent ultra-small exemplar region growing;
			this->int_regionVolumeUpperBound = INF;
			this->createNeighborVector();
			this->Image1D_page = new unsigned char [this->count_totalPageSize];
			this->Image1D_mask = new unsigned char [this->count_totalPageSize];
			this->Image1D_exemplarRegion = new unsigned char [this->count_totalPageSize];
			this->Image1D_segmentation =  new unsigned char [this->count_totalPageSize];
			for (V3DLONG i=0;i<count_totalPageSize;i++)
			{	
				this->Image1D_page[i] = Image1D_original[i+offset_channel];
				this->Image1D_mask[i] = const_double_maxVoxelIntensity; //all available;
				this->Image1D_exemplarRegion[i] = 0;
				this->Image1D_segmentation[i] = 0;
			}
			ofstream ofstream_log;
			if (this->flag_debug) {	ofstream_log.open ("log_class_segmentationMain.txt");}
			vct_exemplarIndex = landMarkList2IndexList(LandmarkList_exemplar);
			if (this->flag_debug) {	ofstream_log<<"estimateThreshold started!"<<endl;}
			this->estimateThreshold(); //store to this->double_threshold;
			this->removeSingleVoxel(this->Image1D_page, this->count_totalPageSize);
			if (this->flag_debug) {	ofstream_log<<"estimateThreshold succeed!"<<endl;}
			if (this->flag_debug) {	ofstream_log<<"categorizeVoxelsByValue started!"<<endl;}
			categorizeVoxelsByValue(this->histo_main, this->Image1D_page, this->count_totalPageSize); //store to this->vctList_potentialSeedListIndex; Although currently doesn't matter, this step is recommended to be before thresholding on Image1D_page;
			if (this->flag_debug) {	ofstream_log<<"categorizeVoxelsByValue succeed!"<<endl;}
			if (this->flag_debug) {	ofstream_log<<"regonGrowOnExemplar started!"<<endl;}
			//note: as it needs to update and check Image1D_mask for scooping, thresholding on Image1D_page needs to perform AFTER it! Otherwise Image1D_mask would be messed!
			if (!this->regonGrowOnExemplar())
			{
				if (this->flag_debug) {	ofstream_log.close();}
				return; //failed;
			}
			if (this->flag_debug) {	ofstream_log<<"regonGrowOnExemplar succeed!"<<endl;}
			if (this->flag_debug) {	ofstream_log<<"thresholdForExemplarRegion started!"<<endl;}
			this->thresholdForExemplarRegion(); //only after thresholding it is reasonable to analyze the geometry property of exemplar regions; 
			if (this->flag_debug) {	ofstream_log<<"thresholdForExemplarRegion succeed!"<<endl;}
			if (this->flag_debug) {	ofstream_log<<"analyzeExemplarRegionVolume started!"<<endl;}
			//it should be right after thresholdForExemplarRegion() (acutally I pulled this function out from it);
			if (!this->analyzeExemplarRegionVolume())
			{
				if (this->flag_debug) {ofstream_log.close();}
				return; //failed;
			}
			if (this->flag_debug) {	ofstream_log<<"analyzeExemplarRegionVolume succeed!"<<endl;}
			
			/* Not used in current setting;
			if (this->flag_debug) {	ofstream_log<<"centralizeRegion started!"<<endl;}
			this->vctList_exemplarRegionCentralizedIndex=this->centralizeRegion(this->Image1D_page, this->count_totalPageSize, this->vctList_exemplarRegionIndex);
			if (this->flag_debug) {ofstream_log<<"centralizeRegion succeed!"<<endl;}
			*/
			
			if (this->flag_debug) {	ofstream_log<<"thresholdForCurrentPage started!"<<endl;}
			this->thresholdForCurrentPage(); // inal step; Preparing for fragment remove in regionGrowing;
			if (this->flag_debug) {	ofstream_log<<"thresholdForCurrentPage succeed!"<<endl;}
			this->int_regionVolumeUpperBound =this->int_maxRegionVolumeByExemplar*const_double_regionVolumeUpperBoundLoosenBy; //Preparing for region growing boundary in regionGrowing;
			this->int_regionVolumeLowerBound =this->int_minRegionVolumeByExemplar*const_double_regionVolumeLowerBoundLoosenBy; //Preparing for fragment remove in regionGrowing;
			flag_success = true;
			if (this->flag_debug) {	ofstream_log.close();}
		}

		#pragma region "regionGrow"
		void regonGrowOnCurrentPage()
		{
			ofstream ofstream_log;
			if (this->flag_debug) {ofstream_log.open ("log_regonGrowOnCurrentPage.txt");}
			memset(this->Image1D_segmentation, 0, this->count_totalPageSize);
			V3DLONG idx_tmp;
			V3DLONG count_potentialSeedList = this->vctList_potentialSeedListIndex.size();
			vctList_grownRegionIndex.clear();
			if (this->flag_debug) {ofstream_log<<"count_potentialSeedList: "<<count_potentialSeedList<<endl;}
			vector<V3DLONG> vct_temp;
			V3DLONG count_totalCoveredVoxel = 0;
			V3DLONG count_potentialSeed = 0;
			for (V3DLONG idx_potentialSeedList=0;idx_potentialSeedList<count_potentialSeedList;idx_potentialSeedList++) //start from the highest intensity seeds;
			{
				count_potentialSeed = this->vctList_potentialSeedListIndex[idx_potentialSeedList].size();
				for (V3DLONG idx_potentialSeed=0;idx_potentialSeed<count_potentialSeed;idx_potentialSeed++)
				{
					idx_tmp = this->vctList_potentialSeedListIndex[idx_potentialSeedList][idx_potentialSeed];
					vct_temp = this->regionGrowOnSeed(this->Image1D_page, this->count_totalPageSize, idx_tmp); //Image1D_mask is updating in this function;
					if (this->flag_debug) {	ofstream_log<<"at idx_potentialSeed==: "<<idx_potentialSeed<<", vct_temp.size(): "<<vct_temp.size()<<endl;}
					if (!vct_temp.empty())	{this->vctList_grownRegionIndex.push_back(vct_temp);}
					count_totalCoveredVoxel += vct_temp.size();
				}
				if (count_totalCoveredVoxel>this->count_totalPageSize*const_double_earlyStopCriteria)
				{
					return;
				}
			}
			//Update Image1D_segmentation;
			V3DLONG count_region = this->vctList_grownRegionIndex.size();
			if (this->flag_debug) {ofstream_log<<"count_region: "<<count_region<<endl;}
			V3DLONG count_voxel = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = this->vctList_grownRegionIndex[idx_region].size();
				if (this->flag_debug) {ofstream_log<<"at idx_region: "<<idx_region<<"count_voxel: "<<count_voxel<<endl;}
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = this->vctList_grownRegionIndex[idx_region][idx_voxel];
					if (this->flag_debug) {ofstream_log<<"at idx_voxel: "<<idx_voxel<<" idx_tmp: "<<idx_tmp<<endl;}
					if ((idx_tmp>=0)&&(idx_tmp<this->count_totalPageSize))
					{
						this->Image1D_segmentation[idx_tmp] = (idx_region+1);
					}
				}
			}
		}

		vector<V3DLONG> regionGrowOnSeed(unsigned char* Image1D_input, V3DLONG count_imageSizeInput, V3DLONG idx_seed)
		//assuming [0-const_double_maxVoxelIntensity] integer Image1D_page, should also work on binary image (not tested yet);
		{
			ofstream ofstream_log;
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_empty (0, 0);
			vector<V3DLONG> vct_temp;
			V3DLONG idx_current;
			V3DLONG count_boundaryBlack = 0;
			V3DLONG count_boundary = 0;
			V3DLONG count_boundaryBlackSum = 0;
			int count_neighbors = 0;
			V3DLONG idx_neighbor;
			double double_neightborValue;
			double double_currentValue;
			double double_meanValue = 0;
			double double_sumValue = 0;
			if ((idx_seed<0)||(idx_seed>=count_imageSizeInput)) //unlikely, but just in case;
			{
				if (this->flag_debug) {ofstream_log.close();}
				return vct_empty;
			}
			double double_seedValue = Image1D_input[idx_seed];
			V3DLONG count_totalVolume = 0;
			if (double_seedValue>this->double_threshold)
			{
				vct_temp.push_back(idx_seed);
				vct_result.push_back(idx_seed);
				double_sumValue = double_seedValue;
				double_meanValue = double_seedValue;
				for (V3DLONG i=0;i<const_int_maximumGrowingLength;i++)
				{
					if (vct_temp.empty()) //growing complete;
					{
						if (this->flag_debug) {ofstream_log.close();}
						if(count_totalVolume<int_regionVolumeLowerBound)
						{
							return vct_empty;
						}
						else
						{
							if (this->int_boundaryRegionCriteria<0) {this->int_boundaryRegionCriteriaEstimation=(int)(count_boundaryBlackSum/count_boundary);} //if this->int_boundaryRegionCriteria not udpated yet, store the result;
							return vct_result;
						}
					}
					idx_current = vct_temp.back();
					vct_temp.pop_back();
					double_currentValue = Image1D_input[idx_current];
					
					count_boundaryBlack = 0;
					for (V3DLONG j=0;j<const_int_count_neighbors;j++)
					{
						idx_neighbor = idx_current+vct_neighbors[j];
						if ((idx_neighbor>=0)&&(idx_neighbor<count_imageSizeInput)) //prevent it from going out of bounds;
						{
							double_neightborValue = Image1D_input[idx_neighbor];
							if (double_neightborValue<this->double_threshold) {count_boundaryBlack++;}; //it is black and count it;
						}
					}
					if (this->int_boundaryRegionCriteria>0) //only if this->int_boundaryRegionCriteria has been udpated (i.e. >0, during whole image growing stage);
					{
						if (count_boundaryBlack>this->int_boundaryRegionCriteria)
						{
							continue; //boundary voxel detected, do not proceed;
						}
					}
					if (this->int_boundaryRegionCriteria<0) {count_neighbors = 0;}
					for (int j=0;j<const_int_count_neighbors;j++)
					{
						idx_neighbor = idx_current+vct_neighbors[j];
						if ((idx_neighbor>=0)&&(idx_neighbor<count_imageSizeInput)) //prevent it from going out of bounds;
						{
							if (this->Image1D_mask[idx_neighbor]>0) //available only; Note: make sure Image1D_mask has been set to the correct value!
							{
								double_neightborValue = Image1D_input[idx_neighbor];
								if ((double_neightborValue>this->double_threshold)&&((abs(double_neightborValue-double_currentValue))<double_meanValue*const_double_valueChangeThreshold))
								{
									this->Image1D_mask[idx_neighbor] = 0; //scooped;
									vct_temp.push_back(idx_neighbor);
									vct_result.push_back(idx_neighbor);
									double_sumValue = double_sumValue+double_neightborValue;
									count_totalVolume++;
									double_meanValue = double_sumValue/count_totalVolume;
									if (this->int_boundaryRegionCriteria<0) {count_neighbors++;}
								}
							}
						}
					}
					//if no neighbors added, it is a boundary voxel;
					//only if this->int_boundaryRegionCriteria has not been udpated (i.e. <0, during whole exemplar growing stage);
					if (this->int_boundaryRegionCriteria<0) 
					{
						if (count_neighbors==0)
						{
							count_boundaryBlackSum += count_boundaryBlack;
							count_boundary ++;
						}
					}
					
					if (count_totalVolume>int_regionVolumeUpperBound)
					//during exemplar region growing stage, int_regionVolumeUpperBound shall be INF;
					//during page image region growing stage (i.e. segmentation), int_regionVolumeUpperBound shall be the value estimated from exemplar regions;
					{	
						if (this->int_boundaryRegionCriteria<0) {this->int_boundaryRegionCriteriaEstimation=(int)(count_boundaryBlackSum/count_boundary);} //if this->int_boundaryRegionCriteria not udpated yet, store the result;
						return vct_result;
					}
				}
				//iterations over const_int_maximumGrowingLength;
				v3d_msg("Warning: maximum iterations reached, results may become unstable!");
				if(count_totalVolume<int_regionVolumeLowerBound)
				{
					return vct_empty;
				}
				else
				{
					if (this->int_boundaryRegionCriteria<0) {this->int_boundaryRegionCriteriaEstimation=(int)(count_boundaryBlackSum/count_boundary);} //if this->int_boundaryRegionCriteria not udpated yet, store the result;
					return vct_result;
				}
			}
			else
			{
				return vct_empty;
			}
		}
		#pragma endregion		

		#pragma region "exemplar" 
		vector<vector<V3DLONG> > centralizeRegionNoWeight(vector<vector<V3DLONG> > vctList_input)
		{
			vector<vector<V3DLONG> > vctList_ouput = vctList_input;
			V3DLONG idx_tmp = 0;
			vector<V3DLONG> vct_xyz (0, 0);
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG centerX = 0;
			V3DLONG centerY = 0;
			V3DLONG centerZ = 0;
			V3DLONG count_region = vctList_input.size();
			V3DLONG count_voxel = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = vctList_input[idx_region].size();
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vctList_input[idx_region][idx_voxel];
					{
						vct_xyz = index2Coordinate(idx_tmp);
						x = vct_xyz[0];
						y = vct_xyz[1];
						z = vct_xyz[2];
						centerX += x;
						centerY += y;
						centerZ += z;
					}
				}
				centerX = centerX/count_voxel;
				centerY = centerY/count_voxel;
				centerZ = centerZ/count_voxel;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vctList_ouput[idx_region][idx_voxel];
					vct_xyz = this->index2Coordinate(idx_tmp);
					x = vct_xyz[0] - centerX;
					y = vct_xyz[1] - centerY;
					z = vct_xyz[2] - centerZ;
					vctList_ouput[idx_region][idx_voxel] = this->coordinate2Index(x, y, z);
				}
			}
			return vctList_ouput;
		}

		vector<vector<V3DLONG> > centralizeRegion(unsigned char* Image1D_input, V3DLONG count_imageSizeInput, vector<vector<V3DLONG> > vctList_input)
		{
			vector<vector<V3DLONG> > vctList_ouput = vctList_input;
			V3DLONG int_indexTmp = 0;
			vector<V3DLONG> vct_xyz (0, 0);
			double double_valueTmp;
			double double_weightedX = 0;
			double double_weightedY = 0;
			double double_weightedZ = 0;
			double double_massSum = 0;
			V3DLONG x = 0;
			V3DLONG y = 0;
			V3DLONG z = 0;
			V3DLONG count_region = vctList_input.size();
			V3DLONG count_voxel = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				double_weightedX = 0;
				double_weightedY = 0;
				double_weightedZ = 0;
				double_massSum = 0;
				count_voxel = vctList_input[idx_region].size();
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					int_indexTmp = vctList_input[idx_region][idx_voxel];
					if ((int_indexTmp>0)&&(int_indexTmp<count_imageSizeInput))
					{
						double_valueTmp = (double) Image1D_input[int_indexTmp];
						vct_xyz = index2Coordinate(int_indexTmp);
						x = vct_xyz[0];
						y = vct_xyz[1];
						z = vct_xyz[2];
						double_weightedX += x*double_valueTmp;
						double_weightedY += y*double_valueTmp;
						double_weightedZ += z*double_valueTmp;
						double_massSum += double_valueTmp;
					}
				}
				double_weightedX = double_weightedX/double_massSum;
				double_weightedY = double_weightedY/double_massSum;
				double_weightedZ = double_weightedZ/double_massSum;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					int_indexTmp = vctList_ouput[idx_region][idx_voxel];
					vct_xyz = this->index2Coordinate(int_indexTmp);
					x = vct_xyz[0] - double_weightedX;
					y = vct_xyz[1] - double_weightedY;
					z = vct_xyz[2] - double_weightedZ;
					vctList_ouput[idx_region][idx_voxel] = this->coordinate2Index(x, y, z);
				}
			}
		}

		bool regonGrowOnExemplar()
		{
			ofstream ofstream_log;
			if (this->flag_debug) {	ofstream_log.open ("log_regonGrowOnExemplar.txt");}
			this->int_boundaryRegionCriteriaEstimation = 0;
			vector<V3DLONG> vct_temp;
			V3DLONG idx_exemplar;
			if (this->flag_debug) {ofstream_log<<"vct_exemplarIndex.size(): "<<vct_exemplarIndex.size()<<endl;}
			for (V3DLONG i=0;i<this->vct_exemplarIndex.size();i++)
			{
				idx_exemplar = this->vct_exemplarIndex[i];
				vct_temp = this->regionGrowOnSeed(this->Image1D_page, this->count_totalPageSize, idx_exemplar);
				if (this->flag_debug) {ofstream_log<<"at i==: "<<i<<"vct_temp.size(): "<<vct_temp.size()<<endl;}
				if (!vct_temp.empty())
				{
					this->vctList_exemplarRegionIndex.push_back(vct_temp);
					if ((this->int_boundaryRegionCriteriaEstimation)>this->int_boundaryRegionCriteria) {this->int_boundaryRegionCriteria=this->int_boundaryRegionCriteriaEstimation;} //update int_boundaryRegionCriteria to the max black voxel count;
				}
			}
			V3DLONG count_region = this->vctList_exemplarRegionIndex.size();
			if (this->flag_debug) {ofstream_log<<"count_region: "<<count_region<<endl;}
			V3DLONG count_voxel = 0;
			V3DLONG idx_tmp = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = this->vctList_exemplarRegionIndex[idx_region].size();
				if (this->flag_debug) {ofstream_log<<"at idx_region: "<<idx_region<<"count_voxel: "<<count_voxel<<endl;}
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = this->vctList_exemplarRegionIndex[idx_region][idx_voxel];
					if (this->flag_debug) {ofstream_log<<"at idx_voxel: "<<idx_voxel<<" idx_tmp: "<<idx_tmp<<endl;}
					if ((idx_tmp>=0)&&(idx_tmp<this->count_totalPageSize))
					{
						this->Image1D_exemplarRegion[idx_tmp] = this->Image1D_page[idx_tmp];
					}
				}
			}
			if (count_region<1)
			{
				v3d_msg("Warning: no exemplar regions grown. Program will terminate, please re-select the exemplar(s)!");
				return false;
			}
			else
			{
				return true;
			}
		}

		bool analyzeExemplarRegionVolume()
		{
			this->int_maxRegionVolumeByExemplar = -INF;
			this->int_minRegionVolumeByExemplar = INF;
			V3DLONG count_region = this->vctList_exemplarRegionIndex.size();
			double double_meanVolume = 0;
			V3DLONG count_voxel = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = this->vctList_exemplarRegionIndex[idx_region].size();
				if (count_voxel>(const_double_fragmentRatioToPageSizeWarning*this->count_totalPageSize))
				{
					v3d_msg("Warning: abnormal large exemplar region detected, program will terminate. Please re-select the exemplar(s)!");
					return false;
				}
				double_meanVolume += count_voxel;
				if (count_voxel>this->int_maxRegionVolumeByExemplar)	{this->int_maxRegionVolumeByExemplar=count_voxel;}
				if (count_voxel<this->int_minRegionVolumeByExemplar)	{this->int_minRegionVolumeByExemplar=count_voxel;}
			}
			double_meanVolume = double_meanVolume/(count_region+1);
			this->int_meanRegionVolumeByExemplar = double_meanVolume;
			return true;
		}

		void thresholdForExemplarRegion() //it's special because we want to update vctList_exemplarRegionIndex accordingly;
		{
			double double_valueTmp;
			V3DLONG idx_tmp;
			vector<V3DLONG> vct_empty (0,0);
			V3DLONG count_voxel = 0;
			V3DLONG count_region = vctList_exemplarRegionIndex.size();
			vector<vector<V3DLONG> > vctList_exemplarRegionIndexCopy = this->vctList_exemplarRegionIndex;
			this->vctList_exemplarRegionIndex.clear();
			bool flag_addRegion = true;
			V3DLONG idx_addRegion = 0;

			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = vctList_exemplarRegionIndexCopy[idx_region].size();
				flag_addRegion = true;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = vctList_exemplarRegionIndexCopy[idx_region][idx_voxel];
					if ((idx_tmp>=this->count_totalPageSize)||(idx_tmp < 0))
					{
						//somthing wrong happened, remove it;
					}
					else
					{	
						double_valueTmp = this->Image1D_exemplarRegion[idx_tmp];
						if (double_valueTmp > this->double_threshold)
						{
							// Do nothing to Image1D_exemplarRegion;
							if (flag_addRegion == true)
							{
								this->vctList_exemplarRegionIndex.push_back(vct_empty);
								flag_addRegion = false;
								idx_addRegion++;
							}
							this->vctList_exemplarRegionIndex[idx_addRegion-1].push_back(idx_tmp);
						}
						else
						{	
							Image1D_exemplarRegion[idx_tmp] = 0;
						}
					}
				}
			}
		}
		#pragma endregion
		
		#pragma region "locate voxels in image with top voxel value (i.e. intensity)"
		void categorizeVoxelsByValue(vector<int> vct_histoInput, unsigned char* Image1D_input, V3DLONG count_imageSize)
		//will only consider voxels with value higher than threshold;
		{
			this->vctList_potentialSeedListIndex.clear();
			vector<V3DLONG> vct_empty (0,0);
			int int_threshold = (int)this->double_threshold;
			for (int i=(int_threshold+1);i<const_int_histogramLength;i++)
			{
				this->vctList_potentialSeedListIndex.push_back(vct_empty);
			}
			int int_valueOffset=0;
			int int_valueTmp=0;
			for (int i=0;i<count_imageSize;i++)
			{
				int_valueTmp = (int)Image1D_input[i];
				int_valueOffset = const_double_maxVoxelIntensity-int_valueTmp;
				if (int_valueTmp>int_threshold)
				{
					vctList_potentialSeedListIndex[int_valueOffset].push_back(i);
				}
			}
		}
		#pragma endregion
		
		#pragma region "methods to estimate threshold"
		void estimateThreshold()
		{
			this->histo_main = class_segmentationMain::getHistogramFromImage1D(this->Image1D_page, this->count_totalPageSize);

			this->double_threshold = class_segmentationMain::estimateThresholdYen(histo_main);
			if (this->double_threshold < const_int_minimumIntensityConsideredAsNonZero)
			{
				v3d_msg("Warning: threshold estimation failed, will use default threshold (15) instead!");
				this->double_threshold = const_double_defaultThreshold;
			}
		}

		//Removed, replace by better strategies;
		/*
		double estimateThresholdFromExemplarRegion()
		{
			ofstream ofstream_log;
			if (this->flag_debug)
			{
				ofstream_log.open ("log_estimateThresholdFromExemplarRegion.txt");
			}
			V3DLONG count_region = this->vctList_exemplarRegionIndex.size();
			if (this->flag_debug)
			{
				ofstream_log<<"count_region: "<<count_region<<endl;
			}
			if (count_region<1)
			{
				v3d_msg("Warning: exemplar region grow has not been performed, wrong function call to estimateThresholdFromExemplarRegion()!");
				return 0;
			}
			V3DLONG count_voxel = 0;
			double double_mean = 0;
			double double_sum = 0;
			double double_meanSum = 0;
			double double_stdev = 0;
			double double_tmp = 0;
			double * vct_threshold = new double[count_region];
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				if (this->flag_debug)
				{
					ofstream_log<<"idx_region: "<<idx_region<<endl;
				}
				count_voxel = this->vctList_exemplarRegionIndex[idx_region].size();
				if (this->flag_debug)
				{
					ofstream_log<<"count_voxel: "<<count_voxel<<endl;
				}
				double_sum = 0;
				double_stdev = 0;
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					double_tmp = (double) this->Image1D_page[vctList_exemplarRegionIndex[idx_region][idx_voxel]];
					double_sum += double_tmp;
				}
				double_mean = double_sum/count_voxel;
				if (this->flag_debug)
				{
					ofstream_log<<"double_mean: "<<double_mean<<endl;
					ofstream_log<<"double_sum: "<<double_sum<<endl;
				}
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					double_tmp = (double) this->Image1D_page[vctList_exemplarRegionIndex[idx_region][idx_voxel]];
					double_stdev += pow((double_tmp-double_mean), 2);
				}
				double_stdev = double_stdev/count_voxel;
				double_stdev = pow(count_voxel, 0.5);
				if (this->flag_debug)
				{
					ofstream_log<<"double_stdev: "<<double_stdev<<endl;
				}
				double_meanSum += (double_mean-double_stdev*const_double_distributionStdevMultiplier);
			}
			if (this->flag_debug)
			{
				ofstream_log.close();
			}
			return (double_meanSum / count_region);
		}
		*/

		static int estimateThresholdYen(vector<int> histo_input)
		{
			// Implements Yen's thresholding method;
			// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
			// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;
			
			int threshold;
			int ih, it;
			double crit;
			double max_crit;
			double* norm_histo = new double[const_int_histogramLength];
			double* P1 = new double[const_int_histogramLength];
			double* P1_sq = new double[const_int_histogramLength]; 
			double* P2_sq = new double[const_int_histogramLength]; 
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
			vector<int> vct_result (const_int_histogramLength, 0);
			V3DLONG int_valueTmp;
			for (V3DLONG i=0;i<count_inputImageVectorLength;i++)
			{
				int_valueTmp = (int)ImageID_input[i];
				if (int_valueTmp>0 && int_valueTmp<const_int_histogramLength)
				{
					vct_result[int_valueTmp] = vct_result[int_valueTmp]+1;
				}
			}
			return vct_result;
		}
		
		V3DLONG thresholdForCurrentPage()
		{
			V3DLONG count_totalWhite = 0;
			for(V3DLONG i=0; i<this->count_totalPageSize; i++)
			{	
				if ((double)Image1D_page[i]>this->double_threshold)
				{
					//do nothing to Image1D_page;
					count_totalWhite = count_totalWhite + 1;
					Image1D_mask[i] = const_double_maxVoxelIntensity; //available;
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

		#pragma region "utility functions"
		void removeSingleVoxel(unsigned char* Image1D_input, V3DLONG count_imageSize)
		{
			bool flag_remove = true;
			double double_currentValue = 0;
			double double_neightborValue = 0;
			V3DLONG idx_neighbor = 0;
			for (int i=0;i<count_imageSize;i++)
			{
				double_currentValue = Image1D_input[i];
				if (double_currentValue>this->double_threshold) //valid voxel;
				{
					flag_remove = true;
					for (int j=0;j<const_int_count_neighbors;j++)
					{
						idx_neighbor = i+vct_neighbors[j];
						double_neightborValue = Image1D_input[idx_neighbor];
						if ((idx_neighbor>=0)&&(idx_neighbor<count_imageSize)) //prevent it from going out of bounds;
						{
							double_neightborValue = Image1D_input[idx_neighbor];
							if (double_neightborValue>this->double_threshold)
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
			if (idx > -1)
			{
				vct_result[2] = floor((double)idx/(double)offset_Z);
				vct_result[1] = floor((double)(idx-vct_result[2]*offset_Z)/(double)offset_Y);
				vct_result[0] = idx- vct_result[2]*offset_Z-vct_result[1]*offset_Y;
			}
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
			V3DLONG idx_neighbor = 0;
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
							this->vct_neighbors[idx_neighbor] = z*this->offset_Z+y*this->offset_Y+x;
							idx_neighbor = idx_neighbor + 1;
						}
					}
				}
			}
		}

		static unsigned char* vctList2Image1D(vector<vector<V3DLONG> > vctList_input, V3DLONG count_imageSize)
		{
			unsigned char* Image1D_result = new unsigned char[count_imageSize];
			memset(Image1D_result, 0, count_imageSize);
			for (int i=0;i<vctList_input.size();i++)
			{
				for (int j=0;j<vctList_input[i].size();j++)
				{
					Image1D_result[vctList_input[i][j]] = i+1;
				}
			}
			return Image1D_result;
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
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1)) //Only SWCList_current is not empty;
	{
		class_segmentationMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	if (count_userDefinedLandmarkList>0) //Only LandmarkList_userDefined is not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	//open dialogMain1 window, get paramters;
    dialogMain dialogMain1(V3DPluginCallback2_currentCallback, QWidget_parent, int_channelDim);
    if (dialogMain1.exec()!=QDialog::Accepted)
    {
		ofstream_log.close();
        return false;
    }
    V3DLONG idx_channel = dialogMain1.int_channel;
    double double_threshold = dialogMain1.double_threshold;
    if (double_threshold>const_double_maxVoxelIntensity)
	{
		v3d_msg("Please provide a valid threshold, program canceled!"); 
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"Initialization of class_segmentationMain start!"<<endl;
	class_segmentationMain segmentationMain1(double_threshold, Image1D_current, dim_X, dim_Y, dim_Z, idx_channel, LandmarkList_current, false);
	ofstream_log<<"Initialization of class_segmentationMain succeed!"<<endl;
	ofstream_log<<"Current dimension: dim_X: "<<segmentationMain1.dim_X<<", dim_Y: "<<segmentationMain1.dim_Y<<", dim_Z: "<<segmentationMain1.dim_Z<<endl;
	ofstream_log<<"Current dimension: count_totalPageSize: "<<segmentationMain1.count_totalPageSize<<endl;
	ofstream_log<<"Current channel: "<<segmentationMain1.idx_channel<<endl;
	ofstream_log<<"Threshold: "<<segmentationMain1.double_threshold<<endl;
	if (segmentationMain1.vct_exemplarIndex.empty())
	{
		ofstream_log<<"No exemplar grown!"<<endl;
	}
	else
	{
		ofstream_log<<"Exemplar region sizes: "<<endl;
		for (int i=0;i<segmentationMain1.vctList_exemplarRegionIndex.size();i++)
		{
			ofstream_log<<"	  region "<<(i+1)<<": "<<segmentationMain1.vctList_exemplarRegionIndex[i].size()<<endl;
		}
	}
	ofstream_log<<"int_maxRegionVolumeByExemplar: "<<segmentationMain1.int_maxRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_minRegionVolumeByExemplar: "<<segmentationMain1.int_minRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_meanRegionVolumeByExemplar: "<<segmentationMain1.int_meanRegionVolumeByExemplar<<endl;;
	ofstream_log<<"int_regionVolumeLowerBound: "<<segmentationMain1.int_regionVolumeLowerBound<<endl;
	ofstream_log<<"int_regionVolumeUpperBound: "<<segmentationMain1.int_regionVolumeUpperBound<<endl;
	visualizationImage1D(segmentationMain1.Image1D_page,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "After Thresholding");
	visualizationImage1D(segmentationMain1.Image1D_exemplarRegion,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Exemplar Regions");
	
	if (!segmentationMain1.flag_success)
	{
		v3d_msg("Warning: segmentationMain initialization failed, program will terminate!");
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"segmentationMain1.regonGrowOnCurrentPage() start!"<<endl;
	segmentationMain1.regonGrowOnCurrentPage();
	ofstream_log<<"segmentationMain1.regonGrowOnCurrentPage() succeed!"<<endl;
	ofstream_log<<"total regions grown: "<<segmentationMain1.vctList_grownRegionIndex.size()<<"!"<<endl;
	v3d_msg(QString("Segmentation succeed! Totally %1 segments!").arg(segmentationMain1.vctList_grownRegionIndex.size()));
	visualizationImage1D(segmentationMain1.Image1D_segmentation,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result");
	visualizationImage1D(segmentationMain1.Image1D_mask,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Mask");

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