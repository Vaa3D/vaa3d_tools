/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

/* Function prototype declarations for functions defined in ma directory */

#ifndef _isoseg_h
#include "isoseg.h"
#endif

/* Not supporting raster file anymore
#ifndef _my_ras_h
#include "my_ras.h"
#endif
*/

#ifndef _my_ma_h
#include "my_ma.h"
#endif

#ifndef _data_h
#include "data.h"
#endif

#ifndef _maset_h
#include "maset.h"
#endif

#ifndef _min_surf_h
#include "min_surf.h"
#endif

#ifndef _tree_h
#include "tree.h"
#endif

//extern	"C"
//{

#include "stdAfx.h"

/* from my_ma.c */
mxArray * mexburnmain(const mxArray *input);
void	dir_set(Fdat *tmp, int task, int NXYZ);
	void	burnmain(
			unsigned char *seg_cube, 
			Bchains *bc, 
			Fdat *tmp,
			int nx,
			int ny, 
			int nz
			);

	/* from bchain.cpp */
	void	initialize_chains(char type, int **chain, int *num, int chain_size);
	void	realloc_chains(char type, int **chain, int chain_size,
						   int size_chng);
	void	free_chains(int *chain, int chain_size);
	void	set_first_chain(Bchains *bc, unsigned char *cube, int bdry_num,
							int nxyz);
	void	add_to_chain(Bchains *bc, int v_ind, int realloc_size);
	void	set_next_chain(Bchains *bc, unsigned char *bfdat,
						   unsigned char b_step);

	/* from gbdry.cpp */
	void    set_sten_6(int nx, int nxy, int *sten6);
	void	set_6_stencil(int nx, int nxy, int *sten);
	void	set_stencil(int nx, int nxy, int *sten);

	void	check_bdry_6_nbr(unsigned char *pbd, int pos, int nx, int ny,
						 int nz, int nxy, int *is_in);
	void	check_bdry_26_nbr(unsigned char *pbd, int pos, int nx, int ny,
						  int nz, int nxy, int *is_in);

	void	set_bdry_smoothing(int *smooth_bdry);
	void	smooth_gbdry_1(unsigned char *cube, int nx, int ny, int nz, int nxy,
						   int nxyz, int sten_sz, unsigned char this_phase,
						   unsigned char other_phase);
	void	smooth_gbdry_2(unsigned char *cube, int nx, int ny, int nz, int nxy,
						   int nxyz, int sten_sz, unsigned char this_phase,
						   unsigned char other_phase);
	void	create_gbdry(Bchains *bc, Fdat *tmp,
						 int *g_num, int nx, int ny, int nz,
						 int task, int *psur);
	void	check_if_gbdry(Fdat *tmp, int nx, int ny, int nz, int ix, int iy,
						   int iz, int pos, int *bdry_num, int *num_pg_faces,
						   int *sten6, int *sten26, int task);
	void	check_bdry_at_edge(unsigned char *fdat, int nx, int ny, int nz,
							   int ix, int iy, int iz, int pos, int *pis_bv,
							   int *sten26, int PORE_val);
	void	count_surf_faces_at_edge(unsigned char  *fdat, int nx, int ny,
									 int nz, int ix, int iy, int iz, int pos,
									 int *vox_num_pg_face, int *sten6,
									 int PORE_val);

	/* from ma_90.cpp */
	void	set_90_ma(Fdat *tmp, int pos, unsigned char dir,
					  unsigned char b_step, unsigned char now);
	void	check_90(Fdat *tmp, int nx, int ny, int nz, int use_bdry);
	void	check_90_ma(Fdat *tmp, int i, int j, int k, int pos, int use_bdry,
						int nx, int ny, int nz, int nxy);
	void	check_burn_num(Fdat *tmp, int i, int j, int k, int pos, int pos1,
						   int pos2, int use_bdry, int nx, int ny, int nz);

	/* from ma_opp.cpp */
	void	check_opposite(Bchains *bc, Fdat *tmp, int nx, int ny, int nz);
	void	find_op(Fdat *tmp, int k, int pos, int go, int nx, int nxy, int nz);


	/* from mastep.cpp */
	void	surf_ma_step(Bchains *bc, Fdat *tmp, int pos, char b_step, int nx,
						 int nxy, int realloc_size);


	/* from surfbrn.cpp */
	void	surf_burn(Bchains *bc, Fdat *tmp, int nx, int ny, int nz,
					  int use_bdry, int task, int *mbn, int *period);
	void    set_6nbr_bd(int ix, int iy, int iz, unsigned char *pbd, int NX,
						int NY, int NZ);
	void	check_pz(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY);
	void	check_nz(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY);
	void	check_py(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY, int NX);
	void	check_ny(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY, int NX);
	void	check_px(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY);
	void	check_nx(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
					 int pos, int NXY);

	/* from madat1.cpp */
