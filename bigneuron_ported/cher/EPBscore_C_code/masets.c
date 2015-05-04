/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <string.h>
#include <math.h>

#include "ma_prototype.h"
#include "maset_prototype.h"
//#include "statistics_prototype.h"
#define _COMPILING_3dmacommon


int	max_bnum_in_maset(MA_set *mas)
{
	int	i;
	int	bnm, tmp;

	bnm = mas->bdat[0];

	for( i = 1;  i < mas->num_in_set;  i++ )
	{
		tmp = mas->bdat[i];
		if( tmp > bnm ) bnm = tmp;
	}
	return bnm;
}

#ifdef TST
void	input_MA_sets(MA_set **pmas_root, MA_set **pmap_root,
					  MA_set **pmac_root, int *pnx, int *pnxy, int *pn_sngl,
					  int *pn_path, int *pn_clust)
{
	DBG_menu	dbg;
	CMrg_menu       cmrg;

	char	ftype[5];
	char	*sngl_fn, *cploc_fn, *cp_fn;

	int	cp_cmprss;

	specify_ma_format("input","110",ftype);

	switch( ftype[0] )
	{
	    case 'v':
		read_and_set_clust_paths(pmas_root,pmap_root,pmac_root,pnx,pnxy,
					pn_sngl,pn_path,pn_clust,file_basename);
		break;
	    case 'c':
	    default:
		set_cpf_fn(&sngl_fn,&cploc_fn,&cp_fn,"111",&cp_cmprss,
							file_basename);
		in_singles(sngl_fn,cp_cmprss,pmas_root,pn_sngl,pnx,pnxy);
		in_cp(cploc_fn,cp_fn,cp_cmprss,pmac_root,pmap_root,pn_clust,
							pn_path,pnx,pnxy);
		summarize_cluster_info(*pmac_root);
		set_CMrg_menu(&cmrg);

		if( cmrg.do_cmrg ) merge_clusters(pmac_root,pmap_root,&cmrg,
						  *pnx,*pnxy,pn_clust,pn_path);
		set_MAS_ind(*pmap_root, 1);
		set_MAS_ind(*pmac_root,-1);
		set_surf_rmnts(*pmac_root,*pnx,*pnxy);

		free_cpf_fn(sngl_fn,cploc_fn,cp_fn,"111");
		break;
	}

	set_DBG_menu(&dbg);
	if( dbg.do_MAS ) dbg_print_MA_sets(*pmas_root,*pmap_root,*pmac_root,
								*pnx,*pnxy);
	if( dbg.do_SR  ) print_surf_rmnts(*pmac_root,*pnx,*pnxy);
}
#endif


#ifdef TST
void	read_and_set_clust_paths(MA_set **pmas_root, MA_set **pmap_root,
								 MA_set **pmac_root, int *pnx, int *pnxy,
								 int *pn_sngl, int *pn_path, int *pn_clust,
								 char *file_basename)
{
	CMrg_menu	cmrg;

	unsigned char	*burn_dat;

	int	*cube;
	int	*ma_list;

	char	*mafn_base, *mafn_name;
	char	*brnfn_base, *brnfn_name;
	char	ext[10];
	int	mafn_len, brnfn_len, in_cmprss, brn_cmprss;
	int	xs, xe, ys, ye, Ts, Te, trim;
	int	nx, nxy, nxyz, zs, ze;
	int	n_ma_vox;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	input_pma_data_arrays_2(&xs,&xe,&ys,&ye,&zs,&ze,&Ts,&Te,&trim,
			&mafn_base,&mafn_name,ext,&brnfn_base,&brnfn_name,
			&mafn_len,&brnfn_len,&in_cmprss,&brn_cmprss,
			1,file_basename);

	set_pma_data_arrays(xs,xe,ys,ye,zs,ze,-1,trim,mafn_base,mafn_name,
			ext,brnfn_base,brnfn_name,mafn_len,brnfn_len,
			in_cmprss,brn_cmprss,&nx,&nxy,&nxyz,
			&n_ma_vox,&ma_list,&burn_dat,&cube);

	/***
	input_pma_data_arrays(&nx,&nxy,&nxyz,&zs,&ze,&n_ma_vox,&ma_list,
							&burn_dat,&cube);
	***/
	*pnx = nx;		*pnxy = nxy;

	set_CMrg_menu(&cmrg);
	set_clust_paths(nx,nxy,nxyz,n_ma_vox,ma_list,burn_dat,cube,&cmrg,
			pmas_root,pmap_root,pmac_root,pn_sngl,pn_path,pn_clust);

	FREE(ma_list, n_ma_vox*isz);
	FREE(burn_dat,n_ma_vox*usz);
	FREE(cube,nxyz*isz);

	free_filenames( mafn_base, mafn_name);
	free_filenames(brnfn_base,brnfn_name);
}
#endif


void	set_clust_paths(int nx, int nxy, int nxyz, int n_ma_vox,
						int *ma_list, unsigned char *burn_dat, int *cube,
						CMrg_menu *cmrg, MA_set **pmas_root,
						MA_set **pmap_root, MA_set **pmac_root,
						int *pn_sngl, int *pn_path, int *pn_clust)
{
	MA_set		*mas_root = NULL;
	MA_set		*map_root = NULL;
	MA_set		*mac_root = NULL;

	unsigned char	*num_nhbr;

	int	**nhbr_list;

	int	offset[27];

	int	n_sngl, n_path, n_clust;

	set_nhbr_offsets(nx,nxy,offset);
	set_nhbr_list(&num_nhbr,&nhbr_list,ma_list,n_ma_vox,cube,offset);

	// pma_nhbr_hist(num_nhbr,n_ma_vox,"ma_nhbr_hist"); 

	set_MA_sets(ma_list,nhbr_list,num_nhbr,burn_dat,cube,offset,n_ma_vox,
					nx,nxy,&mas_root,&map_root,&mac_root,
					&n_sngl,&n_path,&n_clust);

	count_ma_vox_processed(mas_root,map_root,mac_root,num_nhbr,cube);

	unlink_cube_from_ma_list_entries(ma_list,n_ma_vox,cube);

	free_nhbr_list(num_nhbr,nhbr_list,n_ma_vox);

	link_path_2_cluster(&map_root,mac_root,nx,nxy,&n_path);

	if( cmrg->do_cmrg ) 
		merge_clusters(&mac_root,&map_root, cmrg,
						nx,nxy,&n_clust,&n_path);
	set_MAS_ind(map_root, 1);
	set_MAS_ind(mac_root,-1);
	set_surf_rmnts(mac_root,nx,nxy);

	*pmas_root = mas_root;		*pn_sngl  = n_sngl;
	*pmap_root = map_root;		*pn_path  = n_path;
	*pmac_root = mac_root;		*pn_clust = n_clust;
}


