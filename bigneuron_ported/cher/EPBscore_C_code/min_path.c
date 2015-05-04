/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"

#include <math.h>
#include <limits.h>
#include "min_surf.h"
#include "maset.h"
#include "burn_macro.h"
#include "data.h"
#include "isoseg.h"
#include "ma_prototype.h"
#include "lkc_prototype.h"
#include "min_surf_prototype.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

grain_chain *compute_minimal_path(grain_chain *closed_loop,
								  gr_bdry_vect_elem *gr_bdry_vect,
								  int gbdry_size, int grain_index, int nx,
								  int ny, int nz, int nxy, int *cube,
								  int *sten26, int *start_vox, int *end_vox,
								  MoI *moi, int *ma_loc, int ma_num,
								  int ma_start, int ma_end, int find_end,
								  int do_reverse, int *t_loc,
								  int *t_loop_len, float *return_area,
								  FILE *fp, int ma_s, int ma_e,
								  int conn_type, int min_surf_dijk)
{
	network_node	*snw_node, *enw_node;
	grain_chain	*return_loop, *shortest_loop[2];

	float 	r_area[2];

	int	th_loc[2], th_loop_len[2];
	int	*ignore_vox;

	int	connect, pass, num_ignore_vox, ok, success, not_first;

	int	isz = sizeof(int);

	r_area[0] = r_area[1] = (float) INT_MAX;

	shortest_loop[0] = (grain_chain *) NULL;
	shortest_loop[1] = (grain_chain *) NULL;

	return_loop = (grain_chain *) NULL;
	ignore_vox  = (int         *) NULL;

	num_ignore_vox = 0;

	if( find_end ) connect = get_path_ends(gr_bdry_vect,grain_index,nx,ny,
					nz,nxy,cube,sten26,start_vox,end_vox,
					num_ignore_vox,ignore_vox);
	else	       connect = 1;

	pass = not_first = 0;

	while( connect == 1 && pass <= 1 )
	{
	    ok = 1;
	    while( ok )
	    {
		if( num_ignore_vox && not_first && find_end && (pass == 0) )
		{
		      connect = get_path_ends(gr_bdry_vect,grain_index,nx,ny,
					nz,nxy,cube,sten26,start_vox,end_vox,
					      num_ignore_vox,ignore_vox);
		}
		not_first = 1;

		if( connect )
		{
			success = setup_network(closed_loop,gr_bdry_vect,
					grain_index,nx,ny,nz,nxy,cube,sten26,
					*start_vox,*end_vox,&snw_node,
					&enw_node,26,pass+1,moi,
					&num_ignore_vox,&ignore_vox);

			     if( success == 0 ) { ok = connect = 0; pass = 1; }
			else if( success == 1 )   ok = 0;
		}
		else ok = 0;
	    }
	    if( num_ignore_vox )
	    {
		FREE(ignore_vox,num_ignore_vox*isz);
		ignore_vox = (int *) NULL;
		num_ignore_vox = 0;
	    }
	    if( connect )
	    {
		shortest_loop[pass] = get_shortest_path(gr_bdry_vect,
						gbdry_size,snw_node,enw_node,
						cube, grain_index,moi,nx,nxy,
						nxy*nz,ma_loc,ma_num,ma_start,
						ma_end,fp,&(r_area[pass]),
						&(th_loc[pass]),
						&(th_loop_len[pass]),
						sten26,ma_s,ma_e,conn_type,
						min_surf_dijk);

		  /* If do_reverse == 0, we have found a path.    */
		  /* There is no need to also search in the other */
		  /* direction. Incrementing pass prevents this   */

		if(    (do_reverse == 0) && (pass == 0)
		    && (shortest_loop[0] != NULL)   ) pass++;

		free_network(snw_node);
	    }
	    pass++;
	}

	if( shortest_loop[0] != NULL && shortest_loop[1] != NULL )
	{
		if( r_area[0] <= r_area[1] )
		{
			*return_area = r_area[0];
			return_loop  = shortest_loop[0];
			*t_loc       = th_loc[0];
			*t_loop_len  = th_loop_len[0];

			free_g_chain(shortest_loop[1]);
		}
		else
		{
			*return_area = r_area[1];
			return_loop  = shortest_loop[1];
			*t_loc       = th_loc[1];
			*t_loop_len  = th_loop_len[1];

			free_g_chain(shortest_loop[0]);
		}
	}
	else if( shortest_loop[0] != NULL )
	{
		*return_area = r_area[0];
		return_loop  = shortest_loop[0];
		*t_loc       = th_loc[0];
		*t_loop_len  = th_loop_len[0];
	}
	else
	{
		*return_area = r_area[1];
		return_loop  = shortest_loop[1];
		*t_loc       = th_loc[1];
		*t_loop_len  = th_loop_len[1];
	}

	return return_loop;
}


float	compute_throat_size(int *ma_loc, int *t_loc, int *loop_len, int nx,
							int nxy, int ma_start, int ma_end,
							grain_chain *loop,
							int use_all_vox, int use_odd_vox, int do_adj)
{
	grain_chain	*p_loop;

	int		i, x0, y0, z0;
	float		area, min_area;

	min_area = (float) INT_MAX;
	*t_loc = -1;
	*loop_len = 1;

	for( i = ma_start;  i <= ma_end;  i++ )
	{
		Ind_2_ijk(ma_loc[i],x0,y0,z0,nx,nxy)
		area = 0.0;

		/* improved area triangulation  ******************/
		p_loop = loop->next;
		while( (i == ma_start) && (p_loop != NULL) )
		{
			(*loop_len)++;
			p_loop = p_loop->next;
		}
		area = get_triangulated_area(loop,x0,y0,z0,nx,nxy,use_all_vox,
							use_odd_vox,do_adj);

		/************ end improved area computation */

		/***** old area triangulation ***
		p_loop = loop;
		Ind_2_ijk(p_loop->index_in_cube,x1,y1,z1,nx,nxy)

		x1 -= x0;	y1 -= y0;	z1 -= z0;

		area = 0.0;

		p_loop = p_loop->next;
		while ( p_loop != NULL )
		{
			area += triangle_area(x0,y0,z0,p_loop->index_in_cube,
							nx,nxy,&x1,&y1,&z1);
			if( i == ma_start ) (*loop_len)++;
			p_loop = p_loop->next;
		}

		area += triangle_area(x0,y0,z0,loop->index_in_cube,nx,nxy,
								&x1,&y1,&z1);
		***** end old area triangulation *****/

		if( min_area > area )
		{
			min_area = area;
			*t_loc = ma_loc[i];
		}
	}
	if( *t_loc != -1 ) min_area /= 2.0;   /* parallelogram -> triangle area */

	return min_area;
}


int	get_path_ends(gr_bdry_vect_elem *gr_bdry_vect, int grain_index, int nx,
				  int ny, int nz, int nxy, int *cube, int *sten26,
				  int *start_vox, int *end_vox, int num_ignore_vox,
				  int *ignore_vox)
{
	associated_grain_chain	*p_tmp, *tmp1;

	point_info	*pt_info;

	unsigned char	is_edge, *node_value;

	int	is_in[26];
	int	i, j;
	int	nbr_index, gr_bdry_index, g_index;
	int	connect;

	pt_info = point_init();
	node_value = pt_info->node_value;

	g_index = -(cube[grain_index]+1000);

	tmp1 = gr_bdry_vect[g_index].p_associated_gc;
	check_bdry_26_nbr(&is_edge,grain_index,nx,ny,nz,nxy,is_in);

	if( is_edge )
	{
		for( i = 0;  i < 26;  i++ )
		{
			if( is_in[i] ) Set_node_value()
		}
	}
	else
	{
		for( i = 0;  i < 26;  i++ ) Set_node_value()
	}

	for(  j = 0;  j < num_ignore_vox;  j++ )
	{
	    if( is_edge )
	    {
		for( i = 0;  i < 26;  i++ )
		{
		    if( is_in[i] && (grain_index+sten26[i] == ignore_vox[j]) )
		    {
			node_value[i] = 0;
			break;
		    }
		}
	    }
	    else
	    {
		for( i = 0;  i < 26;  i++ )
		{
		    if( grain_index+sten26[i] == ignore_vox[j] )
		    {
			node_value[i] = 0;
			break;
		    }
		}
	    }
	}
		    

	connect = oct_labeling(pt_info,start_vox,end_vox); 

	if( connect )
	{
		*start_vox = grain_index + sten26[*start_vox];
		*end_vox   = grain_index + sten26[*end_vox];
	}

	free_point_info(pt_info);
	return connect;
}


#define Calc_Dist(_i,_itmp)\
	_itmp = ( _i >= 13 ) ? _i+1 : _i;\
\
	Pad_ind_2_ijk(_itmp,x,y,z,3,9)\
	tmp = x*x + y*y + z*z;\
	dist1 = (float)sqrt((double)tmp)


int	oct_labeling(point_info *pt_info, int *start_vox, int *end_vox)
{
	unsigned char	label, label1, label2, inc_label;
	int		i, j, itmp, jtmp, x, y, z, tmp, connect;
	float		dist, dist1, min_dist;

	label = 10;
	min_dist = (float) INT_MAX;

	for( i = 1;  i < 9;  i++ )
	{
		inc_label = 0;
		recur_label(pt_info,i,label,&inc_label);
		if( inc_label ) label++;
	}

	if( label > 11 )
	{
		for( i = 0;  i < 26;  i++ )
		{
			label1 = pt_info->node_value[i];
			if( label1 >= 10 )
			{
				Calc_Dist(i,itmp);
				dist = dist1;

				for( j = 0;  j < 26;  j++ )
				{
					label2 = pt_info->node_value[j];
					if( (label2 >= 10) && (label2 != label1) )
					{
						Calc_Dist(j,jtmp);
						dist1 += dist;
						if( dist1 < min_dist )
						{
							*start_vox = i;
							*end_vox = j;
							min_dist = dist1;
						}

					}
				}
			}
		}
		connect = 1;
	}
	else	connect = 0;

	return connect;
}


void	recur_label(point_info *pt_info, int octant, unsigned char label,
					unsigned char *inc_label)
{
	int		i, j;
	unsigned char	nlist_val;

	for( i = 0;  i < 7;  i++ )
	{
	    nlist_val = pt_info->node_list[octant][i];

	    if( pt_info->node_value[nlist_val] == VOIDVOX )
	    {
		pt_info->node_value[nlist_val] = label;
		*inc_label = 1;
		for( j = 1;  j <= pt_info->octant_list[nlist_val][0];  j++ )
		{
			recur_label(pt_info,pt_info->octant_list[nlist_val][j],
							label,inc_label);
		}
	    }
	}
}


