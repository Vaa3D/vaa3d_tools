// q_pointcloud_match.cpp
// by Lei Qu
// 2009-06-26

#include "q_pointcloud_match_refinematch_affine.h"
#include <math.h>
#include <time.h>

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
		vector<int> &vec_A2Bindex_refined)
{
	bool refine_tag = 0;
	//check parameters
	if(vec_A2Bindex.size()<=0 ||
	   vec_A_norm.size()<=0 || vec_B_norm.size()<=0 ||
	   n_sampling<=0 || n_pairs_persampling<4 || n_affineweight<1 ||
	   vecvec_votematrix.size()!=vec_A_norm.size() || vecvec_votematrix[0].size()!=vec_B_norm.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}
	vec_A2B_cost.clear();
	vec_A2Bindex_refined=vec_A2Bindex;

	//------------------------------------------------------------------------------------------------------------------------------------
	//(1).kill all the multi-matched pairs before running RANSAC (although one of them may be right!)
	//we do not want any possible the wrong matched pairs affect the consequent RANSAC affine estimation
	printf("\t1. kill all multi-matched pairs before running RANSAC! \n");
	if(!q_kill_allmultimatch(vec_A2Bindex_refined,vecvec_votematrix[0].size()))
	{
		fprintf(stderr,"ERROR: q_killall_multimatch() return false! \n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(2). find the best affine matrix based on initial matching result using RANSAC
	printf("\t2. find the best affine matrix based on initial matching result using RANSAC \n");
	if(!q_affine_estimate_affinmatrix_RANSAC(vec_A_norm,vec_B_norm,vec_A2Bindex,
											 n_sampling,n_pairs_persampling,
											 x4x4_affinematrix))
	{
		fprintf(stderr,"ERROR: q_compute_affinmatrix_from_initialmatch() return false! \n");
		return false;
	}

	//2014-01-19 skip the refining step to improve speed
	//------------------------------------------------------------------------------------------------------------------------------------
	//(8). refine the affine matrix and the matching by introducing a new voter - dis_A2invB
	if (refine_tag)
	{
		printf("\t3. refine the affine matrix and the matching by introducing a new voter - dis_A2invB \n");
		if(!q_affine_refineaffinematching_disA2Binvp_voting(vec_A_norm,vec_B_norm,
														n_affineweight,
														vecvec_votematrix,
												        vec_A2Bindex_refined,vec_A2B_cost,
												        x4x4_affinematrix))
		{
			fprintf(stderr,"ERROR: q_refine_affinematching_disA2invB_voting() return false! \n");
			return false;
		}

	}
	else
		vec_A2Bindex_refined = vec_A2Bindex;

	return true;
}

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
bool q_affine_estimate_affinmatrix_RANSAC(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,
		const vector<int> &vec_A2Bindex,
		const int n_sampling,const int n_pairs_persampling,
		Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<=0 || vec_B.size()<=0 || vec_A2Bindex.size()<=0 ||
	   n_sampling<=0 || n_pairs_persampling<4)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}
	//check the validation of vec_A2Bindex, make sure it has more than n_pairs_persampling valid pairs
	vector<int> vec_A2Bindex_validindex;
	for(int i=0;i<vec_A.size();i++)
		if(vec_A2Bindex[i]!=-1)
			vec_A2Bindex_validindex.push_back(i);
	if(vec_A2Bindex_validindex.size()<n_pairs_persampling)
	{
		fprintf(stderr,"ERROR: No enough valid initial matched pairs! \n");
		return false;
	}

	//find the best affine matrix with minimal inverse projection error use RANSAC
	double err_invp_min=1e+38;
	Matrix x4x4_affinematrix_temp(4,4);
	srand((unsigned)time(NULL));
	for(int iter=0;iter<n_sampling;iter++)
	{
		if(iter%50==0) printf("\n\t");
		if(iter%5==0) printf("%4d,",iter);
		// 1. chose n_pairs_per_sampling pairs from initial matching result randomly
		vector<int> vec_random_index(n_pairs_persampling,0);
		vector<Coord3D_PCM> vec_sample_A,vec_sample_B;
		//get sample index
		bool b_valid_sample=true;
		do {
			//generate n_pairs_per_sampling random numbers
			for(int i=0;i<n_pairs_persampling;i++)
			{
				vec_random_index[i]=rand()%vec_A.size();
				for(int k=0;k<i;k++)
					if(vec_random_index[k]==vec_random_index[i])
					{
						vec_random_index[i]=rand()%vec_A.size();
						k=-1;//recheck
					}
			}
			//make sure no same number
			for(int i=0;i<n_pairs_persampling;i++)
			{
				if(vec_A2Bindex[vec_random_index[i]]==-1)
				{
					b_valid_sample=false;
					break;
				}
				b_valid_sample=true;
			}
		} while (!b_valid_sample);
		//get sample array
		for(int i=0;i<vec_random_index.size();i++)
		{
			vec_sample_A.push_back(vec_A[vec_random_index[i]]);
			vec_sample_B.push_back(vec_B[vec_A2Bindex[vec_random_index[i]]]);
		}

		// 2. estimate the affine matrix: A=T*B
		if(!q_affine_compute_affinmatrix_3D(vec_sample_B,vec_sample_A,x4x4_affinematrix_temp))
		{
			fprintf(stderr,"ERROR: q_compute_affinmatrix3D() return false! \n");
			return false;
		}
		else if(x4x4_affinematrix_temp.is_zero())	//degenerate case
		{
			continue;
		}

		// 3. compute the inverse projection error
		double err_invp;
		vector< vector<double> > vecvec_dismatrix_A2Binvp;
		if(!q_affine_compute_invp_err(vec_A,vec_B,x4x4_affinematrix_temp,vecvec_dismatrix_A2Binvp,err_invp))
		{
			fprintf(stderr,"ERROR: q_affine_compute_invp_err() return false! \n");
			return false;
		}

		// 4. find the affine matrix with minimal inv-projection error
		if(err_invp<err_invp_min)
		{
			err_invp_min=err_invp;
			x4x4_affinematrix=x4x4_affinematrix_temp;
			printf("\n\t(%4d). err_invp_min=%f\n\t",iter,err_invp_min);
		}
	}
	printf("\n");

	return true;
}

