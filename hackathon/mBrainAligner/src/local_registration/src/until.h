// q_bspline.h
// by Lei Qu
// 2012-07-23

#ifndef __UNTIL_H__
#define __UNTIL_H__

//#include "jba_mainfunc.h"

#include <vector>
#include <string>
#include <qstring.h>

#include <stdio.h>
#include <time.h>
#include "stackutil.h"
#include "basic_surf_objs.h"
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "getopt.h"
#include "q_imgwarp_tps_quicksmallmemory.cpp"
#include "opencv2/opencv.hpp"
#include "calHogFeature.h"
#include "q_derivatives3D.h"

using namespace std;

#define WANT_STREAM

class point3D64F
{
public:
	double x, y, z, label, outline, offer;
	point3D64F() { x = y = z = label = outline = 0.0, offer = 0.0; }
	point3D64F(double x0, double y0, double z0, double label0, double outline0, double offer0) { x = x0; y = y0; z = z0; label = label0; outline = outline0; offer = offer0; }
};
class neighborinfo
{
public:
	int ind;
	float dis;
	int ind_reg;
	neighborinfo() { ind = -1; dis = 10e+10; ind_reg = -1; }
	neighborinfo(int ind0, float dis0, int ind1) { ind = ind0; dis = dis0; ind_reg = ind1; }
};

struct CellParam
{
	vector<float> bin;
};

struct BlockParam
{
	vector<float> bin;
};

class Parameter
{
public:
	int  Select_modal, iterations_number, star_lamda_outline, kernel_radius, search_radius, fre_save, fre_global_constraint,
		fre_region_constraint, star_iter, lam_end_inner, lam_end_out, resample, star_lamda_inner;
	QString save_path,landmark_path;
	Parameter() {
		Select_modal = 0;
		iterations_number = 0;
		star_lamda_outline = 0;
		kernel_radius = 0;
		search_radius = 0;
		fre_save = 0;
		fre_region_constraint = 0;
		fre_global_constraint = 0;
		star_iter = 0;
		lam_end_inner = 0;
		lam_end_out = 0;
		resample = 1;
		star_lamda_inner = 0;
		save_path = "";
		landmark_path = "";
	}
	Parameter(int Select_modal0, int iterations_number0, int star_lamda_outline0, int kernel_radius0, int search_radius0, int fre_save0,
		int fre_region_constraint0, int fre_global_constraint0, int star_iter0, int lam_end_inner0, int lam_end_out0,
		int resample0, int star_lamda_inner0,  QString save_path0, QString landmark_path0)
	{
		Select_modal = Select_modal0;
		iterations_number = iterations_number0;
		star_lamda_outline = star_lamda_outline0;
		kernel_radius = kernel_radius0;
		search_radius = search_radius0;
		fre_save = fre_save0;
		fre_region_constraint = fre_region_constraint0;
		fre_global_constraint = fre_global_constraint0;
		star_iter = star_iter0;
		lam_end_inner = lam_end_inner0;
		lam_end_out = lam_end_out0;
		resample = resample0;
		star_lamda_inner = star_lamda_inner0;
		save_path = save_path0;
		landmark_path = landmark_path0;
	}
};

#endif

