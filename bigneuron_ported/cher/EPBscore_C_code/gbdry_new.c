
/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <math.h>
#include "my_ma.h"
#include "ma_prototype.h"




void	set_bdry_smoothing(int *smooth_bdry)
{
	printf("\n");
	printf("The material/void boundary can be lightly smoothed.\n");
	printf("Available options are\n");
	printf("\t0) no conversion\n");
	printf("or convert those boundary voxels having\n");
	printf("\t1) exactly one neighbor of the same type\n");
	printf("\t2) less than a majority of neighbors of the same ");
	printf("type\n");
	printf("Enter choice (0(dflt),1,2): ");
	scanf("%d",smooth_bdry);	getchar();
	printf("\n");

	printf("\n");
	printf("The material/void boundary can be lightly smoothed.\n");
	printf("Available options are\n");
	printf("\t0) no conversion\n");
	printf("or convert those boundary voxels having\n");
	printf("\t1) exactly one neighbor of the same type\n");
	printf("\t2) less than a majority of neighbors of the same ");
	printf("type\n");
	printf("Enter choice (0(dflt),1,2): %d\n\n",*smooth_bdry);
}


/*
*	This routine smooths the phase 0/phase 1 boundary by identifying all
*	this_phase voxels that have exactly one this_phase neighbor and 
*	setting them to other_phase. The calling routine can call for either
*	either 6- or 26-neighbor connectivity. A non-iterative, parallel
*	(ie. direction independent) sweep through the mesh is performed.
*/


#define	Check_nbr(_phase)\
{\
	nind = ind + sten[i];\
	if( cube[nind] == _phase ) num_nbrs++;\
}


void	smooth_gbdry_1(unsigned char *cube, int nx, int ny, int nz, int nxy,
					   int nxyz, int sten_sz, unsigned char this_phase,
					   unsigned char other_phase)
{
	void	(*check_bdry)(unsigned char *, int, int, int, int, int, int *),
			(*set_sten)(int, int, int *);

	unsigned char    is_edge;

	char	*msg;

	int	*is_in, *sten, *cnvt_list;
	int	ind, nind, i;
	int	num_nbrs;
	int	realloc_sz, cnvt_sz, cnvt_ind;

	int	isz = sizeof(int);

	is_in = (int *)MALLOC(sten_sz*isz);
	sten  = (int *)MALLOC(sten_sz*isz);

	if( sten_sz == 6 )
	{
		set_sten = set_6_stencil;
		check_bdry = check_bdry_6_nbr;
	}
	else
	{
		set_sten = set_stencil;
		check_bdry = check_bdry_26_nbr;
	}

	(*set_sten)(nx,nxy,sten);

	cnvt_sz = realloc_sz = nx;
	cnvt_list = (int *)MALLOC(cnvt_sz*isz);
	msg = "cnvt_list in smooth_gbdry_1()";
	if( MEMCHECK(cnvt_list,msg,cnvt_sz*isz) ) clean_up(0);

	cnvt_ind = 0;
	for( ind = 0;  ind < nxyz;  ind++ )
	{
		if( cube[ind] != this_phase ) continue;

		(*check_bdry)(&is_edge,ind,nx,ny,nz,nxy,is_in);

		num_nbrs = 0;
		if( is_edge )
		{
		    for( i = 0;  i < sten_sz;  i++ )
		    {
			if( is_in[i] ) Check_nbr(this_phase)
		    }
		}
		else
		{
		    for( i = 0;  i < sten_sz;  i++ ) Check_nbr(this_phase)
		}
		if( num_nbrs == 1 )
		{
		    if( cnvt_ind == cnvt_sz )
		    {
			cnvt_sz += realloc_sz;
			cnvt_list = (int *)REALLOC(cnvt_list,cnvt_sz*isz);
			msg = "cnvt_list realloc in smooth_gbdry_1()";
			if( MEMCHECK(cnvt_list,msg,realloc_sz*isz) ) clean_up(0);
		    }
		    cnvt_list[cnvt_ind] = ind;
		    cnvt_ind++;
		}
	}

	printf("\nIn smooth_gbdy(), %d %d-bdry voxels converted to %d\n",
					cnvt_ind,this_phase,other_phase);
	printf("\nIn smooth_gbdy(), %d %d-bdry voxels converted to %d\n",
					cnvt_ind,this_phase,other_phase);

	for( i = 0;  i < cnvt_ind;  i++ )
	{
		cube[cnvt_list[i]] = other_phase;
	}

	free(is_in);		free(sten);
	FREE(cnvt_list,cnvt_sz*isz);
}


