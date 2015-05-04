/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"

#include <string.h>
#include <math.h>
#include "ma_prototype.h"
#define _COMPILING_3dmacommon


#define	ALLOC_SZ 20


MA_tree	*create_next_MA_tree(MA_tree **root, MA_tree *prev_tree)
{
	MA_tree	*ma_tree;

	int	Msz = sizeof(MA_tree);

	ma_tree = (MA_tree *)MALLOC(Msz);
	if( MEMCHECK(ma_tree,"ma_tree in create_next_MA_tree()",Msz) )
		clean_up(0);

	if( prev_tree == NULL )	*root = ma_tree;
	else			prev_tree->next = ma_tree;

	ma_tree->prev = prev_tree;
	ma_tree->next = NULL;
	ma_tree->num_paths = 0;
	ma_tree->path_list = NULL;
	ma_tree->num_clust = 0;
	ma_tree->clust_list = NULL;

	return ma_tree;
}


void	dbg_print_MA_trees(MA_tree *mat_root, MA_tree *iso_mat)
{
	print_MA_trees(mat_root);
	PRINTF("\n\tIsolated paths and clusters\n");
	print_MA_trees(iso_mat);
}


void	print_MA_trees(MA_tree *mat_root)
{
	MA_tree	*mat;

	PRINTF("\n\t\tMA_trees\n");
	for( mat = mat_root;  mat != NULL;  mat = mat->next ) print_mat(mat);
}


void	summarize_MA_info(MA_tree *mat_root, MA_tree *iso_mat,
						  MA_set *mas_root, MA_set *mac_root)
{
	MA_tree	*mat;
	int	num_mat, num_ip, num_ic, num_iv, tot;

	num_mat = 0;
	for( mat = mat_root;  mat != NULL;  mat = mat->next ) num_mat++;

	num_ip = ( iso_mat == NULL) ? 0 : iso_mat->num_paths;
	num_ic = ( iso_mat == NULL) ? 0 : iso_mat->num_clust;
	num_iv = (mas_root == NULL) ? 0 : mas_root->num_in_set;

	tot = num_mat + num_ip + num_ic + num_iv;
	PRINTFDEBUG("\n");
	PRINTFDEBUG("Disconnected elements comprising the medial axis:\n");
	PRINTFDEBUG("\tcluster-path trees    %d\n",num_mat);
	PRINTFDEBUG("\tisolated paths        %d\n",num_ip);
	PRINTFDEBUG("\tisolated clusters     %d\n",num_ic);
	PRINTFDEBUG("\tisolated voxels       %d\n",num_iv);
	PRINTFDEBUG("\t                     -----\n");
	PRINTFDEBUG("\ttotal                 %d\n",tot);
	PRINTFDEBUG("\n");

	summarize_cluster_info(mac_root);
}


void	print_mat(MA_tree *mat)
{
	int	i;

	PRINTF("\nmat %d  prev %d next %d\n",mat,mat->prev,mat->next);
	PRINTF("\tnum_paths %d  num_clust %d\n",mat->num_paths,mat->num_clust);
	PRINTF("\tPath list\n");
	for( i = 0;  i < mat->num_paths;  i++ )
	{
		PRINTF(" %d",mat->path_list[i]);
		if( (i+1)%5 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");

	PRINTF("\tCluster/Surface list\n");
	for( i = 0;  i < mat->num_clust;  i++ )
	{
		PRINTF(" %d",mat->clust_list[i]);
		if( (i+1)%5 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");
}


void	rec_free_MA_tree(MA_tree *mat, int pMsz, int MTsz)
{
	if( mat == NULL ) return;
	rec_free_MA_tree(mat->next,pMsz,MTsz);

	if( mat->num_paths > 0 ) FREE(mat->path_list ,mat->num_paths*pMsz);
	if( mat->num_clust > 0 ) FREE(mat->clust_list,mat->num_clust*pMsz);

	FREE(mat,MTsz);
}


void	MA_tree_driver(MA_set *mac_root, MA_set *map_root,
					   MA_tree **mat_root, MA_tree **iso_mat)
{
	char	*msg;

	MA_tree	*prev_mat, *mat;
	MA_set	*mac, *map;

	int	n;

	int	pMsz = sizeof(MA_set *);

	prev_mat = NULL;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->on_tree      ) continue;
		if( mac->num_att == 0 ) continue;	/* deal with isolated */
							/* clusters later */
		prev_mat = set_ma_tree(mac,mat_root,prev_mat);
	}

		/* pick out isolated clusters and paths */
		/* put them in iso_mat 			*/

	mat = create_next_MA_tree(iso_mat,NULL);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->on_tree ) continue;
		(mat->num_clust)++;
	}
	if( mat->num_clust > 0 )
	{
		mat->clust_list = (MA_set **)MALLOC(mat->num_clust*pMsz);
		msg = "mat->clust_list in set_tree_driver()";
		if( MEMCHECK(mat->clust_list,msg,mat->num_clust*pMsz) )
			clean_up(0);

		for( mac = mac_root, n = 0;  mac != NULL;  mac = mac->next, n++ )
		{
			if( mac->on_tree ) continue;
			mat->clust_list[n] = mac;
		}
	}

	for( map = map_root;  map != NULL;  map = map->next )
	{
		if( map->on_tree ) continue;
		(mat->num_paths)++;
	}
	if( mat->num_paths > 0 )
	{
		mat->path_list = (MA_set **)MALLOC(mat->num_paths*pMsz);
		msg = "mat->path_list in set_tree_driver()";
		if( MEMCHECK(mat->path_list,msg,mat->num_paths*pMsz) )
			clean_up(0);

		n = 0;
		for( map = map_root;  map != NULL;  map = map->next )
		{
			if( map->on_tree ) continue;
			mat->path_list[n] = map;
			n++;
		}
	}
}


