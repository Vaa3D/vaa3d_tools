/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */


#include "stdafx.h"
#include "burn_macro.h"
#include "burn_prototype.h"
#include "ma_prototype.h"

void	surf_burn(Bchains *bc, Fdat *tmp, int nx, int ny, int nz,
				  int use_bdry, int task, int *mbn, int *period)
{
	unsigned char   b_step;

	unsigned char	bd;

	int	dbg = 0;

	int	i;
	int	ending;
	int	realloc_size;
	int	ind, pos, ix, iy, iz;
	int	nxy, nxyz;

	/*** 
	   bc->cnt[i] = 1 --> no neighbor in direction i
	   bc->cnt[i] = 0 --> neighbor in direction i exists.
	****/

	b_step = 126;		ending = 0;
	nxy = nx*ny;		nxyz = nxy*nz;

	realloc_size = bc->o_num;

	while( !ending )
	{
		if( *period == 0 ) mexPrintf("N(%d)  %d\n",b_step-126,bc->o_num);
		else		  mexPrintf("N(%d)  %d\n",b_step    ,bc->o_num);

		for( i = 0;  i < bc->o_num;  i++ )
		{
			ind = bc->o_chain[i];
			iz = ind/nxy;		pos = ind%nxy;
			iy = pos/nx;		ix = pos%nx;
		     
			set_6nbr_bd(ix,iy,iz,&bd,nx,ny,nz);

			check_pz(bc->cnt,tmp->bfdat,iz,bd,pos,nxy);
			check_nz(bc->cnt,tmp->bfdat,iz,bd,pos,nxy);
			check_px(bc->cnt,tmp->bfdat,iz,bd,pos,nxy);
			check_nx(bc->cnt,tmp->bfdat,iz,bd,pos,nxy);
			check_py(bc->cnt,tmp->bfdat,iz,bd,pos,nxy,nx);
			check_ny(bc->cnt,tmp->bfdat,iz,bd,pos,nxy,nx);
		     
		if( task == MED_AX )
				surf_ma_step(bc,tmp,ind,b_step,nx,nxy,
								realloc_size);
		else 
		if( (task == ERODE) || (task == NO_OPT) )
		surf_burn_step(bc,tmp->bfdat,ind,nx,nxy,
								realloc_size);
		
		}
	   
		if( task == MED_AX )
		{
			check_90(tmp,nx,ny,nz,use_bdry);
			check_opposite(bc,tmp,nx,ny,nz);
		}
		
		b_step++;

		if( b_step >= BARRIER )   /*** BARRIER = 252 ****/
		{
		    if( *period == 0 )
		    {
				b_step = 126;	*period = 1;

				for( i = 0;  i < nxyz;  i++ )
				{
					if(    (tmp->bfdat[i] >= 126   )
						&& (tmp->bfdat[i] < BARRIER)   )
					tmp->bfdat[i] -= 126;
				}
		    }
		    else 
		    {
				mexErrMsgTxt("\nERROR, max burn number exceeds 251\n");
		    }
		}

		set_next_chain(bc,tmp->bfdat,b_step);

		if( bc->o_num == 0 ) ending = 1;
	}
	
	*mbn = ( *period == 0 ) ? b_step-127 : b_step-1;

	mexPrintf("\nHighest burn number is %d\n",*mbn);
	mexPrintf("\n****************************************\n");

	free_chains(bc->o_chain,bc->chain_size);
	free_chains(bc->n_chain,bc->chain_size);
}


void set_6nbr_bd(int ix, int iy, int iz, unsigned char *pbd, int NX,
					int NY, int NZ)
{
	unsigned char	bd;

	/* Note: if() else if() not desired in case NZ = 1, NY = 1, NX = 1 */

	bd = 0;
	if( iz == NZ-1 ) bd = bd | 32;       /* last slice   */
	if( iz == 0    ) bd = bd | 16;       /* first slice  */

	if( iy == NY-1 ) bd = bd | 8;        /* last row     */
	if( iy == 0    ) bd = bd | 4;        /* first row    */

	if( ix == NX-1 ) bd = bd | 2;        /* last column  */
	if( ix == 0    ) bd = bd | 1;        /* first column */

	*pbd = bd;
}


void	check_pz(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY)    /* check positive Z direction */
{
	int	ki;

	if( bd & 32 ) cnt[0] = 0;
	else
	{
	    ki = (k+1)*NXY + pos;
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[0] = 0;
	    else					 cnt[0] = 1;
	}
}


void	check_nz(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY)    /* check negative Z direction */
{
	int	ki;

	if( bd & 16 )  cnt[1] = 0;
	else
	{
	    ki = (k-1)*NXY + pos;
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[1] = 0;
	    else					 cnt[1] = 1;
	}
}


void	check_py(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY, int NX)    /* check positive Y direction */
{
	int	ki;

	if( bd & 8 ) cnt[4] = 0;
	else
	{
	    ki = k*NXY + (pos+NX);
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[4] = 0;
	    else					 cnt[4] = 1;
	}
}


void	check_ny(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY, int NX)    /* check negative Y direction */
{
	int	ki;

	if( bd & 4 ) cnt[5] = 0;
	else
	{
	    ki = k*NXY + (pos-NX);
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[5] = 0;
	    else					 cnt[5] = 1;
	}
}


void	check_px(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY)    /* check positive X direction */
{
	int	ki;

	if( bd & 2 ) cnt[2] = 0;
	else
	{	  
	    ki = k*NXY + (pos+1);
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[2] = 0;
	    else					 cnt[2] = 1;
	}
}


void	check_nx(char *cnt, unsigned char *bfdat, int k, unsigned char bd,
				 int pos, int NXY)    /* check negative X direction */
{
	int	ki;

	if( bd & 1 ) cnt[3] = 0;
	else
	{
	    ki = k*NXY + (pos-1);
	    if( bfdat[ki] > 43 && bfdat[ki] != EXT_VAL ) cnt[3] = 0;
	    else					 cnt[3] = 1;
	}
}
