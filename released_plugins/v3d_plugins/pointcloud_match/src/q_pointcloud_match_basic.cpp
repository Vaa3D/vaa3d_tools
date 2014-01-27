// q_pointcloud_match_basic.cpp
// provide some basic structure and functions
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.h by Lei Qu on 2010-01-22

#include <math.h>
#include "q_pointcloud_match_basic.h"

//sort 1d array from low to high
bool q_sort1d(vector<double> &vec,vector<int> &vec_index)
{
	int n_element=vec.size();

	//check parameters
	if(n_element<1)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(vec_index.empty())
	{
		vec_index.assign(n_element,-1);
	}
	if(vec_index.size()!=n_element)
	{
		vec_index.clear();
		vec_index.assign(n_element,-1);
	}

	//initial index array
	for(int i=0;i<n_element;i++)
		vec_index[i]=i;

	//sort
	int i,j,k,ntemp;
	double ftemp;
	for(i=0;i<n_element-1;i++)
	{
		k=i;
		for(j=i+1;j<n_element;j++)
		{
			if(vec[j]<vec[k])  //find the smallest one
				k=j;
		}
		if(i!=k)
		{
			//swap array element
			ftemp=vec[i];
			vec[i]=vec[k];
			vec[k]=ftemp;
			//swap index array element
			ntemp=vec_index[i];
			vec_index[i]=vec_index[k];
			vec_index[k]=ntemp;
		}
	}

	return true;
}

