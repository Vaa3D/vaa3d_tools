/* matort.c */
void set_quant_av(int *ph, int nbin, float *av);
void hist_ma_quant_2_std(char *ma_type, char *quant_type, int *all_ph, int *bb_ph, int *bl_ph, int *ll_ph, float av_all, float av_bb, float av_bl, float av_ll, float *quant_bin_val, int nbin);