int	setup_network(grain_chain *closed_loop, gr_bdry_vect_elem *gr_bdry_vect,
				  int grain_index, int nx, int ny, int nz, int nxy,
				  int *cube, int *sten26, int start_vox, int end_vox,
				  network_node **snw_node, network_node **enw_node,
				  char conn_type, int pass, MoI *moi, int *num_ignore_vox,
				  int **ignore_vox)
{
	network_node	*p_nw_node, *p_cnw_node, *prev_nw_node, *p_s_node,
			*p_e_node;
	grain_chain	*p_closed_loop;

	char	*msg;

	float	v0[3];
	float	norm;

	int	*obj;
	int	index, tmp1, tmp2, tmp3, success, ok, first;
	int	i, stop, num_in_loop, kount;

	int	dbg = 0;

	int	nwnsz = sizeof(network_node);
	int	  isz = sizeof(int);


	num_in_loop = 0;

	Pad_ind_2_ijk(grain_index,tmp1,tmp2,tmp3,nx,nxy)
	v0[0] = (float)tmp1 - (float)(moi->comx);
	v0[1] = (float)tmp2 - (float)(moi->comy);
	v0[2] = (float)tmp3 - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));

	norm = get_norm(v0);
	if( norm < 0.000001 )
	{
	    ok = 1;
	    if( *num_ignore_vox )
	    {
		for( i = 0;  i < *num_ignore_vox;  i++ )
		{
			if( (*ignore_vox)[i] == grain_index ) ok = 0;
		}
	    }
	    if( ok )
	    {
	  	*num_ignore_vox +=1;
		if( *num_ignore_vox == 1 )
		{
			*ignore_vox = (int *)MALLOC(isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		else
		{
			*ignore_vox = (int *)REALLOC(*ignore_vox,
							*num_ignore_vox*isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		(*ignore_vox)[*num_ignore_vox-1] = grain_index;
		success = 0;
		return success;
	    }
	}

	Pad_ind_2_ijk(start_vox,tmp1,tmp2,tmp3,nx,nxy)
	v0[0] = (float)tmp1 - (float)(moi->comx);
	v0[1] = (float)tmp2 - (float)(moi->comy);
	v0[2] = (float)tmp3 - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));

	norm = get_norm(v0);
	if( norm < 0.000001 )
	{
	    ok = 1;
	    if( *num_ignore_vox )
	    {
		for( i = 0;  i < *num_ignore_vox;  i++ )
		{
			if( (*ignore_vox)[i] == start_vox ) ok = 0;
		}
	    }
	    if( ok )
	    {
	  	*num_ignore_vox +=1;
		if( *num_ignore_vox == 1 )
		{
			*ignore_vox = (int *)MALLOC(isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		else
		{
			*ignore_vox = (int *)REALLOC(*ignore_vox,
							*num_ignore_vox*isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		(*ignore_vox)[*num_ignore_vox-1] = start_vox;
		success = 2;
		return success;
	    }
	}

	Pad_ind_2_ijk(end_vox,tmp1,tmp2,tmp3,nx,nxy)
	v0[0] = (float)tmp1 - (float)(moi->comx);
	v0[1] = (float)tmp2 - (float)(moi->comy);
	v0[2] = (float)tmp3 - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));

	norm = get_norm(v0);
	if( norm < 0.000001 )
	{
	    ok = 1;
	    if( *num_ignore_vox )
	    {
		for( i = 0;  i < *num_ignore_vox;  i++ )
		{
			if( (*ignore_vox)[i] == end_vox ) ok = 0;
		}
	    }
	    if( ok )
	    {
	  	*num_ignore_vox +=1;
		if( *num_ignore_vox == 1 )
		{
			*ignore_vox = (int *)MALLOC(isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		else
		{
			*ignore_vox = (int *)REALLOC(*ignore_vox,
							*num_ignore_vox*isz);
			msg = "*ignore_vox in setup_network";
			if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
		}
		(*ignore_vox)[*num_ignore_vox-1] = end_vox;
		success = 2;
		return success;
	    }
	}

	p_closed_loop = closed_loop;

	while( p_closed_loop )
	{
	    index = p_closed_loop->index_in_cube;
	    if(index == grain_index)
	    {
	    	p_closed_loop = p_closed_loop->next;
		continue;
	    }
	    num_in_loop++;

	    /****
	    fprintf(stderr,"%d) %d in closed loop\n",num_in_loop,index);
	    fprintf(stdout,"%d) %d in closed loop\n",num_in_loop,index);
	    *****/

	    ok = 1;
	    while( ok )
	    {
		for( i = 0;  i < *num_ignore_vox;  i++ )
		{
		    if( ((*ignore_vox)[i] == index) || (index == grain_index) )
		    {
		        /****
			fprintf(stderr,"%d) %d in closed loop removed\n",
							num_in_loop,index);
			 fprintf(stdout,"%d) %d in closed loop removed\n",
							num_in_loop,index);
			*****/

		    	num_in_loop--;
			p_closed_loop = p_closed_loop->next;

			if( p_closed_loop )
				index = p_closed_loop->index_in_cube;
			else	ok = 0;		      

			break;
		    }
		}
	  	if( i == *num_ignore_vox ) ok = 0;
	    }

	    if( p_closed_loop )
	    {
		index = p_closed_loop->index_in_cube;
		Pad_ind_2_ijk(index,tmp1,tmp2,tmp3,nx,nxy)
		v0[0] = (float)tmp1 - (float)(moi->comx);
		v0[1] = (float)tmp2 - (float)(moi->comy);
		v0[2] = (float)tmp3 - (float)(moi->comz);

		project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));

		norm = get_norm(v0);
		if( norm < 0.000001 )
		{
		    /*****
		    fprintf(stderr,"%d) %d in closed loop removed\n",
							num_in_loop,index);
		    fprintf(stdout,"%d) %d in closed loop removed\n",
							num_in_loop,index);
		    *****/

		    num_in_loop--;
		    ok = 1;
		    if( *num_ignore_vox )
		    {
			for( i = 0;  i < *num_ignore_vox;  i++ )
			{
			    if( (*ignore_vox)[i] == index) ok = 0;
			}
		    }
		    if( ok )
		    {
			*num_ignore_vox +=1;
			if( *num_ignore_vox == 1 )
			{
			    *ignore_vox = (int *)MALLOC(isz);
			    msg = "*ignore_vox in setup_network";
			    if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
			}
			else
			{
			    *ignore_vox = (int *)REALLOC(*ignore_vox,
							*num_ignore_vox*isz);
			    msg = "*ignore_vox in setup_network";
			    if( MEMCHECK(*ignore_vox,msg,isz) ) clean_up(0);
			}
			(*ignore_vox)[*num_ignore_vox-1] = index;
		    }
		}
		p_closed_loop = p_closed_loop->next;
	    }
	    else break;
	}

	if( *num_ignore_vox )
	{
		obj = (int *) MALLOC(num_in_loop*isz);
		msg = "obj in setup_network";
		if( MEMCHECK(obj,msg,num_in_loop*isz) ) clean_up(0);

		p_closed_loop = closed_loop;
		stop = *num_ignore_vox;
		kount = 0;
		while( p_closed_loop )
		{
		    index = p_closed_loop->index_in_cube;
		    ok = 1;
		    while( ok == 1 )
		    {
			for( i = 0;  i < stop;  i++ )
			{
			    if(    ((*ignore_vox)[i] == index )
				|| (index == grain_index )    )
			    {
				p_closed_loop = p_closed_loop->next;
				if( p_closed_loop )
					index = p_closed_loop->index_in_cube;
				else	ok = 0;
				break;
			    }
			}
			if( i== stop ) ok = 2;
		    }
		    if( ok == 2 )
		    {
			obj[kount] = index;
			kount++;

			/*****
			fprintf(stderr,"%d) %d in obj\n",kount,index);
			fprintf(stdout,"%d) %d in obj\n",kount,index);
			*****/
		    }
		    if( p_closed_loop) p_closed_loop = p_closed_loop->next;
		}
		if( kount != num_in_loop )
		{
		    fprintf(stderr,"Logic error in setup_network: kount ");
		    fprintf(stderr,"%d num_in_loop %d\n",kount,num_in_loop);
		    fprintf(stdout,"Logic error in setup_network: kount ");
		    fprintf(stdout,"%d num_in_loop %d\n",kount,num_in_loop);
		    clean_up(0);
		}
		success = check_one_obj(obj,kount,nx,nxy,&is_26_connected);

		FREE(obj,kount*isz);

		if( !success ) return 0;
	}

	p_cnw_node = (network_node *)MALLOC(nwnsz);
	msg = "p_cnw_node in setup_network";
	if( MEMCHECK(p_cnw_node,msg,nwnsz) ) clean_up(0);

	p_cnw_node->next = NULL;
	p_cnw_node->nw_adj_list = NULL;
	p_nw_node = p_cnw_node;
	prev_nw_node = NULL;

	p_closed_loop = closed_loop;
	first = 1;
	stop = *num_ignore_vox;
	while( p_closed_loop )
	{
	    index = p_closed_loop->index_in_cube;
	    
	    ok = 1;
	    while(ok)
	    {
		for( i = 0; i < stop; i++ )
		{
		    if(    ((*ignore_vox)[i] == index )
			|| (index == grain_index ) )
		    {
			p_closed_loop = p_closed_loop->next;
			if( p_closed_loop )
				index = p_closed_loop->index_in_cube;
			else	ok = 0;      

			break;
		    }
		}

		if( i== stop ) ok = 0;
	    }
	    if( p_closed_loop == NULL ) break;

	    if( !first )
	    {
		prev_nw_node = p_nw_node;
		p_nw_node->next = (network_node *)MALLOC(nwnsz);
		p_nw_node = p_nw_node->next;
		msg = "p_nw_node in setup_network";
		if( MEMCHECK(p_nw_node,msg,nwnsz) ) clean_up(0);
	    }
	    else first = 0;
	    
	    if( index == start_vox ) p_s_node = p_nw_node;
	    else if( index == end_vox   ) p_e_node = p_nw_node;

	    initialize_nw_node(p_nw_node,prev_nw_node,gr_bdry_vect,
			       index,grain_index,nx,ny,nz,nxy,cube,
			       sten26,conn_type,*num_ignore_vox,*ignore_vox);

	    p_closed_loop = p_closed_loop->next;
	}

	if( p_cnw_node == p_e_node )
	{
		p_cnw_node = p_cnw_node->next;
		if( !p_cnw_node )
		{
			printf("Logic error in setup_network()\n");
			clean_up(0);
		}
	}
	     
	if( p_nw_node != p_e_node )
		 p_nw_node = del_join_for_link(p_nw_node,p_e_node);

	if( p_cnw_node != p_s_node )
		 p_cnw_node = del_join_bac_link(p_cnw_node,p_s_node);

	if( pass == 1 )
	{
	 	*snw_node = p_cnw_node;
		*enw_node = p_nw_node;
	}
	else if( pass == 2 )
	{
		recur_reverse_network_dir(p_cnw_node);
		*snw_node = p_nw_node;
		*enw_node = p_cnw_node;
	}
	else
	{
		 printf("Error in setup_network() \n");
		 clean_up(0);
	}
	     
	if( dbg )
	{
		int	cnt;

		cnt = 1;	dbg_print_network(*snw_node,&cnt,1);
		cnt = 1;	dbg_print_network(*enw_node,&cnt,2);
	}

	success = 1;
	return success;
}


#define Build_Adj_List(_Ind,_pagc)\
{\
    g_index = index+sten26[_Ind];\
\
    if( cube[g_index] > -1000 || g_index == grain_index ) continue;\
    else if( num_ignore_vox )\
    {\
	set_continue =  0;\
    	for( j = 0; j < num_ignore_vox; j++ )\
	{\
		if( g_index == ignore_vox[j] )\
	  	{\
			set_continue = 1;\
			break;\
		}\
	}\
	if( set_continue )	continue;\
    }\
\
    g_index = -(cube[g_index]+1000);\
    if( (gr_bdry_vect[g_index].p_associated_gc != NULL) &&\
	(gr_bdry_vect[g_index].p_associated_gc == _pagc))\
    {\
	if( ok )\
	{\
		p_nw_adj_list = (network_adj_list *)MALLOC(nwalsz);\
		msg = "p_nw_adj_list in initialize_nw_node()";\
		if( MEMCHECK(p_nw_adj_list,msg,nwalsz) ) clean_up(0);\
\
		p_nw_node->nw_adj_list = p_nw_adj_list;\
		ok = 0;\
	}\
	else\
	{\
		p_nw_adj_list->next = (network_adj_list *)MALLOC(nwalsz);\
		msg = "p_nw_adj_list->next in initialize_nw_node()";\
		if( MEMCHECK(p_nw_adj_list->next,msg,nwalsz) ) clean_up(0);\
\
		p_nw_adj_list = p_nw_adj_list->next;\
	}\
	p_nw_adj_list->index_in_cube = index+sten26[_Ind];\
	p_nw_adj_list->next= NULL;\
	if(    _Ind ==  0 || _Ind ==  2 || _Ind ==  6 || _Ind ==  8\
            || _Ind == 17 || _Ind == 19 || _Ind == 23 || _Ind == 25 )\
        {\
                p_nw_adj_list->length = 2;\
        }\
        else if(    _Ind ==  1 || _Ind ==  3 || _Ind ==  5 || _Ind ==  7 ||\
                    _Ind ==  9 || _Ind == 11 || _Ind == 14 || _Ind == 16 ||\
                    _Ind == 18 || _Ind == 20 || _Ind == 22 || _Ind == 24 )\
        {\
                p_nw_adj_list->length = 1;\
        }\
        else\
                p_nw_adj_list->length = 0;\
    }\
}


void	initialize_nw_node(network_node *p_nw_node,
						   network_node *prev_nw_node,
						   gr_bdry_vect_elem *gr_bdry_vect, int index,
						   int grain_index, int nx, int ny, int nz, int nxy,
						   int *cube, int *sten26, char conn_type,
						   int  num_ignore_vox, int *ignore_vox)
{
	network_adj_list	*p_nw_adj_list;

	unsigned char	is_edge;

	char	*msg;

	static float t_pi = (float)(2.0*M_PI);
	static	float flint = (float) INT_MAX;

	int	i, j, g_index, gp_index, ok, set_continue;
	int	is_in[26], six_nbrs[6];

	int	nwalsz = sizeof(network_adj_list);

	if(conn_type != (char)26 )
	{
		/* We are going to use a labeling procedure to get */
		/* connectivity. It is geared towards 26 connectivity */
		/* so  warn user */

		fprintf(stderr,"6 connected boundary grain chains are not ");
		fprintf(stderr,"obtained directly\nbut are generated from ");
		fprintf(stderr,"26 connected ones at a later stage.\nPlease ");
		fprintf(stderr,"desist from using 6 connectivity at this ");
		fprintf(stderr,"stage.\nCode might improve in the future.\n");

		fprintf(stdout,"6 connected boundary grain chains are not ");
		fprintf(stdout,"obtained directly\nbut are generated from ");
		fprintf(stdout,"26 connected ones at a later stage.\nPlease ");
		fprintf(stdout,"desist from using 6 connectivity at this ");
		fprintf(stdout,"stage.\nCode might improve in the future.\n");

		clean_up(0);
	}

	p_nw_node->index_in_cube  = index;
	p_nw_node->forward_angle  = -t_pi;
	p_nw_node->backward_angle = t_pi;
	p_nw_node->forward_label  = flint;
	p_nw_node->backward_label = flint;
	p_nw_node->forward_3d_area  = flint;
	p_nw_node->backward_3d_area = flint;
	p_nw_node->f_tmp          = 2;
	p_nw_node->b_tmp          = 2;
	p_nw_node->for_in_indx_chain = NULL;
	p_nw_node->bac_in_indx_chain = NULL;
	p_nw_node->back_pointer   = prev_nw_node;
	p_nw_node->for_prev_path  = NULL;
	p_nw_node->bac_prev_path  = NULL;
	p_nw_node->next           = NULL;
	p_nw_node->nw_adj_list    = (network_adj_list *) NULL;

	g_index = gp_index = -(cube[index]+1000);

	gr_bdry_vect[g_index].nw_node = p_nw_node;

	check_bdry_26_nbr(&is_edge,index,nx,ny,nz,nxy,is_in);

	ok = 1;

	if( conn_type == 26 )
	{
	    for( i = 0;  i < 26;  i++ )
	    {
		if( is_edge )
		{
		    if( is_in[i] )
			Build_Adj_List(i,gr_bdry_vect[gp_index].p_associated_gc)
		}
		else	Build_Adj_List(i,gr_bdry_vect[gp_index].p_associated_gc)		  
	    }
	}
	else if( conn_type == 6 )
	{
	    six_nbrs[0] =  4;	six_nbrs[3] = 13;
	    six_nbrs[1] = 10;	six_nbrs[4] = 15;
	    six_nbrs[2] = 12;	six_nbrs[5] = 21;

	    for( i = 0;  i < 6;  i++ )
	    {
		if( is_edge )
		{
		    if( is_in[six_nbrs[i]] )
			Build_Adj_List(six_nbrs[i],
					gr_bdry_vect[gp_index].p_associated_gc)
		}
		else	Build_Adj_List(six_nbrs[i],
					gr_bdry_vect[gp_index].p_associated_gc)
	    }
	}
	else
	{
		printf("Cannot handle %d connectivity\n",conn_type);
		clean_up(0);
	}
}


int	check_disconnectivity(associated_grain_chain *pagc, int *cube,
						  int ignore_index, int index, int *sten26,
						  unsigned char *is_edge, int is_in[26],
						  int num_ignore_vox, int *ignore_vox,
						  gr_bdry_vect_elem *gr_bdry_vect, int nx, int nxy)
{
	associated_grain_chain	*p_tmp, *tmp1;  

	point_info	*pt_info;

	unsigned char	*node_value;
	unsigned char	label, inc_label, label1;

	int	new_is_in[26];

	int	i, ok;
	int	x, y, z, n_x, n_y, n_z;
	int	x_diff, y_diff, z_diff, ax_diff, ay_diff, az_diff;
    	int	grain_index, nbr_index, gr_bdry_index;

	Ind_2_ijk(index,x,y,z,nx,nxy)

	Ind_2_ijk(ignore_index,n_x,n_y,n_z,nx,nxy)
	x_diff = n_x-x;		ax_diff = abs(x_diff);
	y_diff = n_y-y;		ay_diff = abs(y_diff);
	z_diff = n_z-z;		az_diff = abs(z_diff);

	if( (ax_diff <= 1) && (ay_diff <= 1) && (az_diff <= 1) )
	{
		x_diff = (x_diff < 0) ? 2 : x_diff;
		y_diff = (y_diff < 0) ? 2 : y_diff;
		z_diff = (z_diff < 0) ? 2 : z_diff;

		is_in[diff_2_index[x_diff][y_diff][z_diff]] = 0;
	}

	for( i = 0;  i < 26;  i++ ) new_is_in[i] = is_in[i];

	ok = 0;
	for( i = 0;  i < num_ignore_vox;  i++ )
	{
		Ind_2_ijk(ignore_vox[i],n_x,n_y,n_z,nx,nxy)
		x_diff = n_x-x;		ax_diff = abs(x_diff);
		y_diff = n_y-y;		ay_diff = abs(y_diff);
		z_diff = n_z-z;		az_diff = abs(z_diff);

		if( (ax_diff <= 1) && (ay_diff <= 1) && (az_diff <= 1) )
		{
			x_diff = (x_diff < 0) ? 2 : x_diff;
			y_diff = (y_diff < 0) ? 2 : y_diff;
			z_diff = (z_diff < 0) ? 2 : z_diff;

			new_is_in[diff_2_index[x_diff][y_diff][z_diff]] = 0;
			ok = 1;
		}
	}

	if( ok == 0 ) return 1;

	pt_info = point_init();
	node_value = pt_info->node_value;
	tmp1 = pagc;
	grain_index = index;	/* Note this is not the usual grain_index. */
				/* This is done to use Set_node_value().   */

	if( *is_edge )
	{
		for( i = 0;  i < 26;  i++ )
		{
			if( is_in[i] ) Set_node_value()
		}
	}
	else
	{
		for( i = 0;  i < 26;  i++ ) 
		{
			    /* see note above: ignore_index is "grain_index" */
			if( ignore_index != grain_index + sten26[i] )
				Set_node_value()
		}
	}

	label = 10;
	for( i = 1;  i < 9;  i++ )
	{
		inc_label = 0;
		recur_label(pt_info,i,label,&inc_label);
		if( inc_label ) label++;
	}  

	for( i = 0;  i < 26;  i++ ) node_value[i] = (unsigned char) 0;

	for( i = 0;  i < 26;  i++ )
	{
		if( new_is_in[i] ) Set_node_value()
	}
	label1 = 10;
	for( i = 1;  i < 9;  i++ )
	{
		inc_label = 0;
		recur_label(pt_info,i,label1,&inc_label);
		if( inc_label ) label1++;
	}  

	free_point_info(pt_info);

	if( label1 != label ) return 0;

	for( i = 0;  i < 26;  i++ ) is_in[i] = new_is_in[i];

	*is_edge = 1;
	return 1;
}


grain_chain *get_shortest_path(gr_bdry_vect_elem *gr_bdry_vect,
							   int gbdry_size, network_node *snw_node,
							   network_node *enw_node, int *cube,
							   int grain_index, MoI *moi, int nx, int nxy,
							   int nxyz, int *ma_loc, int ma_num,
							   int ma_start, int ma_end, FILE *fp,
							   float *return_area, int *t_loc,
							   int *t_loop_len, int *sten26, int ma_s,
							   int ma_e, int conn_type, int min_surf_dijk)
{
	FILE		*fp1, *fp2;

	network_node	*p_snw_node, *p_enw_node, *n_nw_node;
	network_node	*tmp_for_node_list[26], *tmp_bac_node_list[26];
	network_adj_list 	*p_nw_adj_list;
	grain_chain	 *shortest_path, *ptr_shortest_path;
	tmp_indx_chain	*for_tmp_indx_chain, *bac_tmp_indx_chain;
	tmp_indx_chain	*ptmp_indx_chain;

	int		indx_for[26], indx_bac[26];
	char	*msg, for_name[128];

	static	float	t_pi = (float)(2.0*M_PI);
	static	float	cost[3] = {(float)0.9016, (float)1.289, (float)1.615};
	static	float	TOL1 = (float)0.000001;

	float	v0[3], v1[3], vf[3], vb[3];
	float	d_theta, area1, const_d, p_dist;

	static	int	pr_cnt = 0;

	int	g_index, i, winding_num, ny, nz;
	int	ind0, tmp1, tmp2, tmp3, is_loop;
	int	ok, ok1, has_pore, for_nbr_kount, bac_nbr_kount;
	int	o_x, o_y, o_z;
	int	dbg = 0;

	int	gcsz = sizeof(grain_chain);
	int	pnnsz = sizeof(network_node *);
	int	ticsz = sizeof(tmp_indx_chain);

	if( dbg )
	{
		int	cnt;
		cnt = 1;	dbg_print_network(snw_node,&cnt,1);
		cnt = 1;	dbg_print_network(enw_node,&cnt,2);
	}

	ny = nxy/nx;
	nz = nxyz/nxy;

	const_d = (float)(- moi->pa[0]*moi->comx
					  - moi->pa[1]*moi->comy
					  - moi->pa[2]*moi->comz);

	shortest_path = (grain_chain *) NULL;

	snw_node->forward_label = 0.0;
	enw_node->backward_label = 0.0;

	if(    (min_surf_dijk == 1) || (min_surf_dijk == 2)
	    || (min_surf_dijk == 3) || (min_surf_dijk == 4)    )
	{
		snw_node->forward_3d_area  = 0.0;
		enw_node->backward_3d_area = 0.0;
	}
	snw_node->forward_angle  = 0.0;		snw_node->f_tmp = 0;
	enw_node->backward_angle = 0.0;		enw_node->b_tmp = 0;

	ind0 = snw_node->index_in_cube;
	Pad_ind_2_ijk(ind0,o_x,o_y,o_z,nx,nxy)
	v0[0] = (float)o_x - (float)(moi->comx);
	v0[1] = (float)o_y - (float)(moi->comy);
	v0[2] = (float)o_z - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));
	vf[0] = v0[0];
	vf[1] = v0[1];
	vf[2] = v0[2];

	p_nw_adj_list = snw_node->nw_adj_list;
	for_nbr_kount = 0;

	while( p_nw_adj_list != NULL )
	{
		g_index = -(cube[p_nw_adj_list->index_in_cube]+1000);
		n_nw_node = gr_bdry_vect[g_index].nw_node;


		ind0 = n_nw_node->index_in_cube;
		Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
		v1[0] = (float)tmp1 - (float)(moi->comx);
		v1[1] = (float)tmp2 - (float)(moi->comy);
		v1[2] = (float)tmp3 - (float)(moi->comz);

		project_2_plane(v1,&(moi->pa[3]),&(moi->pa[6]));
		if( min_surf_dijk == 1 )
		{
			area1 = triangle_area1(0.0,0.0,0.0,v0[0],v0[1],v0[2],
							   v1[0],v1[1],v1[2]);
			if( area1 < TOL1 )
			{
			      fprintf(stderr,"area1 very small\n");
			      fprintf(stdout,"area1 very small\n");
			}
			n_nw_node->forward_label = snw_node->forward_label
						 + area1;
			area1 = triangle_area1((float)moi->comx,(float)moi->comy,
					       (float)moi->comz,(float)o_x,
					       (float)o_y,(float)o_z,(float)tmp1,
					       (float)tmp2,(float)tmp3);

			n_nw_node->forward_3d_area = snw_node->forward_3d_area
						   + area1;
		}
		else if( (min_surf_dijk == 2) || (min_surf_dijk == 3) )
		{
			area1 = triangle_area1((float)moi->comx,(float)moi->comy,
					       (float)moi->comz,(float)o_x,
					       (float)o_y,(float)o_z,(float)tmp1,
					       (float)tmp2,(float)tmp3);

			n_nw_node->forward_label = snw_node->forward_label
						 + area1;
		}
		else if( min_surf_dijk == 4 )
		{
		    p_dist = moi->pa[0]*tmp1 + moi->pa[1]*tmp2
			   + moi->pa[2]*tmp3 + const_d;

		    p_dist = (float)fabs(p_dist);
		    n_nw_node->forward_label = p_dist;

		    area1 = triangle_area1((float)moi->comx,(float)moi->comy,
					   (float)moi->comz,(float)o_x,
					   (float)o_y,(float)o_z,(float)tmp1,
					   (float)tmp2,(float)tmp3);

		    n_nw_node->forward_3d_area = snw_node->forward_3d_area
								+ area1;
		}
		else
		  	n_nw_node->forward_label = snw_node->forward_label
						 + cost[p_nw_adj_list->length]; 

		d_theta = calc_d_theta(v0,v1,&(moi->pa[0]),'n');

		n_nw_node->forward_angle = snw_node->forward_angle + d_theta;
		n_nw_node->for_prev_path = snw_node;
		n_nw_node->f_tmp = 1;
		p_nw_adj_list = p_nw_adj_list->next;
		tmp_for_node_list[for_nbr_kount] = n_nw_node;
		for_nbr_kount++;
	}
	
	     if( for_nbr_kount == 0 ) return shortest_path;
	else if( for_nbr_kount >= 1 )
		indexx_for(for_nbr_kount,tmp_for_node_list-1,indx_for-1);

	ind0 = enw_node->index_in_cube;
	Pad_ind_2_ijk(ind0,o_x,o_y,o_z,nx,nxy)
	v0[0] = (float)o_x - (float)(moi->comx);
	v0[1] = (float)o_y - (float)(moi->comy);
	v0[2] = (float)o_z - (float)(moi->comz);

	project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));

	vb[0] = v0[0];
	vb[1] = v0[1];
	vb[2] = v0[2];


	p_nw_adj_list = enw_node->nw_adj_list;
	bac_nbr_kount = 0;

	while( p_nw_adj_list != NULL )
	{
		g_index = -(cube[p_nw_adj_list->index_in_cube]+1000);
		n_nw_node = gr_bdry_vect[g_index].nw_node;


		ind0 = n_nw_node->index_in_cube;
		Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
		v1[0] = (float)tmp1 - (float)(moi->comx);
		v1[1] = (float)tmp2 - (float)(moi->comy);
		v1[2] = (float)tmp3 - (float)(moi->comz);

		project_2_plane(v1,&(moi->pa[3]),&(moi->pa[6]));
		if( min_surf_dijk == 1 )
		{
			area1 = triangle_area1(0.0,0.0,0.0,v0[0],v0[1],v0[2],
							   v1[0],v1[1],v1[2]);
			if( area1 < TOL1 )
			{
				fprintf(stderr,"area1 very small\n");
				fprintf(stdout,"area1 very small\n");
			}
			n_nw_node->backward_label = enw_node->backward_label
			  + area1;
			area1 = triangle_area1((float)moi->comx,(float)moi->comy,
					       (float)moi->comz,(float)o_x,
					       (float)o_y,(float)o_z,(float)tmp1,
					       (float)tmp2,(float)tmp3);

			n_nw_node->backward_3d_area = enw_node->backward_3d_area
						    + area1;
		}
		else if( min_surf_dijk == 2 || min_surf_dijk == 3 )
		{
			area1 = triangle_area1((float)moi->comx,(float)moi->comy,
					       (float)moi->comz,(float)o_x,
					       (float)o_y,(float)o_z,(float)tmp1,
					       (float)tmp2,(float)tmp3);

			n_nw_node->backward_label = enw_node->backward_label
			  + area1;
		}
		else if( min_surf_dijk == 4 )
		{
		    p_dist = moi->pa[0]* tmp1 + moi->pa[1]*tmp2 + moi->pa[2]*tmp3 + const_d;
		    p_dist = (float)fabs(p_dist);
		    n_nw_node->backward_label = p_dist;
		    area1 = triangle_area1((float)moi->comx,(float)moi->comy,(float)moi->comz,(float)o_x,(float)o_y,(float)o_z,(float)tmp1,(float)tmp2,(float)tmp3);
		    n_nw_node->backward_3d_area = enw_node->backward_3d_area + area1;
		}
		else	n_nw_node->backward_label = enw_node->backward_label
						  + cost[p_nw_adj_list->length];

		d_theta = calc_d_theta(v0,v1,&(moi->pa[0]),'n');

		n_nw_node->backward_angle = enw_node->backward_angle + d_theta;
		n_nw_node->bac_prev_path = enw_node;
		n_nw_node->b_tmp = 1;
		p_nw_adj_list = p_nw_adj_list->next;
		tmp_bac_node_list[bac_nbr_kount] = n_nw_node;
		bac_nbr_kount++;
	}

	     if( bac_nbr_kount == 0 ) return shortest_path;
	else if( bac_nbr_kount >= 1)
		indexx_back(bac_nbr_kount,tmp_bac_node_list-1,indx_bac-1);

	for_tmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
	msg = "for_tmp_indx_chain in get_shortest_path()";
	if( MEMCHECK(for_tmp_indx_chain,msg,ticsz) ) clean_up(0);

	bac_tmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
	msg = "bac_tmp_indx_chain in get_shortest_path()";
	if( MEMCHECK(bac_tmp_indx_chain,msg,ticsz) ) clean_up(0);

	for_tmp_indx_chain->prev = (tmp_indx_chain*) NULL;
	for_tmp_indx_chain->nw_node = tmp_for_node_list[indx_for[0]-1];
	for_tmp_indx_chain->next = (tmp_indx_chain*) NULL;
	tmp_for_node_list[indx_for[0]-1]->for_in_indx_chain = for_tmp_indx_chain;

	bac_tmp_indx_chain->prev = (tmp_indx_chain*) NULL;
	bac_tmp_indx_chain->nw_node = tmp_bac_node_list[indx_bac[0]-1];
	bac_tmp_indx_chain->next = (tmp_indx_chain*) NULL;
	tmp_bac_node_list[indx_bac[0]-1]->bac_in_indx_chain = bac_tmp_indx_chain;
	
	ptmp_indx_chain = for_tmp_indx_chain;
	for( i = 1;  i < for_nbr_kount;  i++ )
	{
		ptmp_indx_chain->next = (tmp_indx_chain *)MALLOC(ticsz);
		msg = "ptmp_indx_chain->next in get_shortest_path()";
		if( MEMCHECK(ptmp_indx_chain->next,msg,ticsz) ) clean_up(0);

		(ptmp_indx_chain->next)->prev = ptmp_indx_chain;
		ptmp_indx_chain = ptmp_indx_chain->next;
		ptmp_indx_chain->next = (tmp_indx_chain*) NULL;
		ptmp_indx_chain->nw_node = tmp_for_node_list[indx_for[i]-1];
		(ptmp_indx_chain->nw_node)->for_in_indx_chain = ptmp_indx_chain;
	}

	ptmp_indx_chain = bac_tmp_indx_chain;
	for( i = 1;  i < bac_nbr_kount;  i++ )
	{
		ptmp_indx_chain->next = (tmp_indx_chain *)MALLOC(ticsz);
		msg = "ptmp_indx_chain->next in get_shortest_path()";
		if( MEMCHECK(ptmp_indx_chain->next,msg,ticsz) ) clean_up(0);

		(ptmp_indx_chain->next)->prev = ptmp_indx_chain;
		ptmp_indx_chain = ptmp_indx_chain->next;
		ptmp_indx_chain->next = (tmp_indx_chain*) NULL;
		ptmp_indx_chain->nw_node = tmp_bac_node_list[indx_bac[i]-1];
		(ptmp_indx_chain->nw_node)->bac_in_indx_chain = ptmp_indx_chain;
	}

	p_snw_node = snw_node;
	p_enw_node = enw_node;
	ok = 1;
	fp1 = fp2 = (FILE *)NULL;
	if( fp )
	{
		pr_cnt++;
		sprintf(for_name,"/tmp/arun/f_sht.%d",pr_cnt);
		fp1 = fopen(for_name,"w");
		sprintf(for_name,"/tmp/arun/b_sht.%d",pr_cnt);
		fp2 = fopen(for_name,"w");
	}
	else	fp1 = fp2 = (FILE *)NULL;

	while( ok )
	{
	    shortest_path = Bi_Dir_for_Label(&p_snw_node,&for_tmp_indx_chain,
					     gr_bdry_vect,moi,cube,nx,nxy,
					     grain_index,&ok,fp1,min_surf_dijk,
					     vf,const_d);

	    if( ok ) shortest_path = Bi_Dir_bac_Label(&p_enw_node,
						&bac_tmp_indx_chain,
						gr_bdry_vect,moi,cube,nx,nxy,
						grain_index,&ok,fp2,
						min_surf_dijk,vb,const_d);
	}
	if( fp )
	{
		fclose(fp1);
		fclose(fp2);
	}
	if( for_tmp_indx_chain != NULL )
		free_tmp_indx_chain(for_tmp_indx_chain,1);
	if( bac_tmp_indx_chain != NULL )
		free_tmp_indx_chain(bac_tmp_indx_chain,2);

	if( shortest_path )
	{
	//ascii_network_opt(cube,nx,ny,nxyz,gr_bdry_vect,gbdry_size,snw_node,
	//			shortest_path,grain_index,ma_loc,ma_num,fp);

	    winding_num = calc_winding_number(shortest_path,moi,nx,nxy,'n');
	    if( winding_num )
	    {
			is_loop = check_loop(cube,ma_loc,shortest_path,sten26,
						 ma_start,ma_end,ma_s,ma_e,nx,nxy,nxyz);
			if( is_loop )
			{
				*return_area = compute_throat_size(ma_loc,t_loc,t_loop_len,
								nx,nxy,ma_start,ma_end,
									shortest_path,1,1,1);
				has_pore = 0;
				ok = get_6_path(cube,sten26,nx,ny,nz,t_loop_len,
						shortest_path,&has_pore,gbdry_size);
				if( ok )
				{
					ok1 = generate_surface(cube,sten26,*t_loc,nx,ny,nz,
							   *t_loop_len,shortest_path,0,0,
							   (char *) NULL,conn_type,
							   gbdry_size,0,*return_area);
					if( has_pore )
					{
						ptr_shortest_path = shortest_path;
						while( ptr_shortest_path != NULL )
						{
							ind0 = ptr_shortest_path->index_in_cube;
								if( cube[ind0] == -(1000+gbdry_size) )
								cube[ind0] = 0;
							else if( cube[ind0] == -(1000+gbdry_size+1) )
								cube[ind0] = BURNED;					    
							ptr_shortest_path = ptr_shortest_path->next;
						}
					}
					if( ok1 ) return shortest_path;
					else
					{
						free_g_chain(shortest_path);
						return (grain_chain *) NULL;
					}			      
				}
				else
				{
		    		if( has_pore )
					{
						ptr_shortest_path = shortest_path;
						while(ptr_shortest_path != NULL)
						{
							ind0 = ptr_shortest_path->index_in_cube;
							if( cube[ind0] == -(1000+gbdry_size) )
								cube[ind0] = 0;
							else if( cube[ind0] == -(1000+gbdry_size+1) )
								cube[ind0] = BURNED;					    
							ptr_shortest_path = ptr_shortest_path->next;
						}
					}
					free_g_chain(shortest_path);
					return (grain_chain *) NULL;
				}		    	
			}
			else
			{
				/*****
				fprintf(stderr,"Min surf does not intersect medial axis\n");
				fprintf(stdout,"Min surf does not intersect medial axis\n");
				*****/

				free_g_chain(shortest_path);
				return (grain_chain *) NULL;
			}
	    }
	    else
	    {
			/*****
			fprintf(stderr,"Path does not have winding number 1\n");
			fprintf(stdout,"Path does not have winding number 1\n");
			*****/

			free_g_chain(shortest_path);
			return (grain_chain *) NULL;
	    }
	}

	return (grain_chain *) NULL;
}


