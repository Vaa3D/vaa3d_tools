/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "maset.h"
#include "ma_prototype.h"

#ifdef TST
void	hist_ma_tort_driver(char *ma_type, MA_set *map_root, int nx,
							int nxy, char *title, int to_std, int to_jgr)
{
	float	*tort;
	int	num_tort;
	int	fsz = sizeof(float);

	set_path_tort(map_root,nx,nxy,&tort,&num_tort);
	hist_ma_tort(ma_type,map_root,tort,num_tort,nx,nxy,title,to_std,to_jgr);

	FREE(tort,num_tort*fsz);
}


void	set_path_tort(MA_set *map_root, int nx, int nxy, float **tort,
					  int *num_tort)
{
	MA_set	*map;

	char	*msg;

	float	di, dj, dk, dlen, path_len, lin_len;

	int	i;
	int	sind, eind;
	int	si, sj, sk;
	int	ei, ej, ek;

	int	fsz = sizeof(float);

	*num_tort = 0;
	for( map = map_root;  map != NULL;  map = map->next ) (*num_tort)++;

	*tort = (float *)MALLOC((*num_tort)*fsz);
	msg = "*tort in set_path_tort()";
	if( MEMCHECK(*tort,msg,(*num_tort)*fsz) ) clean_up(0);

	for( map = map_root, i = 0;  map != NULL;  map = map->next, i++ )
	{
		if( map->num_in_set < 3 ) { (*tort)[i] = 1.0;    continue; }
		path_len = map->len;
		sind = map->loc[0];
		eind = map->loc[map->num_in_set - 1];

		if( sind == eind ) /* closed path */
			eind = map->loc[map->num_in_set - 2];

		Pad_ind_2_ijk(sind,si,sj,sk,nx,nxy)
		Pad_ind_2_ijk(eind,ei,ej,ek,nx,nxy)
		di = (float)(ei-si);	dj = (float)(ej-sj);	dk = (float)(ek-sk);
		dlen = di*di + dj*dj + dk*dk;
		lin_len = (float)sqrt(dlen);

		(*tort)[i] = path_len/lin_len;
	}
}


void	hist_ma_tort(char *ma_type, MA_set *map_root, float *tort,
					 int num_tort, int nx, int nxy, char *title, int to_std,
					 int to_jgr)
{
	MA_set	*map, *s_mas, *e_mas;

	char	*msg;

	float	*tort_bin_val;
	float	min_tort, med_tort, max_tort, dt;
	float	av_all, av_bb, av_bl, av_ll;

	int	*all_ph, *bb_ph, *bl_ph, *ll_ph;
	int	i, ind;
	int	n, nbin;
	int	t_all;

	int	isz = sizeof(int);
	int	fsz = sizeof(float);

	set_tort_scale(tort,num_tort,&min_tort,&med_tort,&max_tort,&dt,&nbin,
						       	      &tort_bin_val);

	all_ph = (int *)CALLOC(nbin,isz);
	msg = "all_ph in hist_ma_tort()";
	if( MEMCHECK(all_ph,msg,nbin*isz) ) clean_up(0);

	bb_ph = (int *)CALLOC(nbin,isz);
	msg = "bb_ph in hist_ma_tort()";
	if( MEMCHECK(bb_ph,msg,nbin*isz) ) clean_up(0);

	bl_ph = (int *)CALLOC(nbin,isz);
	msg = "bl_ph in hist_ma_tort()";
	if( MEMCHECK(bl_ph,msg,nbin*isz) ) clean_up(0);

	ll_ph = (int *)CALLOC(nbin,isz);
	msg = "ll_ph in hist_ma_tort()";
	if( MEMCHECK(ll_ph,msg,nbin*isz) ) clean_up(0);

	av_all = av_bb = av_bl = av_ll = 0.0;

	for( map = map_root, i = 0;  map != NULL;  map = map->next, i++ )
	{
		ind = (int)((tort[i]-min_tort)/dt);
		if( ind >= nbin ) ind = nbin-1;

		all_ph[ind]++;
		av_all += tort[i];

		s_mas = map->att_mas[0];
		e_mas = map->att_mas[map->num_att-1];

		if( (s_mas != NULL) && (e_mas != NULL) )
		{
			bb_ph[ind]++;	av_bb += tort[i];
		}
		else if( (s_mas == NULL) && (e_mas == NULL) )
		{
			ll_ph[ind]++;	av_ll += tort[i];
		}
		else
		{
			bl_ph[ind]++;	av_bl += tort[i];
		}
	}

	t_all = 0;
	for( n = 0;  n < nbin;  n++ ) t_all += all_ph[n];

	set_quant_av(all_ph,nbin,&av_all);
	set_quant_av( bb_ph,nbin,&av_bb );
	set_quant_av( bl_ph,nbin,&av_bl );
	set_quant_av( ll_ph,nbin,&av_ll );

	if( to_std ) hist_ma_quant_2_std(ma_type,"tortuosity",all_ph,bb_ph,
					bl_ph,ll_ph,av_all,av_bb,av_bl,av_ll,
					tort_bin_val,nbin);
	if( to_jgr ) hist_ma_quant_2_jgr(ma_type,"tortuosity",all_ph,bb_ph,
					 bl_ph,ll_ph,av_all,av_bb,av_bl,av_ll,
					 (float)t_all,tort_bin_val,nbin,title);
	FREE(tort_bin_val,nbin*fsz);
	FREE(      all_ph,nbin*isz);
	FREE(       bb_ph,nbin*isz);
	FREE(       bl_ph,nbin*isz);
	FREE(       ll_ph,nbin*isz);
}


