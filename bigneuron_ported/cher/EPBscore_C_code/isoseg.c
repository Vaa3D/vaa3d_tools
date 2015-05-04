/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <MALLOC.h>
#include "my_ma.h"
#include "isoseg.h"
#include "burn_macro.h"
#include "burn_prototype.h"

//#include "../statistics/statistics_prototype.h"

extern	void	set_stencil(int, int, int *);
extern	void	two_int_heapsort(int, int *, int *);

void	initialize_convert_isolated_cluster(Iso_dat *isodat)
{
	Iso_dat	*pi;

	fprintf(stderr,"Isolated clusters of grain and/or pore voxels ");
	fprintf(stderr,"up to a specifed size\ncan be assumed to be ");
	fprintf(stderr,"misidentified and converted to the opposite ");
	fprintf(stderr,"material type.\n\n");

	fprintf(stdout,"Isolated clusters of grain and/or pore voxels ");
	fprintf(stdout,"up to a specifed size\ncan be assumed to be ");
	fprintf(stdout,"misidentified and converted to the opposite ");
	fprintf(stdout,"material type.\n\n");

	pi = isodat;
	set_max_iso_clust_size("grain",&(pi->fil_grain),&(pi->grain_min),
							&(pi->keep_max_grain));
	set_max_iso_clust_size("pore ",&(pi->fil_pore ),&( pi->pore_min),
							&( pi->keep_max_pore));

	fprintf(stderr,"\nIsolated clusters of grain and/or pore voxels ");
	fprintf(stderr,"greater than a specifed size\ncan also be converted ");
	fprintf(stderr,"to the opposite material type.\n\n");

	fprintf(stdout,"\nIsolated clusters of grain and/or pore voxels ");
	fprintf(stdout,"greater than a specifed size\ncan also be converted ");
	fprintf(stdout,"to the opposite material type.\n\n");

	set_min_iso_clust_size("grain",&(pi->fil_grain),&(pi->grain_max));
	set_min_iso_clust_size("pore ",&(pi->fil_pore ),&( pi->pore_max));

	fprintf(stderr,"\n");
	fprintf(stdout,"\n");
}


void	set_max_iso_clust_size(char *ctype, int *fil, int *threshold,
							   int *keep_largest)
{
	char	ans[4];

	fprintf(stderr,"Convert isolated %s clusters? (y,n): ",ctype);
	scanf("%s",ans);	getchar();

	fprintf(stdout,"Convert isolated %s clusters? (y,n): %s\n",ctype,ans);

	if( ans[0] == 'y' || ans[0] == 'Y' )
	{
	    *fil = 1;

	    fprintf(stderr,"Convert all but the largest %s cluster? ",ctype);
	    fprintf(stderr,"(y(dflt),n): ");
	    scanf("%s",ans);	getchar();

	    fprintf(stdout,"Convert all but the largest %s cluster? ",ctype);
	    fprintf(stdout,"(y(dflt),n): %s\n",ans);

	    if( ans[0] == 'n' )
	    {
		*keep_largest = 0;

		fprintf(stderr,"Enter maximum allowed size (number of voxels)");
		fprintf(stderr," for convertible isolated %s cluster: ",
								ctype);
		scanf("%d",threshold);		getchar();

		fprintf(stdout,"Enter maximum allowed size (number of voxels)");
		fprintf(stdout," for convertible isolated %s cluster: %d\n",
							ctype,*threshold);
	    }
	    else *keep_largest = 1;
	}
	else	*threshold = *fil = *keep_largest = 0;
	fprintf(stderr,"\n");
	fprintf(stdout,"\n");
}


