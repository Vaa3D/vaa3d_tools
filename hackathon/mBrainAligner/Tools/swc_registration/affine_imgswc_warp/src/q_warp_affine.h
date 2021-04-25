// q_warp_affine.h
// warp pointset and image based on given matched pairs
// by Lei Qu
// 2019-09-26

#ifndef __Q_WARP_AFFINE_H__
#define __Q_WARP_AFFINE_H__


#include <vector>
using namespace std;
#define WANT_STREAM
#include "../../jba/newmat11/newmatap.h"
#include "../../jba/newmat11/newmatio.h"
#include "q_warp_common.h"


//centrilize and scale the point set
bool q_normalize_points_2D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x3x3_normalize);
bool q_normalize_points_3D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize);
//compute the affine matraix
//B=T*A
bool q_affine_compute_affinmatrix_2D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x3x3_affinematrix);
bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x4x4_affinematrix);

//affine image warp
bool q_imagewarp_affine(const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	const Matrix x4x4_affinematrix, unsigned char *&p_img_affine);
bool q_imagewarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	Matrix &x4x4_affinematrix, unsigned char *&p_img_affine);
bool q_imagewarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	unsigned char *&p_img_affine);
//affine points warp
bool q_ptswarp_affine(const vector<Coord3D_PCM>  &vec_ctlpt_sub, const Matrix x4x4_affinematrix,
	vector<Coord3D_PCM> &vec_ctlpt_subtar_affine);
bool q_ptswarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	vector<Coord3D_PCM> &vec_ctlpt_subtar_affine);

#endif

