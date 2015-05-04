/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _maset_h
#define _maset_h

struct _MA_set
{
	struct _MA_set	 *prev;
	struct _MA_set	 *next;
	int		  ind;		/* I/O index of MA_set */
	int		  num_in_set;	/* number of voxels in set */
	float		  len;		/* length of map_sets */
	int		 *loc;		/* indices of location in volume */
	unsigned char	 *bdat;		/* burn numbers */
	int		  num_att;	/* number attached MA_sets */
	int		 *attach_list;	/* attached vox indices for mac_sets */
	struct _MA_set	**att_mas;	/* addresses of attached MA_sets */
	unsigned char	  on_tree;	/* assigned to tree ? */
	int		  surf_rmnt;	/* cluster is MS remnant */

		/* additional entries for fibers */
	int               num_cinf;	/* number clusters on fiber */
	int              *cinf;         /* loc list of clusters in fiber */
	int              *pinf;         /* dirs of path components in fiber */

		/* additional entries for neuron */
	int		  level;	/* branch order */
} ;
typedef struct _MA_set MA_set;


struct _MA_tree
{
	struct _MA_tree	 *prev;
	struct _MA_tree	 *next;
	int		  num_paths;
	struct _MA_set	**path_list;
	int		  num_clust;
	struct _MA_set	**clust_list;
} ;
typedef struct _MA_tree MA_tree;


typedef struct _CMrg_menu
{
	int	do_cmrg;
	char	method;
	int	ntol;
} CMrg_menu;


#define Ind_2_ijk(Iind,Ii,Ij,Ik,_nx,_nxy)\
{\
	int	Itmp = Iind;\
\
	Ik = Itmp / _nxy;	Itmp = Itmp % _nxy;\
	Ij = Itmp / _nx;	  Ii = Itmp % _nx;\
}


/*
*	As in Ind_2_ijk but trims cube padding
*/

#define Pad_ind_2_ijk(Iind,Ii,Ij,Ik,_nx,_nxy)\
{\
	Ind_2_ijk(Iind,Ii,Ij,Ik,_nx,_nxy)\
	Ik--;	Ij--;	Ii--;\
}
#endif	/* _maset_h */

typedef struct _DBG_menu
{
	int	do_MAS;
	int	do_SR;
} DBG_menu;

typedef struct _Mod_Menu {
	int	use_bno;
	int	bl_keep_bdry, bl_keep_z_bdry;
	int	bl_mod,	bl_thresh, bl_fast;
	int	ne_mod,	ne_thresh;
	int	ivb_mod, ivb_thresh;
	int	ip_mod,	ip_thresh;
	int	sr_red;
} Mod_Menu;

typedef struct _MA_hist_menu
{
	int	do_path_len;
	int	do_path_tort;
	int	do_clust_sz;
	int	do_coord_num;
	int	do_coord3;
	int	do_c3_stats;
	int	do_path_brn;
	int	do_cp_scatt;
	int	do_iv_bnum;
	int	do_clust_rad;
	int	do_moi;
	int	do_throat_char;

	char	title[80];
	float	vox_len;
} MA_hist_menu;


