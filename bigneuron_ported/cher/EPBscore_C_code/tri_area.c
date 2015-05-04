/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "maset.h"
#include "min_surf.h"
#include "min_surf_prototype.h"


float	get_triangulated_area(grain_chain *loop, int x0, int y0, int z0,
							  int nx, int nxy, int use_all_vox,
							  int use_odd_vox, int do_adj)
{
	float		area;

	if( use_all_vox ) area = use_every_vox_area(loop,nx,nxy,x0,y0,z0,do_adj);
	else		  area = use_every_other_vox_area(loop,nx,nxy,x0,y0,z0,
							   use_odd_vox,do_adj); 
	return area;
}


float	use_every_vox_area(grain_chain *loop, int nx, int nxy, int x0,
						   int y0, int z0, int do_adj)
{
	grain_chain	*p_loop;

	float	f_x1, f_y1, f_z1;
	float	area;

	int	ip, strt_ip;
	int	x1, y1, z1;
  
	strt_ip = loop->index_in_cube;
	Ind_2_ijk(strt_ip,x1,y1,z1,nx,nxy)
  
	x1 -= x0;	y1 -= y0;	z1 -= z0;

	if( do_adj ) get_adjusted_vertex(x1,y1,z1,&f_x1,&f_y1,&f_z1);
	
	area = 0;
	p_loop = loop->next;
	while( p_loop )
	{
		ip = p_loop->index_in_cube;

		if( do_adj ) area += adj_triangle_area(x0,y0,z0,ip,nx,nxy,
							&f_x1,&f_y1,&f_z1);
		else	     area += triangle_area(x0,y0,z0,ip,nx,nxy,
							&x1,&y1,&z1);
		p_loop = p_loop->next;
	}

	if( do_adj ) area += adj_triangle_area(x0,y0,z0,strt_ip,nx,nxy,
							&f_x1,&f_y1,&f_z1);
	else	     area += triangle_area(x0,y0,z0,strt_ip,nx,nxy,&x1,&y1,&z1);

	return area;
}


float	use_every_other_vox_area(grain_chain *loop, int nx, int nxy, int x0,
						   int y0, int z0, int odd_or_even, int do_adj)
{
	grain_chain	*p_loop;

	float	f_x1, f_y1, f_z1;
	float	area;

	int	i;
	int	ip, strt_ip, strt_ip1;
	int	x1, y1, z1;
	int	start, end, num_in_perim;
	int	use_it;

	num_in_perim = 0;
	
	p_loop = loop;
	while( p_loop )
	{
		num_in_perim += 1;
		p_loop = p_loop->next;
	}

	strt_ip = loop->index_in_cube;
	Ind_2_ijk(strt_ip,x1,y1,z1,nx,nxy)
	start = 2;
	end = num_in_perim -1;
	use_it = 0;

	p_loop = loop->next;
	
	if( (num_in_perim % 2 == 0) && (odd_or_even != 1) )
	{
		strt_ip = p_loop->index_in_cube;
		Ind_2_ijk(strt_ip,x1,y1,z1,nx,nxy)
		start = 3;
		end = num_in_perim;
		p_loop = p_loop->next;
	}
	else if( (num_in_perim%2 == 1) && (odd_or_even == 1) )
	{
		end = num_in_perim;
	}
	else if( (num_in_perim%2 == 1) && (odd_or_even != 1) )
	{
		strt_ip1 = p_loop->index_in_cube;
		Ind_2_ijk(strt_ip1,x1,y1,z1,nx,nxy)
		start = 3;
		end = num_in_perim-1;
		p_loop = p_loop->next;
	}
	
	x1 -= x0;	y1 -= y0;	z1 -= z0;

	if( do_adj ) get_adjusted_vertex(x1,y1,z1,&f_x1,&f_y1,&f_z1);

	area = 0;
	for( i = start;  i <= end;  i++ )
	{
		ip = p_loop->index_in_cube;

		if( use_it == 1 )
		{
			if( do_adj ) area += adj_triangle_area(x0,y0,z0,ip,nx,
							nxy,&f_x1,&f_y1,&f_z1);	
			else	     area += triangle_area(x0,y0,z0,ip,nx,nxy,
							&x1,&y1,&z1);
			use_it = 0;
		}
		else
		{
			use_it = 1;
		}
		p_loop = p_loop->next;
	}

	if( do_adj ) area += adj_triangle_area(x0,y0,z0,strt_ip,nx,nxy,
							&f_x1,&f_y1,&f_z1);	
	else	     area += triangle_area(x0,y0,z0,strt_ip,nx,nxy,&x1,&y1,&z1);

	if( (num_in_perim%2 == 1) && (odd_or_even != 1) )
	{
		if( do_adj ) area += adj_triangle_area(x0,y0,z0,strt_ip1,nx,nxy,
							&f_x1,&f_y1,&f_z1);	
		else	     area += triangle_area(x0,y0,z0,strt_ip1,nx,nxy,
							&x1,&y1,&z1);
	}
	return area;
}