void	set_min_iso_clust_size(char *ctype, int *fil, int *threshold)
{
	char	ans[4];

	fprintf(stderr,"Convert isolated %s clusters? (y,n): ",ctype);
	scanf("%s",ans);	getchar();

	fprintf(stdout,"Convert isolated %s clusters? (y,n): %s\n",ctype,ans);

	if( ans[0] == 'y' || ans[0] == 'Y' )
	{
		*fil = 1;
		fprintf(stderr,"Enter minimum allowed size (number of voxels)");
		fprintf(stderr," for convertible isolated %s cluster: ",ctype);
		scanf("%d",threshold);		getchar();

		fprintf(stdout,"Enter minimum allowed size (number of voxels)");
		fprintf(stdout," for convertible isolated %s cluster: %d\n",
							ctype,*threshold);
	}
	else	*threshold = -1;
	fprintf(stderr,"\n");
	fprintf(stdout,"\n");
}


void	convert_isolated_clusters(unsigned char *cube, int nx, int ny,
								  int nz, Iso_dat *isodat, char inv)
{
	Bchains	bc;

	int	nxy, nxyz, realloc_size, tmp;
	char	*phase;

	int	isz = sizeof(int);

	if( ! (isodat->fil_pore || isodat->fil_grain) ) return;

		/*   check if data has been inverted    */
		/* if so, grain/pore designators change */
	if( inv == 'y' )
	{
		tmp = isodat->fil_grain;
		isodat->fil_grain = isodat->fil_pore;
		isodat->fil_pore = tmp;

		tmp = isodat->grain_min;
		isodat->grain_min = isodat->pore_min;
		isodat->pore_min = tmp;

		tmp = isodat->grain_max;
		isodat->grain_max = isodat->pore_max;
		isodat->pore_max = tmp;

		tmp = isodat->keep_max_grain;
		isodat->keep_max_grain = isodat->keep_max_pore;
		isodat->keep_max_pore = tmp;
	}

	nxy = nx*ny;		nxyz = nxy*nz;

	realloc_size = bc.chain_size = 2*nxy;
	/***
	printf("Setting chain size in convert_isolated_clusters to %d\n",
							bc.chain_size);
	***/
	initialize_chains('o',&(bc.o_chain),&(bc.o_num),bc.chain_size);
	initialize_chains('n',&(bc.n_chain),&(bc.n_num),bc.chain_size);

	if( inv == 'y' ) phase = "grain";
	else		 phase = "pore";

	if( isodat->keep_max_pore )
	{
		set_largest_iso_clust_size(cube,&bc,nx,ny,nz,0,26,
				&(isodat->pore_min),realloc_size,phase);
	}
	if( isodat->fil_pore )
	{
		convert_phase(cube,&bc,nx,ny,nz,0,1,26,isodat->pore_min,
				isodat->pore_max,realloc_size,phase);
	}

	if( inv == 'y' ) phase = "pore";
	else		 phase = "grain";

	if( isodat->keep_max_grain )
	{
		set_largest_iso_clust_size(cube,&bc,nx,ny,nz,1,6,
				&(isodat->grain_min),realloc_size,phase);
	}
	if( isodat->fil_grain )
	{
		convert_phase(cube,&bc,nx,ny,nz,1,0,6,isodat->grain_min,
				isodat->grain_max,realloc_size,phase);
	}

	FREE(bc.o_chain,bc.chain_size*isz);
	FREE(bc.n_chain,bc.chain_size*isz);
}


/*
*	convert voxels of this_phase to other_phase using a burn algorithm
*	of either 6- or 26- connectivity
*/

void	set_largest_iso_clust_size(unsigned char *cube, Bchains *bc, int nx,
								   int ny, int nz, unsigned char this_phase,
								   int connectivity, int *threshold,
								   int realloc_size, char *ctype)
{
	unsigned char	bd_types;

	int	i, nxy, nxyz, burnt_voxels, tot_burnt_voxels = 0;
	int	isz = sizeof(int);
  
	nxy = nx*ny;		nxyz = nxy*nz;

	Set_unburned(this_phase)
	*threshold = 0;

	for( i = 0;  i < nxyz;  i++ )
	{
	    if( cube[i] == UNBURNED )
	    {
		cube[i] = BURNED;

		bc->o_chain[0] = i;
		bc->o_num = 1;
	  
		burnt_voxels = burn_phase(bc,cube,nx,ny,nz,nxy,connectivity,
							realloc_size,&bd_types);
		tot_burnt_voxels += burnt_voxels;

		if( burnt_voxels > *threshold ) *threshold = burnt_voxels;
	    }
	}
	
	burnt_voxels = tot_burnt_voxels;
	Set_cube(0,this_phase)

	fprintf(stderr,"\nLargest %s cluster is %d\n",ctype,*threshold);
	fprintf(stdout,"\nLargest %s cluster is %d\n",ctype,*threshold);

	*threshold -= 1;

	fprintf(stderr,"All %s clusters <= %d will be converted\n",
							ctype,*threshold);
	fprintf(stdout,"All %s clusters <= %d will be converted\n",
							ctype,*threshold);
}
  

