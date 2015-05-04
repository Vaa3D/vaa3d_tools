/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _isoseg_h
#include "isoseg.h"
#endif

#ifndef _data_h
#include "data.h"
#endif

/* from check_1_obj.cpp */
int	check_one_obj(int *obj, int obj_size, int nx, int nxy,
				  int (*proc_func)(int, int, int, int, int, int));
int	is_6_connected(int x1, int x2, int y1, int y2, int z1, int z2);
int	is_26_connected(int x1, int x2, int y1, int y2, int z1, int z2);
void	dbg_print_check_one_object(int *check_obj, int obj_size);

/* from check_loop.cpp */
int	check_loop(int *cube, int *ma_loc, grain_chain *loop, int *sten26,
			   int ma_start, int ma_end, int ma_s, int ma_e, int nx,
			   int nxy, int nxyz);

/* from indexx.cpp */
void	indexx_for(int n, network_node **arr, int *indx);
void	indexx_back(int n, network_node **arr, int *indx);
void	nrerror(char *error_text);
int	*ivector(int nl, int nh);
void	free_ivector(int *v, int nl, int nh);

/* from min_path.cpp */
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
								  int conn_type, int min_surf_dijk);
float	compute_throat_size(int *ma_loc, int *t_loc, int *loop_len, int nx,
							int nxy, int ma_start, int ma_end,
							grain_chain *loop,
							int use_all_vox, int use_odd_vox, int do_adj);
int	get_path_ends(gr_bdry_vect_elem *gr_bdry_vect, int grain_index, int nx,
				  int ny, int nz, int nxy, int *cube, int *sten26,
				  int *start_vox, int *end_vox, int num_ignore_vox,
				  int *ignore_vox);
int	oct_labeling(point_info *pt_info, int *start_vox, int *end_vox);
void	recur_label(point_info *pt_info, int octant, unsigned char label,
					unsigned char *inc_label);
int	setup_network(grain_chain *closed_loop, gr_bdry_vect_elem *gr_bdry_vect,
				  int grain_index, int nx, int ny, int nz, int nxy,
				  int *cube, int *sten26, int start_vox, int end_vox,
				  network_node **snw_node, network_node **enw_node,
				  char conn_type, int pass, MoI *moi, int *num_ignore_vox,
				  int **ignore_vox);
void	initialize_nw_node(network_node *p_nw_node,
						   network_node *prev_nw_node,
						   gr_bdry_vect_elem *gr_bdry_vect, int index,
						   int grain_index, int nx, int ny, int nz, int nxy,
						   int *cube, int *sten26, char conn_type,
						   int  num_ignore_vox, int *ignore_vox);
int	check_disconnectivity(associated_grain_chain *pagc, int *cube,
						  int ignore_index, int index, int *sten26,
						  unsigned char *is_edge, int is_in[26],
						  int num_ignore_vox, int *ignore_vox,
						  gr_bdry_vect_elem *gr_bdry_vect, int nx, int nxy);
grain_chain *get_shortest_path(gr_bdry_vect_elem *gr_bdry_vect,
							   int gbdry_size, network_node *snw_node,
							   network_node *enw_node, int *cube,
							   int grain_index, MoI *moi, int nx, int nxy,
							   int nxyz, int *ma_loc, int ma_num,
							   int ma_start, int ma_end, FILE *fp,
							   float *return_area, int *t_loc,
							   int *t_loop_len, int *sten26, int ma_s,
							   int ma_e, int conn_type, int min_surf_dijk);
void	rec_mk_ret_chain(grain_chain *ret_grain_chain,
						 grain_chain **last_link,
						 network_node *prev_nw_node, int dir);
int	get_6_path(int *cube, int *sten26, int nx, int ny, int nz,
			   int *loop_len, grain_chain *closed_loop, int *has_pore,
			   int gbvc);
void	check_and_add_edge_vox(int *cube, int *len_inc, int nbr1,
							   grain_chain *closed_loop,
							   grain_chain **loop_ptr, int cntr,
							   int *has_pore, int gbvc);
void	check_and_add_vertex_vox(int *cube, int *len_inc, int nbr1,
								 int nbr2, grain_chain *closed_loop,
								 grain_chain **loop_ptr, int cntr,
								 int *has_pore, int gbvc);
