#include "stdafx.h"
#include <stdlib.h>
#include <math.h>
#include "ma_prototype.h"


mxArray * mexdendritemain(const mxArray *input)
{
	mxArray * input_cube; 
	mxArray * fout;
	mxArray * den_list;
	mxArray * mx_ma_loc;
	mxArray * mx_ma_loc_out;
	mwSize nx,ny,nz,number_of_dims;
	const mwSize * pind;
	unsigned char *ma_bno;
	unsigned char *burn_cube;
	UINT32	nloc, *old_loc, *new_loc,oldnloc;
	int *ma_loc;
	int usefast,ntol,do_cmrg;
	int n_den;
	MA_set * den_root[100];
	MA_set * cur_den;
	int i,j,ix,iy,iz;
	int num_den,brk_den;
	float um_x,um_y,um_z,dp_tol,loop_tol;
	CMrg_menu cmrg;

	input_cube = mxGetCell(input,0);
	number_of_dims = mxGetNumberOfDimensions(input_cube);
	if (number_of_dims==2)
	{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=1;
	}
		else if (number_of_dims==3)
		{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=pind[2];
		}
		else
		mexErrMsgTxt("Input must be 2 or 3 dimensional");
	if (mxIsUint8(input_cube) == 0 )
		mexErrMsgTxt("Input must be uint8 type.");

	mx_ma_loc = mxGetCell(input,1);

	
	pind=mxGetDimensions(mx_ma_loc);
	nloc=pind[0];
	//Create another instance of ma_loc and ma_bno for modification


	ma_loc = (int*) CALLOC(nloc,sizeof(int));
	ma_bno = (unsigned char*) CALLOC(nloc, sizeof(unsigned char));
	old_loc=(UINT32*)mxGetData(mx_ma_loc);
	for (i=0;i<nloc;i++)
		ma_loc[i]=old_loc[i];
//	memcpy(ma_loc,mxGetData(mx_ma_loc),nloc*sizeof(UINT32));
	
	if ((mxIsUint8(mxGetCell(input,2)) == 0) || (mxIsUint8(mxGetCell(input,3)) == 0) || (mxIsUint8(mxGetCell(input,4)) == 0) || (mxIsUint8(mxGetCell(input,5)) == 0) )
		mexErrMsgTxt("Parameters 1-5 must be uint8 type.");

	if ((mxIsDouble(mxGetCell(input,6)) == 0) || (mxIsDouble(mxGetCell(input,7)) == 0) || (mxIsDouble(mxGetCell(input,8)) == 0) || (mxIsDouble(mxGetCell(input,9)) == 0) || (mxIsDouble(mxGetCell(input,10)) == 0)  )
		mexErrMsgTxt("Parameters 6-10 must be double type.");


	num_den=(int)*((unsigned char *)mxGetData(mxGetCell(input,2)));
	ntol=(int)*((unsigned char*)mxGetData(mxGetCell(input,3)));
	do_cmrg=(int)*((unsigned char*)mxGetData(mxGetCell(input,4)));
	brk_den=(int)*((unsigned char*)mxGetData(mxGetCell(input,5)));
	dp_tol=(float)*(mxGetPr(mxGetCell(input,6)));
	um_x=(float)*(mxGetPr(mxGetCell(input,7)));
	um_y=(float)*(mxGetPr(mxGetCell(input,8)));
	um_z=(float)*(mxGetPr(mxGetCell(input,9)));
	loop_tol=(float)*(mxGetPr(mxGetCell(input,10)));


	burn_cube=(unsigned char *)mxGetData(input_cube);

	
	cmrg.ntol=ntol;
	cmrg.do_cmrg=do_cmrg;
	cmrg.method='c';

	for (i=0;i<nloc;i++)
		ma_bno[i]=burn_cube[ma_loc[i]];

	set_dendritic_path(den_root,&n_den, nx,ny,nz,burn_cube,&nloc,&ma_loc,&ma_bno, dp_tol, num_den,
		0, um_x,um_y,um_z,cmrg,loop_tol);

	den_list=mxCreateCellMatrix(n_den,1);
	cur_den=*den_root;
	for (i=0;i<n_den;i++)
	{
		mx_ma_loc_out = mxCreateNumericMatrix(cur_den->num_in_set, 1, mxUINT32_CLASS, mxREAL);
		new_loc=(UINT32*) CALLOC(cur_den->len,sizeof(UINT32));
		for (j=0;j<cur_den->num_in_set;j++)
		{
			Pad_ind_2_ijk( cur_den->loc[j] ,ix,iy,iz,(nx+2),((ny+2)*(nx+2)) )		
			new_loc[j]=ix+iy*nx+iz*nx*ny;
		}
		mxSetData(mx_ma_loc_out,new_loc);
		mxSetCell(den_list,i,mx_ma_loc_out);
		cur_den=cur_den->next;
	}

	fout = den_list;
//		mxCreateCellMatrix(1, 1);
//	mxSetCell(fout,0,den_list);
	for (i=1;i<n_den+1;i++)
		remove_dendrite(den_root,i);
	FREE(ma_loc,oldnloc);
	return fout;
}

