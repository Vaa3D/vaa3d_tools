/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include "ma_prototype.h"
#include "min_surf_prototype.h"

void	modify_surf_rmnt(int *cube, MA_set *mac_root, int nx, int nxy,
						 int *bc_done)
{
	MA_set		*pmac, tma_set;
	MoI		*moi;
	network_node	*snw_node, *enw_node, **cube_2_network;
	grain_chain	*shortest_path, *tmp_g_chain;

	char	*msg;

	double	min_dist, dist, dx, dy, dz;


	int	sten26[26];
	int	*tloc, *kloc, *att_val, *loc_val;

	int	i, j, k, l;
	int	ix, iy, iz;
	int	min_d_ind, count, tot_cnt, tmp_ind;
	int	cnum;

	int	  isz = sizeof(int);
	int	  usz = sizeof(unsigned char);
	int	pnnsz = sizeof(network_node *);

	set_stencil(nx,nxy,sten26);

	pmac = mac_root;
	tot_cnt = 0;

	tma_set.prev = NULL;
	tma_set.next = NULL;

	while( pmac!= NULL )
	{
	    if( pmac->surf_rmnt )
	    {
		min_dist = 10000000.0;
		cnum = pmac->num_in_set;

		tloc = (int *)MALLOC((cnum+1)*isz);

		cube_2_network = (network_node **)MALLOC((cnum+1)*pnnsz);

		kloc = (int *)CALLOC(cnum,isz);

		tma_set.num_in_set = cnum;
		tma_set.loc = pmac->loc;
		tma_set.num_att = pmac->num_att;
		tma_set.attach_list = pmac->attach_list;

		moi = compute_moi(&tma_set,nx,nxy); /* is this freed? */

		if( tma_set.num_att == 2 )
		{
		    PRINTF("\n");
		    PRINTF("surface remnant with 2 attached paths\n");

			/* set first attached path connection */
			/*     point as `center-of-mass'      */

		    Pad_ind_2_ijk(tma_set.attach_list[0],i,j,k,nx,nxy)
		    moi->comx = i + 0.5;
		    moi->comy = j + 0.5;
		    moi->comz = k + 0.5;
		}
		for( l = 0;  l < cnum;  l++ )
		{
			Pad_ind_2_ijk(pmac->loc[l],i,j,k,nx,nxy)
			dx = moi->comx - i - 0.5;	dx *= dx;
			dy = moi->comy - j - 0.5;	dy *= dy;
			dz = moi->comz - k - 0.5;	dz *= dz;

			dist = dx + dy + dz;
			if( dist < min_dist )
			{
				min_dist = dist;
				min_d_ind = l;
			}
		}
		rec_free_MoI_list(moi,sizeof(MoI));

		tmp_ind = pmac->loc[min_d_ind];
		pmac->loc[min_d_ind] = pmac->loc[cnum-1];
		pmac->loc[cnum-1] = tmp_ind;
		kloc[cnum-1] = 1;   /* should be kept  */
		if( pmac->bdat != NULL )
		{
			tmp_ind = pmac->bdat[min_d_ind];
			pmac->bdat[min_d_ind] = pmac->bdat[cnum-1];
			pmac->bdat[cnum-1] = tmp_ind;
		}

		Pad_ind_2_ijk(pmac->loc[cnum-1],ix,iy,iz,nx,nxy)

		PRINTF("Surf rmnt center (%d %d %d) of size %d ",
						ix,iy,iz,pmac->num_in_set);
		PRINTF("with %d paths\n",pmac->num_att);

		att_val = (int *) MALLOC(tma_set.num_att*isz);
		msg = "att_val in modify_surf_rmnt()";
		if( MEMCHECK(att_val,msg,tma_set.num_att*isz) ) clean_up(0);

		loc_val = (int *) MALLOC(cnum*isz);
		msg = "loc_val in modify_surf_rmnt()";
		if( MEMCHECK(loc_val,msg,cnum*isz) ) clean_up(0);

		for( l = 0;  l < tma_set.num_att;  l++ )
			att_val[l] = cube[tma_set.attach_list[l]];

		for( i = 0;  i < cnum;  i++ )
		{
			loc_val[i] = cube[tma_set.loc[i]];
			cube[tma_set.loc[i]] = -1000-(i+1);
		}

		for( l = 0;  l < tma_set.num_att;  l++ )
		{
		    tloc[   0] = tma_set.attach_list[l];
		    tloc[cnum] = tma_set.loc[cnum-1];

		    cube[tloc[0]] = -1000;

		    for( i = 0;  i < cnum-1;  i++ ) tloc[i+1] = tma_set.loc[i];

		    setup_network_generic(tloc,cube,sten26,&snw_node,&enw_node,
						26,cnum,cube_2_network,-1000);
		    shortest_path = get_shortest_path_generic(snw_node,enw_node,
						cube,nx,nxy,cnum,sten26,
						cube_2_network,-1000);
		    if( shortest_path )
		    {
				tmp_g_chain = shortest_path;
				while( tmp_g_chain)
				{
					tloc[-(cube[tmp_g_chain->index_in_cube]+1000)] = 1;
					tmp_g_chain = tmp_g_chain->next;
				}
				for( i = 1;  i < cnum;  i++ )
				{
					if( tloc[i] == 1 ) kloc[i-1] = 1;
				}
				free_g_chain(shortest_path);
		    }
		    free_network(snw_node); 
		    cube[pmac->attach_list[l]] = att_val[l];
		}

		count = 0;
		for( i = 0;  i < cnum;  i++ )
		{
		    if( !kloc[i] )
		    {
			cube[pmac->loc[i]] = 0;
			Pad_ind_2_ijk(pmac->loc[i],ix,iy,iz,nx,nxy)

//			PRINTF("(%d %d %d) removed voxel\n",ix,iy,iz); 
			count++;
		    }
		    else cube[pmac->loc[i]] = loc_val[i];
		}

		for( l = 0; l < tma_set.num_att; l++ )
			cube[pmac->attach_list[l]] = att_val[l];

		PRINTF("%d voxels removed from surface remanant\n",count);

		pmac->loc = (int *)REALLOC(pmac->loc,(cnum-count)*isz);
		if( MEMCHECK(pmac->loc,msg,(-count)*isz) ) clean_up(0);

		pmac->bdat = (unsigned char *)REALLOC(pmac->bdat,(cnum-count)*usz);
		msg = "pmac->bdat in modify_surf_rmnt()";

		FREE(tloc,(cnum+1)*isz);
		FREE(kloc,cnum*isz);
		FREE(att_val,tma_set.num_att*isz);
		FREE(loc_val,cnum*isz);
		FREE(cube_2_network,(cnum+1)*pnnsz);

		pmac->num_in_set = cnum - count;
		tot_cnt += count;
	    }
	    pmac = pmac->next;
	}
	*bc_done += tot_cnt;
}


