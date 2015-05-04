/* matree.c */
MA_tree *create_next_MA_tree(MA_tree **root, MA_tree *prev_tree);
void dbg_print_MA_trees(MA_tree *mat_root, MA_tree *iso_mat);
void print_MA_trees(MA_tree *mat_root);
void summarize_MA_info(MA_tree *mat_root, MA_tree *iso_mat, MA_set *mas_root, MA_set *mac_root);
void print_mat(MA_tree *mat);
void rec_free_MA_tree(MA_tree *mat, int pMsz, int MTsz);
void MA_tree_driver(MA_set *mac_root, MA_set *map_root, MA_tree **mat_root, MA_tree **iso_mat);
MA_tree *set_ma_tree(MA_set *mac, MA_tree **mat_root, MA_tree *prev_mat);