void	set_dendritic_path(MA_set **den_root, int * n_den, int nx, int ny,int nz,
			unsigned char *burn_cube, int *pnloc, int **pma_loc, unsigned char **pma_bno,			
			float dp_tol,int num_den, int trim, float um_x,float um_y,
			float um_z,CMrg_menu cmrg, float loop_tol)
{
	float	dendritic_len;

	int	n_ma_vox, *ma_list;
	unsigned char	*burn_dat;
	int	* cube;
	int	n_sngl, n_path, n_clust;

	MA_set	*mas_root = NULL;
	MA_set	*map_root = NULL;
	MA_set	*mac_root = NULL;

	MA_set	*den;
	int	i, j, k, ind, low_ind;
	int	i2, j2, k2, l;
	char	*msg;
	float	*map_len;
	int	*den_ind;
	int Nx,Nxy,Nxyz;

	int	isz = sizeof(int);
	int	pMsz = sizeof(MA_set *);
	int	Msz = sizeof(MA_set);
	int	usz = sizeof(unsigned char);
	int	fsz = sizeof(float);
	Nx = nx + 2;	Nxy = Nx * (ny + 2);	Nxyz = Nxy * (nz + 2);
	set_pma_data_arrays(nx,ny,nz,trim,Nx,Nxy,Nxyz,pnloc,pma_loc,pma_bno,&cube,burn_cube);

	set_clust_paths(Nx,Nxy,Nxyz,*pnloc,*pma_loc,*pma_bno,cube,&cmrg,
		&mas_root,&map_root,&mac_root,&n_sngl,&n_path,&n_clust);

	FREE(cube,Nxyz*isz);

	/* set dendritic path and not ignoring bb-paths */
	set_fibers(map_root,mac_root,den_root,Nx,Nxy,dp_tol,n_den,1);

	rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);


	/****
	is this useful at all?  leftovers, no!
	select_bb_path(map_root,*den_root);
	****/

	/* dendrites that are short and both starting and ending are */
	/* attached to the same neighboring dendrite should be removed */
	/* update n_den and den_root after elimination */
	eliminate_dendrite_loop(den_root,n_den,Nx,Nxy,loop_tol);


	/* some spines are too long to remove in trimming algorithm */
	PRINTFSTDOUT("n_den is %d\n",*n_den);

	if( num_den < 0 ) num_den = *n_den;

	if( *n_den >= num_den )
	{
		/* choose "num_den" longest paths; */
		map_len = (float *)MALLOC(*n_den*fsz);
		msg = "map_len in set_dendritic_path()";
		if( MEMCHECK(map_len,msg,*n_den*fsz) ) clean_up(0);

		den_ind = (int *)MALLOC(*n_den*isz);
		msg = "den_ind in set_dendritic_path()";
		if( MEMCHECK(den_ind,msg,*n_den*isz) ) clean_up(0);

		for( i = 0,den = *den_root;  i < *n_den;  i++,den = den->next )
		{
			map_len[i] = den->len;
			den_ind[i] = den->ind;
		}
		if( *n_den > 1 ) float_int_heapsort(*n_den,map_len,den_ind);

		for( i = 0;  i < *n_den - num_den;  i++ )
		{
			PRINTFSTDOUT("\nremoving den_ind %d of len %g\n",
						den_ind[i],map_len[i]);
			remove_dendrite(den_root,den_ind[i]);
		}

		/* order the dendrites so that ind of 1 is longest dendrite. */
		/*******
		low_ind = *n_den - num_den;
		if( low_ind < 0 )  low_ind = 0;
		for( i = *n_den - 1, ind = 1;  i >= low_ind;  i-- )
		{
			for( den = *den_root;  den != NULL;  den = den->next )
			{
				if( den->ind == den_ind[i] )
				{
					den->ind = ind;
					break;
				}
			}
			ind++;
		}
		*******/
		FREE(map_len,*n_den*fsz);
		FREE(den_ind,*n_den*isz);
		*n_den = num_den;
	}

	for( i = 0, den = *den_root;  den != NULL;  i++, den = den->next )
		den->ind = i+1;		/* not ordered by length */
}