void	input_pma_data_arrays(int *pnx, int *pnxy, int *pnxyz, int *pzs,
							  int *pze, int *ptot_ma, int **pma_ind,
							  unsigned char **pbdat, int **pcube);
void	input_pma_data_arrays_2(int *Xs, int *Xe, int *Ys, int *Ye, int *Zs,
								int *Ze, int *Ts, int *Te, int *trim,
								char **mafn_base, char **mafn_name,
								char *ext, char **brnfn_base,
								char **brnfn_name, int *mafn_len,
								int *brnfn_len, int *is_cmprss,
								int *brn_cmprss, int hide_rsz,
								char *file_basename);
//void	set_pma_data_arrays(int Xs, int Xe, int Ys, int Ye, int Zs, int Ze,
//							int ts, int trim, char *mafn_base,
//							char *mafn_name, char *ext, char *brnfn_base,
//							char *brnfn_name, int mafn_len, int brnfn_len,
//							int is_cmprss, int brn_cmprss, int *pnx,
//							int *pnxy, int *pnxyz, int *ptot_ma,
//							int **pma_ind, unsigned char **pbdat,
//							int **pcube);
void	set_pma_data_arrays(int nx, int ny, int nz, int trim,
							int Nx, int Nxy, int Nxyz,
							int *tot_ma, int **ma_ind,
							unsigned char **bdat, int **pcube,
							unsigned char *burn_cube);
void	accum_pma_dat(char *infile_name, char *ext, int infile_len,
					  int in_cmprss, int zs, int ze, int Nx, int Ny,
					  int **pma_ind, unsigned char **pbdat, int *tot_pma,
					  int fzs, int ts);
void	resize_ma_to_cube(int **pma_ind, unsigned char **pbdat,
						  int *ptot_ma, int Nx, int Ny, int Xs, int Xe,
						  int Ys, int Ye);
void	set_cube_ma(int *cube, int *ma_ind, int tot_ma, int val);
void	correct_ma_for_trim(int **pma_ind, unsigned char **pbdat,
							int *ptot_ma, int *cube, int nx, int nxy,
							int nxyz);
void	dbg_print_ma_list(char *strng, int *ma_ind, int tot_ma, int *cube,
						  int nx, int nxy);
int	set_ma_abut_val(int ind, int *stencil, int *cube);
void	ma_data_realloc(int **pma_ind, unsigned char **pbdat, int tot_ma,
						int nvox);
void	cross_link_ma_list_cube_entries(int *ma_list, int n_ma_vox,
										int *cube);
void	unlink_cube_from_ma_list_entries(int *ma_list, int n_ma_vox,
										 int *cube);
void	debug_print_ma_cube(int *ma_ind, int tot_ma, int *cube);

/* from madup.cpp */
//void	set_cube_info_from_burn_files(char *infile_name, int infile_len,
//									  int is_cmprss, int Xs, int Xe, int Ys,
//									  int Ye, int Zs, int Ze, int nx,
//									  int nxy, int nxyz, void *cube,
//									  int cube_sz, int ts);
void	set_cube_info_from_burn_files(int Xs, int Xe, int Ys,
									  int Ye, int Zs, int Ze, int nx,
									  int nxy, int nxyz, void *cube,
									  int cube_sz,
									  unsigned char *burn_cube);
void	s_mark_cube_exterior(short *cube, int nx, int nxy, int nxyz);
void	i_mark_cube_exterior(int *cube, int nx, int nxy, int nxyz);
void	s_mark_cube_slc_fid_ext(short *slc, int nx, int nxy, int Nx, int Ny,
								int Xs, int Xe, int Ys, int Ye,
								unsigned char *tmp_burn);
void	i_mark_cube_slc_fid_ext(int *slc, int nx, int nxy, int Nx, int Ny,
								int Xs, int Xe, int Ys, int Ye,
								unsigned char *tmp_burn);
void	s_mark_cube_slc_grains(int Nx, int Ny, unsigned char *tmp_burn,
							   int Xs, int Xe, int Ys, int Ye, short *slc);
