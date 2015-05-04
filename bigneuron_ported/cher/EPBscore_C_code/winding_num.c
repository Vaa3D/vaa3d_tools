/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "min_surf.h"
#include "maset.h"
#include "isoseg.h"
#include "min_surf_prototype.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int	calc_winding_number(grain_chain *loop, MoI *moi, int nx, int nxy,
						char two_d)
{
	static	float	t_pi = (float)(2.0*M_PI);

	grain_chain	*ptr_loop;

	float	pos_v[3], prev_v[3];

	float	x0, y0, z0, theta, d_theta;
	float	tol;

	int	ind0, tmp1, tmp2, tmp3;
	int	wind_num = 0;

	tol = (float)0.00001;

	theta = 0.0;

	x0 = (float)(moi->comx);
	y0 = (float)(moi->comy);
	z0 = (float)(moi->comz);

	ptr_loop = loop;

	ind0 = ptr_loop->index_in_cube;
	Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
	prev_v[0] = (float)tmp1 - x0;
	prev_v[1] = (float)tmp2 - y0;
	prev_v[2] = (float)tmp3 - z0;

	project_2_plane(prev_v,&(moi->pa[3]),&(moi->pa[6]));
	
	ptr_loop = ptr_loop->next;
	while( ptr_loop )
	{
		ind0 = ptr_loop->index_in_cube;
		Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
		pos_v[0] = (float)tmp1 - x0;
		pos_v[1] = (float)tmp2 - y0;
		pos_v[2] = (float)tmp3 - z0;			

		project_2_plane(pos_v,&(moi->pa[3]),&(moi->pa[6]));

		d_theta = calc_d_theta(prev_v,pos_v,&(moi->pa[0]),two_d);
		if( (d_theta >= 99999.0) && (d_theta <= 100001.0) ) return 0;

		theta += d_theta;

		prev_v[0] = pos_v[0];
		prev_v[1] = pos_v[1];
		prev_v[2] = pos_v[2];

		ptr_loop = ptr_loop->next;
	}

	ind0 = loop->index_in_cube;
	Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
	pos_v[0] = (float)tmp1 - x0;
	pos_v[1] = (float)tmp2 - y0;
	pos_v[2] = (float)tmp3 - z0;
	
	project_2_plane(pos_v,&(moi->pa[3]),&(moi->pa[6]));
	
	d_theta = calc_d_theta(prev_v,pos_v,&(moi->pa[0]),two_d);
	if( (d_theta >= 99999.0) && (d_theta <= 100001.0) ) return 0;

	theta += d_theta;	theta = (float)fabs(theta);

	if( fabs(theta-t_pi) < tol ) return 1;
	else			     return 0;
}


void	cross_product(float prev_v[3], float new_v[3])
{
	float	tmp[3];
	
	tmp[0] = prev_v[1]*new_v[2] - prev_v[2]*new_v[1];
	tmp[1] = prev_v[2]*new_v[0] - prev_v[0]*new_v[2];
	tmp[2] = prev_v[0]*new_v[1] - prev_v[1]*new_v[0];
	
	prev_v[0] = tmp[0];
	prev_v[1] = tmp[1];
	prev_v[2] = tmp[2];
}


#ifdef TST
float	get_d_theta(int ind0, int ind1, MoI *moi, int nx, int nxy)
{
	int	tmp1, tmp2, tmp3;
	float	v0[3], v1[3];
	float	d_theta;
	
	Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
	v0[0] = (float)tmp1 - (float)(moi->comx);
	v0[1] = (float)tmp2 - (float)(moi->comy);
	v0[2] = (float)tmp3 - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));
	
	Pad_ind_2_ijk(ind1,tmp1,tmp2,tmp3,nx,nxy)
	v1[0] = (float)tmp1 - (float)(moi->comx);
	v1[1] = (float)tmp2 - (float)(moi->comy);
	v1[2] = (float)tmp3 - (float)(moi->comz);
	
	project_2_plane(v1,&(moi->pa[3]),&(moi->pa[6]));

	d_theta = calc_d_theta(v0,v1,&(moi->pa[0]));
	return d_theta;
}
#endif


