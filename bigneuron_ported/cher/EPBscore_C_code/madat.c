/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "burn_macro.h"
#include "maset.h"
#include "lkc_prototype.h"
#include "min_surf_prototype.h"
#include "ma_prototype.h"

extern	int	Interface;
extern	char	**guiParam;
extern	int	n_guiParam, guiParam_counter;


#ifdef TST
void	input_pma_data_arrays_2(int *Xs, int *Xe, int *Ys, int *Ye, int *Zs,
								int *Ze, int *Ts, int *Te, int *trim,
								char **mafn_base, char **mafn_name,
								char *ext, char **brnfn_base,
								char **brnfn_name, int *mafn_len,
								int *brnfn_len, int *is_cmprss,
								int *brn_cmprss, int hide_rsz,
								char *file_basename)
{
	char	trim_ans[5];
	int	nx, ny, fzs, fze, dum;
	int	numeric;

	set_ma_filenames(file_basename,"ma",mafn_base,mafn_name,&numeric,ext,
							mafn_len,is_cmprss);
	*Ts = *Te = -1;
	if( numeric )
	{
		get_file_indices("time step",*mafn_name,ext,*mafn_len,Ts,Te);
		set_time_series(Ts,Te);
	}
	else
	{
		/* assume detected timesteps 1 and 1; GUI-easy reason */
		*Ts = *Te =  1;
		set_time_series(Ts,Te);
		*Ts = *Te = -1;
	}

	get_nx_ny(*Ts,*mafn_name,ext,*mafn_len,*is_cmprss,&nx,&ny,&fzs,&fze,
								&dum,'b');

	if( hide_rsz )	set_resize_info(Xs,Xe,Ys,Ye,Zs,Ze,nx,ny,fzs,fze);
	else
	{
		if( ask_rsz() )
		{
			select_resize_info(Xs,Xe,Ys,Ye,Zs,Ze);

			if( (*Zs < fzs) || (*Ze > fze) )
			{
				fprintf(stderr,
				    "chosen z range outside proper range\n");
				fprintf(stdout,
				    "chosen z range outside proper range\n");
				clean_up(0);
			}
		}
		else	set_resize_info(Xs,Xe,Ys,Ye,Zs,Ze,nx,ny,fzs,fze);
	}

	*trim = 0;
	if( Interface == GUI_NONE )
	{
		fprintf(stderr,
			"Trim medial structure voxels on volume edges? ");
		fprintf(stderr,"(y,n(dflt)): ");	gets(trim_ans);
		fprintf(stdout,
			"Trim medial structure voxels on volume edges? ");
		fprintf(stdout,"(y,n(dflt)): %s\n",trim_ans);

		*trim = ( trim_ans[0] == 'y' ) ? 1 : 0;
	}
	else if( n_guiParam - guiParam_counter > 0 )
	{
		*trim = ( strcmp(guiParam[guiParam_counter],"y") )? 0 : 1;
		guiParam_counter++;
	}

	set_in_filenames(file_basename,"burn",brnfn_base,brnfn_name,&numeric,
						"brn",brnfn_len,brn_cmprss);
}
#endif


//void	set_pma_data_arrays(int Xs, int Xe, int Ys, int Ye, int Zs, int Ze,
//							int ts, int trim, char *mafn_base,
//							char *mafn_name, char *ext, char *brnfn_base,
//							char *brnfn_name, int mafn_len, int brnfn_len,
//							int is_cmprss, int brn_cmprss, int *pnx,
//							int *pnxy, int *pnxyz, int *ptot_ma,
//							int **pma_ind, unsigned char **pbdat,
//							int **pcube)
void	set_pma_data_arrays(int nx, int ny, int nz, int trim,
							int Nx, int Nxy, int Nxyz,
							int *ptot_ma, int **pma_ind,
							unsigned char **pbdat, int **pcube,
							unsigned char *burn_cube)
{
	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

		/* Create a data-cube padded on all 6 surfaces */
		/* This allows indices i-1 and i+1 as long as  */
		/* i is restriced to the interior */

	*pcube = (int *)CALLOC(Nxyz,isz);
	if( MEMCHECK(*pcube,"*pcube in input_pma_data_arrays()",Nxyz*isz) )
		clean_up(0);

	set_cube_info_from_burn_files(1,nx,1,ny,1,nz,Nx,Nxy,Nxyz,
		*pcube,isz,burn_cube);

	resize_ma_to_cube(pma_ind,pbdat,ptot_ma,nx,ny,0,nx-1,0,ny-1);
	set_cube_ma(*pcube,*pma_ind,*ptot_ma,99);

	if( trim )
	{
		correct_ma_for_trim(pma_ind,pbdat,ptot_ma,*pcube,Nx,Nxy,Nxyz);
	}
}


