/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "maset.h"
#include "ma_prototype.h"

//#define TST

void	hist_ma_len(char *ma_type, MA_set *map_root, int nx, int nxy,
					float vox_len, char *title, int to_std, int to_jgr)
{
	MA_set	*map, *s_mas, *e_mas;

	char	msg[80];

	float	*len_bin_val;
	float	av_all, av_bb, av_bl, av_ll;

	int	*all_ph, *bb_ph, *bl_ph, *ll_ph;
	int	max_len, i_len;
	int	t_all;
	int	n;

	int	isz = sizeof(int);
	int	fsz = sizeof(float);

	max_len = 0;
	for(  map = map_root;  map != NULL;  map = map->next )
	{
		i_len = (int)map->len;
		if( i_len > max_len ) max_len = i_len;
	}
	max_len++;

	all_ph = (int *)CALLOC(max_len,isz);
	strcpy(msg,"all_ph in hist_ma_len()");
	if( MEMCHECK(all_ph,msg,max_len*isz) ) clean_up(0);

	bb_ph = (int *)CALLOC(max_len,isz);
	strcpy(msg,"bb_ph in hist_ma_len()");
	if( MEMCHECK(bb_ph,msg,max_len*isz) ) clean_up(0);

	bl_ph = (int *)CALLOC(max_len,isz);
	strcpy(msg,"bl_ph in hist_ma_len()");
	if( MEMCHECK(bl_ph,msg,max_len*isz) ) clean_up(0);

	ll_ph = (int *)CALLOC(max_len,isz);
	strcpy(msg,"ll_ph in hist_ma_len()");
	if( MEMCHECK(ll_ph,msg,max_len*isz) ) clean_up(0);

	av_all = av_bb = av_ll = av_bl = 0;

	for( map = map_root;  map != NULL;  map = map->next )
	{
		av_all += map->len;
		i_len = (int)(map->len);

		all_ph[i_len]++;

		s_mas = map->att_mas[0];
		e_mas = map->att_mas[map->num_att-1];

		if( (s_mas != NULL) && (e_mas != NULL) )
		{
			bb_ph[i_len]++;		av_bb += map->len;
		}
		else if( (s_mas == NULL) && (e_mas == NULL) )
		{
			ll_ph[i_len]++;		av_ll += map->len;
		}
		else
		{
			bl_ph[i_len]++;		av_bl += map->len;
		}
	}

	len_bin_val = (float *)MALLOC(max_len*fsz);
	strcpy(msg,"len_bin_val in hist_ma_len()");
	if( MEMCHECK(len_bin_val,msg,max_len*fsz) ) clean_up(0);

	t_all = 0;
	for( n = 0;  n < max_len;  n++ )
	{
		len_bin_val[n] = (float)n;		t_all += all_ph[n];
	}

	set_quant_av(all_ph,max_len,&av_all);
	set_quant_av( bb_ph,max_len,&av_bb );
	set_quant_av( bl_ph,max_len,&av_bl );
	set_quant_av( ll_ph,max_len,&av_ll );

	if( to_std ) hist_ma_quant_2_std(ma_type,"length",all_ph,bb_ph,bl_ph,
					 ll_ph,av_all,av_bb,av_bl,av_ll,
					 len_bin_val,max_len);


	FREE(len_bin_val,max_len*fsz);
	FREE(     all_ph,max_len*isz);
	FREE(      bb_ph,max_len*isz);
	FREE(      bl_ph,max_len*isz);
	FREE(      ll_ph,max_len*isz);
}


#ifdef TST
void	pma_hist_driver(char *file_basename)
{
	unsigned char	*burn_dat, *pburn_dat;

	short	*pma, *ppma;

	int	*hist_dat;
	int	n, nx, nxy, nxyz;
	int	ind, max_ind, total, vox_cnt, trim;
	int	zs, ze;

	int	isz = sizeof(int);
	int	ssz = sizeof(short);
	int	usz = sizeof(unsigned char);


	input_pma_data(&nx,&nxy,&nxyz,&pma,&burn_dat,'n',&vox_cnt,&trim,
								&zs,&ze);

	hist_dat = (int *)CALLOC(100,isz);
	if( MEMCHECK(hist_dat,"hist_dat in pma_hist_driver()",100*isz) )
		clean_up(0);

	total = 0;
	max_ind = -1;
	pburn_dat = burn_dat;
	ppma = pma;
	for( n = 0;  n < nxyz;  n++, pburn_dat++ ,ppma++)
	{
		if( *ppma == 99)
		{
			ind = (int)(*pburn_dat);
			if( ind > max_ind ) max_ind = ind;
			hist_dat[ind]++;
			total++;
		}
	}
	FREE(burn_dat,nxyz*usz); 
	FREE(     pma,nxyz*ssz);
	
	brndat_2_hist_driver(max_ind,hist_dat,"Pore Medial Axis",'n');
	FREE(hist_dat,100*isz);
}


