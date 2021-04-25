// q_warp_affine.cpp
// warp pointset and image based on given matched pairs
// by Lei Qu
// 2019-09-26

#include <QtGui>

#include "q_warp_affine.h"
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

//affine points warp
bool q_ptswarp_affine(const vector<Coord3D_PCM>  &vec_ctlpt_sub, const Matrix x4x4_affinematrix,
	vector<Coord3D_PCM> &vec_ctlpt_subtar_affine)
{
	//check parameters
	if (vec_ctlpt_sub.size() == 0)
	{
		printf("ERROR: subject control points is null!\n");
		return false;
	}
	if (vec_ctlpt_subtar_affine.size() != 0) vec_ctlpt_subtar_affine.clear();

	//------------------------------------------------------------------------------------------------------------------------------------
	//affine points warping
	Matrix x_pt_sub2tar_homo(4, 1), x_pt_sub_homo(4, 1);
	for (unsigned int i = 0; i < vec_ctlpt_sub.size(); i++)
	{
		//compute the inverse affine projected coordinate in subject image
		x_pt_sub2tar_homo(1, 1) = vec_ctlpt_sub[i].x;
		x_pt_sub2tar_homo(2, 1) = vec_ctlpt_sub[i].y;
		x_pt_sub2tar_homo(3, 1) = vec_ctlpt_sub[i].z;
		x_pt_sub2tar_homo(4, 1) = 1.0;
		x_pt_sub_homo = x4x4_affinematrix*x_pt_sub2tar_homo;

		Coord3D_PCM tmp;
		tmp.x = x_pt_sub_homo(1, 1);
		tmp.y = x_pt_sub_homo(2, 1);
		tmp.z = x_pt_sub_homo(3, 1);
		vec_ctlpt_subtar_affine.push_back(tmp);
	}

	return true;
}

bool q_ptswarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	vector<Coord3D_PCM> &vec_ctlpt_subtar_affine)
{
	//check parameters
	if (vec_ctlpt_tar.size() == 0 || vec_ctlpt_sub.size() == 0 || vec_ctlpt_tar.size() != vec_ctlpt_sub.size())
	{
		printf("ERROR: target or subject control points is invalid!\n");
		return false;
	}
	if (vec_ctlpt_subtar_affine.size() != 0) vec_ctlpt_subtar_affine.clear();

	//------------------------------------------------------------------------------------------------------------------------------------
	//estimate the affine matrix
	Matrix x4x4_affinematrix;
	if (!q_affine_compute_affinmatrix_3D(vec_ctlpt_sub, vec_ctlpt_tar, x4x4_affinematrix))	//B=T*A
	{
		printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
		return false;
	}
	//print affine matrix
	for (long long row = 1; row <= x4x4_affinematrix.nrows(); row++)
	{
		printf("\t");
		for (V3DLONG col = 1; col <= x4x4_affinematrix.ncols(); col++)
			printf("%.3f\t", x4x4_affinematrix(row, col));
		printf("\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//affine points warping
	if (!q_ptswarp_affine(vec_ctlpt_sub, x4x4_affinematrix,	vec_ctlpt_subtar_affine))
	{
		printf("ERROR: q_ptswarp_affine() return false.\n");
		return false;
	}

	return true;
}

//affine image warp
bool q_imagewarp_affine(const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	const Matrix x4x4_affinematrix,
	unsigned char *&p_img_affine)
{
	//check parameters
	if (p_img_sub == 0 || sz_img_sub == 0)
	{
		printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
		return false;
	}
	if (p_img_affine)
	{
		printf("WARNNING: output image pointer is not null, original memory it point to will lost!\n");
		p_img_affine = 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//assign output/warp image size
	long long sz_img_output[4] = { 0 };
	if (sz_img_output[0] == 0)		sz_img_output[0] = sz_img_affine[0];
	if (sz_img_output[1] == 0)		sz_img_output[1] = sz_img_affine[1];
	if (sz_img_output[2] == 0)		sz_img_output[2] = sz_img_affine[2];
	sz_img_output[3] = sz_img_sub[3];

	//allocate memory
	p_img_affine = new unsigned char[sz_img_output[0] * sz_img_output[1] * sz_img_output[2] * sz_img_output[3]]();
	if (!p_img_affine)
	{
		printf("ERROR: Fail to allocate memory for p_img_sub2tar.\n");
		return false;
	}
	unsigned char ****p_img_sub_4d = 0, ****p_img_sub2tar_4d = 0;
	if (!new4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3], p_img_sub) ||
		!new4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3], p_img_affine))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img_affine) 		{ delete[]p_img_affine;		p_img_affine = 0; }
		if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
		if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//affine image warping
