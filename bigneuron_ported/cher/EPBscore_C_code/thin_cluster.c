/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"
#include "ma_prototype.h"
#include "lkc_prototype.h"

// from statistics/maset.h
#define Ind_2_ijk(Iind,Ii,Ij,Ik,_nx,_nxy)\
{\
	int	Itmp = Iind;\
\
	Ik = Itmp / _nxy;	Itmp = Itmp % _nxy;\
	Ij = Itmp / _nx;	  Ii = Itmp % _nx;\
}

void	thin_branch_cluster(int nx, int nxy, int nxyz, int num_keep_pts,
							int *keep_pts, unsigned char *data,
							char conn_type, int *min, int t_vox,
							int dbg_print, int iso_or_dir)
{

	point_info  *ptinfo;
	data_info   *datainfo;
	
	unsigned char *tmp_data;
	
	int	i, ny, nz, itno, num_vox_removed;

	int	usz = sizeof(unsigned char);

	nz = nxyz/nxy;		ny = nxy/nx;

	ptinfo = point_init();
	init_deuler_table();

	datainfo = data_init(nx,ny,nz);
	
	tmp_data = datainfo->data;
	datainfo->data = data;

	for( i = 0;  i < num_keep_pts;  i++ )
		datainfo->data[ keep_pts[i] ] = KEEP_THIS_VOX;

	itno = 0;	num_vox_removed = 1;	/* force >= 1 loop */
	while( num_vox_removed )
	{
		if( iso_or_dir == 1 )
			num_vox_removed = thin_object_dir(datainfo,ptinfo,0,0,
						num_keep_pts,itno,
						conn_type,0);

		else	num_vox_removed = thin_object_iso(datainfo,ptinfo,0,0,
						num_keep_pts,itno,
						conn_type,
						0,1);
		itno++;
	}

	for( i = 0;  i < num_keep_pts;  i++ )
		datainfo->data[ keep_pts[i] ] = VOIDVOX;

	data = datainfo->data;
	datainfo->data = tmp_data;

	free_data_info(datainfo);
	free_point_info(ptinfo);
}


#define Set_xyz(_ind)\
	Ind_2_ijk(_ind,x,y,z,wx,wxy)\
	z = z - 2 + min[2];\
	y = y - 2 + min[1];\
	x = x - 2 + min[0]


void	impose_data_info(data_info *datainfo1, data_info *datainfo,
						 unsigned char *prev_data, int *min)
{
	int	i, wx, wxy, wxyz, x, y, z, o_x, o_xy;

	wx   =     datainfo->xsize;	o_x  =     datainfo1->xsize;
	wxy  =  wx*datainfo->ysize;	o_xy = o_x*datainfo1->ysize;
	wxyz = wxy*datainfo->zsize;

	for( i = 0;  i < wxyz;  i++ )
	{
		if( datainfo->data[i] == 1 )
		{
			Set_xyz(i);
			datainfo1->data[z*o_xy+y*o_x+x] = 1;
		}
		else if( datainfo->data[i] != prev_data[i] )
		{
			Set_xyz(i);
			datainfo1->data[z*o_xy+y*o_x+x] = 0;
		}
	}
}


void	impose_keep_pts(data_info *datainfo1, int num_keep_pts,
						int *keep_pts, int *min, int wx, int wxy)
{
	int	i, x, y, z, o_x, o_xy;

	o_x  =     datainfo1->xsize;
	o_xy = o_x*datainfo1->ysize;

	for( i = 0;  i < num_keep_pts;  i++ )
	{
		Set_xyz(keep_pts[i]);
		datainfo1->data[z*o_xy+y*o_x+x] = 127; /* used only for */
						       /* printout in dbg_io.c */
	}
}
