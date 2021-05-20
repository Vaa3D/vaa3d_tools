#ifndef __Q_NONRIGID_REGISTRATION_H__
#define __Q_RIGIDAFFINE_REGISTRATION_H__

#include <vector>
using namespace std;

#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"
#include "until.h"


bool q_nonrigid_warpimage_baseongrid(const unsigned char *p_img8u_sub,const long long sz_img_sub[4], const vector< vector< vector< vector<double> > > > &vec4D_grid,
	const long Spacing, unsigned char *&p_img8u_sub_warp);

bool q_bspline_grid2field_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
	const long Spacing, const Matrix &x_bsplinebasis,
	vector< vector< vector< vector<double> > > > &vec4D_grid_int);

bool T_bsplinebasis2grid_3D(const long sz_grid[3], const long Spacing, const vector<point3D64F> &vec_tar, const vector<point3D64F> &vec_Diff,
	vector< vector< vector< vector<double> > > > &vec4D_grid_tmp, vector< vector< vector< vector<double> > > > &vec4D_grid);


bool q_nonrigid_interpolate_3D(const unsigned char *p_img8u_input, const long sz_img_input[4],
	const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
	unsigned char *&p_img8u_output);

bool T_bsplines_registration(const long long sz_img_sub[4],  vector<point3D64F> &vec_tar,  vector<point3D64F> &vec_sub,
	vector<point3D64F> &vec_new, vector< vector< vector< vector<double> > > > &vec4D_grid, long &New_Spacing, long long & number_outline_landmark);

bool T_bsplinebasis2grid_3D(const long sz_grid[3], const long Spacing, const vector<point3D64F> &vec_tar, const vector<point3D64F> &vec_Diff,
	vector< vector< vector< vector<double> > > > &vec4D_grid_tmp, vector< vector< vector< vector<double> > > > &vec4D_grid);

bool T_bsplines_wrapmarker(const long Spacing, const vector<point3D64F> &vec_tar, const vector<point3D64F> &vec_sub, vector< vector< vector< vector<double> > > > &vec4D_grid,
	vector<point3D64F> &vec_warp, vector<point3D64F> &vec_new, long long & number_outline_landmark);

bool bspline_coefficients(const double ax, const double ay, const double az, Matrix &BxBxB);

bool T_bsplines_RefineGrid(const long Old_Spacing, long sz_grid[3], const long long sz_img_sub[4], const vector< vector< vector< vector<double> > > > &vec4D_grid_tmp,
	vector< vector< vector< vector<double> > > > &vec4D_grid_df, vector< vector< vector< vector<double> > > > &vec4D_grid, long &New_Spacing);

bool q_image_ini_bsplinebasis_3D(const long n, Matrix &BxBxB);

bool q_nonrigid_inigrid_generator(const long long sz_img[4], const long Spacing, vector< vector< vector< vector<double> > > > &vec4D_grid);
#endif