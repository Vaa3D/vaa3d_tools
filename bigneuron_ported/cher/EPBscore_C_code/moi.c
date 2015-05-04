/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"
#include "isoseg.h"
#include "ma_prototype.h"


MoI	*create_next_MoI(MoI **root, MoI *prev_moi)
{
	MoI	*moi;

	int	Msz = sizeof(MoI);

	moi = (MoI *)MALLOC(Msz);
	if( MEMCHECK(moi,"moi in create_next_MoI()",Msz) ) clean_up(0);

	if( prev_moi == (MoI *)NULL ) *root = moi;
	else			      prev_moi->next = moi;

	moi->next = (MoI *)NULL;
	moi->moi_set = 0;
	moi->pa_set  = 0;

	return	moi;
}

/*
void	print_MoI_list(MoI *moi_root, char *mtype)
{
	MoI	*moi;
	int	i;

	if( moi_root != (MoI *)NULL ) PRINTF("\nBegin ");
	else			      PRINTF("\nNo ");
	PRINTF("%s Moment of inertia list\n",mtype);

	for( moi = moi_root, i = 1;  moi != (MoI *)NULL;  moi = moi->next, i++ )
		print_MoI(moi,i);

	if( moi_root != (MoI *)NULL )
		PRINTF("\nEnd %s Moment of inertia list\n",mtype);
}


void	print_MoI(MoI *moi, int i)
{
	if( moi == (MoI *)NULL ) return;

	PRINTF("\n\t moi %d (%g,%g,%g)\n",i,moi->comx,moi->comy,moi->comz);
	PRINTF("%g\n",moi->fI[0]);
	PRINTF("%g\t%g\n",moi->fI[1],moi->fI[4]);
	PRINTF("%g\t%g\t%g\n",moi->fI[2],moi->fI[5],moi->fI[8]);
	if( moi->moi_set ) PRINTF("\nmoi: %g\t%g\t%g\n",
				moi->moi[0],moi->moi[1],moi->moi[2]);
	if( moi->pa_set )
	{
	    PRINTF("\n");
	    PRINTF("pa[1]: (%g,%g,%g)\n",moi->pa[0],moi->pa[1],moi->pa[2]);
	    PRINTF("pa[2]: (%g,%g,%g)\n",moi->pa[3],moi->pa[4],moi->pa[5]);
	    PRINTF("pa[3]: (%g,%g,%g)\n",moi->pa[6],moi->pa[7],moi->pa[8]);
	}
}
*/

void	rec_free_MoI_list(MoI *moi, int Msz)
{
	if( moi == (MoI *)NULL ) return;
	rec_free_MoI_list(moi->next,Msz);

	FREE(moi,Msz);
}


void	compute_prin_ax(MoI *moi_root)
{
	MoI	*moi;
	//float	fv1[3], fv2[3];
	//int	stat, i;
	int i;
	int	rdim, task;

	for( moi = moi_root, i = 0;  moi != (MoI *)NULL;  moi = moi->next, i++ )
	{
		rdim = 3;	task = 1;
#ifdef NEED_NEW_CODE
//#ifdef _AIX
		rs(&rdim,&rdim,moi->fI,moi->moi,&task,moi->pa,fv1,fv2,&stat);
		PRINTF("need new code in moi\n");
		exit(1);
//#else
//		rs_(&rdim,&rdim,moi->fI,moi->moi,&task,moi->pa,fv1,fv2,&stat);
//#endif
#endif
		moi->moi_set = 1;
		moi->pa_set  = 1;
	}
}


#define	Choose_pa(_a,_b,_c)\
	if( moi->pa[_c] < 0.0 )\
	{\
		moi->pa[_a] *= -1.0;\
		moi->pa[_b] *= -1.0;\
		moi->pa[_c] *= -1.0;\
	}