//sort 2d array from low to high
//	b_sort_every_row=1: sort every row from low to high
//	b_sort_every_row=0: sort every col from low to high
bool q_sort2d(vector< vector<double> > &vecvec,vector< vector<int> > &vec_index,bool b_sort_every_row,bool b_low2high)
{
	int nrow,ncol;
	double temp_f,temp_n;
	nrow=vecvec.size();   ncol=vecvec[0].size();

	//check parameters
	if(nrow<=0 || ncol<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(vec_index.empty())
	{
		vec_index.assign(nrow,vector<int>(ncol,0));
	}
	if(vec_index.size()!=nrow || vec_index[0].size()!=ncol)
	{
		vec_index.clear();
		vec_index.assign(nrow,vector<int>(ncol,0));
	}

	if(b_sort_every_row==1)
	{
		for(int i=0;i<nrow;i++)
			if(!q_sort1d(vecvec[i],vec_index[i]))
			{
				fprintf(stderr,"ERROR: Call q_sort1d() return false! \n");
				return false;
			}

		//if sort from high to low, we need to flip the arr and arr_index left to right
		if(!b_low2high)
		{
			for(int i=0;i<nrow;i++)
				for(int j=0;j<ncol/2;j++)
				{
					temp_f=vecvec[i][j];				temp_n=vec_index[i][j];
					vecvec[i][j]=vecvec[i][ncol-j-1];	vec_index[i][j]=vec_index[i][ncol-j-1];
					vecvec[i][ncol-j-1]=temp_f;			vec_index[i][ncol-j-1]=temp_n;
				}
		}
	}

	if(b_sort_every_row==0)
	{
		vector< vector<double> > vecvec_trans(vecvec[0].size(),vector<double>(vecvec.size(),0));
		vector< vector<int> > vecvec_index_trans(vecvec[0].size(),vector<int>(vecvec.size(),0));
		//transform(arr) => arr_trans
		for(int i=0;i<nrow;i++)
			for(int j=0;j<ncol;j++)
				vecvec_trans[j][i]=vecvec[i][j];
		//do sort every row
		for(int i=0;i<ncol;i++)
			if(!q_sort1d(vecvec_trans[i], vecvec_index_trans[i]))
			{
				fprintf(stderr,"ERROR: Call q_sort1d() return false! \n");
				return false;
			}
		//transform(arr_trans) => arr
		//transform(arr_index_trans) => arr_index
		for(int i=0;i<nrow;i++)
			for(int j=0;j<ncol;j++)
			{
				vecvec[i][j]=vecvec_trans[j][i];
				vec_index[i][j]=vecvec_index_trans[j][i];
			}

		//if sort from high to low, we need to flip the arr and arr_index up to down
		if(!b_low2high)
		{
			for(int i=0;i<nrow;i++)
				for(int j=0;j<ncol/2;j++)
				{
					temp_f=vecvec[i][j];				temp_n=vec_index[i][j];
					vecvec[i][j]=vecvec[nrow-i-1][j];	vec_index[i][j]=vec_index[nrow-i-1][j];
					vecvec[nrow-i-1][j]=temp_f;			vec_index[nrow-i-1][j]=temp_n;
				}
		}
	}

	return true;
}

//centrilize and scale the point set
//	xn = T*x;
//	x: every column represent a point [3*N]
bool q_normalize_points(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize)
{
	//check parameters
	if(vec_input.size()<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(!vec_output.empty())
		vec_output.clear();
	vec_output=vec_input;
	if(x4x4_normalize.nrows()!=4 || x4x4_normalize.ncols()!=4)
	{
		x4x4_normalize.ReSize(4,4);
	}

	//compute the centriod of input point set
	Coord3D_PCM cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
		cord_centroid.z+=vec_input[i].z;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	cord_centroid.z/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
		vec_output[i].z-=cord_centroid.z;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y+vec_output[i].z*vec_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
		vec_output[i].z*=d_scale_factor;
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

//sort similarity matrix and voting the best several canditates with given weight
bool q_vote_simmatrix(const vector< vector<double> > &vecvec_simmatrix, const int weight, vector< vector<double> > &vecvec_votematrix)
{
	//check parameters
	int n_row,n_col;
	if((n_row=vecvec_simmatrix.size())<=0 || (n_col=vecvec_simmatrix[0].size())<=0)
	{
		fprintf(stderr,"ERROR: Invalid input arr_simmatrix size! \n");
		return false;
	}
	if(vecvec_votematrix.empty())
	{
		vecvec_votematrix.assign(n_row,vector<double>(n_col,0));
	}
	if(n_row!=vecvec_votematrix.size() || n_col!=vecvec_votematrix[0].size())
	{
		fprintf(stderr,"WARNING: Invalid input arr_votematrix size, orginal data will be deleted! \n");
		vecvec_votematrix.clear();
		vecvec_votematrix.assign(n_row,vector<double>(n_col,0));
	}

	//sort every row of arr_simmatrix in order to select the best several candidates for every A
	vector< vector<double> > vecvec_simmatrix_sort_row(vecvec_simmatrix);
	vector< vector<int> > vecvec_index_row;
	if(!q_sort2d(vecvec_simmatrix_sort_row, vecvec_index_row,1,1))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}
	//sort every col of arr_simmatrix in order to select the best several candidates for every B
	vector< vector<double> > vecvec_simmatrix_sort_col(vecvec_simmatrix);
	vector< vector<int> > vecvec_index_col;
	if(!q_sort2d(vecvec_simmatrix_sort_col, vecvec_index_col,0,1))
	{
		fprintf(stderr,"ERROR: q_sort2d() return false! \n");
		return false;
	}

	//do voting use similarity matrix
	int n_candidate=5;
	for(int i=0;i<n_row;i++)
		for(int k=0;k<n_candidate;k++)
			vecvec_votematrix[i][vecvec_index_row[i][k]]+=(n_candidate-k)*weight;
	for(int i=0;i<n_col;i++)
		for(int k=0;k<n_candidate;k++)
			vecvec_votematrix[vecvec_index_col[k][i]][i]+=(n_candidate-k)*weight;

	return true;
}


//brutally kill all multi-matched pairs (although one of they may be right!)
//	i.e. multi A matched to the same B.
//	since we cannot tell which pair in the multi-matched pairs is right, here we simply break all the mult-matched pairs
//	we do not want the possible wrong matched pairs affect the consequent RANSAC affine estimation
bool q_kill_allmultimatch(vector<int> &vec_A2Bindex,const int n_lengthB)
{
	//check parameters
	if(vec_A2Bindex.size()<=0 || n_lengthB<=0)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}

	int n_lengthA=vec_A2Bindex.size();

	//bulild the multi match index
	vector< vector<int> > arr2d_B2Aindex(n_lengthB,vector<int>(n_lengthA,-1));	//n_B*n_B
	for(int i=0;i<n_lengthA;i++)
	{
		if(vec_A2Bindex[i]==-1)
			continue;

		for(int k=0;k<n_lengthA;k++)
			if(arr2d_B2Aindex[vec_A2Bindex[i]][k]==-1)
			{
				arr2d_B2Aindex[vec_A2Bindex[i]][k]=i;
				break;
			}
	}

	//find and break the muti-matched pairs
	int n_killed=0;
	for(int i=1;i<n_lengthB;i++)
	{
		//no multi A matched to B[i]
		if(arr2d_B2Aindex[i][1]==-1)//no multi A matched to B[i]
			continue;
		//exist multi A matched to B[i]
		for(int k=0;k<n_lengthA;k++)
		{
			if(arr2d_B2Aindex[i][k]!=-1)
			{
				vec_A2Bindex[arr2d_B2Aindex[i][k]]=-1;
				n_killed++;
			}
			else
				break;
		}
	}
	printf("\t>>%d pairs got killed!\n",n_killed);

	return true;
}

//kill multi-match by only keep one pair that has the highest match cost (for affine based refine match)
bool q_kill_badmultimatch_keephigestcost(vector<int> &vec_A2Bindex,const vector<double> &vec_A2B_cost,const int n_lengthB)
{
	//check parameters
	if(vec_A2Bindex.size()<=0 || vec_A2B_cost.size()<=0 || n_lengthB<=0)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}

	int n_lengthA=vec_A2Bindex.size();

	//bulild the multi match index
	vector< vector<int> > arr2d_B2Aindex(n_lengthB,vector<int>(0,0));
	for(int i=0;i<n_lengthA;i++)
	{
		if(vec_A2Bindex[i]!=-1)
			arr2d_B2Aindex[vec_A2Bindex[i]].push_back(i);
	}
	//print mult match info
	for(int i=0;i<n_lengthB;i++)
	{
		if(arr2d_B2Aindex[i].size()>0)
		{
			printf("\t>>B[%d]-->",i);
			for(int j=0;j<arr2d_B2Aindex[i].size();j++)
				printf("A[%d],",arr2d_B2Aindex[i][j]);
			printf("\n");
		}
	}

	//find and break badmuti-matched pairs
	int n_killed=0;
	for(int i=0;i<n_lengthB;i++)
	{
		int i_maxcost_Aind=-1;
		double d_maxcost=-1;
		if(arr2d_B2Aindex[i].size()>1)
		{
			//find pair that has highest match cost
			for(int k=0;k<arr2d_B2Aindex[i].size();k++)
			{
				int A_ind=arr2d_B2Aindex[i][k];
				if(vec_A2B_cost[A_ind]>d_maxcost)
				{
					d_maxcost=vec_A2B_cost[A_ind];
					i_maxcost_Aind=A_ind;
				}
			}
			//kill the others in current multi-match pair set
			for(int k=0;k<arr2d_B2Aindex[i].size();k++)
			{
				int A_ind=arr2d_B2Aindex[i][k];
				if(i_maxcost_Aind!=-1 && A_ind!=i_maxcost_Aind)
				{
					printf("\t>>A[%d]--->B[%d] killed\n",A_ind,vec_A2Bindex[A_ind]);
					vec_A2Bindex[A_ind]=-1;
					n_killed++;
				}
			}
		}
	}
	printf("\t>>%d pairs got killed!\n",n_killed);

	return true;
}