void	rec_mk_ret_chain(grain_chain *ret_grain_chain,
						 grain_chain **last_link,
						 network_node *prev_nw_node, int dir)
{
	grain_chain	*tmp_chain;

	char	*msg;

	int	gcsz = sizeof(grain_chain);

	if( prev_nw_node )
	{
		tmp_chain = (grain_chain *)MALLOC(gcsz);
		msg = "tmp_chain in get_shortest_path()";
		if( MEMCHECK(tmp_chain,msg,gcsz) ) clean_up(0);

		tmp_chain->index_in_cube = prev_nw_node->index_in_cube;

		if( dir == 1 )
		{
			ret_grain_chain->next = tmp_chain;
			ret_grain_chain->next->next = NULL;
			rec_mk_ret_chain(ret_grain_chain->next,last_link,
					 prev_nw_node->for_prev_path,dir);
		}
		else
		{
			tmp_chain->next = ret_grain_chain;
			rec_mk_ret_chain(tmp_chain,last_link,
					 prev_nw_node->bac_prev_path,dir);
		}
	}
	else	*last_link = ret_grain_chain;
}


#define CA_Edge_Vox()\
	nbr1 = n1z*nxy + n1y*nx + n1x;\
\
	check_and_add_edge_vox(cube,&tmp_inc,nbr1,closed_loop,&loop_ptr,cntr,\
							has_pore,gbvc)