void	i_mark_cube_slc_grains(int Nx, int Ny, unsigned char *tmp_burn,
							   int Xs, int Xe, int Ys, int Ye, int *slc);
void	s_trim_cube_ma(short *pma, short val, int nx, int nxy, int nxyz,
					   int *pma_cnt);
void	i_trim_cube_ma(int *pma, int val, int nx, int nxy, int nxyz,
					   int *pma_cnt);
/* matort.c */
void set_quant_av(int *ph, int nbin, float *av);
void hist_ma_quant_2_std(char *ma_type, char *quant_type, int *all_ph, int *bb_ph, int *bl_ph, int *ll_ph, float av_all, float av_bb, float av_bl, float av_ll, float *quant_bin_val, int nbin);
/* from mahist.cpp */
void	pma_hist_driver(char *file_basename);
void	pma_nhbr_hist(unsigned char *num_nhbr, int n_ma_vox, char *title);
void	nhbr_hist_2_jgr(int *nhbr_hist, char *title);
void	hist_ma_len(char *ma_type, MA_set *map_root, int nx, int nxy,
					float vox_len, char *title, int to_std, int to_jgr);
void	hist_clust_size(MA_set *mac_root, int nx, int nxy, char *title,
						int to_std, int to_jgr);
void	hist_clust_size_2_std(int *bc_hist, int *sr_hist, int max_sz);
void	hist_clust_size_2_jgr(int *bc_hist, int *sr_hist, int max_sz,
							  char *title);
void	hist_coord_nums(MA_set *mac_root, char *title, int to_std,
						int to_jgr);
void	hist_coord_nums_2_std(int *bcc_hist, int *src_hist, int max_cn);
void	hist_coord_nums_2_jgr(int *bcc_hist, int *src_hist, int max_cn,
							  char *title);
void	hist_coord3_len(MA_set *map_root, MA_set *mac_root, char *title);
void	hc2_len_2_jgr_driver(int **len_hist, int max_cn, char *title);
void	hist_path_brn(MA_set *map_root);
void	tree_clust_path_scatterplot(MA_tree *mat_root, MA_tree *iso_mat,
									char *title);
int	max_bnum_on_MA_sets(MA_set *mas_root);
void	coord3_stats(MA_set *map_root, MA_set *mac_root, int *bc3_num);
void	hist_iso_vox_bnum(MA_set *mas, char *title, int to_std, int to_jgr);
void	iv_bnum_2_std(int *ivb_hist, int max_bnum);
void	iv_bnum_2_jgr(int *ivb_hist, int max_bnum, char *title);
void	hist_clust_unassigned_paths(MA_set *mac_root, int *with_one);
void	hist_clust_radius(MA_set *mac_root, float vox_len, char *title,
						  int to_std, int to_jgr);
void	hist_clust_radius_2_std(int *bn_hist, int max_bn);
void	hist_clust_radius_2_jgr(int *bn_hist, int max_bn, float vox_len,
								char *title);
void	hist_throat_chars(MA_set *map_root);
/* from mamod.cpp */
mxArray * mextrimmain(const mxArray *input);
void ma_modify_driver(int nx, int ny, int nz,
						 unsigned char *burn_cube, int *pnloc,
						 int **pma_loc, unsigned char **pma_bno,
						 int is_spine,  int LL_PATH_TOL, int BL_PATH_TOL, int use_fast, int ntol, int do_cmrg);
void	set_ma_modify(Mod_Menu *mod, int LL_PATH_TOL,
					  int BL_PATH_TOL, int use_fast);
void	init_ma_modify(int max_bnum, Mod_Menu *mod);
void	init_iv_ip_cleanup(char *type, char *crit, int *mod, int *thresh);
void	init_bl_ne_cleanup(char *type, int *mod, int *thresh,
						   int *keep_bdry, int *keep_z_bdry, int *bl_fast);
int	init_surf_rmnt_cleanup();
void	cleanup_iv_in_cube(int *cube, MA_set *mas, int ivb_thresh,
						   int max_bnum, int *iv_rem);
void	cleanup_ip_in_cube(int *cube, MA_set *map_root, int ip_thresh,
						   int *bc_done);
void	cleanup_bl_ne_in_cube(int *cube, MA_set *map_root, MA_set *mac_root,
							  Mod_Menu *mod, int *bc_done, int nx, int nxy
							  );
void	remove_path(MA_set *smap, MA_set *mac, int *bc_done, int max_coord,
					int *cube, int *c_end, int nx, int nxy);