void	pma_nhbr_hist(unsigned char *num_nhbr, int n_ma_vox, char *title)
{
	int	nhbr_hist[27];
	int	i, n, snum;

	/* initialize nhbr histogram */
	for( i = 0;  i < 27;  i++ ) nhbr_hist[i] = 0;

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		i = num_nhbr[n];	(nhbr_hist[i])++;
	}

	snum = 0;
	for( i = 0;  i < 27;  i++ ) snum += nhbr_hist[i];

	PRINTF("%d voxels examined\n",snum);
	nhbr_hist_2_jgr(nhbr_hist,title);
}

#endif





void	hist_clust_size(MA_set *mac_root, int nx, int nxy, char *title,
						int to_std, int to_jgr)
{
	MA_set	*mac;

	char	*msg;

	int	*bc_hist, *sr_hist;

	int	cl_num;
	int	max_sz, c_sz;

	int	isz = sizeof(int);

	max_sz = 0;
	for(  mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_in_set > max_sz ) max_sz = mac->num_in_set;
	}
	max_sz++;

	bc_hist = (int *)CALLOC(max_sz,isz);
	msg = "bc_hist in hist_clust_size()";
	if( MEMCHECK(bc_hist,msg,max_sz*isz) ) clean_up(0);

	sr_hist = (int *)CALLOC(max_sz,isz);
	msg = "sr_hist in hist_clust_size()";
	if( MEMCHECK(sr_hist,msg,max_sz*isz) ) clean_up(0);

	cl_num = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		cl_num++;
		c_sz = mac->num_in_set;
		if( mac->surf_rmnt ) sr_hist[c_sz]++;
		else		     bc_hist[c_sz]++;
	}
	PRINTF("%d clusters present\n",cl_num);


	FREE(sr_hist,max_sz*isz);
	FREE(bc_hist,max_sz*isz);
}


#define	Print_xy(_fp,_ph,_max)\
	for( n = 1;  n < _max;  n++ )\
	{\
		if( _ph[n] != 0 ) fPRINTF(_fp,"%d %d\n",n,_ph[n]);\
	}\
	fPRINTF(_fp,"\n")

#ifdef TST


void	hist_clust_size_2_std(int *bc_hist, int *sr_hist, int max_sz)
{
	int	n;

	PRINTF("\nbranch cluster sizes\n");
	Print_xy(stdout,bc_hist,max_sz);
	PRINTF("\nsurface remnant sizes\n");
	Print_xy(stdout,sr_hist,max_sz);
}
#endif




void	hist_coord_nums(MA_set *mac_root, char *title, int to_std,
						int to_jgr)
{
	MA_set	*mac;

	char	*msg;

	int	*bcc_hist, *src_hist;
	int	max_cn;

	int	isz = sizeof(int);

	max_cn = 0;
	for(  mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att > max_cn ) max_cn = mac->num_att;
	}
	max_cn++;

	bcc_hist = (int *)CALLOC(max_cn,isz);
	msg = "bcc_hist in hist_coord_nums()";
	if( MEMCHECK(bcc_hist,msg,max_cn*isz) ) clean_up(0);

	src_hist = (int *)CALLOC(max_cn,isz);
	msg = "src_hist in hist_coord_nums()";
	if( MEMCHECK(src_hist,msg,max_cn*isz) ) clean_up(0);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->surf_rmnt ) src_hist[mac->num_att]++;
		else		     bcc_hist[mac->num_att]++;
	}

	FREE(src_hist,max_cn*isz);
	FREE(bcc_hist,max_cn*isz);
}

