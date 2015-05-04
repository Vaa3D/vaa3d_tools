/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "maset.h"
#include "burn_macro.h"
#include "lkc_prototype.h"
#include "ma_prototype.h"

mxArray * mextrimmain(const mxArray *input)
{


	mxArray * input_cube; 
	mxArray * fout;
	mxArray * mx_ma_loc;
	mxArray * mx_ma_loc_out;
	mwSize nx,ny,nz,number_of_dims;
	const mwSize * pind;
	unsigned char *ma_bno;
	unsigned char *burn_cube;
	UINT32	nloc, *old_loc, *new_loc,oldnloc;
	int *ma_loc;
	int LL_PATH_TOL,BL_PATH_TOL,usefast,ntol,do_cmrg;
	int i;
	int ix,iy,iz;
	int	Itmp;


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
	
	if ((mxIsUint8(mxGetCell(input,2)) == 0) || (mxIsUint8(mxGetCell(input,3)) == 0) || (mxIsUint8(mxGetCell(input,4)) == 0) || (mxIsUint8(mxGetCell(input,5)) == 0) || (mxIsUint8(mxGetCell(input,6)) == 0))
		mexErrMsgTxt("Parameters must be uint8 type.");

	LL_PATH_TOL=(int)*((unsigned char *)mxGetData(mxGetCell(input,2)));
	BL_PATH_TOL=(int)*((unsigned char *)mxGetData(mxGetCell(input,3)));
	usefast=(int)*((unsigned char*)mxGetData(mxGetCell(input,4)));
	ntol=(int)*((unsigned char*)mxGetData(mxGetCell(input,5)));
	do_cmrg=(int)*((unsigned char*)mxGetData(mxGetCell(input,6)));
	burn_cube=mxGetData(input_cube);

	for (i=0;i<nloc;i++)
		ma_bno[i]=burn_cube[ma_loc[i]];
	oldnloc=nloc;

	ma_modify_driver(nx,ny,nz,burn_cube, &nloc, &ma_loc, &ma_bno,1,LL_PATH_TOL,BL_PATH_TOL,usefast,ntol,do_cmrg);

	new_loc=(UINT32*) CALLOC(nloc,sizeof(UINT32));
	for (i=0;i<nloc;i++)
	{

		Pad_ind_2_ijk( ma_loc[i] ,ix,iy,iz,(nx+2),((ny+2)*(nx+2)) )
		
		new_loc[i]=ix+iy*nx+iz*nx*ny;
	}
	mx_ma_loc_out = mxCreateNumericMatrix(nloc, 1, mxUINT32_CLASS, mxREAL);
	mxSetData(mx_ma_loc_out,new_loc);
	fout = mxCreateCellMatrix(1, 1);
	mxSetCell(fout,0,mx_ma_loc_out);
	FREE(ma_loc,oldnloc);
	return fout;
}