#ifdef TST
void	pma_sets_driver(char *file_basename)
{
	MA_set		*mas_root = NULL;
	MA_set		*map_root = NULL;
	MA_set		*mac_root = NULL;
	MA_tree		*mat_root = NULL;
	MA_tree		*iso_mat  = NULL;

	MA_hist_menu	menu;

	int	nx, nxy;
	int	n_sngl, n_path, n_clust;

	int 	isz = sizeof(int);
	int    pMsz = sizeof(MA_set *);
	int 	Msz = sizeof(MA_set);
	int 	usz = sizeof(unsigned char);
	int    MTsz = sizeof(MA_tree);

	if( Interface != GUI_NONE )
	{
		PRINTFSTDERR("This function is not written with ");
		PRINTFSTDERR("GUI extension\n");
		clean_up(0);
	}

	input_MA_sets(&mas_root,&map_root,&mac_root,&nx,&nxy,
					&n_sngl,&n_path,&n_clust,file_basename);
	set_MA_hist_menu(&menu);
	PRINTFSTDERR("Enter data identifier for all histograms: ");
	gets(menu.title);
	fPRINTF(stdout,"Enter data identifier for all histograms: %s\n",
								menu.title);
	set_voxel_phys_len(&(menu.vox_len));

	if( menu.do_cp_scatt )
	{
		char	domat[5];

		PRINTFSTDERR("\nPrintout of MA tree structure available. ");
		PRINTFSTDERR("(Warning - printout can be huge!)\n");
		PRINTFSTDERR("Print MA trees? (y,n(dflt)): ");
		gets(domat);	PRINTFSTDERR("\n");

		PRINTFSTDOUT("\nPrintout of MA tree structure available. ");
		PRINTFSTDOUT("(Warning - printout can be huge!)\n");
		PRINTFSTDOUT("Print MA trees? (y,n(dflt)): %s\n\n",domat);

		MA_tree_driver(mac_root,map_root,&mat_root,&iso_mat);

		if( domat[0] == 'y' ) dbg_print_MA_trees(mat_root,iso_mat);
	}
	hist_ma_info(&menu,nx,nxy,mac_root,map_root,mas_root,mat_root,iso_mat);

	rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);
	rec_free_MA_tree(mat_root,pMsz,MTsz);
	rec_free_MA_tree(iso_mat ,pMsz,MTsz);
}
#endif


#ifdef TST
void	hist_ma_info(MA_hist_menu *menu, int nx, int nxy, MA_set *mac_root,
					 MA_set *map_root, MA_set *mas_root, MA_tree *mat_root,
					 MA_tree *iso_mat)
{
	int	nbc3;

	if( menu->do_path_len  ) hist_ma_len("path",map_root,nx,nxy,menu->vox_len,
								menu->title,1,1);
	if( menu->do_path_tort ) hist_ma_tort_driver("path",map_root,nx,nxy,
								menu->title,1,1);
	if( menu->do_clust_rad ) hist_clust_radius(mac_root,menu->vox_len,
								menu->title,1,1);
	if( menu->do_clust_sz  ) hist_clust_size(mac_root,nx,nxy,menu->title,1,1);
	if( menu->do_coord_num ) hist_coord_nums(mac_root,menu->title,1,1);
	if( menu->do_coord3    ) hist_coord3_len(map_root,mac_root,menu->title);
	if( menu->do_c3_stats  ) coord3_stats(map_root,mac_root,&nbc3);
	if( menu->do_path_brn  ) hist_path_brn(map_root);
	if( menu->do_iv_bnum   ) hist_iso_vox_bnum(mas_root,menu->title,1,1);
	if( menu->do_cp_scatt  )
	{
		tree_clust_path_scatterplot(mat_root,iso_mat,menu->title);
		summarize_MA_info(mat_root,iso_mat,mas_root,mac_root);
	}
	if( menu->do_moi       ) hist_ma_moi_driver("MA",map_root,nx,nxy,
								menu->title);
	if( menu->do_throat_char ) hist_throat_chars(map_root);
}
#endif


void	set_MA_hist_menu(MA_hist_menu *menu)
{
	char	s[80];
	int	*pm;
	int	slen, i, j;
	int	msz;

	pm = (int *)menu;	/* SIC  MA_hist_menu  must be aligned */
				/*	on integer boundary */

	msz = (sizeof(MA_hist_menu) - 84)/4;
	for( j = 0;  j < msz;  j++ ) pm[j] = 0;

	PRINTFSTDERR("Available analyses on medial axis\n");
	PRINTFSTDERR("\t0) histogram of path lengths\n");
	PRINTFSTDERR("\t1) histogram of path tortuosities\n");
	PRINTFSTDERR("\t2) histogram of branch cluster sizes\n");
	PRINTFSTDERR("\t3) histogram of coordination numbers\n");
	PRINTFSTDERR("\t4) histogram of paths attached to coord. num. 3 ");
	PRINTFSTDERR("clusters\n");
	PRINTFSTDERR("\t5) statistics on coord. num. 3 clusters\n");
	PRINTFSTDERR("\t6) histogram of erosion numbers on paths\n");
	PRINTFSTDERR("\t7) scatterplot of cluster and path counts for ");
	PRINTFSTDERR("connected MA components\n");
	PRINTFSTDERR("\t8) histogram of isolated voxel burn numbers\n");
	PRINTFSTDERR("\t9) histogram of branch cluster radii\n");
	PRINTFSTDERR("\tM) histogram path moments of inertia/principle axes\n");
	PRINTFSTDERR("\tT) histogram path throat characteristics\n");
	PRINTFSTDERR("Enter a list of options or the letter 'a(ll)': ");

	gets(s);

	PRINTFSTDOUT("Available analyses on medial axis\n");
	PRINTFSTDOUT("\t0) histogram of path lengths\n");
	PRINTFSTDOUT("\t1) histogram of path tortuosities\n");
	PRINTFSTDOUT("\t2) histogram of branch cluster sizes\n");
	PRINTFSTDOUT("\t3) histogram of coordination numbers\n");
	PRINTFSTDOUT("\t4) histogram of paths attached to coord. num. 3 ");
	PRINTFSTDOUT("clusters\n");
	PRINTFSTDOUT("\t5) statistics on coord. num. 3 clusters\n");
	PRINTFSTDOUT("\t6) histogram of erosion numbers on paths\n");
	PRINTFSTDOUT("\t7) scatterplot of cluster and path counts for ");
	PRINTFSTDOUT("connected MA components\n");
	PRINTFSTDOUT("\t8) histogram of isolated voxel burn numbers\n");
	PRINTFSTDOUT("\t9) histogram of branch cluster radii\n");
	PRINTFSTDOUT("\tM) histogram path moments of inertia/principle axes\n");
	PRINTFSTDOUT("\tT) histogram path throat characteristics\n");
	PRINTFSTDOUT("Enter a list of options or the letter 'a(ll)': ");
	PRINTFSTDOUT("%s\n",s);

	PRINTFSTDERR("\n");
	PRINTFSTDOUT("\n");

	slen = strlen(s);

	for( i = 0;  i < slen;  i++ )
	{
	    switch( s[i] )
	    {
		case 'a':   for( j = 0;  j < msz;  j++ ) pm[j] = 1; break;

		case '0':	pm[ 0]= 1;	break;
		case '1':	pm[ 1]= 1;	break;
		case '2':	pm[ 2]= 1;	break;
		case '3':	pm[ 3]= 1;	break;
		case '4':	pm[ 4]= 1;	break;
		case '5':	pm[ 5]= 1;	break;
		case '6':	pm[ 6]= 1;	break;
		case '7':	pm[ 7]= 1;	break;
		case '8':	pm[ 8]= 1;	break;
		case '9':	pm[ 9]= 1;	break;
		case 'M':	pm[10]= 1;	break;
		case 'T':	pm[11]= 1;	break;
	    }
	}
}