void	setup_network_generic(int *tloc, int *cube, int *sten26,
							  network_node **snw_node,
							  network_node **enw_node, char conn_type,
							  int cnum, network_node **cube_2_network,
							  int compare)
{
	network_node	*p_nw_node, *p_cnw_node, *prev_nw_node;
	
	char	*msg;

	int	i, index, cnt;
	int	dbg = 0;

	int	nnsz = sizeof(network_node);
	int     isz = sizeof(int);

	p_cnw_node = (network_node *)MALLOC(nnsz);
	msg = "p_cnw_node in setup_network_surf_rmnt";
	if( MEMCHECK(p_cnw_node,msg,nnsz) ) clean_up(0); 

	p_nw_node = p_cnw_node;
	prev_nw_node = NULL;
	cnt = 0;

	initialize_nw_node_generic(p_nw_node,prev_nw_node,tloc[0],cube,sten26,
					conn_type,cube_2_network,compare,&cnt);

	for( i = 1;  i <= cnum;  i++ )
	{
	    index = tloc[i];
	    prev_nw_node = p_nw_node;

	    p_nw_node->next = (network_node *)MALLOC(nnsz);

	    p_nw_node = p_nw_node->next;

	    initialize_nw_node_generic(p_nw_node,prev_nw_node,index,cube,
				sten26,conn_type,cube_2_network,compare,&cnt);
	} 

	*snw_node = p_cnw_node;
	*enw_node = p_nw_node;

	if( dbg )
	{
		cnt = 1;	dbg_print_network(*snw_node,&cnt,1);
		cnt = 1;	dbg_print_network(*enw_node,&cnt,2);
	}
}


