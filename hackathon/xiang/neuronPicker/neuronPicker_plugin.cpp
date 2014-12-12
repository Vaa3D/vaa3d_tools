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
const double const_infinitesimal = 0.000000001;

static neuronPickerDialog * npdiag = 0;
void finishnpdiag()
{
    if (npdiag)
    {
        delete npdiag;
        npdiag=0;
    }
}

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
	static bool isValid(const vector<V3DLONG> _xyz_input, const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z)
	{
		if ((_xyz_input[0]<0)||(_xyz_input[0]>=_dim_X)||
			(_xyz_input[1]<0)||(_xyz_input[1]>=_dim_Y)||
			(_xyz_input[2]<0)||(_xyz_input[2]>=_dim_Z))
		{
			return false;
		}
		else { return true; }
	}
	static void poss2Image1Dc(const vector<V3DLONG> _poss_input, unsigned char* _image1D_input, const unsigned char value_input)
	{
		V3DLONG size_input=_poss_input.size();
		for (V3DLONG i=0;i<size_input;i++) {_image1D_input[_poss_input[i]]=value_input; }
	}
	static void poss2Image1Dc(const vector<V3DLONG> _poss_input, unsigned char* _image1D_output, const unsigned char* _image1Dc_input,
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
	static void poss2Image1D(const vector<V3DLONG> _poss_input, unsigned char* _Image1D_input, const V3DLONG _value_input)
	{
		V3DLONG size_poss = _poss_input.size();
		for (V3DLONG i=0;i<size_poss;i++) { _Image1D_input[_poss_input[i]]=_value_input; }
	}
	#pragma endregion

	#pragma region "regionGrow"
	static vector<V3DLONG> doRegionGrow(const unsigned char* _image1Dc_input, const V3DLONG _pos_seed, const V3DLONG _size_max,
		const V3DLONG _dim_X, const V3DLONG _dim_Y, const V3DLONG _dim_Z, V3DLONG _dim_C, unsigned char* _image1D_mask,
		const vector<vector<V3DLONG> > _pos4s_neighborRelative, const double _bandWidth_color)
	{
		vector<V3DLONG> poss_result;
		if (_image1D_mask[_pos_seed]<1) {return poss_result;}
		V3DLONG offset_Y=_dim_X; V3DLONG offset_Z=_dim_X*_dim_Y; V3DLONG size_page=_dim_X*_dim_Y*_dim_Z;
		vector<V3DLONG> poss_growing;
		poss_growing.push_back(_pos_seed);
		poss_result.push_back(_pos_seed);
		vector<V3DLONG> color_seed=getColorFromPos(_image1Dc_input, _pos_seed, size_page, _dim_C);
		_image1D_mask[_pos_seed]=0; //scooped;
		V3DLONG count_neighbors=_pos4s_neighborRelative.size();
		V3DLONG size_region = 1;
		while (true)
		{
			//cout<<"poss_growing: "<<poss_growing.size()<<endl;
			if (poss_growing.empty()) //growing complete;
			{
				return poss_result;
			}
			V3DLONG pos_current=poss_growing.back();
			//vector<V3DLONG> color_current=getColorFromPos(_image1Dc_input, pos_current, size_page, _dim_C);
			poss_growing.pop_back();
			vector<V3DLONG> xyz_current=pos2xyz(pos_current, offset_Y, offset_Z);
			for (V3DLONG idx_neighbor=0;idx_neighbor<count_neighbors;idx_neighbor++)
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
				if (isValid(xyz_neighbor, _dim_X, _dim_Y, _dim_Z)) //prevent it from going out of bounds;
				{
					vector<V3DLONG> color_neighbor=getColorFromPos(_image1Dc_input, pos_neighbor, size_page, _dim_C);
					if ((isColorBandpassed(color_seed, color_neighbor, _bandWidth_color, _dim_C))
						&& (_image1D_mask[pos_neighbor]>0))
					{
						_image1D_mask[pos_neighbor]=0; //scooped;
						poss_growing.push_back(pos_neighbor);
						poss_result.push_back(pos_neighbor);
						size_region++;
						if (size_region>_size_max) {return poss_result;}
					}
				}
			}
		}
	}
	static bool isColorBandpassed(vector<V3DLONG> _color_source, vector<V3DLONG> _color_target, double _bandWidth_color, V3DLONG _dim_C)
	{
		for (V3DLONG idx_color=0;idx_color<=_dim_C;idx_color++)
		{
			V3DLONG diff_color=(iabs(_color_source[idx_color]-_color_target[idx_color]));
			//if (diff_color>(_bandWidth_color*_color_source[idx_color])) {return false;}
			if (diff_color>_bandWidth_color) {return false;}
		}
		return true;
	}
	static V3DLONG iabs(V3DLONG _value_input)
	{
		if (_value_input>0) { return _value_input;}
		else { return -1*_value_input; }
	}
	#pragma endregion	

	#pragma region "preProcessings"
	static vector<vector<V3DLONG> > initializeConstants(const V3DLONG _offset_Y, const V3DLONG _offset_Z, const V3DLONG _size_neighbor)
	{
		vector<vector<V3DLONG> > pos4s_result;
		vector<V3DLONG> pos4_neighbor(4, 0); 
		V3DLONG min_neighbor=_size_neighbor*-1;
		V3DLONG max_neighbor=_size_neighbor;
		for (V3DLONG z=min_neighbor;z<=max_neighbor;z++)
		{
			for (V3DLONG y=min_neighbor;y<=max_neighbor;y++)
			{
				for (V3DLONG x=min_neighbor;x<=max_neighbor;x++)
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
	static void doThresholding(const unsigned char* _image1D_input, const V3DLONG _size_page, unsigned char* _image1D_mask, V3DLONG _dim_C)
	{
		vector<V3DLONG> thresholds_page;
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			vector<double> histo_page=getHistogram(_image1D_input, _size_page, idx_color*_size_page);
			V3DLONG threshold_page=getThresholdOtsu(histo_page);
			thresholds_page.push_back(threshold_page);
			cout<<"threshold for current image channel ["<<idx_color<<"]: "<<threshold_page<<endl;
		}
		for (V3DLONG pos_i=0;pos_i<_size_page;pos_i++)
		{
			bool is_foreground=false;
			for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
			{
				if (_image1D_input[pos_i+idx_color*_size_page]>thresholds_page[idx_color])
				{
					_image1D_mask[pos_i]=const_max_voxelValue; 
					is_foreground=true;
					break;
				}
			}
			if (!is_foreground) { _image1D_mask[pos_i]=0; }
			else { _image1D_mask[pos_i]=const_max_voxelValue; }
		}
		return;
	}
	static void doThresholding(const unsigned char* _image1D_input, const V3DLONG _size_page, unsigned char* _image1D_mask)
	{
		vector<V3DLONG> thresholds_page;
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			vector<double> histo_page=getHistogram(_image1D_input, _size_page, idx_color*_size_page);
			V3DLONG threshold_page=getThresholdOtsu(histo_page);
			thresholds_page.push_back(threshold_page);
			cout<<"threshold for current image channel ["<<idx_color<<"]: "<<threshold_page<<endl;
		}
		for (V3DLONG pos_i=0;pos_i<_size_page;pos_i++)
		{
			bool is_foreground=false;
			for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
			{
				if (_image1D_input[pos_i+idx_color*_size_page]>thresholds_page[idx_color])
				{
					_image1D_mask[pos_i]=const_max_voxelValue; 
					is_foreground=true;
					break;
				}
			}
			if (!is_foreground) { _image1D_mask[pos_i]=0; }
			else { _image1D_mask[pos_i]=const_max_voxelValue; }
		}
		return;
	}
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

	#pragma region "utility functions"
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
	static vector<V3DLONG> getColorFromPos(const unsigned char* _image1Dc_input, V3DLONG _pos_input, V3DLONG _size_page, V3DLONG _dim_C)
	{
		vector<V3DLONG> color_result (_dim_C, 0);
		double color_max=0;
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			color_result[idx_color]=_image1Dc_input[_pos_input+idx_color*_size_page];
			if (color_max<color_result[idx_color]) {color_max=color_result[idx_color];}
		}
		double prop_color = (double)const_max_voxelValue/color_max;
		for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
		{
			color_result[idx_color]=floor((double)color_result[idx_color]*prop_color);
		}
		return color_result;
	}
	#pragma endregion

	#pragma region "main"
	static void main(unsigned char* _image1Dc_input, unsigned char* _image1D_output, unsigned char* _image1D_mask,
		V3DLONG _dim_X, V3DLONG _dim_Y, V3DLONG _dim_Z, V3DLONG _dim_C,
		V3DLONG _pos_landmark, vector<vector<V3DLONG> > _pos4s_neighborRelative, double _max_size)
	{
		V3DLONG size_page=_dim_X*_dim_Y*_dim_Z; V3DLONG offset_Y=_dim_X; V3DLONG offset_Z=_dim_X*_dim_Y;
		if (_dim_C>3) {_dim_C=3;}
		unsigned char* image1D_intensity = memory_allocate_uchar1D(size_page);
		for (V3DLONG pos_i;pos_i<size_page;pos_i++)
		{
			V3DLONG intensity_sum = 0;
			for (V3DLONG idx_color=0;idx_color<_dim_C;idx_color++)
			{
				intensity_sum += (_image1Dc_input[pos_i+idx_color*size_page]*_image1Dc_input[pos_i+idx_color*size_page]);
			}
			image1D_intensity[pos_i]=floor(sqrt((double)intensity_sum));
		}
		doThresholding(image1D_intensity, size_page, _image1D_mask, _dim_C);
		cout<<"seed color: ";
		vector<V3DLONG> color_seed=getColorFromPos(_image1Dc_input, _pos_landmark, size_page, _dim_C);
		for (V3DLONG idx_color=0;idx_color<=_dim_C;idx_color++)
		{
			cout<<"["<<idx_color<<"]: "<<color_seed[idx_color]<<"; ";
		}
		cout<<endl;
		V3DLONG step_bandWidth = 10;
		V3DLONG max_seedColor = max(max(color_seed[0],color_seed[1]), color_seed[2]);
		V3DLONG count_steps = floor(const_max_voxelValue/step_bandWidth);
		vector<V3DLONG> poss_region;
		V3DLONG idx_step;
		for (idx_step=0;idx_step<count_steps;idx_step++)
		{
			V3DLONG bandWidth_color = (step_bandWidth)*(idx_step+1);
			cout<<"bandWidth_color: "<<bandWidth_color<<endl;
			poss_region=doRegionGrow(_image1Dc_input, _pos_landmark, _max_size, _dim_X, _dim_Y, _dim_Z, _dim_C,
				_image1D_mask,	_pos4s_neighborRelative, bandWidth_color);
			poss2Image1D(poss_region, _image1D_mask, const_max_voxelValue);
			if (poss_region.size()>_max_size) { break; }
		}
		poss_region = doRegionGrow(_image1Dc_input, _pos_landmark, _max_size, _dim_X, _dim_Y, _dim_Z, _dim_C,
			_image1D_mask,	_pos4s_neighborRelative, (const_max_voxelValue-step_bandWidth*(idx_step-1)));
		poss2Image1Dc(poss_region, _image1D_output, _image1Dc_input, size_page);
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
        if(npdiag){
            npdiag->show();
            npdiag->raise();
            npdiag->activateWindow();
        }else{
            npdiag = new neuronPickerDialog(&callback);
            npdiag->show();
            npdiag->raise();
            npdiag->activateWindow();
        }
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
// func convert2UINT8
void neuronPickerDialog::convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
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
void neuronPickerDialog::convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
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
	vector<vector<V3DLONG> > pos4s_neighborRelative=neuronPickerMain::initializeConstants(offset_Y, offset_Z, dialogRun1.size_neighbor);
	unsigned char* image1D_mask=neuronPickerMain::memory_allocate_uchar1D(size_page);
	for (V3DLONG idx_landmark=0;idx_landmark<count_currentLandmarkList;idx_landmark++)
	{
		V3DLONG pos_landmark=poss_landmark[idx_landmark];
		memset(image1D_mask, const_max_voxelValue, size_page*sizeof(unsigned char));
		unsigned char* image1D_tmp=neuronPickerMain::memory_allocate_uchar1D(size_page);
		neuronPickerMain::main(image1Dc_current, image1D_tmp, image1D_mask, dim_X, dim_Y, dim_Z, dim_C, pos_landmark,
			pos4s_neighborRelative, dialogRun1.bandWidth_color);
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
    LList.clear();

    creat();
    checkButtons();
}

void neuronPickerDialog::reject()
{
    //release memory
    if(npdiag){
        if(image1Dc_in != 0){
            delete []image1Dc_in; image1Dc_in=0;
        }
        if(image1D_out != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_out);
            image1D_out=0;
        }
        if(image1D_mask != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_mask);
            image1D_mask=0;
        }
    }
    finishnpdiag();
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


    btn_update = new QPushButton("Load Markers =>");
    gridLayout->addWidget(btn_update,5,0,1,1);
    cb_marker = new QComboBox();
    gridLayout->addWidget(cb_marker,5,1,1,1);
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
    connect(cb_marker, SIGNAL(currentIndexChanged(int)), this, SLOT(syncMarkers(int)));

    //extract parameters
    spin_color = new QDoubleSpinBox();
    spin_color->setRange(0,100); spin_color->setValue(10);
    spin_distance = new QSpinBox();
    spin_distance->setRange(0,100000); spin_distance->setValue(5);
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

    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

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
    qDebug()<<"NeuronPicker: reset environment";
    {//reset previous content first
        if(image1Dc_in != 0){
            delete []image1Dc_in; image1Dc_in=0;
        }
        if(image1D_out != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_out);
            image1D_out=0;
        }
        if(image1D_mask != 0){
            neuronPickerMain::memory_free_uchar1D(image1D_mask);
            image1D_mask=0;
        }
        fname_outbase="";
        intype=0;
        sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
        LList.clear();
        cb_marker->clear();
        qDebug()<<"NeuronPicker: reset input window";
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
	V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    if(intype!=1)
    {
		if (intype == 2) //V3D_UINT16;
		{
			neuronPickerDialog::convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
		}
		else if(intype == 4) //V3D_FLOAT32;
		{
			neuronPickerDialog::convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
		}
		else
		{
			v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
			return false;
		}
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
    pos4s_neighborRelative=neuronPickerMain::initializeConstants(sz_img[0], sz_img[0]*sz_img[1], (V3DLONG)(spin_distance->value()));
    image1D_mask=neuronPickerMain::memory_allocate_uchar1D(size_page);
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
    LList.clear();
    QStringList cb_items;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {

            LandmarkList LList_in = callback->getLandmark(v3dhandleList_current[i]);
            for(int i=0; i<LList_in.size(); i++){
                LList.append(LList_in.at(i));
                LList[i].color.r=196;
                LList[i].color.g=LList[i].color.b=0;
                cb_items.append("marker: " + QString::number(i+1));
            }

            break;
        }
    }
    poss_landmark=neuronPickerMain::landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    cb_marker->clear();
    cb_marker->addItems(cb_items);
    if(LList.size()>0){
        extract();
    }
    checkButtons();
    return(LList.size());
}

