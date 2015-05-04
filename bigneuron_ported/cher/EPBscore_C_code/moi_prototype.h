/* moi.c */
MoI *create_next_MoI(MoI **root, MoI *prev_moi);
void print_MoI_list(MoI *moi_root, char *mtype);
void print_MoI(MoI *moi, int i);
void rec_free_MoI_list(MoI *moi, int Msz);
void compute_prin_ax(MoI *moi_root);
void hist_moi(MoI *moi_root, int *nbin, float **bin_val, float **den, float **cum);
void set_nbin(int nval, int *nbin);
