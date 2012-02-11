// q_warp_affine_tps.h
// warp pointset and image based on given matched pairs
// by Lei Qu
// 2010-03-22

#ifndef __Q_WARP_AFFINE_TPS_H__
#define __Q_WARP_AFFINE_TPS_H__


#include <vector>
using namespace std;
#define WANT_STREAM
#include "../../jba/newmat11/newmatap.h"
#include "../../jba/newmat11/newmatio.h"


class Coord3D_PCM
{
public:
	double x,y,z;
	Coord3D_PCM(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCM() {x=y=z=0;}
};



//affine image warp
bool q_imagewarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar,const vector<Coord3D_PCM>  &vec_ctlpt_sub,
		const unsigned char *p_img_sub,const long *sz_img_sub,
		unsigned char *&p_img_affine);
//TPS image warp
bool q_imagewarp_tps(const vector<Coord3D_PCM> &vec_ctlpt_tar,const vector<Coord3D_PCM>  &vec_ctlpt_sub,
		const unsigned char *p_img_sub,const long *sz_img_sub,
		unsigned char *&p_img_tps);


//Read matched-pair index file
//	output vec2D_sub2tar_matchind is a 2D (n*2) vector
//		vec2D_sub2tar_matchind[i][0]: sub index of i-th matched pair
//		vec2D_sub2tar_matchind[i][1]: tar index of i-th matched pair
bool q_readMatchInd_file(const QString qs_filename,vector< vector<long> > &vec2D_sub2tar_matchind);


//centrilize and scale the point set
bool q_normalize_points_2D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x3x3_normalize);
bool q_normalize_points_3D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize);
//compute the affine matraix
//B=T*A
bool q_affine_compute_affinmatrix_2D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x3x3_affinematrix);
bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x4x4_affinematrix);


Matrix q_pseudoinverse(Matrix inputMat);
//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_3D(
		const Coord3D_PCM &pt_sub,const vector <Coord3D_PCM> &vec_ctlpt_sub,const Matrix &wa_sub2tar,
		Coord3D_PCM &pt_sub2tar);
//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_3D(
		const vector <Coord3D_PCM> &pos_controlpoint_sub,const vector <Coord3D_PCM> &pos_controlpoint_tar,
		Matrix &wa);


#endif