void neuronPickerDialog::syncMarkers(int c)
{
    qDebug()<<"cojoc: syncronize markers";
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            for(int i=0; i<LList.size(); i++){
                if(i==cb_marker->currentIndex()){
                    LList[i].color.b=0;
                }else{
                    LList[i].color.b=196;
                }
            }
            callback->setLandmark(v3dhandleList_current[i],LList);
            callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
        }
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
    memset(image1D_out, 0, sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
    qDebug()<<"start extracting";
	V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
    neuronPickerMain::main(image1Dc_in, image1D_out, image1D_mask, sz_img[0], sz_img[1], sz_img[2], 3, pos_landmark,
            pos4s_neighborRelative, spin_color->value()/100000*size_page);
    //visualizationImage1D(image1D_tmp, sz_img[0], sz_img[1], sz_img[2], 1, *call, QString(name_currentWindow+QString("%1").arg(idx_landmark)));
    qDebug()<<"push for visualization";
    updateOutputWindow();
}

void neuronPickerDialog::saveFile()
{
    int idx_landmark=cb_marker->currentIndex();
    V3DLONG sz_img_sub[4];
    sz_img_sub[0]=sz_img[0];
    sz_img_sub[1]=sz_img[1];
    sz_img_sub[2]=sz_img[2];
    sz_img_sub[3]=1;
    QString fname_output=fname_outbase + "_" + QString::number(LList.at(idx_landmark).x) + "_" +
            QString::number(LList.at(idx_landmark).y) + "_" + QString::number(LList.at(idx_landmark).z) + ".v3dpbd";
    if(!simple_saveimage_wrapper(*callback, qPrintable(fname_output),image1D_out,sz_img_sub,1)){
        v3d_msg("failed to save file to " + fname_output);
        return;
    }else{
        qDebug()<<"NeuronPicker: "<<fname_output;
    }
    LList[idx_landmark].color.r = 0;
    LList[idx_landmark].color.g = 255;
    if(idx_landmark+1<cb_marker->count()){
        cb_marker->setCurrentIndex(idx_landmark+1);
        extract();
    }else{
        v3d_msg("Reach the end of marker list.");
    }
    checkButtons();
}

