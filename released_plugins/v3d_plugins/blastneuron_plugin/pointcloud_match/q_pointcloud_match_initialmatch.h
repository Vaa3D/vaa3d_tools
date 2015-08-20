// q_pointcloud_match_initialmatch.h
// shape context based initial matching
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.h by Lei Qu on 2010-01-22

#ifndef __Q_POINTCLOUD_MATCH_INITIALMATCH_H__
#define __Q_POINTCLOUD_MATCH_INITIALMATCH_H__

#include "q_pointcloud_match_basic.h"


//shape context and voting mechanism based initial matching
//	(0). the two input points sets A and B should satisfy size(A) <= size(B)
//	(1). normalize two point sets (centrilize and scaling)
//	(2). compute the shape context of two input point sets - dismatrix
//			the shape context is the set of distance (Euclidian or Geodesic) of a point to all others points in a point set
//	(3). rescaling the dismatrix of B to A according to the average pairwise Geodesic distance (this step is skipped under Euclidian mode)
//	(4). compute the context similarity of every possible pair of two point sets using different methods - simmatrix
//	(5). vote the best several candidates
//	(6). do initial match - find the top voted candidate in B for every A
//	
bool q_pointcloud_match_initialmatch(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,
		const int n_method,const int K, const int arr_voterweight[100],const int n_bin,
		vector<Coord3D_PCM> &vec_A_norm,vector<Coord3D_PCM> &vec_B_norm,
		Matrix &x4x4_A_norm,Matrix &x4x4_B_norm,
		vector< vector<double> > &vecvec_dismatrix_A,vector< vector<double> > &vecvec_dismatrix_B,
		vector< vector<double> > &vecvec_votematrix,
		vector<int> &vec_A2Bindex);

//compute and sort the distant context information
bool q_compute_dismatrix(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix);
//compute the Euclidian distance of every possible pair of input point set -> dismatrix
bool q_compute_dismatrix_euclidian(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix);
//compute the Geodesic distance of every possible pair of input point set -> dismatrix
bool q_compute_dismatrix_geodesic(const vector<Coord3D_PCM> vec_input,const int K,vector< vector<double> > &vecvec_dismatrix);

//compute the context similarity - NN interpolate
bool q_compare_context_nn(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,vector< vector<double> > &vecvec_simmatrix);
//compute the context similarity - linear interpolate
bool q_compare_context_linear(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,vector< vector<double> > &vecvec_simmatrix);
//compute the context similarity - histogram
bool q_compare_context_histogram(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,const int n_bin,vector< vector<double> > &vecvec_simmatrix);


#endif

