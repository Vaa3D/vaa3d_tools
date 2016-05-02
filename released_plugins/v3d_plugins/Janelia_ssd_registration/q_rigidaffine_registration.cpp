// q_rigidaffine_registration.cpp
// by Lei Qu
// 2011-04-08

#include "q_rigidaffine_registration.h"

#include <iostream>
#include <math.h>
#include "basic_memory.cpp"
#include "stackutil.h"
#include "common/q_convolve.h"


bool q_rigidaffine_registration(const CParas_reg &paras,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	if(paras.i_regtype!=0 && paras.i_regtype!=1)
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
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

	printf("\t(1). Generate rectangular control points:\n");
	long sz_gridwnd[3];
	for(long i=0;i<3;i++)
		sz_gridwnd[i]=sz_img[i];
	printf("\t\t>>sz_gridwnd: [%ld,%ld,%ld]\n",sz_gridwnd[0],sz_gridwnd[1],sz_gridwnd[2]);

	if(!q_rigidaffine_inigrid_generator(sz_gridwnd,vec4D_grid))
	{
		printf("ERROR: q_rigid_inigrid_generator() return false!\n");
		return false;
	}
	vector< vector< vector< vector<double> > > > vec4D_grid_ori(vec4D_grid);

	printf("\t(3). Estimate initial optimization step size.\n");
	double d_gamma;
	double maxstep=1e+5;
        for(long i=0;i<3;i++)
		if(maxstep>sz_img[i])
			maxstep=sz_img[i];
	d_gamma=maxstep*paras.d_step_inimultiplyfactor;
	printf("\t\t>>d_gamma:%f\n",d_gamma);

	printf("\t(4). Align the mass center of target and subject image.\n");
	double *p_img64f_sub_shift=0;
	long l_sub2tar_masscenteroffset[3]={0,0,0};

	if(!paras.b_alignmasscenter)
	{
		p_img64f_sub_shift=new double[sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]]();
		if(!p_img64f_sub_shift)
		{
			printf("ERROR: Fail to allocate memory for p_img64f_sub_shift!\n");
			return false;
		}
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

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("\t(4). Enter iterative optimization. \n");
	double d_E=1e+10;
	double d_E_tmp=0.0;
	double d_E_change=1.0;
	double *p_img64f_warp=0;
	vector< vector< vector< vector<double> > > > vec4D_grid_tmp(vec4D_grid);
	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	vector< vector< vector< vector<double> > > > vec4D_grid_dT;

	long iter=0;
	while(/*fabs(d_E_change)>1e-5 && */iter<paras.l_iter_max && fabs(d_gamma)>paras.d_step_min && fabs(d_E)>1e-5)
	{
		iter++;

		vec4D_grid_int.clear();
		if(!q_rigidaffine_grid2field_3D(paras.i_regtype,vec4D_grid_tmp,sz_gridwnd,sz_gridwnd,vec4D_grid_int))
		{
			printf("ERROR: q_rigid_grid2field_3D() return false!\n");
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		if(p_img64f_warp)	 {delete []p_img64f_warp;	p_img64f_warp=0;}
		if(!q_rigidaffine_interpolate_3D(p_img64f_sub_shift,sz_img,vec4D_grid_int,p_img64f_warp))
		{
			printf("ERROR: q_rigid_interpolate_3D() return false!\n");
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		double d_E_similarity=0.0;
		if(!q_rigidaffine_similarity_3D(p_img64f_warp,p_img64f_tar,sz_img,d_E_similarity))
		{
			printf("ERROR: q_rigid_similarity_3D() return false!\n");
			if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
			if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
			return false;
		}

		d_E_tmp=d_E_similarity;

		d_E_change=d_E_tmp-d_E;
		if(d_E_change>=0)
		{
			printf("\t\t\t>>Iter=%4ld,  E_tmp=%.5f,  E_change=%.5f,  gamma=%.5f\n",iter,d_E_tmp,d_E_change,d_gamma);
			d_gamma*=paras.d_step_annealingratio;
		}
		else
		{
			vec4D_grid=vec4D_grid_tmp;
			d_E=d_E_tmp;

			vec4D_grid_dT.clear();
			if(!q_rigidaffine_gridnodes_dT_3D(paras.i_regtype,p_img64f_tar,p_img64f_warp,sz_img,sz_gridwnd,vec4D_grid_dT))
			{
				printf("ERROR: q_rigid_gridnodes_dT_3D() return false!\n");
				if(p_img64f_warp)	 		{delete []p_img64f_warp;		p_img64f_warp=0;}
				if(p_img64f_sub_shift)	 	{delete []p_img64f_sub_shift;		p_img64f_sub_shift=0;}
				return false;
			}
			printf("\t\t\t>>Iter=%4ld,  E    =%.5f,  E_change=%.5f,  gamma=%.5f --->update!\n",iter,d_E,d_E_change,d_gamma);
		}

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


bool q_rigidaffine_inigrid_generator(const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
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

	long sz_grid[3]={2,2,2};

	vec4D_grid.assign(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));
	for(long x=0;x<sz_grid[0];x++)
		for(long y=0;y<sz_grid[1];y++)
			for(long z=0;z<sz_grid[2];z++)
			{
				vec4D_grid[y][x][z][0]=x*(sz_gridwnd[0]-1);
				vec4D_grid[y][x][z][1]=y*(sz_gridwnd[1]-1);
				vec4D_grid[y][x][z][2]=z*(sz_gridwnd[2]-1);
			}

	return true;
}

bool q_rigidaffine_compute_rigidmatrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_rigidmatrix)
{
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

	vector<Point3D64f> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	Matrix x3xn_A(3,n_point),x3xn_B(3,n_point);
	for(long i=0;i<n_point;i++)
	{
		x3xn_A(1,i+1)=vec_A_norm[i].x;	x3xn_A(2,i+1)=vec_A_norm[i].y;	x3xn_A(3,i+1)=vec_A_norm[i].z;
		x3xn_B(1,i+1)=vec_B_norm[i].x;	x3xn_B(2,i+1)=vec_B_norm[i].y;	x3xn_B(3,i+1)=vec_B_norm[i].z;
	}

	DiagonalMatrix D;
	Matrix U,V;
	SVD(x3xn_A*x3xn_B.t(),D,U,V);
	Matrix R=V*U.t();

	x4x4_rigidmatrix(1,1)=R(1,1);	x4x4_rigidmatrix(1,2)=R(1,2);	x4x4_rigidmatrix(1,3)=R(1,3);	x4x4_rigidmatrix(1,4)=0.0;
	x4x4_rigidmatrix(2,1)=R(2,1);	x4x4_rigidmatrix(2,2)=R(2,2);	x4x4_rigidmatrix(2,3)=R(2,3);	x4x4_rigidmatrix(2,4)=0.0;
	x4x4_rigidmatrix(3,1)=R(3,1);	x4x4_rigidmatrix(3,2)=R(3,2);	x4x4_rigidmatrix(3,3)=R(3,3);	x4x4_rigidmatrix(3,4)=0.0;
	x4x4_rigidmatrix(4,1)=0.0;		x4x4_rigidmatrix(4,2)=0.0;		x4x4_rigidmatrix(4,3)=0.0;		x4x4_rigidmatrix(4,4)=1.0;
	x4x4_rigidmatrix=x4x4_normalize_B.i()*x4x4_rigidmatrix*x4x4_normalize_A;

	return true;
}

bool q_rigidaffine_compute_affinematrix_3D(const vector<Point3D64f> &vec_A,const vector<Point3D64f> &vec_B,Matrix &x4x4_affinematrix)
{
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	vector<Point3D64f> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

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

	DiagonalMatrix D(13);
	Matrix U(3*n_point,13),V(13,13);
	SVD(A,D,U,V);

	Matrix h=V.column(13);
	for(int i=1;i<=13;i++)
		h(i,1) /= h(13,1);

	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;
	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}


bool q_rigidaffine_grid2field_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd_input[3],const long sz_gridwnd_output[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_int)
{
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

	double d_scalefactor[3];
	for(long i=0;i<3;i++)
		d_scalefactor[i]=double(sz_gridwnd_output[i])/double(sz_gridwnd_input[i]);

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

	Matrix x4x4_transmatrix(4,4);
	if(i_regtype==0)
		q_rigidaffine_compute_rigidmatrix_3D(vec_ori,vec_cur,x4x4_transmatrix);
	else if(i_regtype==1)
		q_rigidaffine_compute_affinematrix_3D(vec_ori,vec_cur,x4x4_transmatrix);
	else
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}

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

bool q_rigidaffine_field2grid_3D(const int i_regtype,
		const vector< vector< vector< vector<double> > > > &vec4D_field,
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
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

	long sz_grid[3]={2,2,2};
	long sz_grid_int[3];
	sz_grid_int[1]=vec4D_field.size();
	sz_grid_int[0]=vec4D_field[0].size();
	sz_grid_int[2]=vec4D_field[0][0].size();

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

	Matrix x4x4_transmatrix(4,4);
	if(i_regtype==0)
		q_rigidaffine_compute_rigidmatrix_3D(vec_ori,vec_cur,x4x4_transmatrix);
	else if(i_regtype==1)
		q_rigidaffine_compute_affinematrix_3D(vec_ori,vec_cur,x4x4_transmatrix);
	else
	{
		printf("ERROR: Unknown given regitration type (0:rigid, 1:affine)!\n");
		return false;
	}

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

bool q_rigidaffine_interpolate_3D(const unsigned char *p_img8u_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		unsigned char *&p_img8u_output)
{
	if(p_img8u_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]!=1)
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

	p_img8u_output=new unsigned char[sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3]]();
	if(!p_img8u_output)
	{
		printf("ERROR: Fail to allocate memory for warpped image!\n");
		return false;
	}

	unsigned char ****p_img_input_4d=0,****p_img_output_4d=0;
	if(!new4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img8u_input) ||
	   !new4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img8u_output))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img8u_output) 		{delete []p_img8u_output;		p_img8u_output=0;}
		if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
		if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
		return false;
	}

	for(long x=0;x<sz_img_input[0];x++)
		for(long y=0;y<sz_img_input[1];y++)
			for(long z=0;z<sz_img_input[2];z++)
			{
				double cur_pos[3];
				cur_pos[0]=vec4D_grid_int[y][x][z][0];
				cur_pos[1]=vec4D_grid_int[y][x][z][1];
				cur_pos[2]=vec4D_grid_int[y][x][z][2];

				if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
				if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
				if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
				if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 ||
				   cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 ||
				   cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
				{
					p_img_output_4d[0][z][y][x]=0.0;
					continue;
				}

				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

				double l_w,r_w,t_w,b_w;
				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
				double u_w,d_w;
				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

				for(long c=0;c<sz_img_input[3];c++)
				{
					double higher_slice;
					higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
					double lower_slice;
					lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
					p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice+0.5;
				}

			}

	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}

	return true;
}
bool q_rigidaffine_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		double *&p_img64f_output)
{
	if(p_img64f_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]!=1)
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

	for(long x=0;x<sz_img_input[0];x++)
		for(long y=0;y<sz_img_input[1];y++)
			for(long z=0;z<sz_img_input[2];z++)
			{
				double cur_pos[3];
				cur_pos[0]=vec4D_grid_int[y][x][z][0];
				cur_pos[1]=vec4D_grid_int[y][x][z][1];
				cur_pos[2]=vec4D_grid_int[y][x][z][2];

				if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
				if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
				if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
				if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 ||
				   cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 ||
				   cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
				{
					p_img_output_4d[0][z][y][x]=-1e+10;
					continue;
				}

				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

				double l_w,r_w,t_w,b_w;
				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
				double u_w,d_w;
				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

				for(long c=0;c<sz_img_input[3];c++)
				{
					double higher_slice;
					higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
					double lower_slice;
					lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
					p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
				}

			}

	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}

	return true;
}