void	ma_modify_driver(int nx, int ny, int nz,
						 unsigned char *burn_cube, int *pnloc,
						 int **pma_loc, unsigned char **pma_bno,
						 int is_spine,  int LL_PATH_TOL, int BL_PATH_TOL, int use_fast, int ntol, int do_cmrg)
{
	MA_set		*mas_root = NULL;
	MA_set		*map_root = NULL;
	MA_set		*mac_root = NULL;
	MA_tree		*mat_root = NULL;
	MA_tree		*iso_mat  = NULL;

	Mod_Menu	mod;
	CMrg_menu cmrg;
	int		cmrg_val;
	int	nloc;
	int *ma_loc;
	unsigned char *ma_bno;

	unsigned char	*burn_dat, *num_nhbr;

	char	out_ftype[5];

	float	vox_len = 1.0;

	int	*cube;
	int	*ma_list, **nhbr_list;

	int	offset[27];

	int	first;
	int	Nx, Nxy, Nxyz;
	int	n, n_ma_vox;
	int	max_bnum;
	int	bc_done, iv_rem, done, sr_done;
	int	n_sngl, n_path, n_clust;



	int	trim;

	int    pisz = sizeof(int *);
	int 	isz = sizeof(int);
	int    pMsz = sizeof(MA_set *);
	int 	Msz = sizeof(MA_set);
	int 	usz = sizeof(unsigned char);
	int    MTsz = sizeof(MA_tree);

	int	alt = 1;
	nloc = *pnloc;
	ma_loc = *pma_loc;
	ma_bno = *pma_bno;

	trim = 0;	// cshl parameter
	n_ma_vox = nloc;
	ma_list  = ma_loc;
	burn_dat = ma_bno;
	Nx = nx + 2;	Nxy = Nx * (ny + 2);	Nxyz = Nxy * (nz + 2);
	set_pma_data_arrays(nx,ny,nz,trim,Nx,Nxy,Nxyz,
				&n_ma_vox,&ma_list,&burn_dat,&cube,burn_cube);

	max_bnum = 0;
	for( n = 0;  n < n_ma_vox;  n++ )
	{
		if( (int)(burn_dat[n]) > max_bnum ) max_bnum = burn_dat[n];
	}

	set_ma_modify(&mod,LL_PATH_TOL,BL_PATH_TOL,use_fast);

	out_ftype[0] = 'v';
	cmrg.do_cmrg = do_cmrg;	// cshl parameters
	cmrg.method = 'c';
	cmrg.ntol=ntol;
	
	set_nhbr_offsets(Nx,Nxy,offset);

	done = 0;	first = 1;
	bc_done = sr_done = -1;
	if( !mod.sr_red ) sr_done = 0;
	while( !done )
	{
		set_nhbr_list(&num_nhbr,&nhbr_list,ma_list,n_ma_vox,cube,
									offset);

		set_MA_sets(ma_list,nhbr_list,num_nhbr,burn_dat,cube,offset,
					n_ma_vox,Nx,Nxy,&mas_root,&map_root,&mac_root,
					&n_sngl,&n_path,&n_clust);

		unlink_cube_from_ma_list_entries(ma_list,n_ma_vox,cube);

		free_nhbr_list(num_nhbr,nhbr_list,n_ma_vox);

		link_path_2_cluster(&map_root,mac_root,Nx,Nxy,&n_path);
		set_surf_rmnts(mac_root,Nx,Nxy);

		MA_tree_driver(mac_root,map_root,&mat_root,&iso_mat);

		iv_rem = 0;
		if( first )
		{
			first = 0;

			if( mod.ivb_mod )
			{
				cleanup_iv_in_cube(cube,mas_root,mod.ivb_thresh,
							max_bnum,&iv_rem);
			}
		}
		else
		{
		}

		if( alt )
		{
			bc_done = 0;
			if( mod.ip_mod )
				cleanup_ip_in_cube(cube,map_root,mod.ip_thresh,
								&bc_done);
			if( mod.bl_mod || mod.ne_mod )
				cleanup_bl_ne_in_cube(cube,map_root,mac_root,
							&mod,&bc_done,Nx,Nxy);
		}
		else
		{
			sr_done = 0;
			if( mod.sr_red ) modify_surf_rmnt(cube,mac_root,Nx,Nxy,
								&sr_done);
		}
		if( mod.sr_red ) alt = 1 - alt;

		if( (bc_done == 0) && (sr_done == 0) && (iv_rem == 0) )
		{
			done = 1;
		}
		else
		{
			reset_ma_list_from_cube(cube,&ma_list,&burn_dat,
									&n_ma_vox);
			rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
			rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
			rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);
			rec_free_MA_tree(mat_root,pMsz,MTsz);
			rec_free_MA_tree(iso_mat ,pMsz,MTsz);

			mas_root = NULL;
			map_root = NULL;
			mac_root = NULL;
			mat_root = NULL;
			iso_mat  = NULL;
		}

		PRINTF("\n%d isolated voxels removed\n\n",iv_rem);
		PRINTF("\n%d branch clusters reduced\n\n",bc_done);
		PRINTF("\n%d surface remnants reduced\n\n",sr_done);
		PRINTF("%d voxels left on MA\n",n_ma_vox);
    }

	rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
	rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);
	rec_free_MA_tree(mat_root,pMsz,MTsz);
	rec_free_MA_tree(iso_mat ,pMsz,MTsz);

    reset_ma_list_from_cube(cube,&ma_list,&burn_dat,&n_ma_vox);
    mas_root = map_root = mac_root = NULL;

    set_clust_paths(Nx,Nxy,Nxyz,n_ma_vox,ma_list,burn_dat,cube,
				&cmrg,&mas_root,&map_root,&mac_root,
				&n_sngl,&n_path,&n_clust);

    if( cmrg.do_cmrg )
    {
		if( mod.sr_red ) 
		{
			bc_done = 0;
			
			modify_surf_rmnt(cube,mac_root,Nx,Nxy,&bc_done);

			reset_ma_list_from_cube(cube,&ma_list,&burn_dat,&n_ma_vox);

			if( (out_ftype[0] == 'c') || (out_ftype[0] == 'v') )
			{
				PRINTF("\n\nAfter 2nd reset_ma_list\n\n");
			}

			rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
			rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
			rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);
			mas_root = map_root = mac_root = NULL;

			cmrg_val = cmrg.do_cmrg;
			cmrg.do_cmrg = 0;
			set_clust_paths(Nx,Nxy,Nxyz,n_ma_vox,ma_list,burn_dat,
				cube,&cmrg,&mas_root,&map_root,&mac_root,
				&n_sngl,&n_path,&n_clust);

			if( (out_ftype[0] == 'c') || (out_ftype[0] == 'v') )
			{
				PRINTF("\n\nFinal cluster/paths\n\n");
			}
		}
    }

    switch( out_ftype[0] )
    {
		case 'v':
			reset_ma_list_from_cube(cube,&ma_list,&burn_dat,&n_ma_vox);
			break;
	}
	*pnloc = n_ma_vox;
	*pma_loc = ma_list;
	*pma_bno = burn_dat;

    FREE(cube,Nxyz*isz);

    rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);
    rec_free_MA_set(map_root,isz,usz,Msz,pMsz);
    rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);
}


#define PE(_strng) fprintf(stderr,_strng)
#define PO(_strng) fprintf(stdout,_strng)


void	set_ma_modify(Mod_Menu *mod, int LL_PATH_TOL,
					  int BL_PATH_TOL, int use_fast)
{
	// cshl parameter
	mod->ivb_mod = 1;
	mod->ivb_thresh = INT_MAX;
	mod->ip_mod = 1;
	mod->ip_thresh = LL_PATH_TOL;
	mod->bl_mod = 1;
	mod->bl_thresh = BL_PATH_TOL;
	mod->bl_keep_bdry = 1;
	mod->bl_keep_z_bdry = 0;
	mod->bl_fast = use_fast;
	mod->ne_mod = 1;
	mod->ne_thresh = INT_MAX;
	mod->use_bno = (    (mod->bl_mod && (mod->bl_thresh < 0))
			 || (mod->ne_mod && (mod->ne_thresh < 0))    ) ? 1 : 0;
	mod->sr_red = 1;
}