#define Build_TAdj_List(_Ind)\
{\
	adj_index = index+sten26[_Ind];\
\
	if( cube[adj_index] > compare ) continue;\
\
	if( first )\
	{\
		p_nw_adj_list = (network_adj_list *)MALLOC(nwalsz);\
		msg = "p_nw_adj_list in initialize_tnw_node()";\
		if( MEMCHECK(p_nw_adj_list,msg,nwalsz) ) clean_up(0);\
\
		p_nw_node->nw_adj_list = p_nw_adj_list;\
		first = 0;\
	}\
	else\
	{\
		p_nw_adj_list->next = (network_adj_list *)MALLOC(nwalsz);\
		msg = "p_nw_adj_list->next in initialize_tnw_node()";\
		if( MEMCHECK(p_nw_adj_list->next,msg,nwalsz) ) clean_up(0);\
\
		p_nw_adj_list = p_nw_adj_list->next;\
	}\
	p_nw_adj_list->index_in_cube = adj_index;\
	p_nw_adj_list->next= NULL;\
\
	if(    _Ind ==  0 || _Ind ==  2 || _Ind ==  6 || _Ind ==  8\
	    || _Ind == 17 || _Ind == 19 || _Ind == 23 || _Ind == 25 )\
	{\
		p_nw_adj_list->length = 2;\
	}\
	else if(    _Ind ==  1 || _Ind ==  3 || _Ind ==  5 || _Ind ==  7\
		 || _Ind ==  9 || _Ind == 11 || _Ind == 14 || _Ind == 16\
		 || _Ind == 18 || _Ind == 20 || _Ind == 22 || _Ind == 24 )\
	{\
		p_nw_adj_list->length = 1;\
	}\
	else\
		p_nw_adj_list->length = 0;\
}


void	initialize_nw_node_generic(network_node *p_nw_node,
								   network_node *prev_nw_node,
								   int index, int *cube, int *sten26,
								   char conn_type,
								   network_node **cube_2_network,
								   int compare, int *cnt)
{
	network_adj_list *p_nw_adj_list;
	
	char	*msg;

	static	float flint = (float) INT_MAX;	/* initial label value */
	
	int	i, adj_index, first;

	int	nwalsz = sizeof(network_adj_list);

	p_nw_node->index_in_cube  = index;
	p_nw_node->forward_label  = flint;
	p_nw_node->backward_label = flint;
	p_nw_node->f_tmp	  = 2;		/* initially not marked */
	p_nw_node->b_tmp	  = 2;
	p_nw_node->back_pointer	  = prev_nw_node;
	p_nw_node->for_prev_path  = NULL;
	p_nw_node->bac_prev_path  = NULL;
	p_nw_node->next		  = NULL;
	p_nw_node->nw_adj_list	  = (network_adj_list *) NULL;

	cube_2_network[*cnt] = p_nw_node;
	*cnt += 1;

	first = 1;

	if( conn_type == 26 )
	{
		for( i = 0;  i < 26;  i++ ) Build_TAdj_List(i)
	}
	else if( conn_type == 6 )
	{
		int     six_nbrs[6];

		six_nbrs[0] =  4;	six_nbrs[3] = 13;
		six_nbrs[1] = 10;	six_nbrs[4] = 15;
		six_nbrs[2] = 12;	six_nbrs[5] = 21;

		for( i = 0;  i <  6;  i++ ) Build_TAdj_List(six_nbrs[i])
	}
	else
	{
		PRINTF("Cannot handle %d connectivity\n",conn_type);
		clean_up(0);
	}
}