/*
*	This routine smooths the phase 0/phase 1 boundary by identifying all
*	this_phase voxels that have more than (sten/2+1) other_phase neighbors
*	and setting them to other_phase. The calling routine can call for either
*	either 6- or 26-neighbor connectivity. A non-iterative, parallel (ie.
*	direction independent) sweep through the mesh is performed.
*/


void	smooth_gbdry_2(unsigned char *cube, int nx, int ny, int nz, int nxy,
					   int nxyz, int sten_sz, unsigned char this_phase,
					   unsigned char other_phase)
{
	void	(*check_bdry)(unsigned char *, int, int, int, int, int, int *),
			(*set_sten)(int, int, int *);

	unsigned char    is_edge;

	char	*msg;

	int	*is_in, *sten, *cnvt_list;
	int	ind, nind, i;
	int	num_nbrs, thresh;
	int	realloc_sz, cnvt_sz, cnvt_ind;

	int	isz = sizeof(int);

	is_in = (int *)MALLOC(sten_sz*isz);
	sten  = (int *)MALLOC(sten_sz*isz);

	if( sten_sz == 6 )
	{
		set_sten = set_6_stencil;
		check_bdry = check_bdry_6_nbr;
	}
	else
	{
		set_sten = set_stencil;
		check_bdry = check_bdry_26_nbr;
	}

	(*set_sten)(nx,nxy,sten);

	cnvt_sz = realloc_sz = nx;
	cnvt_list = (int *)MALLOC(cnvt_sz*isz);
	msg = "cnvt_list in smooth_gbdry_1()";
	if( MEMCHECK(cnvt_list,msg,cnvt_sz*isz) ) clean_up(0);

	cnvt_ind = 0;
	for( ind = 0;  ind < nxyz;  ind++ )
	{
		if( cube[ind] != this_phase ) continue;

		(*check_bdry)(&is_edge,ind,nx,ny,nz,nxy,is_in);

		num_nbrs = 0;
		if( is_edge )
		{
		    thresh = 0;
		    for( i = 0;  i < sten_sz;  i++ )
		    {
			if( is_in[i] )
			{
				Check_nbr(other_phase)
				thresh++;
			}
		    }
		    thresh = thresh/2;
		}
		else
		{
		    for( i = 0;  i < sten_sz;  i++ ) Check_nbr(other_phase)
		    thresh = sten_sz/2;
		}
		if( num_nbrs > thresh )
		{
		    if( cnvt_ind == cnvt_sz )
		    {
			cnvt_sz += realloc_sz;
			cnvt_list = (int *)REALLOC(cnvt_list,cnvt_sz*isz);
			msg = "cnvt_list realloc in smooth_gbdry_1()";
			if( MEMCHECK(cnvt_list,msg,realloc_sz*isz) ) clean_up(0);
		    }
		    cnvt_list[cnvt_ind] = ind;
		    cnvt_ind++;
		}
	}

	printf("\nIn smooth_gbdy(), %d %d-bdry voxels converted to %d\n",
					cnvt_ind,this_phase,other_phase);
	printf("\nIn smooth_gbdy(), %d %d-bdry voxels converted to %d\n",
					cnvt_ind,this_phase,other_phase);

	for( i = 0;  i < cnvt_ind;  i++ )
	{
		cube[cnvt_list[i]] = other_phase;
	}

	FREE(is_in,sten_sz*isz);		FREE(sten,sten_sz);
	FREE(cnvt_list,cnvt_sz*isz);
}


