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
const int const_int_defaultMinimumVolume = 27; //3*3*3;
const int const_int_count_neighbors = 26; //27 directions -1;
const int const_int_maximumGrowingLength = 10000;
const int const_int_minimumIntensityConsideredAsNonZero = 2; //as voxel with value of "1" is unreasonable to be considered as foreground;
const double const_double_defaultThreshold = 15; //a small enough value for the last resort;
const double const_double_valueChangeThreshold = 0.8; //how much the value can change (relative to the mean value of current region) between current voxel and its neighbor, during seed-based region growing;
const double const_double_exemplarRegionThresholdLoosenBy = 0.4; //this value times the mean value of current region is the threshold used during seed-based region growing;
const double const_double_fragmentRatioToPageSizeWarning = 0.05; //if a region with volume larger than this #% of the whole image, a warning would be given;
const double const_double_regionFittingTolerateLevel = 0.90; //percentage of the region fit to be considered as "fit";
const double const_double_regionVolumeLowerBoundLoosenBy = 0.33;  //in case the mean-2*stdev method failed (<0), the lower bound of the volume would be set to this value times the mean volume across all exemplar regions;
const double const_double_distributionStdevMultiplier = 2; //multiplier of the stdev for estimating small and large number;

#define INF 1E9
#define NINF -1E9
#define PI 3.14159265

#define UBYTE unsigned char
#define BYTE signed char
#pragma endregion

#pragma region "UI-related functions and function defination"
Q_EXPORT_PLUGIN2(cellSegmentation, cellSegmentation);

