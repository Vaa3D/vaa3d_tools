// q_pointcloud_match.cpp
// by Lei Qu
// 2009-06-26

#include "q_pointcloud_match.h"
#include "q_pointcloud_match_refinematch_manifold.h"
#include "q_pointcloud_match_refinematch_affine.h"

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
		vector<Coord3D_PCM> &vec_2_invp)																		//output - refine adv - affine
{
	//------------------------------------------------------------------------------------------------------------------------------------
	//(1). make length(A) always < length(B), this makes later process more easyer
	printf("(1). make length(A) always < length(B) \n");
	bool bAeq1=true;	//true means that vec_marker_pos_1.size < vec_marker_pos_2.size, and we do not swap them; otherwise vec_marker_pos_A=vec_marker_pos_2
	vector<Coord3D_PCM> vec_A,vec_B;
	if(vec_1.size()>vec_2.size())
	{
		vec_A=vec_2;
		vec_B=vec_1;
		bAeq1=false;
	}
	else
	{
		vec_A=vec_1;
		vec_B=vec_2;
		bAeq1=true;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(2). initial matching
	printf("(2). initial matching \n");
	vector<Coord3D_PCM> vec_A_norm,vec_B_norm;
	Matrix x4x4_A_norm(4,4),x4x4_B_norm(4,4);
	vector< vector<double> > vecvec_dismatrix_A,vecvec_dismatrix_B;
	vector< vector<double> > vecvec_votematrix;
	vector<int> vec_A2Bindex;
	if(!q_pointcloud_match_initialmatch(vec_A,vec_B,
			n_dismethod,K,arr_voterweight,n_bin,
			vec_A_norm,vec_B_norm,
			x4x4_A_norm,x4x4_B_norm,
			vecvec_dismatrix_A,vecvec_dismatrix_B,
			vecvec_votematrix,
			vec_A2Bindex))
	{
		fprintf(stderr,"ERROR: q_pointcloud_match_initialmatch() return false! \n");
		return false;
	}
	
//	q_export_matches2swc(vec_A,vec_B,vec_A2Bindex,"/Users/qul/work/v3d_2.0/plugin_demo/pointcloud_match/1_initial.swc");

	//------------------------------------------------------------------------------------------------------------------------------------
	//refine matching
	vector<int> vec_A2Bindex_refined(vec_A2Bindex);
	vector<double> vec_A2B_cost;
	Matrix x4x4_affinematrix(4,4);
	if(n_refinemethod)//run refine matching
	{
		//run affine based refine matching
		if(n_refinemethod==1)
		{
			//(3). refine matching - affine based
			printf("(3). refine matching - affine based \n");
			int n_pairs_persampling=5;
			if(!q_pointcloud_match_refinematch_affine(vec_A2Bindex,
					vec_A_norm,vec_B_norm,
					vecvec_votematrix,
					n_sampling,n_pairs_persampling,n_affineweight,
					vec_A2B_cost,
					x4x4_affinematrix,
					vec_A2Bindex_refined))
			{
				fprintf(stderr,"ERROR: q_pointcloud_match_refinematch_affine() return false! \n");
				return false;
			}

//			//(4). kill multi-match by only keep one pair that has highest match cost
//			printf("(4). kill multi-match by only keep one pair that has highest match cost \n");
//			if(!q_kill_badmultimatch_keephigestcost(vec_A2Bindex_refined,vec_A2B_cost,vec_B_norm.size()))
//			{
//				fprintf(stderr,"ERROR: q_killbad_multimatch() return false! \n");
//				return false;
//			}
		}
		//run manifold based refine matching
		else if(n_refinemethod==2)
		{
			//(3). refine matching - minifold based
			printf("(3). refine matching - manifold based \n");
			if(!q_pointcloud_match_refinematch_manifold(vec_A2Bindex,
					vec_A_norm,vec_B_norm,
					vecvec_dismatrix_A,vecvec_dismatrix_B,
					vecvec_votematrix,
					arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,
					vec_A2B_cost,
					vec_A2Bindex_refined))
			{
				fprintf(stderr,"ERROR: q_pointcloud_match_refinematch_manifold() return false! \n");
				return false;
			}

			//(4). kill multi-match by only keep one pair that has lowest match cost
			printf("(4). kill multi-match by only keep one pair that has lowest match cost \n");
			if(!q_killbadmultimatch_keeplowestcost(vec_A2Bindex_refined,vec_A2B_cost,vec_B.size()))
			{
				fprintf(stderr,"ERROR: q_kill_badmultimatch_manifold() return false! \n");
				return false;
			}
		}

//		q_export_matches2swc(vec_A,vec_B,vec_A2Bindex_refined,"/Users/qul/work/v3d_2.0/plugin_demo/pointcloud_match/1_refine.swc");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(5). do some transformation according to step 1
	printf("(5). do some transfroms according to the result of step (1)! \n");
	Matrix x4x4_affinematrix_2to1(4,4);
	if(bAeq1==1)
	{
		printf("\tvec_1=vec_A,vec_2=vec_B --> do nothing\n");
		vec_1to2index=vec_A2Bindex_refined;

		if(n_refinemethod==1)
		{
			x4x4_affinematrix_2to1=x4x4_A_norm.i()*x4x4_affinematrix*x4x4_B_norm;	//de-normlize
		}
	}
	else	//length(vec_1)>length(vec_2)
	{
		printf("\tvec_1=vec_B,vec_2=vec_A --> do swap!\n");
		vec_1to2index.assign(vec_1.size(),-1);
		for(int i=0;i<vec_A.size();i++)
			if(vec_A2Bindex_refined[i]!=-1)
				vec_1to2index[vec_A2Bindex_refined[i]]=i;

		if(n_refinemethod==1)
		{
			x4x4_affinematrix_2to1=x4x4_A_norm.i()*x4x4_affinematrix*x4x4_B_norm;	//de-normlize
			x4x4_affinematrix_2to1=x4x4_affinematrix_2to1.i();						//A=T*B <=> 2=T*1 <=> 1=T.i()*2
		}
	}

	if(n_refinemethod==1)
	{
		//inverse project vec_2 use affine matrix
		//	vec_2_invp = x4x4_affinematrix_2to1 * x_vec2_homo
		Matrix x_vec2_invp(4,vec_2.size()), x_vec2_homo(4,vec_2.size());
		vec_2_invp.assign(vec_2.size(),Coord3D_PCM());
		for(int i=0;i<vec_2.size();i++)
		{
			x_vec2_homo(1,i+1)=vec_2[i].x;
			x_vec2_homo(2,i+1)=vec_2[i].y;
			x_vec2_homo(3,i+1)=vec_2[i].z;
			x_vec2_homo(4,i+1)=1.0;
		}
		x_vec2_invp=x4x4_affinematrix_2to1*x_vec2_homo;
		for(int i=0;i<vec_2.size();i++)
		{
			vec_2_invp[i].x=x_vec2_invp(1,i+1);
			vec_2_invp[i].y=x_vec2_invp(2,i+1);
			vec_2_invp[i].z=x_vec2_invp(3,i+1);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(5). print matching result
	printf("(5). print matching result \n");
	for(int i=0;i<vec_1to2index.size();i++)
	{
		printf("\t(%4d).   vec1[%d] --> vec2[%d]\n",i,i,vec_1to2index[i]);
	}

	return true;
}

