/*
*	Copyrighted, Research Foundation of SUNY, 1998
*/

#ifndef _min_surf_h
#define _min_surf_h
#endif

typedef	struct	_network_adj_list
{
	int				index_in_cube;
	int				length;
	struct	_network_adj_list	*next;
} network_adj_list;


typedef	struct	_g_change_list
{
	int			index_in_cube;
	int			start_vox;
	int			end_vox;
	int			min_ma;
  	int			ma_s;
	int			ma_e;
	float			pa[9];
	struct	_g_change_list	*next;
} g_change_list;


typedef struct _network_node
{
	int			index_in_cube;
	float			 forward_angle;
	float			backward_angle;
	float			 forward_label;
	float			backward_label;
	float			 forward_3d_area;
	float			backward_3d_area;
	char			f_tmp;
	char			b_tmp;
	network_adj_list	*nw_adj_list;
	struct _tmp_indx_chain	*for_in_indx_chain;
	struct _tmp_indx_chain	*bac_in_indx_chain;
	struct _network_node	*back_pointer;
	struct _network_node	*for_prev_path;
	struct _network_node	*bac_prev_path;
	struct _network_node	*next;
} network_node;


typedef struct _grain_chain
{
	int	index_in_cube;
	struct	_grain_chain *next;
} grain_chain;


typedef struct _associated_grain_chain
{
	grain_chain *head_of_grain_chain;
	grain_chain *tail_of_grain_chain;
} associated_grain_chain;


typedef struct _gr_bdry_vect_elem
{
	int			cube_index;
	associated_grain_chain	*p_associated_gc;
	network_node		*nw_node;
} gr_bdry_vect_elem;


typedef struct _distinct_chain
{
	associated_grain_chain 	*p_associated_gc;
	struct _distinct_chain	*next;
} distinct_chain;


typedef struct _tmp_indx_chain
{
	struct _tmp_indx_chain	*prev;
	network_node		*nw_node;
	struct _tmp_indx_chain	*next;
}tmp_indx_chain;


#define	Set_node_value()\
{\
	nbr_index = grain_index + sten26[i];\
	if( cube[nbr_index] <= -1000 )\
	{\
		gr_bdry_index = -(cube[nbr_index]+1000);\
		p_tmp = gr_bdry_vect[gr_bdry_index].p_associated_gc;\
\
		if( tmp1 == p_tmp ) node_value[i] = 1;\
	}\
}


static int diff_2_index[3][3][3] = { { {-1,21,4}, {15,24,7}, {10,18,1} },
			      { {13,22,5}, {16,25,8}, {11,19,2} },
			      { {12,20,3}, {14,23,6}, {9,17,0} } };  
				       
/*
*    diff_2_index works like this:
*
*	if voxel1 and voxel2 are 26-neighbor voxels with indices index1
*							     and index2
*    and
*	xdiff, ydiff and zdiff are the differences in their respective
*							coordinates
*	i.e.,	xdiff = (xcoord of index1) - (xcoord of index2)
*
*    then
*	diff_2 index[xdiff][ydiff][zdiff] is the index (0 to 25) of voxel1
*					  in the 26 neighborhood of voxel2.
*
*	if xdiff, ydiff or zdiff is -1 it is replaced by 2
*
*	i.e., diff_2_index[.][.][-1] is actually diff_2_index[.][.][2]
*/
