// q_pointcloud_match_refinematch_manifold.h
// manifold based refine matching - local consensus optimization
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.h by Lei Qu on 2010-01-22

#ifndef __Q_POINTCLOUD_MATCH_REFINEMATCH_MANIFOLD_H__
#define __Q_POINTCLOUD_MATCH_REFINEMATCH_MANIFOLD_H__

#include "q_pointcloud_match_basic.h"


//manifold based refine matching
//base on the initial mathcing result, this fuction try to minimize the local dir and dis difference
//	(0). the two input points sets A and B should be normalized and also satisfy size(A) <= size(B)
//	(1). shift normalized B one uint away (to decrease the effect of big direction variation caused by close distance)
//	(2). minimize the total direction difference in a manifold way (optimize the local avg dir iteratively)
//	(3). further minimize the total dir, dis different in a manifold way, also limit the candidates to several top voted ones
//
bool q_pointcloud_match_refinematch_manifold(const vector<int> &vec_A2Bindex,
		const vector<Coord3D_PCM> &vec_A_norm,const vector<Coord3D_PCM> &vec_B_norm,
		const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,
		const vector< vector<double> > &vecvec_votematrix,
		const bool arr_constrainter[100],const int n_iternum,const int n_neighbor,const int n_topcandidatenum,
		vector<double> &vec_A2B_cost,
		vector<int> &vec_A2Bindex_refined);


#endif