void neuronPickerDialog::skip()
{
    int idx_landmark=cb_marker->currentIndex();
    if(idx_landmark+1<cb_marker->count()){
        cb_marker->setCurrentIndex(idx_landmark+1);
        extract();
    }else{
        v3d_msg("Reach the end of marker list.");
    }
    checkButtons();
}


void neuronPickerDialog::checkButtons()
{
    if(image1Dc_in==0){
        btn_update->setEnabled(false);
        btn_extract->setEnabled(false);
        btn_save->setEnabled(false);
        btn_next->setEnabled(false);
    }else{
        bool winfound=false;
        v3dhandleList v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                winfound=true;
            }
        }
        if(winfound){
            btn_update->setEnabled(true);
        }
        if(cb_marker->count()>0){
            btn_extract->setEnabled(true);
            if(fname_outbase.isEmpty() || image1D_out==0){
                btn_save->setEnabled(false);
                btn_next->setEnabled(false);
            }else{
                btn_save->setEnabled(true);
                btn_next->setEnabled(true);
            }
        }else{
            btn_extract->setEnabled(false);
            btn_next->setEnabled(false);
            btn_save->setEnabled(false);
        }
    }
}

void neuronPickerDialog::updateInputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it
        qDebug()<<"cojoc: image4d: "<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2]<<":"<<sz_img[3];
        Image4DSimple image4d;
        image4d.setFileName(fname_input.toStdString().c_str());
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1Dc_in, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);
        if(!winfound){ //open a window if none is open
            qDebug()<<"cojoc: creat window";
            v3dhandle v3dhandle_main=callback->newImageWindow();
            //update the image
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_INWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
        }else{
            qDebug()<<"cojoc: refresh content";
            v3dhandleList_current=callback->getImageWindowList();
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
                {
                    //update the image
                    if(!callback->setImage(v3dhandleList_current[i], &image4d)){
                        v3d_msg("Failed to update input image");
                    }
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    //callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }else if(winfound){
        //no image loaded, but there is a window? clean up the contents
        //clean up landmarks
        LandmarkList LList_empty;
        //clean up window
        Image4DSimple image4d;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                callback->setLandmark(v3dhandleList_current[i], LList_empty);
                callback->setImage(v3dhandleList_current[i], &image4d);
                callback->updateImageWindow(v3dhandleList_current[i]);
                callback->pushImageIn3DWindow(v3dhandleList_current[i]);
            }
        }
    }
}

void neuronPickerDialog::updateOutputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
        }
    }
    if(image1D_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
        unsigned char* image1D_input=neuronPickerMain::memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1D_out, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], 1, V3D_UINT8);

        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
        }else{
            //update the image
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
                {
                    callback->setImage(v3dhandleList_current[i], &image4d);
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    //callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }else if(winfound){
        //no image loaded, but there is a window? clean up the window content
        Image4DSimple image4d;
        unsigned char *image1D_input=neuronPickerMain::memory_allocate_uchar1D(1);
        image1D_input[0]=0;
        image4d.setData(image1D_input,1,1,1,1,V3D_UINT8);
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
            {
                callback->setImage(v3dhandleList_current[i], &image4d);
                callback->updateImageWindow(v3dhandleList_current[i]);
                //callback->close3DWindow(v3dhandleList_current[i]);
                callback->pushImageIn3DWindow(v3dhandleList_current[i]);
            }
        }
    }
}
