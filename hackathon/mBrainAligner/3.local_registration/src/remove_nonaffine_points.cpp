//remove_nonaffine_points.cpp
// by Lei Qu and Hanchuan Peng
// 2009-2011


#include <math.h>
#include <time.h>

#include "remove_nonaffine_points.h"

bool q_killwrongmatch(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,
					const int n_sampling, const int n_pairs_per_sampling, const float f_kill_factor,
					Matrix &x4x4_affinematrix_2to1,vector<int> &arr1d_1to2index,vector<Coord3D_PCR> &arr_2_invp,
					vector<Coord3D_PCR> &arr_1_afterkill,vector<Coord3D_PCR> &arr_2_afterkill)
{
	if(arr_1.size()<5 || arr_2.size()<5)
	{
		fprintf(stderr,"ERROR: q_killwrongmatch: Input array/arrays should have more than 4 points! \n");
		return false;
	}
	if(arr_1.size()!=arr_2.size())
	{
		fprintf(stderr,"ERROR: q_killwrongmatch: Input array/arrays should have same size! \n");
		return false;
	}
	if(n_sampling<0)
	{
		fprintf(stderr,"ERROR: q_killwrongmatch: n_sampling should not be negative! \n");
		return false;
	}
	if(n_pairs_per_sampling<5)
	{
		fprintf(stderr,"ERROR: q_killwrongmatch: n_pairs_persample should not be less than 5! \n");
		return false;
	}
	if(x4x4_affinematrix_2to1.nrows()!=4 || x4x4_affinematrix_2to1.ncols()!=4)
	{
		x4x4_affinematrix_2to1.ReSize(4,4);
	}
	if(arr1d_1to2index.size()!=arr_1.size())
	{
		arr1d_1to2index.clear();
		arr1d_1to2index.assign(arr_1.size(),-1);
	}
	if(arr_2_invp.size()!=arr_1.size())
	{
		arr_2_invp.clear();
		arr_2_invp.assign(arr_1.size(),Coord3D_PCR());
	}
	if(arr_1_afterkill.size()!=0)
	{
		arr_1_afterkill.clear();
	}
	if(arr_2_afterkill.size()!=0)
	{
		arr_2_afterkill.clear();
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	//(1). normalize the point cloud
	printf("(1). normalize the point cloud \n");
	vector<Coord3D_PCR> arr_1_norm,arr_2_norm;
	Matrix x4x4_arr1_norm(4,4),x4x4_arr2_norm(4,4);
	if(!q_normalize_points(arr_1,arr_1_norm,x4x4_arr1_norm))
	{
		fprintf(stderr,"ERROR: q_point_cloud_registration_context_RANSAC_voting: q_normalize_points return false! \n");
		return false;
	}
	if(!q_normalize_points(arr_2,arr_2_norm,x4x4_arr2_norm))
	{
		fprintf(stderr,"ERROR: q_point_cloud_registration_context_RANSAC_voting: q_normalize_points return false! \n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(2). estimate the affine matrix from given matching using RANSAC
	printf("(2). estimate the initial affine matrix from initial matching using RANSAC \n");
	for(int i=0;i<arr_1.size();i++)
		arr1d_1to2index[i]=i;
	Matrix x4x4_affinematrix(4,4);
	if(!q_RANSAC_affinmatrix_from_initialmatch(arr_1_norm,arr_2_norm,arr1d_1to2index,
											   n_sampling,n_pairs_per_sampling,
											   x4x4_affinematrix))
	{
		fprintf(stderr,"ERROR: q_point_cloud_registration_context_RANSAC_voting: q_compute_affinmatrix_from_initialmatch return false! \n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(3). kill the wrong matches by comparing the distance between A and the corresponding B_invp
	printf("(3). kill the wrong matches by comparing the distance between A and the corresponding B_invp\n");
	//	compute the inverse projected points
	vector<Coord3D_PCR> arr_2_norm_invp(arr_2.size(),Coord3D_PCR());
	Matrix x_arr2_invp(4,arr_2.size()), x_arr2_homo(4,arr_2.size());
	for(int i=0;i<arr_2.size();i++)
	{
		x_arr2_homo(1,i+1)=arr_2_norm[i].x;
		x_arr2_homo(2,i+1)=arr_2_norm[i].y;
		x_arr2_homo(3,i+1)=arr_2_norm[i].z;
		x_arr2_homo(4,i+1)=1.0;
	}
	x_arr2_invp=x4x4_affinematrix*x_arr2_homo;
	for(int i=0;i<arr_2.size();i++)
	{
		arr_2_norm_invp[i].x=x_arr2_invp(1,i+1);
		arr_2_norm_invp[i].y=x_arr2_invp(2,i+1);
		arr_2_norm_invp[i].z=x_arr2_invp(3,i+1);
	}
	//	compute the average distance between A and the corresponding B_invp
	float invpdis_avg=0.0,dx,dy,dz;
	for(int i=0;i<arr_1.size();i++)
	{
		dx=arr_1_norm[i].x-arr_2_norm_invp[i].x;
		dy=arr_1_norm[i].y-arr_2_norm_invp[i].y;
		dz=arr_1_norm[i].z-arr_2_norm_invp[i].z;
		invpdis_avg+=sqrt(dx*dx+dy*dy+dz*dz);
	}
	invpdis_avg/=arr_1.size();
	//  compute the average distance of point to its nearest neighbor
	float mindis_avg=0.0;
	for(int i=0;i<arr_1.size();i++)
	{
		float min_dis=1e+10;
		for(int j=0;j<arr_1.size();j++)
		{
			if(i==j) continue;
			dx=arr_1_norm[i].x-arr_1_norm[j].x;
			dy=arr_1_norm[i].y-arr_1_norm[j].y;
			dz=arr_1_norm[i].z-arr_1_norm[j].z;
			float dis=sqrt(dx*dx+dy*dy+dz*dz);
			if(dis<min_dis)	min_dis=dis;
		}
		mindis_avg+=min_dis;
	}
	mindis_avg/=arr_1.size();
	//	kill the matches with invpdis > 2.0*invpdis_avg && invpdis > mindis_avg*alpha (is there better judgement?)
	float invpdis;
	for(int i=0;i<arr_1.size();i++)
	{
		dx=arr_1_norm[i].x-arr_2_norm_invp[i].x;
		dy=arr_1_norm[i].y-arr_2_norm_invp[i].y;
		dz=arr_1_norm[i].z-arr_2_norm_invp[i].z;
		invpdis=sqrt(dx*dx+dy*dy+dz*dz);
		if(invpdis>invpdis_avg*f_kill_factor && invpdis>mindis_avg*0.01)
		{
			arr1d_1to2index[i]=-1;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//(4).de-normalization and generate output
	printf("(4). de-normalization and get arr_2_invp! \n");

	x4x4_affinematrix_2to1=x4x4_arr1_norm.i()*x4x4_affinematrix*x4x4_arr2_norm;		//de-normlize

	//inverse project arr_2 use affine matrix
	//	arr_2_invp = x4x4_affinematrix_2to1 * x_arr2_homo
	for(int i=0;i<arr_2.size();i++)
	{
		x_arr2_homo(1,i+1)=arr_2[i].x;
		x_arr2_homo(2,i+1)=arr_2[i].y;
		x_arr2_homo(3,i+1)=arr_2[i].z;
		x_arr2_homo(4,i+1)=1.0;
	}
	x_arr2_invp=x4x4_affinematrix_2to1*x_arr2_homo;
	for(int i=0;i<arr_2.size();i++)
	{
		arr_2_invp[i].x=x_arr2_invp(1,i+1);
		arr_2_invp[i].y=x_arr2_invp(2,i+1);
		arr_2_invp[i].z=x_arr2_invp(3,i+1);
	}

	//reorganize matched pairs
	for(int i=0;i<arr_1.size();i++)
	{
		if(arr1d_1to2index[i]!=-1)
		{
			arr_1_afterkill.push_back(arr_1[i]);
			arr_2_afterkill.push_back(arr_2[i]);
		}
	}

	return true;
}



//************************************************************************************************************************************

//centrilize and scale the point set
//	xn = T*x;
//	x: every column represent a point [3*N]
bool q_normalize_points(const vector<Coord3D_PCR> arr_input,vector<Coord3D_PCR> &arr_output,Matrix &x4x4_normalize)
{
	//check parameters
	if(arr_input.size()<=0)
	{
		fprintf(stderr,"ERROR: q_normalize_points: Input array is null! \n");
		return false;
	}
	if(arr_output.size()>0)
		arr_output.clear();


	arr_output=arr_input;

	//compute the centriod of input point set
	Coord3D_PCR cord_centroid;
	int n_point=arr_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=arr_input[i].x;
		cord_centroid.y+=arr_input[i].y;
		cord_centroid.z+=arr_input[i].z;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	cord_centroid.z/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		arr_output[i].x-=cord_centroid.x;
		arr_output[i].y-=cord_centroid.y;
		arr_output[i].z-=cord_centroid.z;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(arr_output[i].x*arr_output[i].x+arr_output[i].y*arr_output[i].y+arr_output[i].z*arr_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		arr_output[i].x*=d_scale_factor;
		arr_output[i].y*=d_scale_factor;
		arr_output[i].z*=d_scale_factor;
	}

	//compute the transformation matrix
	// 1 row
	x4x4_normalize(1,1)=d_scale_factor;
	x4x4_normalize(1,2)=0;
	x4x4_normalize(1,3)=0;
	x4x4_normalize(1,4)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x4x4_normalize(2,1)=0;
	x4x4_normalize(2,2)=d_scale_factor;
	x4x4_normalize(2,3)=0;
	x4x4_normalize(2,4)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x4x4_normalize(3,1)=0;
	x4x4_normalize(3,2)=0;
	x4x4_normalize(3,3)=d_scale_factor;
	x4x4_normalize(3,4)=-d_scale_factor*cord_centroid.z;
	// 4 row
	x4x4_normalize(4,1)=0;
	x4x4_normalize(4,2)=0;
	x4x4_normalize(4,3)=0;
	x4x4_normalize(4,4)=1;

	return true;
}

bool q_RANSAC_affinmatrix_from_initialmatch(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,
											const vector<int> &arr1d_A2Bindex,
											const int n_sampling,const int n_pairs_per_sampling,
											Matrix &x4x4_affinematrix)
{
	//check parameters
	if(arr_A.size()<=0 || arr_B.size()<=0 || arr1d_A2Bindex.size()<=0 ||
	   n_sampling<=0 || n_pairs_per_sampling<4)
	{
		fprintf(stderr,"ERROR: q_compute_affinmatrix_from_initialmatch: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	vector<int> arr1d_A2Bindex_validindex;
	for(int i=0;i<arr_A.size();i++)
		if(arr1d_A2Bindex[i]!=-1)
			arr1d_A2Bindex_validindex.push_back(i);
	if(arr1d_A2Bindex_validindex.size()<n_pairs_per_sampling)
	{
		fprintf(stderr,"ERROR: q_compute_affinmatrix_from_initialmatch: No enough valid initial matched pairs! \n");
		return false;
	}

	double err_invp_min=1e+38;
	Matrix x4x4_affinematrix_temp(4,4);
	srand((unsigned)time(NULL));
	for(int iter=0;iter<n_sampling;iter++)
	{
		if(iter%50==0) printf("\n\t");
		if(iter%5==0) printf("%4d,",iter);
		vector<int> arr_random_index(n_pairs_per_sampling,0);
		vector<Coord3D_PCR> arr_sample_A,arr_sample_B;
		bool b_valid_sample=true;
		do {
			for(int i=0;i<n_pairs_per_sampling;i++)
			{
				arr_random_index[i]=rand()%arr_A.size();
				for(int k=0;k<i;k++)
				{
					if(arr_random_index[k]==arr_random_index[i])
					{
						arr_random_index[i]=rand()%arr_A.size();
						k=-1;
					}
				}
			}
			//make sure no same number
			for(int i=0;i<n_pairs_per_sampling;i++)
			{
				if(arr1d_A2Bindex[arr_random_index[i]]==-1)
				{
					b_valid_sample=false;
					break;
				}
				b_valid_sample=true;
			}
		} while (!b_valid_sample);
		//get sample array
		for(int i=0;i<arr_random_index.size();i++)
		{
			arr_sample_A.push_back(arr_A[arr_random_index[i]]);
			arr_sample_B.push_back(arr_B[arr1d_A2Bindex[arr_random_index[i]]]);
		}

		// 2. estimate the affine matrix: A=T*B
		if(!q_compute_affinmatrix3D(arr_sample_B,arr_sample_A,x4x4_affinematrix_temp))
		{
			fprintf(stderr,"ERROR: q_compute_affinmatrix_from_initialmatch: q_compute_affinmatrix3D return false! \n");
			return false;
		}
		else if(x4x4_affinematrix_temp.is_zero())	//degenerate case
		{
			continue;
		}

		// 3. compute the inverse projection error
		double err_invp;
		vector< vector<double> > arr_dismatrix_temp(arr_A.size(),vector<double>(arr_B.size(),0.0));
		q_compute_invp_err(arr_A,arr_B,x4x4_affinematrix_temp,arr_dismatrix_temp,err_invp);

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

bool q_compute_affinmatrix3D(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,Matrix &x4x4_affinematrix)
{
	//check parameters
	if(arr_A.size()<4 || arr_A.size()!=arr_B.size())
	{
		fprintf(stderr,"ERROR: q_compute_affinmatrix3D: Invalid input parameters! \n");
		return false;
	}

	x4x4_affinematrix.ReSize(4,4);

	//normalize point set
	vector<Coord3D_PCR> arr_A_norm,arr_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	arr_A_norm=arr_A;	arr_B_norm=arr_B;
	q_normalize_points(arr_A,arr_A_norm,x4x4_normalize_A);
	q_normalize_points(arr_B,arr_B_norm,x4x4_normalize_B);

	int n_point=arr_A.size();
	Matrix A(3*n_point,13);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=arr_A_norm[i].x;	A(row,2)=arr_A_norm[i].y;	A(row,3)=arr_A_norm[i].z;	A(row,4)=1.0;
		A(row,5)=0.0;				A(row,6)=0.0;				A(row,7)=0.0;				A(row,8)=0.0;
		A(row,9)=0.0;				A(row,10)=0.0;				A(row,11)=0.0;				A(row,12)=0.0;
		A(row,13)=-arr_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;				A(row+1,4)=0.0;
		A(row+1,5)=arr_A_norm[i].x;	A(row+1,6)=arr_A_norm[i].y;	A(row+1,7)=arr_A_norm[i].z;	A(row+1,8)=1.0;
		A(row+1,9)=0.0;				A(row+1,10)=0.0;			A(row+1,11)=0.0;			A(row+1,12)=0.0;
		A(row+1,13)=-arr_B_norm[i].y;

		A(row+2,1)=0.0;				A(row+2,2)=0.0;				A(row+2,3)=0.0;				A(row+2,4)=0.0;
		A(row+2,5)=0.0;				A(row+2,6)=0.0;				A(row+2,7)=0.0;				A(row+2,8)=0.0;
		A(row+2,9)=arr_A_norm[i].x;	A(row+2,10)=arr_A_norm[i].y;A(row+2,11)=arr_A_norm[i].z;A(row+2,12)=1.0;
		A(row+2,13)=-arr_B_norm[i].z;

		row+=3;
	}

	DiagonalMatrix D;
	Matrix U,V;
	SVD(A,D,U,V);	

	Matrix h=V.column(13);	
	if(D(12,12)==0)			
	{
		x4x4_affinematrix=0.0;	
		printf("q_compute_affinmatrix3D: Degenerate singular values in SVD! \n");
	}

	for(int i=1;i<=13;i++)
	{
		h(i,1) /= h(13,1);
	}

	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;

	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}

bool q_compute_invp_err(const vector<Coord3D_PCR> &arr_A,const vector<Coord3D_PCR> &arr_B,const Matrix &x4x4_affinematrix,
						vector< vector<double> > &arr_dismatrix,double &d_invp_err)
{
	//check parameters
	if(arr_A.size()<=0 || arr_B.size()<=0 ||
	   arr_dismatrix.size()!=arr_A.size() || arr_dismatrix[0].size()!=arr_B.size())
	{
		fprintf(stderr,"ERROR: q_compute_invp_err: Invalid input parameters! \n");
		return false;
	}

	//inverse project B --> B_invp
	Matrix x_B_invp(4,arr_B.size()), x_B_homo(4,arr_B.size());
	for(int i=0;i<arr_B.size();i++)
	{
		x_B_homo(1,i+1)=arr_B[i].x;
		x_B_homo(2,i+1)=arr_B[i].y;
		x_B_homo(3,i+1)=arr_B[i].z;
		x_B_homo(4,i+1)=1.0;
	}
	x_B_invp=x4x4_affinematrix*x_B_homo;

	d_invp_err=0;
	double dis,mindis;
	double dx,dy,dz;
	for(int i=0;i<arr_A.size();i++)
	{
		mindis=1e+38;
		for(int j=0;j<arr_B.size();j++)
		{
			dx=arr_A[i].x-x_B_invp(1,j+1);
			dy=arr_A[i].y-x_B_invp(2,j+1);
			dz=arr_A[i].z-x_B_invp(3,j+1);
			dis=sqrt(dx*dx+dy*dy+dz*dz);
			arr_dismatrix[i][j]=dis;

			if(dis<mindis)
				mindis=dis;
		}
		d_invp_err+=mindis;
	}

	return true;
}

//output the matched pair to swc format
bool q_export_matches2swc(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<int> &arr1d_1to2index,const char *filename)
{
	//check parameters
	if(arr_1.size()<=0 || arr_2.size()<=0)
	{
		fprintf(stderr,"ERROR: q_matches_2_swc: Invalid array in null! \n");
		return false;
	}
	else if(arr1d_1to2index.size()!=arr_1.size())
	{
		fprintf(stderr,"ERROR: q_matches_2_swc: arr1d_1to2index should have same size as arr_1! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: q_matches_2_swc: filename is null! \n");
		return false;
	}

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: q_matches_2_swc: Failed to open file to write! \n");
		return false;
	}

	fprintf(fp,"# point cloud registration result.\n");
	int n=0;
	double radius=0.5;
	for(int i=0;i<arr1d_1to2index.size();i++)
	{
		n++;
		fprintf(fp,"%d 1 %.2f %.2f %.2f %f -1\n",n,arr_1[i].x,arr_1[i].y,arr_1[i].z,radius);
		if(arr1d_1to2index[i]!=-1)
		{
			n++;
			fprintf(fp,"%d 4 %.2f %.2f %.2f %.2f %d\n",n,arr_2[arr1d_1to2index[i]].x,arr_2[arr1d_1to2index[i]].y,arr_2[arr1d_1to2index[i]].z,radius,n-1);
		}
	}
	printf("\toutput registration result to %s complete!\n",filename);

	fclose(fp);

	return true;

}
//output the matched pair to apo format (only points corrdinate, no matching information)
bool q_export_matches2apo(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option)
{
	//check parameters
	if(arr_1.size()<=0 || arr_2.size()<=0)
	{
		fprintf(stderr,"ERROR: q_export_matches2apo: Invalid array in null! \n");
		return false;
	}
	else if(arr_2_invp.size()!=arr_2.size())
	{
		fprintf(stderr,"ERROR: q_export_matches2apo: arr_2_invp should have same size as arr_2! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: q_export_matches2apo: filename is null! \n");
		return false;
	}

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: q_export_matches2apo: Failed to open file to write! \n");
		return false;
	}

	int n=0;
	double volsize=30;	//size of ball
	if(output_option==10 || output_option==12 || output_option==13 || output_option==123)
	{
		for(int i=0;i<arr_1.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,255,0,0\n",n,n,arr_1[i].z,arr_1[i].x,arr_1[i].y,volsize);
			n++;
		}
	}
	if(output_option==20 || output_option==12 || output_option==23 || output_option==123)
	{
		for(int i=0;i<arr_2.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,0,255\n",n,n,arr_2[i].z,arr_2[i].x,arr_2[i].y,volsize);
			n++;
		}
	}
	if(output_option==30 || output_option==13 || output_option==23 || output_option==123)
	{
		for(int i=0;i<arr_2.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,255,0\n",n,n,arr_2_invp[i].z,arr_2_invp[i].x,arr_2_invp[i].y,volsize);
			n++;
		}
	}
	printf("\toutput registration result to %s complete!\n",filename);

	fclose(fp);

	return true;
}

//output the matched pair to marker format (only points corrdinate, no matching information)
//	if length of arr_1 and arr_2 <100
//	arr_1 will be write to marker file between line 100~100+arr_1.size()
//	arr_2 will be write to marker file between line 200~200+arr_2.size()
//	write in this way make arr_1 and arr_2 have prefix 1 and 2 seperatelly when show in v3d
bool q_export_matches2marker(const vector<Coord3D_PCR> &arr_1,const vector<Coord3D_PCR> &arr_2,const vector<Coord3D_PCR> &arr_2_invp,const char *filename,const int output_option)
{
	//check parameters
	if(arr_1.size()<=0 || arr_2.size()<=0)
	{
		fprintf(stderr,"ERROR: q_export_matches2marker: Invalid array in null! \n");
		return false;
	}
	else if(arr_2_invp.size()!=arr_2.size())
	{
		fprintf(stderr,"ERROR: q_export_matches2marker: arr_2_invp should have same size as arr_2! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: q_export_matches2marker: filename is null! \n");
		return false;
	}

	//	1. how to assign different color to the two point sets?
	int prefix=100;
	int n_maxsize=arr_1.size()>=arr_2.size() ? arr_1.size() : arr_2.size();
	if(n_maxsize<10)		prefix=10;
	else if(n_maxsize<100)	prefix=100;
	else if(n_maxsize<1000)	prefix=1000;
	else if(n_maxsize<10000)prefix=10000;

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: q_export_matches2marker: Failed to open file to write! \n");
		return false;
	}

	fprintf(fp,"#x,y,z,radius,shape,name,comment\n");
	int n=0;
	for(int i=0;i<prefix;i++)
		fprintf(fp,",,,,,,\n");
	if(output_option==10 || output_option==12 || output_option==13 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i< arr_1.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",arr_1[i].x+1,arr_1[i].y+1,arr_1[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	if(output_option==20 || output_option==12 || output_option==23 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i<arr_2.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",arr_2[i].x+1,arr_2[i].y+1,arr_2[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	if(output_option==30 || output_option==13 || output_option==23 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i<arr_2.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",arr_2_invp[i].x+1,arr_2_invp[i].y+1,arr_2_invp[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	fclose(fp);
	printf("\toutput registration result to %s complete!\n",filename);

	return true;
}



