void	convert_phase(unsigned char *cube, Bchains *bc, int nx, int ny,
					  int nz, unsigned char this_phase,
					  unsigned char other_phase, int connectivity,
					  int convert_min, int convert_max, int realloc_size,
					  char *ctype)
{
	char	*msg;
	unsigned char	bd_types;

	int	*num_cnvrt, *x_val, *y_val;
	int	val_sz, alloc_val, nv_used, delsz, sum;
	int	i, j, nxy, nxyz, burnt_voxels;
	int	isz = sizeof(int);
  
	nxy = nx*ny;		nxyz = nxy*nz;

	num_cnvrt = (int *)CALLOC(convert_min+1,isz);
	msg = "num_cnvrt in convert_phase()";
	if( MEMCHECK(num_cnvrt,msg,(convert_min+1)*isz) ) clean_up(0);

	nv_used = 0;
	val_sz = alloc_val = 100;
	x_val = (int *)MALLOC(val_sz*isz);
	msg = "x_val in convert_phase()";
	if( MEMCHECK(x_val,msg,val_sz*isz) ) clean_up(0);

	y_val = (int *)MALLOC(val_sz*isz);
	msg = "y_val in convert_phase()";
	if( MEMCHECK(y_val,msg,val_sz*isz) ) clean_up(0);

	Set_unburned(this_phase)

	if( convert_max < 0 ) convert_max = nxyz+1;

	for( i = 0;  i < nxyz;  i++ )
	{
	    if( cube[i] == UNBURNED )
	    {
		cube[i] = BURNED;

		bc->o_chain[0] = i;
		bc->o_num = 1;
	  
		burnt_voxels = burn_phase(bc,cube,nx,ny,nz,nxy,connectivity,
							realloc_size,&bd_types);
		if( burnt_voxels <= convert_min )
		{
			Set_cube(i,other_phase)
			num_cnvrt[burnt_voxels]++;
		}
		else if( burnt_voxels >= convert_max )
		{
			Set_cube(i,other_phase)
			for( j = 0;  j < nv_used;  j++ )
			{
				if( burnt_voxels == x_val[j] )
				{
					y_val[j] += 1;
					break;
				}
			}
			if( j == nv_used )
			{
				x_val[j] = burnt_voxels;
				y_val[j] = 1;
				nv_used += 1;
			}
			if( nv_used == val_sz )
			{
				val_sz += alloc_val;

				x_val = (int *)REALLOC(x_val,val_sz*isz);
				msg = "x_val realloc in convert_phase()";
				if( MEMCHECK(x_val,msg,alloc_val*isz) )
					clean_up(0);

				y_val = (int *)REALLOC(y_val,val_sz*isz);
				msg = "y_val realloc in convert_phase()";
				if( MEMCHECK(y_val,msg,alloc_val*isz) )
					clean_up(0);
			}
		}
		else if ( (burnt_voxels > convert_min) &&
			  (burnt_voxels < convert_max) )
		{
			Set_cube(i,this_phase)
		}
	    }
	}

	fprintf(stdout,"\nIsolated %s clusters converted\n",ctype);
	fprintf(stdout,"size\tnumber\n");
	for( i = 1;  i <= convert_min;  i++ )
	{
		if( num_cnvrt[i] == 0 ) continue;
		fprintf(stdout,"%d\t%d\n",i,num_cnvrt[i]);
	}
	fprintf(stdout,"\n");

	sum = 0;
	for( i = 0;  i < nv_used;  i++ ) sum += x_val[i]*y_val[i];

	if( nv_used < val_sz )
	{
		delsz = nv_used - val_sz;
		if( !nv_used )
		{
			FREE(x_val,val_sz*isz);
			FREE(y_val,val_sz*isz);
		}
		else
		{
			x_val = (int *)REALLOC(x_val,nv_used*isz);
			msg = "x_val realloc in convert_phase()";
			if( MEMCHECK(x_val,msg,delsz*isz) ) clean_up(0);

			y_val = (int *)REALLOC(y_val,nv_used*isz);
			msg = "y_val realloc in convert_phase()";
			if( MEMCHECK(y_val,msg,delsz*isz) ) clean_up(0);
		}
	}

	if( nv_used > 1 ) two_int_heapsort(nv_used,x_val,y_val);

	for( i = 0;  i < nv_used;  i++ )
		fprintf(stdout,"%d\t%d\n",x_val[i],y_val[i]);
	fprintf(stdout,"\n");

	FREE(num_cnvrt,(convert_min+1)*isz);
	if( nv_used )
	{
		FREE(x_val,nv_used*isz);
		FREE(y_val,nv_used*isz);
	}
}
  