bool q_rigidaffine_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
		double &d_similarity)
{
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
				if(p_img64f_1[ind]>=0 && p_img64f_2[ind]>=0)
				{
					double diff=pow(p_img64f_1[ind]-p_img64f_2[ind],2);
					d_similarity+=diff;
				}
			}

	return true;
}

bool q_rigidaffine_gridnodes_dT_3D(const int i_regtype,
		const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd[3],
		vector< vector< vector< vector<double> > > > &vec4D_grid_dT)
{
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

	double ****p_img64f_tar_4d=0,****p_img64f_sub_4d=0;
	if(!new4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_tar) ||
	   !new4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_sub))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	vector< vector< vector< vector<double> > > > vec4D_img_gradient;
	vec4D_img_gradient.assign(sz_img[1],vector< vector< vector<double> > >(sz_img[0],vector< vector<double> >(sz_img[2],vector<double>(3,0))));

	for(long z=0;z<sz_img[2];z++)
		for(long y=0;y<sz_img[1];y++)
			for(long x=0;x<sz_img[0];x++)
			{
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=x-1;	x_b=x+1;
				y_s=y-1;	y_b=y+1;
				z_s=z-1;	z_b=z+1;
				x_s = x_s<0 ? 0:x_s;		x_b = x_b>=sz_img[0] ? sz_img[0]-1:x_b;
				y_s = y_s<0 ? 0:y_s;		y_b = y_b>=sz_img[1] ? sz_img[1]-1:y_b;
				z_s = z_s<0 ? 0:z_s;		z_b = z_b>=sz_img[2] ? sz_img[2]-1:z_b;

				double Ix,Iy,Iz;
				if(p_img64f_sub_4d[0][z][y][x_b]<0 || p_img64f_sub_4d[0][z][y][x_s]<0 ||
				   p_img64f_sub_4d[0][z][y_b][x]<0 || p_img64f_sub_4d[0][z][y_s][x]<0 ||
				   p_img64f_sub_4d[0][z_b][y][x]<0 || p_img64f_sub_4d[0][z_s][y][x]<0 ||
				   p_img64f_tar_4d[0][z][y][x]<0   || p_img64f_sub_4d[0][z][y][x]<0)
				{
					vec4D_img_gradient[y][x][z][0]=0;
					vec4D_img_gradient[y][x][z][1]=0;
					vec4D_img_gradient[y][x][z][2]=0;
				}
				else
				{
					Ix=p_img64f_sub_4d[0][z][y][x_b]-p_img64f_sub_4d[0][z][y][x_s];
					Iy=p_img64f_sub_4d[0][z][y_b][x]-p_img64f_sub_4d[0][z][y_s][x];
					Iz=p_img64f_sub_4d[0][z_b][y][x]-p_img64f_sub_4d[0][z_s][y][x];

					double I_Itar=p_img64f_sub_4d[0][z][y][x]-p_img64f_tar_4d[0][z][y][x];
					vec4D_img_gradient[y][x][z][0]=I_Itar*Ix;
					vec4D_img_gradient[y][x][z][1]=I_Itar*Iy;
					vec4D_img_gradient[y][x][z][2]=I_Itar*Iz;
				}
			}

	if(!q_rigidaffine_field2grid_3D(i_regtype,vec4D_img_gradient,sz_gridwnd,vec4D_grid_dT))
	{
		printf("ERROR: q_rigid_field2grid_3D() return false!\n");
		if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}