void	set_DBG_menu(DBG_menu *dbg)
{
	char	s[80];
	int	*pm;
	int	slen, i, j;
	int	msz = sizeof(DBG_menu)/4;

	pm = (int *)dbg;	/* SIC  dbg is a struct of ints */

	for( j = 0;  j < msz;  j++ ) pm[j] = 0;

	PRINTFSTDERR("Available debug printout on medial axis ");
	PRINTFSTDERR("(Warning - it can be huge)\n");
	PRINTFSTDERR("\t1) paths and clusters\n");
	PRINTFSTDERR("\t2) surface remnants only\n");
	PRINTFSTDERR("Enter a list of option numbers or the letters ");
	PRINTFSTDERR("'a(ll)' or 'n(one)': ");

	gets(s);

	PRINTFSTDOUT("Available debug printout on medial axis ");
	PRINTFSTDOUT("(Warning - it can be huge)\n");
	PRINTFSTDOUT("\t1) paths and clusters\n");
	PRINTFSTDOUT("\t2) surface remnants only\n");
	PRINTFSTDOUT("Enter a list of option numbers or the letters ");
	PRINTFSTDOUT("'a(ll)' or 'n(one)': %s\n",s);

	PRINTFSTDERR("\n");
	PRINTFSTDOUT("\n");

	slen = strlen(s);

	for( i = 0;  i < slen;  i++ )
	{
		switch( s[i] )
		{
		    case 'a':   for( j = 0;  j < msz;  j++ ) pm[j] = 1; break;
		    case '1':	pm[0]= 1;	break;
		    case '2':	pm[1]= 1;	break;

		    case 'n':
		    default :			break;
		}
	}
}


void	set_nhbr_offsets(int nx, int nxy, int *offset)
{
	offset[13] = -nxy-nx-1;	 offset[12] = -nxy-nx;	offset[14] = -nxy-nx+1;
	offset[10] = -nxy   -1;	 offset[ 9] = -nxy;	offset[11] = -nxy   +1;
	offset[16] = -nxy+nx-1;	 offset[15] = -nxy+nx;	offset[17] = -nxy+nx+1;

	offset[ 4] =     -nx-1;	 offset[ 3] =     -nx;	offset[ 5] =     -nx+1;
	offset[ 1] =        -1;	 offset[ 0] =       0;	offset[ 2] =         1;
	offset[ 7] =      nx-1;	 offset[ 6] =      nx;	offset[ 8] =      nx+1;

	offset[22] =  nxy-nx-1;	 offset[21] =  nxy-nx;	offset[23] =  nxy-nx+1;
	offset[19] =  nxy   -1;	 offset[18] =  nxy;	offset[20] =  nxy   +1;
	offset[25] =  nxy+nx-1;	 offset[24] =  nxy+nx;	offset[26] =  nxy+nx+1;
}


void	set_nhbr_list(unsigned char **num_nhbr, int ***nhbr_list,
					  int *ma_list, int n_ma_vox, int *cube, int *offset)
{
	char	msg[80];
	int	*pcube;
	int	n, i, j, snum;

	int	pisz = sizeof(int *);
	int	 isz = sizeof(int);
	int	 usz = sizeof(unsigned char);

	*num_nhbr = (unsigned char *)MALLOC(n_ma_vox*usz);
	if( MEMCHECK(*num_nhbr,"*num_nhbr in set_nhbr_list()",n_ma_vox*usz) )
		clean_up(0);

	*nhbr_list = (int **)MALLOC(n_ma_vox*pisz);
	if( MEMCHECK(*nhbr_list,"*nhbr_list in set_nhbr_list()",n_ma_vox*pisz) )
		clean_up(0);

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		pcube = cube + ma_list[n];
		snum = 0;
		for( i = 1;  i < 27;  i++ )
		{
			if( *(pcube+offset[i]) == 99 ) snum++;
		}
		(*num_nhbr)[n] = snum;

		if( snum )
		{
			(*nhbr_list)[n] = (int *)MALLOC(snum*isz);
			sprintf(msg,"(*nhbr_list)[%d] in set_nhbr_list()",n);
			if( MEMCHECK((*nhbr_list)[n],msg,snum*isz) ) clean_up(0);
		}
		else	(*nhbr_list)[n] = (int *)NULL;

		j = 0;
		for( i = 1;  (i < 27 && j < snum);  i++ )
		{
			if( *(pcube+offset[i]) == 99 )
			{
				(*nhbr_list)[n][j] = ma_list[n]+offset[i];
				j++;
			}
		}
	}
}


void	free_nhbr_list(unsigned char *num_nhbr, int **nhbr_list, int n_ma_vox)
{	
	int	n;

	int	isz = sizeof(int);
	int	pisz = sizeof(int *);
	int	usz = sizeof(unsigned char);

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		if( num_nhbr[n] ) FREE(nhbr_list[n],num_nhbr[n]*isz);
	}
	FREE(nhbr_list,n_ma_vox*pisz);
	FREE(num_nhbr ,n_ma_vox*usz);
}


