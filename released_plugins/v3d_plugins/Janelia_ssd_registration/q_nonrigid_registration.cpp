// q_nonrigid_registration.cpp
// by Lei Qu
// 2010-02-17

#ifndef SHOWTIME
	#define SHOWTIME 0
#endif

#include "q_nonrigid_registration.h"
#include "q_imresize.h"
#include "q_convolve.h"

#include <QtGui>
#include <time.h>
#include <math.h>
#include "../../basic_c_fun/basic_memory.cpp"
#include "../../basic_c_fun/stackutil.h"


bool q_nonrigid_registration_FFD(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long l_hierarchlevel,const long sz_gridwnd,
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
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
	if(l_hierarchlevel<1)
	{
		printf("ERROR: Invalid input hierarchical level, at least = 1!\n");
		return false;
	}
	if(sz_gridwnd<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	clock_t start;

	printf("(1). Initialize B-spline basis/blending functions:\n");
	if(SHOWTIME) start=clock();
	Matrix x_bsplinebasis(pow(sz_gridwnd,3),pow(4,3));
	if(!q_ini_bsplinebasis_3D(sz_gridwnd,x_bsplinebasis))
	{
		printf("ERROR: q_ini_bsplinebasis_3D() return false!\n");
		return false;
	}
	printf("\t>>x_bsplinebasis:[%d,%d]\n",x_bsplinebasis.nrows(),x_bsplinebasis.ncols());
	if(SHOWTIME) printf("\t>>time consume: %.2f s\n",(clock()-start)/100.0);

	printf("(2). Generate initial B-spline control points meshgrid:\n");
	if(SHOWTIME) start=clock();
	long sz_img_lowest[4]={1,1,1,1};
	for(long i=0;i<3;i++)
		sz_img_lowest[i]=ceil(sz_img[i]/pow(2,l_hierarchlevel-1));
	vector< vector< vector< vector<double> > > > vec4D_grid_ori;
	if(!q_inigrid_generator(sz_img_lowest,sz_gridwnd,vec4D_grid))
	{
		printf("ERROR: q_inigrid_generator() return false!\n");
		return false;
	}
	printf("\t>>grid_size: %d,%d,%d; gridwind_size: %d\n",vec4D_grid[0].size(),vec4D_grid.size(),vec4D_grid[0][0].size(),sz_gridwnd);
	vec4D_grid_ori=vec4D_grid;
	if(SHOWTIME) printf("\t>>time consume: %.2f s\n",(clock()-start)/100.0);

	printf("(4). Enter hierarchical non-rigid registration iteration:\n");
	for(long level=0;level<l_hierarchlevel;level++)
	{
		printf("\t----------------------------------------------------------\n");
		printf("\t         HIERARCHICIAL LEVEL: [%ld]\n",level);
		printf("\t----------------------------------------------------------\n");

		printf("\t4.1. re-embed images for current hierachicial level.\n");
		long sz_img_resize[4]={1,1,1,1};
		for(long i=0;i<3;i++)
			sz_img_resize[i]=ceil(sz_img[i]/pow(2,l_hierarchlevel-level-1));
		double *p_img64f_resize_tar=0,*p_img64f_resize_sub=0;
		if(!q_imresize64f_3D(p_img64f_tar,sz_img,sz_img_resize,p_img64f_resize_tar) ||
		   !q_imresize64f_3D(p_img64f_sub,sz_img,sz_img_resize,p_img64f_resize_sub))
		{
			printf("ERROR: q_imresize64f_3D() return false!\n");
			if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
			if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
			return false;
		}
		printf("\t\t>>sz_img_resize: %ld,%ld,%ld,%ld\n",sz_img_resize[0],sz_img_resize[1],sz_img_resize[2],sz_img_resize[3]);
		double d_value4invalid=0;
		double *p_img64f_embed_tar=0,*p_img64f_embed_sub=0;
		long sz_img64f_embed[4]={1,1,1,1};
		if(!q_img_embed(p_img64f_resize_tar,sz_img_resize,sz_gridwnd,d_value4invalid,p_img64f_embed_tar,sz_img64f_embed) ||
		   !q_img_embed(p_img64f_resize_sub,sz_img_resize,sz_gridwnd,d_value4invalid,p_img64f_embed_sub,sz_img64f_embed))
		{
			printf("ERROR: q_img_embed() return false!\n");
			if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
			if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
			if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
			if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
			return false;
		}
		printf("\t\t>>sz_img_embed: %ld,%ld,%ld,%ld\n",sz_img64f_embed[0],sz_img64f_embed[1],sz_img64f_embed[2],sz_img64f_embed[3]);

		if(p_img64f_resize_tar) 	{delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
		if(p_img64f_resize_sub)	 	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}

		printf("\t4.2. update the Gaussian kernel sigma used for smoothing transformation field.\n");
		double d_gaussian_sigma=1.0;
		printf("\t\t>>d_gaussian_std: %.2f\n",d_gaussian_sigma);

		printf("\t4.3. estimate initial optimization step size.\n");
		double d_gamma=1.0;
		if(level==0)
		{
			double maxstep=1e+5;
			for(long i=0;i<3;i++)
				if(maxstep>sz_img[i])
					maxstep=sz_img[i];
			d_gamma=maxstep/pow(2,l_hierarchlevel-level-1);
		}
		printf("\t\t>>d_gamma:%f\n",d_gamma);

		printf("\t4.4. enter iterative optimization of level:[%ld]. \n",level);
		double d_E=1e+10;
		double d_E_tmp=0.0;
		double d_E_change=1.0;
		double *p_img64f_warp=0;
		vector< vector< vector< vector<double> > > > vec4D_grid_tmp(vec4D_grid);
		vector< vector< vector< vector<double> > > > vec4D_grid_int;
		vector< vector< vector< vector<double> > > > vec4D_grid_dT;

		long iter=0;
		while(/*fabs(d_E_change)>1e-5 && */iter<200 && fabs(d_gamma)>0.01 && fabs(d_E)>1e-5)
		{
			iter++;

			double d_E_smooth=0.0;

			if(SHOWTIME) start=clock();
			vec4D_grid_int.clear();
			if(!q_bspline_grid2field_3D(vec4D_grid_tmp,sz_gridwnd,x_bsplinebasis,vec4D_grid_int))
			{
				printf("ERROR: q_bspline_grid2field_3D() return false!\n");
				if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
				if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
				if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
				if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
				return false;
			}
			if(SHOWTIME) printf("\t\t\t>>q_bspline_grid2field_3D() time consume: %.2f s\n",(clock()-start)/100.0);

			if(SHOWTIME) start=clock();
			if(p_img64f_warp)	 {delete []p_img64f_warp;	p_img64f_warp=0;}
			if(!q_interpolate_3D(p_img64f_embed_sub,sz_img64f_embed,vec4D_grid_int,p_img64f_warp))
			{
				printf("ERROR: q_interpolate_3D() return false!\n");
				if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
				if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
				if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
				if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
				return false;
			}
			if(SHOWTIME) printf("\t\t\t>>q_interpolate_3D() time consume: %.2f s\n",(clock()-start)/100.0);

			if(SHOWTIME) start=clock();
			double d_E_similarity=0.0;
			if(!q_similarity_3D(p_img64f_warp,p_img64f_embed_tar,sz_img64f_embed,d_E_similarity))
			{
				printf("ERROR: q_similarity_3D() return false!\n");
				if(p_img64f_warp)	 	{delete []p_img64f_warp;			p_img64f_warp=0;}
				if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
				if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
				if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
				if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
				return false;
			}
			if(SHOWTIME) printf("\t\t\t>>q_similarity_3D() time consume: %.2f s\n",(clock()-start)/100.0);

			d_E_tmp=d_E_similarity;

			if(SHOWTIME) start=clock();
			d_E_change=d_E_tmp-d_E;
			if(d_E_change>=0)
			{
				printf("\t\t>>Level=%2ld,  Iter=%4ld,  E_tmp=%.5f,  E_change=%.5f,  gamma=%.5f\n",level,iter,d_E_tmp,d_E_change,d_gamma);
				d_gamma*=0.9;
			}
			else
			{
				vec4D_grid=vec4D_grid_tmp;
				d_E=d_E_tmp;

				vec4D_grid_dT.clear();
				if(!q_gridnodes_dT_3D(p_img64f_embed_tar,p_img64f_warp,sz_img64f_embed,sz_gridwnd,x_bsplinebasis,vec4D_grid_dT))
				{
					printf("ERROR: q_gridnodes_gradient_3D() return false!\n");
					if(p_img64f_warp)	 	{delete []p_img64f_warp;			p_img64f_warp=0;}
					if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
					if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
					if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
					if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
					return false;
				}
				printf("\t\t>>Level=%2ld,  Iter=%4ld,  E    =%.5f,  E_change=%.5f,  gamma=%.5f --->update!\n",level,iter,d_E,d_E_change,d_gamma);
			}
			if(SHOWTIME) printf("\t\t\t>>q_gridnodes_dT_3D() time consume: %.2f s\n",(clock()-start)/100.0);

			if(SHOWTIME) start=clock();
			vec4D_grid_tmp.clear();
			if(!q_grid_update_3D(vec4D_grid,vec4D_grid_ori,vec4D_grid_dT,d_gamma,d_gaussian_sigma,vec4D_grid_tmp))
			{
				printf("ERROR: q_grid_update_3D() return false!\n");
				if(p_img64f_warp)	 	{delete []p_img64f_warp;			p_img64f_warp=0;}
				if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
				if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
				if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
				if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
				return false;
			}
			if(SHOWTIME) printf("\t\t\t>>q_grid_update_3D() time consume: %.2f s\n",(clock()-start)/100.0);

		}

		if(level<l_hierarchlevel-1)
		{
			printf("\t4.5. subdivid and trim grid for the next hierarchical level. \n");
			if(!q_grid_subdivide_3D(vec4D_grid) || !q_grid_subdivide_3D(vec4D_grid_ori))
			{
				printf("ERROR: q_grid_subdivide_3D() return false!\n");
				if(p_img64f_warp)	 	{delete []p_img64f_warp;			p_img64f_warp=0;}
				if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
				if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
				if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
				if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
				return false;
			}

			long sz_img_nextlevel[4]={1,1,1,1};
			for(long i=0;i<3;i++)
				sz_img_nextlevel[i]=ceil(sz_img[i]/pow(2,l_hierarchlevel-level-1-1));
			long sz_grid_nextlevel[3];
			for(long i=0;i<3;i++)
				sz_grid_nextlevel[i]=sz_img_nextlevel[i]/sz_gridwnd+4;
			vec4D_grid.resize(sz_grid_nextlevel[1]);
			vec4D_grid_ori.resize(sz_grid_nextlevel[1]);
			for(long y=0;y<sz_grid_nextlevel[1];y++)
			{
				vec4D_grid[y].resize(sz_grid_nextlevel[0]);
				vec4D_grid_ori[y].resize(sz_grid_nextlevel[0]);
			}
			for(long y=0;y<sz_grid_nextlevel[1];y++)
				for(long x=0;x<sz_grid_nextlevel[0];x++)
				{
					vec4D_grid[y][x].resize(sz_grid_nextlevel[2]);
					vec4D_grid_ori[y][x].resize(sz_grid_nextlevel[2]);
				}
			printf("\t\t>>new grid size: %d,%d,%d\n",vec4D_grid.size(),vec4D_grid[0].size(),vec4D_grid[0][0].size());
		}

		if(p_img64f_warp)	 	{delete []p_img64f_warp;			p_img64f_warp=0;}
		if(p_img64f_resize_tar) {delete []p_img64f_resize_tar;		p_img64f_resize_tar=0;}
		if(p_img64f_resize_sub)	{delete []p_img64f_resize_sub;		p_img64f_resize_sub=0;}
		if(p_img64f_embed_tar) 	{delete []p_img64f_embed_tar;		p_img64f_embed_tar=0;}
		if(p_img64f_embed_sub) 	{delete []p_img64f_embed_sub;		p_img64f_embed_sub=0;}
	}

	return true;
}

bool q_img_embed(const double *p_img64f,const long *sz_img64f,
				 const long sz_gridwnd,
				 const double d_value4invalid,
				 double *&p_img64f_embed,long *sz_img64f_embed)
{
	if(p_img64f==0 || sz_img64f==0 || sz_img64f_embed==0)
	{
		printf("ERROR: Invalid input/output image pointer or image size array pointer !\n");
		return false;
	}
	if(sz_img64f[0]<=0 || sz_img64f[1]<=0 || sz_img64f[2]<=0 || sz_img64f[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_gridwnd<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if(p_img64f_embed)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete []p_img64f_embed;		p_img64f_embed=0;
	}

	long sz_grid[3];
	for(long i=0;i<3;i++)
		sz_grid[i]=sz_img64f[i]/sz_gridwnd+4;

	sz_img64f_embed[3]=1;
	for(long i=0;i<3;i++)
		sz_img64f_embed[i]=(sz_grid[i]-3)*sz_gridwnd;

	p_img64f_embed=new double[sz_img64f_embed[0]*sz_img64f_embed[1]*sz_img64f_embed[2]]();
	if(!p_img64f_embed)
	{
		printf("ERROR: Fail to allocate memory for p_img_embed!\n");
		if(p_img64f_embed) 	{delete []p_img64f_embed;		p_img64f_embed=0;}
		return false;
	}
	long pgsz1_input=sz_img64f[0];
	long pgsz2_input=sz_img64f[0]*sz_img64f[1];
	long pgsz1_embed=sz_img64f_embed[0];
	long pgsz2_embed=sz_img64f_embed[0]*sz_img64f_embed[1];
	for(long x=0;x<sz_img64f_embed[0];x++)
		for(long y=0;y<sz_img64f_embed[1];y++)
			for(long z=0;z<sz_img64f_embed[2];z++)
			{
				long ind_embed=pgsz2_embed*z+pgsz1_embed*y+x;
				if(x>=sz_img64f[0] || y>=sz_img64f[1] || z>=sz_img64f[2])
				{
					p_img64f_embed[ind_embed]=d_value4invalid;
				}
				else
				{
					long ind_ori=pgsz2_input*z+pgsz1_input*y+x;
					p_img64f_embed[ind_embed]=p_img64f[ind_ori];
				}
			}

	return true;
}

bool q_warpimage_baseongrid(const double *p_img64f_sub,const long sz_img_sub[4],
		const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd,
		double *&p_img64f_sub_warp)
{
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
	if(sz_gridwnd<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
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

	Matrix x_bsplinebasis(pow(sz_gridwnd,3),pow(4,3));
	if(!q_ini_bsplinebasis_3D(sz_gridwnd,x_bsplinebasis))
	{
		printf("ERROR: q_ini_bsplinebasis_3D() return false!\n");
		return false;
	}

	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if(!q_bspline_grid2field_3D(vec4D_grid,sz_gridwnd,x_bsplinebasis,vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	long sz_img_embed[4]={1,1,1,1};
	sz_img_embed[1]=vec4D_grid_int.size();
	sz_img_embed[0]=vec4D_grid_int[0].size();
	sz_img_embed[2]=vec4D_grid_int[0][0].size();

	long pgsz1_input=sz_img_sub[0];
	long pgsz2_input=sz_img_sub[0]*sz_img_sub[1];
	long pgsz3_input=sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2];
	long pgsz1_embed=sz_img_embed[0];
	long pgsz2_embed=sz_img_embed[0]*sz_img_embed[1];
	p_img64f_sub_warp=new double[sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]*sz_img_sub[3]]();
	if(!p_img64f_sub_warp)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_output_sub!\n");
		return false;
	}

	double *p_img64f_sub_embed=0;
	double *p_img64f_sub_embed_warp=0;
	for(long c=0;c<sz_img_sub[3];c++)
	{
		p_img64f_sub_embed=new double[sz_img_embed[0]*sz_img_embed[1]*sz_img_embed[2]]();
		if(!p_img64f_sub_embed)
		{
			printf("ERROR: Fail to allocate memory for p_img64f_sub_embed!\n");
			if(p_img64f_sub_warp) 	{delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;}
			return false;
		}
		for(long x=0;x<sz_img_embed[0];x++)
			for(long y=0;y<sz_img_embed[1];y++)
				for(long z=0;z<sz_img_embed[2];z++)
				{
					long ind_embed=pgsz2_embed*z+pgsz1_embed*y+x;
					if(x>=sz_img_sub[0] || y>=sz_img_sub[1] || z>=sz_img_sub[2])
					{
						p_img64f_sub_embed[ind_embed]=0;
					}
					else
					{
						long ind_ori=pgsz3_input*c+pgsz2_input*z+pgsz1_input*y+x;
						p_img64f_sub_embed[ind_embed]=p_img64f_sub[ind_ori];
					}
				}

		if(!q_interpolate_3D(p_img64f_sub_embed,sz_img_embed,vec4D_grid_int,p_img64f_sub_embed_warp))
		{
			printf("ERROR: q_interpolate_3D() return false!\n");
			if(p_img64f_sub_embed) 	{delete []p_img64f_sub_embed;		p_img64f_sub_embed=0;}
			if(p_img64f_sub_warp) 	{delete []p_img64f_sub_warp;		p_img64f_sub_warp=0;}
			return false;
		}
		if(p_img64f_sub_embed) 	{delete []p_img64f_sub_embed;		p_img64f_sub_embed=0;}

		for(long x=0;x<sz_img_sub[0];x++)
			for(long y=0;y<sz_img_sub[1];y++)
				for(long z=0;z<sz_img_sub[2];z++)
				{
					long ind_embed=pgsz2_embed*z+pgsz1_embed*y+x;
					long ind_ori=pgsz3_input*c+pgsz2_input*z+pgsz1_input*y+x;
					p_img64f_sub_warp[ind_ori]=p_img64f_sub_embed_warp[ind_embed];
				}
		if(p_img64f_sub_embed_warp) 	{delete []p_img64f_sub_embed_warp;		p_img64f_sub_embed_warp=0;}
	}

	if(p_img64f_sub_embed) 			{delete []p_img64f_sub_embed;			p_img64f_sub_embed=0;}
	if(p_img64f_sub_embed_warp) 	{delete []p_img64f_sub_embed_warp;		p_img64f_sub_embed_warp=0;}

	return true;
}

bool q_ini_bsplinebasis_3D(const long n,Matrix &BxBxB)
{
	if(n<=0)
	{
		printf("ERROR: n should > 0!\n");
		return false;
	}

	Matrix B(4,4);
	B.row(1) << -1 << 3 <<-3 << 1;
	B.row(2) <<  3 <<-6 << 3 << 0;
	B.row(3) << -3 << 0 << 3 << 0;
	B.row(4) <<  1 << 4 << 1 << 0;
	B/=6.0;

	Matrix T(n,4);
	double t_step=1.0/n;
	for(long i=0;i<n;i++)
	{
		double t=t_step*i;
		for(long j=0;j<=3;j++)
			T(i+1,j+1)=pow(t,3-j);
	}

	Matrix TB=T*B;
	Matrix BxB=KP(TB,TB);
	BxBxB=KP(BxB,TB);

	return true;
}


bool q_inigrid_generator(const long sz_img[4],const long sz_gridwnd,
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_gridwnd<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	long sz_grid[3];
	for(long i=0;i<3;i++)
		sz_grid[i]=sz_img[i]/sz_gridwnd+4;

	vec4D_grid.assign(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));
	for(long x=0;x<sz_grid[0];x++)
		for(long y=0;y<sz_grid[1];y++)
			for(long z=0;z<sz_grid[2];z++)
			{
				vec4D_grid[y][x][z][0]=(x-1)*sz_gridwnd;
				vec4D_grid[y][x][z][1]=(y-1)*sz_gridwnd;
				vec4D_grid[y][x][z][2]=(z-1)*sz_gridwnd;
			}

	return true;
}


bool q_bspline_grid2field_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid_int)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(sz_gridwnd<=1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >1!\n");
		return false;
	}
	if(x_bsplinebasis.nrows()!=pow(sz_gridwnd,3) || x_bsplinebasis.ncols()!=pow(4,3))
	{
		printf("ERROR: Invalid input x_bsplinebasis size!\n");
		return false;
	}
	if(vec4D_grid_int.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid_int is not empty, original data will be cleared!\n");
		vec4D_grid_int.clear();
	}

	long sz_grid[3];
	sz_grid[1]=vec4D_grid.size();
	sz_grid[0]=vec4D_grid[0].size();
	sz_grid[2]=vec4D_grid[0][0].size();

	long sz_grid_int[3];
	for(long i=0;i<3;i++)
		sz_grid_int[i]=(sz_grid[i]-3)*sz_gridwnd;

	vec4D_grid_int.assign(sz_grid_int[1],vector< vector< vector<double> > >(sz_grid_int[0],vector< vector<double> >(sz_grid_int[2],vector<double>(3,0))));

	for(long x=0;x<sz_grid[0]-3;x++)
		for(long y=0;y<sz_grid[1]-3;y++)
			for(long z=0;z<sz_grid[2]-3;z++)
				for(long xyz=0;xyz<3;xyz++)
				{
					Matrix x1D_gridblock(4*4*4,1);
					long ind=1;
					for(long dep=z;dep<z+4;dep++)
						for(long col=x;col<x+4;col++)
							for(long row=y;row<y+4;row++)
							{
								x1D_gridblock(ind,1)=vec4D_grid[row][col][dep][xyz];
								ind++;
							}

					Matrix x1D_gridblock_int=x_bsplinebasis*x1D_gridblock;

					ind=1;
					for(long zz=0;zz<sz_gridwnd;zz++)
						for(long xx=0;xx<sz_gridwnd;xx++)
							for(long yy=0;yy<sz_gridwnd;yy++)
							{
								vec4D_grid_int[y*sz_gridwnd+yy][x*sz_gridwnd+xx][z*sz_gridwnd+zz][xyz]=x1D_gridblock_int(ind,1);
								ind++;
							}
				}

	return true;
}