void	process_disc_vols(unsigned char *cube, int nx, int ny, int nz,
						  void (*proc_func)(unsigned char *, Bchains *, int, int, int, unsigned char, int, int, char *, char *, char *),
						  char *proc_struct, char *ignore_exvol)
{
	Bchains	bc;

	int	nxy, nxyz, realloc_size;

	int	isz = sizeof(int);

	nxy = nx*ny;		nxyz = nxy*nz;

	realloc_size = bc.chain_size = 2*nxy;
	printf("Setting chain size in process_disc_vols() to %d\n",
							bc.chain_size);
	initialize_chains('o',&(bc.o_chain),&(bc.o_num),bc.chain_size);
	initialize_chains('n',&(bc.n_chain),&(bc.n_num),bc.chain_size);

		/* do `pore' phase */
	(*proc_func)(cube,&bc,nx,ny,nz,0,26,realloc_size,"pore",proc_struct,
								ignore_exvol);

		/* do `grain' phase */
	(*proc_func)(cube,&bc,nx,ny,nz,1,6,realloc_size,"grain",proc_struct,
								ignore_exvol);

	FREE(bc.o_chain,bc.chain_size*isz);
	FREE(bc.n_chain,bc.chain_size*isz);
}


void	count_voxels(char *msg, unsigned char *dat, int sz)
{
	unsigned char	type;

	fprintf(stderr,"\n%s\n",msg);
	fprintf(stdout,"\n%s\n",msg);

	fprintf(stderr,"\t%d total voxels\n",sz);
	fprintf(stdout,"\t%d total voxels\n",sz);

	type = EXT_VAL;		uc_echo_voxels_of_type(type,dat,sz);
	type = 1;		uc_echo_voxels_of_type(type,dat,sz);
	type = 0;		uc_echo_voxels_of_type(type,dat,sz);
	type = UNBURNED;	uc_echo_voxels_of_type(type,dat,sz);
	type = BURNED;		uc_echo_voxels_of_type(type,dat,sz);
}


#define Echo_voxels_of_type\
	int	i, sum;\
\
	sum = 0;\
	for( i = 0;  i < sz;  i++ )\
	{\
		if( dat[i] == type ) sum++;\
	}\
\
	fprintf(stderr,"\t%d voxels of type %d\n",sum,type);\
	fprintf(stdout,"\t%d voxels of type %d\n",sum,type)



void	uc_echo_voxels_of_type(unsigned char type, unsigned char *dat,
							   int sz)
{
	Echo_voxels_of_type;
}


void	us_echo_voxels_of_type(unsigned short type, unsigned short *dat,
							   int sz)
{
	Echo_voxels_of_type;
}


