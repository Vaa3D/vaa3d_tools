

#ifndef __MBRAINALIGNER_H__
#define __MBRAINALIGNER_H__


#include "until.h"

using namespace std;

#define WANT_STREAM

bool q_mutualinformation2(float *&I1, float *&I2, long long npixels, float &nmi);
//same as hanchuan's jab method (assume invalid pixel intensity<0)
bool q_mutualinformation3(float *&I1, float *&I2, int kernel_radius, float &nmi);

//normalized Cross Correlation [0,1] (assume invalid pixel intensity<0)
bool q_ncc(float *&I1, float *&I2, long long npixels, float &ncc);

//mean square diffence error [0,1] (assume invalid pixel intensity<0)
bool q_mse(float *&I1, float *&I2, long long npixels, float &mse);

//linear interpolate the SubDFBlock to DFBlock
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_linear(DisplaceFieldF3D ***&pppSubDF,
	const V3DLONG szBlock_x, const V3DLONG szBlock_y, const V3DLONG szBlock_z,
	const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock);

//bspline interpolate the DF block
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
bool q_dfblcokinterp_bspline(DisplaceFieldF3D ***&pppSubDF, const Matrix &x_bsplinebasis,
	const V3DLONG sz_gridwnd, const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock);

bool q_TPS_cd(const vector<point3D64F> &vec_sub, const vector<point3D64F> &vec_tar, const double d_lamda,
	Matrix &x4x4_d, Matrix &xnx4_c, Matrix &xnxn_K);

//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub, const vector<point3D64F> &vec_basis, Matrix &xmxn_K);

bool q_imagewarp_stps(const vector<point3D64F> &vec_ctlpt_tar, const vector<point3D64F>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	unsigned char *p_img_stps);

bool q_compute_ptwarped_from_stps_3D(
	const point3D64F &pt_sub, const vector <point3D64F> &vec_ctlpt_sub, Matrix &x4x4_d, Matrix &xnx4_c,
	point3D64F &pt_sub2tar);

bool get_tar_patch(int Select_modal, vector<float *> &vec_pkernels, vector<float *> &vec_pkernels_Hog, vector<float *> &vec_pkernels_map, vector<float *> &vec_pkernels_Hog_map,
	vector<point3D64F>& vec_corners, long long & npixels_kernel, int & kernel_radius, int & kernel_width, long long * & sz_img_tar, float **** &p_img_label,
	float **** & p_img32f_tar_4d, long long &patch_size_all);

bool Compute_source_fMOST(float * & p_kernel_map, float *p_kernel, int & kernel_radius, long long *& sz_img_tar, vector<point3D64F>& vec_corners, vector<point3D64F> & aver_corner,
	vector<point3D64F> &or_sub, float **** & p_img32f_sub_4d, float **** & p_img_label, int  ind_landmark, bool select,
	long long  npixels_kernel, long long  index_fetmap, long long  index_source, int X, int  Y, int  Z, vector<float *> & Source, vector< float * > & vec2d_pfeatmaps,
	float  *&p_kernel_Hog_map, int  kernel_width, float  *& p_kernel_Hog, long long & patch_size_all,
	vector<point3D64F> &vec_corners_last, map <int, float ****> density_map_sub_4d, double outline, float  ****&fmost_label_edge_4d);

bool Compute_source(float * & p_kernel_map, float *p_kernel, int & kernel_radius, long long *& sz_img_tar, vector<point3D64F>& vec_corners, vector<point3D64F> & aver_corner,
	vector<point3D64F> &or_sub, float **** & p_img32f_sub_4d, float **** & p_img_label, int  ind_landmark, bool select,
	long long  npixels_kernel, long long  index_fetmap, long long  index_source, int  X, int  Y, int  Z, vector<float *> & Source, vector< float * > & vec2d_pfeatmaps, float  *&p_kernel_Hog_map,
	int & kernel_width, float  *& p_kernel_Hog, long long & patch_size_all, vector<point3D64F> &vec_corners_last);

bool Dic_brain(unsigned char * p_img_test, QString &dic_brain_file, QString &based_brain, long long * sz_img_test);

bool mul_scale_mBrainAligner(Parameter input_Parameter, vector<point3D64F>vec_corners, vector<point3D64F> fine_sub_corner,
	vector<point3D64F> aver_corner, vector<int> label, long long * sz_img_tar,  float * p_img32f_tar,
	float * p_img32f_sub_bk, float * p_img32_sub_label, unsigned char * p_img_sub, map <int, float *> & density_map_sub);

bool mBrainAligner(Parameter input_Parameter, vector<point3D64F>vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> aver_corner,
	vector<int> label, long long * sz_img_tar, float * p_img32f_tar, float * p_img32f_sub_bk, float * p_img32_sub_label,
	unsigned char * p_img_sub, map <int, float *> & density_map_sub);

bool downsapmle_3Dmarker(vector<point3D64F> & out_marker, vector<point3D64F> input_marker, int resample);

#endif