void	remove_dendrite(	MA_set	** den_root,int ind)
{
	
	

	int	i;
	MA_set	*den, *prev;

	int	isz = sizeof(int);
	int	pMsz = sizeof(MA_set *);
	int	Msz = sizeof(MA_set);
	int	usz = sizeof(unsigned char);

	prev = *den_root;
	for( den = *den_root;  den != NULL;  den = den->next )
	{
		if( den->ind == ind ) break;
		prev = den;
	}
	PRINTFSTDERR("removing %d\n",den);
	PRINTFSTDOUT("removing %d\n",den);

	if( prev == den )	/* root itself */
	{
		if( den != *den_root )
		{
			PRINTFSTDERR("prev == den but not *den_root\n");
			PRINTFSTDOUT("prev == den but not *den_root\n");
			clean_up(0);
		}
		*den_root = (*den_root)->next;
		if( *den_root != NULL ) (*den_root)->prev = NULL;
		PRINTFSTDERR("removing root %d\n",den);
		PRINTFSTDOUT("removing root %d\n",den);
		free_MA_set(den,isz,usz,Msz,pMsz);
	}
	else
	{
		prev->next = den->next;
		if( den->next != NULL ) den->next->prev = prev;
		free_MA_set(den,isz,usz,Msz,pMsz);
	}
}

/*
void	dendrite_print_driver(cube,den_root,nx,nxy,nxyz,zs,ze,ts,
						outfn,outfn_len)
	int	*cube;
	MA_set	*den_root;
	int	nx, nxy, nxyz, zs, ze, ts;
	char	*outfn;
	int	outfn_len;
{
	

	
	

	unsigned char	*burn_dat;
	int		*ma_list;

	int	n_ma_vox;

	int	usz = sizeof(unsigned char);
	int	isz = sizeof(int);

	map_2_cube(den_root,nx,nxy,nxyz,&burn_dat,cube,&ma_list,&n_ma_vox);
	project_modified_ma_files(nx,nxy,nxyz,n_ma_vox,ma_list,burn_dat,cube,
							outfn,outfn_len,1,0);

	FREE(burn_dat,nxyz*usz);
	FREE(ma_list,nxyz*isz);
}
*/


#define Print_ma(mas)\
	for( l = 0;  l < mas->num_in_set;  l++ )\
	{\
	    pos = mas->loc[l];\
	    if( ! (*cube)[pos] )\
	    {\
		(*cube)[pos] = 99;\
		(*ma_list)[*n_ma_vox] = pos;\
		(*burn_dat)[*n_ma_vox]= mas->bdat[l];\
		(*n_ma_vox)++;\
	    }\
	}


void	map_2_cube(map_root,nx,nxy,nxyz,burn_dat,cube,ma_list,n_ma_vox)
	MA_set		*map_root;
	int		nx, nxy, nxyz;
	unsigned char	**burn_dat;
	int		*cube;
	int		**ma_list;
	int		*n_ma_vox;
{
	
	

	char	*msg;

	MA_set	*map, *mac;
	int	l, m, pos;

	int	usz 	= sizeof(unsigned char);
	int	isz 	= sizeof(int);

	/***
	*cube = (int *)CALLOC(nxyz,isz);
	msg = "*cube in map_2_cube()";
        if( MEMCHECK(cube,msg,nxyz*isz) )  clean_up(0);
	***/

	*burn_dat = (unsigned char *)CALLOC(nxyz,usz);
	msg = "*burn_dat in map_2_cube()";
	if( MEMCHECK(burn_dat,msg,nxyz*usz) )  clean_up(0);

	*ma_list = (int *)CALLOC(nxyz,isz);
	msg = "*ma_list in map_2_cube()";
	if( MEMCHECK(ma_list,msg,nxyz*isz) )  clean_up(0);

	*n_ma_vox = 0;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		for( l = 0;  l < map->num_in_set;  l++ )
		{
			pos = map->loc[l];
			/***
			if( ! (cube)[pos] )
			{
			***/
				(cube)[pos] = 99;
				(*ma_list)[*n_ma_vox] = pos;
				if( (*burn_dat)[*n_ma_vox] < map->bdat[l] )
				    (*burn_dat)[*n_ma_vox]= map->bdat[l];
				(*n_ma_vox)++;
			/***
			}
			***/
		}
		/***
		if( map->att_mas[0] != NULL )
		{
		    mac = map->att_mas[0];
		    Print_ma(mac)
		}
		if( map->att_mas[1] != NULL )
		{
		    mac = map->att_mas[1];
		    Print_ma(mac)
		}
		***/
	}

	PRINTFSTDOUT("\n%d den voxels remained\n",*n_ma_vox);
	PRINTFSTDERR("\n%d den voxels remained\n",*n_ma_vox);
}