void	init_ma_modify(int max_bnum, Mod_Menu *mod)
{
	PE("\nMedial axis clean up control menu\n\n");
	PO("\nMedial axis clean up control menu\n\n");

	fprintf(stderr,"\tMaximum burn number on medial axis ");
	fprintf(stderr,"for the first timestep is %d\n",max_bnum);
	fprintf(stdout,"\tMaximum burn number on medial axis ");
	fprintf(stderr,"for the first timestep is %d\n",max_bnum);
	init_iv_ip_cleanup("isolated voxel","burn number",&(mod->ivb_mod),
							&(mod->ivb_thresh));
	init_iv_ip_cleanup("isolated path" ,"length",&(mod->ip_mod),
							&(mod->ip_thresh));
	init_bl_ne_cleanup("branch-leaf",&(mod->bl_mod),&(mod->bl_thresh),
		&(mod->bl_keep_bdry),&(mod->bl_keep_z_bdry),&(mod->bl_fast));
	init_bl_ne_cleanup("needle-eye" ,&(mod->ne_mod),&(mod->ne_thresh),
					(int *)NULL,(int *)NULL,(int *)NULL);

	mod->use_bno = (    (mod->bl_mod && (mod->bl_thresh < 0))
			 || (mod->ne_mod && (mod->ne_thresh < 0))    ) ? 1 : 0;

	mod->sr_red = init_surf_rmnt_cleanup();
}


void	init_iv_ip_cleanup(char *type, char *crit, int *mod, int *thresh)
{
	char	ans[5];

	fprintf(stderr,"\n    Remove %ss (y,n(dflt)): ",type);
	gets(ans);
	fprintf(stdout,"\n    Remove %ss (y,n(dflt)): %s\n",type,ans);

	*mod = (ans[0] == 'y') ? 1 : 0;

	if( *mod )
	{
		fprintf(stderr,"\tRemove all %ss? (y,n(dflt)): ",type);
		gets(ans);
		fprintf(stdout,"\tRemove all %ss? (y,n(dflt)): %s\n",type,ans);

		if( ans[0] == 'y' ) *thresh = INT_MAX;
		else
		{
			fprintf(stderr,"\tRemove %ss having %s less than: ",
								type,crit);
			scanf("%d",thresh);		getchar();
			fprintf(stderr,"\n");

			fprintf(stdout,"\tRemove %ss having %s less than: ",
								type,crit);
			fprintf(stdout,"%d\n",*thresh);
		}
	}
}


void	init_bl_ne_cleanup(char *type, int *mod, int *thresh,
						   int *keep_bdry, int *keep_z_bdry, int *bl_fast)
{
	char	ans[5], ans0[5];

	fprintf(stderr,"\n    Remove %s paths (y,n(dflt)): ",type);
	gets(ans);
	fprintf(stdout,"\n    Remove %s paths (y,n(dflt)): %s\n",type,ans);

	*mod = (ans[0] == 'y') ? 1 : 0;

	if( *mod )
	{
		fprintf(stderr,"\tRemove all %ss? (y,n(dflt)): ",type);
		gets(ans0);
		fprintf(stdout,"\tRemove all %ss? (y,n(dflt)): %s\n",type,ans0);

		if( type[0] == 'b' )
		{
		    fprintf(stderr,"\tRetain %s paths exiting through ",type);
		    fprintf(stderr,"boundary? (y,n(dflt): ");	gets(ans);

		    fprintf(stdout,"\tRetain %s paths exiting through ",type);
		    fprintf(stdout,"boundary? (y,n(dflt): %s\n",ans);

		    *keep_bdry = (ans[0] == 'y') ? 1 : 0;

		    fprintf(stderr,"\tRetain %s paths exiting through ",type);
		    fprintf(stderr,"z-boundary also? (y,n(dflt): ");
		    gets(ans);

		    fprintf(stdout,"\tRetain %s paths exiting through ",type);
		    fprintf(stdout,"z-boundary also? (y,n(dflt): %s\n",ans);

		    *keep_z_bdry = (ans[0] == 'y') ? 1 : 0;

		    fprintf(stderr,"\n\tThere are two algorithms for branch-");
		    fprintf(stderr,"leaf path removal.\n\tThe faster ");
		    fprintf(stderr,"algorithm removes the shortest path at ");
		    fprintf(stderr,"every\n\tcluster at each step.  The ");
		    fprintf(stderr,"slower algorithm is more\n\t");
		    fprintf(stderr,"accurate; it removes one shortest path ");
		    fprintf(stderr,"among all the\n\tclusters at each step.\n");
		    fprintf(stderr,"\t    Use the faster algorithm? ");
		    fprintf(stderr,"(y(dflt),n): ");
		    gets(ans);		fprintf(stderr,"\n");

		    fprintf(stdout,"\n\tThere are two algorithms for branch-");
		    fprintf(stdout,"leaf path removal.\n\tThe faster ");
		    fprintf(stdout,"algorithm removes the shortest path at ");
		    fprintf(stdout,"every\n\tcluster at each step.  The ");
		    fprintf(stdout,"slower algorithm is more\n\t");
		    fprintf(stdout,"accurate; it removes one shortest path ");
		    fprintf(stdout,"among all the\n\tclusters at each step.\n");
		    fprintf(stdout,"\t    Use the faster algorithm? ");
		    fprintf(stdout,"(y(dflt),n): %s\n\n",ans);

		    if( ans[0] == 'n' ) *bl_fast = 0;
		    else		*bl_fast = 1;
		}

		if( ans0[0] == 'y' )
		{
			*thresh = INT_MAX;
			return;
		}

		fprintf(stderr,"\t%s path removal criteria\n",type);
		PE("\t    Remove path if length less than\n");
		PE("\t\tmaximum cluster burn number (auto)\n");
		PE("\t\tuser specified tolerance    (user)\n");
		PE("\t    Enter choice (a(dflt),u): ");
		gets(ans);

		fprintf(stdout,"\t%s path removal criteria\n",type);
		PO("\t    Remove path if length less than\n");
		PO("\t\tmaximum cluster burn number (auto)\n");
		PO("\t\tuser specified tolerance    (user)\n");
		fprintf(stdout,"\t    Enter choice (a(dflt),u): %s\n",ans);

		if( ans[0] == 'u' )
		{
			fprintf(stderr,"\t    Enter tolerance length: ");
			scanf("%d",thresh);	getchar();
			fprintf(stderr,"\n");

			fprintf(stdout,"\t    Enter tolerance length: %d\n",
								*thresh);
		}
		else	*thresh = -1;
	}
}


