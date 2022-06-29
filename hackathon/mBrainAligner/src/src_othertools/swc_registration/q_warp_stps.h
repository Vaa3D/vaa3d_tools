// q_warp_stps.h
// warp pointset and image based on given matched pairs
// by Lei Qu
// 2010-03-22

#ifndef __Q_WARP_STPS_H__
#define __Q_WARP_STPS_H__


#include <vector>
using namespace std;
#define WANT_STREAM
#include "../../jba/newmat11/newmatap.h"
#include "../../jba/newmat11/newmatio.h"
#include "q_warp_common.h"

//STPS image warp
//compute the affine term d and nonaffine term c which decomposed from TPS (tar=sub*d+K*c)
//more stable compare to the q_affine_compute_affinmatrix_3D()
bool q_stps_cd(const vector<Coord3D_PCM> &vec_sub, const vector<Coord3D_PCM> &vec_tar, const double d_lamda,
	Matrix &x4x4_d, Matrix &xnx4_c, Matrix &xnxn_K);
//generate TPS kernel matrix
bool q_stps_k(const vector<Coord3D_PCM> &vec_sub, const vector<Coord3D_PCM> &vec_basis, Matrix &xmxn_K);

//stps points warp
bool q_compute_ptwarped_from_stps_3D(const Coord3D_PCM &pt_sub, const vector <Coord3D_PCM> &vec_ctlpt_sub, Matrix &x4x4_d, Matrix &xnx4_c,
	Coord3D_PCM &pt_sub2tar);

//stps image warp
bool q_imagewarp_stps(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	Matrix &x4x4_d, Matrix &xnx4_c, Matrix &xnxn_K,
	unsigned char *&p_img_stps);
bool q_imagewarp_stps(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	unsigned char *&p_img_stps);


#endif