void	find_path_to_remove(int *cube, int *stencil, int nx, int nxy,
							MA_set *mac, MA_set **map, int *l,
							Mod_Menu *mod, MA_set **smap);
void	thin_bc_driver(int **loc, int *numl, int *c_end, int numc, int nx,
					   int nxy);
void	set_index_limits(int *loc, int numl, int nx, int nxy, int min[3],
						 int max[3]);
void	reset_ma_list_from_cube(int *cube, int **pma_ind,
								unsigned char **pbdat, int *pn_ma_vox);
void	output_modified_ma_files(int zs, int ze, int nx, int nxy, int nxyz,
								 int n_ma_vox, int *ma_list,
								 unsigned char *burn_dat, char *maf_name,
								 char *ext, int maf_len, int maf_cmprss,
								 int ts);
void	project_modified_ma_files(int nx, int nxy, int nxyz, int n_ma_vox,
								  int *ma_list, unsigned char *burn_dat,
								  int *cube, char *out_name, int out_len,
								  int out_cmprss, int ts);
void	project_ma(int *cube, int *ma_list, unsigned char *burn_dat, int nx,
				   int nxy, int nxyz, int n_ma_vox,
				   unsigned char **proj_cube, unsigned char **proj_bdat,
				   int *bmin, int *bmax);
void	print_int_array(char *msg, int *idat, int nx, int nxy, int nxyz);
void	print_ma_nhbr_list(unsigned char *num_nhbr, int **nhbr_list,
						   int *ma_list, int n_ma_vox, int nx, int nxy);


/* masets.c */
void set_voxel_phys_len(float *vox_len);
void set_clust_paths(int nx, int nxy, int nxyz, int n_ma_vox, int *ma_list, unsigned char *burn_dat, int *cube, CMrg_menu *cmrg, MA_set **pmas_root, MA_set **pmap_root, MA_set **pmac_root, int *pn_sngl, int *pn_path, int *pn_clust);
void set_MA_hist_menu(MA_hist_menu *menu);
void set_DBG_menu(DBG_menu *dbg);
void set_nhbr_offsets(int nx, int nxy, int *offset);
void set_nhbr_list(unsigned char **num_nhbr, int ***nhbr_list, int *ma_list, int n_ma_vox, int *cube, int *offset);
void free_nhbr_list(unsigned char *num_nhbr, int **nhbr_list, int n_ma_vox);
void set_MA_sets(int *ma_list, int **nhbr_list, unsigned char *num_nhbr, unsigned char *burn_dat, int *cube, int *offset, int n_ma_vox, int nx, int nxy, MA_set **mas_root, MA_set **map_root, MA_set **mac_root, int *n_sngl, int *n_path, int *n_clust);
void count_MA_sets(MA_set *mas_root, int *pn_mas);
void rec_ma_path(unsigned char *done_list, unsigned char *num_nhbrs, int *nhbr_list[], int *cube, int prev_ind, int this_ind, int **path_list, int *num_on_path, int *path_n);
void rec_ma_cluster(unsigned char *done_list, unsigned char *num_nhbrs, int *nhbr_list[], int *cube, int this_ind, int **clust_list, int *num_in_clust, int *clust_n, int *c_vec_sz, int **att_list, int *num_att, int *att_vec_sz, int nx, int nxy);
void set_MA_set_bdat(int *cube, MA_set *mas, unsigned char *burn_dat);
extern __declspec(dllexport) MA_set *create_next_MA_set(MA_set **root, MA_set *prev_set);
void invert_set_path(MA_set *map);
void dbg_print_coord2_clust(MA_set *mac_root, int nx, int nxy);
void dbg_print_MA_sets(MA_set *mas_root, MA_set *map_root, MA_set *mac_root, int nx, int nxy);
void print_MA_sets(char *mas_type, char *att_mas_type, MA_set *root, int nx, int nxy);
void print_surf_rmnts(MA_set *mac_root, int nx, int nxy);
void print_MAS(char *mas_type, char *att_mas_type, MA_set *mas, int i, int nx, int nxy);
void print_attached_MA_sets(char *att_mas_type, MA_set **att_mas, int num_att);
void print_loc_set(int *set, int num_in_set, int nx, int nxy);
void print_loc_brn_set(int *set, unsigned char *bdat, int num_in_set, int nx, int nxy);
void rec_free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz);
void free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz);
void count_ma_vox_processed(MA_set *mas_root, MA_set *map_root, MA_set *mac_root, unsigned char *num_nhbrs, int *cube);
void link_path_2_cluster(MA_set **map_root, MA_set *mac_root, int nx, int nxy, int *n_path);
void link_3vox_path_2_cluster(MA_set **map_root, MA_set *map, MA_set *mac, int nx, int nxy, int *cur_att_n, int *num_3vox_2_c);
void set_MAS_ind(MA_set *mas_root, int ind_sgn);
void set_path_len(MA_set *map_root, int nx, int nxy);
void set_map_len(MA_set *map, int nx, int nxy);
void set_surf_rmnts(MA_set *mac_root, int nx, int nxy);
int is_surf_rmnt(int *cloc, int cnum, int *offset, int nx, int nxy);
void summarize_cluster_info(MA_set *mac_root);