#define	CA_3_Near_Vox()\
	nbr1 = n1z*nxy + n1y*nx + n1x;\
	nbr2 = n2z*nxy + n2y*nx + n2x;\
	nbr3 = n3z*nxy + n3y*nx + n3x;\
\
	check_and_add_3_near_vox(cube,&tmp_inc,nbr1,nbr2,nbr3,closed_loop,\
						&loop_ptr,cntr,has_pore,gbvc)


#define	CA_Vertex_Vox()\
	nbr1 = n1z*nxy + n1y*nx + n1x;\
	nbr2 = n2z*nxy + n2y*nx + n2x;\
\
	check_and_add_vertex_vox(cube,&tmp_inc,nbr1,nbr2,closed_loop,&loop_ptr,\
						     cntr,has_pore,gbvc)


int	get_6_path(int *cube, int *sten26, int nx, int ny, int nz,
			   int *loop_len, grain_chain *closed_loop, int *has_pore,
			   int gbvc)
{
	grain_chain	*loop_ptr;

	int	i;
	int	len_inc, tmp_inc;
	int	curr_ind, next_ind;
	int	cx, cy, cz;
	int	nex, ney, nez;
	int	n1x, n1y, n1z, n2x, n2y, n2z, n3x, n3y, n3z;
	int	nbr1, nbr2, nbr3;
	int	nxy;
	int	type, cntr;

	len_inc = tmp_inc = 0;

	nxy = nx*ny;

	loop_ptr = closed_loop;

	for( i = 0;  i < *loop_len;  i++ )
	{
	    if( loop_ptr )
	    {
		tmp_inc = 0;
		cntr = 0;
		curr_ind = loop_ptr->index_in_cube;
		if( loop_ptr->next ) next_ind = loop_ptr->next->index_in_cube;
		else		     next_ind = closed_loop->index_in_cube;

		Ind_2_ijk(curr_ind, cx, cy, cz,nx,nxy)
		Ind_2_ijk(next_ind,nex,ney,nez,nx,nxy)

		type = abs(nex - cx) + abs(ney - cy) + abs(nez - cz);

		while( cntr <= 1 )
		{
		    if( type == 1 )
		    {
			loop_ptr = loop_ptr->next;
			cntr = 2;
			continue;
		    }
		    else if( type == 2 )
		    {
			if( nez == cz )
			{
				n1z = n2z = cz;
				n1x = cx;
				n1y = ney;
				CA_Edge_Vox();

				if( tmp_inc == 0 )
				{
					n1x = nex;
					n1y = cy;
					CA_Edge_Vox();
				}
			}
			else if( ney == cy )
			{
				n1y = n2y = cy;
				n1x = cx;
				n1z = nez;
				CA_Edge_Vox();

				if( tmp_inc == 0 )
				{
					n1x = nex;
					n1z = cz;
					CA_Edge_Vox();
				}
			}
			else if( nex == cx )
			{
				n1x = n2x = cx;
				n1y = cy;
				n1z = nez;
				CA_Edge_Vox();
				if( tmp_inc == 0 )
				{
					n1y = ney;
					n1z = cz;
					CA_Edge_Vox();
				}
			}
			if( tmp_inc == 0 )
			{
			    if( nez == cz )
			    {
				if( cz -1 >= 0 )
				{
				    
				    n1z = n2z = n3z = cz-1;
				    n1x = cx;
				    n1y = n2y = cy;
				    n2x = n3x = nex;
				    n3y = ney;
				    CA_3_Near_Vox();

				    if( tmp_inc == 0 )
				    {
					n1z = n2z = n3z = cz-1;
					n1x = n2x = cx;
					n1y = cy;
					n2y = ney;
					n3x = nex;
					n3y = ney;
					CA_3_Near_Vox();
				    }
				}

				if( tmp_inc == 0 )
				{
				    if( cz +1 < nz )
				    {
					n1z = n2z = n3z = cz+1;
					n1x = cx;
					n1y = n2y = cy;
					n2x = n3x = nex;
					n3y = ney;
					CA_3_Near_Vox();

					if( tmp_inc == 0 )
					{
					    n1z = n2z = n3z = cz+1;
					    n1x = n2x = cx;
					    n1y = cy;
					    n2y = ney;
					    n3x = nex;
					    n3y = ney;
					    CA_3_Near_Vox();
					}
				    }
				}
			    }
			    if( ney == cy )
			    {
				if( cy - 1 >= 0 )
				{
				    
				    n1y = n2y = n3y = cy-1;
				    n1x = cx;
				    n1z = n2z = cz;
				    n2x = n3x = nex;
				    n3z = nez;
				    CA_3_Near_Vox();

				    if( tmp_inc == 0 )
				    {
					n1y = n2y = n3y = cy-1;
					n1x = n2x = cx;
					n1z = cz;
					n2z = nez;
					n3x = nex;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				}

				if( tmp_inc == 0 )
				{
				    if( cy + 1 < ny )
				    {
					n1y = n2y = n3y = cy+1;
					n1x = cx;
					n1z = n2z = cz;
					n2x = n3x = nex;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				    if( tmp_inc == 0 )
				    {
					n1y = n2y = n3y = cy+1;
					n1x = n2x = cx;
					n1z = cz;
					n2z = nez;
					n3x = nex;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				}
			    }

			    if( nex == cx )
			    {
				if( cx -1 >= 0 )
				{
				    n1x = n2x = n3x = cx-1;
				    n1y = cy;
				    n1z = n2z = cz;
				    n2y = n3y = ney;
				    n3z = nez;
				    CA_3_Near_Vox();

				    if( tmp_inc == 0 )
				    {
					n1x = n2x = n3x = cx-1;
					n1y = n2y = cy;
					n1z = cz;
					n2z = nez;
					n3y = ney;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				}
				if( tmp_inc == 0 )
				{
				    if( cx+1 < nx )
				    {
					n1x = n2x = n3x = cx+1;
					n1y = cy;
					n1z = n2z = cz;
					n2y = n3y = ney;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				    
				    if( tmp_inc == 0 )
				    {
					n1x = n2x = n3x = cx+1;
					n1y = n2y = cy;
					n1z = cz;
					n2z = nez;
					n3y = ney;
					n3z = nez;
					CA_3_Near_Vox();
				    }
				}
			    }
			}

			if( tmp_inc > 0 )
			{
				len_inc += tmp_inc;
				cntr = 2;
				continue;
			}
			else if( tmp_inc < 0 )
			{
				cntr = 2;
				continue;
			}
		    }
		    else if( type == 3 )
		    {
			n1z = cz;
			n1y = n2y = cy;
			n1x = n2x = nex;
			n2z = nez;
			CA_Vertex_Vox();

			if( tmp_inc == 0 )
			{
			    n1z = cz;
			    n1y = n2y = ney;
			    n1x = n2x = cx;
			    n2z = nez;
			    CA_Vertex_Vox();
			}

			if( tmp_inc == 0 )
			{
			    n1z = n2z = nez;
			    n1y = ney;
			    n2y = cy;
			    n1x = n2x = cx;
			    CA_Vertex_Vox();
			}

			if( tmp_inc == 0  )
			{
			    n1z = n2z = nez;
			    n1x = nex;
			    n2x = cx;
			    n1y = n2y = cy;
			    CA_Vertex_Vox();
			}

			if( tmp_inc == 0 )
			{
			    n1z = n2z = cz;
			    n1y = n2y = ney;
			    n1x = cx;
			    n2x = nex;
			    CA_Vertex_Vox();
			}

			if( tmp_inc == 0 )
			{
			    n1z = n2z = cz;
			    n1x = n2x = nex;
			    n1y = cy;
			    n2y = ney;
			    CA_Vertex_Vox();
			}

			if( tmp_inc > 0 )
			{
				len_inc += tmp_inc;
				cntr = 2;
				continue;
			}
			else if( tmp_inc < 0 )
			{
				cntr = 2;
				continue;
			}
		    }
		    else
		    {
			printf("Logic_error in get_6_path()\n");
			clean_up(0);
		    }
		    
		    if( (tmp_inc == 0) && (cntr == 1) )
		    {
			/*****
			fprintf(stderr,"Couldn't find simple 6 connected ");
			fprintf(stderr,"path in get_6_path()\n");
			fprintf(stdout,"Couldn't find simple 6 connected ");
			fprintf(stdout,"path in get_6_path()\n");
			*****/
			return 0;
		    }
		    else if( (tmp_inc == 0) && (cntr == 0) )
		    {
			/*****
		    	fprintf(stderr,"finding simple 6 connected path in ");
		    	fprintf(stderr,"get_6_path() with pore\n");
		    	fprintf(stdout,"finding simple 6 connected path in ");
		    	fprintf(stdout,"get_6_path() with pore\n");
			*****/
			cntr = 1;
		    }
		    else
		    {
			fprintf(stderr,"Error in logic in get_6_path()\n");
			fprintf(stdout,"Error in logic in get_6_path()\n");
			clean_up(0);
		    }
		}
	    }
	}
	*loop_len += len_inc;
	return 1;
}


void	check_and_add_edge_vox(int *cube, int *len_inc, int nbr1,
							   grain_chain *closed_loop,
							   grain_chain **loop_ptr, int cntr,
							   int *has_pore, int gbvc)
{
	grain_chain	*t_gc, *tmp_loop_ptr;

	char	*msg;

	int	tmp_ind, t_nbr1 = 0;
	int	ok, ok1;

	int	gcsz = sizeof(grain_chain);

	ok = 0;
	ok1 = ( (cube[nbr1] == 0) || (cube[nbr1] == BURNED) );

	if(    ( cntr == 0 && cube[nbr1] <= -1000 )
	    || ( cntr == 1 && ok1                 ) ) ok = 1;
	  
	if( ok )
	{
		tmp_loop_ptr = closed_loop;
		while( tmp_loop_ptr != NULL )
		{
			tmp_ind = tmp_loop_ptr->index_in_cube;
			if( tmp_ind == nbr1 )
			{
				t_nbr1 = -1;
				*len_inc = -1;
				*loop_ptr = (*loop_ptr)->next;
				break;
			}

			tmp_loop_ptr = tmp_loop_ptr->next;
		}
		if( t_nbr1 != -1 )
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz) ) clean_up(0);

			t_gc->index_in_cube = nbr1;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if( cntr == 1 )
			{
			    *has_pore = 1;

			    if( cube[nbr1] == 0 ) cube[nbr1] = -(1000+gbvc);
			    else		  cube[nbr1] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
	}
}


void	check_and_add_vertex_vox(int *cube, int *len_inc, int nbr1,
								 int nbr2, grain_chain *closed_loop,
								 grain_chain **loop_ptr, int cntr,
								 int *has_pore, int gbvc)
{
	grain_chain	*t_gc, *tmp_loop_ptr;

	char	*msg;

	int	tmp_ind, t_nbr1, t_nbr2;
	int	cn1, cn2;
	int	ok;

	int	gcsz = sizeof(grain_chain);

	t_nbr1 = t_nbr2 = 0;
	ok = 0;

		/*last test in if() arg list necessary as this routine   */
		/* is called in check_and_add_3_near_vox() with cntr = 1 */

	cn1 = cube[nbr1];		cn2 = cube[nbr2];

	if(    ( (cntr == 0) && (cn1 <=  -1000) && (cn2 <=  -1000) )
	    || ( (cntr == 1) && (cn1 ==      0) && (cn2 ==      0) )
	    || ( (cntr == 1) && (cn1 ==      0) && (cn2 <=  -1000) )
	    || ( (cntr == 1) && (cn1 ==      0) && (cn2 == BURNED) )
	    || ( (cntr == 1) && (cn1 <=  -1000) && (cn2 ==      0) )
	    || ( (cntr == 1) && (cn1 <=  -1000) && (cn2 <=  -1000) )
	    || ( (cntr == 1) && (cn1 <=  -1000) && (cn2 == BURNED) )
	    || ( (cntr == 1) && (cn1 == BURNED) && (cn2 ==      0) )
	    || ( (cntr == 1) && (cn1 == BURNED) && (cn2 <=  -1000) )    )
		ok = 1;

	if( ok )
	{
		tmp_loop_ptr = closed_loop;
		while( tmp_loop_ptr != NULL )
		{
			tmp_ind = tmp_loop_ptr->index_in_cube;

			     if( tmp_ind == nbr1 ) t_nbr1 = -1;
			else if( tmp_ind == nbr2 ) t_nbr2 = -1;

			tmp_loop_ptr = tmp_loop_ptr->next;
		}
		if( (t_nbr1 == -1) && (t_nbr2 == -1) )
		{
			*len_inc = -1;
			*loop_ptr = (*loop_ptr)->next;
		}
		else if( (t_nbr1 != -1) && (t_nbr2 != -1) )
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg, gcsz)) clean_up(0);

			t_gc->index_in_cube = nbr1;

			t_gc->next = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc->next in get_6_path()";
			if( MEMCHECK(t_gc->next,msg,gcsz)) clean_up(0);

			t_gc->next->index_in_cube = nbr2;
			t_gc->next->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 2;

			if( cntr == 1 )
			{
			    if( cube[nbr1] == 0 || cube[nbr1] == BURNED )
			    {
				*has_pore = 1;

				if( cube[nbr1] == 0 ) cube[nbr1] = -(1000+gbvc);
				else		      cube[nbr1] = -(1000+gbvc+1);
			    }
			    if( cube[nbr2] == 0 || cube[nbr2] == BURNED )
			    {
				*has_pore = 1;

				if( cube[nbr2] == 0 ) cube[nbr2] = -(1000+gbvc);
				else		      cube[nbr2] = -(1000+gbvc+1);
			    }
			}
			*loop_ptr = t_gc->next->next;
		}
		else if( t_nbr1 != -1)
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz)) clean_up(0);

			t_gc->index_in_cube = nbr1;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if(    (cntr == 1)
			    && (cube[nbr1] == 0 || cube[nbr1] == BURNED)   )
			{
				*has_pore = 1;
				if( cube[nbr1] == 0 ) cube[nbr1] = -(1000+gbvc);
				else		      cube[nbr1] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
		else if( t_nbr2 != -1)
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz)) clean_up(0);

			t_gc->index_in_cube = nbr2;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if(    (cntr == 1)
			    && (cube[nbr2] == 0 || cube[nbr2] == BURNED)    )
			{
				*has_pore = 1;
				if( cube[nbr2] == 0 ) cube[nbr2] = -(1000+gbvc);
				else		      cube[nbr2] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
	}
}