//compute the affine matraix
//	B=T*A
bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	//normalize point set
	vector<Coord3D_PCM> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points(vec_B,vec_B_norm,x4x4_normalize_B);

	//fill matrix A
	//
	//	  | h1, h2, h3, h4 |    |x1| |x2|
	//	  | h5, h6, h7, h8 | *  |y1|=|y2| <=>
	//	  | h9, h10,h11,h12|    |z1| |z2|
	//	  | 0 ,  0,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, z1, 1,  0,  0,  0,  0,  0,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, 0, x1, y1, z1,  1,  0,  0,  0,  0, -y2 | * |h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,h13|=0
	//	  | 0,  0,  0, 0, 0, 0, 0, 0,  0, x1, y1, z1,  1, -z2 |
	int n_point=vec_A.size();
	Matrix A(3*n_point,13);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=vec_A_norm[i].z;	A(row,4)=1.0;
		A(row,5)=0.0;				A(row,6)=0.0;				A(row,7)=0.0;				A(row,8)=0.0;
		A(row,9)=0.0;				A(row,10)=0.0;				A(row,11)=0.0;				A(row,12)=0.0;
		A(row,13)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;				A(row+1,4)=0.0;
		A(row+1,5)=vec_A_norm[i].x;	A(row+1,6)=vec_A_norm[i].y;	A(row+1,7)=vec_A_norm[i].z;	A(row+1,8)=1.0;
		A(row+1,9)=0.0;				A(row+1,10)=0.0;			A(row+1,11)=0.0;			A(row+1,12)=0.0;
		A(row+1,13)=-vec_B_norm[i].y;

		A(row+2,1)=0.0;				A(row+2,2)=0.0;				A(row+2,3)=0.0;				A(row+2,4)=0.0;
		A(row+2,5)=0.0;				A(row+2,6)=0.0;				A(row+2,7)=0.0;				A(row+2,8)=0.0;
		A(row+2,9)=vec_A_norm[i].x;	A(row+2,10)=vec_A_norm[i].y;A(row+2,11)=vec_A_norm[i].z;A(row+2,12)=1.0;
		A(row+2,13)=-vec_B_norm[i].z;

		row+=3;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D;
	Matrix U,V;
	SVD(A,D,U,V);	//A = U * D * V.t()

	Matrix h=V.column(13);	//A*h=0
	if(D(12,12)==0)			//degenerate case
	{
		x4x4_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=13;i++)
	{
		h(i,1) /= h(13,1);
	}

	//reshape h:13*1 to 4*4 matrix
	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;

	//denormalize
	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}