bool q_rigidaffine_gridnodes_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update)
{
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

	long sz_grid[3]={2,2,2};
	for(long z=0;z<sz_grid[2];z++)
		for(long y=0;y<sz_grid[1];y++)
			for(long x=0;x<sz_grid[0];x++)
				for(long xyz=0;xyz<3;xyz++)
					vec4D_grid_update[y][x][z][xyz]=vec4D_grid[y][x][z][xyz]-d_gamma*vec4D_grid_dT[y][x][z][xyz];

	return true;
}

bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const unsigned char *p_img8u_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		unsigned char *&p_img8u_sub_warp)
{
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
	if(p_img8u_sub_warp!=0)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img8u_sub_warp;		p_img8u_sub_warp=0;
	}

	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if(!q_rigidaffine_grid2field_3D(i_regtype,vec4D_grid,sz_img_sub,sz_img_sub,vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	p_img8u_sub_warp=new unsigned char[sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]*sz_img_sub[3]]();
	if(!p_img8u_sub_warp)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub_embed_warp!\n");
		return false;
	}
	unsigned char *p_img8u_sub_1c=0;
	p_img8u_sub_1c=new unsigned char[sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]]();
	if(!p_img8u_sub_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub_1c!\n");
		return false;
	}

	long sz_img_1c[4];
	sz_img_1c[0]=sz_img_sub[0];	sz_img_1c[1]=sz_img_sub[1];	sz_img_1c[2]=sz_img_sub[2];	sz_img_1c[3]=1;
	long pgsz_y=sz_img_sub[0];
	long pgsz_xy=sz_img_sub[0]*sz_img_sub[1];
	long pgsz_xyz=sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2];
	for(long c=0;c<sz_img_sub[3];c++)
	{
		for(long x=0;x<sz_img_sub[0];x++)
			for(long y=0;y<sz_img_sub[1];y++)
				for(long z=0;z<sz_img_sub[2];z++)
				{
					long ind_1c=z*pgsz_xy+y*pgsz_y+x;
					long ind_nc=c*pgsz_xyz+ind_1c;
					p_img8u_sub_1c[ind_1c]=p_img8u_sub[ind_nc];
				}

		unsigned char *p_img8u_sub_warp_1c=0;
		if(!q_rigidaffine_interpolate_3D(p_img8u_sub_1c,sz_img_1c,vec4D_grid_int,p_img8u_sub_warp_1c))
		{
			printf("ERROR: q_affine_interpolate_3D() return false!\n");
			delete []p_img8u_sub_warp;		p_img8u_sub_warp=0;
			delete []p_img8u_sub_1c;		p_img8u_sub_1c=0;
			return false;
		}

		for(long x=0;x<sz_img_sub[0];x++)
			for(long y=0;y<sz_img_sub[1];y++)
				for(long z=0;z<sz_img_sub[2];z++)
				{
					long ind_1c=z*pgsz_xy+y*pgsz_y+x;
					long ind_nc=c*pgsz_xyz+ind_1c;
					p_img8u_sub_warp[ind_nc]=p_img8u_sub_warp_1c[ind_1c];
				}

		delete []p_img8u_sub_warp_1c;		p_img8u_sub_warp_1c=0;
	}
	delete []p_img8u_sub_1c;		p_img8u_sub_1c=0;

	return true;
}
bool q_rigidaffine_warpimage_baseongrid(const int i_regtype,
		const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,
		double *&p_img64f_sub_warp)
{
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
	if(p_img64f_sub_warp!=0)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;
	}

	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if(!q_rigidaffine_grid2field_3D(i_regtype,vec4D_grid,sz_img_sub,sz_img_sub,vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	p_img64f_sub_warp=new double[sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]*sz_img_sub[3]]();
	if(!p_img64f_sub_warp)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub_embed_warp!\n");
		return false;
	}
	double *p_img64f_sub_1c=0;
	p_img64f_sub_1c=new double[sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]]();
	if(!p_img64f_sub_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub_1c!\n");
		return false;
	}

	long sz_img_1c[4];
	sz_img_1c[0]=sz_img_sub[0];	sz_img_1c[1]=sz_img_sub[1];	sz_img_1c[2]=sz_img_sub[2];	sz_img_1c[3]=1;
	long pgsz_y=sz_img_sub[0];
	long pgsz_xy=sz_img_sub[0]*sz_img_sub[1];
	long pgsz_xyz=sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2];
	for(long c=0;c<sz_img_sub[3];c++)
	{
		for(long x=0;x<sz_img_sub[0];x++)
			for(long y=0;y<sz_img_sub[1];y++)
				for(long z=0;z<sz_img_sub[2];z++)
				{
					long ind_1c=z*pgsz_xy+y*pgsz_y+x;
					long ind_nc=c*pgsz_xyz+ind_1c;
					p_img64f_sub_1c[ind_1c]=p_img64f_sub[ind_nc];
				}

		double *p_img64f_sub_warp_1c=0;
		if(!q_rigidaffine_interpolate_3D(p_img64f_sub_1c,sz_img_1c,vec4D_grid_int,p_img64f_sub_warp_1c))
		{
			printf("ERROR: q_affine_interpolate_3D() return false!\n");
			delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;
			delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;
			return false;
		}

		for(long x=0;x<sz_img_sub[0];x++)
			for(long y=0;y<sz_img_sub[1];y++)
				for(long z=0;z<sz_img_sub[2];z++)
				{
					long ind_1c=z*pgsz_xy+y*pgsz_y+x;
					long ind_nc=c*pgsz_xyz+ind_1c;

					if(p_img64f_sub_warp_1c[ind_1c]<0)
						p_img64f_sub_warp[ind_nc]=0;
					else
						p_img64f_sub_warp[ind_nc]=p_img64f_sub_warp_1c[ind_1c];
				}

		delete []p_img64f_sub_warp_1c;		p_img64f_sub_warp_1c=0;
	}
	delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;

	return true;
}

bool q_rigidaffine_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd[3],
		const char *filename)
{
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

	long sz_grid[4];
	sz_grid[1]=vec4D_grid.size();
	sz_grid[0]=vec4D_grid[0].size();
	sz_grid[2]=vec4D_grid[0][0].size();

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