void	count_disc_phase_vol(unsigned char *cube, Bchains *bc, int nx,
							 int ny, int nz, unsigned char this_phase,
							 int connectivity, int realloc_size,
							 char *ctype, char *dv_struct,
							 char *ignore_exvol)
{
	void	(*check_bdry)(unsigned char *, int, int, int, int, int, int *),
			(*set_sten)(int, int, int *);

	Disc_vol *disc_vol;

	char	*msg;
	unsigned char	bd_types;

	int	*x_val, *y_val;
	int	val_sz, alloc_val, nv_used;
	int	ind, nxy, nxyz, burnt_voxels;
	int	*is_in, *sten;
	int	i, sum;
	int	delsz;

	int	isz = sizeof(int);

	disc_vol = (Disc_vol *)dv_struct;
  
	nxy = nx*ny;		nxyz = nxy*nz;

	val_sz = alloc_val = 100;

	x_val = (int *)MALLOC(val_sz*isz);
	msg = "x_val in count_disc_phase_vol()";
	if( MEMCHECK(x_val,msg,val_sz*isz) ) clean_up(0);

	y_val = (int *)MALLOC(val_sz*isz);
	msg = "y_val in count_disc_phase_vol()";
	if( MEMCHECK(y_val,msg,val_sz*isz) ) clean_up(0);

	is_in = (int *)MALLOC(connectivity*isz);
	sten  = (int *)MALLOC(connectivity*isz);

	if( connectivity == 6 )
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

	Set_unburned(this_phase)

	nv_used = 0;
	for( ind = 0;  ind < nxyz;  ind++ )
	{
	    if( cube[ind] == UNBURNED )
	    {
		cube[ind] = BURNED;
		bc->o_chain[0] = ind;
		bc->o_num = 1;
	  
		burnt_voxels = burn_phase(bc,cube,nx,ny,nz,nxy,connectivity,
							realloc_size,&bd_types);
		Set_cube(ind,this_phase)

		if( bd_types && (ignore_exvol[0] == 'y') ) continue;

		for( i = 0;  i < nv_used;  i++ )
		{
			if( burnt_voxels == x_val[i] )
			{
				y_val[i] += 1;
				break;
			}
		}
		if( i == nv_used )
		{
			x_val[i] = burnt_voxels;
			y_val[i] = 1;
			nv_used += 1;
		}
		if( nv_used == val_sz )
		{
			val_sz += alloc_val;

			x_val = (int *)REALLOC(x_val,val_sz*isz);
			msg = "x_val realloc in count_disc_phase_vol()";
			if( MEMCHECK(x_val,msg,alloc_val*isz) )
				clean_up(0);

			y_val = (int *)REALLOC(y_val,val_sz*isz);
			msg = "y_val realloc in count_disc_phase_vol()";
			if( MEMCHECK(y_val,msg,alloc_val*isz) )
				clean_up(0);
		}
	    }
	}

	sum = 0;
	for( i = 0;  i < nv_used;  i++ ) sum += x_val[i]*y_val[i];

	if( nv_used < val_sz )
	{
		delsz = nv_used - val_sz;
		x_val = (int *)REALLOC(x_val,nv_used*isz);
		msg = "x_val realloc in count_disc_phase_vol()";
		if( MEMCHECK(x_val,msg,delsz*isz) ) clean_up(0);

		y_val = (int *)REALLOC(y_val,nv_used*isz);
		msg = "y_val realloc in count_disc_phase_vol()";
		if( MEMCHECK(y_val,msg,delsz*isz) ) clean_up(0);
	}
	if( ctype[0] == 'p' )
	{
		disc_vol->p_val_sz = nv_used;
		disc_vol->pore_x_val = x_val;
		disc_vol->pore_y_val = y_val;
		disc_vol->p_vol = sum;
	}
	else
	{
		disc_vol->g_val_sz = nv_used;
		disc_vol->grain_x_val = x_val;
		disc_vol->grain_y_val = y_val;
		disc_vol->g_vol = sum;
	}
	free(is_in);		free(sten);
}


