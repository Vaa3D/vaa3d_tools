/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//bdb_minus.h
// This is the BDB-minus method C/C++ version declaration header
//by Hanchuan Peng
//2008-03-05
//
// Note: many functions and classes' definitions are the same with PALM_C project
//last change: 080316. update the Size3D & Size2D class
//2008-05-12: overloading the multiply, add, minus, and divide operators. Seems slower, thus don't add
//2008-07-12: add a function point_bdb_minus_2d_localwinmass()
//2008-08-22: add the function for fly brain segmentation point_bdb_minus_2d_localwinmass_bl()
//

#ifndef __BDB_MINUS__
#define __BDB_MINUS__

#include "../basic_c_fun/basic_memory.cpp"
#include "../basic_c_fun/basic_landmark.h"
#include "../basic_c_fun/img_definition.h"

#include <vector>
#include <list>
#include <string>

using namespace std;
string gen_file_name(string basename, int layer, string otherinfo);

#include "graph.h"


class BasicEstParameter
{
public:
	char *dfile_input;
	char *dfile_output;
	char *dfile_parameter;
	bool b_verbose_print;

	BasicEstParameter()
	{
	  b_verbose_print=false;
	  dfile_input=NULL;
	  dfile_output=NULL;
	  dfile_parameter=NULL;
	}
};

class Coord2D
{
public:
    MYFLOAT x,y; //should be fine because the real coordinates should between 0~2047
	static int dim;
};

class Coord3D
{
public:
    MYFLOAT x,y,z; //should be fine because the real coordinates should between 0~2047
	static int dim;
};


class BDB_Minus_ConfigParameter
{
public:
  double f_image, f_smooth, f_length; //the coefficients for the image force, smoothness and length forces, respectively.
  double Kfactor; //the factor in "mean+Kfactor*sigma" of an image that would be used to find the backbone
  bool b_adjust_tip;
  int nloops;
  int radius;
  int radius_x, radius_y;
  double TH; //convergence threshold

  BDB_Minus_ConfigParameter()
  {
    f_image=1.0;
	f_smooth=1.0;
	f_length=0.5;
	Kfactor=1.0;
	b_adjust_tip = true;
	nloops=500;
	  radius=20;
	  radius_x=20; radius_y=10;
  }
};

struct Basic_Path_Statistics
{
	double ave;
	double adev;
	double sdev;
	double var;
	double skew;
	double curt;
};

//other enabling functions

template <class T> bool imfilter(MYFLOAT ** outimg2d, T ** inimg2d, V3DLONG s0, V3DLONG s1, MYFLOAT ** ker, V3DLONG s0_ker, V3DLONG s1_ker, unsigned char b_negative);

template <class T> bool vol2d_max(Image2DSimple<T> *target, T & peakVal, V3DLONG & xpos, V3DLONG & ypos);

template <class T> bool vol2d_max(T & res, T ** sa, V3DLONG d0, V3DLONG d1);
template <class T> bool vol2d_max_range(T & res, T ** sa, V3DLONG d0, V3DLONG d1, V3DLONG d0m, V3DLONG d0M, V3DLONG d1m, V3DLONG d1M);

template <class T> bool vol2d_setValue(T ** target, T val, V3DLONG d0, V3DLONG d1);
template <class T> bool vol2d_setValue(T ** target, T ** mask, T val, V3DLONG d0, V3DLONG d1);
template <class T1, class T2, class T3> bool vol2d_setValue(Image2DSimple<T1> *target, Image2DSimple<T2> * mask, T3 val, V3DLONG d0m, V3DLONG d0M, V3DLONG d1m, V3DLONG d1M);

template <class T> bool saveImage2DSimple2RawFloat(Image2DSimple <T> *img, string outbasefilename);

template <class T> T hardLimit(T a, T lowerBound, T upperBound);
template <class T> void swapVal(T & a, T & b);

template <class T> list <Coord2D> * main_simple_BDB_minus(Image2DSimple<T> * inimg, const BDB_Minus_ConfigParameter & mypara);

V3DLONG * mst_parents_list(Image2DSimple <MYFLOAT> * fullConnectMatrix); //return a vector of V3DLONG type, each is the parent node index
Image2DSimple<MYFLOAT> * mst_fullgraph(V3DLONG * parentsList, V3DLONG len);
V3DLONG * img_mst_diameter(Image2DSimple<MYFLOAT> * m, V3DLONG & length_diameter);