int	init_surf_rmnt_cleanup()
{
	char	s[4];
	int	ans;

	PE("\n    Surface Remnant clean up\n");
	PO("\n    Surface Remnant clean up\n");

	PE("\tReduce surface remant structure where possible");
	PE("(y,n(dflt)): ");
	gets(s);

	PO("\tReduce surface remant structure where possible");
	fprintf(stdout,"(y,n(dflt)): %s\n",s);

	ans = (s[0] == 'y') ? 1 : 0;
	return ans;
}


/*
*	MA_sets are left unchanged after cleanup_*() routines.
*	Only the cube is changed.
*/

#define Cube_reset(_ns,_ne,_loc,_val)\
	for( i = _ns;  i < _ne;  i++ )\
	{\
		ind = _loc[i];	cube[ind] = _val;\
	}


void	cleanup_iv_in_cube(int *cube, MA_set *mas, int ivb_thresh,
						   int max_bnum, int *iv_rem)
{
	int	iv_num;
	int	i, ind, sum;

	if( mas == (MA_set *)NULL ) sum = 0;
	else
	{
	    iv_num = mas->num_in_set;

		 if( ivb_thresh < 1        ) sum = 0;
	    else if( ivb_thresh > max_bnum )
	    {
		sum = iv_num;
		Cube_reset(0,iv_num,mas->loc,0);
	    }
	    else
	    {
		sum = 0;
		for( i = 0;  i < iv_num;  i++ )
		{
			if( (int)(mas->bdat[i]) < ivb_thresh )
			{
				ind = mas->loc[i];	cube[ind] = 0;
				sum++;
			}
		}
	    }
	}
	fprintf(stderr,"\n%d isolated voxels removed from MA\n",sum);
	fprintf(stdout,"\n%d isolated voxels removed from MA\n",sum);
	*iv_rem = sum;
}


void	cleanup_ip_in_cube(int *cube, MA_set *map_root, int ip_thresh,
						   int *bc_done)
{
	MA_set	*map;
	int	i, ind, ip_num, sum;

	fprintf(stdout,"cleanup ip %d\n",ip_thresh);
	sum = 0;
	if( ip_thresh > 0 )
	{
		for( map = map_root;  map != NULL;  map = map->next )
		{
			if(    (map->att_mas[0] != NULL)
			    || (map->att_mas[1] != NULL)   ) continue;

			ip_num = map->num_in_set;
			if( ip_num >= ip_thresh ) continue;
			Cube_reset(0,ip_num,map->loc,0);
			sum++;
		}
	}
	fprintf(stderr,"\n%d isolated paths removed from MA\n",sum);
	fprintf(stdout,"\n%d isolated paths removed from MA\n",sum);
	*bc_done += sum;
}


void	cleanup_bl_ne_in_cube(int *cube, MA_set *map_root, MA_set *mac_root,
							  Mod_Menu *mod, int *bc_done, int nx, int nxy)
{
	MA_set	**map, *mac, *smap;
	MA_set	*mac_min, *smap_min;
	int	min;


	int	stencil[26];
	int	*l, *c_end;

	int	max_csz, max_coord;

	int	pMsz = sizeof(MA_set *);
	int	 isz = sizeof(int);

	if( mod->bl_mod ) set_stencil(nx,nxy,stencil);

	max_csz = max_coord = 0;
	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		if( mac->num_in_set > max_csz   ) max_csz   = mac->num_in_set;
		if( mac->num_att    > max_coord ) max_coord = mac->num_att;
	}

	map = (MA_set **)MALLOC(max_coord*pMsz);

	l = (int *)MALLOC(max_coord*isz);

	c_end = (int *)MALLOC(max_coord*isz);

	if( mod->bl_fast )
	{
		for( mac = mac_root;  mac != NULL;  mac = mac->next )
		{
			find_path_to_remove(cube,stencil,nx,nxy,mac,map,l,
								mod,&smap);

			if( smap == NULL ) continue;
		//	PRINTF("\nfast removing path %d\n",smap);
			if( mac->surf_rmnt == 1 )
				PRINTF("\tcluster is s.r.\n");

			remove_path(smap,mac,bc_done,max_coord,cube,c_end,
								nx,nxy);
		}
	}
	else
	{
		smap_min = NULL;
		mac_min = NULL;
		min = INT_MAX;
		for( mac = mac_root;  mac != NULL;  mac = mac->next )
		{
			find_path_to_remove(cube,stencil,nx,nxy,mac,map,l,
								mod,&smap);

			if( smap == NULL ) continue;

			if( smap->num_in_set < min )
			{
				mac_min = mac;
				smap_min = smap;
				min = smap->num_in_set;
			}
		}

		if( smap_min != NULL )
		{
			PRINTF("\nslow removing path %d\n",smap_min);

			remove_path(smap_min,mac_min,bc_done,max_coord,cube,
								c_end,nx,nxy);
		}
	}

	FREE(map   ,max_coord*pMsz);
	FREE(l     ,max_coord* isz);
	FREE(c_end ,max_coord* isz);
}


