// q_tps.h
// by Lei Qu
// 2019-05-01

#ifndef __Q_TPS_H__
#define __Q_TPS_H__

#include <QtGui>
#include <vector>
using namespace std;

#define WANT_STREAM
#include "newmatap.h"
#include "newmatio.h"

#include "basic_surf_objs.h"
#include "v3d_interface.h"

class point3D64F
{
public:
	double x,y,z;
	point3D64F() {x=y=z=0.0;}
	point3D64F(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
};


Matrix q_pseudoinverse(Matrix inputMat);

//normalize 3D point cloud
bool q_normalize_points_3D(const vector<point3D64F> &vec_input,vector<point3D64F> &vec_output,Matrix &x4x4_normalize);
//compute the rigid(actually is similar) transform matrix B=T*A
bool q_compute_rigidmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_rigidmatrix);
//compute the affine transform matrix B=T*A
bool q_compute_affinmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_affinematrix);

//compute the affine term d and nonaffine term c which decomposed from TPS (tar=sub*d+K*c)
//more stable compare to the q_affine_compute_affinmatrix_3D()
bool q_TPS_cd(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_tar,const double d_lamda,
		Matrix &x4x4_d,Matrix &xnx4_c,Matrix &xnxn_K);
//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_basis,Matrix &xmxn_K);

//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_3D(
		const point3D64F &pt_sub,const vector <point3D64F> &vec_ctlpts_sub,const Matrix &wa_sub2tar,
		point3D64F &pt_sub2tar);
//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_3D(
		const vector <point3D64F> &vec_ctlpts_sub,const vector <point3D64F> &vec_ctlpts_tar,
		Matrix &wa);


#endif