grain_chain    *get_shortest_path_generic(network_node *snw_node,
										  network_node *enw_node, int *cube,
										  int nx, int nxy, int cnum,
										  int *sten26,
										  network_node **cube_2_network,
										  int compare)
{
	network_node	*p_snw_node, *p_enw_node, *n_nw_node;
			
	network_adj_list *p_nw_adj_list;
	grain_chain	 *shortest_path;

	int	adj_ind;
	int	ok;
	int	dbg = 0;

	
	static	float cost[3] = {(float)0.9016, (float)1.289, (float)1.615};

	if( dbg )
	{
		int	cnt;

		cnt = 1;	dbg_print_network(snw_node,&cnt,1);
		cnt = 1;	dbg_print_network(enw_node,&cnt,2);
	}
	
	shortest_path = (grain_chain *) NULL;

	snw_node->forward_label  = 0.0;
	enw_node->backward_label = 0.0;
	snw_node->f_tmp = 0;		/* permanently marked */
	enw_node->b_tmp = 0;
	

	p_nw_adj_list = snw_node->nw_adj_list;
	
	while( p_nw_adj_list != NULL )
	{
		adj_ind = p_nw_adj_list->index_in_cube;
		n_nw_node = cube_2_network[-(cube[adj_ind]-compare)];
		n_nw_node->forward_label =   snw_node->forward_label
					   + cost[p_nw_adj_list->length];

		n_nw_node->for_prev_path = snw_node;
		n_nw_node->f_tmp = 1;		/* temporarily marked */
		p_nw_adj_list = p_nw_adj_list->next;
	}

	p_nw_adj_list = enw_node->nw_adj_list;
	
	while( p_nw_adj_list != NULL )
	{
		adj_ind = p_nw_adj_list->index_in_cube;
		n_nw_node = cube_2_network[-(cube[adj_ind]-compare)];
		n_nw_node->backward_label =   enw_node->backward_label
					    + cost[p_nw_adj_list->length];
		
		n_nw_node->bac_prev_path = enw_node;
		n_nw_node->b_tmp = 1;		
		p_nw_adj_list = p_nw_adj_list->next;
	}
	
	p_snw_node = snw_node;
	p_enw_node = enw_node;
	ok = 1;

	while( ok )
	{
	    shortest_path = Bi_Dir_for_Label_generic(snw_node,&p_snw_node,
					cube,nx,nxy,&ok,cube_2_network,compare);

	    if( ok )
		shortest_path = Bi_Dir_bac_Label_generic(enw_node,&p_enw_node,
					cube,nx,nxy,&ok,cube_2_network,compare);
	}
	return shortest_path;
}


#define	Bi_Dir_Params\
	static	float	cost[3] = {(float)0.9016, (float)1.289, (float)1.615};\
	static	int	realloc_param = 100;\
		\
	grain_chain	 *shortest_path;\
	network_node	 *p_nw_node, *n_nw_node, *min_nw_node;\
	network_node	 **tmp_node_list;\
	network_adj_list *p_nw_adj_list;\
	\
	char	*msg;\
	char	tmp_lbl;\
\
	int	*indx;\
	int	adj_ind, ind0, tmp1, tmp2, tmp3;\
	int	tmp_counter, realloc_counter, tmp_size;\
\
	int	  isz = sizeof(int);\
	int	pnnsz = sizeof(network_node *)


#define	Bi_Dir_Init\
	min_nw_node   = (network_node *) NULL;\
	shortest_path = (grain_chain  *) NULL;\
\
	tmp_counter = 0;\
	realloc_counter = 1;\
	tmp_size = realloc_param;\
\
	tmp_node_list = (network_node **)MALLOC(realloc_param*pnnsz);\


#define	Bi_Dir_Tmp_Node_List_Fill \
{\
	tmp_node_list[tmp_counter] = p_nw_node;\
	tmp_counter++;\
	if( tmp_counter == tmp_size )\
	{\
	    realloc_counter++;\
\
	    tmp_node_list = (network_node **)REALLOC(tmp_node_list,\
				realloc_counter*realloc_param*pnnsz);\
\
	    tmp_size += realloc_param;\
	}\
}


#define	Bi_Dir_Alloc\
	tmp_node_list = (network_node **)REALLOC(tmp_node_list,\
						 tmp_counter*pnnsz);\