void	set_tort_scale(float *tort, int num_tort, float *pmin_tort,
					   float *pmed_tort, float *pmax_tort, float *pdt,
					   int *pnbin, float **p_tort_bin_val)
{
	char	*msg;

	float	*tort_bin_val, *sort_tort;
	float	min_tort, med_tort, max_tort, dt;

	int	i, nbin;

	int	fsz = sizeof(float);

	min_tort = max_tort = tort[0];
	for( i = 1;  i < num_tort;  i++ )
	{
		     if( tort[i] < min_tort ) min_tort = tort[i];
		else if( tort[i] > max_tort ) max_tort = tort[i];
	}

		/* check max_tort against median_tort */
		/*   reset max_tort if too outlying   */

	sort_tort = (float *)MALLOC(num_tort*fsz);
	msg = "sort_tort in set_tort_scale()";
	if( MEMCHECK(sort_tort,msg,num_tort*fsz) ) clean_up(0);

	for( i = 0;  i < num_tort;  i++ ) sort_tort[i] = tort[i];
	float_C_heapsort(num_tort,sort_tort);

	med_tort = sort_tort[num_tort/2];

	PRINTF("\ntort - min %g med %g max %g\n",
					min_tort,med_tort,max_tort);
	PRINTF("\ntort - min %g med %g max %g\n",
					min_tort,med_tort,max_tort);
	if( max_tort > 3*med_tort )
	{
		max_tort = 3*med_tort;
		PRINTF("tort max reset to %g\n",max_tort);
		PRINTF("tort max reset to %g\n",max_tort);
	}
	FREE(sort_tort,num_tort*fsz);

		/* generate number of bins */

	set_nbin(num_tort,&nbin);

	tort_bin_val = (float *)MALLOC(nbin*fsz);
	msg = "tort_bin_val in set_tort_scale()";
	if( MEMCHECK(tort_bin_val,msg,nbin*fsz) ) clean_up(0);

	dt = (max_tort-min_tort)/(float)nbin;
	for( i = 0;  i < nbin;  i++ )
		tort_bin_val[i] = (float)(min_tort + (i+0.5)*dt);

	*pmin_tort = min_tort;		*pmed_tort = med_tort;
	*pmax_tort = max_tort;		*pdt = dt;
	*pnbin = nbin;			*p_tort_bin_val = tort_bin_val;
}
#endif


void	set_quant_av(int *ph, int nbin, float *av)
{
	int	n;
	int	tot;

	tot = 0;
	for( n = 0;  n < nbin;  n++ ) tot += ph[n];
	*av = (tot == 0) ? 0 : (*av) / (float)tot;
}


