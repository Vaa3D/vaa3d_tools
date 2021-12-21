// q_pointcloud_match.h
// initial matching + refine matching
// by Lei Qu
// 2009-06-24

#ifndef __Q_POINTCLOUD_MATCH_H__
#define __Q_POINTCLOUD_MATCH_H__

#include "q_pointcloud_match_basic.h"
#include "q_pointcloud_match_initialmatch.h"


//initial matching + refine mathcing
//input:
//	vec_1,vec_2: 				two input point clouds
//	n_dismethod:				0:Euclidian based dis; 1:Geodesic based dis
//	arr_voterweight[100]:		arr_voterweight[0]:NN voter weight; arr_voterweight[1]:Linear voter weight; arr_voterweight[2]:Histogram voter weight; (==0 do not vote)
//	n_bin:						historgram bin num
//	K:							neighbor num, used for Geodesic dis computation
//	n_refinemethod:				n_refinemethod=0:do not run refine match; n_refinemethod=1:run affine based refine match; n_refinemethod=2:run manifold based refine match;
//	n_sampling:					RANSAC sampling num
//	n_affineweight:				affine voter weight
//	arr_constrainter[100]:		arr_constrainter[0]=1: use dir constainter; arr_constrainter[1]=1: use dis constainter; arr_constrainter[2]=1: use top voted candidate constainter;
//	n_iternum:					manifold local consensus optimize num
//	n_neighbor:					manifold local size
//	n_topcandidatenum:			manifold valid top candidate constrainter num
//ouput:
//	vec_1to2index:				output matching index
//	vec_2_invp:					affine inverse projection of arr_2
//
bool q_pointcloud_match(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,
		const int n_dismethod,																					//initial basic
		const int arr_voterweight[100],const int n_bin,const int K,												//initial adv
		const int n_refinemethod,																				//refine basic
		const int n_sampling,const int n_affineweight,															//refine adv - affine
		const bool arr_constrainter[100],const int n_iternum,const int n_neighbor,const int n_topcandidatenum,	//refine adv - manifold
		vector<int> &vec_1to2index,																				//output
		vector<Coord3D_PCM> &vec_2_invp);																		//output - refine adv - affine


#endif

