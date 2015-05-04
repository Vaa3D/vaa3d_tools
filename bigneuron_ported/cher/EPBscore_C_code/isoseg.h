/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _isoseg_h
#define _isoseg_h

typedef struct _Iso_dat
{
	/****
	int	fil_grain, grain_thresh;
	int	fil_pore,   pore_thresh;
	****/
	int	fil_grain, grain_min, grain_max, keep_max_grain;
	int	fil_pore,   pore_min,  pore_max,  keep_max_pore;
} Iso_dat;


typedef struct _Disc_vol
{
	int	p_val_sz;
	int	*pore_x_val, *pore_y_val;
	int	p_vol;
	int	g_val_sz;
	int	*grain_x_val, *grain_y_val;
	int	g_vol;
} Disc_vol;


typedef struct _MoI
{
	struct _MoI *next;
	double	comx, comy, comz;
	float	fI[9];
	int	moi_set;
	float	moi[3];
	int	pa_set;
	float	pa[9];
} MoI;


typedef struct _MoI_info
{
	MoI	*pmoi_root;
	MoI	*gmoi_root;
} MoI_info;
#endif	/* _isoseg_h */
