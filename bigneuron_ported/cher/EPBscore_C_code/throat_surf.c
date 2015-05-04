/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "maset.h"
#include "burn_macro.h"
#include "min_surf.h"
#include "burn_prototype.h"
#include "lkc_prototype.h"
#include "min_surf_prototype.h"
#include "ma_prototype.h"

int	generate_surface(int *cube, int *sten26, int throat_vox, int nx, int ny,
					 int nz, int loop_len, grain_chain *g_loop,
					 int path_seg_ind, int throat_ind, char *fname,
					 int conn_type, int gbvc, int print_it, float area)
{
	FILE		*fpth;
	grain_chain	*p_g_loop;
	unsigned char	*wkspc, bd;
	char		*msg;
	char		deuler;
	char		cmnd[300], fname1[300];
	unsigned char	tmp_pt_info[26];

	float	x_cos, y_cos, z_cos, dist, param, m_fact;
	float	dx, dy, dz;
	float	fx_diff, fy_diff, fz_diff;
	float	t_area;

	int	*loc, *grain_keep, *check_obj;
	int	min[3], max[3], is_in[26], wk_sten[26];
	int	throat_vox_wnbhd[27];

	int	i, j, k, l, m, n;
	int	t_i, t_j, t_k;
	int	start, stop;
	int	wx, wy, wz, wxy, wxyz, nxy;
	int	set;
	int	x_diff, y_diff, z_diff;
	int	tmp_ind;
	int	n_i, n_j, n_k;
	int	x, y, z, start_x, start_y, start_z;
	int	voxel_count_area, kount1, kount2;
	int	w_pos, cp, pos, n_w_pos, n_c_pos;
	int	grain_keep_count, keep_count, one_obj;
	int	boole1, boole2, check_size, check_obj_size, check_kount;
	int	throat_vox_wx, throat_vox_wy, throat_vox_wz, throat_vox_wcube;
	int	throat_vox_met;

	int	isz = sizeof(int);
	int	fsz = sizeof(float);
	int	usz = sizeof(unsigned char);

	check_size = (loop_len+2) > 200 ? loop_len+2 : 200;

	check_obj = (int *)MALLOC(check_size*isz);
	msg = "check_obj in generate_surface()";
	if( MEMCHECK(check_obj,msg,check_size*isz) ) clean_up(0);

	check_obj_size = check_size;

	if( print_it )
	{
	    sprintf(fname1,"%s.tmpr",fname);
	    if( (fpth = fopen(fname1,"w")) == NULL )
	    {
		fprintf(stderr,"Error opening file %s. Exiting\n",fname1);
		fprintf(stdout,"Error opening file %s. Exiting\n",fname1);
		clean_up(0);
	    }
	}
	loc = (int *)MALLOC(loop_len*isz);
	msg = "loc in generate_surface()";
	if( MEMCHECK(loc,msg,loop_len*isz) ) clean_up(0);

	p_g_loop = g_loop;
	nxy = nx * ny;

	if( print_it )
	{
		fwrite(&path_seg_ind,isz,1,fpth);
		fwrite(&throat_ind,isz,1,fpth);
	}

	for( i = 0;  i < loop_len;  i++ )
	{
		loc[i] = check_obj[i+1] = p_g_loop->index_in_cube;
		p_g_loop = p_g_loop->next;
	}


	set_index_limits(loc,loop_len,nx,nxy,min,max);

	Pad_ind_2_ijk(throat_vox,i,j,k,nx,nxy)
	if( print_it )
	{
		tmp_ind = k*(nx-2)*(ny-2) + j*(nx-2) + i;
		fwrite(&tmp_ind,isz,1,fpth);
	}

	max[0] = ( max[0] > i ) ? max[0] : i;
	max[1] = ( max[1] > j ) ? max[1] : j;
	max[2] = ( max[2] > k ) ? max[2] : k;

	min[0] = ( min[0] < i ) ? min[0] : i;
	min[1] = ( min[1] < j ) ? min[1] : j;
	min[2] = ( min[2] < k ) ? min[2] : k;

	if( print_it )
	{
	    fwrite(&loop_len,isz,1,fpth);
	    for( l = 0;  l < loop_len;  l++ )
	    {
	        Pad_ind_2_ijk(loc[l],i,j,k,nx,nxy)
		tmp_ind = k*(nx-2)*(ny-2) + j*(nx-2) + i;
		fwrite(&tmp_ind,isz,1,fpth);
	    }
	    fwrite(&loop_len,isz,1,fpth); /* loop_len to be overwritten by  */
					  /* # of throat voxels, once known */
	}

	for( i = 0;  i < 3;  i++ ) { max[i] += 1;	min[i] += 1; }

	wx = max[0] - min[0] + 5;	/* buffer wkspc cube */
	wy = max[1] - min[1] + 5;
	wz = max[2] - min[2] + 5;
	wxy = wx*wy;		wxyz = wxy*wz;

	wkspc = (unsigned char *)CALLOC(wxyz,usz);
	msg = "wkspc in generate_surface()";
	if( MEMCHECK(wkspc,msg,wxyz*usz) ) clean_up(0);

	set_stencil(wx,wxy,wk_sten);

	grain_keep_count = 0;

	/* TODO: This loop can be speeded up */

	for( k = 1;  k < wz - 1;  k++ )
	{
	    n_k = k-2+min[2];

	    if( (n_k >= 0) && (n_k < nz) )
	    {
		for( j = 1;  j < wy - 1;  j++ )
		{
		    n_j = j-2+min[1];

		    if( (n_j >= 0) && (n_j < ny) )
		    {
			for( i = 1;  i < wx - 1;  i++ )
			{
			    n_i = i-2+min[0];
			    if( (n_i >= 0) && (n_i < nx) )
			    {
				pos = n_k*nxy + n_j* nx + n_i;
				cp = cube[pos];
				if(    (cp !=	      0) && (cp !=   EXT_VAL)
				    && (cp !=    BURNED) && (cp !=  RSEALED1)
				    && (cp !=  RSEALED2) && (cp != RRSEALED )
				    && (cp != RRSEALED1) && (cp != RRSEALED2))
				{
					w_pos = k*wxy + j*wx + i;
					wkspc[w_pos] = 1;
					grain_keep_count++;
				}
			    }
			}
		    }
		}
	    }
	} 


	/****
	printf("\nPrinting check1\n");
	dbg_print_barrier(wkspc,min,wz,wx,wxy,nz,nx,nxy);
	*****/

	grain_keep = (int *)MALLOC(grain_keep_count*isz);
	msg = "grain_keep in generate_surface()";
	if( MEMCHECK(grain_keep,msg,grain_keep_count*isz) ) clean_up(0);

	for( i = 0, j = 0;  i < wxyz;  i++ )
	{
		if( wkspc[i] == 1 ) { grain_keep[j] = i;	j++; }
	} 


	/*****
	printf("\nPrinting check2\n");
	dbg_print_barrier(wkspc,min,wz,wx,wxy,nz,nx,nxy);
	*****/

	Ind_2_ijk(throat_vox,t_i,t_j,t_k,nx,nxy)
	for( n = 0;  n < loop_len;  n++ )
	{
		Ind_2_ijk(loc[n],i,j,k,nx,nxy)

		x_diff = t_i - i;    y_diff = t_j - j;    z_diff = t_k - k;
		dist = (float)(x_diff*x_diff + y_diff*y_diff + z_diff*z_diff);
		dist = (float)sqrt((double)dist);

		x_cos = x_diff/dist;	fx_diff = (float)x_diff;
		y_cos = y_diff/dist;	fy_diff = (float)y_diff;
		z_cos = z_diff/dist;	fz_diff = (float)z_diff;

		x_diff = ( x_diff >= 0 ) ? x_diff : -x_diff;
		y_diff = ( y_diff >= 0 ) ? y_diff : -y_diff;
		z_diff = ( z_diff >= 0 ) ? z_diff : -z_diff;

		if( x_diff >= y_diff ) set = ( x_diff >= z_diff ) ? 1 : 3;
		else		       set = ( y_diff >= z_diff ) ? 2 : 3;

		switch( set )
		{
		    case 1:
			if( t_i > i )
			{
				start_x = i;	start_y = j;	start_z = k;
				stop = t_i;
			}
			else
			{
				start_x = t_i;	start_y = t_j;	start_z = t_k;
				stop = i;
			}
			start = start_x;
			m_fact = (float)(1.0/x_cos);
			dx = 1.0;
			dy = fy_diff/fx_diff;
			dz = fz_diff/fx_diff;
			break;

		    case 2:
			if( t_j > j )
			{
				start_x = i;	start_y = j;	start_z = k;
				stop = t_j;
			}
			else
			{
				start_x = t_i;	start_y = t_j;	start_z = t_k;
				stop = j;
			}
			start = start_y;
			m_fact = (float)(1.0/y_cos);
			dx = fx_diff/fy_diff;
			dy = 1.0;
			dz = fz_diff/fy_diff;
			break;

		    case 3:
			if( t_k > k )
			{
				start_x = i;	start_y = j;	start_z = k;
				stop = t_k;
			}
			else
			{
				start_x = t_i;	start_y = t_j;	start_z = t_k;
				stop = k;
			}
			start = start_z;
			m_fact = (float)(1.0/z_cos);
			dx = fx_diff/fz_diff;
			dy = fy_diff/fz_diff;
			dz = 1.0;
			break;

		    default:
			fprintf(stderr,"Logic error in generate_surface()\n");
			fprintf(stdout,"Logic error in generate_surface()\n");
			clean_up(0);
			break;
		}

		for( m = start+1;  m < stop;  m++ )
		{
			param = (m-start)*m_fact;

		        /*********************
			x = start_x + x_cos*param;
			y = start_y + y_cos*param;
			z = start_z + z_cos*param;
			*********************/

			x = (int)(start_x + (m-start)*dx);
			y = (int)(start_y + (m-start)*dy);
			z = (int)(start_z + (m-start)*dz);

			pos =  z*nxy + y*nx + x;
			w_pos = (z+2 - min[2])*wxy + (y+2 - min[1])*wx
						   +  x+2 - min[0];
			/****
			w_pos = (z+1 - min[2])*wxy + (y+1 - min[1])*wx
						   +  x+1 - min[0];
			****/
			if(    (cube[pos] == 0        )
			    || (cube[pos] <= -1000    )
			    || (cube[pos] == BURNED   )
			    || (cube[pos] == RRSEALED ) 
			    || (cube[pos] == RRSEALED1)
			    || (cube[pos] == RRSEALED2) 
			    || (cube[pos] == RSEALED1 )
			    || (cube[pos] == RSEALED2 )   ) wkspc[w_pos] = 1;

			check_bdry_26_nbr(&bd,pos,nx,ny,nz,nxy,is_in);
			if( bd )
			{
			    for( l = 0; l < 26; l++ )
			    {
				n_c_pos = pos+sten26[l];
				if(    is_in[l]
				    && (    (cube[n_c_pos] ==     0    )
					 || (cube[n_c_pos] <= -1000    )
					 || (cube[n_c_pos] == BURNED   )
					 || (cube[n_c_pos] == RRSEALED )
					 || (cube[n_c_pos] == RRSEALED1)
					 || (cube[n_c_pos] == RRSEALED2) 
					 || (cube[n_c_pos] == RSEALED1 ) 
					 || (cube[n_c_pos] == RSEALED2 ) )   )
				{
					n_w_pos = w_pos + wk_sten[l];
					wkspc[n_w_pos] = 1;
				}
			    }
			}
			else
			{
			    for( l = 0; l < 26; l++ )
			    {
				n_c_pos = pos+sten26[l];

				if(    (cube[n_c_pos] ==     0    )
				    || (cube[n_c_pos] <= -1000    )
				    || (cube[n_c_pos] == BURNED   )
				    || (cube[n_c_pos] == RRSEALED )
				    || (cube[n_c_pos] == RRSEALED1)
				    || (cube[n_c_pos] == RRSEALED2) 
				    || (cube[n_c_pos] == RSEALED1 ) 
				    || (cube[n_c_pos] == RSEALED2 )   )
				{
					n_w_pos = w_pos + wk_sten[l];
					wkspc[n_w_pos] = 1;
				}
			    }
			}
		}
	}

	/****
	printf("\nPrinting check3\n");
	dbg_print_barrier(wkspc,min,wz,wx,wxy,nz,nx,nxy);
	****/

	loc = (int *)REALLOC(loc,(loop_len+1)*isz);
	msg = "loc in generate_surface()";
	if( MEMCHECK(loc,msg,isz) ) clean_up(0);

	loc[loop_len] = check_obj[0] = throat_vox;

	Ind_2_ijk(throat_vox,x,y,z,nx,nxy)
	throat_vox_wz = z + 2 - min[2];
	throat_vox_wy = y + 2 - min[1];
	throat_vox_wx = x + 2 - min[0];

	throat_vox_wcube = throat_vox_wz*wxy + throat_vox_wy*wx + throat_vox_wx;

	for( l = 0;  l < loop_len+1;  l++ )
	{
		pos = loc[l];
		Ind_2_ijk(pos,x,y,z,nx,nxy);

		w_pos= (z+2-min[2])*wxy + (y+2-min[1])*wx + x+2-min[0];
		/****
		w_pos= (z+1-min[2])*wxy + (y+1-min[1])*wx + x+1-min[0];
		****/

		for( m = 0;  m < 26;  m++ )
		{
			boole1 = (cube[loc[l]+sten26[m]] ==     0);
			boole2 = (    (cube[loc[l]+sten26[m]] <=  -1000)
				   || (cube[loc[l]+sten26[m]] == BURNED)   );

			if ( boole1 || boole2) wkspc[w_pos + wk_sten[m]] = 1;
		}
		loc[l] = w_pos;
		wkspc[w_pos] = 1;
	}

	keep_count = loop_len+1+grain_keep_count;
	/* keep_count = loop_len+1; */


	loc = (int *)REALLOC(loc,keep_count*isz);
	msg = "loc in generate_surface()";
	if( MEMCHECK(loc,msg,grain_keep_count*isz) ) exit(0);

	for( l = 0;  l < grain_keep_count;  l++ )
		loc[loop_len+1+l] = grain_keep[l]; 

	FREE(grain_keep,grain_keep_count*isz); 

	/****
	printf("Printing Thick surface\n");
	dbg_print_barrier(wkspc,min,wz,wx,wxy,nz,nx,nxy);
	****/

	thin_branch_cluster(wx,wxy,wxyz,keep_count,loc,wkspc,(char)conn_type,
							min,throat_vox,1,1);

	voxel_count_area = kount1 = kount2 = 0;
	check_kount = loop_len;

	throat_vox_met = throat_vox_wnbhd[0] = 0;
	for( l = 0;  l < wxyz;  l++ )
	{
	    if( wkspc[l] == 1 )
	    {
		Ind_2_ijk(l,x,y,z,wx,wxy);

		pos = (z-2+min[2])*nxy + (y-2+min[1])*nx + x-2+min[0];

		if( l == throat_vox_wcube ) throat_vox_met = 1;
		else
		{
			x_diff = x - throat_vox_wx;
			y_diff = y - throat_vox_wy;
			z_diff = z - throat_vox_wz;

			if(    (abs(x_diff) <= 1)
			    && (abs(y_diff) <= 1)
			    && (abs(z_diff) <= 1)    )
			{
			    x_diff = ( x_diff == -1 ) ? 2: x_diff;
			    y_diff = ( y_diff == -1 ) ? 2: y_diff;
			    z_diff = ( z_diff == -1 ) ? 2: z_diff;

			    throat_vox_wnbhd[0] += 1;
			    throat_vox_wnbhd[throat_vox_wnbhd[0]]
					= diff_2_index[x_diff][y_diff][z_diff];
			}
		}
		boole1 = (cube[pos] > -1000) && (cube[pos] != GRAIN);

		boole2 = (    (cube[pos] == -(1000+gbvc  ))
			   || (cube[pos] == -(1000+gbvc+1))   );

		if( boole1 || boole2 )
		{
		    voxel_count_area++;
		    wkspc[l] = 2;
		    if( !boole2 )
		    {
			kount1++;	check_kount++;

			/*****
			if( print_it )
			{
			    fprintf(stderr,"%d) pos %d cube val %d\n",
						kount1,pos,cube[pos]);
			    fprintf(stdout,"%d) pos %d cube val %d\n",
						kount1,pos,cube[pos]);
			}
			*****/

			if( check_kount == check_obj_size)
			{
				check_obj_size += check_size;
				check_obj = (int *)REALLOC(check_obj,
							   check_obj_size*isz);
				msg = "check_obj in generate_surface()";
				if( MEMCHECK(check_obj,msg,check_size*isz) )
					exit(0);
			}
			check_obj[check_kount] = pos;
		    }
		    /****
		    printf("l %d, pos %d, cube[%d] %d\n",l,pos,pos,cube[pos]);
		    ****/
		}
	    }
	}

	/*****
	if( print_it )
	{
		fprintf(stderr,"loop_len %d\n",loop_len);
		fprintf(stdout,"loop_len %d\n",loop_len);

		fprintf(stderr,"voxel_count_area %d\n",voxel_count_area);
		fprintf(stdout,"voxel_count_area %d\n",voxel_count_area);

		fprintf(stderr,"kount1 %d\n",kount1);
		fprintf(stdout,"kount1 %d\n",kount1);
	}
	*****/
	

	check_obj = (int *)REALLOC(check_obj,(check_kount+1)*isz);
	msg = "check_obj in generate_surface()";
	if( MEMCHECK(check_obj,msg,(check_kount+1-check_obj_size)*isz) )
		exit(0);

	one_obj = check_one_obj(check_obj,check_kount+1,nx,nxy,
						&is_6_connected);
	if( one_obj )
	{
		for( i = 0;  i < 26;  i++ ) tmp_pt_info[i] = 0;

		for( i = 1;  i <= throat_vox_wnbhd[0];  i++ )
			tmp_pt_info[ throat_vox_wnbhd[i] ] = 1;

		deuler = calc_deuler(tmp_pt_info,conn_type);
		if( deuler == 0 ) one_obj = 0;
	}
	if( !one_obj )
	{
		if( print_it )
		{
			fclose(fpth);
			sprintf(cmnd,"rm %s",fname1);
			system(cmnd);

			fprintf(stderr,"Miscalculated throat not printed\n");
			fprintf(stdout,"Miscalculated throat not printed\n");
		}
		FREE(loc,keep_count*isz);
		FREE(wkspc,wxyz*usz);
		FREE(check_obj,(check_kount+1)*isz);

		return 0;
	}
	else
	{
	    if( print_it )
	    {
	    	/*****
		fprintf(stderr,"\n\n\n");
		fprintf(stdout,"\n\n\n");
		*****/

		for( l = 0;  l < wxyz;  l++ )
		{
		    if( wkspc[l] == 2 )
		    {
			Ind_2_ijk(l,x,y,z,wx,wxy);
			pos = (z-2+min[2])*nxy + (y-2+min[1])*nx + x-2+min[0];

			boole1 = (cube[pos] > -1000) && (cube[pos] != GRAIN) ;
			boole2 = (    (cube[pos] == -(1000+gbvc  ))
				   || (cube[pos] == -(1000+gbvc+1))   );

			if( boole1 || boole2 )
			{
				kount2++;
				/*****
				fprintf(stderr,"%d) pos %d cube val %d\n",
							kount2,pos,cube[pos]);
				fprintf(stdout,"%d) pos %d cube val %d\n",
							kount2,pos,cube[pos]);
				*****/

				cube[pos] = BARRIER;
				Pad_ind_2_ijk(pos,i,j,k,nx,nxy)
				tmp_ind = k*(nx-2)*(ny-2) + j*(nx-2) + i;
				fwrite(&tmp_ind,isz,1,fpth);
			}
		    }
		}
	    }
	}

	/*****
	if( print_it )
	{
		fprintf(stderr,"kount2 %d\n",kount2);
		fprintf(stdout,"kount2 %d\n",kount2);
	}
	*****/

	if( voxel_count_area == 1 )
	{
		pos = throat_vox;
		for( l = 0;  l < 26;  l++ )
		{
			boole1 = (cube[pos+sten26[l]] != 0      );
			boole2 = (    (cube[pos+sten26[l]] != EXT_VAL)
				   && (cube[pos+sten26[l]] != BURNED )   );

			tmp_pt_info[l] = ( boole1 && boole2 ) ? 1 : 0;
		}
		deuler = calc_deuler(tmp_pt_info,conn_type);
		if( deuler == 0 )
		{
			FREE(loc,keep_count*isz);
			FREE(wkspc,wxyz*usz);
			FREE(check_obj,(check_kount+1)*isz);

			if( print_it )
			{
			    fclose(fpth);
			    sprintf(cmnd,"rm %s",fname1);
			    system(cmnd);

			    fprintf(stderr,"Degenerate throat not printed\n");
			    fprintf(stdout,"Degenerate throat not printed\n");

			    cube[pos] = 0;
			    return 0;
			}
			else return 1;
		}
	}
	if( print_it )
	{
		if( voxel_count_area != kount2 )
		{
		  	fprintf(stderr,"Check for possible error ");
		  	fprintf(stderr,"voxel_count_area %d  kount2 %d\n",
						voxel_count_area,kount2);
		  	fprintf(stdout,"Check for possible error ");
		  	fprintf(stdout,"voxel_count_area %d  kount2 %d\n",
						voxel_count_area,kount2);
		}
		fseek(fpth,-(kount2+1)*isz,1);
		fwrite(&kount2,isz,1,fpth);	/* replaces loop_len  */
						/* with correct value */
		fseek(fpth,kount2*isz,1);
		t_area = area;
		fwrite(&t_area,fsz,1,fpth);
		fclose(fpth);
		sprintf(cmnd,"cat %s>> %s",fname1,fname);
		system(cmnd);
		sprintf(cmnd,"rm %s",fname1);
		system(cmnd);

		/*****
		printf("\nPrinting thin surface\n");
		dbg_print_barrier(wkspc,min,wz,wx,wxy,nz,nx,nxy);
		*****/

		fprintf(stderr,"\tSurface area as a voxel count %d\n",kount2);
		fprintf(stdout,"\tSurface area as a voxel count %d\n",kount2);
	}

	FREE(loc,keep_count*isz);
	FREE(wkspc,wxyz*usz);
	FREE(check_obj,(check_kount+1)*isz);

	return 1;
}


void	dbg_print_barrier(unsigned char *wkspc, int *min, int wz, int wx,
						  int wxy, int nz, int nx, int nxy)
{
	int	i, kount, wxyz, x, y, z;

	wxyz = wxy*wz;
	kount = 1;

	for( i = 0;  i < wxyz;  i++ )
	{
		if( wkspc[i] == 1 )
		{
			Ind_2_ijk(i,x,y,z,wx,wxy);	z++;

			z = z - 2 + min[2];
			y = y - 2 + min[1];
			x = x - 2 + min[0];

			printf("%d) %d (%d %d %d)\n",kount,i,z,y,x);
			kount++;
		}
	}
}
