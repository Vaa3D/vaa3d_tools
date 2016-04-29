// q_convolve.cpp
// by Lei Qu
// 2010-02-24

#include "q_convolve.h"
#include <math.h>
#include <basic_memory.cpp>


bool q_kernel_gaussian_1D(const long l_radius,const double sigma,
		vector<double> &vec1D_kernel)
{
	//check paras
	if(l_radius<0)
	{
		printf("ERROR: Invalid input kernel radius!\n");
		return false;
	}
	if(sigma<=0)
	{
		printf("ERROR: Invalid input sigma!\n");
		return false;
	}
	if(vec1D_kernel.size()!=0)
	{
		printf("WARNNING: Output vec1D_kernel is not empty, original data will be cleared!\n");
		vec1D_kernel.clear();
	}

	long sz_kernel=l_radius*2+1;

	vec1D_kernel.assign(sz_kernel,0.0);
	double d_kernel_sum=0;
	long x;
	for(long xx=-l_radius;xx<=l_radius;xx++)
	{
		x=xx+l_radius;

		vec1D_kernel[x]=exp(-xx*xx/(2*sigma*sigma));
		d_kernel_sum+=vec1D_kernel[x];
	}

	for(long xx=-l_radius;xx<=l_radius;xx++)
	{
		x=xx+l_radius;
		vec1D_kernel[x]/=d_kernel_sum;
	}

	return true;
}

bool q_kernel_gaussian(const long l_radius_x,const long l_radius_y,const long l_radius_z,const double sigma,
		vector< vector< vector<double> > > &vec3D_kernel)
{
	if(l_radius_x<0 || l_radius_y<0 || l_radius_z<0)
	{
		printf("ERROR: Invalid input kernel radius!\n");
		return false;
	}
	if(sigma<=0)
	{
		printf("ERROR: Invalid input sigma!\n");
		return false;
	}
	if(vec3D_kernel.size()!=0)
	{
		printf("WARNNING: Output vec3D_kernel is not empty, original data will be cleared!\n");
		vec3D_kernel.clear();
	}

	long sz_kernel[3];//w,h,z
	sz_kernel[0]=l_radius_x*2+1;
	sz_kernel[1]=l_radius_y*2+1;
	sz_kernel[2]=l_radius_z*2+1;

	vec3D_kernel.assign(sz_kernel[1],vector< vector<double> >(sz_kernel[0],vector<double>(sz_kernel[2],0.0)));
	double d_kernel_sum=0;
	long x,y,z;
	for(long xx=-l_radius_x;xx<=l_radius_x;xx++)
	{
		x=xx+l_radius_x;
		for(long yy=-l_radius_y;yy<=l_radius_y;yy++)
		{
			y=yy+l_radius_y;
			for(long zz=-l_radius_z;zz<=l_radius_z;zz++)
			{
				z=zz+l_radius_z;
				vec3D_kernel[y][x][z]=exp(-(xx*xx+yy*yy+zz*zz)/(2*sigma*sigma));
				d_kernel_sum+=vec3D_kernel[y][x][z];
			}
		}
	}

	for(long xx=-l_radius_x;xx<=l_radius_x;xx++)
	{
		x=xx+l_radius_x;
		for(long yy=-l_radius_y;yy<=l_radius_y;yy++)
		{
			y=yy+l_radius_y;
			for(long zz=-l_radius_z;zz<=l_radius_z;zz++)
			{
				z=zz+l_radius_z;
				vec3D_kernel[y][x][z]/=d_kernel_sum;
			}
		}
	}

	return true;
}