#ifdef TST
void	hist_coord_nums_2_std(int *bcc_hist, int *src_hist, int max_cn)
{
	int	n;

	if( bcc_hist != NULL )
	{
		PRINTF("\nbranch cluster coordination numbers\n");
		if( bcc_hist[0] ) PRINTF("%d %d\n",0,bcc_hist[0]);
		Print_xy(stdout,bcc_hist,max_cn);
	}

	if( src_hist != NULL )
	{
		PRINTF("\nsurface remnant coordination numbers\n");
		if( src_hist[0] ) PRINTF("%d %d\n",0,src_hist[0]);
		Print_xy(stdout,src_hist,max_cn);
	}
}



#define	L_sort(ii,jj) \
	{ tmp = l[ii];	l[ii] = l[jj];	l[jj] = tmp; }

void	hist_coord3_len(MA_set *map_root, MA_set *mac_root, char *title)
{
	MA_set	*map, *mac;

	char	msg[80];

	int	**len_hist;
	int	i, max_cn, cn;

	float	l[3];
	float	max_len, tmp;

	int    pisz = sizeof(int *);
	int	isz = sizeof(int);

	max_len = 0.0;
	for(  mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att != 3 ) continue;
		for( i = 0;  i < 3;  i++ )
		{
			map = mac->att_mas[i];
			if( map->len > max_len ) max_len = map->len;
		}
	}
	max_cn = (int)max_len;
	max_cn++;

	len_hist = (int **)MALLOC(3*pisz);
	if( MEMCHECK(len_hist,"len_hist in hist_coord3_len()",3*pisz) )
		clean_up(0);
	for( i = 0;  i < 3;  i++ )
	{
		len_hist[i] = (int *)CALLOC(max_cn,isz);
		sPRINTF(msg,"len_hist[%d] in hist_coord3_len()",i);
		if( MEMCHECK(len_hist[i],msg,max_cn*isz) ) clean_up(0);
	}

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att != 3 ) continue;
		for( i = 0;  i < 3;  i++ )
		{
			map = mac->att_mas[i];
			l[i] = map->len;
		}
		if( l[1] < l[0] ) L_sort(0,1)
		if( l[2] < l[0] )
		{
			L_sort(0,2)
			L_sort(1,2)
		}
		else if( l[2] < l[1] ) L_sort(1,2)
		for( i = 0;  i < 3;  i++ )
		{
			cn = (int)l[i];
			len_hist[i][cn]++;
		}
	}

	hc2_len_2_jgr_driver(len_hist,max_cn,title);

	for( i = 0;  i < 3;  i++ ) FREE(len_hist[i],max_cn*isz);
	FREE(len_hist,3*pisz);
}



void	hist_path_brn(MA_set *map_root)
{
	FILE	*fp;

	MA_set	*map;

	JgrMer	Jmer;
	Jgraph	*jgr = &(Jmer.Jgr);

	char	ans[5];

	int	i, i_no, strt_no;
	int	min_len, max_len, max_bnum;
	int	end_br;

	fp = set_jgrfile("path burn number histogram");

	PRINTF("Require paths start and end at branch clusters? ");
	PRINTF("(y,n): ");
	gets(ans);
	PRINTF("Require paths start and end at branch clusters? ");
	PRINTF("(y,n): %s\n",ans);

	end_br = 0;
	if( ans[0] == 'y' ) end_br = 1;

	PRINTF("Enter min path length to include in histograms: ");
	scanf("%d",&min_len);		getchar();
	PRINTF("Enter min path length to include in histograms: %d\n",
								min_len);

	set_merge_jgr_layout("path_burn_number",&Jmer,1);
	set_jcurve(&Jmer.Jc,"\0","none","solid",1.0,1.0);

	max_len = 0;	max_bnum = 0;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		if( map->num_in_set < min_len ) continue;
		if( end_br )
		{
			if(    (map->att_mas[0] == NULL)
			    || (map->att_mas[1] == NULL)   ) continue;
		}

		if( map->num_in_set > max_len ) max_len = map->num_in_set;
		for( i = 0;  i < map->num_in_set;  i++ )
		{
			if( (int)(map->bdat[i]) > max_bnum )
				max_bnum = map->bdat[i];
		}
	}
	PRINTF("Max path length %d, max burn number %d\n",
							max_len,max_bnum);

	set_jaxis_min_max(&(jgr->xaxis),1,0.0,1,(float)max_len);
	set_jaxis_min_max(&(jgr->yaxis),1,0.0,1,(float)max_bnum);

	i_no = 0;		strt_no = 0;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		if( map->num_in_set < min_len ) continue;
		if( end_br )
		{
			if(    (map->att_mas[0] == NULL)
			    || (map->att_mas[1] == NULL)   ) continue;
		}

		set_jgr_posn(jgr,i_no,strt_no,Jmer.nrows,Jmer.ncols,
				Jmer.dx,Jmer.dy,"voxel number","burn number");
		print_jgraph_top(fp,jgr);
		print_jcurve(fp,&Jmer.Jc);

		for( i = 0;  i < map->num_in_set;  i++ )
		{
			fPRINTF(fp," %d %d",i+1,map->bdat[i]);
			if( (i+1)%15 == 0 ) fPRINTF(fp,"\n");
		}
		fPRINTF(fp,"\n\n");

		i_no++;
		if( i_no == Jmer.nmerge )
		{
			fPRINTF(fp,"newpage\n\n");
			i_no = 0;	strt_no = 0;
		}
	}
	fclose(fp);
}