float	adj_triangle_area(int x0, int y0, int z0, int ind, int nx, int nxy,
						  float *x1, float *y1, float *z1)
{
	float	x2, y2, z2, f_x, f_y, f_z;
	float	area;

	int	x, y, z;

	Ind_2_ijk(ind,x,y,z,nx,nxy);

	x = x - x0;	y = y - y0;	z = z - z0;

	get_adjusted_vertex(x,y,z,&x2,&y2,&z2);

	f_x = (*y1)*z2 - y2*(*z1);
	f_y = (*z1)*x2 - z2*(*x1);
	f_z = (*x1)*y2 - x2*(*y1);

	area = (float) (f_x*f_x + f_y*f_y + f_z*f_z);	area = (float)sqrt(area);

	*x1 = x2;	*y1 = y2;	*z1 = z2;
	return area;
}


void	get_adjusted_vertex(int x1, int y1, int z1, float *f_x1, 
							float *f_y1, float *f_z1)
{
	int	a_x1, a_y1, a_z1;

	a_x1 = abs(x1);
	a_y1 = abs(y1);
	a_z1 = abs(z1);
	
	if( (a_x1 != a_y1) && (a_x1 != a_z1) && (a_y1 != a_z1) )
	{
		if( (a_x1 > a_y1) && (a_x1 > a_z1) )
		{
			*f_x1 = (x1 > 0) ? (float)(x1-0.5) : (float)(x1+0.5);
			*f_y1 = (float)y1;
			*f_z1 = (float)z1;
		}
		else if( (a_y1 > a_x1) && (a_y1 > a_z1) )
		{
			*f_x1 = (float)x1;
			*f_y1 = (y1 > 0) ? (float)(y1-0.5) : (float)(y1+0.5);
			*f_z1 = (float)z1;
		}
		else if( (a_z1 > a_x1) && (a_z1 > a_y1) )
		{
			*f_x1 = (float)x1;
			*f_y1 = (float)y1;
			*f_z1 = (z1 > 0) ? (float)(z1-0.5) : (float)(z1+0.5);
		}
	}
	else if( (a_x1 == a_y1) && (a_y1 == a_z1) )
	{
		*f_x1 = (x1 > 0) ? (float)(x1-0.5) : (float)(x1+0.5);
		*f_y1 = (y1 > 0) ? (float)(y1-0.5) : (float)(y1+0.5);
		*f_z1 = (z1 > 0) ? (float)(z1-0.5) : (float)(z1+0.5);
	}
	else if( a_x1 == a_y1 )
	{
		if(  a_x1 > a_z1 )
		{
			*f_x1 = (x1 > 0) ? (float)(x1-0.5) : (float)(x1+0.5);
			*f_y1 = (y1 > 0) ? (float)(y1-0.5) : (float)(y1+0.5);
			*f_z1 = (float)z1;
		}
		else
		{
			*f_x1 = (float)x1;
			*f_y1 = (float)y1;
			*f_z1 = (z1 > 0) ? (float)(z1-0.5) : (float)(z1+0.5);
		}
	}
	else if( a_x1 == a_z1 )
	{
		if( a_x1 > a_y1 )
		{
			*f_x1 = (x1 > 0) ? (float)(x1-0.5) : (float)(x1+0.5);
			*f_y1 = (float)y1;
			*f_z1 = (z1 > 0) ? (float)(z1-0.5) : (float)(z1+0.5);
		}
		else
		{
			*f_x1 = (float)x1;
			*f_y1 = (y1 > 0) ? (float)(y1-0.5) : (float)(y1+0.5);
			*f_z1 = (float)z1;
		}
	}
	else if ( a_y1 == a_z1 )
	{
		if( a_y1 > a_x1 )
		{
			*f_x1 = (float)x1;
			*f_y1 = (y1 > 0) ? (float)(y1-0.5) : (float)(y1+0.5);
			*f_z1 = (z1 > 0) ? (float)(z1-0.5) : (float)(z1+0.5);
		}
		else
		{
			*f_x1 = (x1 > 0) ? (float)(x1-0.5) : (float)(x1+0.5);
			*f_y1 = (float)y1;
			*f_z1 = (float)z1;
		}
	}
}


float	triangle_area(int x0, int y0, int z0, int ind, int nx, int nxy,
					  int *x1, int *y1, int *z1)
{
	int	x, y, z, x2, y2, z2;
	float	area;

	Ind_2_ijk(ind,x,y,z,nx,nxy)

	x2 = x - x0;	y2 = y - y0;	z2 = z - z0;

	x = (*y1)*z2 - y2*(*z1);
	y = (*z1)*x2 - z2*(*x1);
	z = (*x1)*y2 - x2*(*y1);

	area = (float) (x*x + y*y + z*z);	area = (float)sqrt(area);

	*x1 = x2;	*y1 = y2;	*z1 = z2;
	return area;
}


float	triangle_area1(float x0, float y0, float z0, float x1, float y1,
					   float z1, float x2, float y2, float z2)
{
	float	x3, y3, z3, x4, y4, z4, x, y, z;
	float	area;

	x3 = x1 - x0;	y3 = y1 - y0;	z3 = z1 - z0;
	x4 = x2 - x0;	y4 = y2 - y0;	z4 = z2 - z0;
	
	x = (y3)*z4 - y4*(z3);
	y = (z3)*x4 - z4*(x3);
	z = (x3)*y4 - x4*(y3);

	area = (float) (x*x + y*y + z*z);	area = (float)sqrt(area);

	area = (float)(area/2.0);
	return area;
}