void	check_and_add_3_near_vox(int *cube, int *len_inc, int nbr1,
								 int nbr2, int nbr3,
								 grain_chain *closed_loop,
								 grain_chain **loop_ptr, int cntr,
								 int *has_pore, int gbvc)
{
	grain_chain	*t_gc, *tmp_loop_ptr;

	char	*msg;

	int	ok;
	int	tmp_ind;
	int	t_nbr1, t_nbr2, t_nbr3;
	int	cn1, cn2, cn3;

	int	gcsz = sizeof(grain_chain);

	ok = 0;
	cn1 = cube[nbr1];	cn2 = cube[nbr2];	cn3 = cube[nbr3];

	if(    ( cntr == 0 && cn1 <=  -1000 && cn2 <=  -1000 && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 ==      0 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 ==      0 && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 ==      0 && cn3 == BURNED )
	    || ( cntr == 1 && cn1 ==      0 && cn2 <=  -1000 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 == BURNED && cn3 ==      0 )
	    || ( cntr == 1 && cn1 <=  -1000 && cn2 ==      0 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 == BURNED && cn2 ==      0 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 <=  -1000 && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 ==      0 && cn2 <=  -1000 && cn3 == BURNED )
	    || ( cntr == 1 && cn1 ==      0 && cn2 == BURNED && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 <=  -1000 && cn2 ==      0 && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 == BURNED && cn2 ==      0 && cn3 <=  -1000 )
	    || ( cntr == 1 && cn1 <=  -1000 && cn2 ==      0 && cn3 == BURNED )
	    || ( cntr == 1 && cn1 == BURNED && cn2 ==      0 && cn3 == BURNED )
	    || ( cntr == 1 && cn1 <=  -1000 && cn2 <=  -1000 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 == BURNED && cn2 <=  -1000 && cn3 ==      0 )
	    || ( cntr == 1 && cn1 <=  -1000 && cn2 == BURNED && cn3 ==      0 )
	    || ( cntr == 1 && cn1 == BURNED && cn2 == BURNED && cn3 ==      0 )
	    || ( cntr == 1 && cn1 == BURNED && cn2 == BURNED && cn3 == BURNED ) )
		ok = 1;

	t_nbr1 = t_nbr2 = t_nbr3 = 0;

	if( ok )
	{
		tmp_loop_ptr = closed_loop;
		while( tmp_loop_ptr != NULL )
		{
			tmp_ind = tmp_loop_ptr->index_in_cube;

			     if( tmp_ind == nbr1 ) t_nbr1 = -1;
			else if( tmp_ind == nbr2 ) t_nbr2 = -1;
			else if( tmp_ind == nbr3 ) t_nbr3 = -1;

			tmp_loop_ptr = tmp_loop_ptr->next;
		}
		if( (t_nbr1 == -1) && (t_nbr2 == -1) && (t_nbr3 == -1) )
		{
			*len_inc = -1;
			*loop_ptr = (*loop_ptr)->next;
		}
		if( (t_nbr1 != -1) && (t_nbr2 != -1) && (t_nbr3 != -1) )
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz) ) clean_up(0);

			t_gc->index_in_cube = nbr1;

			t_gc->next = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc->next in get_6_path()";
			if( MEMCHECK(t_gc->next,msg,gcsz) ) clean_up(0);

			t_gc->next->index_in_cube = nbr2;

			t_gc->next->next = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc->next->next in get_6_path()";
			if( MEMCHECK(t_gc->next->next,msg,gcsz) ) clean_up(0);

			t_gc->next->next->index_in_cube = nbr3;
			t_gc->next->next->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 3;
			*loop_ptr = t_gc->next->next->next;
		}
		else if( (t_nbr1 != -1) && (t_nbr2 != -1) )
			check_and_add_vertex_vox(cube,len_inc,nbr1,nbr2,
						closed_loop,loop_ptr,cntr,
								has_pore,gbvc);
		else if( (t_nbr1 != -1) && (t_nbr3 != -1) )
			check_and_add_vertex_vox(cube,len_inc,nbr1,nbr3,
						closed_loop,loop_ptr,cntr,
								has_pore,gbvc);
		else if( (t_nbr2 != -1) && (t_nbr3 != -1) )
			check_and_add_vertex_vox(cube,len_inc,nbr2,nbr3,
						closed_loop,loop_ptr,cntr,
								has_pore,gbvc);
		else if( t_nbr1 != -1)
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz) ) clean_up(0);

			t_gc->index_in_cube = nbr1;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if(    (cntr == 1)
			    && (cube[nbr1] == 0 || cube[nbr1] == BURNED) )
			{
				*has_pore = 1;

				if( cube[nbr1] == 0 ) cube[nbr1] = -(1000+gbvc);
				else		      cube[nbr1] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
		else if( t_nbr2 != -1)
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz) ) clean_up(0);

			t_gc->index_in_cube = nbr2;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if(    (cntr == 1)
			   && (cube[nbr2] == 0 || cube[nbr2] == BURNED) )
			{
				*has_pore = 1;

				if( cube[nbr2] == 0 ) cube[nbr2] = -(1000+gbvc);
				else		      cube[nbr2] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
		else if( t_nbr3 != -1)
		{
			t_gc = (grain_chain *)MALLOC(gcsz);
			msg = "t_gc in get_6_path()";
			if( MEMCHECK(t_gc,msg,gcsz) ) clean_up(0);

			t_gc->index_in_cube = nbr3;
			t_gc->next = (*loop_ptr)->next;
			(*loop_ptr)->next = t_gc;
			*len_inc += 1;

			if(    (cntr == 1)
			    && (cube[nbr3] == 0 || cube[nbr3] == BURNED)    )
			{
				*has_pore = 1;

				if( cube[nbr3] == 0 ) cube[nbr3] = -(1000+gbvc);
				else		      cube[nbr3] = -(1000+gbvc+1);
			}
			*loop_ptr = t_gc->next;
		}
	}

}


void	free_network(network_node *nw_node)
{
	if( nw_node->next        ) free_network(nw_node->next);
	if( nw_node->nw_adj_list ) free_nw_adj_list(nw_node->nw_adj_list);
	FREE(nw_node,sizeof(network_node));
}


void	free_nw_adj_list(network_adj_list *nw_adj_list)
{
	if( nw_adj_list->next ) free_nw_adj_list(nw_adj_list->next);
	FREE(nw_adj_list,sizeof(network_adj_list));
}


void	free_g_chain(grain_chain *g_chain)
{
	if( g_chain->next ) free_g_chain(g_chain->next);
	FREE(g_chain,sizeof(grain_chain));
}


void	free_g_change_list(g_change_list *g_chain)
{
	if( g_chain->next ) free_g_change_list(g_chain->next);
	FREE(g_chain,sizeof(g_change_list));
}


void	dbg_print_network(network_node *nw_node, int *kount, int dir)
{
	printf("(%d) %d ",*kount,nw_node->index_in_cube);
	*kount += 1;

	if( dir == 1 )
	{
		if( nw_node->next ) dbg_print_network(nw_node->next,kount,dir);
		else		    printf("\n");
	}
	else if( dir == 2 )
	{
		if( nw_node->back_pointer )
			dbg_print_network(nw_node->back_pointer,kount,dir);
		else
			printf("\n");
	}
	else	printf("Error in print network\n");
}