void	tree_clust_path_scatterplot(MA_tree *mat_root, MA_tree *iso_mat,
									char *title)
{
	FILE	*fp;

	MA_tree	*mat;

	Jgraph	Jgr;
	Jaxis	*xax, *yax;
	Jcurve	Jc;
	Jstring	Jstr;

	char	msg[50];

	int	max_paths, max_clust, num_t;

	float	x_pos, y_pos;

	fp = set_jgrfile("cluster-path scatterplot");

	set_dflt_jgraph(&Jgr);

	num_t = max_paths = max_clust = 0;
	for( mat = mat_root;  mat != NULL;  mat = mat->next )
	{
		num_t++;
		if( mat->num_clust > max_clust ) max_clust = mat->num_clust;
		if( mat->num_paths > max_paths ) max_paths = mat->num_paths;
	}

	xax = &(Jgr.xaxis);	yax = &(Jgr.yaxis);

	set_jaxis_min_max(xax,1,0.0,1,(float)max_clust);
	set_jaxis_min_max(yax,1,0.0,1,(float)max_paths);

	strcpy(xax->lbl,"Number of branch clusters");
	strcpy(yax->lbl,"number of paths");

	print_jgraph_top(fp,&Jgr);

	set_jcurve(&Jc,"\0","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);

	for( mat = mat_root;  mat != NULL;  mat = mat->next )
		fPRINTF(fp,"%d %d\n",mat->num_clust,mat->num_paths);

	x_pos = (float)0.1*max_clust;
	y_pos = (float)0.9*max_paths;

	sPRINTF(msg,"isolated clusters  %d",iso_mat->num_clust);
	set_jstring(&Jstr,msg,14,x_pos,y_pos,'l','t');
	print_jstring(fp,&Jstr);

	y_pos = (float)0.85*max_paths;
	sPRINTF(msg,"isolated paths     %d",iso_mat->num_paths);
	set_jstring(&Jstr,msg,14,x_pos,y_pos,'l','t');
	print_jstring(fp,&Jstr);

	y_pos = (float)0.8*max_paths;
	sPRINTF(msg,"disconnected trees %d",num_t);
	set_jstring(&Jstr,msg,14,x_pos,y_pos,'l','t');
	print_jstring(fp,&Jstr);

	x_pos = (float)0.5*max_clust;
	y_pos = (float)max_paths;
	set_jtitle(&(Jgr.title),title,16,x_pos,y_pos,'c','t');
	print_jtitle(fp,&(Jgr.title));

	fclose(fp);
}


#define	LM_sort(ii,jj)\
	{  tmp =   l[ii];    l[ii] =    l[jj];   l[jj] =  tmp;\
	  mtmp = map[ii];  map[ii] = map[jj];  map[jj] = mtmp; }


#define Sort_coord3_paths()\
{\
	MA_set	*mtmp;\
	int	tmp;\
\
	for( i = 0;  i < 3;  i++ )\
	{\
		map[i] = mac->att_mas[i];\
		l[i] = map[i]->num_in_set;\
	}\
	if( l[1] < l[0] ) LM_sort(0,1)\
	if( l[2] < l[0] )\
	{\
		LM_sort(0,2)\
		LM_sort(1,2)\
	}\
	else if( l[2] < l[1] ) LM_sort(1,2)\
}


int	max_bnum_on_MA_sets(MA_set *mas_root)
{
	MA_set	*mas;

	int	i;
	int	max_bnum, btmp;

	max_bnum = 0;
	for( mas = mas_root;  mas != NULL;  mas = mas->next )
	{
		for( i = 0;  i < mas->num_in_set;  i++ )
		{
			btmp = mas->bdat[i];
			if( btmp > max_bnum ) max_bnum = btmp;
		}
	}
	return max_bnum;
}


void	coord3_stats(MA_set *map_root, MA_set *mac_root, int *bc3_num)
{
	MA_set	*map[3], *mac;

	int	l[3];

	int	i;
	int	good, pn;
	int	max_bnum, btmp;

	int	 cl_num = 0;
	int	cl3_num = 0;
	int	l27_num = 0;
	int	lbn_num = 0;
	int	lbr_num = 0;

	max_bnum = max_bnum_on_MA_sets(map_root);
	btmp     = max_bnum_on_MA_sets(mac_root);
	max_bnum = (btmp > max_bnum) ? btmp : max_bnum;

	PRINTF("max burn number on medial axis is %d\n",max_bnum);
	PRINTF("max burn number on medial axis is %d\n",max_bnum);

	cl_num = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next ) cl_num++;
	PRINTF("%d branch clusters present\n",cl_num);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att    != 3 ) continue;
		cl3_num++;
		if( mac->num_in_set > 27 ) continue;
		l27_num++;

		Sort_coord3_paths()

		good = 0;
		     if(    (map[0]->att_mas[0] == NULL)
			 || (map[0]->att_mas[1] == NULL) ) { pn = 0;  good = 1;}
		else if(    (map[1]->att_mas[0] == NULL)
			 || (map[1]->att_mas[1] == NULL) ) { pn = 1;  good = 1;}
		else if(    (map[2]->att_mas[0] == NULL)
			 || (map[2]->att_mas[1] == NULL) ) { pn = 2;  good = 1;}

		if( ! good ) continue;
		lbr_num++;

		if( l[pn] > max_bnum ) continue;    /* dead end path is short */
		lbn_num++;
	}

	*bc3_num = lbr_num;

	PRINTF("\n%.2f %% = %d of clusters are coord number 3\n",
			100.0*((float)cl3_num)/((float)cl_num),cl3_num);
	PRINTF("%.2f %% = %d of these also have size <= 27 voxels\n",
			100.0*((float)l27_num)/((float)cl_num),l27_num);
	PRINTF("%.2f %% = %d of these also have path not leading ",
			100.0*((float)lbr_num)/((float)cl_num),lbr_num);
	PRINTF("to another branch cluster\n");
	PRINTF("In %.2f %% = %d of these this path is of ",
			100.0*((float)lbn_num)/((float)cl_num),lbn_num);
	PRINTF("length < max burn num\n\n");
}
#endif


