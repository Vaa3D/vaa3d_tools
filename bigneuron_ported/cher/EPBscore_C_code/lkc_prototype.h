/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

/* Function prototype declarations for functions defined in lkc directory */

#ifndef _min_surf_h
#include "min_surf.h"
#endif

#ifndef _data_h
#include "data.h"
#endif

#include <mex.h>
//extern	"C"
//{
	/* from check_surf.cpp */
	int	check_surface_edge(point_info *point);
	int	endpoint_check(int decim);
	int	mod_check_surface_edge(point_info *ptinfo, data_info *datainfo,
							   unsigned char dir, int bdry_alg);
	int	check_interior_face(point_info *ptinfo);

	/* from connect.cpp */
	int	local_connectivity(point_info *point);
	int	local_6_connectivity(point_info *point);
	void	rec_6_connect(point_info *point, char ind, char label,
						  unsigned char *inc_lbl);
	void	dbg_print_node_values(point_info *point);
	void	dbg_print_oct_num_list(int *oct_num, int *oct_list);

	/* from data.cpp */
	data_info *data_init(int nx, int ny, int nz);
	void	free_data_info(data_info *datainfo);
	point_info *point_init();
	void	free_point_info(point_info *ptinfo);
	void	initialize_point_neighborhood(data_info *input, point_info *point);
	void	print_point_info(point_info *point);

	/* from dbg_io.c */
	/***
	void	Volvis_raw_output(data_info *datainfo, FILE *fp);
	void	ascii_output(data_info *datainfo, FILE *fp);
	void	ascii_with_bno_output(data_info *datainfo, char *infile_name,
								  int infile_len, int in_cmprss, int zs,
								  FILE *fp);
	void	ascii_burnback_opt(int *cube, int nx, int ny, int nxyz,
							   gr_bdry_vect_elem *gr_bdry_vect, int gbdry_size,
							   grain_chain *grain_loop,
							   grain_chain *short_loop, int connect_vox,
							   int *ma_loc, int ma_num, FILE *fp);
	void	ascii_network_opt(int *cube, int nx, int ny, int nxyz,
							  gr_bdry_vect_elem *gr_bdry_vect, int gbdry_size,
							  network_node *grain_nw, grain_chain *short_path,
							  int connect_vox, int *ma_loc, int ma_num,
							  FILE *fp);
	void	ascii_barrier_opt(int *cube, int nx, int ny, int nxyz, FILE *fp_loc,
							  FILE *fp_sngl, FILE *fp);
	void	ascii_bstep_output(int *cube, int nx, int nxy, int nxyz, FILE *fp);
	void	set_symbols(char symbols[86]);
	***/

	/* from euler.cpp */
	void	init_deuler_table();
	char	calc_deuler(unsigned char *nodeval, char conn_type);

	/* from input_driver.cpp */
	void	thin_input_obj_data(data_info **datainfo, char tp, char conn_type,
								int bdry_alg, int nx, int ny, int nz,
								unsigned char *burn_cube);
	void	get_nx_ny(int Ts, char *fname, char *ext, int flen, int is_cmprss,
					  int *nx, int *ny, int *zs, int *ze, int *n_polys,
					  char asc_bin);

	/* from lkc_main.cpp */
	mxArray * mexthinmain(const mxArray *input);
	void	print_mydata(FILE *fout, unsigned char *cube,
					 int nx, int ny, int nz);
	void	print_proj_slc(int nloc, int *ma_loc, int nx, int ny, int nz,
					   FILE *fdebug);
	void	thin_driver(
						int nx, int ny, int nz,
						unsigned char *burn_cube,
						int *nloc, UINT32 **ma_loc, unsigned char **ma_bnum);
	void	init_thin_choices(int *surface, int *alg, int *iso_or_dir,
							  int *end_cond, int *clean_ends, char *conn_type);
	int	set_thin_connectivity(data_info *datainfo, point_info *ptinfo,
							  char conn_type, int iso_or_dir);
	void	surface_thin(data_info *datainfo, point_info *ptinfo,
						 int *bord_array, int num_array, char conn_type,
						 int *num_vox_removed, int bdry_alg, unsigned char dir,
						 int iso_or_dir, int end_cond);
	void	axis_thin(data_info *datainfo, point_info *ptinfo, int *bord_array,
					  int num_array, char conn_type, int *num_vox_removed,
					  int num_keep_pts, unsigned char dir, int iso_or_dir,
					  int end_cond);
	void	keep_points_axis_thin(data_info *datainfo, point_info *ptinfo,
								  int *bord_array, int num_array,
								  char conn_type, int *num_vox_removed,
								  unsigned char dir, int iso_or_dir);
	void	new_end_point_axis_thin(data_info *datainfo, point_info *ptinfo,
									int *bord_array, int num_array,
									char conn_type,int *num_vox_removed,
									unsigned char dir, int iso_or_dir);
	void	lkc_end_pt_axis_thin(data_info *datainfo, point_info *ptinfo,
								 int *bord_array, int num_array, char conn_type,
								 int *num_vox_removed, unsigned char dir,
								 int iso_or_dir, int end_cond);
	void	set_thin_border_points(data_info *datainfo, point_info *ptinfo,
								   int *bord_array, int num_array,
								   char conn_type);
	int	thin_object_dir(data_info *datainfo, point_info *ptinfo, int surface,
						int bdry_alg, int num_keep_pts, int itno,
						char conn_type, int dbg_print);
	void	get_border_array_dir(int **bord, data_info *datainfo, int *num_array,
								 int dir, int bdry_alg);
	int	thin_object_iso(data_info *datainfo, point_info *ptinfo, int surface,
						int bdry_alg, int num_keep_pts, int itno,
						char conn_type, int dbg_print, int end_cond);
	void	get_border_array_iso(int **bord, data_info *datainfo,
								 int *num_array, int bdry_alg, int end_cond,
								 int surface, char conn_type);
	void	rehash_bord_array(int *bord, data_info *datainfo, int num_array,
							  int bdry_alg, int end_cond, int surface,
							  char conn_type);
	void	lkcm_debug_call_init(char *outdir, char *outfile,char *outfile1,
								 char *infile_name, int infile_len,
								 int in_cmprss, int zs, data_info *datainfo);
	void	lkcm_debug_call_fp1(char *outdir, char *outfile1,
								data_info *datainfo, FILE **fp1,
								char **infile_name, int *infile_len,
								int *in_cmprss, char *burn_file_name,
								int burn_file_len, int burn_cmprss, int zs,
								char *file_basename);
	void	dbg_dir_sweep(data_info *datainfo, char *dbgdir, int dir, int itno,
						  char *burn_file_name, int burn_file_len,
						  int burn_cmprss, int zs);
	void	thin_no_options(char tp, char in_infile_name[256],
							int in_infile_len, int in_is_cmprss, int zs, int ze,
							int surface, int bdry_alg, int iso_or_dir,
							int end_cond, int clean_ends, char conn_type,
							char *file_basename);


	/* from print_ma.cpp */

	void	pma_output(data_info *datainfo, int *nloc, UINT32 **ma_loc,
				   unsigned char **ma_bno, unsigned char *burn_cube);

	void	print_thin_slice(short *pma, int nx, int ny, int k, UINT32 *ma_loc,
						 unsigned char *ma_bno, int thincount,
						 int tot_ma_vox);

	/* from thin_cluster.cpp */
	void	thin_branch_cluster(int nx, int nxy, int nxyz, int num_keep_pts,
								int *keep_pts, unsigned char *data,
								char conn_type, int *min, int t_vox,
								int dbg_print, int iso_or_dir);
	void	impose_data_info(data_info *datainfo1, data_info *datainfo,
							 unsigned char *prev_data, int *min);
	void	impose_keep_pts(data_info *datainfo1, int num_keep_pts,
							int *keep_pts, int *min, int wx, int wxy);

	/* from thin_io.cpp */
	void	thin_read_binary_file(unsigned char *data,
								  int nx, int ny, int nz,
								  unsigned char *burn_cube);
	void	mark_edge_void(unsigned char *data, int *stencil, int zs, int ze,
						   int nx, int ny,
						   int nz, int bdry_alg, char conn_type);
//}