void	remove_path(MA_set *smap, MA_set *mac, int *bc_done, int max_coord,
					int *cube, int *c_end, int nx, int nxy)
{
	int	cnum;
	int	i, j, ns, ne, ind;
	int	pnum;
	int	*cloc;

	char	*msg;

	int	isz = sizeof(int);

	fprintf(stderr,"\tremoving path %d length %d\n",smap,smap->num_in_set);
	fprintf(stdout,"\tremoving path %d length %d\n",smap,smap->num_in_set);

	(*bc_done)++;

	/* unlink path from cube */
	pnum = smap->num_in_set;
	ns = (smap->att_mas[0] == NULL) ?    0 :      1;
	ne = (smap->att_mas[1] == NULL) ? pnum : pnum-1;

	Cube_reset(ns,ne,smap->loc,0)

	if( mac->num_in_set != 1 )
	{
			/* unlink cluster from cube */
		Cube_reset(0,mac->num_in_set,mac->loc,0)

			/* reduce cluster to path   */
		for( i = 0;  i < max_coord;  i++ ) c_end[i] = -1;

		for( i = 0, j = 0;  i < mac->num_att;  i++ )
		{
			ind = mac->attach_list[i];
			if( (ind == smap->loc[1]) ||
			    (ind == smap->loc[pnum-2]) )
				continue;
			else	c_end[j++] = ind;
		}

		cnum = mac->num_in_set+j;
		cloc = (int *)MALLOC(cnum*isz);
		msg = "cloc in cleanup_bl_ne_in_cube()";
		if( MEMCHECK(cloc,msg,cnum*isz) ) clean_up(0);

		for( i = 0;  i < j;     i++ ) cloc[i] = c_end[i];
		for( i = j;  i < cnum;  i++ ) cloc[i] = mac->loc[i-j];
		thin_bc_driver(&cloc,&cnum,c_end,j,nx,nxy);

			/* link thinned cluster to cube */
		Cube_reset(0,cnum,cloc,99)

		FREE(cloc,cnum*isz);
	}
}


/*
*	At each cluster we search, in order of preference, for
*	1. a needle-eye path (leaves and returns to same cluster)
*	2. the shortest leaf path.
*/


void	find_path_to_remove(int *cube, int *stencil, int nx, int nxy,
							MA_set *mac, MA_set **map, int *l,
							Mod_Menu *mod, MA_set **smap)
{
	int	i, max_bno, bl_thresh1, ne_thresh1;
	int	j, vox_ind, ind, keep, bdry_cnt;
	int	ix, iy, iz;
	int	js, je;

	MA_set	*longest_bdry_bl;

	*smap = NULL;

	if( mod->use_bno )
	{
	    max_bno = 0;
	    for( i = 0;  i < mac->num_in_set;  i++ )
	    {
			if ( max_bno < (int)mac->bdat[i] ) max_bno = (int)mac->bdat[i];
	    }
	}

		/* sort paths by length */
	for( i = 0;  i < mac->num_att;  i++ )
	{
		map[i] = mac->att_mas[i];
		  l[i] = map[i]->num_in_set;
	}
	if( mac->num_att > 1 ) cluster_path_heapsort(mac->num_att,l,map);

		/* Remove any needle-eye path first */
	if( mod->ne_mod )
	{
	    ne_thresh1 = ( mod->ne_thresh == -1 ) ? max_bno+1 : mod->ne_thresh;

	    for( i = 0;  ((i < mac->num_att) && (l[i] <= ne_thresh1));  i++ )
	    {
			if( (map[i]->att_mas[0] == mac) && (map[i]->att_mas[1] == mac) )
			{
				*smap = map[i];
				return;
			}
	    }
	}

		/* Remove branch-leaf paths */
	if( mod->bl_mod )
	{
	    bl_thresh1 = ( mod->bl_thresh == -1 ) ? max_bno+1 : mod->bl_thresh;

		if( mod->bl_keep_bdry )		/* count elegible bl paths */
		{
			bdry_cnt = 0;

			if( mod->bl_keep_z_bdry )
			{
				js = 0;	je = 26;
			}
			else
			{
				js = 9;	je = 17;
			}

			for( i = 0;  ((i < mac->num_att)&&(l[i] <= bl_thresh1));  i++ )
			{
				if(    (map[i]->att_mas[0] == NULL)
					|| (map[i]->att_mas[1] == NULL)    )
				{
					/***
					end = (map[i]->att_mas[0] == NULL)
							? 0 : map[i]->num_in_set - 1;
					***/
					for( ind = 0;  ind < map[i]->num_in_set;  ind++ )
					{
						vox_ind = map[i]->loc[ind];
						for( j = js;  j < je;  j++ )
						{
							if( cube[vox_ind+stencil[j]] == EXT_VAL )
							{
								bdry_cnt++;
								longest_bdry_bl = map[i];	/* map sorted */
								break;
							}
						}
						if( j != je ) break;
					}
				}
			}
	    }

	    for( i = 0;  ((i < mac->num_att) && (l[i] <= bl_thresh1));  i++ )
	    {
			if( (map[i]->att_mas[0] == NULL) ||
				(map[i]->att_mas[1] == NULL) )
			{
				keep = 0;

					/* if desired, retain longest branch-leaf */
					/* 	    path exiting boundary	  */
				if( mod->bl_keep_bdry )
				{
					/***
					end = (map[i]->att_mas[0] == NULL)
								? 0 : map[i]->num_in_set - 1;
					***/
					for( ind = 0;  ind < map[i]->num_in_set;  ind++ )
					{
						vox_ind = map[i]->loc[ind];
						for( j = js;  j < je;  j++ )
						{
							if( cube[vox_ind+stencil[j]] == EXT_VAL )
							{
								keep = 1;
								break;
							}
						}
						if( j != je ) break;
					}
				}
				/*****/
				Pad_ind_2_ijk(vox_ind,ix,iy,iz,nx,nxy)
				/*****/
				if( ! keep )
				{
					*smap = map[i];
					/****
					printf("Found %d bl path (%d,%d,%d)\n",*smap,
										ix,iy,iz);
					****/
					return;
				}
				else
				{
					if( bdry_cnt > 1 )    /* delete bdry bl anyhow */
					{
						*smap = longest_bdry_bl;
						/* *smap = map[i]; */
						/****
						printf("Found bdry bl path (%d,%d,%d)\n",
										ix,iy,iz);
						****/
						return;
					}
					/****
					else printf("Retaining bdry bl path (%d,%d,%d)\n",
										ix,iy,iz);
					****/
				}
			}
	    }
	}
}