void	hist_iso_vox_bnum(MA_set *mas, char *title, int to_std, int to_jgr)
{
	int	*ivb_hist;
	int	num_iv, max_bnum, snum;
	int	i, bn;

	char	*msg;

	int	isz = sizeof(int);

	num_iv = (mas == NULL ) ? 0 : mas->num_in_set;

	if( num_iv == 0 )
	{
		PRINTF("No isolated voxels in ma_set.\n");
		PRINTF("No isolated voxels in ma_set.\n");
		return;
	}

	max_bnum = 0;
	for( i = 0;  i < num_iv;  i++ )
		if( (int)(mas->bdat[i]) > max_bnum ) max_bnum = mas->bdat[i];

	ivb_hist = (int *)CALLOC(max_bnum+1,isz);
	msg = "ivb_hist in iso_vox_bnum_hist()";
	if( MEMCHECK(ivb_hist,msg,(max_bnum+1)*isz) ) clean_up(0);

	for( i = 0;  i < num_iv;  i++ )
	{
		bn = mas->bdat[i];	(ivb_hist[bn])++;
	}

	snum = 0;
	for( i = 0;  i <= max_bnum;  i++ ) snum += ivb_hist[i];

	PRINTF("%d isolated voxels histogrammed\n",snum);

	FREE(ivb_hist,(max_bnum+1)*isz);
}

#ifdef TST