//compute affine inverse projection error - invp
//invp_err = sum(distant of all A to its nearest B_invp)
bool q_affine_compute_invp_err(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,const Matrix &x4x4_affinematrix,
		vector< vector<double> > &vecvec_dismatrix_A2Binvp,double &d_invp_err)
{
	//check parameters
	if(vec_A.size()<=0 || vec_B.size()<=0)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		fprintf(stderr,"ERROR: Input affine matrix is invalid! \n");
		return false;
	}
	vecvec_dismatrix_A2Binvp.clear();
	vecvec_dismatrix_A2Binvp.assign(vec_A.size(),vector<double>(vec_B.size(),0.0));

	//inverse project B --> B_invp
	Matrix x_B_invp(4,vec_B.size()), x_B_homo(4,vec_B.size());
	for(int i=0;i<vec_B.size();i++)
	{
		x_B_homo(1,i+1)=vec_B[i].x;
		x_B_homo(2,i+1)=vec_B[i].y;
		x_B_homo(3,i+1)=vec_B[i].z;
		x_B_homo(4,i+1)=1.0;
	}
	x_B_invp=x4x4_affinematrix*x_B_homo;

	//form vec_dismatrix and compute the inverse projection error
	//error = sum(distant of all A to its nearest B_invp)
	d_invp_err=0;
	double dis,mindis;
	double dx,dy,dz;
	for(int i=0;i<vec_A.size();i++)
	{
		mindis=1e+38;
		for(int j=0;j<vec_B.size();j++)
		{
			dx=vec_A[i].x-x_B_invp(1,j+1);
			dy=vec_A[i].y-x_B_invp(2,j+1);
			dz=vec_A[i].z-x_B_invp(3,j+1);
			dis=sqrt(dx*dx+dy*dy+dz*dz);
			vecvec_dismatrix_A2Binvp[i][j]=dis;

			if(dis<mindis)
				mindis=dis;
		}
		d_invp_err+=mindis;
	}

	return true;
}

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
bool q_affine_refineaffinematching_disA2Binvp_voting(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,
		const int n_affineweight,
		vector< vector<double> > &vecvec_votematrix,
		vector<int> &vec_A2Bindex, vector<double> &vec_A2B_cost,
		Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<5 || vec_B.size()<5 	||
	   vecvec_votematrix.size()!=vec_A.size() 	|| 	vecvec_votematrix[0].size()!=vec_B.size() ||
	   vec_A2Bindex.size()!=vec_A.size() 	||
	   x4x4_affinematrix.nrows()!=4 		|| 	x4x4_affinematrix.ncols()!=4)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(n_affineweight<0)
	{
		fprintf(stderr,"ERROR: n_affineweight should >= 0! \n");
		return false;
	}
	if(vec_A2B_cost.size()!=0)
	{
		printf("WARNING: Input arr_B2Acost is not empty, original data will be cleared!\n");
	}
	vec_A2B_cost.clear();
	vec_A2B_cost.assign(vec_A.size(),-1);
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		fprintf(stderr,"ERROR: Input affine matrix is invalid! \n");
		return false;
	}

	int 	n_point_A		=vec_A.size();
	int 	n_point_B		=vec_B.size();
	double 	d_invp_err_min	=1e+38;
	double 	d_invp_err;

	vector< vector<double> > vec_dismatrix_A2Binvp;
	vector< vector<int> > 	 vecvec_votematrix_sortindex;
	vector<double> 			 vec_votematrix_1col(n_point_A,0.0);
	vector<int> 			 vec_votematrix_1col_sortindex;
	Matrix x4x4_affinematrix_best = x4x4_affinematrix;
	Matrix x_B_invp(4,vec_B.size()) , x_B_homo(4,vec_B.size());
	for(int i=0;i<vec_B.size();i++)
	{
		x_B_homo(1,i+1)=vec_B[i].x;
		x_B_homo(2,i+1)=vec_B[i].y;
		x_B_homo(3,i+1)=vec_B[i].z;
		x_B_homo(4,i+1)=1.0;
	}

	//do voting use the distant contex information
