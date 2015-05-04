/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _isoseg_h
#include "isoseg.h"
#endif

#ifndef _my_ma_h
#include "my_ma.h"
#endif

#include "stdAfx.h"

//extern	"C"
//{

 
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

	/* from brn_io.cpp */
	void	input_data(Fdat *tmp, char inv,
					   int nx, int ny, int nz,
					   int task, unsigned char *seg_cube);
	void	get_burn_data(Fdat *tmp, char *infile_name, int infile_len,
						  int is_cmprss, int nx, int ny, int nz, int zs,
						  int n_polys, int **bh, int *bh_count,
						  unsigned char *tmp_burn);
	void	invert(unsigned char *work, int size);
	void	out_burn_data(Fdat *tmp, int n_polys, int **bh, int *bh_count,
						  int nx, int ny, int nz, char *out_dir,
						  char *outfile_base, int outfile_inct, char fp_mthd,
						  int tfirst, int tlast, int out_cmprss, int period);
	void	print_slice_char(unsigned char *ppfdat, int nx, int ny, int nz);
	void	write_slice_char(unsigned char *ppfdat, int nx, int ny, int nz);

	/* from brnstep.cpp */
	void	surf_burn_step(Bchains *bc, unsigned char *bfdat, int pos, int nx,
						   int nxy, int realloc_size);

	/* from fiducial.cpp */
	void	init_fiducial_polygon(char *fp_mthd, char *afp_type, int nx, int ny,
								  int **npts, int *npt, int **pix, int **piy,
								  int *n_polys,char *file_basename);
	void	manual_bh_info(char *fpm);
	void	init_and_set_manual_bh(int nx, int ny, char *fpm, int *pn_polys,
								   int *pdim_bh, int ***pbh,int **pbh_count,
								   char *file_basename);
	void	init_manual_fiducial_polygon(int nx, int ny, int **pnpts, int *npt,
										 int **pix, int **piy, int *pn_polys,
										 char *file_basename);
	void	set_fid_poly_2_slc(int nx, int ny, int **pnpts, int *pnpt,
							   int **ppix, int **ppiy, int *pn_polys);
	void	init_bh_storage(int n_polys, int ny, int *pdim_bh, int ***pbh,
							int **pbh_count);
	void	free_bh_storage(int ny, int dim_bh, int **bh, int *bh_count);
	void	set_auto_bh(unsigned char *data, int n_polys, int **bh, int *c_bh,
						int *bh_count, int k, CHAR *c_val, char afp_type,
						int task, char *bh_dir, int nx, int ny);
	void	correct_auto_bh(int **bh, int *c_bh, CHAR *c_val, int nx, int ny);
	void	print_bh(char *where, int ny, int n_polys, int **bh);
	void	write_auto_bh_file(int **bh, int k, char *bh_dir, int nx, int ny);
	void	read_auto_bh_file(int **bh, int *bh_count, int k, char *bh_dir,
							  int nx, int ny);
	void	set_manual_bh(int ny, int *npts, int npt, int *pix, int *piy,
						  int **bh, int *bh_count, int n_polys);
	void	C_set_exterior_from_bh(char fp_mthd, int nx, int ny, int nz,
								   int dim_bh, int **bh, int *bh_count,
								   unsigned char *fdat, char *bh_dir,
								   unsigned char ext_val);
	void	I_set_exterior_from_bh(char fp_mthd, int nx, int ny, int nz,
								   int dim_bh, int **bh, int *bh_count,
								   int *fdat, char *bh_dir, int ext_val);
	void	F_set_exterior_from_bh(char fp_mthd, int nx, int ny, int nz,
								   int dim_bh, int **bh, int *bh_count,
								   int *fdat, char *bh_dir, int ext_val);
	void	sort_bh(int ny, int n_polys, int **bh, int *bh_count);
	void	set_sort_row_bh(int row, int n_polys, int *pbh_count, int *tot_vox,
							int *bh_vect, int **bh);
	void	count_fid_voxels(char fp_mthd, int nx, int ny, int nz, int **bh,
							 int *bh_count, char *bh_dir, int *num_fv);
	void	count_fid_voxels_from_slc_bh(int ny, int **bh, int *bh_count,
										 int *num_fv);
	int	test_fiducial_polygon_files(char *file_basename, int *Fs, int *Fe);

	/* from isoseg.cpp */
	void	initialize_convert_isolated_cluster(Iso_dat *isodat);
	void	set_max_iso_clust_size(char *ctype, int *fil, int *threshold,
								   int *keep_largest);
	void	set_min_iso_clust_size(char *ctype, int *fil, int *threshold);
	void	convert_isolated_clusters(unsigned char *cube, int nx, int ny,
									  int nz, Iso_dat *isodat, char inv);
	void	set_largest_iso_clust_size(unsigned char *cube, Bchains *bc, int nx,
									   int ny, int nz, unsigned char this_phase,
									   int connectivity, int *threshold,
									   int realloc_size, char *ctype);
	void	convert_phase(unsigned char *cube, Bchains *bc, int nx, int ny,
						  int nz, unsigned char this_phase,
						  unsigned char other_phase, int connectivity,
						  int convert_min, int convert_max, int realloc_size,
						  char *ctype);
	void	process_disc_vols(unsigned char *cube, int nx, int ny, int nz,
							  void (*proc_func)(unsigned char *, Bchains *, int, int, int, unsigned char, int, int, char *, char *, char *),
							  char *proc_struct, char *ignore_exvol);
	void	count_voxels(char *msg, unsigned char *dat, int sz);
	void	uc_echo_voxels_of_type(unsigned char type, unsigned char *dat,
								   int sz);
	void	us_echo_voxels_of_type(unsigned short type, unsigned short *dat,
								   int sz);
	void	count_disc_phase_vol(unsigned char *cube, Bchains *bc, int nx,
								 int ny, int nz, unsigned char this_phase,
								 int connectivity, int realloc_size,
								 char *ctype, char *dv_struct,
								 char *ignore_exvol);
	void	print_Disc_vol(Disc_vol *dv, char inv);
	void	print_phase_dv(char *ctype, int num, int *x, int *y, int vol);
	void	free_Disc_vol(Disc_vol *dv);
	int	burn_phase(Bchains *bc, unsigned char *cube, int nx, int ny, int nz,
				   int nxy, int sten_sz, int realloc_size,
				   unsigned char *bd_types);
	void	set_6_stencil(int nx, int nxy, int *sten);
	void	check_bdry_26_nbr(unsigned char *pbd, int pos, int nx, int ny,
							  int nz, int nxy, int *is_in);
	void	check_bdry_6_nbr(unsigned char *pbd, int pos, int nx, int ny,
							 int nz, int nxy, int *is_in);
	void	burn_phase_step(Bchains *bc, unsigned char *cube, int pos1,
							int realloc_size, unsigned char edge, int sten_sz,
							int *sten, int *is_in, unsigned char *bd_types);

	/* from l2_dist.cpp */
	void	l2_distance_driver(char *file_basename);
	void	l2_erode(unsigned char *fdat, int nx, int nxy, int nxyz, int zs,
					 int ze, float **pdist, float *pmin, float *pmax);
	void	l2_x_sweep(unsigned char *fdat, unsigned short *dx, int nx, int nxy,
					   int nxyz);
	void	l2_y_sweep(unsigned char *fdat, unsigned short *dy, int nx, int nxy,
					   int nxyz);
	void	l2_z_sweep(unsigned char *fdat, unsigned short *dz, int nx, int nxy,
					   int nxyz);
	void	init_dd(unsigned char *fdat, unsigned short *dd, int nx, int nxy,
					int nxyz);
	void	seg_input_driver(Fdat *tmp, int *pnx, int *pnxy, int *pnxyz,
							 int *pzs, int *pze);
	void	l2_set_dist_from_short(float *dist, unsigned char *fdat,
								   unsigned short *dx, int nx, int nxy,
								   int nxyz, float *pmin, float *pmax);
	unsigned short *set_zone_widths(unsigned char *fdat, unsigned short *dx,
									unsigned short *dy, unsigned short *dz,
									int nxyz);
	void	l2_set_dist(float *dist, unsigned char *fdat, unsigned short *dx,
						unsigned short *zone, int nx, int nxy, int nxyz,
						float *pmin, float *pmax);
	void	sf_l2_set_dist(float *dist, unsigned char *fdat, unsigned short *dx,
						   unsigned short *zone, int nx, int nxy, int nxyz,
						   float *pmin, float *pmax);

	/* from poly.cpp */
	void	generate_polygon(char *file_basename);
	void	generate_polygon_file(char *file_basename);
	void	run_generate_polygon(int xc, int yc, int radius, char pg_out[256]);

	/* from unburn_ma.cpp */
	void	unburn_ma_driver(char *file_basename);
	void	input_burn_mtd(int *mtd);
	void	unburn_mtd(int mtd, int bmin, int bmax, int n_ma_vox,
					   unsigned char *burn_dat, int *ma_list,
					   unsigned char **cube, int nx, int ny, int nz,	int nxy,
					   int nxyz);
	int	unburn_ma(unsigned char *cube, Bchains *bc, int nx, int ny, int nz,
				  int nxy, int nxyz, int the_rest, int connectivity,
				  int ma_voxel,	int threshold, int realloc_size);
	int	burn_next_layer(Bchains *bc, unsigned char *cube, int nx, int ny,
						int nz, int nxy, int sten_sz, int realloc_size,
						void (*burn_step)(Bchains *, unsigned char *, int, int, unsigned char, int, int *, int *, int, int, int),
						void (*set_sten)(int, int, int *),
						void (*check_bdry)(unsigned char *, int, int, int, int, int, int *),
						int low, int high, int not);
	void	burn_step(Bchains *bc, unsigned char *cube, int pos1,
					  int realloc_size, unsigned char is_edge, int sten_sz,
					  int *sten, int *is_in, int low, int high, int not);
	void	burn_updown_step_in_one(Bchains *bc, unsigned char *cube, int pos1,
									int realloc_size, unsigned char is_edge,
									int sten_sz, int *sten, int *is_in);
	void    ma_min_max_bnum(unsigned char *burn_dat, int n_ma_vox, int *bmin,
							int *bmax);
	void	unburn_output2seg(unsigned char *cube, int nx, int ny, int nz,
							  int zs, int ze, char *segf_base, char *segf_name,
							  int segf_len, int seg_cmprss);
	int	burn_next_sp_layer(Bchains *bc, int realloc_size, unsigned char *cube,
						   int nx, int ny, int nz, int nxy, int radius,
						   void (*burn_step)(Bchains *, int, unsigned char *, int *, int *, int, int *, int, int, int *, int),
						   int *nring, int **rings, int **ring_szs,
						   int *low_ind, int dbg, int Size);
	void	set_sp_stencil(int nx, int ny, int nz, int *nring, int **rings,
						   int **ring_szs, int Size);
	void	set_sp_pos(int pos, int nx, int ny, int nz, int nxy, int radius,
					   int tot, int *rings, int *sp_pos, int low_ind, int Size,
					   int dbg);
	void	check_conn_sp_shell(int *sp_pos, int tot, int nx, int ny, int nz,
								int nxy, int *sten, int *sp_conn,
								unsigned char *cube, int *is_conn, int low_ind,
								int dbg);
	void	burn_sp_in_one(Bchains *bc, int realloc_size, unsigned char *cube,
						   int *sp_pos, int *sp_conn, int tot, int *n_vox,
						   int nz, int nxy, int *low_ind, int l);
	void	burn_sp_in_obj(Bchains *bc, int realloc_size, unsigned char *cube,
						   int *sp_pos, int *sp_conn, int tot, int *n_vox,
						   int nz, int nxy, int *low_ind, int l);
//}
