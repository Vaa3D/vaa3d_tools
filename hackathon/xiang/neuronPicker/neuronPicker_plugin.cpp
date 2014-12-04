	/* neuronPicker_plugin.cpp
 * 
 * 2014-12-01 : by Xiang Li, Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuronPicker_plugin.h"
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
const double const_max_voxelValue = 255;
const int const_count_neighbors = 26; //27 directions -1;
const double const_infinitesimal = 0.000000001;
#define INF 1E9
#define NINF -1E9
#define PI 3.14159265
Q_EXPORT_PLUGIN2(neuronPicker, neuronPicker);
void visualizationImage1D(const unsigned char* _image1D_input, const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z, const V3DLONG _dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, const QString name_window);

#pragma region "neuronPickerMain"
class neuronPickerMain  
{
public:
	neuronPickerMain() {}
	~neuronPickerMain() {}

	#pragma region "memoryManagement"
	static V3DLONG *memory_allocate_uint1D(const V3DLONG i_size)
	{
		V3DLONG *ptr_result;
		ptr_result=(V3DLONG *) calloc(i_size, sizeof(V3DLONG));
		return(ptr_result);
	}
	static void memory_free_uint1D(V3DLONG *ptr_input)
	{
		free(ptr_input);
	}
	static unsigned char *memory_allocate_uchar1D(const V3DLONG i_size)
	{
		unsigned char *ptr_result;
		ptr_result=(unsigned char *) calloc(i_size, sizeof(unsigned char));
		return(ptr_result);
	}
	static void memory_free_uchar1D(unsigned char *ptr_input)
	{
		free(ptr_input);
	}
	#pragma endregion

	#pragma region "geometry related"
	static vector<V3DLONG> getBoundary(V3DLONG* _image1D_input, const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z, vector<vector<V3DLONG> > _pos4s_neighborRelative)
	{
		vector<V3DLONG> poss_boundary;
		V3DLONG size_page=_dim_X*_dim_Y*_dim_Z;
		V3DLONG offset_Y=_dim_X; V3DLONG offset_Z=_dim_X*_dim_Y;
		for (V3DLONG pos_i=0;pos_i<size_page;pos_i++)
		{
			vector<V3DLONG> xyz_i=pos2xyz(pos_i, offset_Y, offset_Z);
			for (V3DLONG idx_neighbor=0;idx_neighbor<const_count_neighbors;idx_neighbor++)
			{
				V3DLONG pos_neighbor=pos_i+ _pos4s_neighborRelative[idx_neighbor][3];
				if (isValid(pos_neighbor, size_page))
				{
					if (((xyz_i[0]+_pos4s_neighborRelative[idx_neighbor][0])<0)||
						((xyz_i[0]+_pos4s_neighborRelative[idx_neighbor][0])>=_dim_X)||
						((xyz_i[1]+_pos4s_neighborRelative[idx_neighbor][1])<0)||
						((xyz_i[1]+_pos4s_neighborRelative[idx_neighbor][1])>=_dim_Y)||
						((xyz_i[2]+_pos4s_neighborRelative[idx_neighbor][2])<0)||
						((xyz_i[2]+_pos4s_neighborRelative[idx_neighbor][2])>=_dim_Z))
					{} //invalide anyway;
					else
					{
						V3DLONG value_neighbor=_image1D_input[pos_neighbor];
						if (value_neighbor<1) { poss_boundary.push_back(pos_i); break;} //it is boundary;
					}
				}
			}
		}
		return poss_boundary;
	}
	static vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
	{
		vector<V3DLONG> pos3_result (3, -1);
		pos3_result[2]=floor(_pos_input/(double)_offset_Z);
		pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
		pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
		return pos3_result;
	}
	static V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
	{
		return _z*_offset_Z+_y*_offset_Y+_x;
	}
	static bool isValid(const V3DLONG _pos_input, const V3DLONG _size_page)
	{
		if ((_pos_input>=0)&&(_pos_input<_size_page)) { return true; }
		else { return false; }
	}
	static void poss2Image1D(const vector<V3DLONG> _poss_input, unsigned char* _image1D_input, const unsigned char value_input)
	{
		V3DLONG size_input=_poss_input.size();
		for (V3DLONG i=0;i<size_input;i++) {_image1D_input[_poss_input[i]]=value_input; }
	}
	static void poss2Image1D(const vector<V3DLONG> _poss_input, unsigned char* _image1D_output, const unsigned char* _image1Dc_input,
		V3DLONG _size_page)
	{
		V3DLONG size_input=_poss_input.size();
		for (V3DLONG i=0;i<size_input;i++)
		{
			V3DLONG pos_i=_poss_input[i];
			V3DLONG value_i=(V3DLONG)(sqrt(((double)_image1Dc_input[pos_i]*(double)_image1Dc_input[pos_i]
			+(double)_image1Dc_input[pos_i+_size_page]*(double)_image1Dc_input[pos_i+_size_page]
			+(double)_image1Dc_input[pos_i+_size_page+_size_page]*(double)_image1Dc_input[pos_i+_size_page+_size_page])/3));
			_image1D_output[_poss_input[i]]=value_i; 
		}
	}
	#pragma endregion

	#pragma region "regionGrow"
	static vector<V3DLONG> doRegionGrow(const unsigned char* _image1Dc_input, const V3DLONG _pos_seed, 
		const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z, V3DLONG _dim_C, unsigned char* _image1D_mask,
		const vector<vector<V3DLONG> > _pos4s_neighborRelative, const double _bandWidth_color, const V3DLONG _tolerance_gap, const vector<V3DLONG> _thresholds_page)
	{
		vector<V3DLONG> poss_result;
		if (_image1D_mask[_pos_seed]<1) {return poss_result;}
		V3DLONG offset_Y=_dim_X; V3DLONG offset_Z=_dim_X*_dim_Y; V3DLONG size_page=_dim_X*_dim_Y*_dim_Z;
		if (!isPassedThreshold(_image1Dc_input, _pos_seed, _thresholds_page, _dim_C, size_page)) {return poss_result;} 
		vector<V3DLONG> poss_growing;
		poss_growing.push_back(_pos_seed);
		poss_result.push_back(_pos_seed);
		vector<V3DLONG> color_seed=getColorFromPos(_image1Dc_input, _pos_seed, size_page, _dim_C);
		vector<V3DLONG> thresholds_page=_thresholds_page;
		_image1D_mask[_pos_seed]=0; //scooped;
		while (true)
		{
			if (poss_growing.empty()) //growing complete;
			{
				return poss_result;
			}
			V3DLONG pos_current=poss_growing.back();
            //vector<V3DLONG> color_current=getColorFromPos(_image1Dc_input, pos_current, size_page, _dim_C);
			poss_growing.pop_back();
			vector<V3DLONG> xyz_current=pos2xyz(pos_current, offset_Y, offset_Z);
			for (V3DLONG idx_neighbor=0;idx_neighbor<const_count_neighbors;idx_neighbor++)
			{
				vector<V3DLONG> poss_direction (4, 0);
				poss_direction[0]=_pos4s_neighborRelative[idx_neighbor][0];
				poss_direction[1]=_pos4s_neighborRelative[idx_neighbor][1];
				poss_direction[2]=_pos4s_neighborRelative[idx_neighbor][2];
				poss_direction[3]=_pos4s_neighborRelative[idx_neighbor][3];
				V3DLONG pos_neighbor=pos_current+poss_direction[3];
				vector<V3DLONG> xyz_neighbor (3, 0);
				xyz_neighbor[0]=xyz_current[0]+poss_direction[0];
				xyz_neighbor[1]=xyz_current[1]+poss_direction[1];
				xyz_neighbor[2]=xyz_current[2]+poss_direction[2];
				
				if (isValid(pos_neighbor, size_page)) //prevent it from going out of bounds;
				{
					if ((xyz_neighbor[0]<0)||(xyz_neighbor[0]>=_dim_X)||
						(xyz_neighbor[1]<0)||(xyz_neighbor[1]>=_dim_Y)||
						(xyz_neighbor[2]<0)||(xyz_neighbor[2]>=_dim_Z))
					{
						//invalide anyway;
					}
					else
					{
						vector<V3DLONG> color_neighbor=getColorFromPos(_image1Dc_input, pos_neighbor, size_page, _dim_C);
						if ((isColorTolerable(color_seed, color_neighbor, _bandWidth_color, _dim_C))
							&& (isPassedThreshold(_image1Dc_input, pos_neighbor, thresholds_page, _dim_C, size_page))
							&& (_image1D_mask[pos_neighbor]>0))
						{
							_image1D_mask[pos_neighbor]=0; //scooped;
							poss_growing.push_back(pos_neighbor);
							poss_result.push_back(pos_neighbor);
						}
						if (isGapTolerable(_image1Dc_input, pos_neighbor, color_seed, xyz_neighbor, _bandWidth_color, _thresholds_page, poss_direction,
							 _dim_X, _dim_Y, _dim_Z, _dim_C, size_page, _tolerance_gap,	_image1D_mask))
						{
							_image1D_mask[pos_neighbor]=0; //scooped;
							poss_growing.push_back(pos_neighbor);
							poss_result.push_back(pos_neighbor);
						}
					}
				}
			}
		}
	}
	static bool isColorTolerable(vector<V3DLONG> _color_source, vector<V3DLONG> _color_target, vector<V3DLONG> _color_seed, double _bandWidth_color, V3DLONG _dim_C)
	{
		V3DLONG max_seed=getMaxIdx(_color_seed);
		V3DLONG max_target=getMaxIdx(_color_target);
		if (max_target!=max_seed) {return false;}
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			double diff_color1=(double)(iabs(_color_source[idx_color]-_color_target[idx_color]));
			double diff_color2=(double)(iabs(_color_source[idx_color]+_color_target[idx_color]));
			double diff_color=diff_color1/diff_color2;
			if (diff_color>_bandWidth_color) {return false;}
		}
		return true;
		//if (getCorrelation(_color_source, _color_target)>_bandWidth_color) {return true;}
		//else {return false;}
	}
	static bool isColorTolerable(vector<V3DLONG> _color_source, vector<V3DLONG> _color_target, double _bandWidth_color, V3DLONG _dim_C)
	{
		V3DLONG max_source=getMaxIdx(_color_source);
		V3DLONG max_target=getMaxIdx(_color_target);
		if (max_target!=max_source) {return false;}
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			double diff_color=(double)(iabs(_color_source[idx_color]-_color_target[idx_color]));
			diff_color=diff_color/(double)_color_source[idx_color];
			if (diff_color>_bandWidth_color) {return false;}
		}
		return true;
		//if (getCorrelation(_color_source, _color_target)>_bandWidth_color) {return true;}
		//else {return false;}
	}
	static bool isPassedThreshold(const unsigned char* _image1Dc_input, const V3DLONG _pos_input, 
		const vector<V3DLONG> _thresholds_page, const V3DLONG _dim_C, const V3DLONG _size_page)
	{
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			V3DLONG threshold_page=_thresholds_page[idx_color];
			if(_image1Dc_input[_pos_input+_size_page*idx_color]>threshold_page) { return true;}
		}
		return false;
	}
	static bool isGapTolerable(const unsigned char* _image1Dc_input, const V3DLONG _pos_input, const vector<V3DLONG> _color_seed,
		const vector<V3DLONG> _xyz_input, double _bandWidth_color, vector<V3DLONG> _thresholds_page,
		const vector<V3DLONG> _pos_direction, const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z,
		const V3DLONG _dim_C, const V3DLONG _size_page, const V3DLONG _tolerance_gap,
		const unsigned char* _image1D_mask)
	{
		for (V3DLONG idx_gap=1;idx_gap<_tolerance_gap;idx_gap++)
		{
			V3DLONG pos_ray=_pos_input+_pos_direction[3]*idx_gap;
			if (isValid(pos_ray, _size_page)) //prevent it from going out of bounds;
			{
				if (((_xyz_input[0]+_pos_direction[0]*idx_gap)<0)||
					((_xyz_input[0]+_pos_direction[0]*idx_gap)>=_dim_X)||
					((_xyz_input[1]+_pos_direction[1]*idx_gap)<0)||
					((_xyz_input[1]+_pos_direction[1]*idx_gap)>=_dim_Y)||
					((_xyz_input[2]+_pos_direction[2]*idx_gap)<0)||
					((_xyz_input[2]+_pos_direction[2]*idx_gap)>=_dim_Z))
				{
					return false; //invalide anyway;
				}
				else
				{
					vector<V3DLONG> color_ray=getColorFromPos(_image1Dc_input, pos_ray, _size_page, _dim_C);
					if ((isColorTolerable(_color_seed, color_ray, _bandWidth_color, _dim_C))
						&& (isPassedThreshold(_image1Dc_input, pos_ray, _thresholds_page, _dim_C, _size_page))
						&& (_image1D_mask[pos_ray]>0))
					{
						return true;
					}
				}
			}
			else
			{
				return false;
			}
		}
		return false;
	}
	#pragma endregion	

	#pragma region "preProcessings"
	static vector<vector<V3DLONG> > initializeConstants(const V3DLONG _offset_Y, const V3DLONG _offset_Z)
	{
		vector<vector<V3DLONG> > pos4s_result;
		vector<V3DLONG> pos4_neighbor(4, 0); 
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
						pos4_neighbor[0]=x; pos4_neighbor[1]=y; pos4_neighbor[2]=z; pos4_neighbor[3]=(z*_offset_Z+y*_offset_Y+x);
						pos4s_result.push_back(pos4_neighbor);
					}
				}
			}
		}
		return pos4s_result;
	}
	static V3DLONG doThresholding(const unsigned char* _image1D_input, const V3DLONG _size_page, unsigned char* _image1D_mask, V3DLONG _dim_C)
	{
		vector<double> histo_page=getHistogram(_image1D_input, _size_page*_dim_C);
		V3DLONG threshold_page=getThresholdOtsu(histo_page);
		cout<<"threshold for current image: "<<threshold_page<<endl;
		for (V3DLONG pos_i=0;pos_i<_size_page;pos_i++)
		{
			bool is_foreground=false;
			for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
			{
				if (_image1D_input[pos_i+idx_color*_size_page]>threshold_page)
				{
					_image1D_mask[pos_i]=const_max_voxelValue; 
					is_foreground=true;
				}
			}
			if (!is_foreground) { _image1D_mask[pos_i]=0; }
		}
		return threshold_page;
	}
	#pragma endregion

	#pragma region "threshold estimation"
	static V3DLONG getThresholdOtsu(vector<double> _histo_input)
	{
		V3DLONG i, value_threshold;
		float s,ut,uk,wk,max;
		float b,q;
		ut=0.0;
		for(i=0;i<const_length_histogram;i++)
		{
			ut=ut + i*_histo_input[i];
		}
		uk=0.0; wk=0.0; max=0.0;
		for(i=0;i<const_length_histogram;i++)
		{
			wk=wk + _histo_input[i];
			uk=uk + i*_histo_input[i];
			b=ut * wk - uk;
			q=wk * (1.0-wk);
			if(q<const_infinitesimal) continue;
			b=b*b/q;
			if(b > max)
			{
				max=b;
				value_threshold=i;
			}
		}
		return value_threshold;
	}
	static V3DLONG getThresholdYen(vector<double> histo_input)
	{
		// Implements Yen's thresholding method;
		// 1) Yen J.C., Chang F.J., and Chang S. (1995) "A New Criterion for Automatic Multilevel Thresholding" IEEE Trans. on Image Processing, 4(3): 370-378;
		// 2) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding Techniques and Quantitative Performance Evaluation" Journal of Electronic Imaging, 13(1): 146-165;

		V3DLONG value_threshold;
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
	static V3DLONG getThresholdHCP(const unsigned char* _ImageID_input, const V3DLONG _size_page)
	{
		double mean_page=getMean(_ImageID_input, _size_page);
		V3DLONG threshold_page = (V3DLONG)mean_page;
		while (true)
		{
			vector<V3DLONG> poss_foreground;
			vector<V3DLONG> poss_background;
			for (V3DLONG pos_i=0;pos_i<_size_page;pos_i++)
			{
				V3DLONG value_voxel=_ImageID_input[pos_i];
				if (value_voxel>threshold_page) { poss_foreground.push_back(pos_i); }
				else  { poss_background.push_back(pos_i); }
			}
			double mean_foreground=getMean(poss_foreground, _ImageID_input);
			double mean_background=getMean(poss_background, _ImageID_input);
			V3DLONG threshold_new=(V3DLONG)((mean_background+mean_foreground)/2);
			if ((threshold_new-threshold_page)<const_infinitesimal) {break;}
			threshold_page=threshold_new;
		}
		return threshold_page;
	}
	static vector<double> getHistogram(const unsigned char* _ImageID_input, const V3DLONG _size_page)
	{
		vector<double> histo_result (const_length_histogram, 0);
		for (V3DLONG pos_i=0;pos_i<_size_page;pos_i++)
		{
			V3DLONG value_voxel=_ImageID_input[pos_i];
			histo_result[value_voxel]=histo_result[value_voxel]+1;
		}
		for (V3DLONG idx_i=0;idx_i<const_length_histogram;idx_i++)
		{
			histo_result[idx_i]=histo_result[idx_i]/_size_page;
		}
		return histo_result;
	}
	static vector<double> getHistogram(const unsigned char* _ImageID_input, const V3DLONG _size_page, const V3DLONG _pos_start)
	{
		vector<double> histo_result (const_length_histogram, 0);
		for (V3DLONG pos_i=_pos_start;pos_i<(_pos_start+_size_page);pos_i++)
		{
			V3DLONG value_voxel=_ImageID_input[pos_i];
			histo_result[value_voxel]=histo_result[value_voxel]+1;
		}
		for (V3DLONG idx_i=0;idx_i<const_length_histogram;idx_i++)
		{
			histo_result[idx_i]=histo_result[idx_i]/_size_page;
		}
		return histo_result;
	}
	#pragma endregion

	#pragma region "statistics&math"
	static double getCovariance(vector<V3DLONG> vct_input1, vector<V3DLONG> vct_input2)
	{
		double xmean = getMean(vct_input1);
		double ymean = getMean(vct_input1);
		double total = 0;
		for(int i = 0; i < vct_input1.size(); i++)
		{
			total += ((double)vct_input1[i] - xmean) * ((double)vct_input2[i] - ymean);
		}
		return total/vct_input1.size();
	}
	static double getStdev(vector<V3DLONG> vct_input)
	{
		double mean = getMean(vct_input);
		double temp = 0;
		for(int i = 0; i < vct_input.size(); i++)
		{
			temp += ((double)vct_input[i] - mean)*((double)vct_input[i] - mean) ;
		}
		return sqrt(temp/(vct_input.size()-1));
	}
	static double getCorrelation(vector<V3DLONG> vct_input1, vector<V3DLONG> vct_input2)
	{
		double double_covariance = getCovariance(vct_input1, vct_input2);
		return double_covariance/((getStdev(vct_input1))*(getStdev(vct_input2)));
	}
	static V3DLONG getMaxIdx(vector<V3DLONG> _values_input)
	{
		V3DLONG value_max=-INF;
		V3DLONG idx_max=-1;
		V3DLONG size_input=_values_input.size();
		for (V3DLONG idx_i=0;idx_i<size_input;idx_i++)
		{
			if (value_max<_values_input[idx_i])
			{
				value_max=_values_input[idx_i];
				idx_max=idx_i;
			}
		}
		return idx_max;
	}
	static V3DLONG iabs(V3DLONG _value_input)
	{
		if (_value_input>0) { return _value_input;}
		else { return -1*_value_input; }
	}
	static vector<V3DLONG> getMinMax(const V3DLONG* _values_input, const V3DLONG _size_input)
	{
		V3DLONG value_max=-INF;
		V3DLONG value_min=INF;
		for (V3DLONG pos_i=0;pos_i<_size_input;pos_i++)
		{
			V3DLONG value_i=_values_input[pos_i];
			if (value_i>value_max) {value_max=value_i;}
			if (value_i<value_min) {value_min=value_i;}
		}
		vector<V3DLONG> values_result (2, 0); values_result[0]=value_min; values_result[1]=value_max;
		return values_result;
	}
	static double getMean(const unsigned char* _values_input, const V3DLONG _size_input)
	{
		V3DLONG value_mean=0;
		for (V3DLONG pos_i=0;pos_i<_size_input;pos_i++)
		{
			value_mean+=_values_input[pos_i];
		}
		value_mean=value_mean/_size_input;
		return value_mean;
	}
	static double getMean(const vector<V3DLONG> _poss_input, const unsigned char* _values_input)
	{
		V3DLONG value_mean=0;
		V3DLONG size_input=_poss_input.size();
		for (V3DLONG idx_i=0;idx_i<size_input;idx_i++)
		{
			value_mean+=_values_input[_poss_input[idx_i]];
		}
		value_mean=value_mean/size_input;
		return value_mean;
	}
	static double getMean(vector<V3DLONG> vct_input)
	{
		double sum = 0;
		for(int i=0;i<vct_input.size();i++)
		{
			sum += vct_input[i];
		}
		return ((double)sum/(double)vct_input.size());
	}
	static vector<V3DLONG> getMinMax(const vector<V3DLONG> _values_input, const V3DLONG _size_input)
	{
		V3DLONG value_max=-INF;
		V3DLONG value_min=INF;
		for (V3DLONG pos_i=0;pos_i<_size_input;pos_i++)
		{
			V3DLONG value_i=_values_input[pos_i];
			if (value_i>value_max) {value_max=value_i;}
			if (value_i<value_min) {value_min=value_i;}
		}
		vector<V3DLONG> values_result; values_result.push_back(value_min); values_result.push_back(value_max);
		return values_result;
	}
	#pragma endregion

	#pragma region "utility functions"
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
	static vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
	{
		vector<V3DLONG> poss_result;
		V3DLONG count_landmark=LandmarkList_input.count();
		for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
		{
			poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
		}
		return poss_result;
	}
	static V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z)
	{
		float x=0;
		float y=0;
		float z=0;
		Landmark_input.getCoord(x, y, z);
		return (xyz2pos(x-1, y-1, z-1, _offset_Y, _offset_Z));
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
	static vector<V3DLONG> getColorFromPos(const unsigned char* _image1Dc_input, V3DLONG _pos_input, V3DLONG _size_page, V3DLONG _dim_C)
	{
		vector<V3DLONG> color_result (_dim_C, 0);
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			color_result[idx_color]=_image1Dc_input[_pos_input+idx_color*_size_page];
		}
		return color_result;
	}
	#pragma endregion

	#pragma region "main"
	static void main(unsigned char* _image1Dc_input, unsigned char* _image1D_output, unsigned char* _image1D_mask,
		V3DLONG _dim_X, V3DLONG _dim_Y, V3DLONG _dim_Z, V3DLONG _dim_C,
		V3DLONG _pos_landmark, vector<vector<V3DLONG> > _pos4s_neighborRelative, double _bandWidth_color, V3DLONG _tolerance_gap,
		vector<V3DLONG> _thresholds_page)
	{
		V3DLONG size_page=_dim_X*_dim_Y*_dim_Z; V3DLONG offset_Y=_dim_X; V3DLONG offset_Z=_dim_X*_dim_Y;
		cout<<"seed color: ";
		vector<V3DLONG> color_seed=getColorFromPos(_image1Dc_input, _pos_landmark, size_page, _dim_C);
		vector<V3DLONG> thresholds_page=_thresholds_page;
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			cout<<"["<<idx_color<<"]: "<<color_seed[idx_color]<<"; ";
		}
		cout<<endl;
		vector<V3DLONG> poss_region=doRegionGrow(_image1Dc_input, _pos_landmark, _dim_X, _dim_Y, _dim_Z, _dim_C,
			_image1D_mask,	_pos4s_neighborRelative, _bandWidth_color, _tolerance_gap, thresholds_page);
		poss2Image1D(poss_region, _image1D_output, _image1Dc_input, size_page);
	}
	#pragma endregion
};
#pragma endregion

QStringList neuronPicker::menulist() const
{
	return QStringList() 
		<<tr("neuronPicker")
        <<tr("neuronPicker_old")
		<<tr("about");
}

QStringList neuronPicker::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuronPicker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuronPicker"))
	{
        neuronPickerDialog* npdiag = new neuronPickerDialog(&callback);
        npdiag->show();
	}
    else if (menu_name == tr("neuronPicker_old"))
	{
        interface_run(callback, parent);
	}
	else
	{
		v3d_msg(tr(". "
			"Developed by Xiang Li, Hanbo Chen, 2014-12-01"));
	}
}

bool neuronPicker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

#pragma region "interface"
bool neuronPicker::interface_run(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent)
{
	v3dhandle v3dhandle_currentWindow = _V3DPluginCallback2_currentCallback.currentImageWindow();
	if (!v3dhandle_currentWindow) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
	Image4DSimple* Image4DSimple_current = _V3DPluginCallback2_currentCallback.getImage(v3dhandle_currentWindow);
	if (!Image4DSimple_current) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
	V3DLONG count_totalBytes = Image4DSimple_current->getTotalBytes();
	if (count_totalBytes<1) {v3d_msg("You have not loaded any image or the image is corrupted, program canceled!"); return false;}
	unsigned char* image1Dc_current = Image4DSimple_current->getRawData();
	QString name_currentWindow = _V3DPluginCallback2_currentCallback.getImageName(v3dhandle_currentWindow);
	V3DLONG dim_X = Image4DSimple_current->getXDim(); V3DLONG dim_Y = Image4DSimple_current->getYDim();
	V3DLONG dim_Z = Image4DSimple_current->getZDim(); V3DLONG dim_C = Image4DSimple_current->getCDim();if (dim_C>3) {dim_C=3;}
	V3DLONG size_image=dim_X*dim_Y*dim_Z*dim_C; V3DLONG offset_Y=dim_X; V3DLONG offset_Z=dim_X*dim_Y;
	V3DLONG size_page=dim_X*dim_Y*dim_Z;
	LandmarkList LandmarkList_userDefined = _V3DPluginCallback2_currentCallback.getLandmark(v3dhandle_currentWindow);
	V3DLONG count_userDefinedLandmarkList = LandmarkList_userDefined.count();
    //Hanbo: we will not use SWC in this way
//    QList<NeuronTree> * SWCList_current = _V3DPluginCallback2_currentCallback.getHandleNeuronTrees_3DGlobalViewer(v3dhandle_currentWindow);
//	V3DLONG count_SWCList = 0;
//	if (SWCList_current) {count_SWCList = SWCList_current->count();}
    LandmarkList LandmarkList_current;
    V3DLONG count_currentLandmarkList = -1;
//	if ((count_SWCList<1) && (count_userDefinedLandmarkList<1)) {v3d_msg("You have not defined any landmarks or swc structure to run the segmenation, program canceled!"); return false;}
//	else if ((count_SWCList>0) && (count_userDefinedLandmarkList>0))
//	{
//		LandmarkList_current = LandmarkList_userDefined;
//		neuronPickerMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
//	}
//	else if ((count_SWCList>0) && (count_userDefinedLandmarkList<1))
//	{
//		neuronPickerMain::neuronTree2LandmarkList(SWCList_current->first(), LandmarkList_current);
//	}
	if (count_userDefinedLandmarkList>0)
	{
		LandmarkList_current = LandmarkList_userDefined;
	}
	dialogRun dialogRun1(_V3DPluginCallback2_currentCallback, _QWidget_parent);
	if (dialogRun1.exec()!=QDialog::Accepted) {return false;}
	count_currentLandmarkList = LandmarkList_current.count();
	vector<V3DLONG> poss_landmark=neuronPickerMain::landMarkList2poss(LandmarkList_current, offset_Y, offset_Z);
	vector<vector<V3DLONG> > pos4s_neighborRelative=neuronPickerMain::initializeConstants(offset_Y, offset_Z);
	unsigned char* image1D_mask=neuronPickerMain::memory_allocate_uchar1D(size_page);
	vector<V3DLONG> thresholds_page (dim_C, 0);
	cout<<"threshold: ";
	for (V3DLONG idx_color=0;idx_color<dim_C;idx_color++)
	{
		vector<double> histo_page=neuronPickerMain::getHistogram(image1Dc_current, size_page, idx_color*size_page);
		thresholds_page[idx_color]=neuronPickerMain::getThresholdOtsu(histo_page);
		cout<<" ["<<idx_color<<"]: "<<thresholds_page[idx_color]<<"; ";
	}
	cout<<endl;
	for (V3DLONG idx_landmark=0;idx_landmark<count_currentLandmarkList;idx_landmark++)
	{
		V3DLONG pos_landmark=poss_landmark[idx_landmark];
		memset(image1D_mask, const_max_voxelValue, size_page*sizeof(unsigned char));
		unsigned char* image1D_tmp=neuronPickerMain::memory_allocate_uchar1D(size_page);
		neuronPickerMain::main(image1Dc_current, image1D_tmp, image1D_mask, dim_X, dim_Y, dim_Z, dim_C, pos_landmark,
			pos4s_neighborRelative, dialogRun1.bandWidth_color, dialogRun1.size_gap, thresholds_page);
		visualizationImage1D(image1D_tmp, dim_X, dim_Y, dim_Z, 1, _V3DPluginCallback2_currentCallback, QString(name_currentWindow+QString("%1").arg(idx_landmark)));
		neuronPickerMain::memory_free_uchar1D(image1D_tmp);
	}
}

void visualizationImage1D(const unsigned char* _image1D_input, const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z, const V3DLONG _dim_C, V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, const QString name_window)
{
	V3DLONG size_page=_dim_X*_dim_Y*_dim_Z*_dim_C;
	unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
	memcpy(image1D_input, _image1D_input, size_page*sizeof(unsigned char));
	Image4DSimple Image4DSimple_temp;
	Image4DSimple_temp.setData(image1D_input, _dim_X, _dim_Y, _dim_Z, _dim_C, V3D_UINT8);
	v3dhandleList v3dhandleList_current=_V3DPluginCallback2_currentCallback.getImageWindowList();
	V3DLONG count_v3dhandle=v3dhandleList_current.size();
	bool is_found=false;
	for (V3DLONG i=0;i<count_v3dhandle;i++)
	{
		if (_V3DPluginCallback2_currentCallback.getImageName(v3dhandleList_current[i]).contains(name_window))
		{
			_V3DPluginCallback2_currentCallback.setImage(v3dhandleList_current[i], &Image4DSimple_temp);
			_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandleList_current[i]);
			is_found=true; break;
		}
	}
	if (!is_found)
	{
		v3dhandle v3dhandle_main=_V3DPluginCallback2_currentCallback.newImageWindow();
		_V3DPluginCallback2_currentCallback.setImage(v3dhandle_main, &Image4DSimple_temp);
		_V3DPluginCallback2_currentCallback.setImageName(v3dhandle_main, name_window);
		_V3DPluginCallback2_currentCallback.updateImageWindow(v3dhandle_main);
	}
}
#pragma endregion

//Hanbo
//dialog
neuronPickerDialog::neuronPickerDialog(V3DPluginCallback2 * cb)
{
    callback = cb;

    fname_input="";
    fname_outbase="";
    image1Dc_in=0;
    image1D_out=0;
    image1D_mask=0;
    intype=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    v3dhandle_in = 0;
    v3dhandle_out = 0;
    LList.clear();

    creat();
    checkButtons();

}

void neuronPickerDialog::creat()
{
    QGridLayout *gridLayout = new QGridLayout();

    //I/O zone
    QLabel* label_load = new QLabel(QObject::tr("Input Image:"));
    gridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit();
    edit_load->setText(""); edit_load->setReadOnly(true);
    gridLayout->addWidget(edit_load,0,1,1,4);
    btn_load = new QPushButton("...");
    gridLayout->addWidget(btn_load,0,5,1,1);

    QLabel* label_output = new QLabel(QObject::tr("Output Prefix:"));
    gridLayout->addWidget(label_output,1,0,1,1);
    edit_output = new QLineEdit();
    edit_output->setText(""); edit_output->setReadOnly(true);
    gridLayout->addWidget(edit_output,1,1,1,4);
    btn_output = new QPushButton("...");
    gridLayout->addWidget(btn_output,1,5,1,1);

    connect(btn_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(output()));

    //marker zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,4,0,1,6);

    cb_marker = new QComboBox();
    gridLayout->addWidget(cb_marker,5,0,1,1);
    btn_update = new QPushButton("Load Markers");
    gridLayout->addWidget(btn_update,5,1,1,1);
    btn_extract = new QPushButton("Extract Neuron");
    gridLayout->addWidget(btn_extract,5,2,1,1);
    btn_save = new QPushButton("Accept, Save, and Next");
    gridLayout->addWidget(btn_save,5,3,1,2);
    btn_next = new QPushButton("Reject and Next");
    gridLayout->addWidget(btn_next,5,5,1,1);

    connect(btn_update, SIGNAL(clicked()), this, SLOT(loadMarkers()));
    connect(btn_extract, SIGNAL(clicked()), this, SLOT(extract()));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(saveFile()));
    connect(btn_next, SIGNAL(clicked()), this, SLOT(skip()));
    connect(cb_marker, SIGNAL(currentIndexChanged()), this, SLOT(syncMarkers()));

    //extract parameters
    spin_color = new QDoubleSpinBox();
    spin_color->setRange(0,1); spin_color->setValue(0.9);
    spin_distance = new QSpinBox();
    spin_distance->setRange(0,100000); spin_distance->setValue(10);
    QLabel* label_0 = new QLabel("color filtering bandwidth:");
    gridLayout->addWidget(label_0,6,0,1,2);
    gridLayout->addWidget(spin_color,6,2,1,1);
    QLabel* label_1 = new QLabel("maximum gap distance: ");
    gridLayout->addWidget(label_1,6,3,1,2);
    gridLayout->addWidget(spin_distance,6,5,1,1);

    //other
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,10,0,1,6);

    btn_quit = new QPushButton("Quit");
    gridLayout->addWidget(btn_quit,11,5,1,1);

    connect(btn_quit, SIGNAL(clicked()), this, SLOT(accept()));

    setLayout(gridLayout);
}

bool neuronPickerDialog::load()
{
    fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                               QDir::currentPath(),
                                               QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if(fname_input.isEmpty()){
        return false;
    }

    qDebug()<<"NeuronPicker: is going to load "<<fname_input;
    {//reset previous content first
        if(image1Dc_in != 0){
            neuronPickerMain::memory_free_uchar1D(image1Dc_in);
        }
        if(image1D_out != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_out);
        }
        if(image1D_mask != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_mask);
        }
        image1Dc_in=0;
        image1D_out=0;
        image1D_mask=0;
        fname_outbase="";
        intype=0;
        sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
        LList.clear();
        cb_marker->clear();
        qDebug()<<"NeuronPicker: reset input window";
        updateInputWindow();
    }
    qDebug()<<"NeuronPicker: load new image";
    if(!simple_loadimage_wrapper(*callback, fname_input.toStdString().c_str(), image1Dc_in, sz_img, intype))
    {
      v3d_msg("load image "+fname_input+" error!");
      return false;
    }
    qDebug()<<"\t>>read image file "<< fname_input <<" complete.";
    qDebug()<<"\t\timage size: [w="<<sz_img[0]<<", h="<<sz_img[1]<<", z="<<sz_img[2]<<", c="<<sz_img[3]<<"]";
    qDebug()<<"\t\tdatatype: "<<intype;
    if(intype!=1)
    {
        v3d_msg("ERROR: Input image datatype is not UINT8.");
        return false;
    }

    fname_outbase=fname_input+".extract";
    edit_load->setText(fname_input);
    edit_output->setText(fname_outbase);

    qDebug()<<"NeuronPicker: update visualization windows";
    updateInputWindow();
    updateOutputWindow();
    checkButtons();

    //initialize other stuffs for calculation
    qDebug()<<"NeuronPicker: initialize veriables";
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    image1D_out=neuronPickerMain::memory_allocate_uchar1D(size_page);
    pos4s_neighborRelative=neuronPickerMain::initializeConstants(sz_img[0], sz_img[0]*sz_img[1]);
    image1D_mask=neuronPickerMain::memory_allocate_uchar1D(size_page);
    thresholds_page.resize(sz_img[3]);
    cout<<"threshold: ";
    for (V3DLONG idx_color=0;idx_color<sz_img[3];idx_color++)
    {
        vector<double> histo_page=neuronPickerMain::getHistogram(image1Dc_in, size_page, idx_color*size_page);
        thresholds_page[idx_color]=neuronPickerMain::getThresholdOtsu(histo_page);
        cout<<" ["<<idx_color<<"]: "<<thresholds_page[idx_color]<<"; ";
    }
    cout<<endl;
}

void neuronPickerDialog::output()
{
    QString fname_output = QFileDialog::getSaveFileName(0, QObject::tr("Choose the output folder and prefix"),
                                               fname_outbase,
                                               "");

    if(!fname_output.isEmpty()){
        fname_outbase=fname_output;
    }
    edit_output->setText(fname_outbase);
    checkButtons();
}

int neuronPickerDialog::loadMarkers()
{
    qDebug()<<"cojoc: try loading markers";
    v3dhandle * v3dhandle_in = getInwinHandle();
    if(v3dhandle_in!=0){
        LList.clear();
        QStringList cb_items;
        LandmarkList LList_in = callback->getLandmark(*v3dhandle_in);
        qDebug()<<"cojoc: there are "<<LList_in.size()<<" markers";
        for(int i=0; i<LList_in.size(); i++){
            LList.append(LList_in.at(i));
            LList[i].color.r=196;
            LList[i].color.g=LList[i].color.b=0;
            cb_items.append("marker: " + QString::number(i+1));
        }
        qDebug()<<"cojoc: "<<LList.size()<<" markers";
        cb_marker->clear();
        cb_marker->addItems(cb_items);
        poss_landmark=neuronPickerMain::landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    }
    checkButtons();
}

void neuronPickerDialog::syncMarkers()
{
    v3dhandle * v3dhandle_in = getInwinHandle();
    if(v3dhandle_in!=0){
        for(int i=0; i<LList.size(); i++){
            if(i==cb_marker->currentIndex()){
                if(LList[i].color.r>0) LList[i].color.r=255;
                if(LList[i].color.g>0) LList[i].color.g=255;
                if(LList[i].color.b>0) LList[i].color.b=255;
            }else{
                if(LList[i].color.r>0) LList[i].color.r=196;
                if(LList[i].color.g>0) LList[i].color.g=196;
                if(LList[i].color.b>0) LList[i].color.b=196;
            }
        }
        callback->setLandmark(*v3dhandle_in,LList);
    }
}

void neuronPickerDialog::extract()
{
    if(LList.size()<1 || cb_marker->count()<1 || cb_marker->currentIndex()+1>LList.size() ){
        return;
    }
    int idx_landmark=cb_marker->currentIndex();
    V3DLONG pos_landmark=poss_landmark[idx_landmark];
    memset(image1D_mask, const_max_voxelValue, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
    qDebug()<<"start extracting";
    neuronPickerMain::main(image1Dc_in, image1D_out, image1D_mask, sz_img[0], sz_img[1], sz_img[2], sz_img[3], pos_landmark,
            pos4s_neighborRelative, spin_color->value(), (V3DLONG)(spin_distance->value()), thresholds_page);
    //visualizationImage1D(image1D_tmp, sz_img[0], sz_img[1], sz_img[2], 1, *call, QString(name_currentWindow+QString("%1").arg(idx_landmark)));
    qDebug()<<"push for visualization";
    updateOutputWindow();
}

void neuronPickerDialog::saveFile()
{

}

void neuronPickerDialog::skip()
{

}


void neuronPickerDialog::checkButtons()
{
    if(image1Dc_in==0){
        btn_update->setEnabled(false);
        btn_extract->setEnabled(false);
        btn_save->setEnabled(false);
        btn_next->setEnabled(false);
    }else{
        v3dhandle * v3dhandle_in = getInwinHandle();
        if(v3dhandle_in!=0){
            btn_update->setEnabled(true);
        }
        if(fname_outbase.isEmpty() || image1D_out==0){
            btn_save->setEnabled(false);
        }else{
            btn_save->setEnabled(true);
        }
        if(cb_marker->count()>0){
            btn_extract->setEnabled(true);
            if(cb_marker->count()>cb_marker->currentIndex()+1)
                btn_next->setEnabled(true);
            else
                btn_next->setEnabled(false);
        }else{
            btn_extract->setEnabled(false);
            btn_next->setEnabled(false);
        }
    }
}

void neuronPickerDialog::updateInputWindow()
{
    //search in open windows
    v3dhandle * v3dhandle_in = getInwinHandle();
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it
        qDebug()<<"cojoc: image4d: "<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2]<<":"<<sz_img[3];
        Image4DSimple image4d;
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1Dc_in, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);

        qDebug()<<"cojoc: refresh content";
        if(v3dhandle_in==0){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            v3dhandle_in = &v3dhandle_main;
            //update the image
            callback->setImage(*v3dhandle_in, &image4d);
            callback->setImageName(*v3dhandle_in, NAME_INWIN);
            callback->updateImageWindow(*v3dhandle_in);
        }else{
            //update the image
            callback->setImage(*v3dhandle_in, &image4d);
            callback->updateImageWindow(*v3dhandle_in);
        }
    }else if(v3dhandle_in!=0){
        //no image loaded, but there is a window? clean up the contents
        //clean up landmarks
        LandmarkList LList_empty;
        callback->setLandmark(*v3dhandle_in, LList_empty);
        //clean up window
        Image4DSimple image4d;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        callback->setImage(*v3dhandle_in, &image4d);
        callback->setImageName(*v3dhandle_in, NAME_INWIN);
        callback->updateImageWindow(*v3dhandle_in);
    }
}

v3dhandle * neuronPickerDialog::getInwinHandle()
{
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            return &(v3dhandleList_current[i]);
        }
    }
    return 0;
}

void neuronPickerDialog::updateOutputWindow()
{
    //search in open windows
    v3dhandle * v3dhandle_out = getOutwinHandle();
    if(image1D_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
        unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1D_out, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], 1, V3D_UINT8);

        if(v3dhandle_out==0){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            v3dhandle_out = &v3dhandle_main;
            callback->setImage(*v3dhandle_out, &image4d);
            callback->setImageName(*v3dhandle_out, NAME_OUTWIN);
            callback->updateImageWindow(*v3dhandle_out);
        }else{
            //update the image
            callback->setImage(*v3dhandle_out, &image4d);
            callback->updateImageWindow(*v3dhandle_out);
        }
    }else if(v3dhandle_out!=0){
        //no image loaded, but there is a window? clean up the window content
        Image4DSimple image4d;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        callback->setImage(*v3dhandle_out, &image4d);
        callback->setImageName(*v3dhandle_out, NAME_OUTWIN);
        callback->updateImageWindow(*v3dhandle_out);
    }
}

v3dhandle * neuronPickerDialog::getOutwinHandle()
{
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            return &(v3dhandleList_current[i]);
        }
    }
    return 0;
}
