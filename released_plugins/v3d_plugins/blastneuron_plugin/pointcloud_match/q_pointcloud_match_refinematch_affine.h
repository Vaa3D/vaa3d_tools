// q_pointcloud_match_refinematch_affine.h
// affine based refine matching
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.h by Lei Qu on 2010-01-22

#ifndef __Q_POINTCLOUD_MATCH_AFFINE_H__
#define __Q_POINTCLOUD_MATCH_AFFINE_H__

#include "q_pointcloud_match_basic.h"


//affine based refine matching
//base on the initial mathcing result, this fuction try to minimize the affine inverse projection error
//	(0). the two input points sets A and B should be normalized and also satisfy size(A) <= size(B)
//	(1). kill all the multi-matched pairs before running RANSAC (although one of them may be right!)
//	(2). find the best affine matrix based on initial matching result using RANSAC
//	(3). refine the affine matrix and the matching by introducing a new voter - dis_A2invB
//
bool q_pointcloud_match_refinematch_affine(const vector<int> &vec_A2Bindex,
		const vector<Coord3D_PCM> &vec_A_norm,const vector<Coord3D_PCM> &vec_B_norm,
		vector< vector<double> > &vecvec_votematrix,
		const int n_sampling, const int n_pairs_persampling,const int n_affineweight,
		vector<double> &vec_A2B_cost,
		Matrix &x4x4_affinematrix,
		vector<int> &vec_A2Bindex_refined);


//find the best affine matrix based on initial matching result using RANSAC
//
//   perform RANSAC on the initial matching result to find the affine matrix
//   with minimal inverse projection error
//
//   1. chose 4 pairs from initial matching result randomly
//   2. estimate the affine matrix: A=T*B
//   3. project the B to A
//   4. compute the inverse projection error
//   5. loop 1~4 n_sampling times
//   6. find the affine matrix and the dis_matrix with minimal inv-projection error
//
bool q_affine_estimate_affinmatrix_RANSAC(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,
		const vector<int> &arr1d_A2Bindex,
		const int n_sampling,const int n_pairs_per_sampling,
		Matrix &x4x4_affinematrix);

//refine the affine matrix and the rematching by introducing a new voter - dis_A2Binvp
//
//   1.compute the distant of A to Binvp (from the best affine matrix) --> dis_matrix_A2Binvp
//   2.take the distant of A to Binvp as a new voter to vote again
//   	(we give the new voter more weight than the shape context based ones)
//   3.choose the top voted 5+i candidates (i represent current iteration times) ?(how to deal with cross match)?
//   	i.e. this means that we will increase the matched pair 1 by 1. in this
//   	loop process, no matching pair is fixed, they allow to change in order
//   	to make the global inverse projection error minimal
//   4.use the choosed 5+i pairs to refine the affine matrix and matching, then return to 1 to loop
//
bool q_affine_refineaffinematching_disA2Binvp_voting(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,
		const int n_affineweight,
		vector< vector<double> > &vecvec_votematrix,
		vector<int> &vec_A2Bindex, vector<double> &vec_A2B_cost,
		Matrix &x4x4_affinematrix);

//compute the affine matraix
//A=T*B
bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x4x4_affinematrix);

//compute the inverse projection error
//invp_err = sum(distant of all A to its nearest B_invp)
bool q_affine_compute_invp_err(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,const Matrix &x4x4_affinematrix,
		vector< vector<double> > &arr_dismatrix,double &d_invp_err);


#endif