void	set_MA_sets(int *ma_list, int **nhbr_list, unsigned char *num_nhbr,
					unsigned char *burn_dat, int *cube, int *offset,
					int n_ma_vox, int nx, int nxy, MA_set **mas_root,
					MA_set **map_root, MA_set **mac_root, int *n_sngl,
					int *n_path, int *n_clust)
{
	MA_set	*prev_mas = NULL, *mas;
	MA_set	*prev_map = NULL, *map;
	MA_set	*prev_mac = NULL, *mac;

	char	*msg;

	unsigned char	*done_list;

	int	*path;

	int	i, j;
	int	n, s_n;
	int	right_ind;
	int	num_on_path, tot_num_on_path, path_n;
	int	num_in_clust, clust_n, num_att;
	int	clust_alloc_size, att_alloc_size, alloc_diff;
	int	num_singlets, num_paths, num_clusters;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	cross_link_ma_list_cube_entries(ma_list,n_ma_vox,cube);

	done_list = (unsigned char *)CALLOC(n_ma_vox,usz);
	msg = "done_list in set_MA_sets()";
	if( MEMCHECK(done_list,msg,n_ma_vox*usz) ) clean_up(0);

	num_singlets = num_paths = num_clusters = 0;
	for( i = 0;  i < n_ma_vox;  i++ )
	{
		if( num_nhbr[i] == 0 ) num_singlets++;
	}

	if( num_singlets > 0 )
	{
		mas = create_next_MA_set(mas_root,prev_mas);
		mas->num_in_set = num_singlets;

		mas->loc = (int *)MALLOC(num_singlets*isz);
		msg = "mas->loc in set_MA_sets()";
		if( MEMCHECK(mas->loc,msg,num_singlets*isz) ) clean_up(0);

		if( burn_dat != NULL )
		{
			mas->bdat = (unsigned char *)MALLOC(num_singlets*usz);
			msg = "mas->bdat in set_MA_sets()";
			if( MEMCHECK(mas->bdat,msg,num_singlets*usz) )
				clean_up(0);
		}

		s_n = 0;
	}
	*n_sngl = num_singlets;

	for( n = 0;  n < n_ma_vox;  n++ )
	{
	    if( done_list[n] ) continue;	/* already processed */

	    switch( num_nhbr[n] )
	    {
		case 0:
		    done_list[n] = 1;
		    mas->loc[s_n]  =  ma_list[n];
		    if( burn_dat != NULL ) mas->bdat[s_n] = burn_dat[n];
		    s_n++;
		    break;

		case 1:
		    map = create_next_MA_set(map_root,prev_map);

		    done_list[n] = 1;
		    num_on_path = path_n = 1;
		    rec_ma_path(done_list,num_nhbr,nhbr_list,cube,ma_list[n],
				nhbr_list[n][0],&(map->loc),&num_on_path,
				&path_n);
		    map->loc[0] = ma_list[n];
		    map->num_in_set = num_on_path;

		    /*************
		    PRINTF("\nOne neighbor path\n");
		    print_loc_set(map->loc,num_on_path,nx,nxy);
		    *************/

		    if( burn_dat != NULL ) set_MA_set_bdat(cube,map,burn_dat);

		    num_paths++;
		    map->ind = num_paths;

		    prev_map = map;
		    break;

		case 2:
		    map = create_next_MA_set(map_root,prev_map);

		    right_ind = nhbr_list[n][0];

		    num_on_path = path_n = 0;
		    rec_ma_path(done_list,num_nhbr,nhbr_list,cube,right_ind,
				ma_list[n],&(map->loc),&num_on_path,&path_n);
		    map->num_in_set = num_on_path;

		    /***************
		    PRINTF("\nTwo neighbor path\nFirst segment\n");
		    print_loc_set(map->loc,num_on_path,nx,nxy);
		    ***************/

		    if( map->loc[0] != map->loc[num_on_path-1] )
		    {
			invert_set_path(map);

			num_on_path = path_n = 0;
			rec_ma_path(done_list,num_nhbr,nhbr_list,cube,ma_list[n],
					right_ind,&path,&num_on_path,&path_n);

			/***************
			PRINTF("\nSecond segment\n");
			print_loc_set(path,num_on_path,nx,nxy);
			***************/

			tot_num_on_path = map->num_in_set + num_on_path;

			map->loc = (int *)REALLOC(map->loc,tot_num_on_path*isz);
			msg = "map->loc in set_MA_sets()";
			if( MEMCHECK(map->loc,msg,num_on_path*isz) ) clean_up(0);

			for( i = map->num_in_set, j = 0;  i < tot_num_on_path;
								      i++, j++ )
			{
				map->loc[i]  = path[j];
			}
			FREE(path,num_on_path*isz);
			map->num_in_set = tot_num_on_path;

			/***************
			PRINTF("\nJoined segments\n");
			print_loc_set(map->loc,map->num_in_set,nx,nxy);
			***************/
		    }

		    if( burn_dat != NULL ) set_MA_set_bdat(cube,map,burn_dat);

		    num_paths++;
		    map->ind = num_paths;

		    prev_map = map;
		    break;

		default:
		    mac = create_next_MA_set(mac_root,prev_mac);

		    att_alloc_size = clust_alloc_size = 20;

		    mac->loc = (int *)MALLOC(clust_alloc_size*isz);
		    msg = "mac->loc in set_MA_sets()";
		    if( MEMCHECK(mac->loc,msg,clust_alloc_size*isz) ) clean_up(0);

		    mac->attach_list = (int *)MALLOC(att_alloc_size*isz);
		    msg = "mac->attach_list in set_MA_sets()";
		    if( MEMCHECK(mac->attach_list,msg,att_alloc_size*isz) )
			clean_up(0);
			
		    mac->loc[0] = ma_list[n];	done_list[n] = 1;
		    num_in_clust = 1;	clust_n = 0;	num_att = 0;
		    rec_ma_cluster(done_list,num_nhbr,nhbr_list,cube,ma_list[n],
					&(mac->loc),&num_in_clust,&clust_n,
					&clust_alloc_size,&(mac->attach_list),
					&num_att,&att_alloc_size,nx,nxy);

		    alloc_diff = clust_alloc_size - num_in_clust;
		    if( alloc_diff > 0 )
		    {
			mac->loc = (int *)REALLOC(mac->loc,num_in_clust*isz);
			msg = "mac->loc in set_MA_sets()";
			if( MEMCHECK(mac->loc,msg,-alloc_diff*isz) ) clean_up(0);
		    }
		    mac->num_in_set = num_in_clust;

		    mac->num_att = num_att;
		    if( num_att )
		    {
		    	alloc_diff = att_alloc_size - num_att;
		    	if( alloc_diff > 0 )
		    	{
			    mac->attach_list = (int *)REALLOC(mac->attach_list,
								num_att*isz);
			    msg = "mac->attach_list in set_MA_sets()";
			    if( MEMCHECK(mac->attach_list,msg,-alloc_diff*isz) )
				clean_up(0);
		    	}
		    }
		    else
		    {
			FREE(mac->attach_list,att_alloc_size*isz);
			mac->attach_list = (int *)NULL;
		    }

		    /***************
		    PRINTF("\nCluster\n");
		    print_loc_set(mac->loc,num_in_clust,nx,nxy);
		    if( num_att > 0 )
		    {
		    	PRINTF("attached list\n");
		    	print_loc_set(mac->attach_list,num_att,nx,nxy);
		    }
		    else PRINTF("No attach list\n");
		    ***************/

		    if( burn_dat != NULL ) set_MA_set_bdat(cube,mac,burn_dat);

		    num_clusters--;
		    mac->ind = num_clusters;

		    prev_mac = mac;
		    break;
	    }
	}
	*n_path  = num_paths;
	*n_clust = -num_clusters;

	FREE(done_list,n_ma_vox*usz);

	set_path_len(*map_root,nx,nxy);
}