#define	Bi_Dir_Params\
\
	static	float	cost[3] = {(float)0.9016, (float)1.289, (float)1.615};\
	static	float	TOL = (float)0.00001;\
	static	float	TOL1 = (float)0.000001;\
	static	int	realloc_param = 100;\
\
	network_node	*n_nw_node, *min_nw_node;\
	network_node	*tmp_node_list[26];\
\
	network_adj_list *p_nw_adj_list;\
	grain_chain	 *shortest_path;\
	tmp_indx_chain	 *ttmp_indx_chain, *ttmp_indx_chain1,\
			 *ntmp_indx_chain, *nnwnpiich;\
	tmp_indx_chain	 *start_search_here, *pstart_search_here,\
			 *tmp_start_search_here;\
\
	char	*msg;\
	char	tmp_lbl;\
\
	float	v0[3], v_prev[3];\
	float	min_angle, min_3d;\
	float	nnwfl, min_d_theta, d_theta, area1, p_dist;\
\
	int	indx[26];\
	int	i, first;\
	int	g_index, ind0;\
	int	tmp1, tmp2, tmp3;\
	int	nbr_kount, stop_search, start_search_set;\
	int	o_x, o_y, o_z;\
\
	int	  isz = sizeof(int);\
	int	pnnsz = sizeof(network_node *);\
	int	ticsz = sizeof(tmp_indx_chain)


#define	Bi_Dir_Init\
	shortest_path = (grain_chain  *) NULL;\
	min_nw_node   = (network_node *) NULL;\
	ttmp_indx_chain1 = (tmp_indx_chain *) NULL;\
\
	min_d_theta = -t_pi;	/* note t_pi = -2*pi in Bi_Dir_bac_() */\



grain_chain *Bi_Dir_for_Label(network_node **start_node,
							  tmp_indx_chain **ptmp_indx_chain,
							  gr_bdry_vect_elem *gr_bdry_vect, MoI *moi,
							  int *cube, int nx, int nxy, int grain_index,
							  int *ok, FILE *fp1, int min_surf_dijk,
							  float v_term[3], float const_d)
{
	Bi_Dir_Params;

	static	float	t_pi = (float)(2.0*M_PI);

	float	i_fang, i_flbl, ip1_flbl, min_flbl, i_f3d;

	Bi_Dir_Init;

	first = 1;

	ttmp_indx_chain = *ptmp_indx_chain;
	while( ttmp_indx_chain )
	{
	     i_fang = ttmp_indx_chain->nw_node->forward_angle;
	     i_flbl = ttmp_indx_chain->nw_node->forward_label;

	    if( ttmp_indx_chain->next )
			ip1_flbl= ttmp_indx_chain->next->nw_node->forward_label;
		if( min_surf_dijk == 0  || min_surf_dijk == 3 )
		{
		    d_theta = i_fang
			    - ttmp_indx_chain->nw_node->for_prev_path->forward_angle;
		    if( d_theta > 0.0 && (fabs(d_theta) >TOL) )
		    {
		        if( first )
		        {
			min_nw_node = ttmp_indx_chain->nw_node;
			min_angle   = i_fang;
			first = 0;
			ttmp_indx_chain1 = ttmp_indx_chain;
			if(    (ttmp_indx_chain->next == NULL)
			    || (fabs(i_flbl - ip1_flbl) > TOL)    ) break;
			}
			else if( fabs(min_nw_node->forward_label - i_flbl) <= TOL )
			{
			    if( min_angle < i_fang )
			    {
			  	ttmp_indx_chain1 = ttmp_indx_chain;
				min_nw_node = ttmp_indx_chain->nw_node;
				min_angle   = i_fang;
			    }

			    if(    (ttmp_indx_chain->next == NULL)
				   || (fabs(i_flbl - ip1_flbl) > TOL)   ) break;
			}
			else break;
		    }
		    else if( first && (d_theta > min_d_theta) )
		      {
			ttmp_indx_chain1 = ttmp_indx_chain;
			min_nw_node = ttmp_indx_chain->nw_node;
			min_angle   = i_fang;
			min_d_theta = d_theta;
		      }
		    else if(    (! first)
				&& (fabs(min_nw_node->forward_label - i_flbl) > TOL) )
		      break; 
		}
		else if ( min_surf_dijk == 1 )
		{
		    i_f3d = ttmp_indx_chain->nw_node->forward_3d_area;
		    if( first )
		    {
			min_nw_node = ttmp_indx_chain->nw_node;
			min_angle   = i_fang;
			min_3d = i_f3d;
			first = 0;
			ttmp_indx_chain1 = ttmp_indx_chain;
			if(    (ttmp_indx_chain->next == NULL)
			    || (fabs(i_flbl - ip1_flbl) > TOL)    ) break;
		    }
		    else if( fabs(min_nw_node->forward_label - i_flbl) <= TOL )
		    {
		    	if( min_3d > i_f3d )
			{
			  	ttmp_indx_chain1 = ttmp_indx_chain;
				min_nw_node = ttmp_indx_chain->nw_node;
				min_3d = i_f3d;
			}

			if(    (ttmp_indx_chain->next == NULL)
			    || (fabs(i_flbl - ip1_flbl) > TOL)   ) break;
		    }
		    else break;
		}
		else if ( min_surf_dijk == 2 )
		{
		    min_nw_node = ttmp_indx_chain->nw_node;
		    ttmp_indx_chain1 = ttmp_indx_chain;
		    break;
		}
		else if ( min_surf_dijk == 4 )
		{
		    if( first )
		    {
			min_nw_node = ttmp_indx_chain->nw_node;
			min_angle   = i_fang;
			first = 0;
			ttmp_indx_chain1 = ttmp_indx_chain;
			if(    (ttmp_indx_chain->next == NULL)
			    || (fabs(i_flbl - ip1_flbl) > TOL)    ) break;
		    }
		    else if( fabs(min_nw_node->forward_label - i_flbl) <= TOL )
		    {
			if( min_angle < i_fang )
			{
				ttmp_indx_chain1 = ttmp_indx_chain;
				min_nw_node = ttmp_indx_chain->nw_node;
				min_angle   = i_fang;
			}

			if(    (ttmp_indx_chain->next == NULL)
			    || (fabs(i_flbl - ip1_flbl) > TOL)   ) break;
		    }
		    else break;
		}
		ttmp_indx_chain = ttmp_indx_chain->next;
	}
	
	if( !min_nw_node ) return shortest_path;

	if( ttmp_indx_chain1 == *ptmp_indx_chain )
		*ptmp_indx_chain = (*ptmp_indx_chain)->next;
	
	min_nw_node->for_in_indx_chain = NULL;
	start_search_set = 0;

	if( ttmp_indx_chain1->prev )
	{
	 	(ttmp_indx_chain1->prev)->next = ttmp_indx_chain1->next;
		start_search_set = 1;
	}
	if( ttmp_indx_chain1->next )
	{
		(ttmp_indx_chain1->next)->prev = ttmp_indx_chain1->prev;
		start_search_set = 2;
	}		

	if( start_search_set == 1 )
		start_search_here = ttmp_indx_chain1->prev;
	else if( start_search_set == 2 )
		start_search_here = ttmp_indx_chain1->next;
	else
		start_search_here = NULL;

	FREE(ttmp_indx_chain1,sizeof(tmp_indx_chain));

	nbr_kount = 0;

	if( min_nw_node->b_tmp != 0 )
	{
	    if( fp1 ) print_min_node_data(min_nw_node,1,nx,nxy,fp1);

	    min_nw_node->f_tmp = 0;
	    *start_node = del_join_for_link(*start_node,min_nw_node);

	    ind0 = min_nw_node->index_in_cube;
	    Pad_ind_2_ijk(ind0,o_x,o_y,o_z,nx,nxy)
	    v_prev[0] = (float)o_x - (float)(moi->comx);
	    v_prev[1] = (float)o_y - (float)(moi->comy);
	    v_prev[2] = (float)o_z - (float)(moi->comz);

	    project_2_plane(v_prev,&(moi->pa[3]),&(moi->pa[6]));

	    p_nw_adj_list = min_nw_node->nw_adj_list;
	    while( p_nw_adj_list != NULL )
	    {
			g_index = -(cube[p_nw_adj_list->index_in_cube]+1000);
			n_nw_node = gr_bdry_vect[g_index].nw_node;
			tmp_lbl = n_nw_node->f_tmp;

			if( tmp_lbl != 0 )
			{
				nnwfl = n_nw_node->forward_label;

				min_flbl  = min_nw_node->forward_label;

				ind0 = n_nw_node->index_in_cube;
				Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
				v0[0] = (float)tmp1 - (float)(moi->comx);
				v0[1] = (float)tmp2 - (float)(moi->comy);
				v0[2] = (float)tmp3 - (float)(moi->comz);

				project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));
				if( min_surf_dijk == 1 )
				{
					area1 = triangle_area1(0.0,0.0,0.0,v_prev[0],v_prev[1],
							v_prev[2],v0[0],v0[1],v0[2]);
				if( area1 < TOL1 )
				{
					fprintf(stderr,"area1 very small\n");
					fprintf(stdout,"area1 very small\n");
				}
				min_flbl += area1;
			}
		    else if( (min_surf_dijk == 2) || (min_surf_dijk == 3) )
		    {
				area1 = triangle_area1((float)moi->comx,(float)moi->comy,
							   (float)moi->comz,(float)o_x,
							   (float)o_y,(float)o_z,(float)tmp1,
							   (float)tmp2,(float)tmp3);
				min_flbl += area1;
		    }
		    else if ( min_surf_dijk == 0 )
		    	min_flbl += cost[p_nw_adj_list->length];

		    if( min_surf_dijk == 4 )
		    {
		    	if( tmp_lbl == 2 )
				{
					n_nw_node->f_tmp = 1;
					tmp_node_list[nbr_kount] = n_nw_node;
					nbr_kount++;
					p_dist = moi->pa[0]*tmp1 + moi->pa[1]*tmp2
					   + moi->pa[2]*tmp3 + const_d;
					p_dist = (float)fabs(p_dist);
					n_nw_node->forward_label = p_dist;

					area1 = triangle_area1((float)moi->comx,
							   (float)moi->comy,
							   (float)moi->comz,(float)o_x,
							   (float)o_y,(float)o_z,
							   (float)tmp1,(float)tmp2,
							   (float)tmp3);
					n_nw_node->forward_3d_area
						= min_nw_node->forward_3d_area + area1;
					n_nw_node->for_prev_path = min_nw_node;
					n_nw_node->forward_angle
						= calc_d_theta(v_term,v0,&(moi->pa[0]),'n');
				}
				else
				{
					area1 = triangle_area1((float)moi->comx,
							   (float)moi->comy,
							   (float)moi->comz,(float)o_x,
							   (float)o_y,(float)o_z,
							   (float)tmp1,(float)tmp2,
							   (float)tmp3);
					area1 += min_nw_node->forward_3d_area;

					if(    (area1 < n_nw_node->forward_3d_area)
					&& (fabs(area1-n_nw_node->forward_3d_area) > TOL) )
					{
						n_nw_node->forward_3d_area = area1;
						n_nw_node->for_prev_path = min_nw_node;
					}
				}
		    }
		    else if( nnwfl >   min_flbl )
		    {
			if( n_nw_node->f_tmp == 1 )
			{
			    nnwnpiich = n_nw_node->for_in_indx_chain;

			    if(     start_search_here
				&& (nnwnpiich == start_search_here)    )
			    {
			  	if( start_search_here->next )
					start_search_here
						= start_search_here->next;
				else if( start_search_here->prev)
					start_search_here
						= start_search_here->prev;
				else
				  	start_search_here = NULL;
			    }
			    if( nnwnpiich == *ptmp_indx_chain )
				*ptmp_indx_chain = (*ptmp_indx_chain)->next;

			    if( nnwnpiich->prev)
				nnwnpiich->prev->next= nnwnpiich->next;

			    if(nnwnpiich->next)
				nnwnpiich->next->prev = nnwnpiich->prev;

			    FREE(nnwnpiich,sizeof(tmp_indx_chain));
				n_nw_node->for_in_indx_chain = NULL;
			}
			else	n_nw_node->f_tmp = 1;

			if(    (min_surf_dijk == 1)
			    || (min_surf_dijk == 2)
			    || (min_surf_dijk == 3)    )
			{
			    area1 = triangle_area1((float)moi->comx,
						   (float)moi->comy,
						   (float)moi->comz,(float)o_x,
						   (float)o_y,(float)o_z,
						   (float)tmp1,(float)tmp2,
						   (float)tmp3);

			    n_nw_node->forward_3d_area
				= min_nw_node->forward_3d_area + area1;
			}
			n_nw_node->forward_label = min_flbl;
			n_nw_node->for_prev_path = min_nw_node;

			d_theta = calc_d_theta(v_prev,v0,&(moi->pa[0]),'n');
			n_nw_node->forward_angle = d_theta + min_angle;
			tmp_node_list[nbr_kount] = n_nw_node;
			nbr_kount++;
		    }
		}
		p_nw_adj_list = p_nw_adj_list->next;
	    }
	    if( nbr_kount >= 1 ) indexx_for(nbr_kount,tmp_node_list-1,indx-1);

	    tmp_start_search_here = start_search_here;
	    for( i = 0; i < nbr_kount; i++ )
	    {
		if( tmp_start_search_here == NULL )
		{
		    *ptmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
		    msg = "*ptmp_indx_chain in Bi_Dir_for_Label()";
		    if( MEMCHECK(*ptmp_indx_chain,msg,ticsz) ) clean_up(0);

		    (*ptmp_indx_chain)->prev = (tmp_indx_chain*) NULL;
		    (*ptmp_indx_chain)->nw_node = tmp_node_list[indx[0]-1];
		    (*ptmp_indx_chain)->next = (tmp_indx_chain*) NULL;

		    start_search_here = *ptmp_indx_chain;
		    tmp_start_search_here= start_search_here;
		    tmp_node_list[indx[0]-1]->for_in_indx_chain
						= *ptmp_indx_chain;
		}
		else
		{
		    pstart_search_here = tmp_start_search_here;
		    stop_search = 0;
		    if( pstart_search_here->nw_node->forward_label
			 > tmp_node_list[indx[i]-1]->forward_label )
		    {
			while( !stop_search )
			{
			    if(    pstart_search_here->prev
				&& pstart_search_here->prev->nw_node->forward_label
					>= tmp_node_list[indx[i]-1]->forward_label )
			    {
				pstart_search_here = pstart_search_here->prev;
			    }
			    else
			    {
				ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
				msg = "ntmp_indx_chain in Bi_Dir_for_Label()";
				if( MEMCHECK(ntmp_indx_chain,msg,ticsz) )
					clean_up(0);
				ntmp_indx_chain->nw_node
					= tmp_node_list[indx[i]-1];

				ntmp_indx_chain->next = pstart_search_here;
				tmp_node_list[indx[i]-1]->for_in_indx_chain
					= ntmp_indx_chain;
				if( pstart_search_here->prev == NULL )
				{
					ntmp_indx_chain->prev = NULL;
					*ptmp_indx_chain = ntmp_indx_chain;
				}
				else
				{
					ntmp_indx_chain->prev = pstart_search_here->prev;
					pstart_search_here->prev->next = ntmp_indx_chain;
				}
				pstart_search_here->prev = ntmp_indx_chain;
				tmp_start_search_here = ntmp_indx_chain;
				stop_search = 1;
			    }
			}
		    }
		    else if( pstart_search_here->nw_node->forward_label
				< tmp_node_list[indx[i]-1]->forward_label )
		    {
			while( !stop_search )
			{
			    if(    pstart_search_here->next
				&& pstart_search_here->next->nw_node->forward_label
					<= tmp_node_list[indx[i]-1]->forward_label )
			    {
				pstart_search_here = pstart_search_here->next;
			    }
			    else
			    {
				ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
				msg = "ntmp_indx_chain in Bi_Dir_for_Label()";
				if( MEMCHECK(ntmp_indx_chain,msg,ticsz) )
					clean_up(0);
				ntmp_indx_chain->nw_node = tmp_node_list[indx[i]-1];
				ntmp_indx_chain->prev= pstart_search_here;
				tmp_node_list[indx[i]-1]->for_in_indx_chain
							= ntmp_indx_chain;

				if( pstart_search_here->next == NULL )
					ntmp_indx_chain->next = NULL;
				else
				{
				    pstart_search_here->next->prev
						= ntmp_indx_chain;
				    ntmp_indx_chain->next
						= pstart_search_here->next;
				}
				pstart_search_here->next = ntmp_indx_chain;
				tmp_start_search_here = ntmp_indx_chain;
				stop_search = 1;
			    }
			}
		    }
		    else
		    {
			ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
			msg = "ntmp_indx_chain in Bi_Dir_for_Label()";
			if( MEMCHECK(ntmp_indx_chain,msg,ticsz) ) clean_up(0);

			ntmp_indx_chain->nw_node = tmp_node_list[indx[i]-1];
			ntmp_indx_chain->prev= pstart_search_here;
			tmp_node_list[indx[i]-1]->for_in_indx_chain
							= ntmp_indx_chain;

			if( pstart_search_here->next == NULL )
				ntmp_indx_chain->next = NULL;
			else
			{
			    pstart_search_here->next->prev = ntmp_indx_chain;
			    ntmp_indx_chain->next = pstart_search_here->next;
			}
			pstart_search_here->next = ntmp_indx_chain;
			tmp_start_search_here = ntmp_indx_chain;
			stop_search = 1;
		    }
		}
	    }
	}
	else
	{
	    min_nw_node->f_tmp = 0;
	    shortest_path = return_shortest_path(min_nw_node,1,grain_index);
	    *ok = 0;
	}
	return	shortest_path;
}