/* matree.c */
MA_tree *create_next_MA_tree(MA_tree **root, MA_tree *prev_tree);
void dbg_print_MA_trees(MA_tree *mat_root, MA_tree *iso_mat);
void print_MA_trees(MA_tree *mat_root);
void summarize_MA_info(MA_tree *mat_root, MA_tree *iso_mat, MA_set *mas_root, MA_set *mac_root);
void print_mat(MA_tree *mat);
void rec_free_MA_tree(MA_tree *mat, int pMsz, int MTsz);
void MA_tree_driver(MA_set *mac_root, MA_set *map_root, MA_tree **mat_root, MA_tree **iso_mat);
MA_tree *set_ma_tree(MA_set *mac, MA_tree **mat_root, MA_tree *prev_mat);
/* from mod_rmnt.cpp */
void	modify_surf_rmnt(int *cube, MA_set *mac_root, int nx, int nxy,
						 int *bc_done);
void	setup_network_generic(int *tloc, int *cube, int *sten26,
							  network_node **snw_node,
							  network_node **enw_node, char conn_type,
							  int cnum, network_node **cube_2_network,
							  int compare);
void	initialize_nw_node_generic(network_node *p_nw_node,
								   network_node *prev_nw_node,
								   int index, int *cube, int *sten26,
								   char conn_type,
								   network_node **cube_2_network,
								   int compare, int *cnt);
grain_chain    *get_shortest_path_generic(network_node *snw_node,
										  network_node *enw_node, int *cube,
										  int nx, int nxy, int cnum,
										  int *sten26,
										  network_node **cube_2_network,
										  int compare);
grain_chain *Bi_Dir_for_Label_generic(network_node *snw_node,
									  network_node **start_node, int *cube,
									  int nx, int nxy, int *ok,
									  network_node **cube_2_network,
									  int compare);
grain_chain *Bi_Dir_bac_Label_generic(network_node *enw_node,
									  network_node **start_node,
									  int *cube, int nx, int nxy, int *ok,
									  network_node **cube_2_network,
									  int compare);
grain_chain *return_shortest_path_generic(network_node *min_nw_node,
										  network_node *start_node,
										  network_node *stop_node, int dir,
										  network_node **cube_2_network,
										  int *cube, int compare);
/* from heap.cpp */
void	int_C_heapsort(int n, int *ra);
void	float_C_heapsort(int n, float *ra);
void	double_C_heapsort(int n, double *ra);
void	cluster_path_heapsort(int n, int *pl, MA_set **ma);
void	two_int_heapsort(int n, int *pl, int *ma);
void	float_int_heapsort(int n, float *pl, int *ma);
/* from fiber.cpp */
void	fiber_driver(char *file_basename);
void	sort_fiber_wrt_angle(int num_fbr, MA_set *fbr_root, int cnt, int nx,
						   int nxy);
void	init_visual_fibers(int *vis, int *nz, int *longest, char **iv_fn);
void	select_cp_set(int *slct, MA_set **smap_root, MA_set *mac_root,
					  int nx, int nxy);
int	delete_leaving_fibers(MA_set *fbr_root, int nx, int nxy);
void	visualize_fibers(char *iv_fn, int num_fbr, MA_set *fbr_root,
						 int cnt, int nx, int nxy, int nz, float dp_tol);
void	set_fibers(MA_set *map_root, MA_set *mac_root, MA_set **fbr_root,
				   int nx, int nxy, float dp_tol, int *num_fbr,
				   int bb_fiber);
void	rec_fiber(MA_set *strt_clust, MA_set *map, MA_set *fbr, int *tot_sz,
				  int realloc_sz, int nx, int nxy, int dbg, float dp_tol);