float	calc_d_theta(float prev_v[3], float pos_v[3], float *normal,
					 char two_d)
{
	float	tmp[3];
	float	pi = (float)M_PI;
	float	d_theta, p_norm, cu_norm, dp;
	double	argument;

	int	i, dir;

	tmp[0] = prev_v[0];
	tmp[1] = prev_v[1];
	tmp[2] = prev_v[2];

	cross_product(tmp,pos_v);
	dir = -1;
	
	for( i = 0;  i < 3;  i++ )
	{
		    /* if( tmp[i] != 0 ) */
		if( fabs(normal[i]) > 0.00001 ) { dir = i;	break; }
	}

	if( dir == -1 )
	{
		fprintf(stderr,"Error normal[0] %f, "		 ,normal[0]);
		fprintf(stderr,"normal[1] %f, "			 ,normal[1]);
		fprintf(stderr,"normal[2] %f in calc_d_theta()\n",normal[2]);

		fprintf(stdout,"Error normal[0] %f, "		 ,normal[0]);
		fprintf(stdout,"normal[1] %f, "			 ,normal[1]);
		fprintf(stdout,"normal[2] %f in calc_d_theta()\n",normal[2]);

		clean_up(0);
		
	    /* return 0; */
	}

	find_norms(prev_v,pos_v,&p_norm,&cu_norm);

	if( p_norm < 0.000001  || cu_norm < 0.000001  )
	{
		if( two_d == 'n' )
		{
			fprintf(stderr,"Error p_norm %f, "	       , p_norm);
			fprintf(stderr,"cu_norm %f in calc_d_theta()\n",cu_norm);
			fprintf(stdout,"Error p_norm %f, "	       , p_norm);
			fprintf(stdout,"cu_norm %f in calc_d_theta()\n",cu_norm);
			clean_up(0);
		}
		else
		{
			return 100000.0;
		}
	}
	
	argument = (double)(tmp[dir]/(p_norm*cu_norm*normal[dir]));
	if( fabs(argument) >1.0 )
	{
		if( (fabs(argument) - 1.0) > 0.00001 )
		{
			fprintf(stderr,"Logic error in calc_d_theta()\n");
			fprintf(stdout,"Logic error in calc_d_theta()\n");
			clean_up(0);
		}
		else if (argument < 0.0) argument = -1.0;
		else			 argument = 1.0;
	}
	d_theta = (float)asin(argument);
	
	dp  = dot_product(prev_v,pos_v);

	if( dp < 0.0 )
	{
		     if( d_theta > 0.0 ) d_theta =  pi - d_theta;
		else if( d_theta < 0.0 ) d_theta = -pi - d_theta;
	}
	return d_theta;
}


void	find_norms(float prev_v[3], float pos_v[3], float *p_norm,
				   float *cu_norm)
{
	*p_norm = get_norm(prev_v);
	*cu_norm = get_norm(pos_v);
}


float	get_norm(float v[3])
{
	float norm;

	norm = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	norm = (float)sqrt(norm);

	return norm;
}


void	project_2_plane(float new_v[3], float *basis_1, float *basis_2)
{
	float	tmp[3];
	float	tmp_sum;
	
	tmp_sum = dot_product(new_v,basis_1);
	
	tmp[0] = tmp_sum*(basis_1[0]);
	tmp[1] = tmp_sum*(basis_1[1]);
	tmp[2] = tmp_sum*(basis_1[2]);

	tmp_sum = dot_product(new_v,basis_2);	

	new_v[0] = tmp[0] + tmp_sum*(basis_2[0]);
	new_v[1] = tmp[1] + tmp_sum*(basis_2[1]);
	new_v[2] = tmp[2] + tmp_sum*(basis_2[2]);
}


float	dot_product(float vect_1[3], float *vect_2)
{
	int i;
	
	float dp;

	dp = 0.0;
	
	for( i = 0;  i < 3;  i++ ) dp += vect_1[i]*vect_2[i];
	return dp;
}