MA_tree	*set_ma_tree(MA_set *mac, MA_tree **mat_root, MA_tree *prev_mat)
{
	MA_tree	*mat;
	MA_set	*map, *add_map, *next_mac;

	char	*msg;

	int	np, nc, na;
	int	path_alloc_size, clust_alloc_size, alloc_diff;

	int	pMsz = sizeof(MA_set *);

	mat = create_next_MA_tree(mat_root,prev_mat);

	clust_alloc_size = ALLOC_SZ;
	mat->clust_list = (MA_set **)MALLOC(clust_alloc_size*pMsz);
	msg = "mat->clust_list in set_mat_tree()";
	if( MEMCHECK(mat->clust_list,msg,clust_alloc_size*pMsz) ) clean_up(0);

	mat->num_clust = 1;
	mat->clust_list[0] = mac;
	mac->on_tree = 1;

	path_alloc_size = mac->num_att;
	mat->path_list = (MA_set **)MALLOC(path_alloc_size*pMsz);
	msg = "mat->path_list in set_mat_tree()";
	if( MEMCHECK(mat->path_list,msg,path_alloc_size*pMsz) ) clean_up(0);

	mat->num_paths = mac->num_att;
	for( na = 0;  na < mac->num_att;  na++ )
	{
		map = mac->att_mas[na];
		mat->path_list[na] = map;
		map->on_tree = 1;
	}

	for( np = 0;  np < mat->num_paths;  np++ )
	{
	    map = mat->path_list[np];
	    for( nc = 0;  nc < 2;  nc++ )
	    {
		next_mac = map->att_mas[nc];
		if( next_mac == NULL  ) continue;
		if( next_mac->on_tree ) continue;

		if( mat->num_clust == clust_alloc_size )
		{
		    clust_alloc_size += ALLOC_SZ;
		    mat->clust_list = (MA_set **)REALLOC(mat->clust_list,
							clust_alloc_size*pMsz);
		    msg = "mat->clust_list realloc in set_mat_tree()";
		    if( MEMCHECK(mat->clust_list,msg,ALLOC_SZ*pMsz) ) clean_up(0);
		}

		next_mac->on_tree = 1;
		mat->clust_list[mat->num_clust] = next_mac;
		(mat->num_clust)++;

		path_alloc_size += next_mac->num_att;
		mat->path_list = (MA_set **)REALLOC(mat->path_list,
							  path_alloc_size*pMsz);
		msg = "mat->path_list realloc in set_mat_tree()";
		if( MEMCHECK(mat->path_list,msg,next_mac->num_att*pMsz) )
			clean_up(0);

		for( na = 0;  na < next_mac->num_att;  na++ )
		{
		    add_map = next_mac->att_mas[na];
		    if( add_map->on_tree ) continue;

		    add_map->on_tree = 1;
		    mat->path_list[mat->num_paths] = add_map;
		    (mat->num_paths)++;
		}
	    }
	}

	alloc_diff = clust_alloc_size - mat->num_clust;
	if( alloc_diff > 0 )
	{
		mat->clust_list = (MA_set **)REALLOC(mat->clust_list,
							  mat->num_clust*pMsz);
		msg = "mat->clust_list final realloc in set_mat_tree()";
		if( MEMCHECK(mat->clust_list,msg,-alloc_diff*pMsz) ) clean_up(0);
	}

	alloc_diff = path_alloc_size - mat->num_paths;
	if( alloc_diff > 0 )
	{
		mat->path_list = (MA_set **)REALLOC(mat->path_list,
							mat->num_paths*pMsz);
		msg = "mat->path_list final realloc in set_mat_tree()";
		if( MEMCHECK(mat->path_list,msg,-alloc_diff*pMsz) ) clean_up(0);
	}

	return mat;
}