bool q_convolve_img64f_3D(double *&p_img64f,const long sz_img[4],
		const vector< vector< vector<double> > > &vec3D_kernel)
{
	//check paras
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
	if(vec3D_kernel.size()==0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long sz_kernel[3];//w,h,z
	sz_kernel[0]=vec3D_kernel[0].size();
	sz_kernel[1]=vec3D_kernel.size();
	sz_kernel[2]=vec3D_kernel[0][0].size();
	long radius_x,radius_y,radius_z;
	radius_x=(sz_kernel[0]-1)/2;
	radius_y=(sz_kernel[1]-1)/2;
	radius_z=(sz_kernel[2]-1)/2;

	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

	double *p_img64f_tmp=0;
	p_img64f_tmp=new double[l_npixels]();
	if(!p_img64f_tmp)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}

	double ****p_img64f_4d=0,****p_img64f_tmp_4d=0;
	if(!new4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f) ||
	   !new4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_tmp))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_tmp) 		{delete []p_img64f_tmp;		p_img64f_tmp=0;}
		if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_tmp_4d) 	{delete4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	for(long Z=0;Z<sz_img[2];Z++)
		for(long Y=0;Y<sz_img[1];Y++)
			for(long X=0;X<sz_img[0];X++)
			{
				long x_k,y_k,z_k;
				long x_i,y_i,z_i;
				for(long xx=-radius_x;xx<=radius_x;xx++)
				{
					x_k=xx+radius_x;
					x_i=X+xx;
					x_i = x_i<0 ? 0:x_i;		x_i = x_i>=sz_img[0] ? sz_img[0]-1:x_i;
					for(long yy=-radius_y;yy<=radius_y;yy++)
					{
						y_k=yy+radius_y;
						y_i=Y+yy;
						y_i = y_i<0 ? 0:y_i;		y_i = y_i>=sz_img[1] ? sz_img[1]-1:y_i;
						for(long zz=-radius_z;zz<=radius_z;zz++)
						{
							z_k=zz+radius_z;
							z_i=Z+zz;
							z_i = z_i<0 ? 0:z_i;		z_i = z_i>=sz_img[2] ? sz_img[2]-1:z_i;

							for(long C=0;C<sz_img[3];C++)
								p_img64f_tmp_4d[C][Z][Y][X]+=p_img64f_4d[C][z_i][y_i][x_i]*vec3D_kernel[y_k][x_k][z_k];
						}
					}
				}

			}

	if(p_img64f) 		{delete []p_img64f;		p_img64f=0;}
	p_img64f=p_img64f_tmp;

	if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_tmp_4d) 	{delete4dpointer(p_img64f_tmp_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}

bool q_convolve_vec64f_3D(vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector< vector< vector<double> > > &vec3D_kernel)
{
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input vec4D!\n");
		return false;
	}
	if(vec3D_kernel.size()==0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long sz_grid[4];//w,h,z
	sz_grid[1]=vec4D_grid.size();
	sz_grid[0]=vec4D_grid[0].size();
	sz_grid[2]=vec4D_grid[0][0].size();
	sz_grid[3]=vec4D_grid[0][0][0].size();
	long sz_kernel[3];//w,h,z
	sz_kernel[0]=vec3D_kernel[0].size();
	sz_kernel[1]=vec3D_kernel.size();
	sz_kernel[2]=vec3D_kernel[0][0].size();
	long radius_x,radius_y,radius_z;
	radius_x=(sz_kernel[0]-1)/2;
	radius_y=(sz_kernel[1]-1)/2;
	radius_z=(sz_kernel[2]-1)/2;

	vector< vector< vector< vector<double> > > > vec4D_grid_tmp(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0.0))));

	for(long Z=0;Z<sz_grid[2];Z++)
		for(long Y=0;Y<sz_grid[1];Y++)
			for(long X=0;X<sz_grid[0];X++)
			{
				long x_k,y_k,z_k;
				long x_i,y_i,z_i;
				for(long xx=-radius_x;xx<=radius_x;xx++)
				{
					x_k=xx+radius_x;
					x_i=X+xx;
					x_i = x_i<0 ? 0:x_i;		x_i = x_i>=sz_grid[0] ? sz_grid[0]-1:x_i;
					for(long yy=-radius_y;yy<=radius_y;yy++)
					{
						y_k=yy+radius_y;
						y_i=Y+yy;
						y_i = y_i<0 ? 0:y_i;		y_i = y_i>=sz_grid[1] ? sz_grid[1]-1:y_i;
						for(long zz=-radius_z;zz<=radius_z;zz++)
						{
							z_k=zz+radius_z;
							z_i=Z+zz;
							z_i = z_i<0 ? 0:z_i;		z_i = z_i>=sz_grid[2] ? sz_grid[2]-1:z_i;

							for(long C=0;C<sz_grid[3];C++)
								vec4D_grid_tmp[Y][X][Z][C]+=vec4D_grid[Y][X][Z][C]*vec3D_kernel[y_k][x_k][z_k];
						}
					}
				}

			}

	for(long Z=0;Z<sz_grid[2];Z++)
		for(long Y=0;Y<sz_grid[1];Y++)
			for(long X=0;X<sz_grid[0];X++)
				for(long C=0;C<sz_grid[3];C++)
					vec4D_grid[Y][X][Z][C]=vec4D_grid_tmp[Y][X][Z][C];

	return true;
}