template <class T> bool point_bmeans_2d(const vector <Coord2D> & xypos, T ** inimg_data2d, vector <Coord2D> * mCoord_out, double TH, const BDB_Minus_ConfigParameter & mypara);
template <class T> bool point_bmeans_3d(const vector <LocationSimple> & xypos, T *** inimg_data2d, vector <LocationSimple> * mCoord_out, double TH, const BDB_Minus_ConfigParameter & mypara);

bool point_bdb_minus_2d_localwinmass(unsigned char ** inimg_data2d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, vector <Coord2D> & mCoord_out, const BDB_Minus_ConfigParameter & mypara);
bool point_bdb_minus_2d_localwinmass_bl(unsigned char ** inimg_data2d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, vector<Coord2D> & mCoord_in, vector<Coord2D> &mCoord_out, const BDB_Minus_ConfigParameter & mypara);

bool point_bdb_minus_3d_localwinmass(unsigned char *** inimg_data3d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, V3DLONG inimg_sz2, vector <Coord3D> & mCoord_out, const BDB_Minus_ConfigParameter & mypara);
bool path_statistics_3d(Basic_Path_Statistics & bps, unsigned char *** inimg_data3d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, V3DLONG inimg_sz2, vector <Coord3D> & mCoord);

class BFSClass;
BFSClass * bfs_1root(Image2DSimple <MYFLOAT> * fullConnectMatrix, V3DLONG rootnode);

bool save_estResult(list <Coord2D> * my_est);


//definition of the template related functions

//template <class T>
//bool point_bdb_minus_3d_localwinmass_prior(unsigned char *** inimg_data3d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, V3DLONG inimg_sz2,
//		vector <T> & mCoord_out, const BDB_Minus_ConfigParameter & mypara,
//		const vector <T> & mCoord_prior, bool b_fix_end); // 090520 RZC, 090619 RZC move it to neuron_tracing.h

template <class T> bool vol2d_mean_and_std(Image2DSimple<T> *target, double & ave, double & sdev);
template <class T> bool vol3d_mean_and_std(Vol3DSimple<T> *target, double & ave, double & sdev);
template <class T> bool data_array_mean_and_std(T *data, V3DLONG n, double & ave, double & sdev);
bool find_mst_diameter(const vector <LocationSimple> & ptCoord, vector<LocationSimple> & diameterCoord, vector<PtIndexAndParents> & parentList);

//resampling function
bool straight_nearestfill(UINT8_TYPE * invol1d, V3DLONG *insz, int szlen,
                          double *bposx, double *bposy, double *alpha, V3DLONG cutPlaneNum,
						  V3DLONG OutWid, //a good default value could be 160
						  UINT8_TYPE * & outvol1d, V3DLONG *&outdims);


//Implementation of some template based functions

template <class T> bool vol2d_mean_and_std(Image2DSimple<T> *target, double & ave, double & sdev)
{
    if (!target || !target->valid())
	  return false;

    T * data = target->getData1dHandle();
	V3DLONG n = target->getTotalElementNumber();

	V3DLONG j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = double(data[0]);
	  sdev = (double)0;
	  return true; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	double ave_double=(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(sqrt(var));
	ave=ave_double; //use ave_double for the best accuracy

	return true;
}

template <class T> bool vol2d_mean_and_std(Vol3DSimple<T> *target, double & ave, double & sdev)
{
    if (!target || !target->valid())
	  return false;

    T * data = target->getData1dHandle();
	V3DLONG n = target->getTotalElementNumber();

	V3DLONG j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = double(data[0]);
	  sdev = (double)0;
	  return true; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	double ave_double=(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(sqrt(var));
	ave=ave_double; //use ave_double for the best accuracy

	return true;
}


template <class T> bool data_array_mean_and_std(T *data, V3DLONG n, double & ave, double & sdev)
{
    if (!data || n<=0)
	  return false;

	V3DLONG j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = double(data[0]);
	  sdev = (double)0;
	  return true; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++)
	{
		s += data[j];
	}
	double ave_double=(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(sqrt(var));
	ave=ave_double; //use ave_double for the best accuracy

	return true;
}



#endif