void	count_MA_sets(MA_set *mas_root, int *pn_mas)
{
	MA_set	*mas;

	int	num;

	num = 0;
	for( mas = mas_root;  mas != (MA_set *)NULL;  mas = mas->next ) num++;

	*pn_mas = num;
}


void	rec_ma_path(unsigned char *done_list, unsigned char *num_nhbrs,
					int *nhbr_list[], int *cube, int prev_ind, int this_ind,
					int **path_list, int *num_on_path, int *path_n)
{
	int	n, next_ind;
	int	this_path_n;

	int	isz = sizeof(int);

	n = cube[this_ind];

	/*********
	PRINTF("rec_ma_path: n %d this_ind %d\n",n,this_ind);
	*********/

	switch( num_nhbrs[n] )
	{
	    case 0:		/* shouldn't get here */
	    	PRINTFSTDERR("ERROR: Inconsistency in rec_ma_path()\n");
		clean_up(0);
		break;

	    case 1:		/* path ends */

		done_list[n] = 1;	/* no case break;  !!!!! */

	    default:		/* path branches at cluster point */
	    			/* don't mark done_list[n]. Save  */
				/* for cluster search.		  */

		(*num_on_path)++;
	    	*path_list = (int *)MALLOC((*num_on_path)*isz);
		if( MEMCHECK(*path_list,"*path_list in rec_ma_path()",
					(*num_on_path)*isz) ) clean_up(0);
		(*path_list)[*path_n] = this_ind;
		break;

	    case 2:		/* path continues */

		if( done_list[n] == 1 )		/* closed loop !!!!! */
		{
			PRINTFSTDOUT("Closed loop terminates at n %d ind %d\n",
								n,this_ind);
			(*num_on_path)++;
	    		*path_list = (int *)MALLOC((*num_on_path)*isz);
			if( MEMCHECK(*path_list,"*path_list in rec_ma_path()",
						(*num_on_path)*isz) ) clean_up(0);
			(*path_list)[*path_n] = this_ind;
		}
		else
		{
			done_list[n] = 1;
	    		next_ind = nhbr_list[n][0];
			if( next_ind == prev_ind ) next_ind = nhbr_list[n][1];

			this_path_n = *path_n;		(*path_n)++;
							(*num_on_path)++;
			rec_ma_path(done_list,num_nhbrs,nhbr_list,cube,this_ind,
					next_ind,path_list,num_on_path,path_n);
			(*path_list)[this_path_n] = this_ind;
		}
		break;
	}
}


/*
*	Perform a recursive, breadth first tree-based search for branch clusters
*/

void	rec_ma_cluster(unsigned char *done_list, unsigned char *num_nhbrs,
					   int *nhbr_list[], int *cube, int this_ind,
					   int **clust_list, int *num_in_clust, int *clust_n,
					   int *c_vec_sz, int **att_list, int *num_att,
					   int *att_vec_sz, int nx, int nxy)
{
	char	*msg;
	int	i, n, next_ind, next_n;
	int	j, on;
	int	chng_sz;

	int	isz = sizeof(int);

	n = cube[this_ind];
	/***************
	PRINTF("\nn %d this_ind %d ",this_ind);
	print_loc_set(&this_ind,1,nx,nxy);
	***************/

	switch( num_nhbrs[n] )
	{
	    case 0:		/* shouldn't get here */
	    	PRINTFSTDERR("ERROR: Inconsistency in rec_ma_clust()\n");
		clean_up(0);
		break;

	    case 1:		/* not in cluster */
	    case 2:		/* not in cluster */
		break;


	    default:
		for( i = 0;  i < (int)(num_nhbrs[n]);  i++ )
		{
		    next_ind = nhbr_list[n][i];
		    next_n   = cube[next_ind];

		    if( num_nhbrs[next_n] > 2 )
		    {
			if( ! done_list[next_n] )
			{
			    /***************
			    PRINTF("nhbr %d ",next_ind);
			    print_loc_set(&next_ind,1,nx,nxy);
			    ***************/

			    done_list[next_n] = 1;
			    if( *num_in_clust == *c_vec_sz )
			    {
				chng_sz = *c_vec_sz;
				*c_vec_sz += *c_vec_sz;
				*clust_list = (int *)REALLOC(*clust_list,
							(*c_vec_sz)*isz);
				msg = "*clust_list in rec_ma_cluster()";
				if( MEMCHECK(*clust_list,msg,chng_sz*isz) )
					clean_up(0);
			    }
			    (*clust_list)[*num_in_clust] = next_ind;
			    (*num_in_clust)++;
			}
		    }
		    else
		    {
			on = 0;
			for( j = 0;  j < *num_att;  j++ )
			{
				if( next_ind == (*att_list)[j] )
				{
					on = 1;
					break;
				}
			}
			if( !on )
			{
			    if( *num_att == *att_vec_sz )
			    {
				chng_sz = *att_vec_sz;
				*att_vec_sz += *att_vec_sz;
				*att_list = (int *)REALLOC(*att_list,
							(*att_vec_sz)*isz);
				msg = "*att_list in rec_ma_cluster()";
				if( MEMCHECK(*att_list,msg,chng_sz*isz) )
					clean_up(0);
			    }
			    (*att_list)[*num_att] = next_ind;
			    (*num_att)++;
			}
		    }
		}

		/***************
		PRINTF("cluster list\n");
		print_loc_set(*clust_list,*num_in_clust,nx,nxy);
		***************/

		if( *clust_n < (*num_in_clust)-1 )    /* not finished search */
		{
			(*clust_n)++;	next_ind = (*clust_list)[*clust_n];
			rec_ma_cluster(done_list,num_nhbrs,nhbr_list,cube,
					next_ind,clust_list,num_in_clust,
					clust_n,c_vec_sz,att_list,num_att,
					att_vec_sz,nx,nxy);
		}
		break;
	}
}


