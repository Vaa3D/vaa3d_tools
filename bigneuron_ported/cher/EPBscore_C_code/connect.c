/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

/*
*	label.c - this file contains all the routines to label
*	the point and its octree. These routines therefore
*	determine if there is a single connected object in
*	the neighborhood of point v
*/

#include "stdafx.h"

#include "data.h"
#include "lkc_prototype.h"

#define OCT_MARKED 9

#define DISCONNECTED 0
#define CONNECTED 1


int	local_connectivity(point_info *point)
{
	unsigned char	*pnl;

	int	 oct_num[NUM_OCTANTS];
	int	oct_list[NUM_OCTANTS];

	int	i, j, k;
	int	node, this_oct, nbr_oct;
	int	oct_list_ind, add_oct_list_ind;

		/* zero oct_num[], oct_list[] */

	for(  i = 0;  i < NUM_OCTANTS;  i++ )
	{
		oct_num[i] = 0;		oct_list[i] = 0;
	}
	oct_list_ind = -1;

	/*** dbg_print_node_values(point); ***/


	/* 	FIRST PASS THROUGH OCTANTS	 */
	/* scan through the octants sequentially */
	/* until we find one with a VOID voxel	 */

	for( i = 1;  i < NUM_OCTANTS;  i++ )
	{
	    pnl = point->node_list[i];	/* this is just data.h:nlist[][] */

	    for( j = 0;  j < NUM_OCTANT_NODES;  j++ )
	    {
		node = pnl[j];
		if( point->node_value[node] == VOIDVOX )
		{
			oct_list_ind++;
			oct_list[oct_list_ind] = i;
			oct_num[i] = OCT_MARKED;
			goto found;
		}
	    }
	}

    found:
	/*** dbg_print_oct_num_list(oct_num,oct_list); ***/

	if( oct_list_ind == -1 ) return DISCONNECTED;

	add_oct_list_ind = oct_list_ind;
	while( (this_oct = oct_list[oct_list_ind]) != 0 )
	{
	    pnl = point->node_list[this_oct];
	    for( j = 0;  j < NUM_OCTANT_NODES;  j++ )
	    {
		node = pnl[j];
		if( point->node_value[node] == VOIDVOX )
		{
		    point->node_value[node] = NV_MARKED;

			/* add unmarked neighboring octants to oct_list[] */
		    for( k = 1;  k <= (int)olist[node][0];  k++ )
		    {
			nbr_oct = olist[node][k];
			if( oct_num[nbr_oct] == OCT_MARKED ) continue;
			oct_num[nbr_oct] = OCT_MARKED;
			add_oct_list_ind++;
			oct_list[add_oct_list_ind] = nbr_oct;
		    }
		}
	    }
	    /*** dbg_print_oct_num_list(oct_num,oct_list); ***/

	    oct_list_ind++;
	}

			/* FINAL PASS THROUGH NODES  */
		/* check to see if any nodes are unmarked */

	for( i = 0;  i < NUM_NEIGHBORS;  i++ )
	{
		if( point->node_value[i] == VOIDVOX ) return DISCONNECTED;
	}

	return CONNECTED;
}


int	local_6_connectivity(point_info *point)
{
	unsigned char	inc_lbl;
	unsigned char	*pnv;

	inc_lbl = 0;
		
	/* scan point's 6-neighbours sequentially */
	/* until we find one with a VOID voxel	  */
	/* When found, call rec_6_connect() to determine whether */
	/* all VOID voxels except for point are mutually 6-connected  */

	pnv = point->node_value;

	     if( pnv[ 4] == VOIDVOX ) rec_6_connect(point, 4,NV_MARKED,&inc_lbl);
	else if( pnv[10] == VOIDVOX ) rec_6_connect(point,10,NV_MARKED,&inc_lbl);
	else if( pnv[12] == VOIDVOX ) rec_6_connect(point,12,NV_MARKED,&inc_lbl);
	else if( pnv[13] == VOIDVOX ) rec_6_connect(point,13,NV_MARKED,&inc_lbl);
	else if( pnv[15] == VOIDVOX ) rec_6_connect(point,15,NV_MARKED,&inc_lbl);
	else if( pnv[21] == VOIDVOX ) rec_6_connect(point,21,NV_MARKED,&inc_lbl);

	/* check to see if any VOID voxel unmarked, ie. not 6-connected */
	if(    (inc_lbl == 0)
	    || (pnv[ 4] == VOIDVOX) || (pnv[10] == VOIDVOX) || (pnv[12] == VOIDVOX)
	    || (pnv[13] == VOIDVOX) || (pnv[15] == VOIDVOX) || (pnv[21] == VOIDVOX)  )
	{
		return DISCONNECTED;
	}
	else	return CONNECTED;
}


/*
*	recursively determine whether all VOID voxels in a 3x3x3 block
*	except for the central voxel, 'point', are mutually 6-connected
*/

void	rec_6_connect(point_info *point, char ind, char label,
					  unsigned char *inc_lbl)
{
	int	i;
	
	if( point->node_value[ind] == VOIDVOX )
	{
		*inc_lbl = 1;
		point->node_value[ind] = label;
		for( i = 1;  i <= (int)nblist[ind][0];  i++ )
			rec_6_connect(point,nblist[ind][i],label,inc_lbl);
	}
}


void	dbg_print_node_values(point_info *point)
{
	unsigned char	*pnl;

	int	i, j;
	int	node;

	printf("\nnode_value[]\n");
	for( i = 1;  i < NUM_OCTANTS;  i++ )
	{
	    pnl = point->node_list[i];

	    printf("%d:  ",i);
	    for( j = 0;  j < NUM_OCTANT_NODES;  j++ )
	    {
		node = pnl[j];
		printf("%d ",point->node_value[node]);
	    }
	    printf("\n");
	}
	printf("\n");
}


void	dbg_print_oct_num_list(int *oct_num, int *oct_list)
{
	int	i;

	printf("\noct_num[]\n");
	for( i = 0;  i < NUM_OCTANTS;  i++ ) printf("%d ",oct_num[i]);
	printf("\n");

	printf("\noct_list[]\n");
	for( i = 0;  i < NUM_OCTANTS;  i++ ) printf("%d ",oct_list[i]);
	printf("\n");
}