#pragma omp parallel for
	for (long long x = 0; x < sz_img_output[0]; x++)
	{
		printf("\taffine image warp: [%d/%d]\n", sz_img_output[0], x);
		for (long long y = 0; y < sz_img_output[1]; y++)
			for (long long z = 0; z < sz_img_output[2]; z++)
			{
				Matrix x_pt_sub2tar_homo(4, 1), x_pt_sub_homo(4, 1);
				//compute the inverse affine projected coordinate in subject image
				x_pt_sub2tar_homo(1, 1) = x;
				x_pt_sub2tar_homo(2, 1) = y;
				x_pt_sub2tar_homo(3, 1) = z;
				x_pt_sub2tar_homo(4, 1) = 1.0;
				x_pt_sub_homo = x4x4_affinematrix*x_pt_sub2tar_homo;

				//------------------------------------------------------------------
				//linear interpolate
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0] = x_pt_sub_homo(1, 1);
				cur_pos[1] = x_pt_sub_homo(2, 1);
				cur_pos[2] = x_pt_sub_homo(3, 1);

				//if interpolate pixel is out of subject image region, set to -inf
				if (cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0] - 1 ||
					cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1] - 1 ||
					cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2] - 1)
				{
					p_img_sub2tar_4d[0][z][y][x] = 0.0;
					continue;
				}

				//find 8 neighbor pixels boundary
				long long x_s, x_b, y_s, y_b, z_s, z_b;
				x_s = floor(cur_pos[0]);		x_b = ceil(cur_pos[0]);
				y_s = floor(cur_pos[1]);		y_b = ceil(cur_pos[1]);
				z_s = floor(cur_pos[2]);		z_b = ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w, r_w, t_w, b_w;
				l_w = 1.0 - (cur_pos[0] - x_s);	r_w = 1.0 - l_w;
				t_w = 1.0 - (cur_pos[1] - y_s);	b_w = 1.0 - t_w;
				//compute weight for higher slice and lower slice
				double u_w, d_w;
				u_w = 1.0 - (cur_pos[2] - z_s);	d_w = 1.0 - u_w;

				//linear interpolate each channel
				for (long long c = 0; c < sz_img_output[3]; c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice = t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s] + r_w*p_img_sub_4d[c][z_s][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s] + r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice = t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s] + r_w*p_img_sub_4d[c][z_b][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s] + r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					p_img_sub2tar_4d[c][z][y][x] = u_w*higher_slice + d_w*lower_slice;
				}
			}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. free memory. \n");
	if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
	if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }

	return true;
}
bool q_imagewarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar,const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	Matrix &x4x4_affinematrix, unsigned char *&p_img_affine)
{
	//check parameters
	if (vec_ctlpt_tar.size() == 0 || vec_ctlpt_sub.size() == 0 || vec_ctlpt_tar.size() != vec_ctlpt_sub.size())
	{
		printf("ERROR: target or subject control points is invalid!\n");
		return false;
	}
	if (p_img_sub == 0 || sz_img_sub == 0)
	{
		printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
		return false;
	}
	if (p_img_affine)
	{
		printf("WARNNING: output image pointer is not null, original memory it point to will lost!\n");
		p_img_affine = 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//estimate the affine matrix
	//Matrix x4x4_affinematrix;
	if (!q_affine_compute_affinmatrix_3D(vec_ctlpt_tar, vec_ctlpt_sub, x4x4_affinematrix))	//B=T*A
	{
		printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
		return false;
	}
	//print affine matrix
	for (long long row = 1; row <= x4x4_affinematrix.nrows(); row++)
	{
		printf("\t");
		for (long long col = 1; col <= x4x4_affinematrix.ncols(); col++)
			printf("%.3f\t", x4x4_affinematrix(row, col));
		printf("\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//affine image warping
	if (!q_imagewarp_affine(p_img_sub, sz_img_sub, sz_img_affine,
		x4x4_affinematrix,
		p_img_affine))
	{
		printf("ERROR: q_imagewarp_affine() return false.\n");
		return false;
	}

	return true;
}
bool q_imagewarp_affine(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_affine,
	unsigned char *&p_img_affine)
{
	Matrix x4x4_affinematrix;
	if (!q_imagewarp_affine(vec_ctlpt_tar, vec_ctlpt_sub,
		p_img_sub, sz_img_sub, sz_img_affine,
		x4x4_affinematrix, p_img_affine))
	{
		printf("ERROR: q_imagewarp_affine() return false.\n");
		return false;
	}
	return true;
}

//centrilize and scale the point set
//	xn = T*x;
//	x: every column represent a point [2/3*N]
bool q_normalize_points_2D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x3x3_normalize)
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
	if(x3x3_normalize.nrows()!=3 || x3x3_normalize.ncols()!=3)
	{
		x3x3_normalize.ReSize(3,3);
	}

	//compute the centriod of input point set
	Coord3D_PCM cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y);
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
	}

	//compute the transformation matrix
	// 1 row
	x3x3_normalize(1,1)=d_scale_factor;
	x3x3_normalize(1,2)=0;
	x3x3_normalize(1,3)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x3x3_normalize(2,1)=0;
	x3x3_normalize(2,2)=d_scale_factor;
	x3x3_normalize(2,3)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x3x3_normalize(3,1)=0;
	x3x3_normalize(3,2)=0;
	x3x3_normalize(3,3)=1;

	return true;
}

