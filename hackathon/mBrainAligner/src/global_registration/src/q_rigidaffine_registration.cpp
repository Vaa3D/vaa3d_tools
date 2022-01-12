// q_rigidaffine_registration.cpp
// by Lei Qu
// 2011-04-08

#include "q_rigidaffine_registration.h"

#include <iostream>
#include <math.h>
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "../../basic_c_fun/stackutil.h"
#include "../src/q_convolve.h"


//************************************************************************************************************************************
bool q_rigidaffine_registration(const CParas_reg &paras,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	//check paras
	if(paras.i_regtype!=0 && paras.i_regtype!=1)
	{
		printf("ERROR: Unknown given registration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(p_img64f_tar==0 || p_img64f_sub==0)
	{
		printf("ERROR: Invalid input image pointer (target or subject)!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t(1). Generate rectangular control points:\n");
	//get grid wnd size (save as image size)
	long sz_gridwnd[3];//w,h,z,c
	for(long i=0;i<3;i++)
		sz_gridwnd[i]=sz_img[i];
	printf("\t\t>>sz_gridwnd: [%ld,%ld,%ld]\n",sz_gridwnd[0],sz_gridwnd[1],sz_gridwnd[2]);

	//Generate initial control rectangular/grid
	if(!q_rigidaffine_inigrid_generator(sz_gridwnd,vec4D_grid))
	{
		printf("ERROR: q_rigid_inigrid_generator() return false!\n");
		return false;
	}
	//backup the initial grid
	vector< vector< vector< vector<double> > > > vec4D_grid_ori(vec4D_grid);

	//------------------------------------------------------------------------------------------------------------------------------------
	//estimate initial optimization step size = min(image_size)
	//make sure the step size is big enough, since latter we only decrease it
	printf("\t(3). Estimate initial optimization step size.\n");
	double d_gamma;
	double maxstep=1e+5;
	for(long i=0;i<3;i++)
		if(maxstep>sz_img[i])
			maxstep=sz_img[i];
	d_gamma=maxstep*paras.d_step_inimultiplyfactor;
	printf("\t\t>>d_gamma:%f\n",d_gamma);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t(4). Align the mass center of target and subject image.\n");
	//shift subject image
	double *p_img64f_sub_shift=0;
	long l_sub2tar_masscenteroffset[3]={0,0,0};//xyz

	if(!paras.b_alignmasscenter)
	{
		p_img64f_sub_shift=new double[sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]]();
		if(!p_img64f_sub_shift)
		{
			printf("ERROR: Fail to allocate memory for p_img64f_sub_shift!\n");
			return false;
		}
		//copy subject image
		long l_npixels=sz_img[0]*sz_img[1]*sz_img[2];
		for(long i=0;i<l_npixels;i++)
			p_img64f_sub_shift[i]=p_img64f_sub[i];
	}
	else
	{
		if(!q_align_masscenter(p_img64f_tar,p_img64f_sub,sz_img,
				p_img64f_sub_shift,l_sub2tar_masscenteroffset))
		{
			printf("ERROR: q_align_masscenter() return false!\n");
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}
	}
//	q_save64f01_image(p_img64f_tar,sz_img,"tar.tif");
//	q_save64f01_image(p_img64f_sub,sz_img,"sub.tif");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t(4). Enter iterative optimization. \n");
	//------------------------------------------------------------------------------------------------------------------------------------
	double d_E=1e+10;
	double d_E_tmp=0.0;
	double d_E_change=1.0;
	double *p_img64f_warp=0;
	vector< vector< vector< vector<double> > > > vec4D_grid_tmp(vec4D_grid);
	vector< vector< vector< vector<double> > > > vec4D_grid_int;				//interpolated meshgrid (transformation field)
	vector< vector< vector< vector<double> > > > vec4D_grid_dT;					//grid control pointa increment: grid=grid+gamma*grid_dT

	long iter=0;
	while(/*fabs(d_E_change)>1e-5 && */iter<paras.l_iter_max && fabs(d_gamma)>paras.d_step_min && fabs(d_E)>1e-5)
	{
		iter++;

		//rigid/affine interpolate the meshgrid into transformation field
		vec4D_grid_int.clear();
		if(!q_rigidaffine_grid2field_3D(paras.i_regtype,vec4D_grid_tmp,sz_gridwnd,sz_gridwnd,vec4D_grid_int))
		{
			printf("ERROR: q_rigid_grid2field_3D() return false!\n");
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		//warp the subject image based on transformation field
		if(p_img64f_warp)	 {delete []p_img64f_warp;	p_img64f_warp=0;}
		if(!q_rigidaffine_interpolate_3D(p_img64f_sub_shift,sz_img,vec4D_grid_int,p_img64f_warp))
		{
			printf("ERROR: q_rigid_interpolate_3D() return false!\n");
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		//compute the similarity cost of warped subject image to target image
		double d_E_similarity=0.0;
		if(!q_rigidaffine_similarity_3D(p_img64f_warp,p_img64f_tar,sz_img,d_E_similarity))
		{
			printf("ERROR: q_rigid_similarity_3D() return false!\n");
			if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		//compute the total cost
//		d_E_tmp=d_E_similarity+0.01*d_E_smooth;
		d_E_tmp=d_E_similarity;

		//judge whether current step size is right
		// if right: d_E_tmp>=d_E, update grid position, energy and grid increment dT
		// if wrong: d_E_tmp< d_E, descrease the step size
		d_E_change=d_E_tmp-d_E;
		if(d_E_change>=0)		//current step (gamma) is too big, decrease it
		{
			printf("\t\t\t>>Iter=%4ld,  E_tmp=%.5f,  E_change=%.5f,  gamma=%.5f\n",iter,d_E_tmp,d_E_change,d_gamma);
			d_gamma*=paras.d_step_annealingratio;//annealing=0.9
		}
		else					//current step (gamma) is right, do updating
		{
			vec4D_grid=vec4D_grid_tmp;	//update meshgrid
			d_E=d_E_tmp;				//update current total energy
			//d_gamma /= paras.d_step_annealingratio;//should be faster, need test!

			//compute the meshgrid nodes increment dE/dT
			//min(E) <=> T=T-gamma*(dE/dT)  <-- gradient descent
			vec4D_grid_dT.clear();
			if(!q_rigidaffine_gridnodes_dT_3D(paras.i_regtype,p_img64f_tar,p_img64f_warp,sz_img,sz_gridwnd,vec4D_grid_dT))
			{
				printf("ERROR: q_rigid_gridnodes_dT_3D() return false!\n");
				if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
				if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
				return false;
			}
			printf("\t\t\t>>Iter=%4ld,  E    =%.5f,  E_change=%.5f,  gamma=%.5f --->update!\n",iter,d_E,d_E_change,d_gamma);
//			q_save64f01_image(p_img64f_warp,sz_img,"sub2tar_tmp.tif");
		}

		//update grid control points position: T_tmp=T-gamma*dT
		vec4D_grid_tmp.clear();
		if(!q_rigidaffine_gridnodes_update_3D(vec4D_grid,vec4D_grid_dT,d_gamma,vec4D_grid_tmp))
		{
			printf("ERROR: q_rigid_gridnodes_update_3D() return false!\n");
			if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

	}

	if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
	if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t(5). prepare output grid (scale the grid). \n");
	for(long x=0;x<2;x++)
		for(long y=0;y<2;y++)
			for(long z=0;z<2;z++)
			{
				vec4D_grid[y][x][z][0]=vec4D_grid[y][x][z][0]+l_sub2tar_masscenteroffset[0];
				vec4D_grid[y][x][z][1]=vec4D_grid[y][x][z][1]+l_sub2tar_masscenteroffset[1];
				vec4D_grid[y][x][z][2]=vec4D_grid[y][x][z][2]+l_sub2tar_masscenteroffset[2];
			}

	return true;
}


//************************************************************************************************************************************
//initial rectangular grid generator (used to model rigid distortion)
bool q_rigidaffine_inigrid_generator(const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	//check paras
	if(sz_gridwnd[0]<1 || sz_gridwnd[1]<1 || sz_gridwnd[2]<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	//initial meshgrid size
	long sz_grid[3]={2,2,2};//w,h,z

	//fill gridnode coordinate
	vec4D_grid.assign(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));
	for(long x=0;x<sz_grid[0];x++)
		for(long y=0;y<sz_grid[1];y++)
			for(long z=0;z<sz_grid[2];z++)
			{
				vec4D_grid[y][x][z][0]=x*(sz_gridwnd[0]-1);//x coord
				vec4D_grid[y][x][z][1]=y*(sz_gridwnd[1]-1);//y coord
				vec4D_grid[y][x][z][2]=z*(sz_gridwnd[2]-1);//z coord
			}

	return true;
}

//compute the rigid (actually is similar) transformation matrix
//	B=T*A
bool q_rigidaffine_compute_rigidmatrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_rigidmatrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_rigidmatrix.nrows()!=4 || x4x4_rigidmatrix.ncols()!=4)
	{
		x4x4_rigidmatrix.ReSize(4,4);
	}

	int n_point=vec_A.size();

	//normalize point set
	vector<Point3D64f> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//format
	Matrix x3xn_A(3,n_point),x3xn_B(3,n_point);
	for(long i=0;i<n_point;i++)
	{
		x3xn_A(1,i+1)=vec_A_norm[i].x;	x3xn_A(2,i+1)=vec_A_norm[i].y;	x3xn_A(3,i+1)=vec_A_norm[i].z;
		x3xn_B(1,i+1)=vec_B_norm[i].x;	x3xn_B(2,i+1)=vec_B_norm[i].y;	x3xn_B(3,i+1)=vec_B_norm[i].z;
	}

	//compute rotation matrix R
	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(x3xn_A*x3xn_B.t(),D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix R=V*U.t();

	//reshape R to rigid transform matrix: T(1:3,1:3)=R
	x4x4_rigidmatrix(1,1)=R(1,1);	x4x4_rigidmatrix(1,2)=R(1,2);	x4x4_rigidmatrix(1,3)=R(1,3);	x4x4_rigidmatrix(1,4)=0.0;
	x4x4_rigidmatrix(2,1)=R(2,1);	x4x4_rigidmatrix(2,2)=R(2,2);	x4x4_rigidmatrix(2,3)=R(2,3);	x4x4_rigidmatrix(2,4)=0.0;
	x4x4_rigidmatrix(3,1)=R(3,1);	x4x4_rigidmatrix(3,2)=R(3,2);	x4x4_rigidmatrix(3,3)=R(3,3);	x4x4_rigidmatrix(3,4)=0.0;
	x4x4_rigidmatrix(4,1)=0.0;		x4x4_rigidmatrix(4,2)=0.0;		x4x4_rigidmatrix(4,3)=0.0;		x4x4_rigidmatrix(4,4)=1.0;

	//denormalize
	x4x4_rigidmatrix=x4x4_normalize_B.i()*x4x4_rigidmatrix*x4x4_normalize_A;

	return true;
}

//compute the affine transformation matrix
//	B=T*A
bool q_rigidaffine_compute_affinematrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_affinematrix)
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
	vector<Point3D64f> vec_A_norm,vec_B_norm;
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
	DiagonalMatrix D(13);
	Matrix U(3*n_point,13),V(13,13);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}

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


//rigid interpolate the 3D grid to transformation field
bool q_rigidaffine_grid2field_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd_input[3],const long sz_gridwnd_output[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_int)
{
	//check paras
	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(vec4D_grid.size()!=2 || vec4D_grid[0].size()!=2 || vec4D_grid[0][0].size()!=2 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(sz_gridwnd_input[0]<1 || sz_gridwnd_input[1]<1 || sz_gridwnd_input[2]<1 ||
	   sz_gridwnd_output[0]<1 || sz_gridwnd_output[1]<1 || sz_gridwnd_output[2]<1)
	{
		printf("ERROR: Invalid sz_gridwnd_input or sz_gridwnd_output, it should >=1!\n");
		return false;
	}
	if(vec4D_grid_int.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid_int is not empty, original data will be cleared!\n");
		vec4D_grid_int.clear();
	}

	//compute the gridwnd scale factor
	double d_scalefactor[3];
	for(long i=0;i<3;i++)
		d_scalefactor[i]=double(sz_gridwnd_output[i])/double(sz_gridwnd_input[i]);

	//fill subject grid and target grid
	vector<Point3D64f> vec_ori(8,Point3D64f()),vec_cur(8,Point3D64f());
	for(long x=0;x<2;x++)
		for(long y=0;y<2;y++)
			for(long z=0;z<2;z++)
			{
				long ind=4*z+2*y+x;
				vec_ori[ind].x=(x*(sz_gridwnd_input[0]-1)+1)*d_scalefactor[0]-1;
				vec_ori[ind].y=(y*(sz_gridwnd_input[1]-1)+1)*d_scalefactor[1]-1;
				vec_ori[ind].z=(z*(sz_gridwnd_input[2]-1)+1)*d_scalefactor[2]-1;

				vec_cur[ind].x=(vec4D_grid[y][x][z][0]+1)*d_scalefactor[0]-1;
				vec_cur[ind].y=(vec4D_grid[y][x][z][1]+1)*d_scalefactor[1]-1;
				vec_cur[ind].z=(vec4D_grid[y][x][z][2]+1)*d_scalefactor[2]-1;
			}

	//compute rigid transformation matrix: X_cur=T*X_ori
	Matrix x4x4_transmatrix(4,4);
	if(i_regtype==0)
		q_rigidaffine_compute_rigidmatrix_3D(vec_ori,vec_cur,x4x4_transmatrix);//	B=T*A
	else if(i_regtype==1)
		q_rigidaffine_compute_affinematrix_3D(vec_ori,vec_cur,x4x4_transmatrix);//	B=T*A
	else
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}

	//rigid transform the regular field to current dense field
	vec4D_grid_int.assign(sz_gridwnd_output[1],vector< vector< vector<double> > >(sz_gridwnd_output[0],vector< vector<double> >(sz_gridwnd_output[2],vector<double>(3,0))));
	Matrix x_ori(4,1),x_trans(4,1);
	for(long x=0;x<sz_gridwnd_output[0];x++)
		for(long y=0;y<sz_gridwnd_output[1];y++)
			for(long z=0;z<sz_gridwnd_output[2];z++)
			{
				x_ori(1,1)=x;	x_ori(2,1)=y;	x_ori(3,1)=z;	x_ori(4,1)=1.0;
				x_trans=x4x4_transmatrix*x_ori;

				vec4D_grid_int[y][x][z][0]=x_trans(1,1)/x_trans(4,1);
				vec4D_grid_int[y][x][z][1]=x_trans(2,1)/x_trans(4,1);
				vec4D_grid_int[y][x][z][2]=x_trans(3,1)/x_trans(4,1);
			}

	return true;
}

//inverse rigid interpolate the 3D transformation field to grid (least square fitting)
bool q_rigidaffine_field2grid_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_field,
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	//check paras
	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(vec4D_field.size()==0 || vec4D_field[0].size()==0 || vec4D_field[0][0].size()==0 || vec4D_field[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input field size!\n");
		return false;
	}
	if(sz_gridwnd[0]<1 || sz_gridwnd[1]<1 || sz_gridwnd[2]<1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >=1!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	long sz_grid[3]={2,2,2};//w,h,z
	long sz_grid_int[3];//w,h,z
	sz_grid_int[1]=vec4D_field.size();
	sz_grid_int[0]=vec4D_field[0].size();
	sz_grid_int[2]=vec4D_field[0][0].size();

	//fill subject and target corresponding points position vector (use them to estimate the affine paras by using least square fitting)
	long pgsz_y=sz_grid_int[0];
	long pgsz_xy=sz_grid_int[0]*sz_grid_int[1];
	long pgsz_xyz=sz_grid_int[0]*sz_grid_int[1]*sz_grid_int[2];
	vector<Point3D64f> vec_ori(pgsz_xyz,Point3D64f()),vec_cur(pgsz_xyz,Point3D64f());
	for(long x=0;x<sz_grid_int[0];x++)
		for(long y=0;y<sz_grid_int[1];y++)
			for(long z=0;z<sz_grid_int[2];z++)
			{
				long ind=z*pgsz_xy+y*pgsz_y+x;
				vec_ori[ind].x=x;
				vec_ori[ind].y=y;
				vec_ori[ind].z=z;

				vec_cur[ind].x=x+vec4D_field[y][x][z][0];
				vec_cur[ind].y=y+vec4D_field[y][x][z][1];
				vec_cur[ind].z=z+vec4D_field[y][x][z][2];
			}

	//estimate the best fitting rigid transformation matrix: X_cur=T*X_ori
	Matrix x4x4_transmatrix(4,4);
	if(i_regtype==0)
		q_rigidaffine_compute_rigidmatrix_3D(vec_ori,vec_cur,x4x4_transmatrix);//	B=T*A
	else if(i_regtype==1)
		q_rigidaffine_compute_affinematrix_3D(vec_ori,vec_cur,x4x4_transmatrix);//	B=T*A
	else
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}

	//apply the rigid transform on the inital grid control points
	vec4D_grid.assign(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));
	Matrix x_ori(4,1),x_aff(4,1);
	for(long x=0;x<sz_grid[0];x++)
		for(long y=0;y<sz_grid[1];y++)
			for(long z=0;z<sz_grid[2];z++)
			{
				x_ori(1,1)=x*(sz_gridwnd[0]-1);
				x_ori(2,1)=y*(sz_gridwnd[1]-1);
				x_ori(3,1)=z*(sz_gridwnd[2]-1);
				x_ori(4,1)=1.0;
				x_aff=x4x4_transmatrix*x_ori;

				vec4D_grid[y][x][z][0]=x_aff(1,1)/x_aff(4,1)-x_ori(1,1);
				vec4D_grid[y][x][z][1]=x_aff(2,1)/x_aff(4,1)-x_ori(2,1);
				vec4D_grid[y][x][z][2]=x_aff(3,1)/x_aff(4,1)-x_ori(3,1);
			}

	return true;
}

//given input image and transformation field, linear interpolate the warpped image (table lookup)
bool q_rigidaffine_interpolate_3D(const unsigned char *p_img8u_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		unsigned char *&p_img8u_output)
{
	//check paras
	if(p_img8u_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec4D_grid_int.size()!=sz_img_input[1] || vec4D_grid_int[0].size()!=sz_img_input[0] ||
	   vec4D_grid_int[0][0].size()!=sz_img_input[2] || vec4D_grid_int[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size, it should be same as input image size!\n");
		return false;
	}
	if(p_img8u_output)
	{
		printf("WARNNING: Output image pointer is not empty, original data will be deleted!\n");
		if(p_img8u_output) 	{delete []p_img8u_output;		p_img8u_output=0;}
	}

	//allocate memory
	p_img8u_output=new unsigned char[sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3]]();
	if(!p_img8u_output)
	{
		printf("ERROR: Fail to allocate memory for warpped image!\n");
		return false;
	}

	//linear interpolate the output image according to the given transformation field
	long pgsz_y=sz_img_input[0];
	long pgsz_xy=sz_img_input[0]*sz_img_input[1];
	long pgsz_xyz=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
#pragma omp parallel for
	for(long x=0;x<sz_img_input[0];x++)
		for(long y=0;y<sz_img_input[1];y++)
			for(long z=0;z<sz_img_input[2];z++)
			{
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0]=vec4D_grid_int[y][x][z][0];
				cur_pos[1]=vec4D_grid_int[y][x][z][1];
				cur_pos[2]=vec4D_grid_int[y][x][z][2];

				//compensate the minor float type error (+-0) around 0
				if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
				if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
				if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
				//if interpolate pixel is out of subject image region, set to -inf
				if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 ||
				   cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 ||
				   cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
				{
					for(long c=0;c<sz_img_input[3];c++)
					{
						long ind=pgsz_xyz*c+z*pgsz_xy+y*pgsz_y+x;
						p_img8u_output[ind]=0.0;
					}
					continue;
				}

				//find 8 neighor pixels boundary
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w,r_w,t_w,b_w;
				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
				//compute weight for higer slice and lower slice
				double u_w,d_w;
				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

				//linear interpolate each channel
				for(long c=0;c<sz_img_input[3];c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					long ind_zsysxs=pgsz_xyz*c+z_s*pgsz_xy+y_s*pgsz_y+x_s;
					long ind_zsysxb=pgsz_xyz*c+z_s*pgsz_xy+y_s*pgsz_y+x_b;
					long ind_zsybxs=pgsz_xyz*c+z_s*pgsz_xy+y_b*pgsz_y+x_s;
					long ind_zsybxb=pgsz_xyz*c+z_s*pgsz_xy+y_b*pgsz_y+x_b;
					higher_slice=t_w*(l_w*p_img8u_input[ind_zsysxs]+r_w*p_img8u_input[ind_zsysxb])+
								 b_w*(l_w*p_img8u_input[ind_zsybxs]+r_w*p_img8u_input[ind_zsybxb]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					long ind_zbysxs=pgsz_xyz*c+z_b*pgsz_xy+y_s*pgsz_y+x_s;
					long ind_zbysxb=pgsz_xyz*c+z_b*pgsz_xy+y_s*pgsz_y+x_b;
					long ind_zbybxs=pgsz_xyz*c+z_b*pgsz_xy+y_b*pgsz_y+x_s;
					long ind_zbybxb=pgsz_xyz*c+z_b*pgsz_xy+y_b*pgsz_y+x_b;
					lower_slice =t_w*(l_w*p_img8u_input[ind_zbysxs]+r_w*p_img8u_input[ind_zbysxb])+
								 b_w*(l_w*p_img8u_input[ind_zbybxs]+r_w*p_img8u_input[ind_zbybxb]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					long ind=pgsz_xyz*c+z*pgsz_xy+y*pgsz_y+x;
					p_img8u_output[ind]=u_w*higher_slice+d_w*lower_slice+0.5;
				}

			}

//	//less memory efficient, more computation efficient
//	unsigned char ****p_img_input_4d=0,****p_img_output_4d=0;
//	if(!new4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img8u_input) ||
//	   !new4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img8u_output))
//	{
//		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
//		if(p_img8u_output) 		{delete []p_img8u_output;		p_img8u_output=0;}
//		if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
//		if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
//		return false;
//	}
//
//	//linear interpolate the output image according to the given transformation field
//	for(long x=0;x<sz_img_input[0];x++)
//		for(long y=0;y<sz_img_input[1];y++)
//			for(long z=0;z<sz_img_input[2];z++)
//			{
//				//coordinate in subject image
//				double cur_pos[3];//x,y,z
//				cur_pos[0]=vec4D_grid_int[y][x][z][0];
//				cur_pos[1]=vec4D_grid_int[y][x][z][1];
//				cur_pos[2]=vec4D_grid_int[y][x][z][2];
//
//				//compensate the minor float type error (+-0) around 0
//				if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
//				if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
//				if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
//				//if interpolate pixel is out of subject image region, set to -inf
//				if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 ||
//				   cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 ||
//				   cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
//				{
//					for(long c=0;c<sz_img_input[3];c++)
//						p_img_output_4d[c][z][y][x]=0.0;
//					continue;
//				}
//
//				//find 8 neighor pixels boundary
//				long x_s,x_b,y_s,y_b,z_s,z_b;
//				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
//				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
//				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);
//
//				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
//				double l_w,r_w,t_w,b_w;
//				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
//				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
//				//compute weight for higer slice and lower slice
//				double u_w,d_w;
//				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;
//
//				//linear interpolate each channel
//				for(long c=0;c<sz_img_input[3];c++)
//				{
//					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
//					double higher_slice;
//					higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
//								 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
//					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
//					double lower_slice;
//					lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
//								 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
//					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
//					p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice+0.5;
//				}
//
//			}
//
//	//free memory
//	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
//	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}

	return true;
}
bool q_rigidaffine_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		double *&p_img64f_output)
{
	//check paras
	if(p_img64f_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec4D_grid_int.size()!=sz_img_input[1] || vec4D_grid_int[0].size()!=sz_img_input[0] ||
	   vec4D_grid_int[0][0].size()!=sz_img_input[2] || vec4D_grid_int[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size, it should be same as input image size!\n");
		return false;
	}
	if(p_img64f_output)
	{
		printf("WARNNING: Output image pointer is not empty, original data will be deleted!\n");
		if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
	}

	//allocate memory
	p_img64f_output=new double[sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3]]();
	if(!p_img64f_output)
	{
		printf("ERROR: Fail to allocate memory for warpped image!\n");
		return false;
	}

	double ****p_img_input_4d=0,****p_img_output_4d=0;
	if(!new4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img64f_input) ||
	   !new4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img64f_output))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
		if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
		if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
		return false;
	}

	//linear interpolate the output image according to the given transformation field
