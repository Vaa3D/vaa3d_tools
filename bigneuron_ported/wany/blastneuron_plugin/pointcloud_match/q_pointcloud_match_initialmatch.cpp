// q_pointcloud_match_initialmatch.cpp
// shape context based initial matching
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.cpp by Lei Qu on 2010-01-22

#include <math.h>
#include "q_pointcloud_match_initialmatch.h"


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
									 vector<int> &vec_A2Bindex)
{
	//check parameters
	int n_A=vec_A.size();
	int n_B=vec_B.size();
	if(n_A<5 || n_B<5)
	{
		fprintf(stderr,"ERROR: Input array/arrays should have more than 4 points! \n");
		return false;
	}
	if(n_method!=0 && n_method!=1)
	{
		fprintf(stderr,"ERROR: n_method shoud be 0:(Euclidian based) or 1:(Geodesic based)! \n");
		return false;
	}
	if(n_method==1 && (K<=1 || K>n_A || K>n_B))
	{
		fprintf(stderr,"ERROR: K is invalid! \n");
		return false;
	}
	int n_voter=0;
	for(int i=0;i<100;i++)
		if(arr_voterweight[i])
			n_voter++;
	if(n_voter==0)
	{
		fprintf(stderr,"ERROR: At least 1 voter needed! \n");
		return false;
	}
	if(arr_voterweight[2] && n_bin<=1)
	{
		fprintf(stderr,"ERROR: n_bin should > 1! \n");
		return false;
	}
	
	vec_A_norm.clear();
	vec_B_norm.clear();
	vecvec_dismatrix_A.clear();
	vecvec_dismatrix_B.clear();
	vecvec_votematrix.clear();
	vec_A2Bindex.clear();
	vec_A2Bindex.assign(vec_A.size(),-1);
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//(1). normalize point clouds
	printf("\t1. normalize two input point clouds \n");
	if(!q_normalize_points(vec_A,vec_A_norm,x4x4_A_norm))
	{
		fprintf(stderr,"ERROR: q_normalize_points() return false! \n");
		return false;
	}
	if(!q_normalize_points(vec_B,vec_B_norm,x4x4_B_norm))
	{
		fprintf(stderr,"ERROR: q_normalize_points() return false! \n");
		return false;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//(2). compute the shape context - dismatrix
	//compute the distance of every point in A to all others points in A
	if(n_method==0)//Euclidian
	{
		printf("\t2. compute the Euclidian distance, and dismatrix \n");
		q_compute_dismatrix_euclidian(vec_A_norm,vecvec_dismatrix_A);
		q_compute_dismatrix_euclidian(vec_B_norm,vecvec_dismatrix_B);
	}
	else if(n_method==1)//Geodesic
	{
		printf("\t2. compute the Geodesic distance, and dismatrix \n");
		q_compute_dismatrix_geodesic(vec_A_norm,K,vecvec_dismatrix_A);
		q_compute_dismatrix_geodesic(vec_B_norm,K,vecvec_dismatrix_B);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//(3). rescaling the dismatrix of B to A according to the average pairwise Geodesic distance (SIGNIFICANT IMPROVE!)
	//	the previous normalization was done according to the euclidian distance
	//	this will affect NN and linear similarity measurement, not affect histogram
	if(n_method==0)//Euclidian
	{
		printf("\t3. skipped - this step only needed by euclidian based method \n");
	}
	else if(n_method==1)//Geodesic
	{
		printf("\t3. rescaling the point clouds B to A according to the average pairwise geodesic distance \n");
		double d_rescalefactor_B;
		//compute the averge geodesic distance of A
		double d_dis_avg_A=0;
		for(int i=0;i<n_A;i++)
			for(int j=0;j<n_A;j++)
				d_dis_avg_A+=vecvec_dismatrix_A[i][j];
		d_dis_avg_A/=n_A*n_A-n_A;
		//compute the averge geodesic distance of B
		double d_dis_avg_B=0;
		for(int i=0;i<n_B;i++)
			for(int j=0;j<n_B;j++)
				d_dis_avg_B+=vecvec_dismatrix_B[i][j];
		d_dis_avg_B/=n_B*n_B-n_B;
		//compute the rescaling factor and rescaling B
		d_rescalefactor_B=d_dis_avg_A/d_dis_avg_B;
		printf("\t\t>>d_rescalefactor_B=%f\n",d_rescalefactor_B);
		for(int i=0;i<n_B;i++)
			for(int j=0;j<n_B;j++)
				vecvec_dismatrix_B[i][j]*=d_rescalefactor_B;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//(4). compute the context similarity use different methods (we vote base on these similarity results)
	printf("\t4. compute the context similarity use different methods \n");
	//sort every row of the dismatrix from low to high before perform comparing
	printf("\t\t>>sort dismatrix\n");
	vector< vector<double> > vecvec_dismatrix_A_sort,vecvec_dismatrix_B_sort;
	vector< vector<int> > vecvec_dismatrix_A_sortindex,vecvec_dismatrix_B_sortindex;
	vecvec_dismatrix_A_sort=vecvec_dismatrix_A;
	vecvec_dismatrix_B_sort=vecvec_dismatrix_B;
	if(!q_sort2d(vecvec_dismatrix_A_sort,vecvec_dismatrix_A_sortindex,1,1))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}
	if(!q_sort2d(vecvec_dismatrix_B_sort,vecvec_dismatrix_B_sortindex,1,1))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}
	//		a. method 1: compute correlation directly from the NN interpolated distant context
	vector< vector<double> > vecvec_simmatrix_nn;
	if(arr_voterweight[0])
	{
		printf("\t\t>>compute the similarity based on NN interpolated distant context curve\n");
		if(!q_compare_context_nn(vecvec_dismatrix_A_sort,vecvec_dismatrix_B_sort,vecvec_simmatrix_nn))
		{
			fprintf(stderr,"ERROR: q_comare_context_nn() return false! \n");
			return false;
		}
	}
	//		b. method 2: compute correlation directly from the linear interpolated distant context
	vector< vector<double> > vecvec_simmatrix_linear;
	if(arr_voterweight[1])
	{
		printf("\t\t>>compute the similarity based on Linear interpolated distant context curve\n");
		if(!q_compare_context_linear(vecvec_dismatrix_A_sort,vecvec_dismatrix_B_sort,vecvec_simmatrix_linear))
		{
			fprintf(stderr,"ERROR: q_comare_context_linear() return false! \n");
			return false;
		}
	}
	//		c. method 3: compute correlation from the histograms of distant context
	vector< vector<double> > vecvec_simmatrix_histrogram;
	if(arr_voterweight[2])
	{
		printf("\t\t>>compute the similarity based on Histogram curve of distant context\n");
		if(!q_compare_context_histogram(vecvec_dismatrix_A_sort,vecvec_dismatrix_B_sort,n_bin,vecvec_simmatrix_histrogram))
		{
			fprintf(stderr,"ERROR: q_comare_context_histogram() return false! \n");
			return false;
		}
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//(5). vote and do initial match
	printf("\t5. vote and do initial match \n");
	//do vote based on the arr_simmatrix_nn
	if(arr_voterweight[0])
	{
		printf("\t\t>>vote based on the arr_simmatrix_nn\n");
		if(!q_vote_simmatrix(vecvec_simmatrix_nn,1,vecvec_votematrix))
		{
			fprintf(stderr,"ERROR: q_vote_simmatrix() return false! \n");
			return false;
		}
	}
	//do vote based on the arr_simmatrix_linear
	if(arr_voterweight[1])
	{
		printf("\t\t>>vote based on the arr_simmatrix_linear\n");
		if(!q_vote_simmatrix(vecvec_simmatrix_linear,1,vecvec_votematrix))
		{
			fprintf(stderr,"ERROR: q_vote_simmatrix() return false! \n");
			return false;
		}
	}
	//do vote based on the arr_simmatrix_histogram
	if(arr_voterweight[2])
	{
		printf("\t\t>>vote based on the arr_simmatrix_histogram\n");
		if(!q_vote_simmatrix(vecvec_simmatrix_histrogram,1,vecvec_votematrix))
		{
			fprintf(stderr,"ERROR: q_vote_simmatrix() return false! \n");
			return false;
		}
	}
	//sort the votematrix and pick the top voted candidates as the initial matched pairs
	//sort the every row of the votematrix from high to low
	vector< vector<double> > vecvec_votematrix_sort(vecvec_votematrix);
	vector< vector<int> > vecvec_votematrix_sort_index;
	if(!q_sort2d(vecvec_votematrix_sort, vecvec_votematrix_sort_index,1,0))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}
	
	//pick the top voted candidates as the initial matched pairs
	//if the top candidate got the same vote as the second one, we cannot dicide which one is the best, so just leave it unmatched
	for(unsigned int i=0;i<vecvec_votematrix_sort.size();i++)
		if(vecvec_votematrix_sort[i][0]>vecvec_votematrix_sort[i][1])
			vec_A2Bindex[i]=vecvec_votematrix_sort_index[i][0];
	
	printf("\tInitial match exit sucess!\n");
	
	return true;
}


//compute and sort the distant context information
bool q_compute_dismatrix(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix)
{
	//check parameters
	int n_point=vec_input.size();
	if(n_point<=0)
	{
        fprintf(stderr,"ERROR: q_compute_dismatrix: Input array is null! \n");
        return false;
	}
	vecvec_dismatrix.clear();
	vecvec_dismatrix.assign(n_point,vector<double>(n_point,0));
	
	//compute the distance of every point in the point set to all others points in the same set
	double dx,dy,dz;
	for(int i=0;i<n_point;i++)
		for(int j=0;j<n_point;j++)
		{
			dx=vec_input[i].x-vec_input[j].x;
			dy=vec_input[i].y-vec_input[j].y;
			dz=vec_input[i].z-vec_input[j].z;
			vecvec_dismatrix[i][j]=dx*dx+dy*dy+dz*dz;
		}
	
	//sort every row of the dismatrix from low to high
	vector< vector<int> > vecvec_dismatrix_index;
	if(!q_sort2d(vecvec_dismatrix, vecvec_dismatrix_index,1,1))
	{
		fprintf(stderr,"ERROR: q_compute_dismatrix: Call q_sort2d return false! \n");
		return false;
	}
	
	return true;
}

//compute the Euclidian distance of every possible pair of input point set -> dismatrix
bool q_compute_dismatrix_euclidian(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix)
{
	//check parameters
	if(vec_input.size()<=0)
	{
        fprintf(stderr,"ERROR: q_compute_dismatrix_euclidian: Input array is null! \n");
        return false;
	}
	int n_point=vec_input.size();
	if(!vecvec_dismatrix.empty())
	{
		fprintf(stderr, "WARNING: q_compute_dismatrix_euclidian: arr_dismatrix is not empty, original data will be deleted! \n");
		vecvec_dismatrix.clear();
	}
	vecvec_dismatrix.assign(n_point,vector<double>(n_point,-1));
	
	
	//compute the distance of every point in the point set to all others points in the same set
	double dx,dy,dz;
	for(int i=0;i<n_point;i++)
		for(int j=0;j<n_point;j++)
		{
			dx=vec_input[i].x-vec_input[j].x;
			dy=vec_input[i].y-vec_input[j].y;
			dz=vec_input[i].z-vec_input[j].z;
			vecvec_dismatrix[i][j]=sqrt(dx*dx+dy*dy+dz*dz);
		}
	
	return true;
}

//compute the Geodesic distance of every possible pair of input point set -> dismatrix
bool q_compute_dismatrix_geodesic(const vector<Coord3D_PCM> vec_input,const int K,vector< vector<double> > &vec_dismatrix)
{
	//check parameters
	if(vec_input.empty())
	{
        fprintf(stderr,"ERROR: q_compute_dismatrix_geodesic: Input array is empty! \n");
        return false;
	}
	int N=vec_input.size();
	if(K<2 || K>N)
	{
        fprintf(stderr,"ERROR: q_compute_dismatrix_geodesic: Input K is invalid! \n");
        return false;
	}
	if(!vec_dismatrix.empty())
	{
		fprintf(stderr, "WARNING: q_compute_dismatrix_geodesic: Input arr_dismatrix is not empty, original data will be deleted! \n");
		vec_dismatrix.clear();
	}
	
	double INF=1.e100;
	vector< vector<double> > vecvec_dismatrix_euclidian,vecvec_dismatrix_geodesic;
	
	//compute the Euclidian distance of every possible pairs of input point set
	//then use Euclidian distance to initialize the Geodesic distance
	if(!q_compute_dismatrix_euclidian(vec_input,vecvec_dismatrix_euclidian))
	{
        fprintf(stderr,"ERROR: q_compute_dismatrix_geodesic() return false! \n");
        return false;
	}
	vecvec_dismatrix_geodesic=vecvec_dismatrix_euclidian;
	
	//only keep the local distance finite, i.e. arr_dismatrix[i][j]=inf, if j is not i's K nearest neighbors
	vector< vector<double> > vecvec_temp(vecvec_dismatrix_geodesic);
	vector< vector<int> > vecvec_temp_index;
	if(!q_sort2d(vecvec_temp,vecvec_temp_index,1,1))//sort every row of the dismatrix from low to high
	{
		fprintf(stderr,"ERROR: q_compute_dismatrix_geodesic() return false! \n");
		return false;
	}
	for(int i=0;i<N;i++)
		for(int j=K+1;j<N;j++)
		{
			int ind=vecvec_temp_index[i][j];
			vecvec_dismatrix_geodesic[i][ind]=INF;
		}
	vecvec_temp.clear();
	vecvec_temp_index.clear();
	
	//compute pairwise geodesic distance iteratively untill all isolated groups are connected
	while(1)
	{
		//make sure the arr_dismatrix is symmetric
		//nonsymmetric occur when: j is i's K nearest neighbor, but not vise versa
		for(int i=0;i<N;i++)
			for(int j=i;j<N;j++)
			{
				double mindis=min(vecvec_dismatrix_geodesic[i][j],vecvec_dismatrix_geodesic[j][i]);
				vecvec_dismatrix_geodesic[i][j]=mindis;
				vecvec_dismatrix_geodesic[j][i]=mindis;
			}
		
		//compute pairwise geodesic distance
		//dis_ij=min(dis_ij,dis_ik+dis_kj), k=1:N
		for(int k=0;k<N;k++)
		{
			for(int i=0;i<N;i++)
				for(int j=0;j<N;j++)
				{
					double ikj=vecvec_dismatrix_geodesic[i][k]+vecvec_dismatrix_geodesic[k][j];
					vecvec_dismatrix_geodesic[i][j]=min(vecvec_dismatrix_geodesic[i][j],ikj);
				}
		}
		
		//if do not exist isolated groups - exit iteration
		bool b_hasisolategroup=false;
		for(int i=0;i<N;i++)
			for(int j=0;j<N;j++)
				if(vecvec_dismatrix_geodesic[i][j]>INF/2)
					b_hasisolategroup=true;
		if(b_hasisolategroup)
		{
			;
			//			printf("\t>>found isolated groups, initiate isolated groups segmentating\n");
		}
		else
		{
			//			printf("\t>>do not found isolated groups, Geodesic distance detecting exit sucess!\n");
			break;
		}
		
		//find the isolated groups - the geodesic distance between isolated groups is inf
		vector< vector<int> > vecvec_groupsind;
		vector<bool> vec_isgrouped(N,0);
		while(1)
		{
			//find one ungouped point index
			int n_1ungrouped_ind=-1;
			for(int i=0;i<N;i++)
				if(vec_isgrouped[i]==0)
				{
					n_1ungrouped_ind=i;
					//					printf("%d\n",i);
					break;
				}
			if(n_1ungrouped_ind==-1)
				break;
			
			//find all points connnected to this point, and take them as one group
			vector<int> vec_groupind;
			for(int i=0;i<N;i++)
				if(vecvec_dismatrix_geodesic[n_1ungrouped_ind][i]<INF/2)
				{
					vec_groupind.push_back(i);
					vec_isgrouped[i]=1;
				}
			vecvec_groupsind.push_back(vec_groupind);
		}
		//		printf("\t>>%d isolated groups founded, initiate groups merging\n",vecvec_groupsind.size());
		
		//connect the isolated groups by only connect their two nearest (Euclidian) element
		//then if we run the geodesic distance algorithm again, all the others distance can be obtained through this new connection
		for(int i=0;i<vecvec_groupsind.size();i++)
			for(int j=i+1;j<vecvec_groupsind.size();j++)
			{
				//find the nearest Euclidian distance and correponding point index between group i and j
				double d_ii2jj_mindis=INF;
				int n_ii_minind,n_jj_minind;
				for(int ii=0;ii<vecvec_groupsind[i].size();ii++)
					for(int jj=0;jj<vecvec_groupsind[j].size();jj++)
					{
						int n_ii_ind=vecvec_groupsind[i][ii];
						int n_jj_ind=vecvec_groupsind[j][jj];
						double d_dis_ii2jj=vecvec_dismatrix_euclidian[n_ii_ind][n_jj_ind];
						if(d_dis_ii2jj<d_ii2jj_mindis)
						{
							d_ii2jj_mindis=d_dis_ii2jj;
							n_ii_minind=n_ii_ind;
							n_jj_minind=n_jj_ind;
						}
					}
				
				//connect the nearest two points in two groups with smallest Euclidian distance
				vecvec_dismatrix_geodesic[n_ii_minind][n_jj_minind]=d_ii2jj_mindis;
			}
	}
	
	vec_dismatrix=vecvec_dismatrix_geodesic;
}

//compute the context similarity - NN interpolate
bool q_compare_context_nn(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,vector< vector<double> > &vecvec_simmatrix)
{
	int n_point_A,n_point_B;
	n_point_A=vecvec_dismatrix_A.size();
	n_point_B=vecvec_dismatrix_B.size();

	//check parameters
	if(n_point_A<=0 || vecvec_dismatrix_A[0].size()<=0 || n_point_A!=vecvec_dismatrix_A[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_A size! \n");
        return false;
	}
	if(n_point_B<=0 || vecvec_dismatrix_B[0].size()<=0 || n_point_B!=vecvec_dismatrix_B[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_B size! \n");
        return false;
	}
	if(vecvec_simmatrix.empty())
	{
		vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}
	if(vecvec_simmatrix.size()!=n_point_A || vecvec_simmatrix[0].size()!=n_point_B)
	{
        fprintf(stderr,"WARNING: arr_simmatrix is not empty, and has wrong size, original data will be deleted! \n");
        vecvec_simmatrix.clear();
        vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}

	// length(A)/length(B)
	double d_lenAonlenB=double(n_point_A)/double(n_point_B);

	//compare every row of arr_dismatrix_A with every row of arr_dismatrix_B
	//	first intepolate every row of arr_dismatrix_A to make it the same lengh as B
	//	then we can compare their similarity use the simplest correlation method
	int posA_NN;
	double err,temp;
	for(int i=0;i<n_point_A;i++)//compare ith row of A with jth row of B
	{
		for(int j=0;j<n_point_B;j++)
		{
			vecvec_simmatrix[i][j]=0.0;
			for(int k=0;k<n_point_B;k++)
			{
				//interpolate the correpsponding position of A for B's current positon k
				posA_NN=int(k*d_lenAonlenB+0.5);
				posA_NN=(posA_NN<0) ? 0 : posA_NN;	posA_NN=(posA_NN>=n_point_A) ? n_point_A-1 : posA_NN;
				//accumulate the error
				temp=vecvec_dismatrix_A[i][posA_NN]-vecvec_dismatrix_B[j][k];
				err=temp*temp;
				vecvec_simmatrix[i][j]+=err;
			}
		}
	}
	//	int posB_NN;
	//	double err,temp;
	//	for(int i=0;i<n_point_A;i++)//compare ith row of A with jth row of B
	//	{
	//		for(int j=0;j<n_point_B;j++)
	//		{
	//			arr_simmatrix[i][j]=0.0;
	//			for(int k=0;k<n_point_A;k++)
	//			{
	//				//interpolate the correpsponding position of B for A's current positon k
	//				posB_NN=int(k/d_lenAonlenB+0.5);
	//				posB_NN=(posB_NN<0) ? 0 : posB_NN;	posB_NN=(posB_NN>=n_point_B) ? n_point_B-1 : posB_NN;
	//				//accumulate the error
	//				temp=arr_dismatrix_A[i][k]-arr_dismatrix_B[j][posB_NN];
	//				err=temp*temp;
	//				arr_simmatrix[i][j]+=err;
	//			}
	//		}
	//	}

	return true;
}

//compute the context similarity - linear interpolate
bool q_compare_context_linear(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,vector< vector<double> > &vecvec_simmatrix)
{
	int n_point_A,n_point_B;
	n_point_A=vecvec_dismatrix_A.size();
	n_point_B=vecvec_dismatrix_B.size();

	//check parameters
	if(n_point_A<=0 || vecvec_dismatrix_A[0].size()<=0 || n_point_A!=vecvec_dismatrix_A[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_A size! \n");
        return false;
	}
	if(n_point_B<=0 || vecvec_dismatrix_B[0].size()<=0 || n_point_B!=vecvec_dismatrix_B[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_B size! \n");
        return false;
	}
	if(vecvec_simmatrix.empty())
	{
		vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}
	if(vecvec_simmatrix.size()!=n_point_A || vecvec_simmatrix[0].size()!=n_point_B)
	{
        fprintf(stderr,"WARNING: arr_simmatrix is not empty, and has wrong size, original data will be deleted! \n");
        vecvec_simmatrix.clear();
        vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}

	// length(A)/length(B)
	double d_lenAonlenB=double(n_point_A)/double(n_point_B);

	//compare every row of arr_dismatrix_A with every row of arr_dismatrix_B
	//	first intepolate every row of arr_dismatrix_A to make it the same lengh as B
	//	then we can compare their similarity use the simplest correlation method
	int posA_floor,posA_ceil;
	double left,right,fraction,interpolated_A;
	double err,temp;
	for(int i=0;i<n_point_A;i++)//compare ith row of A with jth row of B
	{
		for(int j=0;j<n_point_B;j++)
		{
			vecvec_simmatrix[i][j]=0.0;
			for(int k=0;k<n_point_B;k++)
			{
				//linear interpolate
				posA_floor=floor(k*d_lenAonlenB);	posA_ceil=ceil(k*d_lenAonlenB);
				posA_floor=(posA_floor<0) ? 0 : posA_floor;	posA_floor=(posA_floor>=n_point_A) ? n_point_A-1 : posA_floor;
				posA_ceil=(posA_ceil<0) ? 0 : 	posA_ceil;	posA_ceil=(posA_ceil>=n_point_A) ? 	 n_point_A-1 : posA_ceil;
				fraction=k*d_lenAonlenB-posA_floor;
				left=vecvec_dismatrix_A[i][posA_floor];
				right=vecvec_dismatrix_A[i][posA_ceil];
				interpolated_A=left*fraction+right*(1-fraction);
				//accumulate the error
				temp=interpolated_A-vecvec_dismatrix_B[j][k];
				err=temp*temp;
				vecvec_simmatrix[i][j]+=err;
			}
		}
	}

	return true;
}

//compute the context similarity - histogram
bool q_compare_context_histogram(const vector< vector<double> > &vecvec_dismatrix_A,const vector< vector<double> > &vecvec_dismatrix_B,const int n_bin,vector< vector<double> > &vecvec_simmatrix)
{
	int n_point_A,n_point_B;
	n_point_A=vecvec_dismatrix_A.size();
	n_point_B=vecvec_dismatrix_B.size();

	//check parameters
	if(n_point_A<=0 || vecvec_dismatrix_A[0].size()<=0 || n_point_A!=vecvec_dismatrix_A[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_A size! \n");
        return false;
	}
	if(n_point_B<=0 || vecvec_dismatrix_B[0].size()<=0 || n_point_B!=vecvec_dismatrix_B[0].size())
	{
        fprintf(stderr,"ERROR: Invalid input arr_dismatrix_B size! \n");
        return false;
	}
	if(vecvec_simmatrix.empty())
	{
		vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}
	if(vecvec_simmatrix.size()!=n_point_A || vecvec_simmatrix[0].size()!=n_point_B)
	{
        fprintf(stderr,"WARNING: arr_simmatrix is not empty, and has wrong size, original data will be deleted! \n");
        vecvec_simmatrix.clear();
        vecvec_simmatrix.assign(n_point_A,vector<double>(n_point_B,0.0));
	}

	//compute the bin width for given n_bin
	double d_binwidth=0,d_maxdis_AB=0;
	//find the max distant in arr_dismatrix_A and arr_dismatrix_B
	for(int i=0;i<n_point_A;i++)
		for(int j=0;j<n_point_A;j++)
		{
			if(d_maxdis_AB<vecvec_dismatrix_A[i][j])
				d_maxdis_AB=vecvec_dismatrix_A[i][j];
		}
	for(int i=0;i<n_point_B;i++)
		for(int j=0;j<n_point_B;j++)
		{
			if(d_maxdis_AB<vecvec_dismatrix_B[i][j])
				d_maxdis_AB=vecvec_dismatrix_B[i][j];
		}
	//compute the bin width
	d_binwidth=d_maxdis_AB/n_bin;

	//build histogram and compare histogram
	vector<int> his_A,his_B;
	int binhit,err,temp;
	for(int i=0;i<n_point_A;i++)//compare ith row of A with jth row of B
	{
		//build histrogram for every row of arr_dismatrix_A
		his_A.assign(n_bin,0);
		for(int k=0;k<n_point_A;k++)
		{
			binhit=floor(vecvec_dismatrix_A[i][k]/d_binwidth);
			binhit=(binhit<0) ? 0 : binhit;	binhit=(binhit>=n_bin) ? n_bin-1 : binhit;
			his_A[binhit]+=1;
		}

		for(int j=0;j<n_point_B;j++)
		{
			//build histrogram for every row of arr_dismatrix_B
			his_B.assign(n_bin,0);
			for(int k=0;k<n_point_B;k++)
			{
				binhit=floor(vecvec_dismatrix_B[j][k]/d_binwidth);
				binhit=(binhit<0) ? 0 : binhit;	binhit=(binhit>=n_bin) ? n_bin-1 : binhit;
				his_B[binhit]+=1;
			}
			//compare the histogram
			err=0;
			for(int k=0;k<n_bin;k++)
			{
				temp=his_A[k]-his_B[k];
				err+=temp*temp;
			}
			vecvec_simmatrix[i][j]=err;
		}
	}

	return true;
}