#ifdef TST
#if 0
void	hist_pa_dirs(MoI *moi_root, char *moi_type, char *title,
					 char *file_basename, char *dir)
{
	FILE	*fp;
	MoI	*moi;

	Jgraph	Jgr;
	Jaxis	*xax, *yax;

	char	xlbl[80], ylbl[80];
	char	msg[256];


	sPRINTF(msg,"%s",moi_type);
	strcat(msg," moment of inertia/principle axis distribution");
	fp = set_jgrfile_2(file_basename,dir,"_moi");

	set_title_graph(fp,&Jgr,title,8.5);

	set_dflt_jgraph(&Jgr);
	xax = &(Jgr.xaxis);		yax = &(Jgr.yaxis);
	xax->size = 1.75;		yax->size = 1.75;

	/*** hist_moi_2_jgr(moi_root,fp,&Jgr); ***/
	moi_scatt_2_jgr(moi_root,fp,&Jgr);

		/* choose principal-axis directions s.t. */
		/*   z components are always positive   */

	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		Choose_pa(0,1,2)
		Choose_pa(3,4,5)
		Choose_pa(6,7,8)
	}

	sPRINTF(xlbl,"\0");		xax->log  = 'n';
	sPRINTF(ylbl,"\0");		yax->log  = 'n';

	set_jaxis_min_max(xax,1,-1.0,1,1.0);
	set_jaxis_min_max(yax,1,-1.0,1,1.0);

	set_jgr_posn(&Jgr,3,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xy",moi_root,0,1,fp,&Jgr);

	set_jgr_posn(&Jgr,4,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("yz",moi_root,1,2,fp,&Jgr);

	set_jgr_posn(&Jgr,5,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xz",moi_root,0,2,fp,&Jgr);

	set_jgr_posn(&Jgr,6,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xy",moi_root,3,4,fp,&Jgr);

	set_jgr_posn(&Jgr,7,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("yz",moi_root,4,5,fp,&Jgr);

	set_jgr_posn(&Jgr,8,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xz",moi_root,3,5,fp,&Jgr);

	set_jgr_posn(&Jgr,9,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xy",moi_root,6,7,fp,&Jgr);

	set_jgr_posn(&Jgr,10,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("yz",moi_root,7,8,fp,&Jgr);

	set_jgr_posn(&Jgr,11,0,4,3,2.0,2.0,xlbl,ylbl);
	pa_2_jgr("xz",moi_root,6,8,fp,&Jgr);
}
#endif
#endif


#ifdef TST
void	moi_scatt_2_jgr(MoI *moi_root, FILE *fp, Jgraph *jgr)
{
	MoI	*moi;
	Jaxis	*xax, *yax;

	char	xlbl[80], ylbl[80];
	char	*msg;

	float	*m_vals, *ms, *mm, *mh;
	float	min[3], max[3];
	float	xpos, ypos;

	int	i, nval;
	int	fsz = sizeof(float);

	xax = &(jgr->xaxis);		yax = &(jgr->yaxis);

	nval = 0;
	for( i = 0;  i < 3;  i++ )
	{
		min[i] = max[i] = moi_root->moi[i];
	}
	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		nval++;
		for( i = 0;  i < 3;  i++ )
		{
			     if( moi->moi[i] < min[i] ) min[i] = moi->moi[i];
			else if( moi->moi[i] > max[i] ) max[i] = moi->moi[i];
		}
	}

	m_vals = (float *)MALLOC(3*nval*fsz);
	msg = "m_vals in moi_scatt_2_jgr()";
	if( MEMCHECK(m_vals,msg,3*nval*fsz) ) clean_up(0);

	ms = m_vals;    mm = ms + nval;    mh = mm + nval;
	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		*ms = moi->moi[0];	ms++;
		*mm = moi->moi[1];	mm++;
		*mh = moi->moi[2];	mh++;
	}

	sPRINTF(xlbl,"moment of inertia");	xax->log = 'n';
	sPRINTF(ylbl,"moment of inertia");	yax->log = 'n';

	jgr->xaxis.auto_hash_label = 1;
	jgr->yaxis.auto_hash_label = 1;

	set_jaxis_min_max(xax,1,min[0],1,max[0]);
	set_jaxis_min_max(yax,1,min[1],1,max[1]);
	strcpy(jgr->xaxis.lbl,"\0");	jgr->x_tran = 0.0;
	strcpy(jgr->yaxis.lbl,ylbl);	jgr->y_tran = 6.75;
	moi_2_scatt(nval,m_vals,m_vals+nval,fp,jgr);
	xpos = (float)(0.5*(min[0]+max[0]));	ypos = max[1];
	set_jtitle(&(jgr->title),"s/m",14,xpos,ypos,'c','b');
	print_jtitle(fp,&(jgr->title));

	set_jaxis_min_max(xax,1,min[0],1,max[0]);
	set_jaxis_min_max(yax,1,min[2],1,max[2]);
	strcpy(jgr->xaxis.lbl,xlbl);	jgr->x_tran = 2.25;
	strcpy(jgr->yaxis.lbl,"\0");	jgr->y_tran = 6.75;
	moi_2_scatt(nval,m_vals,m_vals+2*nval,fp,jgr);
	xpos = (float)(0.5*(min[0]+max[0]));	ypos = max[2];
	set_jtitle(&(jgr->title),"s/l",14,xpos,ypos,'c','b');
	print_jtitle(fp,&(jgr->title));

	set_jaxis_min_max(xax,1,min[1],1,max[1]);
	set_jaxis_min_max(yax,1,min[2],1,max[2]);
	strcpy(jgr->xaxis.lbl,"\0");	jgr->x_tran = 4.5;
	strcpy(jgr->yaxis.lbl,"\0");	jgr->y_tran = 6.75;
	moi_2_scatt(nval,m_vals+nval,m_vals+2*nval,fp,jgr);
	xpos = (float)(0.5*(min[1]+max[1]));	ypos = max[2];
	set_jtitle(&(jgr->title),"m/l",14,xpos,ypos,'c','b');
	print_jtitle(fp,&(jgr->title));

	FREE(m_vals,3*nval*fsz);
}
#endif


#ifdef TST
void	moi_2_scatt(int nval, float *mx, float *my, FILE *fp, Jgraph *jgr)
{
	Jcurve	Jc;

	int	n;

	fPRINTF(fp,"\n");
	print_jgraph_top(fp,jgr);

	set_jcurve(&Jc,"\0","circle","none",0.0,1.0);
	print_jcurve(fp,&Jc);

	for( n = 0;  n < nval;  n++ ) fPRINTF(fp,"%g %g\n",mx[n],my[n]);
	fPRINTF(fp,"\n");
}
#endif


#ifdef TST
void	hist_moi_2_jgr(MoI *moi_root, FILE *fp, Jgraph *jgr)
{
	Jaxis	*xax, *yax;

	char	xlbl[80], ylbl[80];

	float	*bin_val, *den, *cum;

	int	nbin;
	int	fsz = sizeof(float);

	xax = &(jgr->xaxis);		yax = &(jgr->yaxis);

	hist_moi(moi_root,&nbin,&bin_val,&den,&cum);

	sPRINTF(xlbl,"moment of inertia");
	sPRINTF(ylbl,"probability");
	xax->log  = 'y';
	set_jaxis_min_max(xax,1,bin_val[0],1,bin_val[nbin-1]);
	set_jaxis_min_max(yax,1,0.0,1,1.0);

	set_jgr_posn(jgr,0,0,1,3,2.0,2.0,xlbl,ylbl);
	jgr->y_tran = 6.75;
	moi_2_jgr(nbin,bin_val,den,cum,fp,jgr);

	set_jaxis_min_max(xax,1,bin_val[nbin],1,bin_val[2*nbin-1]);
	set_jgr_posn(jgr,1,0,1,3,2.0,2.0,xlbl,ylbl);
	jgr->y_tran = 6.75;
	moi_2_jgr(nbin,bin_val+nbin,den+nbin,cum+nbin,fp,jgr);

	set_jaxis_min_max(xax,1,bin_val[2*nbin],1,bin_val[3*nbin-1]);
	set_jgr_posn(jgr,2,0,1,3,2.0,2.0,xlbl,ylbl);
	jgr->y_tran = 6.75;
	moi_2_jgr(nbin,bin_val+2*nbin,den+2*nbin,cum+2*nbin,fp,jgr);

	FREE(bin_val,3*nbin*fsz);
	FREE(den    ,3*nbin*fsz);
	FREE(cum    ,3*nbin*fsz);
}
#endif


#define	Hist_den(_moi,_den,_min,_dx)\
	tmp = _moi;\
	ind = (int)((tmp - _min)/_dx);\
	ind = ( ind > (*nbin)-1 ) ? (*nbin)-1 : ind;\
	_den[ind]++


void	hist_moi(MoI *moi_root, int *nbin, float **bin_val, float **den,
				 float **cum)
{
	MoI	*moi;

	char	*msg;

	float	*sort_moi;
	float	*lsmoi, *msmoi, *hsmoi;
	float	*lbv,   *mbv,   *hbv;
	float	*lden,  *mden,  *hden;
	float	*lcum,  *mcum,  *hcum;
	float	min[3], med[3], max[3], dx[3];
	float	tmp;
	float	Ninv, lsum, msum, hsum;

	int	nval, i, ind;

	int	fsz = sizeof(float);

	nval = 0;
	for( i = 0;  i < 3;  i++ )
	{
		min[i] = max[i] = moi_root->moi[i];
	}
	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		nval++;
		for( i = 0;  i < 3;  i++ )
		{
			     if( moi->moi[i] < min[i] ) min[i] = moi->moi[i];
			else if( moi->moi[i] > max[i] ) max[i] = moi->moi[i];
		}
	}

	sort_moi = (float *)MALLOC(3*nval*fsz);
	msg = "sort_moi in hist_moi()";
	if( MEMCHECK(sort_moi,msg,3*nval*fsz) ) clean_up(0);

	lsmoi = sort_moi;    msmoi = lsmoi + nval;    hsmoi = msmoi + nval;
	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		*lsmoi = moi->moi[0];		lsmoi++;
		*msmoi = moi->moi[1];		msmoi++;
		*hsmoi = moi->moi[2];		hsmoi++;
	}
	lsmoi = sort_moi;    msmoi = lsmoi + nval;    hsmoi = msmoi + nval;
	float_C_heapsort(nval,lsmoi);	med[0] = lsmoi[nval/2];
	float_C_heapsort(nval,msmoi);	med[1] = msmoi[nval/2];
	float_C_heapsort(nval,hsmoi);	med[2] = hsmoi[nval/2];

	FREE(sort_moi,3*nval*fsz);

	PRINTF("\n\tMoment of inertia ranges\n");
	PRINTF("\tmin\tmed\tmax\n");
	for( i = 0;  i < 3;  i++ ) PRINTF("%g\t%g\t%g\n",min[i],med[i],max[i]);
	PRINTF("\n");

	set_nbin(nval,nbin);

		/* reset max[i], set dx[i] */
	for( i = 0;  i < 3;  i++ )
	{
		max[i] = (max[i] < 2.0*med[i]) ? max[i] : (float)2.0*med[i];
		dx[i] = (max[i]-min[i])/(float)(*nbin);
	}

	PRINTF("\n\tReset Moment of inertia ranges, nbin %d\n",*nbin);
	PRINTF("\tmin\tmed\tmax\tdx\n");
	for( i = 0;  i < 3;  i++ )
		PRINTF("%g\t%g\t%g\t%g\n",min[i],med[i],max[i],dx[i]);
	PRINTF("\n");

	*bin_val = (float *)MALLOC(3*(*nbin)*fsz);
	msg = "*bin_val in hist_moi()";
	if( MEMCHECK(*bin_val,msg,3*(*nbin)*fsz) ) clean_up(0);

	*den = (float *)CALLOC(3*(*nbin),fsz);
	msg = "*den in hist_moi()";
	if( MEMCHECK(*den,msg,3*(*nbin)*fsz) ) clean_up(0);

	*cum = (float *)CALLOC(3*(*nbin),fsz);
	msg = "*cum in hist_moi()";
	if( MEMCHECK(*cum,msg,3*(*nbin)*fsz) ) clean_up(0);

	lbv = *bin_val;    mbv = lbv + (*nbin);    hbv = mbv + (*nbin);
	for( i = 0;  i < (*nbin);  i++ )
	{
		lbv[i] = (float)(min[0] + (i+0.5)*dx[0]);
		mbv[i] = (float)(min[1] + (i+0.5)*dx[1]);
		hbv[i] = (float)(min[2] + (i+0.5)*dx[2]);
	}

	lden = *den;    mden = lden + (*nbin);    hden = mden + (*nbin);
	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		Hist_den(moi->moi[0],lden,min[0],dx[0]);
		Hist_den(moi->moi[1],mden,min[1],dx[1]);
		Hist_den(moi->moi[2],hden,min[2],dx[2]);
	}

	lden = *den;    mden = lden + (*nbin);    hden = mden + (*nbin);
	lcum = *cum;    mcum = lcum + (*nbin);    hcum = mcum + (*nbin);

	Ninv = (float)1.0/(float)nval;		lsum = msum = hsum = 0.0;
	for( i = 0;  i < *nbin;  i++ )
	{
		lden[i] *= Ninv;	lsum += lden[i];	lcum[i] = lsum;
		mden[i] *= Ninv;	msum += mden[i];	mcum[i] = msum;
		hden[i] *= Ninv;	hsum += hden[i];	hcum[i] = hsum;
	}
}


void	set_nbin(int nval, int *nbin)
{
	int	nav;

	nav = (int)((float)nval/16.0);
	     if( nav < 24 ) *nbin = 16;
	else if( nav < 48 ) *nbin = 32;
	else if( nav < 96 ) *nbin = 64;
	else                *nbin = 128;
}


#define	Print_xy(_dc)\
	for( n = 0;  n < nbin;  n++ )\
	{\
		if( _dc[n] >= 0.001 ) fPRINTF(fp,"%f %g\n",bin_val[n],_dc[n]);\
	}\
	fPRINTF(fp,"\n")


#ifdef TST
void	moi_2_jgr(int nbin, float *bin_val, float *den, float *cum,
				  FILE *fp, Jgraph *jgr)
{
	Jcurve	Jc;

	int	n;

	fPRINTF(fp,"\n");
	print_jgraph_top(fp,jgr);

	set_jcurve(&Jc,"\0","circle","none",1.0,1.0);
	print_jcurve(fp,&Jc);
	Print_xy(den);

	set_jcurve(&Jc,"\0","none","solid",1.0,1.0);
	print_jcurve(fp,&Jc);
	Print_xy(cum);
}
#endif


#ifdef TST
void	pa_2_jgr(char *proj, MoI *moi_root, int abc, int ord, FILE *fp,
				 Jgraph *jgr)
{
	MoI	*moi;

	fPRINTF(fp,"\n");
	print_jgraph_top(fp,jgr);

	fPRINTF(fp,"\n");
	fPRINTF(fp,"newcurve\n");
	fPRINTF(fp,"  marktype circle fill 0\n");
	fPRINTF(fp,"  linetype none\n");
	fPRINTF(fp,"  pts\n");

	for( moi = moi_root;  moi != (MoI *)NULL;  moi = moi->next )
	{
		fPRINTF(fp,"%g %g\n",moi->pa[abc],moi->pa[ord]);
	}

	set_jtitle(&(jgr->title),proj,14,(float)0.95,(float)0.95,'r','t');
	print_jtitle(fp,&(jgr->title));
}
#endif