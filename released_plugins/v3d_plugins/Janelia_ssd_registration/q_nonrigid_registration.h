// q_nonrigid_registration.h
// by Lei Qu
// 2010-02-16

#ifndef __Q_NONRIGID_REGISTRATION_H__
#define __Q_NONRIGID_REGISTRATION_H__

#include <vector>
using namespace std;

#define WANT_STREAM
#include "../../jba/newmat11/newmatap.h"
#include "../../jba/newmat11/newmatio.h"


bool q_nonrigid_registration_FFD(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long l_hierarchlevel,const long sz_gridwnd,
		vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_warpimage_baseongrid(const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd,
		double *&p_img64f_sub_warp);

bool q_ini_bsplinebasis_3D(const long n,Matrix &BxBxB);

bool q_inigrid_generator(const long sz_img[4],const long sz_gridwnd,
		vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_img_embed(const double *p_img64f,const long *sz_img64f,
				 const long sz_gridwnd,
				 const double d_value4invalid,
				 double *&p_img64f_embed,long *sz_img64f_embed);

bool q_bspline_grid2field_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid_int);
bool q_bspline_field2grid_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		double *&p_img64f_output);

bool q_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
		double &d_similarity);

bool q_gridnodes_dT_3D(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid_gradient);

bool q_grid_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,const vector< vector< vector< vector<double> > > > &vec4D_grid_ori,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,const double d_gaussian_sigma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update);

bool q_grid_subdivide_3D(vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_save64f01_image(const double *p_img64f,const long sz_img[4],const char *filename);

bool q_savegrid_apo(const vector< vector< vector< vector<double> > > > &vec4D_grid,const double d_volsize,
		const char *filename);
bool q_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd,
		const char *filename);

#endif