void	check_and_add_3_near_vox(int *cube, int *len_inc, int nbr1,
								 int nbr2, int nbr3,
								 grain_chain *closed_loop,
								 grain_chain **loop_ptr, int cntr,
								 int *has_pore, int gbvc);
void	free_network(network_node *nw_node);
void	free_nw_adj_list(network_adj_list *nw_adj_list);
void	free_g_chain(grain_chain *g_chain);
void	free_g_change_list(g_change_list *g_chain);
void	dbg_print_network(network_node *nw_node, int *kount, int dir);
grain_chain *Bi_Dir_for_Label(network_node **start_node,
							  tmp_indx_chain **ptmp_indx_chain,
							  gr_bdry_vect_elem *gr_bdry_vect, MoI *moi,
							  int *cube, int nx, int nxy, int grain_index,
							  int *ok, FILE *fp1, int min_surf_dijk,
							  float v_term[3], float const_d);
network_node *del_join_for_link(network_node *p_nw_node,
								network_node *p_e_node);
grain_chain *Bi_Dir_bac_Label(network_node **start_node,
							  tmp_indx_chain **ptmp_indx_chain,
							  gr_bdry_vect_elem *gr_bdry_vect, MoI *moi,
							  int *cube, int nx, int nxy, int grain_index,
							  int *ok, FILE *fp2, int min_surf_dijk,
							  float v_term[3], float const_d);
network_node *del_join_bac_link(network_node *p_nw_node,
								network_node *p_e_node);
grain_chain *return_shortest_path(network_node *min_nw_node,
								  int join_grain_index, int grain_index);
void	recur_reverse_network_dir(network_node *start_node);
void	print_min_node_data(network_node *min_node, int dir, int nx,
							int nxy, FILE *fp1);
void	free_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain, int dir);
void	dbg_print_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain);
void	dbg_reverse_print_tmp_indx_chain(tmp_indx_chain *ttmp_indx_chain);
void	dbg_recur_reverse_print(tmp_indx_chain *ttmp_indx_chain, int i);
void	dbg_nw_print_f_b(network_node *nw_node, int print_coords, int nx,
						 int nxy, char fc, char bc);
void	dbg_nw_print_network(network_node *nw_node, int print_coords,
							 int nx, int nxy);

/* from throat_surf.cpp */
int	generate_surface(int *cube, int *sten26, int throat_vox, int nx, int ny,
					 int nz, int loop_len, grain_chain *g_loop,
					 int path_seg_ind, int throat_ind, char *fname,
					 int conn_type, int gbvc, int print_it, float area);
void	dbg_print_barrier(unsigned char *wkspc, int *min, int wz, int wx,
						  int wxy, int nz, int nx, int nxy);

/* from tri_area.cpp */
float	get_triangulated_area(grain_chain *loop, int x0, int y0, int z0,
							  int nx, int nxy, int use_all_vox,
							  int use_odd_vox, int do_adj);
float	use_every_vox_area(grain_chain *loop, int nx, int nxy, int x0,
						   int y0, int z0, int do_adj);
float	use_every_other_vox_area(grain_chain *loop, int nx, int nxy, int x0,
						   int y0, int z0, int odd_or_even, int do_adj);
float	adj_triangle_area(int x0, int y0, int z0, int ind, int nx, int nxy,
						  float *x1, float *y1, float *z1);
void	get_adjusted_vertex(int x1, int y1, int z1, float *f_x1, 
							float *f_y1, float *f_z1);
float	triangle_area(int x0, int y0, int z0, int ind, int nx, int nxy,
					  int *x1, int *y1, int *z1);
float	triangle_area1(float x0, float y0, float z0, float x1, float y1,
					   float z1, float x2, float y2, float z2);

/* from winding_num.cpp */
int	calc_winding_number(grain_chain *loop, MoI *moi, int nx, int nxy,
						char two_d);
void	cross_product(float prev_v[3], float new_v[3]);
// float	get_d_theta(int ind0, int ind1, MoI *moi, int nx, int nxy);
float	calc_d_theta(float prev_v[3], float pos_v[3], float *normal,
					 char two_d);
void	find_norms(float prev_v[3], float pos_v[3], float *p_norm,
				   float *cu_norm);
float	get_norm(float v[3]);
void	project_2_plane(float new_v[3], float *basis_1, float *basis_2);
float	dot_product(float vect_1[3], float *vect_2);