MA_set	*continue_fiber_past_clust(MA_set *fbr, MA_set *map, MA_set *mac,
								   int nx, int nxy, float dp_tol);
void	match_connecting_paths(MA_set *mac, int nx, int nxy, float dp_tol);
float	get_angle_between_paths(MA_set *mac, MA_set *map_in,
								MA_set *map_out, int sten, int nx, int nxy);
MA_set	*set_path_by_sten(MA_set *fbr, MA_set *mac, int sten, int nx,
						  int nxy, float dp_tol);
void	dbg_print_angle(MA_set *map1, MA_set *map2, MA_set *mac, int sten,
						int nx, int nxy, float dp_tol);
void	set_path_vec(int is_fbr, MA_set *map, MA_set *mac, int *sten,
					 int nx, int nxy, float *vec, float *t);
void	lsq_fit_t(int sten, float *vec, float *t);
void	add_map_2_fiber(MA_set *fbr, MA_set *strt_clust, MA_set *map,
						int *tot_sz, int realloc_sz);
void	add_last_voxel_in_map_2_fbr(MA_set *fbr, MA_set *end_clust,
									MA_set *map, int *tot_sz,
									int realloc_sz);
void	alloc_att_mas_array(MA_set *mas);
void	alloc_loc_bdry_arrays(MA_set *mas);
void	realloc_loc_bdry_arrays(MA_set *mas, int needed, int *tot_sz,
								int realloc_sz);
void	final_set_loc_bdry_arrays(MA_set *mas, int *tot_sz);
int	set_fiber_thru_cluster(MA_set *fbr, MA_set *mac, MA_set *imap,
						   MA_set *omap, int *tot_sz, int realloc_sz,
						   int nx, int nxy);
void	thin_surf_rmnt(int **ploc, int *pnum, int nx, int nxy);
int	is_child(int child, int par, int *offset);
int	order_ind_on_path(int *loc, int num, int nx, int nxy);
void	set_cbdat(MA_set *mac, int *cloc, unsigned char **cbdat, int cnum,
				  int nx, int nxy);
void	add_cpath_2_fiber(MA_set *fbr, int *c_end, int *cloc,
						  unsigned char *cbdat, int num, int *tot_sz,
						  int realloc_sz);
void	print_unassigned_map_count(MA_set *map_root, char *msg);
void	hist_ma_moi_driver(char *ma_type, MA_set *map_root, int nx, int nxy,
						   char *title);
MoI	*compute_moi(MA_set *map_root, int nx, int nxy);
void	fit(float *x, float *y, int strt, int end, float *sig, int mwt,
			float *a, float *b, float *siga, float *sigb, float *chi2,
			float *q);

/* from moi.cpp */
MoI	*create_next_MoI(MoI **root, MoI *prev_moi);
void	print_MoI_list(MoI *moi_root, char *mtype);
void	print_MoI(MoI *moi, int i);
void	rec_free_MoI_list(MoI *moi, int Msz);
void	compute_prin_ax(MoI *moi_root);
void	hist_pa_dirs(MoI *moi_root, char *moi_type, char *title,
					 char *file_basename, char *dir);
void	hist_moi(MoI *moi_root, int *nbin, float **bin_val, float **den,
				 float **cum);
void	set_nbin(int nval, int *nbin);

/* from clust_mrg.cpp */
void	set_CMrg_menu(CMrg_menu *cmrg);
void	merge_clusters(MA_set **mac_root, MA_set **map_root,
					   CMrg_menu *cmrg, int nx, int nxy, int *pn_clust,
					   int *pn_path);
int	max_bnum_in_maset(MA_set *mas);
int	merge_mac2_into_mac1(MA_set **map_root, MA_set **mac_root, MA_set *mac1,
						 MA_set *mac2, int ntol, int nx, int nxy, int dbg);
int	merge_map_into_mac1(MA_set **map_root, MA_set **mac_root, MA_set *mac1,
						MA_set *map2, int nx, int nxy, int dbg);

/* from madat.cpp */
void	input_pma_data(int *pnx, int *pnxy, int *pnxyz, short **ppma,
					   unsigned char **pburn_dat, char cnt_vox,
					   int *num_vox, int *trim, int *pzs, int *pze);