void	set_MA_set_bdat(int *cube, MA_set *mas, unsigned char *burn_dat)
{
	char	*msg;

	int	ind, sn, cn;
	int	usz = sizeof(unsigned char);

	mas->bdat = (unsigned char *)MALLOC(mas->num_in_set*usz);
	msg = "mas->bdat in set_MA_set_bdat()";
	if( MEMCHECK(mas->bdat,msg,mas->num_in_set*usz) ) clean_up(0);

	for( sn = 0;  sn < mas->num_in_set;  sn++ )
	{
		ind = mas->loc[sn];
		cn  = cube[ind];
		mas->bdat[sn] = burn_dat[cn];
	}
}


extern __declspec(dllexport) MA_set	*create_next_MA_set(MA_set **root, MA_set *prev_set)
{
	MA_set	*ma_set;

	int	Msz = sizeof(MA_set);

	ma_set = (MA_set *)MALLOC(Msz);
	if( MEMCHECK(ma_set,"ma_set in create_next_MA_set()",Msz) ) clean_up(0);

	if( prev_set == NULL )	*root = ma_set;
	else			prev_set->next = ma_set;

	ma_set->prev = prev_set;
	ma_set->next = NULL;
	ma_set->ind  = 0;
	ma_set->num_in_set = 0;
	ma_set->len = 0.0;
	ma_set->loc  = NULL;
	ma_set->bdat = NULL;
	ma_set->num_att = 0;
	ma_set->attach_list = NULL;
	ma_set->att_mas  = NULL;
	ma_set->on_tree  = 0;
	ma_set->surf_rmnt = 0;

	ma_set->num_cinf = 0;
	ma_set->cinf = NULL;
	ma_set->pinf = NULL;

	ma_set->level = 0;

	return ma_set;
}


void	invert_set_path(MA_set *map)
{
	int	ns, ne, tmp;

	ns = 0;		ne = map->num_in_set-1;

	while( ns <= ne )
	{
		tmp = map->loc[ne];
		map->loc[ne] = map->loc[ns];
		map->loc[ns] = tmp;
		ns++;		ne--;
	}
}


void	dbg_print_coord2_clust(MA_set *mac_root, int nx, int nxy)
{
	MA_set	*mac = mac_root;

	int	i;

	PRINTF("\n\tCoordination number 2 Clusters\n");
	for( mac = mac_root, i = 1;  mac != NULL;  mac = mac->next, i++ )
	{
		if( mac->num_att != 2 ) continue;
		print_MAS("Cluster","paths",mac,i,nx,nxy);
	}
	if( mac_root != NULL ) PRINTF("\nEnd %ss\n","Cluster");
	else		       PRINTF("\nNo  %ss\n","Cluster");
}


void	dbg_print_MA_sets(MA_set *mas_root, MA_set *map_root,
						  MA_set *mac_root, int nx, int nxy)
{
	print_MA_sets("Isolated MA Voxel Set","\0"      ,mas_root,nx,nxy);
	print_MA_sets("Path"		     ,"clusters",map_root,nx,nxy);
	print_MA_sets("Cluster"	             ,"paths"   ,mac_root,nx,nxy);
}


void	print_MA_sets(char *mas_type, char *att_mas_type, MA_set *root,
					  int nx, int nxy)
{
	MA_set	*mas;
	int	i;

	for( mas = root, i = 1;  mas != NULL;  mas = mas->next, i++ )
	{
		print_MAS(mas_type,att_mas_type,mas,i,nx,nxy);
	}
	if( root != NULL ) PRINTF("\nEnd %ss\n",mas_type);
	else		   PRINTF("\nNo %ss\n",mas_type);
}


void	print_surf_rmnts(MA_set *mac_root, int nx, int nxy)
{
	char	*mas_type, *att_mas_type;
	MA_set	*mac = mac_root;
	int	i;

	mas_type = "Surface remnant";	att_mas_type = "path";

	for( mac = mac_root, i = 1;  mac != NULL;  mac = mac->next, i++ )
	{
		if( mac->surf_rmnt )
			print_MAS(mas_type,att_mas_type,mac,i,nx,nxy);
	}
	if( mac_root != NULL ) PRINTF("\nEnd %ss\n",mas_type);
	else		       PRINTF("\nNo %ss\n",mas_type);
}


void	print_MAS(char *mas_type, char *att_mas_type, MA_set *mas,
				  int i, int nx, int nxy)
{	
	PRINTF("\n%s %d (%d:%d)\n",mas_type,i,mas->ind,mas);
	if( mas->len > 0 ) PRINTF("length %f\n",mas->len);
	if( mas->loc != NULL )
	{
		if( mas->bdat != NULL )
			print_loc_brn_set(mas->loc,mas->bdat,mas->num_in_set,
									nx,nxy);
		else
			print_loc_set(mas->loc,mas->num_in_set,nx,nxy);
	}
	if( mas->num_att > 0 )
	{
		if( mas->attach_list != NULL )
		{
			PRINTF("attached list\n");
			print_loc_set(mas->attach_list,mas->num_att,nx,nxy);
		}
		print_attached_MA_sets(att_mas_type,mas->att_mas,mas->num_att);
	}
	if( mas->surf_rmnt ) PRINTF("surface remnant\n");
}