//	q_vote_simmatrix(arr_simmatrix_nn,1,arr_votematrix);
//	q_vote_simmatrix(arr_simmatrix_linear,1,arr_votematrix);
//	q_vote_simmatrix(arr_simmatrix_histrogram,1,arr_votematrix);
	vector< vector<double> > arr_votematrix_onlycontext(vecvec_votematrix);

	for(int iter=0;iter<n_point_A;iter++)
	{
		//------------------------------------------------------------------------------------------------------------
		// 1.compute the distant of A to invB(from the best affine matrix) --> dis_matrix, use it as a new voter latter
		q_affine_compute_invp_err(vec_A,vec_B,x4x4_affinematrix_best,vec_dismatrix_A2Binvp,d_invp_err);

		//------------------------------------------------------------------------------------------------------------
		// 2. sort similarity matrix and voting the best several canditates with given weight
		//vote with dismatrix based the vote result from contex information
		vecvec_votematrix=arr_votematrix_onlycontext;
		q_vote_simmatrix(vec_dismatrix_A2Binvp,n_affineweight,vecvec_votematrix);

		//------------------------------------------------------------------------------------------------------------
		// 3.choose top voted 5+i pairs to refine affine matrix (i represent current iteration times)  ?(how to deal with cross matching)?
		//sort every row of votematrix to find the best several candidates for every A
		q_sort2d(vecvec_votematrix, vecvec_votematrix_sortindex, 1, 0);
		//sort the first col of sorted votematrix to fined the top vote pairs
		for(int i=0;i<n_point_A;i++)	vec_votematrix_1col[i]=vecvec_votematrix[i][0];
		q_sort1d(vec_votematrix_1col, vec_votematrix_1col_sortindex);	//note: from low to high

		//choose the top voted 5+i pairs
		int n_pair=0,n_pair_last=0;;
		vec_A2Bindex.assign(n_point_A,-1);
		for(int i=0;i<n_point_A;i++)
		{
			//if top candidate got the same vote as the second one, we leave it alone
			if(vecvec_votematrix[vec_votematrix_1col_sortindex[n_point_A-i-1]][0]==vecvec_votematrix[vec_votematrix_1col_sortindex[n_point_A-i-1]][1])
				continue;
			//record top candidate's index: arr1d_A2Bindex(i)=j  <==>  A(i)->B(j)
			int A_ind=vec_votematrix_1col_sortindex[n_point_A-i-1];
			int B_ind=vecvec_votematrix_sortindex[A_ind][0];
			vec_A2Bindex[A_ind]=B_ind;
			//take the vote tickets as the matching cost (this matching cost will be used to pick up the best pair in the multi-matched pairs)
			vec_A2B_cost[A_ind]=vecvec_votematrix[A_ind][B_ind];
			//only choose 5+i pairs in the ith iteration, we use them to refine the affine matrix latter
			if(++n_pair>=5+iter)
				break;
		}

		//------------------------------------------------------------------------------------------------------------
		// 4.use the choosed 5+i pairs to re-estimate the affine matrix
		Matrix x4x4_affinematrix_temp;
		vector<Coord3D_PCM> vec_matched_A,vec_matched_B;
		int n=0;
		for(int i=0;i<n_point_A;i++)
		{
			if(vec_A2Bindex[i]!=-1)
			{
				vec_matched_A.push_back(vec_A[i]);
				vec_matched_B.push_back(vec_B[vec_A2Bindex[i]]);
				n++;
			}
		}

		q_affine_compute_affinmatrix_3D(vec_matched_B,vec_matched_A,x4x4_affinematrix_temp);

		//------------------------------------------------------------------------------------------------------------
		// 5. compute the invp_err, and then select the best affine matrix
		//******************if error increase , test the second candidate, detail how?************************
		//compute the inverse projection error
		double d_invp_err=0;
		q_affine_compute_invp_err(vec_A,vec_B,x4x4_affinematrix_temp,vec_dismatrix_A2Binvp,d_invp_err);
		//pick the best affine matrix and use it to begain the next refine iteration
		if(d_invp_err<d_invp_err_min)
		{
			d_invp_err_min=d_invp_err;
			x4x4_affinematrix_best=x4x4_affinematrix_temp;
		}
		printf("\t(%3d). %3d pairs obtained:\tcur_invp_err=%f,\tmin_invp_err=%f\n",iter,n_pair,d_invp_err,d_invp_err_min);

		//------------------------------------------------------------------------------------------------------------
		// 6. stop iteration
		if(n_pair==n_pair_last || n_pair>=n_point_A)
			break;
		else
			n_pair_last=n_pair;
	}

	return true;
}