void	input_pma_data_2(int *Xs, int *Xe, int *Ys, int *Ye, int *Zs,
						 int *Ze, int *Ts, int *Te, int *trim,
						 char **infile_base, char **infile_name, char *ext,
						 char **brnfn_base, char **brnfn_name,
						 int *infile_len, int *brnfn_len, int *in_cmprss,
						 int *brn_cmprss, char grain_ans[5], char cnt_vox,
						 char *file_basename, int hide_rsz);
void	set_pma_data(int Xs, int Xe, int Ys, int Ye, int Zs, int Ze, int ts,
					 int trim, char *infile_base, char *infile_name,
					 char *ext, char *brnfn_base, char *brnfn_name,
					 int infile_len, int brnfn_len, int in_cmprss,
					 int brn_cmprss, char grain_ans[5], char cnt_vox,
					 int *pnx, int *pnxy, int *pnxyz, short **ppma,
					 unsigned char **pburn_dat, int *num_vox);
void	read_pma_bin_data(FILE *fp, int Xs, int Xe, int Ys, int Ye, int Nx,
						  int Ny, short *pma, unsigned char *burn_dat,
						  int *tot_pma, int swap, int slc_in);
void	unmark_cube_grains(short *cube, int nxyz);
void	count_cube_interior_vox(short *cube, int nx, int nxy, int nxyz,
								int trim, int *num_vox);
void	subtract_interior_edge_vox(short *cube, int nx, int nxy, int nxyz,
								   int *num_vox);
/* thin_cluster.c */
void thin_branch_cluster(int nx, int nxy, int nxyz, int num_keep_pts, int *keep_pts, unsigned char *data, char conn_type, int *min, int t_vox, int dbg_print, int iso_or_dir);
void impose_data_info(data_info *datainfo1, data_info *datainfo, unsigned char *prev_data, int *min);
void impose_keep_pts(data_info *datainfo1, int num_keep_pts, int *keep_pts, int *min, int wx, int wxy);
/* tree.c */
void set_stencil(int nx, int nxy, int *sten);
void set_2D_restricted_stencil(int nx, int nxy, int *sten);
NODE *set_child_node(int ind, unsigned char burn_num, NODE *parent);
short num_children(int ind, short *pma, int *stencil);
void add_children_to_tree(NODE *par, short *pma, unsigned char *burn_dat, int *stencil);
NODE *make_BF_tree(short *pma, unsigned char *burn_dat, int rind, int *stencil, int NXY, int NX);
TREE_LIST *set_pma_trees(short *pma, unsigned char *burn_dat, int nx, int nxy, int nxyz);
void print_node(NODE *nod, int NXY, int NX);
void print_level_nodes(short num_nodes, NODE **node_list, int NXY, int NX);
void print_tree_BF(NODE *root, int NXY, int NX);
void print_TREE_LIST(TREE_LIST *trees, int NXY, int NX);
void print_TREE_LIST_info(TREE_LIST *trees);
void echo_int_list(int n, int *list, char *name);
void compress_int_list(int *n, int *ulist, int *clist);
int BF_tree_count_nodes(NODE *root);
int BF_tree_limits(NODE *root, int *ix_min, int *ix_max, int *iy_min, int *iy_max, int *iz_min, int *iz_max, int NXY, int NX);
void tree_2_pma_driver(TREE_LIST *tree_list, short *pma);
void tree_2_pma(NODE *root, short *pma);
int BF_trim_tree_count_nodes(NODE *root, short *pma);
void free_tree_list(TREE_LIST *tl_ptr);
void free_tree(TREE_LIST *t_ptr);
void free_node(NODE *n_ptr);
/* from masets.cpp */
void	set_voxel_phys_len(float *vox_len);
void	input_MA_sets(MA_set **pmas_root, MA_set **pmap_root,
					  MA_set **pmac_root, int *pnx, int *pnxy, int *pn_sngl,
					  int *pn_path, int *pn_clust);
void	read_and_set_clust_paths(MA_set **pmas_root, MA_set **pmap_root,
								 MA_set **pmac_root, int *pnx, int *pnxy,
								 int *pn_sngl, int *pn_path, int *pn_clust,
								 char *file_basename);
void	set_clust_paths(int nx, int nxy, int nxyz, int n_ma_vox,
						int *ma_list, unsigned char *burn_dat, int *cube,
						CMrg_menu *cmrg, MA_set **pmas_root,
						MA_set **pmap_root, MA_set **pmac_root,
						int *pn_sngl, int *pn_path, int *pn_clust);