void	select_bb_path(MA_set *map_root,MA_set *den_root)
{
	
	

	MA_set	*map, *mac1, *mac2;
	int	isSame = -1;
	int	i, j;

	for( map = map_root;  map != NULL;  map = map->next )
	{
	    /* bb-path remains if attached to different fibers */
	    if( !map->on_tree )
	    {
		mac1 = map->att_mas[0];
		mac2 = map->att_mas[1];
		if( mac1 != mac2 )
		{
		    for( i = 0;  i < mac1->num_att;  i++ )
		    {
			if( mac1->att_mas[i] != map )
			{
			    for( j = 0;  j < mac2->num_att;  j++ )
			    {
				if( mac2->att_mas[j] != map )
				{
				    isSame = check_if_same_den(mac1->att_mas[i],
						mac2->att_mas[j],den_root);
				    if( isSame == 0 ) break;
				}
			    }
			}
		    }

		    if( isSame == -1 )
		    {
			PRINTFSTDERR("Logic error\n");
			PRINTFSTDOUT("Logic error\n");
			clean_up(0);
		    }
		    else if( isSame == 0 )	/* don't remove this bb-path */
		    {
			map->on_tree = 1;
		    }
		}
	    }
	}
}


int	check_if_same_den(MA_set *map1,MA_set *map2,MA_set *den_root)
{
	MA_set	*den;
	int	fbr_num1, fbr_num2;
	int	i;

	for( den = den_root;  den != NULL;  den = den->next )
	{
		for( i = 0;  i < den->num_att;  i++ )	/* each path on fiber */
		{
			if( den->att_mas[i] == map1 ) fbr_num1 = den->ind;
			if( den->att_mas[i] == map2 ) fbr_num2 = den->ind;
		}
	}

	if( fbr_num1 == fbr_num2 )  return 1;
	else			    return 0;
}


void	eliminate_dendrite_loop(MA_set **den_root,int *n_den,int Nx,int Nxy,float loop_tol)
{
	

	MA_set	*den, *nbr;

	int	si, sj, sk;
	int	ei, ej, ek;
	int	i, j, k, l, m;
	int	start_ind, end_ind, remove;

	for( den = *den_root;  den != NULL;  den = den->next ) den->on_tree = 1;

	for( den = *den_root;  den != NULL;  den = den->next )
	{
		/* TMP: HARDCODED */
		if( den->len > loop_tol ) continue;

		l = den->loc[0];
		Pad_ind_2_ijk( l,si,sj,sk,Nx,Nxy )
		l = den->loc[den->num_in_set-1];
		Pad_ind_2_ijk( l,ei,ej,ek,Nx,Nxy )

		start_ind = end_ind = -1;
		for( nbr = *den_root;  nbr != NULL;  nbr = nbr->next )
		{
			if( nbr == den ) continue;
			for( m = 0;  m < nbr->num_in_set;  m++ )
			{
				l = nbr->loc[m];
				Pad_ind_2_ijk( l,i,j,k,Nx,Nxy )
				if( (start_ind < 0) &&
				    (abs(i-si) <= 2) &&
				    (abs(j-sj) <= 2) &&
				    (abs(k-sk) <= 2) )
				{
					start_ind = nbr->ind;
				}
				if( (end_ind < 0) &&
				    (abs(i-ei) <= 2) &&
				    (abs(j-ej) <= 2) &&
				    (abs(k-ek) <= 2) )
				{
					end_ind = nbr->ind;
				}
				if( (start_ind >= 0) && (end_ind >= 0) )
					break;	/* found start_ind */
			}
		}
PRINTFSTDOUT("start_ind %d\n",start_ind);
PRINTFSTDOUT("end_ind %d\n",end_ind);

		if( (start_ind == end_ind) && (start_ind >= 0) )
		{
			PRINTFSTDOUT("Dendrite %d forms loop\n",den->ind);
			den->on_tree = 0;
		}
	}


	do
	{
		remove = 0;
		for( den = *den_root;  den != NULL;  den = den->next )
		{
			if( den->on_tree == 0 )
			{
				remove_MA_set(den_root,den);
				remove = 1;
			}
			if( remove ) break;
		}
	}while( remove );

	i = 1;
	for( den = *den_root, *n_den = 0;  den != NULL;  den = den->next )
	{
		den->ind = i;
		i++;
		(*n_den)++;
	}
}


void	remove_MA_set(MA_set **den_root,MA_set *den)
{
	

	int	isz = sizeof(int);
	int    pMsz = sizeof(MA_set *);
	int	Msz = sizeof(MA_set);
	int	usz = sizeof(unsigned char);

	if( *den_root == den )
	{
		*den_root = den->next;
		if( *den_root != NULL ) (*den_root)->prev = NULL;
	}
	else
	{
		if( den->prev != NULL ) den->prev->next = den->next;
		if( den->next != NULL ) den->next->prev = den->prev;
	}
	free_MA_set(den,isz,usz,Msz,pMsz);
}


