/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include "min_surf.h"
#include "burn_macro.h"
#include "maset.h"
#include <math.h>
#include "burn_prototype.h"


int	check_loop(int *cube, int *ma_loc, grain_chain *loop, int *sten26,
			   int ma_start, int ma_end, int ma_s, int ma_e, int nx,
			   int nxy, int nxyz)
{
	grain_chain	*p_loop, *pn_loop;

	unsigned char	bd;

	float	x_cos, y_cos, z_cos, dist, param, m_fact;
	float	dx, dy, dz;
	float	fx_diff, fy_diff, fz_diff;
	
	int	is_in[26];	

	int	i, l, m, pl_i, pl_j, pl_k;
	int	n_i, n_j, n_k, ny, nz;
	int	start, stop, set;
	int	x_diff, y_diff, z_diff;
	int	x, y, z, start_x, start_y, start_z;
	int	pos, n_c_pos;

	for( i = ma_start;  i <= ma_end;  i++ ) cube[ma_loc[i]] = MA_MARK;

	/****
	for( i = ma_s;  i <= ma_e;  i++ ) cube[ma_loc[i]] = MA_MARK;
	*****/

	ny = nxy/nx;		nz = nxyz/nxy;

	p_loop = loop;		pn_loop = p_loop->next;
	while( p_loop )
	{
	    Ind_2_ijk(p_loop->index_in_cube,pl_i,pl_j,pl_k,nx,nxy)
	    pn_loop = p_loop->next;
	    while(pn_loop)
	    {
		Ind_2_ijk(pn_loop->index_in_cube,n_i,n_j,n_k,nx,nxy)

		dist = (float)(pl_i-n_i)*(float)(pl_i-n_i) +
			   (float)(pl_j-n_j)*(float)(pl_j-n_j) +
			   (float)(pl_k-n_k)*(float)(pl_k-n_k);
		dist = (float)sqrt(dist);

		x_cos = (pl_i-n_i)/dist;
		y_cos = (pl_j-n_j)/dist;
		z_cos = (pl_k-n_k)/dist;

		x_diff = pl_i - n_i;		fx_diff = (float)x_diff;
		y_diff = pl_j - n_j;		fy_diff = (float)y_diff;
		z_diff = pl_k - n_k;		fz_diff = (float)z_diff;

		x_diff = ( x_diff >= 0 ) ? x_diff : -x_diff;
		y_diff = ( y_diff >= 0 ) ? y_diff : -y_diff;
		z_diff = ( z_diff >= 0 ) ? z_diff : -z_diff;

		if( x_diff >= y_diff ) set = ( x_diff >= z_diff ) ? 1 : 3;
		else		       set = ( y_diff >= z_diff ) ? 2 : 3;

		switch( set )
		{
		    case 1:
			if( pl_i > n_i )
			{
				start_x = n_i;	start_y = n_j;	start_z = n_k;
				stop = pl_i;
			}
			else
			{
				start_x = pl_i;	start_y = pl_j;	start_z = pl_k;
				stop = n_i;
			}
			start = start_x;
			m_fact = (float)(1.0/x_cos);
			dx = 1.0;
			dy = fy_diff/fx_diff;
			dz = fz_diff/fx_diff;
			break;

		    case 2:
			if( pl_j > n_j )
			{
				start_x = n_i;	start_y = n_j;	start_z = n_k;
				stop = pl_j;
			}
			else
			{
				start_x = pl_i;	start_y = pl_j;	start_z = pl_k;
				stop = n_j;
			}
			start = start_y;
			m_fact = (float)(1.0/y_cos);
			dx = fx_diff/fy_diff;
			dy = 1.0;
			dz = fz_diff/fy_diff;
			break;

		    case 3:
			if( pl_k > n_k )
			{
				start_x = n_i;	start_y = n_j;	start_z = n_k;
				stop = pl_k;
			}
			else
			{
				start_x = pl_i;	start_y = pl_j;	start_z = pl_k;
				stop = n_k;
			}
			start = start_z;
			m_fact = (float)(1.0/z_cos);
			dx = fx_diff/fz_diff;
			dy = fy_diff/fz_diff;
			dz = 1.0;
			break;

		    default:
			PRINTF("Logic error in check_loop()\n");
			clean_up(0);
			break;
		}

		for( m = start+1;  m < stop;  m++ )
		{
			param = (m-start)*m_fact;

			x = (int)(start_x + (m-start)*dx);
			y = (int)(start_y + (m-start)*dy);
			z = (int)(start_z + (m-start)*dz);

			pos =  z*nxy + y*nx + x;

			if( cube[pos] == MA_MARK )
			{
				for( i = ma_start;  i <= ma_end;  i++ )
					cube[ma_loc[i]] = BURNED;
				return 1;
			}

			check_bdry_26_nbr(&bd,pos,nx,ny,nz,nxy,is_in);
			if( bd )
			{
			    for( l = 0;  l < 26;  l++ )
			    {
				n_c_pos = pos+sten26[l];
				if( is_in[l] && (cube[n_c_pos] == MA_MARK) )
				{
					for( i = ma_start;  i <= ma_end;  i++ )
						cube[ma_loc[i]] = BURNED;
					return 1;
				}
			    }
			}
			else
			{
			    for( l = 0;  l < 26;  l++ )
			    {
				n_c_pos = pos+sten26[l];

				if( cube[n_c_pos] == MA_MARK )
				{
					for( i = ma_start;  i <= ma_end;  i++ )
						cube[ma_loc[i]] = BURNED;
					return 1;
				}
			    }
			}
		}
		pn_loop = pn_loop->next;
	    }
	    p_loop = p_loop->next;
	}

	p_loop = loop;
	while( p_loop )
	{
		pos = p_loop->index_in_cube;
		if( cube[pos] == MA_MARK )
		{
			for( i = ma_start;  i <= ma_end;  i++ )
				cube[ma_loc[i]] = BURNED;
			return 1;
		}

		for( m = 0;  m < 26;  m++ )
		{
			if (cube[pos+sten26[m]] == MA_MARK)
			{
				for( i = ma_start;  i <= ma_end;  i++ )
					cube[ma_loc[i]] = BURNED;
				return 1;
			}
		}
		p_loop = p_loop->next;
	}

	for( i = ma_start;  i <= ma_end;  i++ ) cube[ma_loc[i]] = BURNED;

	return 0;
}
