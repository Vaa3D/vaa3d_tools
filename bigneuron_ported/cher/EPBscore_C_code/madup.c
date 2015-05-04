/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "arch.h"
#include "burn_macro.h"
#include "ma_prototype.h"


void	set_cube_info_from_burn_files(int Xs, int Xe, int Ys,
									  int Ye, int Zs, int Ze, int nx,
									  int nxy, int nxyz, void *cube,
									  int cube_sz,
									  unsigned char *burn_cube
									 )
{
	unsigned char	*tmp_burn;

	char	*slc;

	int	i, Nx, Ny;

	int	isz = sizeof(int);
	int	ssz = sizeof(short);


	if( cube_sz == ssz ) s_mark_cube_exterior((short *)cube,nx,nxy,nxyz);
	else				 i_mark_cube_exterior((int *)cube,nx,nxy,nxyz);

	Nx = nx - 2;	Ny = nxy/nx - 2;
	slc = (char *)cube + nxy*cube_sz;
	for( i = Zs;  i <= Ze;  i++, slc += nxy*cube_sz )
	{
		tmp_burn = &(burn_cube[(i-Zs)*Nx*Ny]);
		if( cube_sz == ssz )
		{
			s_mark_cube_slc_fid_ext((short *)slc,nx,nxy,Nx,Ny,Xs,Xe,Ys,Ye,tmp_burn);
			s_mark_cube_slc_grains(Nx,Ny,tmp_burn,Xs,Xe,Ys,Ye,(short *)slc);
	    }
	    else
	    {
			i_mark_cube_slc_fid_ext((int *)slc,nx,nxy,Nx,Ny,Xs,Xe,Ys,Ye,tmp_burn);
			i_mark_cube_slc_grains(Nx,Ny,tmp_burn,Xs,Xe,Ys,Ye,(int *)slc);
	    }
	
	}

}


#define	Mark_cube_exterior \
 \
	int	i, k, ny, nz; \
 \
	ny = nxy/nx;	nz = nxyz/nxy; \
 \
	for( i = 0;  i < nxy;  i++ ) cube[i] = EXT_VAL; \
 \
	slc = cube + nxy; \
	for( k = 1;  k < nz-1;  k++, slc += nxy ) \
	{ \
	    for( i = 0;  i < nx;  i++ ) slc[i] = EXT_VAL; \
	    for( i = 1;  i < ny-1;  i++ ) \
	    { \
		slc[nx*i        ] = EXT_VAL; \
		slc[nx*(i+1) - 1] = EXT_VAL; \
	    } \
	    for( i = nxy-nx;  i < nxy;  i++ ) slc[i] = EXT_VAL; \
	} \
 \
	for( i = nxyz-nxy;  i < nxyz;  i++ ) cube[i] = EXT_VAL;


void	s_mark_cube_exterior(short *cube, int nx, int nxy, int nxyz)
{
	short	*slc;
	Mark_cube_exterior
}


void	i_mark_cube_exterior(int *cube, int nx, int nxy, int nxyz)
{
	int	*slc;
	Mark_cube_exterior
}


#define	Mark_cube_slc_fid_ext \
 \
	int	i, j, n, ma_ind, dX; \
   \
	dX = Xe-Xs+3; \
   \
	for( n = 0;  n < Nx*Ny;  n++ ) \
	{ \
		if( tmp_burn[n] == EXT_VAL ) \
		{ \
			j = n/Nx; \
			     if( j < Ys ) continue; \
			else if( j > Ye ) break; \
 \
			i = n - Nx*j; \
			if( (i < Xs) || (i > Xe) ) continue; \
 \
			i = i-Xs+1;	j = j-Ys+1; \
			ma_ind = i + j*dX; \
			slc[ma_ind] = EXT_VAL; \
		} \
	}


void	s_mark_cube_slc_fid_ext(short *slc, int nx, int nxy, int Nx, int Ny,
								int Xs, int Xe, int Ys, int Ye,
								unsigned char *tmp_burn)
{
	Mark_cube_slc_fid_ext
}


void	i_mark_cube_slc_fid_ext(int *slc, int nx, int nxy, int Nx, int Ny,
								int Xs, int Xe, int Ys, int Ye,
								unsigned char *tmp_burn)
{
	Mark_cube_slc_fid_ext
}


#define	Mark_cube_slc_grains \
 \
	int	i, j, n, ma_ind; \
	int	dX; \
 \
	dX = Xe-Xs+3; \
	for( n = 0;  n < Nx*Ny;  n++ ) \
	{ \
	      if( (tmp_burn[n] == GRAIN) || (tmp_burn[n] == 0) ) \
	      { \
		  j = n/Nx; \
		       if( j < Ys ) continue; \
		  else if( j > Ye ) break; \
		   \
		  i = n - Nx*j; \
		  if( (i < Xs) || (i > Xe) ) continue; \
		   \
		  i = i-Xs+1;	j = j-Ys+1; \
		  ma_ind = i + j*dX; \
		  if( tmp_burn[n] == GRAIN ) slc[ma_ind] = GRAIN; \
		  else			     slc[ma_ind] = GRN_BDRY; \
              } \
	}


void	s_mark_cube_slc_grains(int Nx, int Ny, unsigned char *tmp_burn,
							   int Xs, int Xe, int Ys, int Ye, short *slc)
{
	Mark_cube_slc_grains
}


void	i_mark_cube_slc_grains(int Nx, int Ny, unsigned char *tmp_burn,
							   int Xs, int Xe, int Ys, int Ye, int *slc)
{
	Mark_cube_slc_grains
}


#define Trim_ma()					\
  							\
	int	stencil[26];				\
	int	i, j, chk;				\
	int	pma_cnt_sav, dvcnt;			\
  							\
	set_stencil(nx,nxy,stencil);			\
							\
	if( nxyz/nxy == 3 )				\
	{						\
	    stencil[ 0] = 0; stencil[ 1] = 0; stencil[ 2] = 0;	\
	    stencil[ 3] = 0; stencil[ 4] = 0; stencil[ 5] = 0;	\
	    stencil[ 6] = 0; stencil[ 7] = 0; stencil[ 8] = 0;	\
	    stencil[17] = 0; stencil[18] = 0; stencil[19] = 0;	\
	    stencil[20] = 0; stencil[21] = 0; stencil[22] = 0;	\
	    stencil[23] = 0; stencil[24] = 0; stencil[25] = 0;	\
	}						\
  							\
	pma_cnt_sav = *pma_cnt;				\
	for( i = 0; i < nxyz; i++ )			\
	{						\
	    if( pma[i] == val )				\
	    {						\
		chk = 1;				\
		j = 0;					\
		while( (chk == 1) && (j < 26) )		\
		{					\
		    if( pma[i+stencil[j]] == EXT_VAL )	\
		    {					\
			pma[i] = 101;			\
			*pma_cnt = *pma_cnt - 1;	\
			chk = 0;			\
		    }					\
		    j++;				\
		}					\
	    }						\
	}						\
	dvcnt = pma_cnt_sav - (*pma_cnt);		\
	fprintf(stderr,"%d pma voxels removed due to trimming\n",dvcnt); \
	fprintf(stdout,"%d pma voxels removed due to trimming\n",dvcnt)



void	s_trim_cube_ma(short *pma, short val, int nx, int nxy, int nxyz,
					   int *pma_cnt)
{
	Trim_ma();
}


void	i_trim_cube_ma(int *pma, int val, int nx, int nxy, int nxyz,
					   int *pma_cnt)
{
	Trim_ma();
}