void	print_attached_MA_sets(char *att_mas_type, MA_set **att_mas,
							   int num_att)
{
	int	i;

	if( att_mas == NULL ) return;

	PRINTF("attached %s\n",att_mas_type);
	for( i = 0;  i < num_att;  i++ )
	{
		if( i%8 ) PRINTF(", ");
		if( att_mas[i] == NULL ) PRINTF("NULL");
		else			 PRINTF("%d",att_mas[i]);
		if( (i+1)%8 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");
}


void	print_loc_set(int *set, int num_in_set, int nx, int nxy)
{
	int	n, i, j, k;

	Pad_ind_2_ijk(set[0],i,j,k,nx,nxy)
	PRINTF("    (%3d,%3d,%3d)",i,j,k);

	for( n = 1;  n < num_in_set;  n++ )
	{
		Pad_ind_2_ijk(set[n],i,j,k,nx,nxy)
		PRINTF(" -> (%3d,%3d,%3d)",i,j,k);
		if( (n+1)%4 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");
}


void	print_loc_brn_set(int *set, unsigned char *bdat, int num_in_set,
						  int nx, int nxy)
{
	int	n, i, j, k;

	Pad_ind_2_ijk(set[0],i,j,k,nx,nxy)
	PRINTF("    (%3d,%3d,%3d)[%3d]",i,j,k,bdat[0]);

	for( n = 1;  n < num_in_set;  n++ )
	{
		Pad_ind_2_ijk(set[n],i,j,k,nx,nxy)
		PRINTF(" -> (%3d,%3d,%3d)[%3d]",i,j,k,bdat[n]);
		if( (n+1)%3 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");
}


void	rec_free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz)
{
	if( mas == NULL ) return;
	rec_free_MA_set(mas->next,isz,usz,Msz,pMsz);
	free_MA_set(mas,isz,usz,Msz,pMsz);
}


void	free_MA_set(MA_set *mas, int isz, int usz, int Msz, int pMsz)
{
	if( mas == NULL ) return;

	FREE(mas->loc ,mas->num_in_set*isz);

	if( mas->bdat        != NULL ) FREE(mas->bdat    ,mas->num_in_set*usz);
	if( mas->attach_list != NULL ) FREE(mas->attach_list,mas->num_att*isz);
	if( mas->att_mas     != NULL ) FREE(mas->att_mas    ,mas->num_att*pMsz);
	if( mas->cinf        != NULL ) FREE(mas->cinf       ,mas->num_cinf*isz);
	if( mas->pinf        != NULL ) FREE(mas->pinf       ,mas->num_cinf*isz);

	FREE(mas,Msz);
}


void	count_ma_vox_processed(MA_set *mas_root, MA_set *map_root,
							   MA_set *mac_root, unsigned char *num_nhbrs,
							   int *cube)
{
	MA_set	*map, *mac;

	int	num_proc;
	int	end_ind, end_n, end_nhbrs;

	num_proc = 0;

	num_proc += (mas_root == NULL ) ? 0 : mas_root->num_in_set;

	for( map = map_root;  map != NULL;  map = map->next )
	{
		num_proc += map->num_in_set;

		end_ind = map->loc[0];
		end_n   = cube[end_ind];
		end_nhbrs = num_nhbrs[end_n];
		if( end_nhbrs > 1 ) num_proc -= 1;

		end_ind = map->loc[map->num_in_set-1];
		end_n   = cube[end_ind];
		end_nhbrs = num_nhbrs[end_n];
		if( end_nhbrs > 1 ) num_proc -= 1;
	}

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
		num_proc += mac->num_in_set;

	PRINTFSTDERR("%d ma voxels processed in sets\n",num_proc);
	PRINTFSTDOUT("%d ma voxels processed in sets\n",num_proc);
}


void	link_path_2_cluster(MA_set **map_root, MA_set *mac_root, int nx,
							int nxy, int *n_path)
{
	MA_set	*map, *mac;

	char	*msg;
	int	n, path_v_ind, s_ind, e_ind;
	int	num_3vox_2_c;

	int	pMsz = sizeof(MA_set *);

	for( map = (*map_root);  map != NULL;  map = map->next )
	{
		map->num_att = 2;
		map->att_mas = (MA_set **)CALLOC(2,pMsz); /* init to NULL */
		msg = "map->att_mas in link_path_2_cluster()";
		if( MEMCHECK(map->att_mas,msg,2*pMsz) ) clean_up(0);
	}

	num_3vox_2_c = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att == 0 ) continue;

		mac->att_mas = (MA_set **)MALLOC(mac->num_att*pMsz);
		msg = "mac->att_mas in link_path_2_cluster()";
		if( MEMCHECK(mac->att_mas,msg,mac->num_att*pMsz) ) clean_up(0);

		for( n = 0;  n < mac->num_att;  n++ )
		{
		    path_v_ind = mac->attach_list[n];
		    for( map = (*map_root);  map != NULL;  map = map->next )
		    {
				/* logic OK even if map->num_in_set == 2 */

			s_ind = map->loc[1];
			e_ind = map->loc[map->num_in_set-2];
			if( (s_ind == path_v_ind) && (e_ind == path_v_ind) )
			{
				link_3vox_path_2_cluster(map_root,map,mac,
							nx,nxy,&n,&num_3vox_2_c);
				break;
			}
			else if( s_ind == path_v_ind )
			{
				map->att_mas[0] = mac;
				mac->att_mas[n] = map;
				break;
			}
			else if( e_ind == path_v_ind )
			{
				map->att_mas[1] = mac;
				mac->att_mas[n] = map;
				break;
			}
		    }
		}
	}
	PRINTFSTDERR("\n%d single voxel needle-eye paths ",num_3vox_2_c);
	PRINTFSTDERR("absorbed into clusters\n");
	PRINTFSTDOUT("\n%d single voxel needle-eye paths ",num_3vox_2_c);
	PRINTFSTDOUT("absorbed into clusters\n");

	(*n_path) -= num_3vox_2_c;
}


/*
*	The three types of short paths dealt with in this special subroutine
*	are
*			 P
*			C C	CCpcc  and	CCpp
*
*	In the first case this routine will absorb the short path into
*	the cluster C.
*	In the second case the path will become a branch-branch path
*	between the cluster CC and the cluster cc.
*	In the third case the path will become a branch-leaf path.
*/

void	link_3vox_path_2_cluster(MA_set **map_root, MA_set *map,
								 MA_set *mac, int nx, int nxy,
								 int *cur_att_n, int *num_3vox_2_c)
{
	char	*msg;

	MA_set	*map_prev, *map_next;

	int	n, c_ind, s_ind, m_ind, e_ind;
	int	beg, end;
	int	dbg = 0;

	int	 isz = sizeof(int);
	int	 usz = sizeof(unsigned char);
	int	 Msz = sizeof(MA_set);
	int	pMsz = sizeof(MA_set *);

	s_ind = map->loc[0];	e_ind = map->loc[map->num_in_set-1];

	beg = end = 0;
	for( n = 0;  n < mac->num_in_set;  n++ )
	{
		c_ind = mac->loc[n];
		if( c_ind == s_ind ) beg = 1;
		if( c_ind == e_ind ) end = 1;
	}

	if( beg && end )		/* first case path */
	{
		if( dbg )
		{
			PRINTF("\nRemoving path %d from linked list\n",map);
			print_MAS("Path","clusters",map,map->ind,nx,nxy);
			PRINTF("\nadding to cluster %d\n",mac);
			print_MAS("Cluster","paths",mac,mac->ind,nx,nxy);
		}

			/* unlink this map */

		map_prev = map->prev;	map_next = map->next;
		if( map_prev == NULL )
		{
			 *map_root = map_next;
			(*map_root)->prev = NULL;
		}
		else
		{
			map_prev->next = map->next;
			if( map_next != NULL ) map_next->prev = map_prev;
		}

			/* add path to cluster */
			/* Remove entry from cluster attach list */

		mac->num_in_set++;

		mac->loc = (int *)REALLOC(mac->loc,mac->num_in_set*isz);
		msg = "mac->loc in link_3vox_path_2_cluster()";
		if( MEMCHECK(mac->loc,msg,isz) ) clean_up(0);

		mac->loc[mac->num_in_set-1] = map->loc[1];

		mac->bdat = (unsigned char *)REALLOC(mac->bdat,
						mac->num_in_set*usz);
		msg = "mac->bdat in link_3vox_path_2_cluster()";
		if( MEMCHECK(mac->bdat,msg,usz) ) clean_up(0);

		mac->bdat[mac->num_in_set-1] = map->bdat[1];

		m_ind = map->loc[1];
		for( n = 0;  n < mac->num_att;  n++ )
			if( mac->attach_list[n] == m_ind ) break;

		if( n == mac->num_att )
		{
			PRINTFSTDERR(
				"Logic error in link_3vox_path_2_cluster()\n");
			PRINTFSTDOUT(
				"Logic error in link_3vox_path_2_cluster()\n");
			clean_up(0);
		}
		for( n = n+1;  n < mac->num_att;  n++ )
			mac->attach_list[n-1] = mac->attach_list[n];

		mac->num_att--;
		mac->attach_list = (int *)REALLOC(mac->attach_list,
							mac->num_att*isz);
		msg = "mac->attach_list in link_3vox_path_2_cluster()";
		if( MEMCHECK(mac->attach_list,msg,-1*isz) ) clean_up(0);

		/* free one entry from mac->att_mas */
		mac->att_mas = (MA_set **)REALLOC(mac->att_mas,
						mac->num_att*isz);
		msg = "mac->att_mas in link_3vox_path_2_cluster()";
		if( MEMCHECK(mac->att_mas,msg,-1*pMsz) ) clean_up(0);

		(*cur_att_n)--;

		free_MA_set(map,isz,usz,Msz,pMsz);
		(*num_3vox_2_c)++;

		if( dbg )
		{
			PRINTF("\nAfter removal of path %d from linked list\n",
									map);
			print_MAS("Cluster","paths",mac,mac->ind,nx,nxy);
		}
	}
	else
	{
	    for( n = 0;  n < mac->num_in_set;  n++ )
	    {
		c_ind = mac->loc[n];
		     if( c_ind == s_ind ) { map->att_mas[0] = mac; break; }
		else if( c_ind == e_ind ) { map->att_mas[1] = mac; break; }
	    }
	    mac->att_mas[*cur_att_n] = map;
	}
}


void	set_MAS_ind(MA_set *mas_root, int ind_sgn)
{
	MA_set	*mas;
	int	i;

	for( mas = mas_root, i = 1;  mas != NULL;  mas = mas->next, i++ )
		mas->ind = ind_sgn*i;
}


void	set_path_len(MA_set *map_root, int nx, int nxy)
{
	MA_set	*map;

	for( map = map_root;  map != NULL;  map = map->next )
		set_map_len(map,nx,nxy);
}


void	set_map_len(MA_set *map, int nx, int nxy)
{
	int	n;
	int	pi, pj, pk;
	int	ni, nj, nk;
	float	di, dj, dk, dlen, path_len;

	path_len = 0.0;
	Pad_ind_2_ijk(map->loc[0],pi,pj,pk,nx,nxy)
	for( n = 1;  n < map->num_in_set;  n++ )
	{
		Pad_ind_2_ijk(map->loc[n],ni,nj,nk,nx,nxy)
		di = (float)(ni-pi);	dj = (float)(nj-pj);	dk = (float)(nk-pk);
		dlen = di*di + dj*dj + dk*dk;
		path_len += (float)sqrt(dlen);

		pi = ni;	pj = nj;	pk = nk;
	}
	map->len = path_len;
}


void	set_surf_rmnts(MA_set *mac_root, int nx, int nxy)
{
	MA_set	*mac;

	char	*msg;

	int	c_end[2], offset[27];
	int	*cloc;

	int	i, cnum;

	int	isz = sizeof(int);

	set_nhbr_offsets(nx,nxy,offset);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_in_set ==   1 ) continue;
		if( mac->num_att     <   3 ) { mac->surf_rmnt = 1;  continue; }
		if( mac->num_in_set  > 125 ) { mac->surf_rmnt = 1;  continue; }

		cnum = mac->num_in_set + 2;

		cloc = (int *)MALLOC(cnum*isz);
		msg = "cloc in set_surf_rmnts()";
		if( MEMCHECK(cloc,msg,cnum*isz) ) clean_up(0);

		c_end[0] = mac->attach_list[0];
		c_end[1] = mac->attach_list[1];

		for( i = 0;  i < 2;     i++ ) cloc[i] = c_end[i];
		for( i = 2;  i < cnum;  i++ ) cloc[i] = mac->loc[i-2];
		thin_bc_driver(&cloc,&cnum,c_end,2,nx,nxy);

		mac->surf_rmnt = is_surf_rmnt(cloc,cnum,offset,nx,nxy);

		FREE(cloc,cnum*isz);
	}
}


int	is_surf_rmnt(int *cloc, int cnum, int *offset, int nx, int nxy)
{
	char	*msg;

	int	*nbr;
	int	i, j, k;
	int	rmnt;
	int	isz = sizeof(int);

	nbr = (int *)CALLOC(cnum,isz);
	msg = "nbr in is_surf_rmnt()";
	if( MEMCHECK(nbr,msg,cnum*isz) ) clean_up(0);

	for( i = 0;  i < cnum-1;  i++ )
	{
	    for( j = i+1;  j < cnum;  j++ )
	    {
		for( k = 0;  k < 27;  k++ )
		{
			if( cloc[i] + offset[k] == cloc[j] )
			{
				(nbr[i])++;	(nbr[j])++;
			}
		}
	    }
	}

	rmnt = 0;
	for( i = 0;  i < cnum;  i++ )
	{
		if( nbr[i] > 2 ) { rmnt = 1;    break; }
	}

	FREE(nbr,cnum*isz);

	return rmnt;
}


void	summarize_cluster_info( MA_set *mac_root)
{
	MA_set	*mac;

	int	num_bc, num_sr;

	num_bc = num_sr = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->surf_rmnt ) num_sr++;
		else		     num_bc++;
	}
	PRINTFDEBUG("\n");
	PRINTFDEBUG("Cluster breakdown:\n");
	PRINTFDEBUG("\tbranch  clusters %d\n",num_bc);
	PRINTFDEBUG("\tsurface remnants %d\n",num_sr);
	PRINTFDEBUG("\t               -----\n");
	PRINTFDEBUG("\ttotal           %d\n",num_bc + num_sr);
	PRINTFDEBUG("\n");
}