#define Add_to_line(_ph,_tot)\
	if( _ph ) { sprintf(val,"%d\t",_ph);  _tot += _ph;  doit = 1; }\
	else	    sprintf(val,"\t");\
	strcat(line,val)


void	hist_ma_quant_2_std(char *ma_type, char *quant_type, int *all_ph,
							int *bb_ph, int *bl_ph, int *ll_ph,
							float av_all, float av_bb, float av_bl,
							float av_ll, float *quant_bin_val, int nbin
						)
{
	char	line[80], val[10];
	int	n, doit;
	int	tot_all, tot_bb, tot_bl, tot_ll;

	tot_all = tot_bb = tot_bl = tot_ll = 0;
	for( n = 0;  n < nbin;  n++ )
	{
		sprintf(line,"\t%g \t",quant_bin_val[n]);
		doit = 0;
		Add_to_line(all_ph[n],tot_all);
		Add_to_line( bb_ph[n],tot_bb);
		Add_to_line( bl_ph[n],tot_bl);
		Add_to_line( ll_ph[n],tot_ll);

	}
}

#ifdef TST
void	hist_ma_quant_2_jgr(char *ma_type, char *quant_type, int *all_ph,
							int *bb_ph, int *bl_ph, int *ll_ph,
							float av_all, float av_bb, float av_bl,
							float av_ll, float t_all, float *quant_bin_val,
							int nbin, char *title)
{
	FILE	*fp;

	Jgraph	Jgr;
	Jcurve	Jc;
	Jaxis	*xax, *yax;

	char	xlbl[80], ylbl[80];
	char	ftype[80];
	char	*msg;

	float	*den, *cum;
	float	x_pos, y_pos;
	float	fN;

	int	n;

	int	fsz = sizeof(float);

	sprintf(ftype,"%s %s distribution",ma_type,quant_type);
	fp = set_jgrfile(ftype);

	set_title_graph(fp,&Jgr,title,7.0);

		/* fake curves to get a global legend */
	set_jcurve(&Jc,"cumulative","none","solid",0.0,1.0);
	print_jcurve(fp,&Jc);
	set_jcurve(&Jc,"density","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);
	set_jlegend(&(Jgr.legend),14,6.5,0.5,'r','t');
	print_jlegend(fp,&(Jgr.legend));

	den = (float *)MALLOC(nbin*fsz);
	msg = "den in hist_ma_quant_2_jgr()";
	if( MEMCHECK(den,msg,nbin*fsz) ) clean_up(0);
			 
	cum = (float *)MALLOC(nbin*fsz);
	msg = "cum in hist_ma_quant_2_jgr()";
	if( MEMCHECK(cum,msg,nbin*fsz) ) clean_up(0);

	x_pos = quant_bin_val[nbin-1];
	y_pos = (float)0.4;

	set_dflt_jgraph(&Jgr);
	xax = &(Jgr.xaxis);		yax = &(Jgr.yaxis);
	xax->size = 3.0;		yax->size = 3.0;
					yax->log = 'y';

	sprintf(xlbl,"%s %s",ma_type,quant_type);
	sprintf(ylbl,"probability");
	set_jaxis_min_max(xax,1,1.0,1,quant_bin_val[nbin-1]);
	set_jaxis_min_max(yax,1,(float)(0.9/t_all),1,1.0);

	set_jgr_posn(&Jgr,0,0,2,2,3.25,3.25,xlbl,ylbl);
	ph_2_jgr(nbin,"all",av_all,all_ph,quant_bin_val,den,cum,fp,&Jgr,
		                                         x_pos,y_pos);

	jgr_y_axis_overlay(fp,&Jgr,yax,"\0",0,t_all);

	fN = 0.0;
	for( n = 0;  n < nbin;  n++ ) fN += bb_ph[n];
	set_jaxis_min_max(yax,1,(float)(0.9/t_all),1,t_all/fN);
	set_jaxis_hash_scale(yax,-1.0);

	set_jgr_posn(&Jgr,1,0,2,2,3.25,3.25,xlbl,ylbl);
	ph_2_jgr(nbin,"branch-branch",av_bb,bb_ph,quant_bin_val,den,cum,
		                                 fp,&Jgr,x_pos,y_pos);

	jgr_y_axis_overlay(fp,&Jgr,yax,"number of occurrences",1,t_all);

	fN = 0.0;
	for( n = 0;  n < nbin;  n++ ) fN += bl_ph[n];
	set_jaxis_min_max(yax,1,(float)(0.9/t_all),1,t_all/fN);
	set_jaxis_hash_scale(yax,-1.0);

	set_jgr_posn(&Jgr,2,0,2,2,3.25,3.25,xlbl,ylbl);
	ph_2_jgr(nbin,"branch-leaf",av_bl,bl_ph,quant_bin_val,den,cum,fp,
		                                     &Jgr,x_pos,y_pos);

	jgr_y_axis_overlay(fp,&Jgr,yax,"\0",0,t_all);

	fN = 0.0;
	for( n = 0;  n < nbin;  n++ ) fN += ll_ph[n];
	set_jaxis_min_max(yax,1,(float)(0.9/t_all),1,t_all/fN);
	set_jaxis_hash_scale(yax,-1.0);

	set_jgr_posn(&Jgr,3,0,2,2,3.25,3.25,xlbl,ylbl);
	ph_2_jgr(nbin,"leaf-leaf",av_ll,ll_ph,quant_bin_val,den,cum,fp,&Jgr,
		                                             x_pos,y_pos);

	jgr_y_axis_overlay(fp,&Jgr,yax,"\0",1,t_all);

	FREE(den,nbin*fsz);
	FREE(cum,nbin*fsz);
}


void	jgr_y_axis_overlay(FILE *fp, Jgraph *jgr, Jaxis *yax, char *ylbl,
						   int y_ahl, float t_all)
{
	strcpy(jgr->xaxis.lbl,"\0");
	jgr->xaxis.auto_hash_label = 0;
	strcpy(jgr->yaxis.lbl,ylbl);
	jgr->yaxis.auto_hash_label = y_ahl;
	set_jaxis_min_max(yax,1,(float)0.9,1,t_all);
	set_jaxis_hash_scale(yax,1.0);

	fprintf(fp,"\n");
	print_jgraph_top(fp,jgr);
}


void	set_den_cum(int nbin, int *ph, float *den, float *cum, float *Ninv)
{
	float	sum;
	int	n, N;

	N = 0;
	for( n = 0;  n < nbin;  n++ ) N += ph[n];

	*Ninv = (float)(1.0 / (float)N);
	sum = 0.0;
	for( n = 0;  n < nbin;  n++ )
	{
		den[n] = (float)ph[n] * (*Ninv);
		sum += den[n];
		cum[n] = sum;
	}
}


void	ph_2_jgr(int nbin, char *type, float av, int *ph,
				 float *quant_bin_val, float *den, float *cum, FILE *fp,
				 Jgraph *jgr, float x_pos, float y_pos)
{
	Jcurve	Jc;

	int	n, last_bin;
	float	Ninv;
	char	ph_type[256], s[50];

	set_den_cum(nbin,ph,den,cum,&Ninv);

	fprintf(fp,"\n");
	print_jgraph_top(fp,jgr);

	set_jcurve(&Jc,"\0","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);
	last_bin = 0;
	for( n = 0;  n < nbin;  n++ )
	{
		if( ph[n] > 0 )		/* plot is on log scale */
		{
			fprintf(fp,"%f %g\n",quant_bin_val[n],den[n]);
			last_bin = n;
		}
	}
	fprintf(fp,"\n");

	set_jcurve(&Jc,"\0","none","solid",0.0,1.0);
	print_jcurve(fp,&Jc);
	for( n = 0;  n <= last_bin;  n++ )
	{
		if( ph[n] >  0 )	/* plot is on log scale */
			fprintf(fp,"%f %g\n",quant_bin_val[n],cum[n]);
	}
	fprintf(fp,"\n");

	strcpy(ph_type,type);
	sprintf(s," (av %g)",av);
	strcat(ph_type,s);

	set_jtitle(&(jgr->title),ph_type,14,x_pos,y_pos,'r','t');
	print_jtitle(fp,&(jgr->title));
}
#endif