void	thin_bc_driver(int **loc, int *numl, int *c_end, int numc, int nx,
					   int nxy)
{
	unsigned char	*wkspc;

	int	*w_end;
	int	min[3], max[3];

	int	wx, wy, wz, wxy, wxyz;
	int	n, i, j, k;
	int	wind;
	int	ns, ne, ctmp;
	int	numl_sav, nvox;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	set_index_limits(*loc,*numl,nx,nxy,min,max);

	wx = max[0] - min[0] + 3;	/* buffer wkspc cube */
	wy = max[1] - min[1] + 3;
	wz = max[2] - min[2] + 3;
	wxy = wx*wy;		wxyz = wxy*wz;

	wkspc = (unsigned char *)CALLOC(wxyz,usz);
	if( MEMCHECK(wkspc,"wkspc in thin_bc_driver()",wxyz*usz) ) clean_up(0);

	for( n = 0;  n < *numl;  n++ )
	{
		Pad_ind_2_ijk((*loc)[n],i,j,k,nx,nxy)

		i = i - min[0] + 1;	/* convert to wkspc indicies */
		j = j - min[1] + 1;
		k = k - min[2] + 1;
		wind = k*wxy + j*wx + i;

		wkspc[wind] = 1;
	}

	w_end = (int *)MALLOC(numc*isz);
	if( MEMCHECK(w_end,"w_end in thin_bc_driver()",numc*isz) ) clean_up(0);

	for( n = 0;  n < numc;  n++ )
	{
		Pad_ind_2_ijk(c_end[n],i,j,k,nx,nxy)

		i = i - min[0] + 1;	/* convert to wkspc indicies */
		j = j - min[1] + 1;
		k = k - min[2] + 1;
		wind = k*wxy + j*wx + i;

		w_end[n] = wind;
	}

	thin_branch_cluster(wx,wxy,wxyz,numc,w_end,wkspc,26,min,w_end[0],0,1);
	FREE(w_end,numc*isz);

	numl_sav = *numl;
	ns = 0;		ne = (*numl)-1;
	while( ns <= ne )
	{
		Pad_ind_2_ijk((*loc)[ns],i,j,k,nx,nxy)

		i = i - min[0] + 1;	/* convert to wkspc indicies */
		j = j - min[1] + 1;
		k = k - min[2] + 1;
		wind = k*wxy + j*wx + i;

		if( wkspc[wind] == 1 ) ns++;
		else
		{
			ctmp = (*loc)[ne];
			(*loc)[ne] = (*loc)[ns];	(*loc)[ns] = ctmp;

			ne--;	(*numl)--;
		}
	}
	if( *numl != ns )
	{
		fprintf(stderr,"Logic error in thin_bc_driver()\n");
		fprintf(stderr,"*numl (%d) != ns (%d)\n",*numl,ns);
		clean_up(0);
	}

	nvox = (*numl) - numl_sav;
	*loc = (int *)REALLOC(*loc,(*numl)*isz);
	if( MEMCHECK(*loc,"*loc realloc in thin_bc_driver()",nvox*isz) )
		clean_up(0);

	FREE(wkspc,wxyz*usz);
}