#ifdef TST
void	accum_pma_dat(char *infile_name, char *ext, int infile_len,
					  int in_cmprss, int zs, int ze, int Nx, int Ny,
					  int **pma_ind, unsigned char **pbdat, int *tot_pma,
					  int fzs, int ts)
{
	FILE	*ifp;
	char	uncmp_fn[256];
	int	swap;

	unsigned char	*tmp_brn;

	int	*tmp_ind;

	int	n, i, j;
	int	old_tot, nvox;
	int	first;
	int	Nxy, slc_ind;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	if( ts > 0 )
	{
		ts--;	incfname(infile_name,infile_len,&ts);
	}
	else	infile_name[infile_len] = '\0';
	strcat(infile_name,".");

	ifp = open_in_file(infile_name,ext,uncmp_fn,in_cmprss,"r");
	infile_name[infile_len+1] = '\0';

	read_ma_header(ifp,Nx,Ny,&swap);

	for( n = fzs;  n < zs;  n++ )
	{
	    read_ma_fp(ifp,Nx,Ny,&nvox,&tmp_ind,&tmp_brn,swap);
	    if( nvox > 0 )
	    {
			FREE(tmp_ind,nvox*isz);    /*  allocated in   */
			FREE(tmp_brn,nvox*usz);    /* read_ma_file() */
	    }
	}

	Nxy = Nx*Ny;		slc_ind = 0;
	*tot_pma = 0;
	first = 1;
	for( n = zs;  n <= ze;  n++, slc_ind += Nxy )
	{
	    read_ma_fp(ifp,Nx,Ny,&nvox,&tmp_ind,&tmp_brn,swap);

	    old_tot = *tot_pma;
	    (*tot_pma) += nvox;

	    if( nvox > 0 )
	    {
		if( first )
		{
		    first = 0;

		    *pma_ind = tmp_ind;	    /*  allocated in   */
		    *pbdat   = tmp_brn;	    /* read_ma_file() */
		    for( j = 0;  j < nvox;  j++ )
			(*pma_ind)[j] = (*pma_ind)[j] + slc_ind;
		}
		else
		{
		    ma_data_realloc(pma_ind,pbdat,*tot_pma,nvox);

		    for( j = 0, i = old_tot;  j < nvox;  j++, i++ )
		    {
			(*pma_ind)[i] = tmp_ind[j] + slc_ind;
			(*pbdat  )[i] = tmp_brn[j];
		    }

		    FREE(tmp_ind,nvox*isz);    /*  allocated in   */
		    FREE(tmp_brn,nvox*usz);    /* read_ma_file() */
		}
	    }
	}
	fprintf(stderr,"%d ma voxels read in\n",*tot_pma);
	//PRINT_O("%d ma voxels read in\n",*tot_pma);

	close_in_file(ifp,uncmp_fn,in_cmprss);

}
#endif