void	iv_bnum_2_std(int *ivb_hist, int max_bnum)
{
	int	n;

	PRINTF("\nIsolated Voxel burn number histogram\n");
	PRINTF("bnum\tnum_vox\n");
	for( n = 0;  n <= max_bnum;  n++ )
	{
		if( ivb_hist[n] > 0 ) PRINTF("%d\t%d\n",n,ivb_hist[n]);
	}
	PRINTF("\n");
}


void	iv_bnum_2_jgr(int *ivb_hist, int max_bnum, char *title)
{
	FILE	*fp;

	Jgraph	Jgr;
	Jaxis	*xax, *yax;
	Jcurve	Jc;

	int	n;

	float	x_pos, y_pos, y_max;

	fp = set_jgrfile("isolated voxel burn number histogram");

	set_dflt_jgraph(&Jgr);
	xax = &(Jgr.xaxis);		yax = &(Jgr.yaxis);

	strcpy(xax->lbl,"burn number");
	strcpy(yax->lbl,"number of isolated voxels");
	set_jaxis_min_max(xax,1,0.0,1,(float)max_bnum);
	set_jaxis_min_max(yax,1,0.0,0,0.0);

	print_jgraph_top(fp,&Jgr);

	set_jcurve(&Jc,"\0","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);

	y_max = 0.0;
	for( n = 0;  n <= max_bnum;  n++ )
	{
		if( ivb_hist[n] > 0 )
		{
			fPRINTF(fp,"%d %d\n",n,ivb_hist[n]);
			if( y_max < ivb_hist[n] ) y_max = (float)ivb_hist[n];
		}
	}
	fPRINTF(fp,"\n");

	x_pos = (float)max_bnum;	x_pos *= 0.5;
	y_pos = y_max;

	set_jtitle(&(Jgr.title),title,16,x_pos,y_pos,'c','t');
	print_jtitle(fp,&(Jgr.title));

	fclose(fp);
}
#endif

void	hist_clust_unassigned_paths(MA_set *mac_root, int *with_one)
{
	MA_set	*mac;

	char	*msg;

	int	*cup_hist;
	int	n, max_cn, unused;

	int	isz = sizeof(int);

	max_cn = 0;
	for(  mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_att > max_cn ) max_cn = mac->num_att;
	}
	max_cn++;

	cup_hist = (int *)CALLOC(max_cn,isz);
	msg = "cup_hist in hist_clust_unassigned_paths()";
	if( MEMCHECK(cup_hist,msg,max_cn*isz) ) clean_up(0);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		unused = 0;
		for( n = 0;  n < mac->num_att;  n++ )
		{
			if( mac->att_mas[n]->on_tree ) continue;
			unused++;
		}
		cup_hist[unused]++;
	}

	PRINTF("\ncluster unassigned map histogram\n");
	PRINTF("coord\tnum_clusters\n");
	for( n = 0;  n < max_cn;  n++ )
	{
		if( cup_hist[n] > 0 ) PRINTF("%d\t%d\n",n,cup_hist[n]);
	}
	PRINTF("\n");

	*with_one = (max_cn > 1) ? cup_hist[1] : 0;

	FREE(cup_hist,max_cn*isz);
}


void	hist_clust_radius(MA_set *mac_root, float vox_len, char *title,
						  int to_std, int to_jgr)
{
	MA_set	*mac;

	char	*msg;

	int	*bn_hist;

	int	cl_num;
	int	max_bn, bn;

	int	isz = sizeof(int);

	max_bn = 0;
	for(  mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		bn = max_bnum_in_maset(mac);
		if( bn > max_bn ) max_bn = bn;
	}
	PRINTF("Maximum burn number in clusters is %d\n",max_bn);
	PRINTF("Maximum burn number in clusters is %d\n",max_bn);

	max_bn++;		/* burn number 0 not used */

	bn_hist = (int *)CALLOC(max_bn,isz);
	msg = "bn_hist in hist_clust_radius()";
	if( MEMCHECK(bn_hist,msg,max_bn*isz) ) clean_up(0);

	cl_num = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		cl_num++;
		bn = max_bnum_in_maset(mac);
		bn_hist[bn]++;
	}
	PRINTF("%d clusters considered\n",cl_num);


	FREE(bn_hist,max_bn*isz);
}
#ifdef TST

void	hist_clust_radius_2_std(int *bn_hist, int max_bn)
{
	int	n;

	PRINTF("\nMaximum burn numbers in clusters\n");
	Print_xy(stdout,bn_hist,max_bn);
}