bool q_normalize_points_3D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize)
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

//compute the affine matraix
//	B=T*A
bool q_affine_compute_affinmatrix_2D(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,Matrix &x3x3_affinematrix)
{
	//check parameters
	if(vec_A.size()<3 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x3x3_affinematrix.nrows()!=3 || x3x3_affinematrix.ncols()!=3)
	{
		x3x3_affinematrix.ReSize(3,3);
	}

	//normalize point set
	vector<Coord3D_PCM> vec_A_norm,vec_B_norm;
	Matrix x3x3_normalize_A(4,4),x3x3_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_2D(vec_A,vec_A_norm,x3x3_normalize_A);
	q_normalize_points_2D(vec_B,vec_B_norm,x3x3_normalize_B);

	//	 fill matrix A
	//
	//	  | h1, h2, h3 |    |x1| |x2|
	//	  | h4, h5, h6 | *  |y1|=|y2| <=>
	//	  | 0 ,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, 1,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, x1, y1, 1, -y2 | * |h1,h2,h3,h4,h5,h6,h7|=0
	//
	int n_point=vec_A.size();
	Matrix A(2*n_point,7);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=1.0;
		A(row,4)=0.0;				A(row,5)=0.0;				A(row,6)=0.0;
		A(row,7)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;
		A(row+1,4)=vec_A_norm[i].x;	A(row+1,5)=vec_A_norm[i].y;	A(row+1,6)=1.0;
		A(row+1,7)=-vec_B_norm[i].y;

		row+=2;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D;
	Matrix U,V;
	SVD(A,D,U,V);	//A = U * D * V.t()

	Matrix h=V.column(7);	//A*h=0
	if(D(6,6)==0)			//degenerate case
	{
		x3x3_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=7;i++)
	{
		h(i,1) /= h(7,1);
	}

	//reshape h:7*1 to 3*3 matrix
	x3x3_affinematrix(1,1)=h(1,1);	x3x3_affinematrix(1,2)=h(2,1);	x3x3_affinematrix(1,3)=h(3,1);
	x3x3_affinematrix(2,1)=h(4,1);	x3x3_affinematrix(2,2)=h(5,1);	x3x3_affinematrix(2,3)=h(6,1);
	x3x3_affinematrix(3,1)=0.0;		x3x3_affinematrix(3,2)=0.0;		x3x3_affinematrix(3,3)=1.0;

	//denormalize
	x3x3_affinematrix=x3x3_normalize_B.i()*x3x3_affinematrix*x3x3_normalize_A;

	return true;
}

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
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

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