void	resize_ma_to_cube(int **pma_ind, unsigned char **pbdat,
						  int *ptot_ma, int Nx, int Ny, int Xs, int Xe,
						  int Ys, int Ye)
{
	unsigned char	btmp;

	int	i, j, k, ind, ns, ne, dX, dY, dXY;
	int	Nxy = Nx*Ny;
	int	mtmp;
	int	tot_ma_sav, nvox;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	ns = 0;		ne = (*ptot_ma) - 1;

	tot_ma_sav = *ptot_ma;

	dX = Xe-Xs+3;	dY = Ye-Ys+3;	dXY = dX*dY;
	while( ns <= ne )
	{
		ind = (*pma_ind)[ns];
		k = ind/Nxy;	mtmp =  ind - k*Nxy;
		j = mtmp/Nx;	   i = mtmp - j*Nx;

		if( (i < Xs) || (i > Xe) || (j < Ys) || (j > Ye) )
		{
			mtmp = (*pma_ind)[ne];	btmp = (*pbdat)[ne];
			(*pma_ind)[ne] = ind;	(*pbdat)[ne] = (*pbdat)[ns];
			(*pma_ind)[ns] = mtmp;	(*pbdat)[ns] = btmp;

			ne--;
		}
		else
		{
			i = i-Xs+1;	j = j-Ys+1;	k = k+1;
			(*pma_ind)[ns] = i + j*dX + k*dXY;
			ns++;
		}
	}

	fprintf(stderr,"%d ma voxels after resize\n",ns);
	////PRINT_O("%d ma voxels after resize\n",ns);

	if( *ptot_ma != ns )
	{
		*ptot_ma = ns;

		nvox = (*ptot_ma) - tot_ma_sav;
		ma_data_realloc(pma_ind,pbdat,*ptot_ma,nvox);
	}
}


void	set_cube_ma(int *cube, int *ma_ind, int tot_ma, int val)
{
	int	n;

	for( n = 0;  n < tot_ma;  n++ ) cube[ma_ind[n]] = val;
}


void	correct_ma_for_trim(int **pma_ind, unsigned char **pbdat,
							int *ptot_ma, int *cube, int nx, int nxy,
							int nxyz)
{
	unsigned char	btmp;

	int	stencil[26];

	int	ns, ne, ind;
	int	val, nval;
	int	tot_ma_sav, nvox;
	int	mtmp;
	int	j;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	if( nxyz/nxy == 3 ) set_2D_restricted_stencil(nx,nxy,stencil);
	else		    set_stencil(nx,nxy,stencil);

		/* Set boundary MA voxels from MA to MA_TRIM */
	for( ns = 0;  ns < *ptot_ma;  ns++ )
	{
		ind = (*pma_ind)[ns];
		for( j = 0;  j < 26;  j++ )
		{
			if( cube[ind+stencil[j]] == EXT_VAL )
			{
				cube[ind] = MA_TRIM;
				break;
			}
		}
	}

	/****
	dbg_print_ma_list("after init bdry voxels",*pma_ind,*ptot_ma,cube,
								nx,nxy);
	****/

		/*  MA now marks only interior MA voxels  */
		/* re-set boundary MA voxels according to */
		/* type of abutment with interior MA voxel */

	for( ns = 0;  ns < *ptot_ma;  ns++ )
	{
		ind = (*pma_ind)[ns];
		if( cube[ind] != MA_TRIM ) continue;
		cube[ind] = set_ma_abut_val(ind,stencil,cube);
	}
	/****
	dbg_print_ma_list("after set bdry voxel abutment",*pma_ind,*ptot_ma,
							cube,nx,nxy);
	****/

	    /* Keep boundary MA voxels with max local abutment values */

	for( ns = 0;  ns < *ptot_ma;  ns++ )
	{
		ind = (*pma_ind)[ns];
		val = cube[ind];
		if( val < 102 ) continue;
		for( j = 0;  j < 26;  j++ )
		{
			nval = cube[ind + stencil[j]];
			if(    ((nval == 102) || (nval == 103) || (nval == 104))			    && (nval > val)    )
			{
				cube[ind] = MA_TRIM;
				break;
			}
		}
	}
	/****
	dbg_print_ma_list("after keep local max bdry voxels",*pma_ind,*ptot_ma,
								cube,nx,nxy);
	****/

	    /* boundary MA voxels with val = 102, 103 or 104 are kept */

	for( ns = 0;  ns < *ptot_ma;  ns++ )
	{
		ind = (*pma_ind)[ns];
		val = cube[ind];
		if( val > MA_TRIM ) cube[ind] = MA;
	}
	/****
	dbg_print_ma_list("after keep bdry voxels",*pma_ind,*ptot_ma,
							cube,nx,nxy);
	****/


		/* erase all MA boundary voxels marked MA_TRIM */
	tot_ma_sav = *ptot_ma;

	ns = 0;		ne = (*ptot_ma) - 1;
	while( ns <= ne )
	{
		ind = (*pma_ind)[ns];

		if( cube[ind] != MA_TRIM ) ns++;
		else
		{
			(*ptot_ma)--;

			mtmp = (*pma_ind)[ne];	btmp = (*pbdat)[ne];
			(*pma_ind)[ne] = ind;	(*pbdat)[ne] = (*pbdat)[ns];
			(*pma_ind)[ns] = mtmp;	(*pbdat)[ns] = btmp;

			ne--;
		}
	}
	if( *ptot_ma != ns )
	{
		fprintf(stderr,"Logic error in correct_ma_for_trim()\n");
		fprintf(stderr,"*ptot_ma (%d) != ns (%d)\n",*ptot_ma,ns);
		clean_up(0);
	}
	/****
	dbg_print_ma_list("after erase bdry voxels",*pma_ind,*ptot_ma,
							cube,nx,nxy);
	****/

	nvox = (*ptot_ma) - tot_ma_sav;
	ma_data_realloc(pma_ind,pbdat,*ptot_ma,nvox);

	fprintf(stderr,"%d total ma voxels corrected for trim\n",-nvox);
	//PRINT_O("%d total ma voxels corrected for trim\n",-nvox);
}