#pragma omp parallel for
	for(long x=0;x<sz_img_input[0];x++)
		for(long y=0;y<sz_img_input[1];y++)
			for(long z=0;z<sz_img_input[2];z++)
			{
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0]=vec4D_grid_int[y][x][z][0];
				cur_pos[1]=vec4D_grid_int[y][x][z][1];
				cur_pos[2]=vec4D_grid_int[y][x][z][2];

				//compensate the minor float type error (+-0) around 0
				if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
				if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
				if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
				//if interpolate pixel is out of subject image region, set to -inf
				if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 ||
				   cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 ||
				   cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
				{
					for(long c=0;c<sz_img_input[3];c++)
					{
//						p_img_output_4d[c][z][y][x]=0.0;
						p_img_output_4d[c][z][y][x]=-1e+10;
					}
					continue;
				}

				//find 8 neighor pixels boundary
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w,r_w,t_w,b_w;
				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
				//compute weight for higer slice and lower slice
				double u_w,d_w;
				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

				//linear interpolate each channel
				for(long c=0;c<sz_img_input[3];c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
				}

			}

	//free memory
	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}

	return true;
}

//compute the similarity error (SSD) of two images
bool q_rigidaffine_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
		double &d_similarity)
{
	//check paras
	if(p_img64f_1==0 || p_img64f_2==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}

	long pgsz1_s=sz_img[0];
	long pgsz2_s=sz_img[0]*sz_img[1];
	for(long x=0;x<sz_img[0];x++)
		for(long y=0;y<sz_img[1];y++)
			for(long z=0;z<sz_img[2];z++)
			{
				long ind=pgsz2_s*z+pgsz1_s*y+x;
				if(p_img64f_1[ind]>=0 && p_img64f_2[ind]>=0)//disregard the invalid pixels
				{
					double diff=pow(p_img64f_1[ind]-p_img64f_2[ind],2);
					d_similarity+=diff;
				}
			}

	return true;
}