void	create_gbdry(Bchains *bc, Fdat *tmp,
					 int *g_num, int nx, int ny, int nz,
					 int task, int *psur)
{
	int	sten6[6], sten26[26];
	int	i, j, k, ki;
	int	pos;
	int	bdry_num;
	int	tot_num;
	int	dbg;
	int	num_pg_faces = 0;
	int	nxy, nxyz;

	int	isz = sizeof(int);

	nxy = nx*ny;	nxyz = nxy*nz;

	set_stencil(nx,nxy,sten26);
	set_sten_6(nx,nxy,sten6);

	dbg = 0;

	bdry_num = 0;
	for( k = 0;  k < nz;  k++ )
	{
		tot_num = 0;		ki = k*nxy;
		/* seg file data, grain = 1 */
		if( (task == ERODE) || (task == NO_OPT) )
		{
			for( j = 0;  j < ny;  j++ )
	        {
				pos = ki + j*nx;
				for( i = 0;  i < nx;  i++, pos++ )
				{
					if( tmp->fdat[pos] == 1 )
					{
						check_if_gbdry(tmp,nx,ny,nz,i,j,k,pos,&bdry_num,
									&num_pg_faces,sten6,sten26,task);
						tot_num++;
					}
				}
			}
		}
		else			/* burn file data, GRAIN = 254 or 0 */
		{
			for( j = 0;  j < ny;  j++ )
			{
				pos = j*nx + ki;
				for( i = 0;  i < nx;  i++, pos++ )
				{
					if( (tmp->fdat[pos] == 254) || (tmp->fdat[pos] == 0) )
					{
						check_if_gbdry(tmp,nx,ny,nz,i,j,k,pos,&bdry_num,
									&num_pg_faces,sten6,sten26,task);
						tot_num++;
					}
				}
			}
		}
		(*g_num) += tot_num;
	}

	*psur = num_pg_faces;

	dbg = 0;

	set_first_chain(bc,tmp->cube,bdry_num,nxyz); 
}


/* 
*	Check if grain voxel on grain boundary
*/


#define Check_26_bdry(is,ie) \
	for( i = is;  i < ie;  i++ )\
	{\
		cind = pos + sten26[i];\
		if( fdat[cind] == PORE_val ) { is_bv = 1;	break; } \
	}


#define Count_surf_faces(is,ie) \
	for( i = is;  i < ie;  i++ )\
	{\
		cind = pos+sten6[i];	\
		if( fdat[cind] == PORE_val ) (*num_pg_faces)++; \
	}