void	dbg_print_ma_list(char *strng, int *ma_ind, int tot_ma, int *cube,
						  int nx, int nxy)
{
	int	j, ix, iy, iz;
	int	ind;

	printf("\nMA list %s\n",strng);
	for( j = 0;  j < tot_ma;  j++ )
	{
		ind = ma_ind[j];
		Pad_ind_2_ijk(ind,ix,iy,iz,nx,nxy)
		printf("(%d,%d,%d) %d\n",ix,iy,iz,cube[ind]);
	}
	printf("\n");
}


/*
*	MA voxel `ind' is a boundary MA voxel and is to be trimmed.
*	Record it's abutment value with any interior MA voxel.  Face
*	abutment takes precedence over edge abutment which takes
*	precedence over corner abutment.
*/

int	set_ma_abut_val(int ind, int *stencil, int *cube)
{
	int	n_ind;

			/* face abutment */
	n_ind = ind + stencil[ 4];	if( cube[n_ind] == MA ) return 104;
	n_ind = ind + stencil[10];	if( cube[n_ind] == MA ) return 104;
	n_ind = ind + stencil[12];	if( cube[n_ind] == MA ) return 104;
	n_ind = ind + stencil[13];	if( cube[n_ind] == MA ) return 104;
	n_ind = ind + stencil[15];	if( cube[n_ind] == MA ) return 104;
	n_ind = ind + stencil[21];	if( cube[n_ind] == MA ) return 104;

			/* edge abutment */
	n_ind = ind + stencil[ 1];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[ 3];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[ 5];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[ 7];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[ 9];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[11];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[14];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[16];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[18];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[20];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[22];	if( cube[n_ind] == MA ) return 103;
	n_ind = ind + stencil[24];	if( cube[n_ind] == MA ) return 103;

			/* corner abutment */
	n_ind = ind + stencil[ 0];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[ 2];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[ 6];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[ 8];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[17];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[19];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[23];	if( cube[n_ind] == MA ) return 102;
	n_ind = ind + stencil[25];	if( cube[n_ind] == MA ) return 102;

	return MA_TRIM;
}