void	hist_clust_radius_2_jgr(int *bn_hist, int max_bn, float vox_len,
								char *title)
{
	FILE	*fp;

	Jgraph	Jgr;
	Jaxis	*xax, *yax;
	Jcurve	Jc;

	int	n;

	float	x_pos, y_max, norm;

	fp = set_jgrfile("cluster radius histogram");

	set_dflt_jgraph(&Jgr);
	xax = &(Jgr.xaxis);		yax = &(Jgr.yaxis);

	strcpy(xax->lbl,"radius (microns)");
	strcpy(yax->lbl,"probability");
	set_jaxis_min_max(xax,1,0.0,1,max_bn*vox_len);

	y_max = 0.0;	norm = 0.0;
	for( n = 1;  n < max_bn;  n++ )
	{
		norm += bn_hist[n];
		if( y_max < bn_hist[n] ) y_max = (float)bn_hist[n];
	}
	y_max /= norm;

	set_jaxis_min_max(yax,1,0.0,1,y_max);

	print_jgraph_top(fp,&Jgr);

	set_jcurve(&Jc,"\0","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);

	for( n = 1;  n < max_bn;  n++ )
	{
		if( bn_hist[n] > 0 )
		{
			fPRINTF(fp,"%f %f\n",n*vox_len,bn_hist[n]/norm);
		}
	}
	fPRINTF(fp,"\n");

	x_pos = (float)0.5*vox_len*(float)max_bn;
	set_jtitle(&(Jgr.title),title,16,x_pos,y_max,'c','t');

	print_jtitle(fp,&(Jgr.title));
	fclose(fp);
}


void	hist_throat_chars(MA_set *map_root)
{
#ifdef NEW_CODE
	MA_set	*map, *s_mas, *e_mas;

	int	nndl, nbb, nbl, nll;
	int	*flair;
	int	*max_cbn;
	int	*plen;

	int	isz = sizeof(int);

	nndl = nbb = nbl = nll = 0;
	for(  map = map_root;  map != NULL;  map = map->next )
	{
		s_mas = map->att_mas[0];
		e_mas = map->att_mas[map->num_att-1];

		if( (s_mas != NULL) && (e_mas != NULL) )
		{
			if( s_mas == e_max ) nndl++;
			else		     nbb++;
		}
		else if( (s_mas == NULL) && (e_mas == NULL) ) nll++;
		else					      nbl++;
	}
	PRINTF("\nNum paths needle %d  bb %d  bl %d ll %d\n",nndl,nbb,nbl,nll);

	hist_path_flairs(map_root,&flair);
	hist_pflair_2_std(flair);
	/* max_cbn = (int *)MALLOC((nbb+nbl)*isz); */
	FREE(flair,101*isz);
#endif
}


#ifdef NEW_CODE
void	hist_path_flairs(map_root,pflair)
	MA_set	*map_root;
	int	**pflair;
{
	MA_set	*map, *s_mas, *e_mas;

	int	*flair;
	int	isz = sizeof(int);

	flair = (int *)CALLOC(101,isz);
	if( MEMCHECK(flair,"flair in hist_path_flairs()",101*isz) ) clean_up();

	for(  map = map_root;  map != NULL;  map = map->next )
	{
		s_mas = map->att_mas[0];
		e_mas = map->att_mas[map->num_att-1];

		if( (s_mas != NULL) && (e_mas != NULL) )
		{
			if( s_mas != e_max ) set_flair(map,flair);
		}
		else if( (s_mas == NULL) && (e_mas == NULL) ) continue;
		else			     set_flair(map,flair);
	}

	*pflair = flair;
}


void	set_flair(map,flair)
	MA_set	*map;
	int	*flair;
{
	int	i, irat;
	int	min_bn, max_bn, bn;
	float	rat;

	min_bn = max_bn = map->bdat[0];
	for( i = i;  i < map->num_in_set;  i++ )
	{
		bn = (int) map->bdat[i];
		     if( bn < min_bn ) min_bn = bn;
		else if( bn > max_bn ) max_bn = bn;
	}
	rat = (float)min_bn/(float)max_bn;
	irat = 100.0*rat;
	flair[irat]++;
}


void	hist_pflair_2_std(flair)
	int	*flair;
{
	PRINTF("\npath Flairs\n");
}
#endif
#endif