bool func_interface_Segmentation(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent);
bool subFunc_NeuronTree2LandmarkList(const NeuronTree & NeuronTree_input, LandmarkList & LandmarkList_output);
void func_Image1DVisualization(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback);

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
        func_interface_Segmentation(V3DPluginCallback2_currentCallback,QWidget_parent);
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
		
		//Exemplar;
		LandmarkList LandmarkList_exemplar;
		vector<V3DLONG> vct_exemplarIndex;
		unsigned char *Image1D_exemplarRegion;
		vector<V3DLONG> vct_exemplarRegionVolume;
		vector<vector<V3DLONG> > vctList_exemplarRegionIndex;
		vector<V3DLONG> vct_exemplarRegionMassCenterIndex;
		vector<vector<V3DLONG> > vctList_exemplarRegionCentralizedIndex;
		V3DLONG int_smallRegionVolumeByExemplar;
		V3DLONG int_largeRegionVolumeByExemplar;
		V3DLONG int_maxRegionVolumeByExemplar;
		V3DLONG int_minRegionVolumeByExemplar;
		V3DLONG int_meanRegionVolumeByExemplar;
		
		//regionFit;
		vector<vector<V3DLONG> > vctList_regionFitIndex;
		V3DLONG int_regionVolumeUpperBound;

		//regonGrow;
		V3DLONG int_regionVolumeLowerBound;
		vector<V3DLONG*> vctList_grownRegionIndex;
		vector<V3DLONG> vct_grownRegionVolume;
		
		class_segmentationMain(double double_thresholdInput, unsigned char* Image1D_input, V3DLONG int_xDimInput, V3DLONG int_yDimInput, V3DLONG int_zDimInput , int int_channelInput, LandmarkList LandmarkList_input, bool flag_debugInput)
		{
			this->flag_debug = flag_debugInput;
			this->Image1D_original = Image1D_input;
			this->dim_X = int_xDimInput;
			this->dim_Y = int_yDimInput;
			this->dim_Z = int_zDimInput;
			this->idx_channel = int_channelInput;
			this->LandmarkList_exemplar = LandmarkList_input;
			this->count_totalPageSize = dim_X*dim_Y*dim_Z;
			this->offset_channel = (idx_channel-1)*count_totalPageSize;
			this->offset_Z = dim_X*dim_Y;
			this->offset_Y = dim_X;
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
							vct_neighbors[idx_neighbor] = z*offset_Z+y*offset_Y+x;
							idx_neighbor = idx_neighbor + 1;
						}
					}
				}
			}
			this->Image1D_page = new unsigned char [this->count_totalPageSize];
			this->Image1D_mask = new unsigned char [this->count_totalPageSize];
			this->Image1D_exemplarRegion = new unsigned char [this->count_totalPageSize];
			for (V3DLONG i=0;i<count_totalPageSize;i++)
			{	
				this->Image1D_page[i] = Image1D_original[i+offset_channel];
				this->Image1D_mask[i] = const_double_maxVoxelIntensity; //all available;
				this->Image1D_exemplarRegion[i] = 0;
			}
			vct_exemplarIndex = landMarkList2IndexList(LandmarkList_exemplar);
			ofstream ofstream_log;
			if (this->flag_debug)
			{
				ofstream_log.open ("log_class_segmentationMain.txt");
			}
			if (this->flag_debug)
			{
				ofstream_log<<"regonGrowOnExemplar started!"<<endl;
			}
			this->regonGrowOnExemplar();
			if (this->flag_debug)
			{
				ofstream_log<<"regonGrowOnExemplar succeed!"<<endl;
			}
			if (this->flag_debug)
			{
				ofstream_log<<"estimateThreshold started!"<<endl;
			}
			this->estimateThreshold();
			if (this->flag_debug)
			{
				ofstream_log<<"estimateThreshold succeed!"<<endl;
			}
			if (this->flag_debug)
			{
				ofstream_log<<"threshold2BinaryForExemplarRegion started!"<<endl;
			}
			this->threshold2BinaryForExemplarRegion();
			if (this->flag_debug)
			{
				ofstream_log<<"threshold2BinaryForExemplarRegion succeed!"<<endl;
			}
			if (this->flag_debug)
			{
				ofstream_log<<"centralizeExemplarRegion started!"<<endl;
			}
			//NOTE: For this function to run correctly, do it AFTER threshold2BinaryForExemplarRegion() and BEFORE threshold2BinaryForMain();
			//Because it would use mass (i.e. voxel value) for calcuating the centers;
			this->centralizeExemplarRegion();
			if (this->flag_debug)
			{
				ofstream_log<<"centralizeExemplarRegion succeed!"<<endl;
			}
			if (this->flag_debug)
			{
				ofstream_log<<"threshold2BinaryForMain started!"<<endl;
			}
			this->threshold2BinaryForMain();
			if (this->flag_debug)
			{
				ofstream_log<<"threshold2BinaryForMain succeed!"<<endl;
			}
			if (this->int_largeRegionVolumeByExemplar > this->int_maxRegionVolumeByExemplar)
			{
				this->int_regionVolumeUpperBound =this->int_largeRegionVolumeByExemplar;
			}
			else
			{
				this->int_regionVolumeUpperBound =this->int_maxRegionVolumeByExemplar;
			}
			if (this->int_smallRegionVolumeByExemplar > 0)
			{
				if (this->int_smallRegionVolumeByExemplar < this->int_minRegionVolumeByExemplar)
				{
					this->int_regionVolumeLowerBound = this->int_smallRegionVolumeByExemplar;
				}
				else
				{
					this->int_regionVolumeLowerBound = int_meanRegionVolumeByExemplar*const_double_regionVolumeLowerBoundLoosenBy;
				}
			}
			else
			{
				this->int_regionVolumeLowerBound = int_meanRegionVolumeByExemplar*const_double_regionVolumeLowerBoundLoosenBy;
			}
			if (this->flag_debug)
			{
				ofstream_log.close();
			}
		}

		static unsigned char* vctList2Image1D(vector<vector<V3DLONG> > vctList_input, V3DLONG count_inputVectorLength)
		{
			unsigned char* Image1D_result = new unsigned char[count_inputVectorLength];
			memset(Image1D_result, 0, count_inputVectorLength);
			for (int i=0;i<vctList_input.size();i++)
			{
				for (int j=0;j<vctList_input[i].size();j++)
				{
					Image1D_result[vctList_input[i][j]] = i+1;
				}
			}
			return Image1D_result;
		}

		void regionFitting()
		{
			ofstream ofstream_log;
			if (this->flag_debug)
			{
				ofstream_log.open ("log_regionFitting.txt");
			}
			V3DLONG idx_tmp;
			V3DLONG int_value;
			V3DLONG count_hit = 0;
			V3DLONG count_miss = 0;
			V3DLONG idx_hitExemplar = 0;
			V3DLONG int_label;
			V3DLONG count_exemplarRegion = this->vctList_exemplarRegionIndex.size();
			this->vctList_regionFitIndex.clear();
			vector<V3DLONG> vct_tempResult (0, 0);
			V3DLONG count_grownRegion = this->vctList_grownRegionIndex.size();
			for (int idx_grownRegion=0;idx_grownRegion<count_grownRegion;idx_grownRegion++)
			{
				if (this->flag_debug)
				{
					ofstream_log<<"idx_grownRegion: "<<idx_grownRegion<<endl;
				}
				int_label = idx_grownRegion+1;
				if (this->flag_debug)
				{
					ofstream_log<<"int_label: "<<int_label<<endl;
				}
				if (this->vct_grownRegionVolume[idx_grownRegion]<(this->int_regionVolumeUpperBound))
				{
					//already good enough, scoop it out, store as new segment;
					vct_tempResult.clear();
					for (int i=0;i<vct_grownRegionVolume[idx_grownRegion];i++)
					{
						idx_tmp = this->vctList_grownRegionIndex[idx_grownRegion][i];
						Image1D_mask[idx_tmp]=0; //scooped;
						vct_tempResult.push_back(idx_tmp);
					}
					this->vctList_regionFitIndex.push_back(vct_tempResult);
				}
				else //too large;
				{
					for (int idx_grownRegionVoxel=0;idx_grownRegionVoxel<this->vct_grownRegionVolume[idx_grownRegion];idx_grownRegionVoxel++)
					{
						if (this->flag_debug)
						{
							ofstream_log<<"idx_grownRegionVoxel: "<<idx_grownRegionVoxel<<endl;
						}
						idx_tmp = this->vctList_grownRegionIndex[idx_grownRegion][idx_grownRegionVoxel];
						if (this->flag_debug)
						{
							ofstream_log<<"idx_tmp: "<<idx_tmp<<endl;
						}
						if ((idx_tmp<0) || (idx_tmp>=this->count_totalPageSize))
						{
							v3d_msg("Waring: error occured during examination of grown regions. Please terminate the program immediately!");
							return;
						}
						if (this->flag_debug)
						{
							ofstream_log<<"hit if (this->Image1D_mask[idx_tmp] > 0)"<<endl;
						}
						if (this->Image1D_mask[idx_tmp] > 0) //available;
						{
							if (this->flag_debug)
							{
								ofstream_log<<"if (this->Image1D_mask[idx_tmp] > 0): true"<<endl;
							}
							for (V3DLONG idx_exemplarRegion=0;idx_exemplarRegion<count_exemplarRegion;idx_exemplarRegion++)
							{
								if (this->flag_debug)
								{
									ofstream_log<<"idx_exemplarRegion: "<<idx_exemplarRegion<<endl;
								}
								count_hit = 1;
								count_miss = 0;
								vct_tempResult.clear();
								vct_tempResult.push_back(idx_tmp);
								for (V3DLONG idx_exemplarRegionVoxel=0;idx_exemplarRegionVoxel<this->vct_exemplarRegionVolume[idx_exemplarRegion];idx_exemplarRegionVoxel++)
								{
									if (this->flag_debug)
									{
										ofstream_log<<"idx_exemplarRegionVoxel: "<<idx_exemplarRegionVoxel<<endl;
									}
									idx_hitExemplar = idx_tmp + vctList_exemplarRegionCentralizedIndex[idx_exemplarRegion][idx_exemplarRegionVoxel];
									if (this->flag_debug)
									{
										ofstream_log<<"idx_hitExemplar: "<<idx_hitExemplar<<endl;
										ofstream_log<<"hit if ((idx_hitExemplar<0)||(idx_hitExemplar>=this->count_totalPageSize))"<<endl;
									}
									if ((idx_hitExemplar<0)||(idx_hitExemplar>=this->count_totalPageSize))
									{
										if (this->flag_debug)
										{
											ofstream_log<<"if ((idx_hitExemplar<0)||(idx_hitExemplar>=this->count_totalPageSize)): false"<<endl;
											//it's out of bounds;
										}
									}
									else
									{
										if (this->flag_debug)
										{
											ofstream_log<<"if ((idx_hitExemplar<0)||(idx_hitExemplar>=this->count_totalPageSize)): true"<<endl;
											ofstream_log<<"hit if ((this->Image1D_mask[idx_hitExemplar]>0)&&(this->Image1D_resultFloat[idx_hitExemplar]<(int_label+1))&&(this->Image1D_resultFloat[idx_hitExemplar]>(int_label-1)))"<<endl;
										}
										int_value = this->Image1D_page[idx_hitExemplar];
										if ((this->Image1D_mask[idx_hitExemplar]>0)&&(int_value<(int_label+1))&&(int_value>(int_label-1))) //available and belonging to the same segments by regionGrowing;
										{
											if (this->flag_debug)
											{
												ofstream_log<<"if ((this->Image1D_mask[idx_hitExemplar]>0)&&(this->Image1D_resultFloat[idx_hitExemplar]<(int_label+1))&&(this->Image1D_resultFloat[idx_hitExemplar]>(int_label-1))): true"<<endl;
											}
											count_hit++;
											vct_tempResult.push_back(idx_hitExemplar);
										}
										else
										{
											count_miss++;
										}
									}
									if (count_miss > (this->vct_exemplarRegionVolume[idx_exemplarRegion]*(1-const_double_regionFittingTolerateLevel)))
									{
										break;
									}
								}
								if (this->flag_debug)
								{
									ofstream_log<<"count_hit: "<<count_hit<<endl;
								}
								if (count_hit >= (this->vct_exemplarRegionVolume[idx_exemplarRegion]*const_double_regionFittingTolerateLevel))
								{
									//Scoop it out, store as new segment;
									this->vctList_regionFitIndex.push_back(vct_tempResult);
									for (int i=0;i<count_hit;i++)
									{
										Image1D_mask[vct_tempResult[i]]=0; //scooped;
									}
									break;
								}
							}
						}
					}
				}
			}

			v3d_msg(QString("segmentation complete, total regions: %1").arg(this->vctList_regionFitIndex.size()));
			if (this->flag_debug)
			{
				ofstream_log.close();
			}
		}


		static vector<vector<V3DLONG> > regionFitting(unsigned char* Image1D_input, V3DLONG count_inputImageVectorLength, vector<vector<V3DLONG> > vctList_exemplarInput, vector<vector<V3DLONG> > vctList_regionFitInput)
		{
			vector<vector<V3DLONG> > vctList_regionFitOutput = vctList_regionFitInput;
			ofstream ofstream_log;
			V3DLONG count_hit = 0;
			V3DLONG count_miss = 0;
			V3DLONG idx_hitExemplar = 0;
			V3DLONG count_exemplarRegion = vctList_exemplarInput.size();
			vector<V3DLONG> vct_tempResult (0, 0);
			for (V3DLONG idx_voxel=0;idx_voxel<count_inputImageVectorLength;idx_voxel++)
			{	
				if (Image1D_input[idx_voxel] > 0) //available;
				{
					for (V3DLONG idx_exemplarRegion=0;idx_exemplarRegion<count_exemplarRegion;idx_exemplarRegion++)
					{
						count_hit = 1;
						count_miss = 0;
						vct_tempResult.clear();
						vct_tempResult.push_back(idx_voxel);
						for (V3DLONG idx_exemplarRegionVoxel=0;idx_exemplarRegionVoxel<vctList_exemplarInput[idx_exemplarRegion].size();idx_exemplarRegionVoxel++)
						{
							idx_hitExemplar = idx_voxel + vctList_exemplarInput[idx_exemplarRegion][idx_exemplarRegionVoxel];
							if ((idx_hitExemplar<0)||(idx_hitExemplar>=count_inputImageVectorLength))
							{
								//it's out of bounds;
							}
							else
							{
								if (Image1D_input[idx_hitExemplar]>0)
								{
									count_hit++;
									vct_tempResult.push_back(idx_hitExemplar);
								}
								else
								{
									count_miss++;
								}
							}
							if (count_miss > (vctList_exemplarInput[idx_exemplarRegion].size()*(1-const_double_regionFittingTolerateLevel*0.8)))
							{
								break;
							}
						}
						if (count_hit >= (vctList_exemplarInput[idx_exemplarRegion].size()*const_double_regionFittingTolerateLevel*0.8))
						{
							//Scoop it out, store as new segment;
							vctList_regionFitOutput.push_back(vct_tempResult);
							for (int i=0;i<count_hit;i++)
							{
								Image1D_input[vct_tempResult[i]]=0; //scooped;
							}
							break;
						}
					}
				}

			}
			v3d_msg(QString("segmentation complete, total regions: %1").arg(vctList_regionFitOutput.size()));
			return vctList_regionFitOutput;
		}

		#pragma region "exemplar" 
		vector<vector<V3DLONG> > centralizeExemplarRegionNoWeight(vector<vector<V3DLONG> > vctList_input) //no weight so it can be static;
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
						vct_xyz = convertCoordiante1DTo3D(idx_tmp);
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
					vct_xyz = convertCoordiante1DTo3D(idx_tmp);
					x = vct_xyz[0] - centerX;
					y = vct_xyz[1] - centerY;
					z = vct_xyz[2] - centerZ;
					vctList_ouput[idx_region][idx_voxel] = convertCoordinateToIndex(x, y, z);
				}
			}
			return vctList_ouput;
		}

		void centralizeExemplarRegion()
		{
			this->vctList_exemplarRegionCentralizedIndex = this->vctList_exemplarRegionIndex;
			vct_exemplarRegionMassCenterIndex.clear();
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
			V3DLONG count_region = vctList_exemplarRegionIndex.size();
			V3DLONG count_voxel = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				double_weightedX = 0;
				double_weightedY = 0;
				double_weightedZ = 0;
				double_massSum = 0;
				count_voxel = vctList_exemplarRegionIndex[idx_region].size();
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					int_indexTmp = vctList_exemplarRegionIndex[idx_region][idx_voxel];
					if ((int_indexTmp>0)&&(int_indexTmp<this->count_totalPageSize))
					{
						double_valueTmp = (double) this->Image1D_page[int_indexTmp]; //changed to main as we want the real value rather than the binarized one;
						vct_xyz = convertCoordiante1DTo3D(int_indexTmp);
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
				vct_exemplarRegionMassCenterIndex.push_back(convertCoordinateToIndex((V3DLONG)double_weightedX, (V3DLONG)double_weightedY, (V3DLONG)double_weightedZ));
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					int_indexTmp = vctList_exemplarRegionCentralizedIndex[idx_region][idx_voxel];
					vct_xyz = convertCoordiante1DTo3D(int_indexTmp);
					x = vct_xyz[0] - double_weightedX;
					y = vct_xyz[1] - double_weightedY;
					z = vct_xyz[2] - double_weightedZ;
					vctList_exemplarRegionCentralizedIndex[idx_region][idx_voxel] = convertCoordinateToIndex(x, y, z);
				}
			}
		}

		void regonGrowOnExemplar() //done;
		{
			ofstream ofstream_log;
			if (this->flag_debug)
			{
				ofstream_log.open ("log_regonGrowOnExemplar.txt");
			}
			vector<V3DLONG> vct_temp;
			V3DLONG idx_exemplar;
			if (this->flag_debug)
			{
				ofstream_log<<"vct_exemplarIndex.size(): "<<vct_exemplarIndex.size()<<endl;
			}
			for (V3DLONG i=0;i<this->vct_exemplarIndex.size();i++)
			{
				idx_exemplar = this->vct_exemplarIndex[i];
				vct_temp = this->regionGrowOnSeed(idx_exemplar);
				if (this->flag_debug)
				{
					ofstream_log<<"at i==: "<<i<<"vct_temp.size(): "<<vct_temp.size()<<endl;
				}
				if (!vct_temp.empty())
				{
					this->vctList_exemplarRegionIndex.push_back(vct_temp);
				}
			}
			V3DLONG count_region = this->vctList_exemplarRegionIndex.size();
			if (this->flag_debug)
			{
				ofstream_log<<"count_region: "<<count_region<<endl;
			}
			V3DLONG count_voxel = 0;
			V3DLONG idx_tmp = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = this->vctList_exemplarRegionIndex[idx_region].size();
				if (this->flag_debug)
				{
					ofstream_log<<"at idx_region: "<<idx_region<<"count_voxel: "<<count_voxel<<endl;
				}
				for (V3DLONG idx_voxel=0;idx_voxel<count_voxel;idx_voxel++)
				{
					idx_tmp = this->vctList_exemplarRegionIndex[idx_region][idx_voxel];
					if (this->flag_debug)
					{
						ofstream_log<<"at idx_voxel: "<<idx_voxel<<" idx_tmp: "<<idx_tmp<<endl;
					}
					if ((idx_tmp>=0)&&(idx_tmp<this->count_totalPageSize))
					{
						this->Image1D_exemplarRegion[idx_tmp] = this->Image1D_page[idx_tmp];
					}
				}
			}
		}

		vector<V3DLONG> regionGrowOnSeed(V3DLONG idx_seed) //done;
		//assuming [0-const_double_maxVoxelIntensity] integer Image1D_page, should also work on binary image (not tested yet);
		{
			ofstream ofstream_log;
			if (this->flag_debug)
			{
				ofstream_log.open ("log_regionGrowOnSeed.txt");
			}
			vector<V3DLONG> vct_result;
			vector<V3DLONG> vct_temp;
			V3DLONG idx_current;
			V3DLONG idx_neighbor;
			double double_neightborValue;
			double double_currentValue;
			double double_meanValue = 0;
			double double_sumValue = 0;
			if ((idx_seed<0)||(idx_seed>=this->count_totalPageSize)) //unlikely, but just in case;
			{
				if (this->flag_debug)
				{
					ofstream_log.close();
				}
				return vct_result;
			}
			if (this->flag_debug)
			{
				ofstream_log<<"idx_seed: "<<idx_seed<<endl;
			}
			double double_seedValue = this->Image1D_page[idx_seed];
			V3DLONG count_totalVolume = 0;
			if (double_seedValue>const_int_minimumIntensityConsideredAsNonZero)
			{
				vct_temp.push_back(idx_seed);
				vct_result.push_back(idx_seed);
				double_sumValue = double_seedValue;
				double_meanValue = double_seedValue;
				for (V3DLONG i=0;i<const_int_maximumGrowingLength;i++)
				{
					if (vct_temp.empty())
					{
						if (this->flag_debug)
						{
							ofstream_log.close();
						}
						return vct_result;
					}
					idx_current = vct_temp.back();
					vct_temp.pop_back();
					if (this->flag_debug)
					{
						ofstream_log<<"idx_current: "<<idx_current<<endl;
					}
					if ((idx_current<0)||(idx_current>=this->count_totalPageSize)) //unlikely, but just in case;
					{
						continue;
					}
					double_currentValue = this->Image1D_page[idx_current];
					if (this->flag_debug)
					{
						ofstream_log<<"double_currentValue: "<<double_currentValue<<endl;
					}
					for (V3DLONG j=0;j<const_int_count_neighbors;j++)
					{
						idx_neighbor = idx_current+vct_neighbors[j];
						if (this->flag_debug)
						{
							ofstream_log<<"idx_neighbor: "<<idx_neighbor<<endl;
						}
						if ((idx_neighbor>=0)&&(idx_neighbor<count_totalPageSize)) //prevent it from going out of bounds;
						{
							if (this->Image1D_mask[idx_neighbor]>0) //available only;
							{
								double_neightborValue = Image1D_page[idx_neighbor];
								if ((double_neightborValue > double_meanValue*const_double_exemplarRegionThresholdLoosenBy) && ((abs(double_neightborValue-double_currentValue))<double_meanValue*const_double_valueChangeThreshold))
								{
									this->Image1D_mask[idx_neighbor] = 0; //scooped;
									vct_temp.push_back(idx_neighbor);
									vct_result.push_back(idx_neighbor);
									double_sumValue = double_sumValue + double_neightborValue;
									count_totalVolume++;
									if (this->flag_debug)
									{
										ofstream_log<<"count_totalVolume: "<<count_totalVolume<<endl;
									}
									double_meanValue = double_sumValue/count_totalVolume;
								}
							}
						}
					}
				}
				if (this->flag_debug)
				{
					ofstream_log.close();
				}
				return vct_result;
			}
			else
			{
				if (this->flag_debug)
				{
					ofstream_log.close();
				}
				return vct_result;
			}
		}
		
		void threshold2BinaryForExemplarRegion() //done;
		{
			double double_valueTmp;
			V3DLONG idx_tmp;
			vector<V3DLONG> vct_empty (0,0);
			V3DLONG count_voxel = 0;
			V3DLONG count_region = vctList_exemplarRegionIndex.size();
			this->int_maxRegionVolumeByExemplar = -INF;
			this->int_minRegionVolumeByExemplar = INF;
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
			this->vct_exemplarRegionVolume.clear();
			count_region = vctList_exemplarRegionIndex.size();
			double double_meanVolume = 0;
			double double_sumVolume = 0;
			double double_stdevVolume = 0;
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = vctList_exemplarRegionIndex[idx_region].size();
				if (count_voxel > const_double_fragmentRatioToPageSizeWarning*this->count_totalPageSize)
				{
					v3d_msg("Warning: abnormal large exemplar region detected, program results may become unstable. Please re-select the exemplar(s)!");
				}
				this->vct_exemplarRegionVolume.push_back(count_voxel);
				double_sumVolume += count_voxel;
			}
			double_meanVolume = double_sumVolume/(count_region+1);
			for (V3DLONG idx_region=0;idx_region<count_region;idx_region++)
			{
				count_voxel = vctList_exemplarRegionIndex[idx_region].size();
				double_stdevVolume += pow((count_voxel-double_meanVolume), 2);
				if (this->int_minRegionVolumeByExemplar > count_voxel)
				{
					this->int_minRegionVolumeByExemplar = count_voxel;
				}
				if (this->int_maxRegionVolumeByExemplar < count_voxel)
				{
					this->int_maxRegionVolumeByExemplar = count_voxel;
				}
			}
			double_stdevVolume = double_stdevVolume/count_region;
			double_stdevVolume = pow(double_stdevVolume, 0.5);
			this->int_smallRegionVolumeByExemplar = double_meanVolume-double_stdevVolume*const_double_distributionStdevMultiplier;
			this->int_largeRegionVolumeByExemplar = double_meanVolume+double_stdevVolume*const_double_distributionStdevMultiplier;
			this->int_meanRegionVolumeByExemplar = double_meanVolume;
		}
		#pragma endregion
		
		vector<V3DLONG> landMarkList2IndexList(LandmarkList LandmarkList_Input)
		{
			vector<V3DLONG> vct_result;
			for (V3DLONG idx_input=0;idx_input<LandmarkList_Input.count();idx_input++)
			{
				vct_result.push_back(get1DIndexAtLandmark(LandmarkList_Input.at(idx_input)));
			}
			return vct_result;
		}

		V3DLONG get1DIndexAtLandmark(LocationSimple Landmark_Input)
		{
			float x=0;
			float y=0;
			float z=0;
			Landmark_Input.getCoord(x, y, z);
			return (convertCoordinateToIndex(x-1, y-1, z-1));
		}
	
		vector<V3DLONG> convertCoordiante1DTo3D(V3DLONG idx)
		{
			vector<V3DLONG> vct_coordinate3D (3, -1);
			if (idx > -1)
			{
				vct_coordinate3D[2] = floor((double)idx/(double)offset_Z);
				vct_coordinate3D[1] = floor((double)(idx-vct_coordinate3D[2]*offset_Z)/(double)offset_Y);
				vct_coordinate3D[0] = idx- vct_coordinate3D[2]*offset_Z-vct_coordinate3D[1]*offset_Y;
			}
			return vct_coordinate3D;
		}

		V3DLONG convertCoordinateToIndex(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return z*offset_Z+y*offset_Y+x;
		}

		double getValueAtCoordiante(V3DLONG x, V3DLONG y, V3DLONG z)
		{
			return (Image1D_page[convertCoordinateToIndex(x, y, z)]);
		}

		V3DLONG threshold2BinaryForMain()
		{
			V3DLONG count_totalWhite = 0;
			for(V3DLONG i=0; i<this->count_totalPageSize; i++)
			{	
				if ((double)Image1D_page[i]>this->double_threshold)
				{
					Image1D_page[i] = const_double_maxVoxelIntensity;
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

		#pragma region "locate voxels in image with top voxel value (i.e. intensity)"
		

		#pragma endregion
		
		#pragma region "methods to estimate threshold"
		void estimateThreshold()
		{
			this->histo_main = class_segmentationMain::getHistogramFromImage1D(this->Image1D_page, this->count_totalPageSize);

			this->double_threshold = class_segmentationMain::estimateThresholdYen(histo_main);
			if (this->double_threshold < const_int_minimumIntensityConsideredAsNonZero)
			{
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
		#pragma endregion

		bool regionGrow()
		{	
			this->vct_grownRegionVolume.clear();
			this->vctList_grownRegionIndex.clear();
			RgnGrow3dClass * pRgnGrow = new RgnGrow3dClass;
			if (!pRgnGrow)
			{
				v3d_msg("Fail to allocate memory for RgnGrow3dClass(), program canceled!");
				return false;
			}
			pRgnGrow->ImgDep = this->dim_Z;
			pRgnGrow->ImgHei = this->dim_Y;
			pRgnGrow->ImgWid = this->dim_X;
			if (newIntImage3dPairMatlabProtocol(pRgnGrow->quantImg3d,pRgnGrow->quantImg1d,pRgnGrow->ImgDep,pRgnGrow->ImgHei,pRgnGrow->ImgWid)==0) return false;
			int nstate;
			UBYTE minlevel,maxlevel;
			copyvecdata((unsigned char *)this->Image1D_page,this->count_totalPageSize,pRgnGrow->quantImg1d,nstate,minlevel,maxlevel);
			minlevel = minlevel+1;
			if (minlevel>maxlevel)
				minlevel = maxlevel;
			if (newIntImage3dPairMatlabProtocol(pRgnGrow->PHCLABELSTACK3d,pRgnGrow->PHCLABELSTACK1d,1,3,this->count_totalPageSize)==0) return false;
			pRgnGrow->PHCLABELSTACKPOS = 0;
			pRgnGrow->PHCURGN = new RGN;
			if (!pRgnGrow->PHCURGN)
			{
				v3d_msg("Unable to do:pRgnGrow->PHCURGN = new RGN, program canceled!");
				return false;
			}
			pRgnGrow->PHCURGN_head = pRgnGrow->PHCURGN;
			pRgnGrow->TOTALRGNnum = 1;
			if (pRgnGrow->PHCDONEIMG1d) {delete pRgnGrow->PHCDONEIMG1d;pRgnGrow->PHCDONEIMG1d=0;}
			if (pRgnGrow->PHCDONEIMG3d) {delete pRgnGrow->PHCDONEIMG3d;pRgnGrow->PHCDONEIMG3d=0;}

			if (newIntImage3dPairMatlabProtocol(pRgnGrow->PHCDONEIMG3d,pRgnGrow->PHCDONEIMG1d,pRgnGrow->ImgDep,pRgnGrow->ImgHei,pRgnGrow->ImgWid)==0) return false;
			for(int j=minlevel;j<=maxlevel;j++)
			{
				int depk, colj, rowi;
				BYTE * PHCDONEIMG1d = pRgnGrow->PHCDONEIMG1d;
				UBYTE * quantImg1d = pRgnGrow->quantImg1d;
				BYTE *** flagImg = pRgnGrow->PHCDONEIMG3d;
				for (V3DLONG tmpi=0; tmpi<this->count_totalPageSize; tmpi++)
				{
					PHCDONEIMG1d[tmpi] = (quantImg1d[tmpi]==(UBYTE)j)?1:0;
				}
				pRgnGrow->PHCURLABEL = 0;
				for(depk=0; depk<pRgnGrow->ImgDep; depk++)
				{
					for(colj=0; colj<pRgnGrow->ImgHei; colj++)
					{
						for(rowi=0; rowi<pRgnGrow->ImgWid; rowi++)
						{
							if (flagImg[depk][colj][rowi]==1)
							{
								pRgnGrow->IFINCREASELABEL = 1;
								pRgnGrow->PHCURLABEL++;
								pRgnGrow->PHCLABELSTACKPOS = 0;
								pRgnGrow->PHCLABELSTACK3d[0][0][pRgnGrow->PHCLABELSTACKPOS] = depk;
								pRgnGrow->PHCLABELSTACK3d[0][1][pRgnGrow->PHCLABELSTACKPOS] = colj;
								pRgnGrow->PHCLABELSTACK3d[0][2][pRgnGrow->PHCLABELSTACKPOS] = rowi;
								pRgnGrow->PHCURGNPOS = new POS;
								if (pRgnGrow->PHCURGNPOS==0)
								{
									v3d_msg("Fail to allocate memory for PHCURGNPOS, program canceled!");
									return false;
								}
								pRgnGrow->PHCURGNPOS_head = pRgnGrow->PHCURGNPOS;
								pRgnGrow->TOTALPOSnum = 1;
								while(1)
								{
									pRgnGrow->IFINCREASELABEL = 1;
									V3DLONG posbeg = pRgnGrow->PHCLABELSTACKPOS;
									V3DLONG mypos = posbeg;
									while (mypos>=0)
									{
										pRgnGrow->STACKCNT = 0;
										int curdep = pRgnGrow->PHCLABELSTACK3d[0][0][mypos];
										int curcol = pRgnGrow->PHCLABELSTACK3d[0][1][mypos];
										int currow = pRgnGrow->PHCLABELSTACK3d[0][2][mypos];
										if (flagImg[curdep][curcol][currow]==1)
										{
											rgnfindsub(currow,curcol,curdep,0,1,pRgnGrow);
										}
										else if(flagImg[curdep][curcol][currow]==-1)
										{
											rgnfindsub(currow,curcol,curdep,0,0,pRgnGrow);
										}
										V3DLONG posend = pRgnGrow->PHCLABELSTACKPOS;
										if (posend>posbeg)
										{mypos = pRgnGrow->PHCLABELSTACKPOS;}
										else
										{mypos = mypos-1;}
										posbeg = posend;
									}
									if (pRgnGrow->IFINCREASELABEL==1)
										break;
								}
								pRgnGrow->PHCURGN->layer = j;
								pRgnGrow->PHCURGN->no = pRgnGrow->PHCURLABEL;
								pRgnGrow->PHCURGN->poslist = pRgnGrow->PHCURGNPOS_head;
								pRgnGrow->PHCURGN->poslistlen = pRgnGrow->TOTALPOSnum;
								pRgnGrow->TOTALPOSnum = 0;
								pRgnGrow->PHCURGN->next = new RGN;
								if(pRgnGrow->PHCURGN->next==0)
								{
									v3d_msg("Fail to do --> pRgnGrow->PHCURGN->next = new RGN, program canceled!");
									return false;
								}
								pRgnGrow->PHCURGN = pRgnGrow->PHCURGN->next;
								pRgnGrow->TOTALRGNnum++;
							}
						}
					}
				}
			}

			STCL *staRegion = new STCL;
			STCL *staRegion_begin = staRegion;
			RGN *curRgn = pRgnGrow->PHCURGN_head;
			V3DLONG nrgncopied = 0;
			std::vector<STCL> stclList;
			while(curRgn && curRgn->next)
			{
				staRegion->no = curRgn->no;
				staRegion->count = 0;
				POS * curPos = curRgn->poslist;
				V3DLONG count = 0;
				staRegion->desposlist = new V3DLONG [curRgn->poslistlen-1];
				while(curPos && curPos->next)
				{
					staRegion->desposlist[count++] = curPos->pos;
					curPos = curPos->next;
				}
				staRegion->count = count;
				if(count<int_regionVolumeLowerBound)
				{
					nrgncopied++;
					curRgn = curRgn->next;
					for (int i=0;i<count;i++)
					{
						Image1D_mask[staRegion->desposlist[i]] = 0; //invalid;
						Image1D_page[staRegion->desposlist[i]] = 0;
					}
					continue; 
				}
				stclList.push_back(*staRegion);
				curRgn = curRgn->next;
				staRegion->next = new STCL;
				staRegion = staRegion->next;
				nrgncopied++;
			}
			V3DLONG length;
			V3DLONG n_rgn = stclList.size();
			
			float *pRGCL = new float [this->count_totalPageSize];
			memset(pRGCL, 0, sizeof(float)*this->count_totalPageSize);
			for(int ii=0; ii<n_rgn; ii++)
			{
				length = stclList.at(ii).count;
				V3DLONG *cutposlist = stclList.at(ii).desposlist;
				float scx=0,scy=0,scz=0,si=0;
				for(int i=0; i<length; i++)
				{
					pRGCL[ cutposlist[i] ] = (float)ii + 1.0;
					float cv = Image1D_page[ cutposlist[i] ];
					V3DLONG idx = cutposlist[i];
					V3DLONG k1 = idx/(dim_X*dim_Y);
					V3DLONG j1 = (idx - k1*dim_X*dim_Y)/dim_X;
					V3DLONG i1 = idx - k1*dim_X*dim_Y - j1*dim_X;
					scz += k1*cv;
					scy += j1*cv;
					scx += i1*cv;
					si += cv;
				}
				if (si>0)
				{
					this->vct_grownRegionVolume.push_back(length);
					this->vctList_grownRegionIndex.push_back(cutposlist);
				}
			}
			for (int i=0;i<this->count_totalPageSize;i++)
			{
				Image1D_page[i] = (unsigned char)pRGCL[i];
			}


			
			if (pRgnGrow->quantImg1d) {delete pRgnGrow->quantImg1d;pRgnGrow->quantImg1d=0;}
			if (pRgnGrow->quantImg3d) {delete pRgnGrow->quantImg3d;pRgnGrow->quantImg3d=0;}
			if (pRgnGrow->PHCLABELSTACK1d) {delete pRgnGrow->PHCLABELSTACK1d;pRgnGrow->PHCLABELSTACK1d=0;}
			if (pRgnGrow->PHCLABELSTACK3d) {delete pRgnGrow->PHCLABELSTACK3d;pRgnGrow->PHCLABELSTACK3d=0;}
			if (pRgnGrow->PHCDONEIMG3d) {delete pRgnGrow->PHCDONEIMG3d;pRgnGrow->PHCDONEIMG3d=0;}
			if (pRgnGrow->PHCDONEIMG1d) {delete pRgnGrow->PHCDONEIMG1d;pRgnGrow->PHCDONEIMG1d=0;}
			return true;
		}

		V3DLONG removeSingleVoxel()
		{
			V3DLONG count_voxelRemoved = 0;
			
			for(V3DLONG k = 0; k < dim_Z; k++)
			{
				V3DLONG idxk = k*this->offset_Z;
				for(V3DLONG j = 0;  j < dim_Y; j++)
				{
					V3DLONG idxj = idxk + j*this->offset_Y;
					for(V3DLONG i = 0, idx = idxj; i < dim_X;  i++, idx++)
					{
						if(i==0 || i==dim_X-1 || j==0 || j==dim_Y-1 || k==0 || k==dim_Z-1)
							continue;
						if(Image1D_page[idx])
						{
							bool one_point = true;
							for(V3DLONG ineighbor=0; ineighbor<const_int_count_neighbors; ineighbor++)
							{
								V3DLONG n_idx = idx + vct_neighbors[ineighbor];
								if (n_idx>0 && n_idx<this->count_totalPageSize)
								{
									if(Image1D_page[n_idx])
									{
										one_point = false;
										break;
									}
								}
							}
							if(one_point==true)
							{
								count_voxelRemoved = count_voxelRemoved + 1;
								Image1D_page[idx] = 0;
								Image1D_mask[idx] = 0; //invalid;
							}
						}
					}
				}
			}
			return count_voxelRemoved;
		}
		
		
	#pragma region private
	private: 
		class POS
		{
		public:
			V3DLONG pos;
			V3DLONG order;
			POS * next;
			POS()
			{
				pos = -1;order=-1;
				next = 0;
			}
			~POS()
			{
				
			}
		};

		class RGN
		{
		public:
			V3DLONG layer;
			V3DLONG no;
			POS *poslist;
			V3DLONG poslistlen;
			RGN * next;
			RGN()
			{
				layer=no=-1;
				poslistlen=0;poslist=0;
				next=0;
			}
			~RGN()
			{
				layer=no=-1;
				poslistlen = 0;
			}
		};

		//statistics of count of labeling
		class STCL
		{
		public:

			V3DLONG count;
			V3DLONG no;
			V3DLONG *desposlist;
			STCL *next;

			STCL()
			{
				count=no=-1;
				next=0;
			}
			~STCL()
			{
				count=no=-1;
			}
		};

		//function of swap
		template <class T>
		void swap (T& x, T& y)
		{
			T tmp = x;	x = y; y = tmp;
		}

		//function of quickSort
		template <class T>
		void quickSort(T a[], int l, int r)
		{
			if(l>=r) return;
			int i = l;
			int j = r+1;

			T pivot = a[l];
			while(true)
			{
				do{ i = i+1; } while(a[i]>pivot);
				do{ j = j-1; } while(a[j]<pivot);
				if(i >= j) break;
				swap(a[i], a[j]);
			}
			a[l] = a[j];
			a[j] = pivot;
			quickSort(a, l, j-1);
			quickSort(a, j+1, r);
		}

		//memory management
		template <class T> int newIntImage3dPairMatlabProtocol(T *** & img3d,T * & img1d, V3DLONG imgdep, V3DLONG imghei,V3DLONG imgwid)
		{
			V3DLONG totalpxlnum = imghei*imgwid*imgdep;
			try
			{
				img1d = new T [totalpxlnum];
				img3d = new T ** [imgdep];

				V3DLONG i,j;

				for (i=0;i<imgdep;i++)
				{
					img3d[i] = new T * [imghei];
					for(j=0; j<imghei; j++)
						img3d[i][j] = img1d + i*imghei*imgwid + j*imgwid;

				}

				memset(img1d, 0, sizeof(T)*totalpxlnum);
			}
			catch(...)
			{
				if (img1d) {delete img1d;img1d=0;}
				if (img3d) {delete img3d;img3d=0;}
				printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
				return 0; //fail
			}
			return 1; //succeed
		}

		class RgnGrow3dClass // region growing class
		{
		public:
			RgnGrow3dClass()
			{
				ImgWid = 0, ImgHei = 0, ImgDep = 0;
				quantImg1d=0; quantImg3d=0;
				PHCDONEIMG3d = 0, PHCDONEIMG1d = 0;

				STACKCNT = -1, MAXSTACKSIZE = 16, IFINCREASELABEL=-1, PHCURLABEL=-1;
				PHCLABELSTACK3d = 0, PHCLABELSTACK1d = 0;
				PHCLABELSTACKPOS = 0;

				PHCURGNPOS = 0, PHCURGNPOS_head = 0;
				PHCURGN = 0, PHCURGN_head = 0;
				TOTALPOSnum = 0, TOTALRGNnum = 0;
			}

			~RgnGrow3dClass()
			{
				if (quantImg1d) {delete quantImg1d;quantImg1d=0;}
				if (quantImg3d) {delete quantImg3d;quantImg3d=0;}
				if (PHCLABELSTACK1d) {delete PHCLABELSTACK1d;PHCLABELSTACK1d=0;}
				if (PHCLABELSTACK3d) {delete PHCLABELSTACK3d;PHCLABELSTACK3d=0;}
				if (PHCDONEIMG1d) {delete PHCDONEIMG1d;PHCDONEIMG1d=0;}
				if (PHCDONEIMG3d) {delete PHCDONEIMG3d;PHCDONEIMG3d=0;}


				ImgWid = 0, ImgHei = 0, ImgDep = 0;

				STACKCNT = -1, MAXSTACKSIZE = 16, IFINCREASELABEL=-1, PHCURLABEL=-1;
				PHCLABELSTACKPOS = 0;

				PHCURGN = PHCURGN_head;
				for(V3DLONG i=0;i<TOTALRGNnum;i++)
				{
					RGN * pnextRgn = 0;
					if (PHCURGN)
					{
						pnextRgn = PHCURGN->next;
						PHCURGNPOS = PHCURGN->poslist;
						for(V3DLONG j=0;j<PHCURGN->poslistlen;j++)
						{
							POS *pnextPos = 0;
							if (PHCURGNPOS)
							{
								pnextPos = PHCURGNPOS->next;
								delete PHCURGNPOS;
							}
							PHCURGNPOS = pnextPos;
						}
						delete PHCURGN;
					}
					PHCURGN = pnextRgn;
				}
				TOTALPOSnum = 0, TOTALRGNnum = 0;
			}

		public:
			V3DLONG ImgWid, ImgHei, ImgDep;
			UBYTE * quantImg1d,  *** quantImg3d;
			BYTE *** PHCDONEIMG3d, * PHCDONEIMG1d;

			int STACKCNT;
			int MAXSTACKSIZE;
			int IFINCREASELABEL;
			V3DLONG PHCURLABEL;
			int ***PHCLABELSTACK3d, * PHCLABELSTACK1d;
			V3DLONG PHCLABELSTACKPOS;

			POS * PHCURGNPOS, * PHCURGNPOS_head;
			RGN * PHCURGN, * PHCURGN_head;

			V3DLONG TOTALPOSnum, TOTALRGNnum;
		};

		//generating an int image for any input image type
		template <class T> void copyvecdata(T * srcdata, V3DLONG len, UBYTE * desdata, int& nstate, UBYTE &minn, UBYTE &maxx)
		{
			if(!srcdata || !desdata)
			{
				printf("NULL pointers in copyvecdata()!\n");
				return;
			}

			V3DLONG i;
			//copy data
			if (srcdata[0]>0)
				maxx = minn = int(srcdata[0]+0.5);
			else
				maxx = minn = int(srcdata[0]-0.5);

			int tmp;
			double tmp1;
			for (i=0;i<len;i++)
			{
				tmp1 = double(srcdata[i]);
				tmp = (tmp1>0)?(int)(tmp1+0.5):(int)(tmp1-0.5);//round to integers
				minn = (minn<tmp)?minn:tmp;
				maxx = (maxx>tmp)?maxx:tmp;
				desdata[i] = (UBYTE)tmp;
			}
			maxx = (UBYTE)maxx;
			minn = (UBYTE)minn;
			//return the #state
			nstate = (maxx-minn+1);
			return;
		}

		void rgnfindsub(int rowi,int colj, int depk, int direction,int stackinc, RgnGrow3dClass * pRgnGrow)
		{
			if (pRgnGrow->STACKCNT >= pRgnGrow->MAXSTACKSIZE)
			{
				if (pRgnGrow->IFINCREASELABEL != 0)
					pRgnGrow->IFINCREASELABEL = 0;
				return;
			}

			BYTE *** flagImg = pRgnGrow->PHCDONEIMG3d;
			int ImgWid = pRgnGrow->ImgWid;
			int ImgHei = pRgnGrow->ImgHei;
			int ImgDep = pRgnGrow->ImgDep;

			if (stackinc==1)
			{
				pRgnGrow->PHCLABELSTACK3d[0][0][pRgnGrow->PHCLABELSTACKPOS] = depk;
				pRgnGrow->PHCLABELSTACK3d[0][1][pRgnGrow->PHCLABELSTACKPOS] = colj;
				pRgnGrow->PHCLABELSTACK3d[0][2][pRgnGrow->PHCLABELSTACKPOS] = rowi;

				pRgnGrow->STACKCNT++;
				pRgnGrow->PHCLABELSTACKPOS++;

				flagImg[depk][colj][rowi] = -1;
				
				//set the current pos location and return the
				if (pRgnGrow->PHCURGNPOS)
				{
					pRgnGrow->PHCURGNPOS->pos = (V3DLONG) depk*(pRgnGrow->ImgHei * pRgnGrow->ImgWid) + colj*(pRgnGrow->ImgWid) + rowi; //
					pRgnGrow->PHCURGNPOS->next = new POS;
					if (pRgnGrow->PHCURGNPOS->next==0)
					{printf("Fail to do: pRgnGrow->PHCURGNPOS->next = new POS;");}
					pRgnGrow->PHCURGNPOS = pRgnGrow->PHCURGNPOS->next;
					pRgnGrow->TOTALPOSnum++;
				}
				else
				{
					printf("PHCURGNPOS is null!!\n");
				}
			}
			else //%if stackinc==0,
			{
				flagImg[depk][colj][rowi] = -2;
			}
			// % search 26 direction orders
			// 1
			if (rowi>0 && flagImg[depk][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk,1,1,pRgnGrow);
			// 2
			if (rowi<ImgWid-1 && flagImg[depk][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk,1,1,pRgnGrow);
			// 3
			if (colj>0 && flagImg[depk][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk,1,1,pRgnGrow);
			// 4
			if (colj<ImgHei-1 && flagImg[depk][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk,1,1,pRgnGrow);
			// 5
			if (depk>0 && flagImg[depk-1][colj][rowi]==1)
				rgnfindsub(rowi,colj,depk-1,1,1,pRgnGrow);
			// 6
			if (depk<ImgDep-1 && flagImg[depk+1][colj][rowi]==1)
				rgnfindsub(rowi,colj,depk+1,1,1,pRgnGrow);
			// 7
			if (rowi>0 && colj>0 && flagImg[depk][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk,1,1,pRgnGrow);
			// 8
			if (rowi<ImgWid-1 && colj>0 && flagImg[depk][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk,1,1,pRgnGrow);
			// 9
			if (rowi>0 && colj<ImgHei-1 && flagImg[depk][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk,1,1,pRgnGrow);
			// 10
			if (rowi>ImgWid && colj<ImgHei-1 && flagImg[depk][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk,1,1,pRgnGrow);
			// 11
			if (rowi>0 && depk>0 && flagImg[depk-1][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk-1,1,1,pRgnGrow);
			// 12
			if (rowi<ImgWid-1 && depk>0 && flagImg[depk-1][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk-1,1,1,pRgnGrow);
			// 13
			if (rowi>0 && depk<ImgDep-1 && flagImg[depk+1][colj][rowi-1]==1)
				rgnfindsub(rowi-1,colj,depk+1,1,1,pRgnGrow);
			// 14
			if (rowi<ImgWid-1 && depk<ImgDep-1 && flagImg[depk+1][colj][rowi+1]==1)
				rgnfindsub(rowi+1,colj,depk+1,1,1,pRgnGrow);
			// 15
			if (colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk-1,1,1,pRgnGrow);
			// 16
			if (colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk-1,1,1,pRgnGrow);
			// 17
			if (colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi]==1)
				rgnfindsub(rowi,colj-1,depk+1,1,1,pRgnGrow);
			// 18
			if (colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi]==1)
				rgnfindsub(rowi,colj+1,depk+1,1,1,pRgnGrow);
			// 19
			if (rowi>0 && colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk-1,1,1,pRgnGrow);
			// 20
			if (rowi<ImgWid-1 && colj>0 && depk>0 && flagImg[depk-1][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk-1,1,1,pRgnGrow);
			// 21
			if (rowi>0 && colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk-1,1,1,pRgnGrow);
			// 22
			if (rowi>0 && colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi-1]==1)
				rgnfindsub(rowi-1,colj-1,depk+1,1,1,pRgnGrow);
			// 23
			if (rowi<ImgWid-1 && colj<ImgHei-1 && depk>0 && flagImg[depk-1][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk-1,1,1,pRgnGrow);
			// 24
			if (rowi<ImgWid-1 && colj>0 && depk<ImgDep-1 && flagImg[depk+1][colj-1][rowi+1]==1)
				rgnfindsub(rowi+1,colj-1,depk+1,1,1,pRgnGrow);
			// 25
			if (rowi>0 && colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi-1]==1)
				rgnfindsub(rowi-1,colj+1,depk+1,1,1,pRgnGrow);
			// 26
			if (rowi<ImgWid-1 && colj<ImgHei-1 && depk<ImgDep-1 && flagImg[depk+1][colj+1][rowi+1]==1)
				rgnfindsub(rowi+1,colj+1,depk+1,1,1,pRgnGrow);
			return;
		}
		#pragma endregion
};
#pragma endregion

#pragma region "function: func_Image1DVisualization"
void func_Image1DVisualization(unsigned char* Image1D_input, V3DLONG dim_X, V3DLONG dim_Y, V3DLONG dim_Z, int int_channelDim, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QString string_windowName)
{
	Image4DSimple Image4DSimple_temp;
	//V3DLONG count_totalPageSize = dim_X*dim_Z*dim_Z;
	Image4DSimple_temp.setData((unsigned char*)Image1D_input, dim_X, dim_Y, dim_Z, int_channelDim, V3D_UINT8);
	v3dhandle v3dhandle_main = V3DPluginCallback2_currentCallback.newImageWindow();
	V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
	V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, string_windowName);
	V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
}


#pragma endregion

#pragma region "Segmentation interface"
bool func_interface_Segmentation(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
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
	ofstream_log<<"Voxel count (by couting bytes): "<<(count_totalBytes/sizeof(Image4DSimple_current[0]))<<endl;
	//sets data into 1D array, note: size is count_totalVoxelsCurrent;
    unsigned char* Image1D_current = Image4DSimple_current->getRawData();
	//get the currenlty-opened file name
    QString QString_fileNameCurrent = V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
    //defining dimensions
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

    //check data availability;
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
		subFunc_NeuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1)) //Only SWCList_current is not empty;
	{
		subFunc_NeuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}
	if (count_userDefinedLandmarkList>0) //Only LandmarkList_userDefined is not empty;
	{
		LandmarkList_current = LandmarkList_userDefined;
		count_currentLandmarkList = LandmarkList_current.count();
		ofstream_log<<"User-defined landmarks count: "<<count_currentLandmarkList<<endl;
	}

	//Open dialogMain1 window, get paramters;
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
	ofstream_log<<"int_largeRegionVolumeByExemplar: "<<segmentationMain1.int_largeRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_smallRegionVolumeByExemplar: "<<segmentationMain1.int_smallRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_maxRegionVolumeByExemplar: "<<segmentationMain1.int_maxRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_minRegionVolumeByExemplar: "<<segmentationMain1.int_minRegionVolumeByExemplar<<endl;
	ofstream_log<<"int_meanRegionVolumeByExemplar: "<<segmentationMain1.int_meanRegionVolumeByExemplar<<endl;;
	ofstream_log<<"int_regionVolumeLowerBound: "<<segmentationMain1.int_regionVolumeLowerBound<<endl;
	ofstream_log<<"int_regionVolumeUpperBound: "<<segmentationMain1.int_regionVolumeUpperBound<<endl;

	unsigned char* Image1D_mainAfterThreshold = new unsigned char[segmentationMain1.count_totalPageSize];
	for (int i=0;i<segmentationMain1.count_totalPageSize;i++)
	{
		if ((double)segmentationMain1.Image1D_page[i]>0)
		{
			Image1D_mainAfterThreshold[i] = segmentationMain1.Image1D_original[i+segmentationMain1.offset_channel];
		}
	}
	func_Image1DVisualization(Image1D_mainAfterThreshold,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "After thresholding");
	
	ofstream_log<<"segmentationMain1.removeSingleVoxel() start!"<<endl;
	V3DLONG count_voxelRemoved = segmentationMain1.removeSingleVoxel();
	ofstream_log<<"segmentationMain1.removeSingleVoxel() succeed, "<<count_voxelRemoved<<" removed!"<<endl;

	ofstream_log<<"Visualize segmentationMain1.Image1D_exemplarRegion start !"<<endl;
	func_Image1DVisualization(segmentationMain1.Image1D_exemplarRegion,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Exemplar Regions");
	ofstream_log<<"Visualize segmentationMain1.Image1D_exemplarRegion succeed !"<<endl;
	
	ofstream_log<<"segmentationMain1.regionGrow() start!"<<endl;
	if(segmentationMain1.regionGrow())
	{
		ofstream_log<<"segmentationMain1.regionGrow() succeed!"<<endl;
		ofstream_log<<"total regions grown: "<<segmentationMain1.vct_grownRegionVolume.size()<<"!"<<endl;
	}
	else
	{
		ofstream_log<<"segmentationMain1.regionGrow() failed!"<<endl;
		ofstream_log.close();
		return false;
	}
	ofstream_log<<"segmentationMain1.regionFitting() start!"<<endl;
	segmentationMain1.regionFitting();
	ofstream_log<<"segmentationMain1.regionFitting() succeed!"<<endl;
	ofstream_log<<"total regions fit: "<<segmentationMain1.vctList_regionFitIndex.size()<<"!"<<endl;

	ofstream_log<<"Visualize Image1D_firstSegmentationResult start !"<<endl;
	unsigned char* Image1D_firstSegmentationResult = class_segmentationMain::vctList2Image1D(segmentationMain1.vctList_regionFitIndex, segmentationMain1.count_totalPageSize);
	func_Image1DVisualization(Image1D_firstSegmentationResult,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Segmentation Result");
	ofstream_log<<"Visualize Image1D_firstSegmentationResult succeed !"<<endl;

	//2nd-round segmentation (on mask) seems not working well;
	/*ofstream_log<<"segmentationMain1.regionFitting() on mask start!"<<endl;
	vector<vector<V3DLONG> > vctList_newCenter;
	vctList_newCenter = segmentationMain1.centralizeExemplarRegionNoWeight(segmentationMain1.vctList_regionFitIndex); 
	segmentationMain1.vctList_regionFitIndex = segmentationMain1.regionFitting(segmentationMain1.Image1D_mask, segmentationMain1.count_totalPageSize, vctList_newCenter, segmentationMain1.vctList_regionFitIndex);
	unsigned char* Image1D_secondSegmentationResult = class_segmentationMain::vctList2Image1D(segmentationMain1.vctList_regionFitIndex, segmentationMain1.count_totalPageSize);
	ofstream_log<<"segmentationMain1.regionFitting() on mask succeed!"<<endl;
	ofstream_log<<"total regions fit: "<<segmentationMain1.vctList_regionFitIndex.size()<<"!"<<endl;
	ofstream_log<<"Visualize Image1D_secondSegmentationResult start !"<<endl;
	func_Image1DVisualization(Image1D_secondSegmentationResult,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "2nd-round Segmentation Result");
	ofstream_log<<"Visualize Image1D_secondSegmentationResult succeed !"<<endl;
	*/

	ofstream_log<<"Visualize segmentationMain1.Image1D_mask start !"<<endl;
	func_Image1DVisualization(segmentationMain1.Image1D_mask,segmentationMain1.dim_X,segmentationMain1.dim_Y,segmentationMain1.dim_Z,1,V3DPluginCallback2_currentCallback, "Mask");
	ofstream_log<<"Visualize segmentationMain1.Image1D_mask succeed !"<<endl;


	ofstream_log.close();
    return true;
}
#pragma endregion

#pragma region "Utility funcions"
bool subFunc_NeuronTree2LandmarkList(const NeuronTree & NeuronTree_input, LandmarkList & LandmarkList_output)
{
    LocationSimple LocationSimple_temp(0,0,0);
    for (V3DLONG i=0;i<NeuronTree_input.listNeuron.size();i++)
    {
        LocationSimple_temp.x = NeuronTree_input.listNeuron.at(i).x;
        LocationSimple_temp.y = NeuronTree_input.listNeuron.at(i).y;
        LocationSimple_temp.z = NeuronTree_input.listNeuron.at(i).z;
        LandmarkList_output.append(LocationSimple_temp);
    }
    return true;
}

unsigned char* subFunc_Image1DTypeConversion(unsigned short *Image1D_input, V3DLONG count_total)
{
	unsigned char* Image1D_Output = new unsigned char [count_total];
	for (V3DLONG i=0;i<count_total;i++)
	{
		unsigned short int_temp = Image1D_input[i];
		Image1D_Output[i] = int_temp;
	}
	return Image1D_Output;
}

unsigned char* subFunc_Image1DTypeConversion(float *Image1D_input, V3DLONG count_total)
{
	unsigned char* Image1D_Output = new unsigned char [count_total];
	for (V3DLONG i=0;i<count_total;i++)
	{
		float int_temp = Image1D_input[i];
		Image1D_Output[i] = int_temp;
	}
	return Image1D_Output;
}
#pragma endregion