/*****
void	correct_ma_for_trim(pma_ind,pbdat,ptot_ma,ptot_mas,cube,nx,nxy,val_on)
	int		**pma_ind;
	unsigned char	**pbdat;
	int		*ptot_ma, *ptot_mas;
	int		*cube;
	int		nx, nxy, val_on;
{
	extern	void	clean_up();
	extern	void	set_stencil(), ma_data_realloc();
	extern	void	//PRINT_O();

	unsigned char	btmp;

	int	stencil[26];

	int	ns, ne, ind;
	int	tot_ma_sav, tot_mas_sav, nvox;
	int	mtmp;
	int	j;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	set_stencil(nx,nxy,stencil);

	tot_ma_sav = *ptot_ma;		tot_mas_sav = *ptot_mas;

	ns = 0;		ne = (*ptot_ma) - 1;
	while( ns <= ne )
	{
		ind = (*pma_ind)[ns];

		j = 0;
		while( j < 26 )
		{
		    if( cube[ind+stencil[j]] == EXT_VAL )
		    {
			(*ptot_ma)--;
			if( cube[ind] == val_on ) (*ptot_mas)--;
			cube[ind] = 101;
			break;
		    }
		    j++;
		}

		if( cube[ind] != 101 ) ns++;
		else
		{
			mtmp = (*pma_ind)[ne];	btmp = (*pbdat)[ne];
			(*pma_ind)[ne] = ind;	(*pbdat)[ne] = (*pbdat)[ns];
			(*pma_ind)[ns] = mtmp;	(*pbdat)[ns] = btmp;

			ne--;
		}
	}
	if( *ptot_ma != ns )
	{
		fprintf(stderr,"Logic error in correct_ma_for_trim()\n");
		fprintf(stderr,"*ptot_ma (%d) != ns (%d)\n",*ptot_ma,ns);
		clean_up(0);
	}

	nvox = (*ptot_ma) - tot_ma_sav;
	ma_data_realloc(pma_ind,pbdat,*ptot_ma,nvox);

	fprintf(stderr,"%d total ma voxels corrected for trim\n",-nvox);
	//PRINT_O("%d total ma voxels corrected for trim\n",-nvox);

	nvox = (*ptot_mas) - tot_mas_sav;
	fprintf(stderr,"%d on surface ma voxels corrected for trim\n",-nvox);
	//PRINT_O("%d on surface ma voxels corrected for trim\n",-nvox);
}
*****/


void	ma_data_realloc(int **pma_ind, unsigned char **pbdat, int tot_ma,
						int nvox)
{
	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	*pma_ind = (int *)REALLOC(*pma_ind,tot_ma*isz);
	if( MEMCHECK(*pma_ind,"*pma_ind in ma_data_realloc()",nvox*isz) )
		clean_up(0);

	*pbdat = (unsigned char *)REALLOC(*pbdat,tot_ma*usz);
	if( MEMCHECK(*pbdat,"*pbdat in ma_data_realloc()",nvox*usz) )
		clean_up(0);
}



void	debug_print_ma_cube(int *ma_ind, int tot_ma, int *cube)
{
	int	i;

	fprintf(stdout,"First 10 ma indices");
	if( cube != NULL ) fprintf(stdout," and corresponding cube values");
	fprintf(stdout,"\n");

	for( i = 0;  i < 10;  i++ )
	{
		fprintf(stdout,"%d: %d",i,ma_ind[i]);
		if( cube != NULL ) fprintf(stdout,"\t%d",cube[ma_ind[i]]);
		fprintf(stdout,"\n");
	}
	fprintf(stdout,"\n");

	fprintf(stdout,"Last 10 ma indices");
	if( cube != NULL ) fprintf(stdout," and corresponding cube values");
	fprintf(stdout,"\n");

	for( i = tot_ma-10;  i < tot_ma;  i++ )
	{
		fprintf(stdout,"%d: %d",i,ma_ind[i]);
		if( cube != NULL ) fprintf(stdout,"\t%d",cube[ma_ind[i]]);
		fprintf(stdout,"\n");
	}
	fprintf(stdout,"\n");
}