void	pma_sets_driver(char *file_basename);
void	hist_ma_info(MA_hist_menu *menu, int nx, int nxy, MA_set *mac_root,
					 MA_set *map_root, MA_set *mas_root, MA_tree *mat_root,
					 MA_tree *iso_mat);
void	set_MA_hist_menu(MA_hist_menu *menu);
void	set_DBG_menu(DBG_menu *dbg);
void	set_nhbr_offsets(int nx, int nxy, int *offset);
void	set_nhbr_list(unsigned char **num_nhbr, int ***nhbr_list,
					  int *ma_list, int n_ma_vox, int *cube, int *offset);
void	free_nhbr_list(unsigned char *num_nhbr, int **nhbr_list, int n_ma_vox);
void	set_MA_sets(int *ma_list, int **nhbr_list, unsigned char *num_nhbr,
					unsigned char *burn_dat, int *cube, int *offset,
					int n_ma_vox, int nx, int nxy, MA_set **mas_root,
					MA_set **map_root, MA_set **mac_root, int *n_sngl,
					int *n_path, int *n_clust);
void	count_MA_sets(MA_set *mas_root, int *pn_mas);
void	rec_ma_path(unsigned char *done_list, unsigned char *num_nhbrs,
					int *nhbr_list[], int *cube, int prev_ind, int this_ind,
					int **path_list, int *num_on_path, int *path_n);
void	rec_ma_cluster(unsigned char *done_list, unsigned char *num_nhbrs,
					   int *nhbr_list[], int *cube, int this_ind,
					   int **clust_list, int *num_in_clust, int *clust_n,
					   int *c_vec_sz, int **att_list, int *num_att,
					   int *att_vec_sz, int nx, int nxy);
void	set_MA_set_bdat(int *cube, MA_set *mas, unsigned char *burn_dat);
MA_set	*create_next_MA_set(MA_set **root, MA_set *prev_set);
void	invert_set_path(MA_set *map);
void	dbg_print_coord2_clust(MA_set *mac_root, int nx, int nxy);
void	dbg_print_MA_sets(MA_set *mas_root, MA_set *map_root,
						  MA_set *mac_root, int nx, int nxy);
void	print_MA_sets(char *mas_type, char *att_mas_type, MA_set *root,
					  int nx, int nxy);
void	print_surf_rmnts(MA_set *mac_root, int nx, int nxy);
void	print_MAS(char *mas_type, char *att_mas_type, MA_set *mas,
				  int i, int nx, int nxy);
void	print_attached_MA_sets(char *att_mas_type, MA_set **att_mas,
							   int num_att);
void	print_loc_set(int *set, int num_in_set, int nx, int nxy);
void	print_loc_brn_set(int *set, unsigned char *bdat, int num_in_set,
						  int nx, int nxy);
void	rec_free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz);
void	free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz);
void	count_ma_vox_processed(MA_set *mas_root, MA_set *map_root,
							   MA_set *mac_root, unsigned char *num_nhbrs,
							   int *cube);
void	link_path_2_cluster(MA_set **map_root, MA_set *mac_root, int nx,
							int nxy, int *n_path);
void	link_3vox_path_2_cluster(MA_set **map_root, MA_set *map,
								 MA_set *mac, int nx, int nxy,
								 int *cur_att_n, int *num_3vox_2_c);
void	set_MAS_ind(MA_set *mas_root, int ind_sgn);
void	set_path_len(MA_set *map_root, int nx, int nxy);
void	set_map_len(MA_set *map, int nx, int nxy);
void	set_surf_rmnts(MA_set *mac_root, int nx, int nxy);
int	is_surf_rmnt(int *cloc, int cnum, int *offset, int nx, int nxy);

/*dendrite.c*/
void	set_dendritic_path(MA_set **den_root, int * n_den, int nx, int ny,int nz,
			unsigned char *burn_cube, int *pnloc, int **pma_loc, unsigned char **pma_bno,			
			float dp_tol,int num_den, int trim, float um_x,float um_y,
			float um_z,CMrg_menu cmrg, float loop_tol);
mxArray * mexdendritemain(const mxArray *input);
void	remove_dendrite(	MA_set	** den_root,int ind);
void	select_bb_path(MA_set *map_root,MA_set *den_root);
int	check_if_same_den(MA_set *map1,MA_set *map2,MA_set *den_root);
void	eliminate_dendrite_loop(MA_set **den_root,int *n_den,int Nx,int Nxy,float loop_tol);
void	remove_MA_set(MA_set **den_root,MA_set *den);