void	set_index_limits(int *loc, int numl, int nx, int nxy, int min[3],
						 int max[3])
{
	int	n, i, j, k;

	Pad_ind_2_ijk(loc[0],i,j,k,nx,nxy)
	min[0] = max[0] = i;  min[1] = max[1] = j;  min[2] = max[2] = k;

	for( n = 1;  n < numl;  n++ )
	{
		Pad_ind_2_ijk(loc[n],i,j,k,nx,nxy)

		     if( i < min[0] ) min[0] = i;
		else if( i > max[0] ) max[0] = i;

		     if( j < min[1] ) min[1] = j;
		else if( j > max[1] ) max[1] = j;

		     if( k < min[2] ) min[2] = k;
		else if( k > max[2] ) max[2] = k;
	}
}


void	reset_ma_list_from_cube(int *cube, int **pma_ind,
								unsigned char **pbdat, int *pn_ma_vox)
{
	unsigned char	btmp;

	int	ns, ne, ind;
	int	tot_ma_sav, dv;
	int	mtmp;

	tot_ma_sav = *pn_ma_vox;

	ns = 0;		ne = (*pn_ma_vox) - 1;
	while( ns <= ne )
	{
		ind = (*pma_ind)[ns];
		if( cube[ind] != 99 )
		{
			mtmp = (*pma_ind)[ne];  btmp = (*pbdat)[ne];
			(*pma_ind)[ne] = ind;   (*pbdat)[ne] = (*pbdat)[ns];
			(*pma_ind)[ns] = mtmp;  (*pbdat)[ns] = btmp;

			ne--;	(*pn_ma_vox)--;
		}
		else	ns++;
	}
	if( *pn_ma_vox != ns )
	{
		fprintf(stderr,"Logic error in reset_ma_list_from_cube()\n");
		fprintf(stderr,"*pn_ma_vox (%d) != ns (%d)\n",*pn_ma_vox,ns);
		clean_up(0);
	}

	dv = (*pn_ma_vox) - tot_ma_sav;
	ma_data_realloc(pma_ind,pbdat,*pn_ma_vox,dv);
}


#ifdef TST
void	output_modified_ma_files(int zs, int ze, int nx, int nxy, int nxyz,
								 int n_ma_vox, int *ma_list,
								 unsigned char *burn_dat, char *maf_name,
								 char *ext, int maf_len, int maf_cmprss,
								 int ts)
{
	FILE	*fp;
	char	*msg;

	unsigned char	*slc_bdat;

	int	*slc_ma, *ma_z;
	int	slc_nvox;
	int	j, k, n;
	int	ix, iy, iz, ind;
	int	ny, tnx, tny;
	int	fnlen;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	if( ts > 0 )
	{
		ts--;	incfname(maf_name,maf_len,&ts);
	}
	else	maf_name[maf_len] = '\0';

	fnlen = strlen(maf_name);
	strcat(maf_name,".");

	fp = open_out_file(maf_name,ext,"w");

	ma_z = (int *)MALLOC(n_ma_vox*isz);
	msg = "ma_z in output_modified_ma_files()";
	if( MEMCHECK(ma_z,msg,n_ma_vox*isz) ) clean_up(0);

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		Pad_ind_2_ijk(ma_list[n],ix,iy,iz,nx,nxy)
		ma_z[n] = iz+zs;	/* zs <= ma_z[n] <= ze */
	}

	ny = nxy/nx;	tnx = nx-2;	tny = ny-2;
	write_ma_header(fp,tnx,tny,zs,ze);
	for( k = zs;  k <= ze;  k++ )
	{
		slc_nvox = 0;
		for( n = 0;  n < n_ma_vox;  n++ )
		{
			if( ma_z[n] == k ) slc_nvox++;
		}

		if( slc_nvox > 0 )
		{
			slc_ma = (int *)MALLOC(slc_nvox*isz);
			msg = "slc_ma in output_modified_ma_files()";
			if( MEMCHECK(slc_ma,msg,slc_nvox*isz) ) clean_up(0);

			slc_bdat = (unsigned char *)MALLOC(slc_nvox*usz);
			msg = "slc_bdat in output_modified_ma_files()";
			if( MEMCHECK(slc_bdat,msg,slc_nvox*usz) ) clean_up(0);

			j = 0;
			for( n = 0;  n < n_ma_vox;  n++ )
			{
			    if( ma_z[n] == k )
			    {
				Pad_ind_2_ijk(ma_list[n],ix,iy,iz,nx,nxy)
				ind = iy*tnx + ix;
				slc_ma[j]   = ind;
				slc_bdat[j] = burn_dat[n];
				j++;
			    }
			}
		}
		else
		{
			slc_ma   = (int           *)NULL;
			slc_bdat = (unsigned char *)NULL;

		}

			/* slc_ma and slc_bdat freed in write_ma_file() */
		write_ma_fp(fp,slc_nvox,slc_ma,slc_bdat);
	}

	FREE(ma_z,n_ma_vox*isz);
	close_out_file(fp,maf_name,ext,maf_cmprss);
	maf_name[fnlen] = '\0';
}
#endif


#ifdef TST
void	project_modified_ma_files(int nx, int nxy, int nxyz, int n_ma_vox,
								  int *ma_list, unsigned char *burn_dat,
								  int *cube, char *out_name, int out_len,
								  int out_cmprss, int ts)
{
	unsigned char	*proj_cube, *proj_bdat, *cmap;

	int	bmin, bmax;
	int	cmap_len, bground, def_col;

	int	usz = sizeof(unsigned char);

	project_ma(cube,ma_list,burn_dat,nx,nxy,nxyz,n_ma_vox,
				&proj_cube,&proj_bdat,&bmin,&bmax);

	if( ts > 0 )
	{
		ts--;	incfname(out_name,out_len,&ts);
	}
	else	out_name[out_len] = '\0';

	strcat(out_name,".");

	create_min_2_max_colormap(bmin,bmax,&cmap,&cmap_len,&bground,&def_col);

	usz_pma_2_ras(proj_cube,proj_bdat,1,1,nx,nxy/nx,nxy,nxy*3,out_name,
				1,1,"n",NULL,cmap,cmap_len,bground,def_col);

	FREE(proj_cube,3*nxy*usz);
	FREE(proj_bdat,3*nxy*usz);

	out_name[out_len] = '\0';
	strcat(out_name,".");
}
#endif