\
	indx = (int *)MALLOC(tmp_counter*isz);\
	msg = "indx in Bi_Dir_..._Label_generic()";\


grain_chain *Bi_Dir_for_Label_generic(network_node *snw_node,
									  network_node **start_node, int *cube,
									  int nx, int nxy, int *ok,
									  network_node **cube_2_network,
									  int compare)
{
	Bi_Dir_Params;
	float	nnwfl, min_fl;

	Bi_Dir_Init;
	
	p_nw_node = (*start_node)->next;
	while( p_nw_node != NULL )
	{
	    if( p_nw_node->f_tmp == 1 ) Bi_Dir_Tmp_Node_List_Fill
	    p_nw_node = p_nw_node->next;
	}

	if( tmp_counter == 0 )		/* There is no short_path  */
	{
		*ok = 0;
		return shortest_path;
	}
	
	Bi_Dir_Alloc;

	indexx_for(tmp_counter,tmp_node_list-1,indx-1);
	
	min_nw_node = tmp_node_list[indx[0]-1];
		
	FREE(tmp_node_list,tmp_counter*pnnsz);
	FREE(indx,tmp_counter*isz);

	if( min_nw_node->b_tmp != 0 )
	{
		min_nw_node->f_tmp = 0;
		*start_node = del_join_for_link(*start_node,min_nw_node);

		ind0 = min_nw_node->index_in_cube;
		Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)

		p_nw_adj_list = min_nw_node->nw_adj_list;

		while( p_nw_adj_list != NULL )
		{
			adj_ind = p_nw_adj_list->index_in_cube;
			n_nw_node = cube_2_network[-(cube[adj_ind]-compare)];
			tmp_lbl = n_nw_node->f_tmp;

			if( tmp_lbl != 0 )
			{
				nnwfl = n_nw_node->forward_label;
				min_fl  = min_nw_node->forward_label;
				min_fl += cost[p_nw_adj_list->length];
				if( nnwfl > min_fl )
				{
					n_nw_node->f_tmp = 1;
					n_nw_node->forward_label = min_fl;
					n_nw_node->for_prev_path = min_nw_node;
				}
			}
			p_nw_adj_list = p_nw_adj_list->next;
		}
	}
	else
	{
	    min_nw_node->f_tmp = 0;
	    shortest_path = return_shortest_path_generic(min_nw_node,snw_node,
						 (*start_node)->next,1,
						 cube_2_network,cube,compare);
	    *ok = 0;
	}
	return  shortest_path;
}


grain_chain *Bi_Dir_bac_Label_generic(network_node *enw_node,
									  network_node **start_node,
									  int *cube, int nx, int nxy, int *ok,
									  network_node **cube_2_network,
									  int compare)
{
	Bi_Dir_Params;
	float	nnwbl, min_bl;

	Bi_Dir_Init;

	p_nw_node = (*start_node)->back_pointer;
	while( p_nw_node != NULL )
	{
	    if( p_nw_node->b_tmp == 1 ) Bi_Dir_Tmp_Node_List_Fill
	    p_nw_node = p_nw_node->back_pointer;
	}

	if( tmp_counter == 0)		/* There is no short_path */
	{
		*ok = 0;
		return shortest_path;
	}
	
	Bi_Dir_Alloc;

	indexx_back(tmp_counter,tmp_node_list-1,indx-1);

	min_nw_node = tmp_node_list[indx[0]-1];
	
	FREE(tmp_node_list,tmp_counter*pnnsz);
	FREE(indx,tmp_counter*isz);

	if( min_nw_node->f_tmp != 0 )
	{
		min_nw_node->b_tmp = 0;
		*start_node = del_join_bac_link(*start_node,min_nw_node);

		ind0 = min_nw_node->index_in_cube;
		Pad_ind_2_ijk(ind0,tmp1,tmp2,tmp3,nx,nxy)
		      
		p_nw_adj_list = min_nw_node->nw_adj_list;
		
		while( p_nw_adj_list != NULL )
		{
			adj_ind = p_nw_adj_list->index_in_cube;
			n_nw_node = cube_2_network[-(cube[adj_ind]-compare)];
			tmp_lbl = n_nw_node->b_tmp;

			if( tmp_lbl != 0 )
			{
				nnwbl = n_nw_node->backward_label;
				min_bl  = min_nw_node->backward_label;
				min_bl += cost[p_nw_adj_list->length];
				if( nnwbl > min_bl )
				{
					n_nw_node->b_tmp = 1;
					n_nw_node->backward_label = min_bl;
					n_nw_node->bac_prev_path  = min_nw_node;
				}
			}
			p_nw_adj_list = p_nw_adj_list->next;
		}
	}
	else
	{
	    min_nw_node->b_tmp = 0;
	    shortest_path = return_shortest_path_generic(min_nw_node,enw_node,
						(*start_node)->back_pointer,2,
						cube_2_network,cube,compare); 
	    *ok = 0;
	}
	return	shortest_path;
}


