// q_rigidaffine_registration.h
// by Lei Qu
// 2011-04-08

#ifndef __Q_RIGIDAFFINE_REGISTRATION_H__
#define __Q_RIGIDAFFINE_REGISTRATION_H__

#include <vector>
using namespace std;

#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"

#include "q_registration_common.h"

//------------------------------------------------------------------------------------------------------------------------------------
//rigid and affine registration
//
//input paras:
//	p_img64f_tar/sub:	input target and subject image should be 1 channel 3D image stack of double type
//	sz_img[4]:			input target and subject image size, they are supposed to be same and sz_img[3]==1 (1 channel)
//
//output paras:
//	vec4D_grid:			deformed grid
//
//note:
//rigid registration is much faster than the affine one, especially when the image is big
//the reason is rigid transform fitting only need to do a 3x3 matrix SVD decomposion, no matter how big the image is
//but the affine transform fitting need to do a 3nx14 matrix SVD decomposion, where n is image pixel number
//
bool q_rigidaffine_registration(const CParas_reg &paras,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		vector< vector< vector< vector<double> > > > &vec4D_grid);


//------------------------------------------------------------------------------------------------------------------------------------
//initial rectangular grid generator (used to model rigid/affine distortion)
bool q_rigidaffine_inigrid_generator(const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid);

//rigid interpolate the 3D grid to transformation field
bool q_rigidaffine_grid2field_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd_input[3],const long sz_gridwnd_output[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_int);
//inverse rigid interpolate the 3D transformation field to grid (least square fitting)
bool q_rigidaffine_field2grid_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid);

//compute the rigid (actually is similar) transformation matrix B=T*A
bool q_rigidaffine_compute_rigidmatrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_rigidmatrix);
//compute the affine transformation matrix B=T*A
bool q_rigidaffine_compute_affinematrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_affinematrix);

//given input image and transformation field, linear interpolate the warpped image (table lookup)
bool q_rigidaffine_interpolate_3D(const unsigned char *p_img8u_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		unsigned char *&p_img8u_output);
bool q_rigidaffine_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		double *&p_img64f_output);

//compute the similarity error (SSD) of two images
bool q_rigidaffine_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
		double &d_similarity);

//compute the rigid control grid nodes increment dE/dT (for Gradient Descent Method)
bool q_rigidaffine_gridnodes_dT_3D(const int i_regtype,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_dT);

//update grid nodes position: T_tmp=T-gamma*dT
bool q_rigidaffine_gridnodes_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update);

//save deformd meshgrid to swc file (offset of nodes are indicate by lines)
bool q_rigidaffine_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd[3],
		const char *filename);

//generate affine warped subject image according to the deformed grid
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const unsigned char *p_img8u_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		unsigned char *&p_img8u_sub_warp);
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		double *&p_img64f_sub_warp);


#endif