//compute the meshgrid nodes increment dE/dT (for Gradient Descent Method)
//E=[I(T(x))-I_tar(x)]^2
// => min(E) <=> T=T-u*(dE/dT)  <-- gradient descent
// => dE/dT=2(I-I_tar)(dI/dT)
// => x=AT => dI/dx=A(dI/dT) => dI/dT=inv(A)*dI/dx  <--- A indicates affine interpolation
bool q_rigidaffine_gridnodes_dT_3D(const int i_regtype,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_dT)
{
	//check paras
	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(p_img64f_tar==0 || p_img64f_sub==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]!=1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_gridwnd[0]<1 || sz_gridwnd[1]<1 || sz_gridwnd[2]<1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >=1!\n");
		return false;
	}
	if(vec4D_grid_dT.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid_gradient is not empty, original data will be cleared!\n");
		vec4D_grid_dT.clear();
	}

	//allocate memory
	double ****p_img64f_tar_4d=0,****p_img64f_sub_4d=0;
	if(!new4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_tar) ||
	   !new4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_sub))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	vector< vector< vector< vector<double> > > > vec4D_img_gradient;//4D: row,col,z,dxdydz
	vec4D_img_gradient.assign(sz_img[1],vector< vector< vector<double> > >(sz_img[0],vector< vector<double> >(sz_img[2],vector<double>(3,0))));

	//compute the gradient at every image pixel: 2(I-I_tar)(dI/dx)
	for(long z=0;z<sz_img[2];z++)
		for(long y=0;y<sz_img[1];y++)
			for(long x=0;x<sz_img[0];x++)
			{
				//find 6 neighor pixels boundary
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=x-1;	x_b=x+1;
				y_s=y-1;	y_b=y+1;
				z_s=z-1;	z_b=z+1;
				x_s = x_s<0 ? 0:x_s;		x_b = x_b>=sz_img[0] ? sz_img[0]-1:x_b;
				y_s = y_s<0 ? 0:y_s;		y_b = y_b>=sz_img[1] ? sz_img[1]-1:y_b;
				z_s = z_s<0 ? 0:z_s;		z_b = z_b>=sz_img[2] ? sz_img[2]-1:z_b;

				//compute image gradient <==> dI/dx=(Ix,Iy,Iz)
				double Ix,Iy,Iz;
				if(p_img64f_sub_4d[0][z][y][x_b]<0 || p_img64f_sub_4d[0][z][y][x_s]<0 ||
				   p_img64f_sub_4d[0][z][y_b][x]<0 || p_img64f_sub_4d[0][z][y_s][x]<0 ||
				   p_img64f_sub_4d[0][z_b][y][x]<0 || p_img64f_sub_4d[0][z_s][y][x]<0 ||
				   p_img64f_tar_4d[0][z][y][x]<0   || p_img64f_sub_4d[0][z][y][x]<0)	//disregard the invalid pixels
				{
					vec4D_img_gradient[y][x][z][0]=0;
					vec4D_img_gradient[y][x][z][1]=0;
					vec4D_img_gradient[y][x][z][2]=0;
				}
				else
				{
					Ix=p_img64f_sub_4d[0][z][y][x_b]-p_img64f_sub_4d[0][z][y][x_s];	//Ix=(E-W)/2
					Iy=p_img64f_sub_4d[0][z][y_b][x]-p_img64f_sub_4d[0][z][y_s][x];	//Iy=(S-N)/2
					Iz=p_img64f_sub_4d[0][z_b][y][x]-p_img64f_sub_4d[0][z_s][y][x];	//Iz=(down-up)/2

					//2(I-I_tar)(dI/dx)
					double I_Itar=p_img64f_sub_4d[0][z][y][x]-p_img64f_tar_4d[0][z][y][x];
					vec4D_img_gradient[y][x][z][0]=I_Itar*Ix;
					vec4D_img_gradient[y][x][z][1]=I_Itar*Iy;
					vec4D_img_gradient[y][x][z][2]=I_Itar*Iz;
				}
			}

	//inverse affine interpolate to obtain 2(I-Io)(dI/dT) at every grid node
	//the affine paras are estimated using least square fitting
	//2(I-I_tar)(dI/dT)=2(I-I_tar)[inv(A)*(dI/dx)]
	if(!q_rigidaffine_field2grid_3D(i_regtype,vec4D_img_gradient,sz_gridwnd,vec4D_grid_dT))
	{
		printf("ERROR: q_rigid_field2grid_3D() return false!\n");
		if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	//free memory
	if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}