bool q_convolve_img64f_3D_fast(double *&p_img64f,const long sz_img[4],
		const vector<double> &vec1D_kernel)
{
	//check paras
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
	if(vec1D_kernel.size()==0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long radius=(vec1D_kernel.size()-1)/2;

	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

	double *p_img64f_tmp1=0,*p_img64f_tmp2=0;
	p_img64f_tmp1=new double[l_npixels]();
	p_img64f_tmp2=new double[l_npixels]();
	if(!p_img64f_tmp1 || !p_img64f_tmp2)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}

	double ****p_img64f_4d=0,****p_img64f_tmp1_4d=0,****p_img64f_tmp2_4d=0;
	if(!new4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f) ||
	   !new4dpointer(p_img64f_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_tmp1) ||
	   !new4dpointer(p_img64f_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img64f_tmp2))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img64f_tmp1) 		{delete []p_img64f_tmp1;		p_img64f_tmp1=0;}
		if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_tmp1_4d) 	{delete4dpointer(p_img64f_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img64f_tmp2_4d) 	{delete4dpointer(p_img64f_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	for(long X=0;X<sz_img[0];X++)
	{
		long x_k;
		long x_i;
		for(long xx=-radius;xx<=radius;xx++)
		{
			x_k=xx+radius;
			x_i=X+xx;
			x_i = x_i<0 ? 0:x_i;		x_i = x_i>=sz_img[0] ? sz_img[0]-1:x_i;

			for(long Z=0;Z<sz_img[2];Z++)
				for(long Y=0;Y<sz_img[1];Y++)
					for(long C=0;C<sz_img[3];C++)
						p_img64f_tmp1_4d[C][Z][Y][X]+=p_img64f_4d[C][Z][Y][x_i]*vec1D_kernel[x_k];
		}
	}
	for(long Y=0;Y<sz_img[1];Y++)
	{
		long y_k;
		long y_i;
		for(long yy=-radius;yy<=radius;yy++)
		{
			y_k=yy+radius;
			y_i=Y+yy;
			y_i = y_i<0 ? 0:y_i;		y_i = y_i>=sz_img[1] ? sz_img[1]-1:y_i;

			for(long Z=0;Z<sz_img[2];Z++)
				for(long X=0;X<sz_img[0];X++)
					for(long C=0;C<sz_img[3];C++)
						p_img64f_tmp2_4d[C][Z][Y][X]+=p_img64f_tmp1_4d[C][Z][y_i][X]*vec1D_kernel[y_k];
		}
	}
	for(long i=0;i<l_npixels;i++) p_img64f_tmp1[i]=0.0;
	for(long Z=0;Z<sz_img[2];Z++)
	{
		long z_k;
		long z_i;
		for(long zz=-radius;zz<=radius;zz++)
		{
			z_k=zz+radius;
			z_i=Z+zz;
			z_i = z_i<0 ? 0:z_i;		z_i = z_i>=sz_img[2] ? sz_img[2]-1:z_i;

			for(long Y=0;Y<sz_img[1];Y++)
				for(long X=0;X<sz_img[0];X++)
					for(long C=0;C<sz_img[3];C++)
						p_img64f_tmp1_4d[C][Z][Y][X]+=p_img64f_tmp2_4d[C][z_i][Y][X]*vec1D_kernel[z_k];
		}
	}
						
	if(p_img64f) 		{delete []p_img64f;		p_img64f=0;}
	p_img64f=p_img64f_tmp1;
	p_img64f_tmp1=0;

	if(p_img64f_tmp1) 		{delete []p_img64f_tmp1;		p_img64f_tmp1=0;}
	if(p_img64f_tmp2) 		{delete []p_img64f_tmp2;		p_img64f_tmp2=0;}
	if(p_img64f_4d)			{delete4dpointer(p_img64f_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_tmp1_4d) 	{delete4dpointer(p_img64f_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img64f_tmp2_4d) 	{delete4dpointer(p_img64f_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}

bool q_convolve_vec64f_3D_fast(vector< vector< vector< vector<double> > > > &vec4D_grid,
		const vector<double> &vec1D_kernel)
{
	//check paras
	if(vec4D_grid.size()==0 || vec4D_grid[0].size()==0 || vec4D_grid[0][0].size()==0 || vec4D_grid[0][0][0].size()!=3)
	{
		printf("ERROR: Invalid input vec4D!\n");
		return false;
	}
	if(vec1D_kernel.size()==0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long sz_grid[4];//w,h,z
	sz_grid[1]=vec4D_grid.size();
	sz_grid[0]=vec4D_grid[0].size();
	sz_grid[2]=vec4D_grid[0][0].size();
	sz_grid[3]=vec4D_grid[0][0][0].size();

	long radius_kernel=(vec1D_kernel.size()-1)/2;

	vector< vector< vector< vector<double> > > > vec4D_grid_tmp1(sz_grid[1],vector< vector< vector<double> > >(sz_grid[0],vector< vector<double> >(sz_grid[2],vector<double>(3,0.0))));
	vector< vector< vector< vector<double> > > > vec4D_grid_tmp2(vec4D_grid_tmp1);
	vector< vector< vector< vector<double> > > > vec4D_grid_tmp3(vec4D_grid_tmp1);

	for(long X=0;X<sz_grid[0];X++)
	{
		long x_k;
		long x_i;
		for(long xx=-radius_kernel;xx<=radius_kernel;xx++)
		{
			x_k=xx+radius_kernel;
			x_i=X+xx;
			x_i = x_i<0 ? 0:x_i;		x_i = x_i>=sz_grid[0] ? sz_grid[0]-1:x_i;

			for(long Z=0;Z<sz_grid[2];Z++)
				for(long Y=0;Y<sz_grid[1];Y++)
					for(long C=0;C<sz_grid[3];C++)
						vec4D_grid_tmp1[Y][X][Z][C]+=vec4D_grid[Y][X][Z][C]*vec1D_kernel[x_k];
		}
	}
	for(long Y=0;Y<sz_grid[1];Y++)
	{
		long y_k;
		long y_i;
		for(long yy=-radius_kernel;yy<=radius_kernel;yy++)
		{
			y_k=yy+radius_kernel;
			y_i=Y+yy;
			y_i = y_i<0 ? 0:y_i;		y_i = y_i>=sz_grid[1] ? sz_grid[1]-1:y_i;

			for(long Z=0;Z<sz_grid[2];Z++)
				for(long X=0;X<sz_grid[0];X++)
					for(long C=0;C<sz_grid[3];C++)
						vec4D_grid_tmp2[Y][X][Z][C]+=vec4D_grid_tmp1[Y][X][Z][C]*vec1D_kernel[y_k];
		}
	}
	vec4D_grid_tmp1.clear();
	for(long Z=0;Z<sz_grid[2];Z++)
	{
		long z_k;
		long z_i;
		for(long zz=-radius_kernel;zz<=radius_kernel;zz++)
		{
			z_k=zz+radius_kernel;
			z_i=Z+zz;
			z_i = z_i<0 ? 0:z_i;		z_i = z_i>=sz_grid[2] ? sz_grid[2]-1:z_i;

			for(long Y=0;Y<sz_grid[1];Y++)
				for(long X=0;X<sz_grid[0];X++)
					for(long C=0;C<sz_grid[3];C++)
						vec4D_grid_tmp3[Y][X][Z][C]+=vec4D_grid_tmp2[Y][X][Z][C]*vec1D_kernel[z_k];
		}
	}
	vec4D_grid_tmp2.clear();

	for(long Z=0;Z<sz_grid[2];Z++)
		for(long Y=0;Y<sz_grid[1];Y++)
			for(long X=0;X<sz_grid[0];X++)
				for(long C=0;C<sz_grid[3];C++)
					vec4D_grid[Y][X][Z][C]=vec4D_grid_tmp3[Y][X][Z][C];
	vec4D_grid_tmp3.clear();

	return true;
}