network_node *del_join_for_link(network_node *p_nw_node,
								network_node *p_e_node)
{
	network_node	*tmp_nw_node;

	tmp_nw_node = p_nw_node->next;
	if( tmp_nw_node != p_e_node )
	{
		p_nw_node->next = p_e_node;
		if( tmp_nw_node ) tmp_nw_node->back_pointer = p_e_node;

		if( p_e_node->back_pointer )
			p_e_node->back_pointer->next = p_e_node->next;

		if( p_e_node->next )
			p_e_node->next->back_pointer = p_e_node->back_pointer;

		p_e_node->back_pointer = p_nw_node;
		p_e_node->next = tmp_nw_node;
	}

	return p_e_node;
}


grain_chain *Bi_Dir_bac_Label(network_node **start_node,
							  tmp_indx_chain **ptmp_indx_chain,
							  gr_bdry_vect_elem *gr_bdry_vect, MoI *moi,
							  int *cube, int nx, int nxy, int grain_index,
							  int *ok, FILE *fp2, int min_surf_dijk,
							  float v_term[3], float const_d)
{
	Bi_Dir_Params;

	static	float	t_pi = (float)(-2.0*M_PI);

	float	i_bang, i_blbl, ip1_blbl, min_blbl, i_b3d;

	Bi_Dir_Init;

	first = 1;

	ttmp_indx_chain = *ptmp_indx_chain;
	while( ttmp_indx_chain )
	{
	    i_bang = ttmp_indx_chain->nw_node->backward_angle;
	    i_blbl = ttmp_indx_chain->nw_node->backward_label;

	    if( ttmp_indx_chain-> next )
			ip1_blbl = ttmp_indx_chain->next->nw_node->backward_label;
		if( min_surf_dijk == 0  || min_surf_dijk == 3 )
		{
		    d_theta = i_bang - ttmp_indx_chain->nw_node->bac_prev_path->backward_angle;
		    if( d_theta < 0.0 && (fabs(d_theta) > TOL ) )
		    {
		        if( first )
		        {
					min_nw_node = ttmp_indx_chain->nw_node;
					min_angle   = i_bang;
					first = 0;
					ttmp_indx_chain1 = ttmp_indx_chain;
					if(    (ttmp_indx_chain->next == NULL)
						|| (fabs(i_blbl - ip1_blbl) > TOL)    ) break;
				}
				else if( fabs(min_nw_node->backward_label - i_blbl) <= TOL )
				{
					if( min_angle > i_bang )
					{
			  			ttmp_indx_chain1 = ttmp_indx_chain;
						min_nw_node = ttmp_indx_chain->nw_node;
						min_angle   = i_bang;
					}
					if(    (ttmp_indx_chain->next == NULL)
					   || (fabs(i_blbl - ip1_blbl) > TOL)   ) break;
				}
				else break;
		    }
		    else if( first && (d_theta < min_d_theta) )
		    {
		    	ttmp_indx_chain1 = ttmp_indx_chain;
				min_nw_node = ttmp_indx_chain->nw_node;
				min_angle   = i_bang;
				min_d_theta = d_theta;
		    }
		    else if(    (! first)
			     && (fabs(min_nw_node->backward_label - i_blbl) > TOL)   )
		      break;
		}
		else if (min_surf_dijk == 1 )
		{
		    i_b3d = ttmp_indx_chain->nw_node->backward_3d_area;
		    if( first )
		    {
				min_nw_node = ttmp_indx_chain->nw_node;
				min_angle   = i_bang;
				min_3d = i_b3d;
				first = 0;
				ttmp_indx_chain1 = ttmp_indx_chain;
				if(    (ttmp_indx_chain->next == NULL)
					|| (fabs(i_blbl - ip1_blbl) > TOL)    ) break;
		    }
		    else if( fabs(min_nw_node->backward_label - i_blbl) <= TOL )
		    {
		    	if( min_3d > i_b3d )
				{
					ttmp_indx_chain1 = ttmp_indx_chain;
					min_nw_node = ttmp_indx_chain->nw_node;
					min_3d   = i_b3d;
				}

				if(    (ttmp_indx_chain->next == NULL)
					|| (fabs(i_blbl - ip1_blbl) > TOL)   ) break;
		    }
		    else break;
		}
		else if ( min_surf_dijk == 2 )
		{
			min_nw_node = ttmp_indx_chain->nw_node;
			ttmp_indx_chain1 = ttmp_indx_chain;
			break;
		}
		else if ( min_surf_dijk == 4 )
		{
			if( first )
			{
				min_nw_node = ttmp_indx_chain->nw_node;
				min_angle   = i_bang;
				first = 0;
				ttmp_indx_chain1 = ttmp_indx_chain;
				if(    (ttmp_indx_chain->next == NULL)
					|| (fabs(i_blbl - ip1_blbl) > TOL)    ) break;
			}
	        else if( fabs(min_nw_node->backward_label - i_blbl) <= TOL )
	        {
				if( min_angle > i_bang )
				{
					ttmp_indx_chain1 = ttmp_indx_chain;
					min_nw_node = ttmp_indx_chain->nw_node;
					min_angle   = i_bang;
				}

				if(   (ttmp_indx_chain->next == NULL)
				   || (fabs(i_blbl - ip1_blbl) > TOL)   ) break;
			}
			else break;
		}
		ttmp_indx_chain = ttmp_indx_chain->next;
	}
	
	if( !min_nw_node ) return shortest_path;

	if( ttmp_indx_chain1 == *ptmp_indx_chain )
		*ptmp_indx_chain = (*ptmp_indx_chain)->next;

	min_nw_node->bac_in_indx_chain = NULL;
	start_search_set = 0;
	if( ttmp_indx_chain1->prev )
	{
	 	(ttmp_indx_chain1->prev)->next = ttmp_indx_chain1->next;
		start_search_set = 1;
	}
	if(ttmp_indx_chain1->next)
	{
		(ttmp_indx_chain1->next)->prev = ttmp_indx_chain1->prev;
		start_search_set = 2;
	}		
	if( start_search_set == 1 )
		start_search_here = ttmp_indx_chain1->prev;
	else if( start_search_set == 2 )
		start_search_here = ttmp_indx_chain1->next;
	else	start_search_here = NULL;
	
	FREE(ttmp_indx_chain1,sizeof(tmp_indx_chain));

	nbr_kount = 0;

	if( min_nw_node->f_tmp != 0 )
	{
	    if( fp2 ) print_min_node_data(min_nw_node,2,nx,nxy,fp2);

	    min_nw_node->b_tmp = 0;
	    *start_node = del_join_bac_link(*start_node,min_nw_node);

	    ind0 = min_nw_node->index_in_cube;
	    Pad_ind_2_ijk(ind0,o_x,o_y,o_z,nx,nxy)
	    v_prev[0] = (float)o_x - (float)(moi->comx);
	    v_prev[1] = (float)o_y - (float)(moi->comy);
	    v_prev[2] = (float)o_z - (float)(moi->comz);

	    project_2_plane(v_prev,&(moi->pa[3]),&(moi->pa[6]));

	    p_nw_adj_list = min_nw_node->nw_adj_list;
	    while( p_nw_adj_list != NULL )
	    {
			g_index = -(cube[p_nw_adj_list->index_in_cube]+1000);
			n_nw_node = gr_bdry_vect[g_index].nw_node;
			tmp_lbl = n_nw_node->b_tmp;

			if( tmp_lbl != 0 )
			{
				nnwfl = n_nw_node->backward_label;

				min_blbl  = min_nw_node->backward_label;
				/****
				min_blbl += cost[p_nw_adj_list->length];
				****/

				ind0 = n_nw_node->index_in_cube;
				Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
				v0[0] = (float)tmp1 - (float)(moi->comx);
				v0[1] = (float)tmp2 - (float)(moi->comy);
				v0[2] = (float)tmp3 - (float)(moi->comz);

				project_2_plane(v0,&(moi->pa[3]),&(moi->pa[6]));
				if( min_surf_dijk == 1 )
				{
					area1 = triangle_area1(0.0,0.0,0.0,v_prev[0],v_prev[1],
							v_prev[2],v0[0],v0[1],v0[2]);
					if( area1 < TOL1 )
					{
						fprintf(stderr,"area1 very small\n");
						fprintf(stdout,"area1 very small\n");
					}
					min_blbl += area1;
				}
				else if( min_surf_dijk == 2 || min_surf_dijk == 3 )
				{
					area1 = triangle_area1((float)moi->comx,(float)moi->comy,
								   (float)moi->comz,(float)o_x,
								   (float)o_y,(float)o_z,
								   (float)tmp1,(float)tmp2,
								   (float)tmp3);
					min_blbl += area1;
				}
				else if( min_surf_dijk == 0 )
		    		min_blbl += cost[p_nw_adj_list->length];
				if( min_surf_dijk == 4 )
				{
		    		if( tmp_lbl == 2 )
					{
						n_nw_node->b_tmp = 1;
						tmp_node_list[nbr_kount] = n_nw_node;
						nbr_kount++;
						p_dist = moi->pa[0]*tmp1 + moi->pa[1]*tmp2
						   + moi->pa[2]*tmp3 + const_d;
						p_dist = (float)fabs(p_dist);

						n_nw_node->backward_label = p_dist;

						area1 = triangle_area1((float)moi->comx,
								   (float)moi->comy,
								   (float)moi->comz,(float)o_x,
								   (float)o_y,(float)o_z,
								   (float)tmp1,(float)tmp2,
								   (float)tmp3);

						n_nw_node->backward_3d_area
							= min_nw_node->backward_3d_area + area1;
						n_nw_node->bac_prev_path = min_nw_node;
						n_nw_node->backward_angle
							= calc_d_theta(v_term,v0,&(moi->pa[0]),'n');
					}
					else
					{
						area1 = triangle_area1((float)moi->comx,
								   (float)moi->comy,
								   (float)moi->comz,(float)o_x,
								   (float)o_y,(float)o_z,
								   (float)tmp1,(float)tmp2,
								   (float)tmp3);
						area1 += min_nw_node->backward_3d_area;

						if(    (area1 < n_nw_node->backward_3d_area)
						&& (fabs(area1-n_nw_node->backward_3d_area) > TOL) )
						{
							n_nw_node->backward_3d_area = area1;
							n_nw_node->bac_prev_path = min_nw_node;
						}
					}
				}
				else if( nnwfl > min_blbl )
				{
					if( n_nw_node->b_tmp == 1 )
					{
						nnwnpiich = n_nw_node->bac_in_indx_chain;
						if(    start_search_here
						&& (nnwnpiich == start_search_here)    )
						{
							if( start_search_here->next )
								 start_search_here = start_search_here->next;
							else if( start_search_here->prev)
								 start_search_here = start_search_here->prev;
							else start_search_here = NULL;
						}
						if( nnwnpiich == *ptmp_indx_chain )
							*ptmp_indx_chain = (*ptmp_indx_chain)->next;
						if( nnwnpiich->prev )
							nnwnpiich->prev->next = nnwnpiich->next;
						if( nnwnpiich->next )
							nnwnpiich->next->prev = nnwnpiich->prev;
						FREE(nnwnpiich,sizeof(tmp_indx_chain));
						n_nw_node->bac_in_indx_chain = NULL;
					}
					else
						n_nw_node->b_tmp = 1;
					if(    (min_surf_dijk == 1)
						|| (min_surf_dijk == 2)
						|| (min_surf_dijk == 3)    )
					{
						area1 = triangle_area1((float)moi->comx,
								   (float)moi->comy,
								   (float)moi->comz,(float)o_x,
								   (float)o_y,(float)o_z,
								   (float)tmp1,(float)tmp2,
								   (float)tmp3);
						n_nw_node->backward_3d_area
							= min_nw_node->backward_3d_area + area1;
					}
					n_nw_node->backward_label = min_blbl;
					n_nw_node->bac_prev_path = min_nw_node;

					d_theta = calc_d_theta(v_prev,v0,&(moi->pa[0]),'n');
					n_nw_node->backward_angle = d_theta + min_angle;
					tmp_node_list[nbr_kount] = n_nw_node;
					nbr_kount++;
					}
				}
				p_nw_adj_list = p_nw_adj_list->next;
			}
			if( nbr_kount >= 1 ) indexx_back(nbr_kount,tmp_node_list-1,indx-1);

			tmp_start_search_here = start_search_here;
			for( i = 0; i < nbr_kount; i++ )
			{
				if( tmp_start_search_here == NULL )
				{
					*ptmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
					msg = "*ptmp_indx_chain in Bi_Dir_bac_Label()";
					if( MEMCHECK(*ptmp_indx_chain,msg,ticsz) ) clean_up(0);

					(*ptmp_indx_chain)->prev = (tmp_indx_chain*) NULL;
					(*ptmp_indx_chain)->nw_node = tmp_node_list[indx[0]-1];
					(*ptmp_indx_chain)->next = (tmp_indx_chain*) NULL;
					start_search_here = *ptmp_indx_chain;
					tmp_start_search_here= start_search_here;
					tmp_node_list[indx[0]-1]->bac_in_indx_chain
						= *ptmp_indx_chain;
				}
				else
				{
					pstart_search_here = tmp_start_search_here;
					stop_search = 0;
					if( pstart_search_here->nw_node->backward_label
						> tmp_node_list[indx[i]-1]->backward_label )
					{
						while( !stop_search )
						{
							if(    pstart_search_here->prev
							&& pstart_search_here->prev->nw_node->backward_label
								>= tmp_node_list[indx[i]-1]->backward_label )
							{
								pstart_search_here = pstart_search_here->prev;
							}
							else
							{
								ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
								msg = "ntmp_indx_chain in Bi_Dir_bac_Label()";
								if( MEMCHECK(ntmp_indx_chain,msg,ticsz) )
									clean_up(0);

								ntmp_indx_chain->nw_node
										= tmp_node_list[indx[i]-1];
								ntmp_indx_chain->next = pstart_search_here;
								tmp_node_list[indx[i]-1]->bac_in_indx_chain
										= ntmp_indx_chain;

								if( pstart_search_here->prev == NULL )
								{
									ntmp_indx_chain->prev = NULL;
									*ptmp_indx_chain = ntmp_indx_chain;
								}
								else
								{
									ntmp_indx_chain->prev
										= pstart_search_here->prev;
									pstart_search_here->prev->next
										= ntmp_indx_chain;
								}
								pstart_search_here->prev = ntmp_indx_chain;
								tmp_start_search_here = ntmp_indx_chain;
								stop_search = 1;
							}
						}
					}
					else if( pstart_search_here->nw_node->backward_label
						< tmp_node_list[indx[i]-1]->backward_label )
					{
						while( !stop_search )
						{
							if(    pstart_search_here->next
							&& pstart_search_here->next->nw_node->backward_label
								<= tmp_node_list[indx[i]-1]->backward_label )
							{
								pstart_search_here = pstart_search_here->next;
						}
						else
						{
							ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
							msg = "ntmp_indx_chain in Bi_Dir_bac_Label()";
							if( MEMCHECK(ntmp_indx_chain,msg,ticsz) )
								clean_up(0);

							ntmp_indx_chain->nw_node
									= tmp_node_list[indx[i]-1];
							ntmp_indx_chain->prev= pstart_search_here;
							tmp_node_list[indx[i]-1]->bac_in_indx_chain
										= ntmp_indx_chain;

							if( pstart_search_here->next == NULL )
								ntmp_indx_chain->next = NULL;
							else
							{
								pstart_search_here->next->prev
										= ntmp_indx_chain;
								ntmp_indx_chain->next
									= pstart_search_here->next;
							}
							pstart_search_here->next = ntmp_indx_chain;
							tmp_start_search_here = ntmp_indx_chain;
							stop_search = 1;
						}
					}
				}
				else
				{
					ntmp_indx_chain = (tmp_indx_chain *)MALLOC(ticsz);
					msg = "ntmp_indx_chain in Bi_Dir_bac_Label()";
					if( MEMCHECK(ntmp_indx_chain,msg,ticsz) ) clean_up(0);

					ntmp_indx_chain->nw_node = tmp_node_list[indx[i]-1];
					ntmp_indx_chain->prev= pstart_search_here;
					tmp_node_list[indx[i]-1]->bac_in_indx_chain
									= ntmp_indx_chain;
					if( pstart_search_here->next == NULL )
						ntmp_indx_chain->next = NULL;
					else
					{
						pstart_search_here->next->prev = ntmp_indx_chain;
						ntmp_indx_chain->next = pstart_search_here->next;
					}
					pstart_search_here->next = ntmp_indx_chain;
					tmp_start_search_here = ntmp_indx_chain;
					stop_search = 1;
				}
			}
		}
	}
	else
	{
		min_nw_node->b_tmp = 0;
		shortest_path = return_shortest_path(min_nw_node,1,grain_index);
		*ok = 0;
	}
	return	shortest_path;
}


