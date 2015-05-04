/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <string.h>
#include "data.h"
#include "lkc_prototype.h"
#include "ma_prototype.h"


void	pma_output(data_info *datainfo, int *nloc, UINT32 **ma_loc,
				   unsigned char **ma_bno, unsigned char *burn_cube)
{
	int	i, j, nx, ny, nz, nxy, Nx, tmpx, tmpy;
	int	thincount, tot_ma_vox;

	int	usz = sizeof(unsigned char);
	int	ssz = sizeof(short);
	int	isz = sizeof(int);
	int uint32sz = sizeof(UINT32);

	short	*pma;
	unsigned char	*ptr;

	nx = datainfo->xsize;	ny = datainfo->ysize;	nz = datainfo->zsize;
	nxy = nx*ny;
	Nx = nx - 2;

	ptr = datainfo->data + nxy;

	tot_ma_vox = 0;
	for( i = 0; i < nz-2;  i++ )
	{
		thincount = 0;      

		for( j = 0;  j < nxy;  j++, ptr++ )
		{
			if( (*ptr != EXTERIOR) && (*ptr != MATERIAL) ) thincount++;
		}

	    tot_ma_vox += thincount;
	}
	*nloc = tot_ma_vox;
	*ma_loc = (UINT32 *)MALLOC(tot_ma_vox*uint32sz);
	*ma_bno = (unsigned char *)MALLOC(tot_ma_vox*usz);

	pma = (short *)MALLOC(nxy*ssz);

	tot_ma_vox = 0;
	ptr = datainfo->data + nxy;
	for( i = 0; i < nz-2;  i++ )
	{
		thincount = 0;      

		for( j = 0;  j < nxy;  j++, ptr++ )
		{
				 if( *ptr == EXTERIOR ) pma[j] = -1;
			else if( *ptr != MATERIAL )
			{
				pma[j] = 0;    thincount++;
			}
			else pma[j] = -1;
		}

		for( j = 0;  j < nxy;  j++ )
		{
			if( pma[j] != -1 )
			{
				tmpy = j/nx;		tmpx = j - (tmpy * nx);
				tmpy--;			tmpx--;
				pma[j] = burn_cube[(tmpy*Nx)+tmpx];
			}
		}

		print_thin_slice(pma,nx,ny,i,*ma_loc,*ma_bno,thincount,tot_ma_vox);
		tot_ma_vox += thincount;
	}
  
	FREE(pma,nxy*usz);
}


void	print_thin_slice(short *pma, int nx, int ny, int k, UINT32 *ma_loc,
						 unsigned char *ma_bno, int thincount,
						 int tot_ma_vox)
{
	int	i, j, nxy;
	int	tmpx, tmpy, tmp;
	UINT32	*pma_loc;
	unsigned char	*pma_bno;
	nxy = nx*ny;

		/* These are freed in write_ma_fp() */

	if(thincount != 0)
	{
		pma_loc = &(ma_loc[tot_ma_vox]);
		pma_bno = &(ma_bno[tot_ma_vox]);

		for( i = 0, j = 0;  i < nxy;  i++ )
		{
			if( pma[i] != -1 )
			{
				tmpy = i/nx;
				tmpx = i - tmpy*nx;
				tmp = k*(nx-2)*(ny-2) + (tmpy-1)*(nx-2) + tmpx - 1;
				pma_loc[j] = (UINT32) tmp;
				pma_bno[j] = (unsigned char)pma[i];
				j++;
			}
		}
	}

	return;
}