void	check_if_gbdry(Fdat *tmp, int nx, int ny, int nz, int ix, int iy,
					   int iz, int pos, int *bdry_num, int *num_pg_faces,
					   int *sten6, int *sten26, int task)
{
	unsigned char	*fdat = tmp->fdat;

	int	is_bv;
	int	i, cind;
	int	num_face;
	int	PORE_val;

		/* burn file data, first layer of pore voxels have value 1 */
		/*  seg file data, pore voxels have value 0 */

	PORE_val = ( task == MED_AX ) ? 1 : 0;

	is_bv = 0;
	if( iz == 0 )
	{
	   if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
		check_bdry_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,&is_bv,sten26,
								PORE_val);
	   /** else if( iz == nz-1 ) Check_26_bdry(9,17) **/
	   else			 Check_26_bdry(9,26)
	}
	else if( iz != nz-1 )
	{
	   if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
		check_bdry_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,&is_bv,sten26,
								PORE_val);
	   else Check_26_bdry(0,26)
	}
	else
	{
	   if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
		check_bdry_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,&is_bv,sten26,
								PORE_val);
	   else Check_26_bdry(0,17)
	}

	if( is_bv )		/* If boundary voxel, increment */
			/* boundary number & specific surface */
	{
		if( (task == ERODE) || (task == NO_OPT) )  fdat[pos] = 126;
	    tmp->cube[pos] = 1;
	    (*bdry_num)++;

	    if( iz == 0 )
	    {
			if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
			{
				count_surf_faces_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,
						 &num_face,sten6,PORE_val);
				(*num_pg_faces) += num_face;
			}
			/** else if( iz == nz-1 ) Count_surf_faces(1,5) **/
			else			 Count_surf_faces(1,6)
		}
	    else if( iz != nz-1 )
	    {
			if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
			{
				count_surf_faces_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,
						 &num_face,sten6,PORE_val);
				(*num_pg_faces) += num_face;
			}
			else Count_surf_faces(0,6)
		}
		else
		{
			if( ix == 0 || ix == nx-1 || iy == 0 || iy == ny-1 )
			{
				count_surf_faces_at_edge(fdat,nx,ny,nz,ix,iy,iz,pos,
						 &num_face,sten6,PORE_val);
				(*num_pg_faces) += num_face;
			}
			else Count_surf_faces(0,5)
		}
	}
}


#define	Check_bdry(dir) \
	cind = pos + sten26[dir];\
	if( fdat[cind] == PORE_val ) is_bv = 1


void	check_bdry_at_edge(unsigned char *fdat, int nx, int ny, int nz,
						   int ix, int iy, int iz, int pos, int *pis_bv,
						   int *sten26, int PORE_val)
{
	int	cind;
	int	is_bv;

	is_bv = 0;

	if( ix == 0 && iy == 0 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(4); Check_bdry(5);
			Check_bdry(7); Check_bdry(8);
	    }

	    Check_bdry(13); Check_bdry(15); Check_bdry(16);

	    if( iz != nz-1 )
	    {
			Check_bdry(21);	Check_bdry(22);
			Check_bdry(24);	Check_bdry(25);
	    }
	}
	else if( ix == nx-1 && iy == 0 )
	{
	    if( iz != 0 )
		{
			Check_bdry(3); Check_bdry(4);
			Check_bdry(6); Check_bdry(7);
	    }

	    Check_bdry(12); Check_bdry(14); Check_bdry(15);

	    if( iz != nz-1 )
	    {
			Check_bdry(20);	Check_bdry(21);
			Check_bdry(23);	Check_bdry(24);
	    }
	}
	else if( ix == nx-1 && iy == ny-1 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(0); Check_bdry(1);
			Check_bdry(3); Check_bdry(4);
	    }

	    Check_bdry(9); Check_bdry(10); Check_bdry(12);

	    if( iz != nz-1 )
	    {
			Check_bdry(17);	Check_bdry(18);
			Check_bdry(20);	Check_bdry(21);
	    }
	}
	else if( ix == 0 && iy == ny-1 )
	{
	    if( iz != 0 )
		{
			Check_bdry(1); Check_bdry(2);
			Check_bdry(4); Check_bdry(5);
		}

	    Check_bdry(10); Check_bdry(11); Check_bdry(13);

	    if( iz != nz-1 )
	    {
			Check_bdry(18); Check_bdry(19);
			Check_bdry(21); Check_bdry(22);
	    }
	}
	else if( ix == 0 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(1); Check_bdry(2); Check_bdry(4);
			Check_bdry(5); Check_bdry(7); Check_bdry(8);
	    }

	    Check_bdry(10); Check_bdry(11); Check_bdry(13);
	    Check_bdry(15); Check_bdry(16);

	    if( iz != nz-1 )
	    {
			Check_bdry(18); Check_bdry(19);
			Check_bdry(21); Check_bdry(22);
			Check_bdry(24); Check_bdry(25);
	    }
	}
	else if( iy == 0 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(3); Check_bdry(4);
			Check_bdry(5); Check_bdry(6);
			Check_bdry(7); Check_bdry(8);
	    }

	    Check_bdry(12); Check_bdry(13); Check_bdry(14);
	    Check_bdry(15); Check_bdry(16);

	    if( iz != nz-1 )
	    {
			Check_bdry(20); Check_bdry(21); Check_bdry(22);
			Check_bdry(23); Check_bdry(24); Check_bdry(25);
	    }
	}
	else if( ix == nx-1 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(0); Check_bdry(1); Check_bdry(3);
			Check_bdry(4); Check_bdry(6); Check_bdry(7);
	    }

	    Check_bdry( 9); Check_bdry(10); Check_bdry(12);
	    Check_bdry(14); Check_bdry(15);

	    if( iz != nz-1 )
	    {
			Check_bdry(17); Check_bdry(18); Check_bdry(20);
			Check_bdry(21); Check_bdry(23); Check_bdry(24);
	    }
	}
	else if( iy == ny-1 )
	{
	    if( iz != 0 )
	    {
			Check_bdry(0); Check_bdry(1); Check_bdry(2);
			Check_bdry(3); Check_bdry(4); Check_bdry(5);
	    }

	    Check_bdry( 9); Check_bdry(10); Check_bdry(11);
	    Check_bdry(12); Check_bdry(13);

	    if( iz != nz-1 )
	    {
			Check_bdry(17); Check_bdry(18); Check_bdry(19);
			Check_bdry(20); Check_bdry(21); Check_bdry(22);
	    }
	}
	*pis_bv = is_bv;
}