void	print_Disc_vol(Disc_vol *dv, char inv)
{
	if( inv != 'y' )
	{
		print_phase_dv("pore" ,dv->p_val_sz,dv->pore_x_val ,
					dv->pore_y_val ,dv->p_vol);
		print_phase_dv("grain",dv->g_val_sz,dv->grain_x_val,
					dv->grain_y_val,dv->g_vol);
	}
	else
	{
		print_phase_dv("grain",dv->p_val_sz,dv->pore_x_val ,
					dv->pore_y_val ,dv->p_vol);
		print_phase_dv("pore" ,dv->g_val_sz,dv->grain_x_val,
					dv->grain_y_val,dv->g_vol);
	}
}


void	print_phase_dv(char *ctype, int num, int *x, int *y, int vol)
{
	int	i;

	fprintf(stdout,"\ndisconnected %s volumes\n",ctype);
	fprintf(stdout,"total volume %d\n",vol);
	fprintf(stdout,"size\tnumber\n");
	for( i = 0;  i < num;  i++ ) fprintf(stdout,"%d\t%d\n",x[i],y[i]);
	fprintf(stdout,"\n");
}


void	free_Disc_vol(Disc_vol *dv)
{
	int	isz = sizeof(int);

	FREE(dv->pore_x_val ,dv->p_val_sz*isz);
	FREE(dv->pore_y_val ,dv->p_val_sz*isz);
	FREE(dv->grain_x_val,dv->g_val_sz*isz);
	FREE(dv->grain_y_val,dv->g_val_sz*isz);
}


int	burn_phase(Bchains *bc, unsigned char *cube, int nx, int ny, int nz,
			   int nxy, int sten_sz, int realloc_size,
			   unsigned char *bd_types)
{
	void	(*check_bdry)(unsigned char *, int, int, int, int, int, int *),
			(*set_sten)(int, int, int *);

	unsigned char    edge;

	int	i, end, pos, burnt_voxels;
	int	*is_in, *sten;

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

	*bd_types = 0;
	burnt_voxels = 1;	end = 0;
	while( !end )
	{
	    for( i = 0;  i < bc->o_num;  i++ )
	    {
		pos = bc->o_chain[i];

		(*check_bdry)(&edge,pos,nx,ny,nz,nxy,is_in);
		if( edge ) *bd_types |= edge;	/* accum edge info */

		burn_phase_step(bc,cube,pos,realloc_size,edge,sten_sz,sten,
								is_in,bd_types);

	    }
	    for( i = 0;  i < bc->n_num;  i++ ) bc->o_chain[i] = bc->n_chain[i];

	    bc->o_num = bc->n_num;
	    bc->n_num = 0;	   

	    burnt_voxels += bc->o_num;
	    if( bc->o_num == 0 ) end = 1;
	}

	free(is_in);		free(sten);

	return burnt_voxels;
}


void	set_6_stencil(int nx, int nxy, int *sten)
{
			sten[5] =  nxy;

			sten[3] =   nx;
	sten[0] = -1;			sten[1] = 1;
			sten[2] =  -nx;

			sten[4] = -nxy;
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


#define	Burn_nbr()\
{\
	pos2 = pos1 + sten[i];\
	if( cube[pos2] == UNBURNED )\
	{\
		add_to_chain(bc,pos2,realloc_size);\
		cube[pos2] = BURNED;\
	}\
	else if( cube[pos2] == EXT_VAL ) *bd_types |= FP_SIDE;\
}


void	burn_phase_step(Bchains *bc, unsigned char *cube, int pos1,
						int realloc_size, unsigned char edge, int sten_sz,
						int *sten, int *is_in, unsigned char *bd_types)
{
	int	i, pos2;

	if( edge )
	{
		for( i = 0;  i < sten_sz;  i++ )
		{
			if( is_in[i] ) Burn_nbr()
		}
	}
	else
	{
		for( i = 0;  i < sten_sz;  i++ ) Burn_nbr()
	}
}