bool q_bspline_field2grid_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	if(vec4D_grid_int.size()==0 || vec4D_grid_int[0].size()==0 || vec4D_grid_int[0][0].size()==0 || vec4D_grid_int[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(sz_gridwnd<=1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >1!\n");
		return false;
	}
	if(x_bsplinebasis.nrows()!=pow(sz_gridwnd,3) || x_bsplinebasis.ncols()!=pow(4,3))
	{
		printf("ERROR: Invalid input x_bsplinebasis size!\n");
		return false;
	}
	if(vec4D_grid.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	long sz_grid_int[3];
	sz_grid_int[1]=vec4D_grid_int.size();
	sz_grid_int[0]=vec4D_grid_int[0].size();
	sz_grid_int[2]=vec4D_grid_int[0][0].size();

	long sz_grid[3];
	for(long i=0;i<3;i++)
		sz_grid[i]=sz_grid_int[i]/sz_gridwnd+3;

	vec4D_grid.assign(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));

	for(long x=0;x<sz_grid[0]-3;x++)
		for(long y=0;y<sz_grid[1]-3;y++)
			for(long z=0;z<sz_grid[2]-3;z++)
				for(long xyz=0;xyz<3;xyz++)
				{
					Matrix x1D_transblock(sz_gridwnd*sz_gridwnd*sz_gridwnd,1);
					long ind=1;
					for(long zz=0;zz<sz_gridwnd;zz++)
						for(long xx=0;xx<sz_gridwnd;xx++)
							for(long yy=0;yy<sz_gridwnd;yy++)
							{
								x1D_transblock(ind,1)=vec4D_grid_int[y*sz_gridwnd+yy][x*sz_gridwnd+xx][z*sz_gridwnd+zz][xyz];
								ind++;
							}

					Matrix x1D_grid=x_bsplinebasis.t()*x1D_transblock;

					ind=1;
					for(long dep=z;dep<z+4;dep++)
						for(long col=x;col<x+4;col++)
							for(long row=y;row<y+4;row++)
							{
								vec4D_grid[row][col][dep][xyz]+=x1D_grid(ind,1);
								ind++;
							}
				}

	return true;
}


bool q_interpolate_3D(const double *p_img64f_input,const long sz_img_input[4],
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


bool q_similarity_3D(const double *p_img64f_1,const double *p_img64f_2,const long sz_img[4],
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

	long l_npixles=sz_img[0]*sz_img[1]*sz_img[2];
	for(long i=0;i<l_npixles;i++)
	{
		double d_int_1=p_img64f_1[i];
		double d_int_2=p_img64f_2[i];

		double diff=pow(d_int_1-d_int_2,2);
		d_similarity+=diff;
	}

	return true;
}


bool q_gridnodes_dT_3D(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		const long sz_gridwnd,const Matrix &x_bsplinebasis,
		vector< vector< vector< vector<double> > > > &vec4D_grid_dT)
{
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
	if(sz_gridwnd<=1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >1!\n");
		return false;
	}
	if(x_bsplinebasis.nrows()!=pow(sz_gridwnd,3) || x_bsplinebasis.ncols()!=pow(4,3))
	{
		printf("ERROR: Invalid input x_bsplinebasis size!\n");
		return false;
	}
	if(vec4D_grid_dT.size()!=0)
	{
		printf("WARNNING: Output vec4D_grid_gradient is not empty, original data will be cleared!\n");
		vec4D_grid_dT.clear();
	}

	clock_t start;

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

	if(SHOWTIME) start=clock();
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
	if(SHOWTIME) printf("\t\t\t\t>>compute dT time consume: %.2f s\n",(clock()-start)/100.0);

	if(SHOWTIME) start=clock();
	if(!q_bspline_field2grid_3D(vec4D_img_gradient,sz_gridwnd,x_bsplinebasis,vec4D_grid_dT))
	{
		printf("ERROR: q_bspline_field2grid_3D() return false!\n");
		if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}
	if(SHOWTIME) printf("\t\t\t\t>>compute dT-dM time consume: %.2f s\n",(clock()-start)/100.0);

	if(p_img64f_tar_4d)		{delete4dpointer(p_img64f_tar_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_sub_4d) 	{delete4dpointer(p_img64f_sub_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}

bool q_grid_update_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,const vector< vector< vector< vector<double> > > > &vec4D_grid_ori,
		const vector< vector< vector< vector<double> > > > &vec4D_grid_dT,
		const double d_gamma,const double d_gaussian_sigma,
		vector< vector< vector< vector<double> > > > &vec4D_grid_update)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(vec4D_grid.size()!=vec4D_grid_ori.size() || vec4D_grid[0].size()!=vec4D_grid_ori[0].size() ||
	   vec4D_grid[0][0].size()!=vec4D_grid_ori[0][0].size() || vec4D_grid[0][0][0].size()!=vec4D_grid_ori[0][0][0].size())
	{
		printf("ERROR: Input vec4D_grid.size() != Input vec4D_grid_ori.size()!\n");
		return false;
	}
	if(vec4D_grid.size()!=vec4D_grid_dT.size() || vec4D_grid[0].size()!=vec4D_grid_dT[0].size() ||
	   vec4D_grid[0][0].size()!=vec4D_grid_dT[0][0].size() || vec4D_grid[0][0][0].size()!=vec4D_grid_dT[0][0][0].size())
	{
		printf("ERROR: Input vec4D_grid.size() != Input vec4D_grid_dT.size()!\n");
		return false;
	}
	if(d_gaussian_sigma<=0)
	{
		printf("ERROR: Invalid input d_gaussian_sigma!\n");
		return false;
	}
	if(vec4D_grid_update.size())
	{
		vec4D_grid_update.clear();
	}
	vec4D_grid_update=vec4D_grid;

	long sz_grid[3];
	sz_grid[1]=vec4D_grid.size();
	sz_grid[0]=vec4D_grid[0].size();
	sz_grid[2]=vec4D_grid[0][0].size();

	vector< vector< vector< vector<double> > > > vec4D_grid_offset(vec4D_grid);
	for(long z=0;z<sz_grid[2];z++)
		for(long y=0;y<sz_grid[1];y++)
			for(long x=0;x<sz_grid[0];x++)
				for(long xyz=0;xyz<3;xyz++)
				{
					vec4D_grid_offset[y][x][z][xyz]=vec4D_grid[y][x][z][xyz]-vec4D_grid_ori[y][x][z][xyz];
					vec4D_grid_offset[y][x][z][xyz]-=d_gamma*vec4D_grid_dT[y][x][z][xyz];
				}

	vector<double> vec1D_kernel;
	if(!q_kernel_gaussian_1D(3,d_gaussian_sigma,vec1D_kernel))
	{
		printf("ERROR: q_kernel_gaussian() return false!\n");
		return false;
	}
	if(!q_convolve_vec64f_3D_fast(vec4D_grid_offset,vec1D_kernel))
	{
		printf("ERROR: q_convolve_vec64f_3D_fast() return false!\n");
		return false;
	}

	for(long z=0;z<sz_grid[2];z++)
		for(long y=0;y<sz_grid[1];y++)
			for(long x=0;x<sz_grid[0];x++)
				for(long xyz=0;xyz<3;xyz++)
					vec4D_grid_update[y][x][z][xyz]=vec4D_grid_ori[y][x][z][xyz]+vec4D_grid_offset[y][x][z][xyz];

	return true;
}

bool q_grid_subdivide_3D(vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}

	long sz_grid[3];//w,h,z
	sz_grid[1]=vec4D_grid.size();			//nrow
	sz_grid[0]=vec4D_grid[0].size();		//ncol
	sz_grid[2]=vec4D_grid[0][0].size();		//nz
	long sz_grid_sub_tmp[3];//w,h,z
	sz_grid_sub_tmp[1]=sz_grid[1]*2-1;		//nrow
	sz_grid_sub_tmp[0]=sz_grid[0]*2-1;		//ncol
	sz_grid_sub_tmp[2]=sz_grid[2]*2-1;		//nz
	long sz_grid_sub[3];//w,h,z
	sz_grid_sub[1]=sz_grid_sub_tmp[1]-2;	//nrow
	sz_grid_sub[0]=sz_grid_sub_tmp[0]-2;	//ncol
	sz_grid_sub[2]=sz_grid_sub_tmp[2]-2;	//nz

	vector< vector< vector< vector<double> > > > vec4D_grid_subdivided;
	vec4D_grid_subdivided.assign(sz_grid_sub[1],vector< vector< vector<double> > >(sz_grid_sub[0],vector< vector<double> >(sz_grid_sub[2],vector<double>(3,0.0))));

	for(long x=1;x<sz_grid_sub_tmp[0]-1;x++)
		for(long y=1;y<sz_grid_sub_tmp[1]-1;y++)
			for(long z=1;z<sz_grid_sub_tmp[2]-1;z++)
			{
				double cur_pos[3];//x,y,z
				cur_pos[0]=x/2.0;
				cur_pos[1]=y/2.0;
				cur_pos[2]=z/2.0;

				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

				double l_w,r_w,t_w,b_w;
				l_w=r_w=t_w=b_w=0.5;
				double u_w,d_w;
				u_w=d_w=0.5;

				for(long xyz=0;xyz<3;xyz++)
				{
					double higher_slice;
					higher_slice=t_w*(l_w*vec4D_grid[y_s][x_s][z_s][xyz]+r_w*vec4D_grid[y_s][x_b][z_s][xyz])+
								 b_w*(l_w*vec4D_grid[y_b][x_s][z_s][xyz]+r_w*vec4D_grid[y_b][x_b][z_s][xyz]);
					double lower_slice;
					lower_slice =t_w*(l_w*vec4D_grid[y_s][x_s][z_b][xyz]+r_w*vec4D_grid[y_s][x_b][z_b][xyz])+
								 b_w*(l_w*vec4D_grid[y_b][x_s][z_b][xyz]+r_w*vec4D_grid[y_b][x_b][z_b][xyz]);
					vec4D_grid_subdivided[y-1][x-1][z-1][xyz]=u_w*higher_slice+d_w*lower_slice;
					vec4D_grid_subdivided[y-1][x-1][z-1][xyz]=vec4D_grid_subdivided[y-1][x-1][z-1][xyz]*2;
				}
			}

	vec4D_grid.clear();
	vec4D_grid=vec4D_grid_subdivided;

	return true;
}

bool q_save64f01_image(const double *p_img64f,const long sz_img[4],const char *filename)
{
	if(p_img64f==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(!filename)
	{
		printf("ERROR: Invalid output image filename pointer!\n");
		return false;
	}

	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

	unsigned char *p_img4save=0;
	p_img4save=new unsigned char[l_npixels]();
	if(!p_img4save)
	{
		printf("ERROR: Fail to allocate memory for image.\n");
		return false;
	}

	for(long i=0;i<l_npixels;i++)
	{
		double tmp=p_img64f[i]*255.0;
		tmp=tmp<0?0:tmp;	tmp=tmp>255?255:tmp;
		p_img4save[i]=tmp+0.5;
	}

	saveImage(filename,p_img4save,sz_img,1);

	if(p_img4save) 	{delete []p_img4save;		p_img4save=0;}

	return true;
}

bool q_savegrid_apo(const vector< vector< vector< vector<double> > > > &vec4D_grid,const double d_volsize,
		const char *filename)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()==0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(d_volsize<=0)
	{
		printf("ERROR: Invalid input d_volsize!\n");
		return false;
	}
	if(!filename)
	{
		printf("ERROR: Invalid output filename!\n");
		return false;
	}

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
				fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,0,255\n",n,n,vec4D_grid[y][x][z][2],vec4D_grid[y][x][z][0],vec4D_grid[y][x][z][1],d_volsize);
				n++;
			}

	fclose(fp);

	return true;
}

bool q_savegrid_swc(const vector< vector< vector< vector<double> > > > &vec4D_grid,const long sz_gridwnd,
		const char *filename)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()==0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if(sz_gridwnd<=0)
	{
		printf("ERROR: Invalid input sz_gridwnd!\n");
		return false;
	}
	if(!filename)
	{
		printf("ERROR: Invalid output filename!\n");
		return false;
	}

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
				fprintf(fp,"%ld 1 %ld %ld %ld 1 %ld\n",2*n+1,(x-1)*sz_gridwnd, (y-1)*sz_gridwnd, (z-1)*sz_gridwnd, 2*n+2);
				fprintf(fp,"%ld 2 %.2f %.2f %.2f 1 -1\n",2*n+2,vec4D_grid[y][x][z][0], vec4D_grid[y][x][z][1],vec4D_grid[y][x][z][2]);
				n++;
			}

	fclose(fp);

	return true;
}
