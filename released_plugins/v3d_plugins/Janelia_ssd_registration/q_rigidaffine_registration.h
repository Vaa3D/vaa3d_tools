// q_rigidaffine_registration.h
// by Lei Qu
// 2011-04-08

#ifndef __Q_RIGIDAFFINE_REGISTRATION_H__
#define __Q_RIGIDAFFINE_REGISTRATION_H__

#include <vector>
using namespace std;

#define WANT_STREAM
#include <newmatap.h>
#include <newmatio.h>

#include "q_registration_common.h"

bool q_rigidaffine_registration(const CParas_reg &paras,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		vector< vector< vector< vector<double> > > > &vec4D_grid);


bool q_rigidaffine_inigrid_generator(const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_rigidaffine_grid2field_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd_input[3],const long sz_gridwnd_output[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_int);
bool q_rigidaffine_field2grid_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid);

bool q_rigidaffine_compute_rigidmatrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_rigidmatrix);
bool q_rigidaffine_compute_affinematrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_affinematrix);

bool q_rigidaffine_interpolate_3D(const unsigned char *p_img8u_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		unsigned char *&p_img8u_output);
bool q_rigidaffine_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		double *&p_img64f_output);

bool q_rigidaffine_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
		double &d_similarity);

bool q_rigidaffine_gridnodes_dT_3D(const int i_regtype,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_dT);

bool q_rigidaffine_gridnodes_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update);

bool q_rigidaffine_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd[3],
		const char *filename);

bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const unsigned char *p_img8u_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		unsigned char *&p_img8u_sub_warp);
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		double *&p_img64f_sub_warp);


#endif