#define	Check_surf(dir)\
	cind = pos + sten6[dir];\
	if( fdat[cind] == PORE_val ) num_face += 1


void	count_surf_faces_at_edge(unsigned char  *fdat, int nx, int ny,
								 int nz, int ix, int iy, int iz, int pos,
								 int *vox_num_pg_face, int *sten6,
								 int PORE_val)
{
	int	cind, num_face;

	num_face = 0;

	if( ix == 0 && iy == 0 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(3);
					Check_surf(4);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( ix == nx-1 && iy == 0 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(2);
					Check_surf(4);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( ix == nx-1 && iy == ny-1 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(1);
					Check_surf(2);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( ix == 0 && iy == ny-1 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(1);
					Check_surf(3);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( ix == 0 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(1);
					Check_surf(3);
					Check_surf(4);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( iy == 0 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(2);
					Check_surf(3);
					Check_surf(4);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( ix == nx-1 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(1);
					Check_surf(2);
					Check_surf(4);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	else if( iy == ny-1 )
	{
		if( iz != 0    ) {	Check_surf(0); }
					Check_surf(1);
					Check_surf(2);
					Check_surf(3);
		if( iz != nz-1 ) {	Check_surf(5); }
	}
	*vox_num_pg_face = num_face;
}
void	set_6_stencil(int nx, int nxy, int *sten)
{
			sten[5] =  nxy;

			sten[3] =   nx;
	sten[0] = -1;			sten[1] = 1;
			sten[2] =  -nx;

			sten[4] = -nxy;
}
void    set_sten_6(int nx, int nxy, int *sten6)
{
	int     nxm1, nxp1;

	nxm1 = nx-1;            nxp1 = nx+1;

	sten6[0] = -nxy;	sten6[1] = -nx;
	sten6[2] = -1;		sten6[3] =  1;
        sten6[4] =  nx;		sten6[5] =  nxy;
}

void	check_bdry_6_nbr(unsigned char *pbd, int pos, int nx, int ny,
						 int nz, int nxy, int *is_in)
{
	unsigned char	bd;

	int	i;
	int	ind, ix, iy, iz;

	for( i = 0;  i < 6;  i++ ) is_in[i] = 1;


	ind = pos;		iz = ind/nxy;
	ind = ind%nxy;		iy = ind/nx;
				ix = ind%nx;

	/* Note: if() else if() not desired in case nz = 1, ny = 1, nx = 1 */

	bd = 0;

	if( iz == nz-1 ) { bd |= ZU_SIDE;    is_in[5] = 0; }
	if( iz ==    0 ) { bd |= ZL_SIDE;    is_in[4] = 0; }

	if( iy == ny-1 ) { bd |= YU_SIDE;    is_in[3] = 0; }
	if( iy ==    0 ) { bd |= YL_SIDE;    is_in[2] = 0; }

	if( ix == nx-1 ) { bd |= XU_SIDE;    is_in[1] = 0; }
	if( ix ==    0 ) { bd |= XL_SIDE;    is_in[0] = 0; }

	*pbd = bd;
}
void	check_bdry_26_nbr(unsigned char *pbd, int pos, int nx, int ny,
						  int nz, int nxy, int *is_in)
{
	unsigned char	bd;

	int	i;
	int	ind, ix, iy, iz;

	for( i = 0;  i < 26;  i++ ) is_in[i] = 1;

	bd = 0;

	ind = pos;		iz = ind/nxy;
	ind = ind%nxy;		iy = ind/nx;
				ix = ind%nx;

	/* Note: if() else if() not desired in case nz = 1, ny = 1, nx = 1 */

	if( iz == nz-1 )
	{
		bd |= ZU_SIDE;
		for( i = 17;  i < 26;  i++ ) is_in[i] = 0;
	}
	if( iz == 0  )
	{
		bd |= ZL_SIDE;
		for( i =  0;  i <  9;  i++ ) is_in[i] = 0;
	}

	if( iy == ny-1 )
	{
		bd |= YU_SIDE;
		for( i =  6;  i <  9;  i++ ) is_in[i] = 0;
		for( i = 14;  i < 17;  i++ ) is_in[i] = 0;
		for( i = 23;  i < 26;  i++ ) is_in[i] = 0;
	}
	if( iy == 0 )
	{
		bd |= YL_SIDE;
		for( i =  0;  i <  3;  i++ ) is_in[i] = 0;
		for( i =  9;  i < 12;  i++ ) is_in[i] = 0;
		for( i = 17;  i < 20;  i++ ) is_in[i] = 0;
	}

	if( ix == nx-1 )
	{
		bd |= XU_SIDE;
		for( i =  2;  i < 12;  i += 3 ) is_in[i] = 0;
		for( i = 13;  i < 26;  i += 3 ) is_in[i] = 0;
	}
	if( ix == 0 )
	{
		bd |= XL_SIDE;
		for( i =  0;  i < 13;  i += 3 ) is_in[i] = 0;
		for( i = 14;  i < 26;  i += 3 ) is_in[i] = 0;
	}

	*pbd = bd;
}
void	set_stencil(int nx, int nxy, int *sten)
{
	int	nxm1, nxp1;

	nxm1 = nx-1;		nxp1 = nx+1;

	sten[ 0] = -nxy-nxp1;  sten[ 1] = -nxy-nx;  sten[ 2] = -nxy-nxm1;
	sten[ 3] = -nxy-1;     sten[ 4] = -nxy;     sten[ 5] = -nxy+1;
	sten[ 6] = -nxy+nxm1;  sten[ 7] = -nxy+nx;  sten[ 8] = -nxy+nxp1;
	sten[ 9] = -nxp1;      sten[10] = -nx;      sten[11] = -nxm1;
	sten[12] = -1;                              sten[13] =  1;
	sten[14] =  nxm1;      sten[15] =  nx;      sten[16] =  nxp1;
	sten[17] =  nxy-nxp1;  sten[18] =  nxy-nx;  sten[19] =  nxy-nxm1;
	sten[20] =  nxy-1;     sten[21] =  nxy;     sten[22] =  nxy+1;
	sten[23] =  nxy+nxm1;  sten[24] =  nxy+nx;  sten[25] =  nxy+nxp1;
}