network_node *del_join_bac_link(network_node *p_nw_node,
								network_node *p_e_node)
{
	network_node	*tmp_nw_node;

	tmp_nw_node = p_nw_node->back_pointer;
	if( tmp_nw_node != p_e_node )
	{
		p_nw_node->back_pointer = p_e_node;
		if( tmp_nw_node ) tmp_nw_node->next = p_e_node;

		if( p_e_node->back_pointer )
			p_e_node->back_pointer->next = p_e_node->next;

		if( p_e_node->next )
			p_e_node->next->back_pointer = p_e_node->back_pointer;

		p_e_node->next = p_nw_node;
		p_e_node->back_pointer = tmp_nw_node;
	}

	return p_e_node;
}


grain_chain *return_shortest_path(network_node *min_nw_node,
								  int join_grain_index, int grain_index)
{
	grain_chain	*ret_grain_chain, *p_last_link, *p_last1_link;

	char	*msg;

	int	gcsz = sizeof(grain_chain);

	ret_grain_chain = (grain_chain *)MALLOC(gcsz);
	msg = "ret_grain_chain in get_shortest_path()";
	if( MEMCHECK(ret_grain_chain,msg,gcsz) ) clean_up(0);

	ret_grain_chain->index_in_cube = min_nw_node->index_in_cube;
	ret_grain_chain->next = NULL;
	rec_mk_ret_chain(ret_grain_chain,&p_last_link,
				min_nw_node->for_prev_path,1);
	rec_mk_ret_chain(ret_grain_chain,&p_last_link,
				min_nw_node->bac_prev_path,2);

	if( join_grain_index )
	{
		p_last1_link= (grain_chain *)MALLOC(gcsz);
		msg = "p_last1_link in get_shortest_path()";
		if( MEMCHECK(p_last1_link,msg,gcsz) ) clean_up(0);

		p_last1_link->index_in_cube = grain_index;
		p_last1_link->next = p_last_link;
	}
	else	p_last1_link = p_last_link;

	return p_last1_link;
}


void	recur_reverse_network_dir(network_node *start_node)
{
	network_node	*tmp_nw_node;

	if( start_node->next ) recur_reverse_network_dir(start_node->next);

	tmp_nw_node = start_node->back_pointer;
	start_node->back_pointer = start_node->next;
	start_node->next = tmp_nw_node;
}


void	print_min_node_data(network_node *min_node, int dir, int nx,
							int nxy, FILE *fp1)
{
	int	Ind0;
	int	x, y, z, x1, y1, z1;

	Ind0 = min_node->index_in_cube;
	Ind_2_ijk(Ind0,x,y,z,nx,nxy)

	if( dir == 1 )
	{
		Ind0 = min_node->for_prev_path->index_in_cube;
		Ind_2_ijk(Ind0,x1,y1,z1,nx,nxy)

		fprintf(fp1,"for min(%d %d %d) ang(%f) dis(%f), ",
			x,y,z,min_node->forward_angle,min_node->forward_label);
		fprintf(fp1,"pre(%d %d %d) ang(%f) dis(%f)\n",
			x1,y1,z1,min_node->for_prev_path->forward_angle,
			min_node->for_prev_path->forward_label);
	}
	else
	{
		Ind0 = min_node->bac_prev_path->index_in_cube;
		Ind_2_ijk(Ind0,x1,y1,z1,nx,nxy)
		fprintf(fp1,"bac min(%d %d %d) ang(%f) dis(%f), ",
				  x,y,z,min_node->backward_angle,
					min_node->backward_label);
		fprintf(fp1,"pre(%d %d %d) ang(%f) dis(%f)\n",
			x1,y1,z1,min_node->bac_prev_path->backward_angle,
			min_node->bac_prev_path->backward_label);
	}
}


void	free_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain, int dir)
{
	int ticsz = sizeof(tmp_indx_chain);

	if( ttmp_indx_chain->next != NULL )
		free_tmp_indx_chain(ttmp_indx_chain->next, dir);
	if( dir == 1 )
		ttmp_indx_chain->nw_node->for_in_indx_chain = NULL;
	else if( dir == 2 )
		ttmp_indx_chain->nw_node->bac_in_indx_chain = NULL;
	else
	{
		fprintf(stderr,"Error in free_tmp_indx_chain()\n");
		fprintf(stdout,"Error in free_tmp_indx_chain()\n");
		clean_up(0);
	}
	FREE(ttmp_indx_chain,ticsz);
}


void	dbg_print_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain)
{
	tmp_indx_chain	*ptmp_indx_chain;

	int	i;

	ptmp_indx_chain = ttmp_indx_chain;
	i = 0;
	while( ptmp_indx_chain )
	{
		i++;
	  	fprintf(stderr,"%d) %d\n",
				i,ptmp_indx_chain->nw_node->index_in_cube);
		fprintf(stdout,"%d) %d\n",
				i,ptmp_indx_chain->nw_node->index_in_cube);
		ptmp_indx_chain = ptmp_indx_chain->next;
	}
}


void	dbg_reverse_print_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain)
{
	tmp_indx_chain	*ptmp_indx_chain;

	int	i;

	ptmp_indx_chain = ttmp_indx_chain;

	while (ptmp_indx_chain->next != NULL )
		ptmp_indx_chain = ptmp_indx_chain->next;
	
	i = 1;
	fprintf(stderr,"%d) %d\n",i,ptmp_indx_chain->nw_node->index_in_cube);
	fprintf(stdout,"%d) %d\n",i,ptmp_indx_chain->nw_node->index_in_cube);

	i++;
	dbg_recur_reverse_print(ptmp_indx_chain->prev,i);
}


void	dbg_recur_reverse_print(tmp_indx_chain *ttmp_indx_chain, int i)
{
	int	j;
	j = i;
	if( ttmp_indx_chain != NULL )
	{
		fprintf(stderr,"%d) %d\n",
			j,ttmp_indx_chain->nw_node->index_in_cube);
		fprintf(stdout,"%d) %d\n",
			j,ttmp_indx_chain->nw_node->index_in_cube);
		j++;
		dbg_recur_reverse_print(ttmp_indx_chain->prev,j);
	  }
}


void	dbg_nw_print_f_b(network_node *nw_node, int print_coords, int nx,
						 int nxy, char fc, char bc)
{
	network_node	*pnw_node;
	int		ind, x, y, z;

	pnw_node = nw_node;
	if( print_coords )
	{
		while( pnw_node != NULL )
		{
			if( pnw_node->f_tmp == fc && pnw_node->b_tmp == bc )
			{
				ind = pnw_node->index_in_cube;
				Ind_2_ijk(ind,x,y,z,nx,nxy)
				fprintf(stderr,"%d (%d %d %d)\n",ind,x,y,z);
				fprintf(stdout,"%d (%d %d %d)\n",ind,x,y,z);
			}
			pnw_node = pnw_node->next;
		}
	}
	else
	{
		while( pnw_node != NULL )
		{
			if( pnw_node->f_tmp == fc && pnw_node->b_tmp == bc )
			{
				ind = pnw_node->index_in_cube;
				fprintf(stderr,"%d\n",ind);
				fprintf(stdout,"%d\n",ind);
			}
			pnw_node = pnw_node->next;
		}	    
	}
}


void	dbg_nw_print_network(network_node *nw_node, int print_coords,
							 int nx, int nxy)
{
	fprintf(stderr,"printing join point(s)\n");
	fprintf(stdout,"printing join point(s)\n");

	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,0,0);
	
	fprintf(stderr,"f_perm_b_tmp\n");
	fprintf(stdout,"f_perm_b_tmp\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,0,1);

	fprintf(stderr,"f_perm_only\n");
	fprintf(stdout,"f_perm_only\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,0,2);

	fprintf(stderr,"b_perm_f_tmp\n");
	fprintf(stdout,"b_perm_f_tmp\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,1,0);

	fprintf(stderr,"f_tmp_b_tmp\n");
	fprintf(stdout,"f_tmp_b_tmp\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,1,1);

	fprintf(stderr,"f_tmp_only\n");
	fprintf(stdout,"f_tmp_only\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,1,2);

	fprintf(stderr,"b_perm_only\n");
	fprintf(stdout,"b_perm_only\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,2,0);

	fprintf(stderr,"b_tmp_only\n");
	fprintf(stdout,"b_tmp_only\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,2,1);

	fprintf(stderr,"unmarked\n");
	fprintf(stdout,"unmarked\n");
	dbg_nw_print_f_b(nw_node,print_coords,nx,nxy,2,2);

	fprintf(stderr,"end of dbg_nw_print_network\n");
	fprintf(stdout,"end of dbg_nw_print_network\n");
}