//update grid nodes position: T_tmp=T-gamma*dT
bool q_rigidaffine_gridnodes_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update)
{
	//check paras
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(vec4D_grid.size()!=vec4D_grid_dT.size() || vec4D_grid[0].size()!=vec4D_grid_dT[0].size() ||
	   vec4D_grid[0][0].size()!=vec4D_grid_dT[0][0].size() || vec4D_grid[0][0][0].size()!=vec4D_grid_dT[0][0][0].size())
	{
		printf("ERROR: Input vec4D_grid.size() != Input vec4D_grid_dT.size()!\n");
		return false;
	}
	if(vec4D_grid_update.size())
	{
		vec4D_grid_update.clear();
	}
	vec4D_grid_update=vec4D_grid;

	//update grid nodes position: T=T_ori+T_offset
	long sz_grid[3]={2,2,2};//w,h,z
	for(long z=0;z<sz_grid[2];z++)
		for(long y=0;y<sz_grid[1];y++)
			for(long x=0;x<sz_grid[0];x++)
				for(long xyz=0;xyz<3;xyz++)
					vec4D_grid_update[y][x][z][xyz]=vec4D_grid[y][x][z][xyz]-d_gamma*vec4D_grid_dT[y][x][z][xyz];

	return true;
}

//generate rigid warped subject image according to the deformed grid
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const unsigned char *p_img8u_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		unsigned char *&p_img8u_sub_warp)
{
	//check paras
	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(p_img8u_sub==0)
	{
		printf("ERROR: Invalid input image pointer !\n");
		return false;
	}
	if(sz_img_sub[0]<=0 || sz_img_sub[1]<=0 || sz_img_sub[2]<=0 || sz_img_sub[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec4D_grid.size()==0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(p_img8u_sub_warp)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img8u_sub_warp;		p_img8u_sub_warp=0;
	}

	//rigid interpolate the warp grid to field
	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if(!q_rigidaffine_grid2field_3D(i_regtype,vec4D_grid,sz_img_sub,sz_img_sub,vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	//warp embeded subject image base on the warp field (linear interpolation)
	if(!q_rigidaffine_interpolate_3D(p_img8u_sub,sz_img_sub,vec4D_grid_int,p_img8u_sub_warp))
	{
		printf("ERROR: q_affine_interpolate_3D() return false!\n");
		delete []p_img8u_sub_warp;		p_img8u_sub_warp=0;
		return false;
	}

	return true;
}
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		double *&p_img64f_sub_warp)
{
	//check paras
	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}
	if(p_img64f_sub==0)
	{
		printf("ERROR: Invalid input image pointer !\n");
		return false;
	}
	if(sz_img_sub[0]<=0 || sz_img_sub[1]<=0 || sz_img_sub[2]<=0 || sz_img_sub[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec4D_grid.size()==0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(p_img64f_sub_warp)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;
	}

	//rigid interpolate the warp grid to field
	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if(!q_rigidaffine_grid2field_3D(i_regtype,vec4D_grid,sz_img_sub,sz_img_sub,vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	//warp embeded subject image base on the warp field (linear interpolation)
	if(!q_rigidaffine_interpolate_3D(p_img64f_sub,sz_img_sub,vec4D_grid_int,p_img64f_sub_warp))
	{
		printf("ERROR: q_affine_interpolate_3D() return false!\n");
		delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;
		return false;
	}

	return true;
}

//save deformd meshgrid to swc file (offset of nodes are indicate by lines)
bool q_rigidaffine_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd[3],
		const char *filename)
{
	//check paras
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()==0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(sz_gridwnd[0]<=0 || sz_gridwnd[1]<=0 || sz_gridwnd[2]<=0)
	{
		printf("ERROR: Invalid input sz_gridwnd!\n");
		return false;
	}
	if(!filename)
	{
		printf("ERROR: Invalid output filename!\n");
		return false;
	}

	//input grid size
	long sz_grid[4];//w,h,z
	sz_grid[1]=vec4D_grid.size();			//nrow
	sz_grid[0]=vec4D_grid[0].size();		//ncol
	sz_grid[2]=vec4D_grid[0][0].size();		//nz

	FILE *fp;
	fp=fopen(filename,"w");
	if(fp==NULL)
	{
		fprintf(stderr,"ERROR: Failed to open file to write! \n");
		return false;
	}

	long n=0;
	for(long x=0;x<sz_grid[0];x++)
		for(long y=0;y<sz_grid[1];y++)
			for(long z=0;z<sz_grid[2];z++)
			{
				fprintf(fp,"%ld 1 %ld %ld %ld 1 %ld\n",2*n+1,x*(sz_gridwnd[0]-1), y*(sz_gridwnd[1]-1), z*(sz_gridwnd[2]-1), 2*n+2);
				fprintf(fp,"%ld 2 %.2f %.2f %.2f 1 -1\n",2*n+2,vec4D_grid[y][x][z][0], vec4D_grid[y][x][z][1],vec4D_grid[y][x][z][2]);
				n++;
			}

	fclose(fp);

	return true;
}

