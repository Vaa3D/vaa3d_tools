/* thin_cluster.c */
void thin_branch_cluster(int nx, int nxy, int nxyz, int num_keep_pts, int *keep_pts, unsigned char *data, char conn_type, int *min, int t_vox, int dbg_print, int iso_or_dir);
void impose_data_info(data_info *datainfo1, data_info *datainfo, unsigned char *prev_data, int *min);
void impose_keep_pts(data_info *datainfo1, int num_keep_pts, int *keep_pts, int *min, int wx, int wxy);