//kill multi-match by only keep one pair that has the lowest match cost (for manifold based refine match)
bool q_killbadmultimatch_keeplowestcost(vector<int> &vec_A2Bindex,const vector<double> &vec_A2Bcost,const int n_lengthB)
{
	//check parameters
	if(vec_A2Bindex.size()<=0 || vec_A2Bcost.size()<=0 || n_lengthB<=0)
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}

	int n_lengthA=vec_A2Bindex.size();

	//find the index of B that matched to multi-A
	vector< vector<int> > vecvec_B2Aindex(n_lengthB,vector<int>(0,0));
	for(unsigned int i=0;i<n_lengthA;i++)
	{
		int i_B_ind=vec_A2Bindex[i];
		if(i_B_ind!=-1)
			vecvec_B2Aindex[i_B_ind].push_back(i);
	}

	//find and break badmuti-matched pairs
	int n_killed=0;
	for(unsigned int i=0;i<n_lengthB;i++)
	{
		//find the index of pair in the multi-matched pairs that has the smallest dirdiscost
		double d_mincost=1000;
		int i_minind=-1;

//		printf("\tB[%d]->",i);
		for(unsigned int j=0;j<vecvec_B2Aindex[i].size();j++)
		{
			int i_A_ind=vecvec_B2Aindex[i][j];
//			printf("A[%d],",i_A_ind);

			//record the index of A with smallest cost to B
			if(d_mincost>vec_A2Bcost[i_A_ind])
			{
				d_mincost=vec_A2Bcost[i_A_ind];
				i_minind=i_A_ind;
			}
		}
//		printf("\n");

		//break the multi-matched pair that do not have the smallest dir and dis difference
		for(unsigned int j=0;j<vecvec_B2Aindex[i].size();j++)
		{
			int i_A_ind=vecvec_B2Aindex[i][j];
			if(i_minind!=-1 && i_A_ind!=i_minind)
			{
				vec_A2Bindex[i_A_ind]=-1;
				n_killed++;
			}
		}
	}
	printf("\t>>%d bad multi-matched pairs got killed!\n",n_killed);

	return true;
}