grain_chain *return_shortest_path_generic(network_node *min_nw_node,
										  network_node *start_node,
										  network_node *stop_node, int dir,
										  network_node **cube_2_network,
										  int *cube, int compare)
{
	static	float	cost[3] = {(float)0.9016, (float)1.289, (float)1.615};
	
	grain_chain	 *ret_grain_chain, *tmp_g_chain, *p_last_link;
	network_node	 *ptr_node, *n_nw_node, *min_dist_fend, *min_dist_bend;
	network_adj_list *p_nw_adj_list;

	char	*msg;

	float	min_dist, dist;

	int	ok, g_index, tmp_lbl;
	int	gcsz = sizeof(grain_chain);


	min_dist = min_nw_node->forward_label + min_nw_node->backward_label;
	min_dist_fend = min_nw_node;
	min_dist_bend = min_nw_node;
	ptr_node = start_node;
	ok = 1;

	while( ok )
	{
		if( ptr_node == NULL )
		{
			msg = "Logic error in return_shortest_path_generic\n";
			PRINTF(msg);
			clean_up(0);
		}

		p_nw_adj_list = ptr_node->nw_adj_list;
		while( p_nw_adj_list )
		{
			g_index = -(cube[p_nw_adj_list->index_in_cube]-compare);
			n_nw_node = cube_2_network[g_index];

			tmp_lbl = (dir == 1 ) ? n_nw_node->b_tmp
					      : n_nw_node->f_tmp;

			if( tmp_lbl == 0 )
			{
				if( dir == 1 )
				{
					dist = ptr_node->forward_label
					     + n_nw_node->backward_label
					     + cost[p_nw_adj_list->length];

					if( dist < min_dist )
					{
					    min_dist = dist;
					    min_dist_fend = ptr_node;
					    min_dist_bend = n_nw_node;
					}
				}
				else
				{
					dist = ptr_node->backward_label
					     + n_nw_node->forward_label
					     + cost[p_nw_adj_list->length];

					if( dist < min_dist )
					{
					    min_dist = dist;
					    min_dist_bend = ptr_node;
					    min_dist_fend = n_nw_node;
					}
				}
			}
			p_nw_adj_list = p_nw_adj_list->next;
		}

		if( dir == 1 )
		{
			ptr_node = ptr_node->next;
			if( ptr_node == stop_node) ok = 0;
		}
		else if( dir == 2  )
		{
			ptr_node = ptr_node->back_pointer;
			if( ptr_node == stop_node) ok = 0;
		}
	}
	
	ret_grain_chain = (grain_chain *)MALLOC(gcsz);
	
	ret_grain_chain->index_in_cube = min_dist_fend->index_in_cube;
	ret_grain_chain->next = NULL;

	rec_mk_ret_chain(ret_grain_chain,&p_last_link,
			 min_dist_fend->for_prev_path,1);

	if( min_dist_fend != min_dist_bend )
	{
		tmp_g_chain = (grain_chain *)MALLOC(gcsz);

		tmp_g_chain->index_in_cube = min_dist_bend->index_in_cube;
		tmp_g_chain->next = ret_grain_chain;
		ret_grain_chain = tmp_g_chain;
	}

	rec_mk_ret_chain(ret_grain_chain,&p_last_link,
			 min_dist_bend->bac_prev_path,2);
	
	return p_last_link;
}