void	project_ma(int *cube, int *ma_list, unsigned char *burn_dat, int nx,
				   int nxy, int nxyz, int n_ma_vox,
				   unsigned char **proj_cube, unsigned char **proj_bdat,
				   int *bmin, int *bmax)
{
	unsigned char	*pcube, *pbdat;

	int	i, k, knxy, nz, n_grain = 0;
	char	*msg;

	int	usz = sizeof(unsigned char);

	*proj_cube = (unsigned char *)CALLOC(3*nxy,usz);
	msg = "*proj_cube in project_ma()";
	if( MEMCHECK(*proj_cube,msg,3*nxy*usz) ) clean_up(0);

	*proj_bdat = (unsigned char *)CALLOC(3*nxy,usz);
	msg = "*proj_bdat in project_ma()";
	if( MEMCHECK(*proj_bdat,msg,3*nxy*usz) ) clean_up(0);

	nz = nxyz/nxy;
	pcube = *proj_cube;	pbdat = *proj_bdat;
	for( k = 0;  k < nz;  k++ )
	{
		knxy = k*nxy;
		for( i = 0;  i < nxy;  i++ )
		{
			/* pore */
			if( (cube[knxy+i] == 0) || (cube[knxy+i] == 99 ) ||
						   (cube[knxy+i] == 101) )
			{
				/***
				if( cube[knxy+i] == 101 )
					printf("101 at %d\n",knxy+i);
				***/
				pcube[nxy+i] = GRAIN;
				n_grain++;
			}
			else if( (cube[knxy+i] == EXT_VAL) && (k > 0   ) &&
							      (k < nz-1) )
			{
				pcube[nxy+i] = EXT_VAL;
			}
			/*** exterior
			else if( (cube[knxy+i] != GRAIN   ) &&
				 (cube[knxy+i] != GRN_BDRY) )
				printf("not grn or gbdry %d at %d\n",
						cube[knxy+i],knxy+i);
			***/
		}
	}

	/***
	for( i = 0;  i < nxy;  i++ )
		if( pcube[nxy+i] == GRAIN ) pcube[nxy+i] = 0;
		else			    pcube[nxy+i] = GRAIN;
	***/

	fprintf(stderr,"n grains in whole volume %d n_ma_vox %d\n",
						n_grain,n_ma_vox);
	fprintf(stdout,"n grains in whole volume %d n_ma_vox %d\n",
						n_grain,n_ma_vox);

	*bmin = burn_dat[0];	*bmax = burn_dat[0];
	for( i = 0;  i < n_ma_vox;  i++ )
	{
		if( *bmin > burn_dat[i] ) *bmin = burn_dat[i];
		if( *bmax < burn_dat[i] ) *bmax = burn_dat[i];
	}
	fprintf(stderr,"bmin %d bmax %d\n",*bmin,*bmax);
	fprintf(stdout,"bmin %d bmax %d\n",*bmin,*bmax);

	/* project largest burn number */
	for( i = 0;  i < n_ma_vox;  i++ )
	{
		k = ma_list[i] % nxy;
		pcube[nxy+k] = 99;
		/***
		if( burn_dat[i] == *bmax )
		    fprintf(stdout,"is bmax %d %d (%d %d %d) ind %d nx %d %d\n",
			*bmax,ma_list[i],k%nx,k/nx,ma_list[i]/nxy,i,nx,nxy);
		***/
		if( pbdat[nxy+k] < burn_dat[i] ) pbdat[nxy+k] = burn_dat[i];
	}
}


/* Some debugging printout routines */


void	print_int_array(char *msg, int *idat, int nx, int nxy, int nxyz)
{
	int	*pdat;
	int	ny, nz;
	int	i, j, k;

	ny = nxy/nx;	nz = nxyz/nxy;	pdat = idat;
	printf("%s\n",msg);
	for( k = 0;  k < nz;  k++ )
	{
		for( j = 0;  j < ny;  j++ )
		{
			for( i = 0;  i < nx;  i++, pdat++ )
			{
				printf(" %4d",*pdat);
			}
			printf("\n");
		}
		printf("\n");
	}
	printf("\n");
}


void	print_ma_nhbr_list(unsigned char *num_nhbr, int **nhbr_list,
						   int *ma_list, int n_ma_vox, int nx, int nxy)
{
	int	n, i, ix, iy, iz;

	printf("\nma_list\t\tnum_nhbr\tnhbr_list\n");
	for( n = 0;  n < n_ma_vox;  n++ )
	{
		Pad_ind_2_ijk(ma_list[n],ix,iy,iz,nx,nxy)
		printf("%d(%d %d %d)\t%d:\t",ma_list[n],ix,iy,iz,num_nhbr[n]);
		for( i = 0;  i < (int)(num_nhbr[n]);  i++ )
			printf(" %d",nhbr_list[n][i]);
		printf("\n");
	}
	printf("\n");
}