//output the matched pair to swc format
bool q_export_matches2swc(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<int> &vec_1to2index,const char *filename)
{
	//check parameters
	if(vec_1.size()<=0 || vec_2.size()<=0)
	{
		fprintf(stderr,"ERROR: Invalid array in null! \n");
		return false;
	}
	else if(vec_1to2index.size()!=vec_1.size())
	{
		fprintf(stderr,"ERROR: arr1d_1to2index should have same size as arr_1! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: filename is null! \n");
		return false;
	}

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: Failed to open file to write! \n");
		return false;
	}

	fprintf(fp,"# point cloud registration result.\n");
	int n=0;
	double radius=0.5;
	for(int i=0;i<vec_1to2index.size();i++)
	{
		n++;
		fprintf(fp,"%d 1 %.2f %.2f %.2f %f -1\n",n,vec_1[i].x,vec_1[i].y,vec_1[i].z,radius);
		if(vec_1to2index[i]!=-1)
		{
			n++;
			fprintf(fp,"%d 4 %.2f %.2f %.2f %.2f %d\n",n,vec_2[vec_1to2index[i]].x,vec_2[vec_1to2index[i]].y,vec_2[vec_1to2index[i]].z,radius,n-1);
		}
	}
	printf("output registration result to %s complete!\n",filename);

	fclose(fp);

	return true;

}

//output the matched pair to apo format (only points corrdinate, no matching information)
bool q_export_matches2apo(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<Coord3D_PCM> &vec_2_invp,const char *filename,const int output_option)
{
	//check parameters
	if(vec_1.size()<=0 || vec_2.size()<=0)
	{
		fprintf(stderr,"ERROR: Invalid array in null! \n");
		return false;
	}
//	else if(vec_2_invp.size()!=vec_2.size())
//	{
//		fprintf(stderr,"ERROR: arr_2_invp should have same size as arr_2! \n");
//		return false;
//	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: filename is null! \n");
		return false;
	}

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: Failed to open file to write! \n");
		return false;
	}

	int n=0;
	double volsize=30;	//size of ball
	if(output_option==10 || output_option==12 || output_option==13 || output_option==123)
	{
		for(int i=0;i<vec_1.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,255,0,0\n",n,n,vec_1[i].z,vec_1[i].x,vec_1[i].y,volsize);
			n++;
		}
	}
	if(output_option==20 || output_option==12 || output_option==23 || output_option==123)
	{
		for(int i=0;i<vec_2.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,0,255\n",n,n,vec_2[i].z,vec_2[i].x,vec_2[i].y,volsize);
			n++;
		}
	}
	if(output_option==30 || output_option==13 || output_option==23 || output_option==123)
	{
		for(int i=0;i<vec_2.size();i++)
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,255,0\n",n,n,vec_2_invp[i].z,vec_2_invp[i].x,vec_2_invp[i].y,volsize);
			n++;
		}
	}
	printf("output registration result to %s complete!\n",filename);

	fclose(fp);

	return true;
}

