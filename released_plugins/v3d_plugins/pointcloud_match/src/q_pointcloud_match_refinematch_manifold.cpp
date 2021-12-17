// q_pointcloud_match_refinematch_manifold.cpp
// manifold based refine matching - local consensus optimization
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.cpp by Lei Qu on 2010-01-22

#include <math.h>
#include "q_pointcloud_match_refinematch_manifold.h"


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
		vector<int> &vec_A2Bindex_refined)
{
	//check parameters
	if(vec_A2Bindex.empty() 		|| vec_A_norm.empty() 			|| vec_B_norm.empty() ||
	   vecvec_dismatrix_A.empty() 	|| vecvec_dismatrix_B.empty() 	|| vecvec_votematrix.empty())
	{
		fprintf(stderr,"ERROR: At least one input para is empty! \n");
		return false;
	}
	if(vec_A_norm.size()!=vec_A2Bindex.size() || vec_A_norm.size()!=vecvec_dismatrix_A.size() || vec_A_norm.size()!=vecvec_dismatrix_A[0].size() ||
	   vec_A_norm.size()!=vecvec_votematrix.size())
	{
		fprintf(stderr,"ERROR: The size of at least one input vector is invalid (related to A)! \n");
		return false;
	}
	if(vec_B_norm.size()!=vecvec_dismatrix_B.size() || vec_B_norm.size()!=vecvec_dismatrix_B[0].size() || vec_B_norm.size()!=vecvec_votematrix[0].size())
	{
		fprintf(stderr,"ERROR: The size of at least one input vector is invalid (related to B)! \n");
		return false;
	}
	int n_A=vec_A_norm.size();
	int n_B=vec_B_norm.size();
	if(n_A<5 || n_B<5)
	{
		fprintf(stderr,"ERROR: Input array/arrays should have more than 4 points! \n");
		return false;
	}
	if(n_A>n_B)
	{
		fprintf(stderr,"ERROR: size(A) should <= size(B)! \n");
		return false;
	}
	int n_constrainter=0;
	for(int i=0;i<100;i++)
		if(arr_constrainter[i])
			n_constrainter++;
	if(n_constrainter==0)
	{
		fprintf(stderr,"ERROR: At least 1 n_constrainter needed! \n");
		return false;
	}

	vec_A2B_cost.clear();
	vec_A2Bindex_refined.clear();
	vec_A2Bindex_refined.assign(n_A,-1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t0. preliminery computation\n");
	//sort every row of the dismatrix from low to high before perform comparing
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
	//sort the every row of the votematrix from high to low
	vector< vector<double> > vecvec_votematrix_sort(vecvec_votematrix);
	vector< vector<int> > vecvec_votematrix_sort_index;
	if(!q_sort2d(vecvec_votematrix_sort, vecvec_votematrix_sort_index,1,0))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(1). shift normalized B one uint away (to decrease the effect of big direction variation caused by close distance)
	printf("\t1. do artificial uint shift on B \n");
	vector<Coord3D_PCM> vec_B_norm_shift(vec_B_norm);
	for(unsigned int i=0;i<n_B;i++)
	{
		vec_B_norm_shift[i].x+=1;
		vec_B_norm_shift[i].y+=1;
		vec_B_norm_shift[i].z+=1;
	}

	//(2). minimize the total direction difference in a manifold way (optimize the local avg dir iteratively)
	printf("\t2. minimize the total direction difference in a manifold way (optimize the local avg dir iteratively) \n");
	vec_A2Bindex_refined=vec_A2Bindex;
	vec_A2B_cost.assign(n_A,-1.0);//record the cost of point A matched to point B, this value will be used to break multi-matched pairs
	for(int iter=0;iter<n_iternum;iter++)
	{
		//build the direction and magnitude/dis matrix for every matched pair AB
		vector<Coord3D_PCM>	vec_dirmatrix_A2B(n_A,Coord3D_PCM());
		vector<double>		vec_dismatrix_A2B(n_A,0.0);
		for(unsigned int i=0;i<n_A;i++)
		{
			vec_dirmatrix_A2B[i].x=vec_A_norm[i].x-vec_B_norm_shift[vec_A2Bindex_refined[i]].x;
			vec_dirmatrix_A2B[i].y=vec_A_norm[i].y-vec_B_norm_shift[vec_A2Bindex_refined[i]].y;
			vec_dirmatrix_A2B[i].z=vec_A_norm[i].z-vec_B_norm_shift[vec_A2Bindex_refined[i]].z;
			vec_dismatrix_A2B[i]=sqrt(vec_dirmatrix_A2B[i].x*vec_dirmatrix_A2B[i].x+
					                  vec_dirmatrix_A2B[i].y*vec_dirmatrix_A2B[i].y+
					                  vec_dirmatrix_A2B[i].z*vec_dirmatrix_A2B[i].z);
			vec_dirmatrix_A2B[i].x/=vec_dismatrix_A2B[i];
			vec_dirmatrix_A2B[i].y/=vec_dismatrix_A2B[i];
			vec_dirmatrix_A2B[i].z/=vec_dismatrix_A2B[i];
		}

		//compute the avgdir(which use dis2nighbor as weight) and avgdis for every A in k-nearest neighborhood
		vector<Coord3D_PCM>	vec_avgdirmatrix_A2B(n_A,Coord3D_PCM());
		vector<double>		vec_avgdismatrix_A2B(n_A,0.0);
		int index_neighbor;
		for(unsigned int i=0;i<n_A;i++)
		{
//			double totaldis2neighbor=0.0;
//			double totaldis2neighbor_inv=0.0;
//			for(int k=0;k<n_neighbor;k++)
//			{
//				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
//				totaldis2neighbor+=vecvec_dismatrix_A_sort[i][index_neighbor];
//			}
//			for(int k=0;k<n_neighbor;k++)
//			{
//				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
//				totaldis2neighbor_inv+=1.0-vecvec_dismatrix_A_sort[i][index_neighbor]/totaldis2neighbor;
//			}

			for(int k=0;k<n_neighbor;k++)
			{
				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
				double disweight=1.0/n_neighbor;
//				double disweight=(1.0-vecvec_dismatrix_A_sort[i][index_neighbor]/totaldis2neighbor)/totaldis2neighbor_inv;
				vec_avgdirmatrix_A2B[i].x+=vec_dirmatrix_A2B[index_neighbor].x*disweight;
				vec_avgdirmatrix_A2B[i].y+=vec_dirmatrix_A2B[index_neighbor].y*disweight;
				vec_avgdirmatrix_A2B[i].z+=vec_dirmatrix_A2B[index_neighbor].z*disweight;
				vec_avgdismatrix_A2B[i]+=vec_dismatrix_A2B[index_neighbor]*disweight;
			}
		}

		//update the matched point to which with minimal direction difference to avgdir
		vector<int> vec_A2Bindex_update(n_A,-1);
		Coord3D_PCM dir_AB;
		double d_dis_AB;
		double d_angle2avg,d_angle2avg_min;
		double d_avgdirerr=0.0;
		for(unsigned int i=0;i<n_A;i++)
		{
			d_angle2avg_min=360.0;
			for(unsigned int j=0;j<n_B;j++)
			{
				dir_AB.x=vec_A_norm[i].x-vec_B_norm_shift[j].x;
				dir_AB.y=vec_A_norm[i].y-vec_B_norm_shift[j].y;
				dir_AB.z=vec_A_norm[i].z-vec_B_norm_shift[j].z;
				d_dis_AB=sqrt(dir_AB.x*dir_AB.x+dir_AB.y*dir_AB.y+dir_AB.z*dir_AB.z);
				dir_AB.x/=d_dis_AB;
				dir_AB.y/=d_dis_AB;
				dir_AB.z/=d_dis_AB;
				d_angle2avg=acos(dir_AB.x*vec_avgdirmatrix_A2B[i].x+
								 dir_AB.y*vec_avgdirmatrix_A2B[i].y+
								 dir_AB.z*vec_avgdirmatrix_A2B[i].z);
				d_angle2avg=(d_angle2avg/3.1415926)*180;
				if(d_angle2avg<d_angle2avg_min)
				{
					d_angle2avg_min=d_angle2avg;
					vec_A2Bindex_update[i]=j;
				}
			}
			d_avgdirerr+=d_angle2avg_min;
			vec_A2B_cost[i]=d_angle2avg;
		}
		d_avgdirerr/=n_A;
		printf("\t\t(%4d).    avgdirerr=%.4f\n",iter,d_avgdirerr);

		//update matching index
		for(unsigned int i=0;i<n_A;i++)
			vec_A2Bindex_refined[i]=vec_A2Bindex_update[i];
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(3). further minimize the total dir, dis different in a manifold way, also limit the candidates to several top voted ones
	printf("\t3. further minimize the total dir, dis different in a manifold way, also limit the candidates to several top voted ones \n");
	vec_A2B_cost.assign(n_A,-1.0);//record the cost of point A matched to point B, this value will be used to break multi-matched pairs
	for(int iter=0;iter<n_iternum;iter++)
	{
		//build the direction and magnitude matrix for every matched pair AB
		vector<Coord3D_PCM> vec_dirmatrix_A2B(n_A,Coord3D_PCM());
		Coord3D_PCM 		dir_A2B_noshift;
		vector<double> 		vec_dismatrix_A2B(n_A,0.0);
		vector<double> 		vec_dismatrix_A2B_noshift(n_A,0.0);
		for(unsigned int i=0;i<n_A;i++)
		{
			vec_dirmatrix_A2B[i].x=vec_A_norm[i].x-vec_B_norm_shift[vec_A2Bindex_refined[i]].x;
			vec_dirmatrix_A2B[i].y=vec_A_norm[i].y-vec_B_norm_shift[vec_A2Bindex_refined[i]].y;
			vec_dirmatrix_A2B[i].z=vec_A_norm[i].z-vec_B_norm_shift[vec_A2Bindex_refined[i]].z;
			vec_dismatrix_A2B[i]=sqrt(vec_dirmatrix_A2B[i].x*vec_dirmatrix_A2B[i].x+
					                  vec_dirmatrix_A2B[i].y*vec_dirmatrix_A2B[i].y+
					                  vec_dirmatrix_A2B[i].z*vec_dirmatrix_A2B[i].z);
			vec_dirmatrix_A2B[i].x/=vec_dismatrix_A2B[i];
			vec_dirmatrix_A2B[i].y/=vec_dismatrix_A2B[i];
			vec_dirmatrix_A2B[i].z/=vec_dismatrix_A2B[i];

			//no shift
			dir_A2B_noshift.x=vec_A_norm[i].x-(vec_B_norm_shift[vec_A2Bindex_refined[i]].x-1.0);
			dir_A2B_noshift.y=vec_A_norm[i].y-(vec_B_norm_shift[vec_A2Bindex_refined[i]].y-1.0);
			dir_A2B_noshift.z=vec_A_norm[i].z-(vec_B_norm_shift[vec_A2Bindex_refined[i]].z-1.0);
			vec_dismatrix_A2B_noshift[i]=sqrt(dir_A2B_noshift.x*dir_A2B_noshift.x+
											  dir_A2B_noshift.y*dir_A2B_noshift.y+
											  dir_A2B_noshift.z*dir_A2B_noshift.z);
		}

		//compute the avgdir(which use dis2nighbor as weight) and avgdis for every A
		vector<Coord3D_PCM> vec_avgdirmatrix_A2B(n_A,Coord3D_PCM());
		vector<double> 		vec_avgdismatrix_A2B(n_A,0.0);
		vector<double> 		vec_avgdismatrix_A2B_noshift(n_A,0.0);
		int index_neighbor;
		for(unsigned int i=0;i<n_A;i++)
		{
//			double totaldis2neighbor=0.0;
//			double totaldis2neighbor_inv=0.0;
//			for(int k=0;k<n_neighbor;k++)
//			{
//				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
//				totaldis2neighbor+=vecvec_dismatrix_A_sort[i][index_neighbor];
//			}
//			for(int k=0;k<n_neighbor;k++)
//			{
//				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
//				totaldis2neighbor_inv+=1.0-vecvec_dismatrix_A_sort[i][index_neighbor]/totaldis2neighbor;
//			}

			for(int k=0;k<n_neighbor;k++)
			{
				index_neighbor=vecvec_dismatrix_A_sortindex[i][k+1];
				double disweight=1.0/n_neighbor;
//				double disweight=(1.0-arr_dismatrix_A_sort[i][index_neighbor]/totaldis2neighbor)/totaldis2neighbor_inv;
				vec_avgdirmatrix_A2B[i].x+=vec_dirmatrix_A2B[index_neighbor].x*disweight;
				vec_avgdirmatrix_A2B[i].y+=vec_dirmatrix_A2B[index_neighbor].y*disweight;
				vec_avgdirmatrix_A2B[i].z+=vec_dirmatrix_A2B[index_neighbor].z*disweight;
				vec_avgdismatrix_A2B[i]+=vec_dismatrix_A2B[index_neighbor]*disweight;
				vec_avgdismatrix_A2B_noshift[i]+=vec_dismatrix_A2B_noshift[index_neighbor]*disweight;
			}
		}

		//update the matched point to the highly voted candidates with minimal dirdis cost
		vector<int> vec_A2Bindex_update(n_A,-1);
		Coord3D_PCM vec_dir_AB;
		Coord3D_PCM vec_dir_AB_noshift;
		double d_dis_AB;
		double d_dis_AB_noshift;
		double d_angle2avg;
		double d_dis2avg_cost,d_angle2avg_cost,d_dirdis_cost,d_dirdis_cost_min,d_dirdis_cost_avg=0.0;
		int n_candidate_index;
        for(unsigned int i=0;i<n_A;i++)
        {
			d_dirdis_cost_min=1e+10;
			int n_curcandidatenum;
			if(!arr_constrainter[2])
				n_curcandidatenum=n_B;//disable top vote candidates constrainter
			else
				n_curcandidatenum=n_topcandidatenum;
			for(int k=0;k<n_curcandidatenum;k++)//limit the candidates to several top voted ones
			{
				n_candidate_index=vecvec_votematrix_sort_index[i][k];

				vec_dir_AB.x=vec_A_norm[i].x-vec_B_norm_shift[n_candidate_index].x;
				vec_dir_AB.y=vec_A_norm[i].y-vec_B_norm_shift[n_candidate_index].y;
				vec_dir_AB.z=vec_A_norm[i].z-vec_B_norm_shift[n_candidate_index].z;
				d_dis_AB=sqrt(vec_dir_AB.x*vec_dir_AB.x+vec_dir_AB.y*vec_dir_AB.y+vec_dir_AB.z*vec_dir_AB.z);
				vec_dir_AB.x/=d_dis_AB;
				vec_dir_AB.y/=d_dis_AB;
				vec_dir_AB.z/=d_dis_AB;

				vec_dir_AB_noshift.x=vec_A_norm[i].x-(vec_B_norm_shift[n_candidate_index].x-1.0);
				vec_dir_AB_noshift.y=vec_A_norm[i].y-(vec_B_norm_shift[n_candidate_index].y-1.0);
				vec_dir_AB_noshift.z=vec_A_norm[i].z-(vec_B_norm_shift[n_candidate_index].z-1.0);
				d_dis_AB_noshift=sqrt(vec_dir_AB_noshift.x*vec_dir_AB_noshift.x+vec_dir_AB_noshift.y*vec_dir_AB_noshift.y+vec_dir_AB_noshift.z*vec_dir_AB_noshift.z);

				d_angle2avg=acos(vec_dir_AB.x*vec_avgdirmatrix_A2B[i].x+
								 vec_dir_AB.y*vec_avgdirmatrix_A2B[i].y+
								 vec_dir_AB.z*vec_avgdirmatrix_A2B[i].z);
				d_angle2avg=(d_angle2avg/3.1415926)*180;

//				d_dis2avg_cost=fabs(d_dis_AB_noshift/arr_avgdismatrix_A2B_noshift[i]-1.0);
				d_dis2avg_cost=fabs(d_dis_AB/vec_avgdismatrix_A2B[i]-1.0);
				d_angle2avg_cost=fabs(d_angle2avg/30.0);//here 40 served as a threshold, i.e. when d_angle2avg=40, d_angle2avg_cost=1

				if(arr_constrainter[1])	//enable dis constrainter
					d_dirdis_cost=d_dis2avg_cost+d_angle2avg_cost;
				else					//disable dis constrainter
					d_dirdis_cost=d_angle2avg_cost;

				if(d_dirdis_cost<d_dirdis_cost_min)
				{
					d_dirdis_cost_min=d_dirdis_cost;
					vec_A2Bindex_update[i]=n_candidate_index;
				}
			}
			d_dirdis_cost_avg+=d_dirdis_cost_min;
			vec_A2B_cost[i]=d_dirdis_cost_min;
        }
		d_dirdis_cost_avg/=n_A;
		printf("\t\t(%4d).    avgdirdiserr=%.4f\n",iter,d_dirdis_cost_avg);

		//update matching index
		for(unsigned int i=0;i<n_A;i++)
			vec_A2Bindex_refined[i]=vec_A2Bindex_update[i];
	}

	return true;
}