//output the matched pair to marker format (only points corrdinate, no matching information)
//	if length of arr_1 and arr_2 <100
//	arr_1 will be write to marker file between line 100~100+arr_1.size()
//	arr_2 will be write to marker file between line 200~200+arr_2.size()
//	write in this way make arr_1 and arr_2 have prefix 1 and 2 seperatelly when show in v3d
bool q_export_matches2marker(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<Coord3D_PCM> &vec_2_invp,const char *filename,const int output_option)
{
	//check parameters
	if(vec_1.size()<=0 || vec_2.size()<=0)
	{
		fprintf(stderr,"ERROR: Invalid array in null! \n");
		return false;
	}
	else if(vec_2_invp.size()!=vec_2.size())
	{
		fprintf(stderr,"ERROR: arr_2_invp should have same size as arr_2! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: filename is null! \n");
		return false;
	}

	//	1. how to assign different color to the two point sets?
	int prefix=100;
	int n_maxsize=vec_1.size()>=vec_2.size() ? vec_1.size() : vec_2.size();
	if(n_maxsize<10)		prefix=10;
	else if(n_maxsize<100)	prefix=100;
	else if(n_maxsize<1000)	prefix=1000;
	else if(n_maxsize<10000)prefix=10000;

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: Failed to open file to write! \n");
		return false;
	}

	fprintf(fp,"#x,y,z,radius,shape,name,comment\n");
	//	for(i=0;i<prefix*2+arr_2.size();i++)
	//	{
	//		if(i<prefix)
	//			fprintf(fp,",,,,,,\n");
	//		else if(i>=prefix && i<prefix+arr_1.size())
	//			fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",arr_1[i-prefix].x,arr_1[i-prefix].y,arr_1[i-prefix].z);
	//		else if(i>=prefix+arr_1.size() && i<prefix*2)
	//			fprintf(fp,",,,,,,\n");
	//		else
	//			fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",arr_2[i-prefix*2].x,arr_2[i-prefix*2].y,arr_2[i-prefix*2].z);
	//	}
	int n=0;
	for(int i=0;i<prefix;i++)
		fprintf(fp,",,,,,,\n");
	if(output_option==10 || output_option==12 || output_option==13 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i< vec_1.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",vec_1[i].x+1,vec_1[i].y+1,vec_1[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	if(output_option==20 || output_option==12 || output_option==23 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i<vec_2.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",vec_2[i].x+1,vec_2[i].y+1,vec_2[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	if(output_option==30 || output_option==13 || output_option==23 || output_option==123)
	{
		for(int i=0;i<prefix;i++)
		{
			if(i>=0 && i<vec_2.size())
				fprintf(fp,"%.2f,%.2f,%.2f,,,,\n",vec_2_invp[i].x+1,vec_2_invp[i].y+1,vec_2_invp[i].z+1);
			else
				fprintf(fp,",,,,,,\n");
			n++;
		}
	}
	fclose(fp);
	printf("output registration result to %s complete!\n",filename);

	return true;
}

//output only valid markers to marker file(output_flag=1: output valid/matched marker in arr_1; output_flag=2: output valid/matched marker in arr_2)
bool q_export_validmarker(const int output_flag,const vector<Coord3D_PCM> &vec,const vector<int> &vec_1to2index,const char *filename)
{
	//check parameters
	if(output_flag!=1 && output_flag!=2)
	{
		fprintf(stderr,"ERROR: output_flag is invalid! (1: output valid/matched marker in arr_1; 2: output valid/matched marker in arr_2) \n");
		return false;
	}
	else if(vec.size()<=0)
	{
		fprintf(stderr,"ERROR: Invalid array is null! \n");
		return false;
	}
	else if(vec_1to2index.size()<=0)
	{
		fprintf(stderr,"ERROR: arr1d_1to2index is empty! \n");
		return false;
	}
	else if(filename==0)
	{
		fprintf(stderr,"ERROR: filename is null! \n");
		return false;
	}
	
	//output valid/matched tar or sub markers to file
	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: Failed to open file to write! \n");
		return false;
	}
	fprintf(fp,"#x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");
	for(int i=0;i<vec_1to2index.size();i++)
	{
		if(output_flag==1)
		{
			if(vec_1to2index[i]!=-1)
				fprintf(fp,"%f,%f,%f,,,,\n",vec[i].x,vec[i].y,vec[i].z);
		}
		else if(output_flag==2)
		{
			if(vec_1to2index[i]!=-1)
				fprintf(fp,"%f,%f,%f,,,,\n",vec[vec_1to2index[i]].x,vec[vec_1to2index[i]].y,vec[vec_1to2index[i]].z);
		}
	}
	fclose(fp);
	
	printf("output registration result to %s complete!\n",filename);

}

