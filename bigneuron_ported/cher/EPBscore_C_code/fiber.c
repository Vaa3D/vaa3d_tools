/*
*	Copyrighted, Research Foundation of SUNY, 1998
*/

#include "stdafx.h"
#include <string.h>
#include <math.h>
#include "maset.h"
#include "ma_prototype.h"



void	fiber_driver(char *file_basename)
{
	
	

	
	
	

	
	

	
	
	
	
	

	MA_set	*mas_root = NULL;
	MA_set	*map_root = NULL;
	MA_set	*mac_root = NULL;
	MA_set	*fbr_root = NULL;

	MA_set	*smap_root = NULL;

	char	title[80];
	char	*iv_fn;

	float	vox_len, dp_tol;

	int	nx, nxy, num_fbr;
	int	slct, vis, nz, longest, deleted;
	int	n_sngl, n_path, n_clust;

	int	isz = sizeof(int);
	int    pMsz = sizeof(MA_set *);
	int     Msz = sizeof(MA_set);
	int	usz = sizeof(unsigned char);


//	input_MA_sets(&mas_root,&map_root,&mac_root,&nx,&nxy,
//				&n_sngl,&n_path,&n_clust);

	PRINTFSTDERR("\nEnter maximum kink angle (degrees) a fiber can ");
	PRINTFSTDERR("sustain: ");	scanf("%f",&dp_tol);	getchar();
	PRINTFSTDOUT("\nEnter maximum kink angle (degrees) a fiber can ");
	PRINTFSTDOUT("sustain: %f\n",dp_tol);

	/*****
	dp_tol *= M_PI/180.0;	dp_tol = 1.0 + cos(dp_tol);

	PRINTFSTDERR("magnitude of kink angle tolerances is %f\n",dp_tol);
	PRINTFSTDOUT("magnitude of kink angle tolerances is %f\n",dp_tol);
	*****/

	PRINTFSTDERR("\nEnter data identifier for fiber histograms: ");
	gets(title);
	PRINTFSTDOUT("\nEnter data identifier for fiber histograms: %s\n",
								title);
//	set_voxel_phys_len(&vox_len);
//
//	init_visual_fibers(&vis,&nz,&longest,&iv_fn);

//	select_cp_set(&slct,&smap_root,mac_root,nx,nxy);

//	summarize_cluster_info(mac_root);

	hist_ma_len("Path",map_root,nx,nxy,vox_len,"\0",1,0);
	hist_clust_size(mac_root,nx,nxy,"\0",1,0);
	hist_coord_nums(mac_root,"\0",1,0);
	hist_iso_vox_bnum(mas_root,"\0",1,0);

//	dbg_print_MA_sets(mas_root,map_root,mac_root,nx,nxy);

	if( slct ) set_fibers(smap_root,mac_root,&fbr_root,nx,nxy,dp_tol,
								  &num_fbr,1);
	else	   set_fibers( map_root,mac_root,&fbr_root,nx,nxy,dp_tol,
								  &num_fbr,1);


	rec_free_MA_set(mas_root,isz,usz,Msz,pMsz);

	if( slct ) rec_free_MA_set(smap_root,isz,usz,Msz,pMsz);
	else	   rec_free_MA_set( map_root,isz,usz,Msz,pMsz);

	rec_free_MA_set(mac_root,isz,usz,Msz,pMsz);

	/***
	deleted = delete_leaving_fibers(fbr_root,nx,nxy);
	PRINTF("\n%d fibers left\n",num_fbr - deleted);
	***/

//	hist_ma_len("fiber",fbr_root,nx,nxy,vox_len,title,1,1);
//	hist_ma_tort_driver("fiber",fbr_root,nx,nxy,title);
//	hist_ma_moi_driver("fiber",fbr_root,nx,nxy,title);

	/*** This is used only for the fan of fibers ***
	sort_fiber_wrt_angle(num_fbr,fbr_root,90,nx,nxy);
	***/

	/***
	print_MA_sets("fiber","clusters/paths",fbr_root,nx,nxy);
	***/

	rec_free_MA_set(fbr_root,isz,usz,Msz,pMsz);
}


void  sort_fiber_wrt_angle(int num_fbr, MA_set *fbr_root, int cnt, int nx, int nxy)
{
        
	
	
	

        MA_set  *map, **fbr, **longfbr;

        int     ind, len, nis, e0, e1;
        int     *l, *loc;
        int     i, j, k, h;
	int	i0, j0, k0, i1, j1, k1;

        int     isz = sizeof(int);
        int    pMsz = sizeof(MA_set *);

        l = (int *)MALLOC(num_fbr*isz);
        if( MEMCHECK(l,"l in sort_fiber_wrt_angle()",num_fbr*isz) ) clean_up(0);

        fbr = (MA_set **)MALLOC(num_fbr*pMsz);
        if( MEMCHECK(fbr,"fbr in sort_fiber_wrt_angle()",num_fbr*pMsz) )
                clean_up(0);

        longfbr = (MA_set **)MALLOC(cnt*pMsz);
        if( MEMCHECK(longfbr,"longfbr in sort_fiber_wrt_angle()",cnt*pMsz) )
                clean_up(0);

        for( map = fbr_root, i = 0;  map != NULL;  map = map->next, i++ )
          {
                fbr[i] = map;
                  l[i] = fbr[i]->num_in_set;
          }
        if( num_fbr > 1 )       cluster_path_heapsort(num_fbr,l,fbr);
        FREE(l  ,num_fbr*isz);

        l = (int *)MALLOC(cnt*isz);
        if( MEMCHECK(l,"l in sort_fiber_wrt_angle()",cnt*isz) ) clean_up(0);

        for( h = 1;  h <= cnt;  h++ )
	  {
               longfbr[h-1] = fbr[num_fbr-h];
               nis = (longfbr[h-1])->num_in_set;

               e0 = (longfbr[h-1])->loc[0];
	       Pad_ind_2_ijk(e0,i0,j0,k0,nx,nxy)
	       e0 = i0 + j0;

               e1 = (longfbr[h-1])->loc[nis-1];
	       Pad_ind_2_ijk(e1,i1,j1,k1,nx,nxy)
	       e1 = i1 + j1;

	       l[h-1]   = ( e0 < e1 ) ? i0 : i1;
	  }
        FREE(fbr,num_fbr*pMsz);

        cluster_path_heapsort(cnt,l,longfbr);

        for( h = cnt-1;  h >= 0;  h-- )
	  {
	  	nis = (longfbr[h])->num_in_set;
                Pad_ind_2_ijk((longfbr[h])->loc[0],i,j,k,nx,nxy)
		PRINTF("\n %d %g fiber[%d] = (%3d,%3d,%3d)",cnt-h,
		                   (longfbr[h])->len,h,i,j,k);
		Pad_ind_2_ijk((longfbr[h])->loc[nis-1],i,j,k,nx,nxy)
                PRINTF("->(%3d,%3d,%3d), ind %d",i,j,k,(longfbr[h])->ind);
	  }
	FREE(l  ,cnt*isz);
        FREE(longfbr,cnt*pMsz);

}
/*

void	init_visual_fibers(int *vis, int *nz, int *longest, char **iv_fn)
{
	

	char	ans[5];

	PRINTFSTDERR("Visualize identified fibers? (y,n(dflt)): ");
	gets(ans);	PRINTFSTDERR("\n");

	PRINTFSTDOUT("Visualize identified fibers? (y,n(dflt)): %s\n",ans);

	*nz = 0;	*longest = 0;
	if( ans[0] == 'y' )
	{
		*vis = 1;

		PRINTFSTDERR("\nEnter the data size in z direction: ");
		scanf("%d",nz);		getchar();
	
		PRINTFSTDOUT("\nEnter the data size in z direction");
		PRINTFSTDOUT(": %d\n",*nz);

		PRINTFSTDERR("\nVisualize `n' longest fibers ");
		PRINTFSTDERR("\nEnter `n': ");
		scanf("%d",longest);	getchar();
	
		PRINTFSTDOUT("\nVisualize `n' longest fibers ");
		PRINTFSTDOUT("\nEnter `n': %d\n",*longest);

		*iv_fn = input_filename_base("inventor");
	}
	else	*vis = 0;

	return;
}
*/

void	select_cp_set(int *slct, MA_set **smap_root, MA_set *mac_root, int nx, int nxy)
{
	
	

	
	
	
	
	MA_set	**map, *mac;

	int	i, j, k, skip;
	int	na, n_path = 0;

	int	pMsz = sizeof(MA_set *);

	char	ans[5];
	char	*msg;

	PRINTFSTDERR("Select clusters and their attached paths? (y,n(dflt)): ");
	gets(ans);	PRINTFSTDERR("\n");

	PRINTFSTDOUT("Select clusters and their attached paths? (y,n(dflt)): ");
	PRINTFSTDOUT("%s\n",ans);

	if( ans[0] == 'y' )
	{
		*slct = 1;
		k = 0;
		for( mac = mac_root;  mac != NULL; mac = mac->next )
		{
			na = mac->num_att;
			n_path += na;
		}
		
		map = (MA_set **)MALLOC(n_path*pMsz);
		msg = "map in select_cp_set()";
		if( MEMCHECK(map,msg,n_path*pMsz) ) clean_up(0);
		
		for( mac = mac_root;  mac != NULL; mac = mac->next )
		{
			na = mac->num_att;
			for( j = 0;  j < na;  j++ )
			{
				skip = 0;
				for( i = 0;  i < k;  i++ )
				{
					if( map[i] == mac->att_mas[j] )
					{
				      		skip = 1;	break;
					}
				}
				if( !skip )
				{
					map[k] = mac->att_mas[j];
					map[k]->ind = k + 1;
					if( k == 0 )	map[k]->prev = NULL;
					else
					{
			         		map[k]->prev = map[k-1];
				 		map[k-1]->next = map[k];
					}
					k++;
		        	}
			}
		}
		map[k-1]->next = NULL;

		n_path = k;
		PRINTFSTDOUT("\n n_path = %d \n",n_path);
		*smap_root = map[0];
	}	
	else	*slct = 0;

}


int	delete_leaving_fibers(MA_set *fbr_root, int nx, int nxy)
{
	MA_set	*map;
	int	s, n, cnt;
	int	ind, s_z, e_z;
	int	i, j, k;

	cnt = 0;

	for( map = fbr_root, s = 0;  map != NULL;  map = map->next, s++ )
	{
		n = map->num_in_set;

		ind = map->loc[0];
		Ind_2_ijk(ind,i,j,k,nx,nxy)
		s_z = k;

		ind = map->loc[n-1];	
		Ind_2_ijk(ind,i,j,k,nx,nxy)
		e_z = k;
		
		/*** Hyunmi: FIX THIS ***/
		/*** 196 was chosen just for the case of 200 slice run ***/
		/*** the number should be adjusted depending on the    ***/
		/*** number of slices                                  ***/

		if( (s_z > 196) || (e_z > 196) )
		{
			PRINTF("fiber %d, len %g, ni %d",map->ind,map->len,
			       map->num_in_set);
			PRINTF("\tstarting z %d, ending z %d\n",s_z,e_z);
			if( map->prev != NULL ) (map->prev)->next = map->next;
			if( map->next != NULL ) (map->next)->prev = map->prev;
			cnt++;
		}
	}
	PRINTF("\nTotal %d fibers leaving the boundary deleted\n",cnt);
	return(cnt);
}


/*
*	Merge paths into `fibers' assuming springy fibers which resist bending
*/

#define	Is_ll_path(map)\
	( (map->att_mas[0] == NULL) && (map->att_mas[1] == NULL) )


#define	Is_lb_path(map)\
	(    ( (map->att_mas[0] == NULL) && (map->att_mas[1] != NULL) )\
	  || ( (map->att_mas[0] != NULL) && (map->att_mas[1] == NULL) )   )


void	set_fibers(MA_set *map_root, MA_set *mac_root, MA_set **fbr_root, int nx, int nxy, float dp_tol, int *num_fbr, int bb_fiber)
{
	MA_set	*map, *mac;
	MA_set	*prev_fbr = NULL, *fbr;

	char	msg[80];

	int	i, fbr_num;
	int	num_maps;
	int	realloc_sz, tot_sz;
	int	unused, with_one, done, loop;

	int	dbg = 0;

	int	isz = sizeof(int);
	int    pMsz = sizeof(MA_set *);
	int	usz = sizeof(unsigned char);

	realloc_sz = nx;

		/* use  MA_set->on_tree  variable to indicate */
		/* whether path has been assigned to a fiber  */

	num_maps = 0;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		map->on_tree = 0;
		num_maps++;
	}
	PRINTFSTDOUT("%d unassigned paths\n",num_maps);

	for( mac = mac_root;  mac != NULL;  mac = mac->next )
	{
		mac->on_tree = 0;
	}

	fbr_num = 1;
	for( map = map_root;  map != NULL;  map = map->next )
	{
	    if( map->on_tree ) continue;	/* already assigned to fiber */

	    if( Is_ll_path(map) )
	    {
		map->on_tree = 1;

		fbr = create_next_MA_set(fbr_root,prev_fbr);

		fbr->num_in_set = map->num_in_set;
		fbr->num_att = 3;

		alloc_loc_bdry_arrays(fbr);
		alloc_att_mas_array(fbr);

		fbr->att_mas[0] = (MA_set *)NULL;
		fbr->att_mas[1] = map;
		fbr->att_mas[2] = (MA_set *)NULL;

		for( i = 0;  i < fbr->num_in_set;  i++ )
		{
			fbr->loc[i]  = map->loc[i];
			fbr->bdat[i] = map->bdat[i];
		}
		if( dbg ) print_MAS("ll_fiber","clusters/paths",
					    fbr,fbr_num,nx,nxy);
		fbr->ind = fbr_num;
		fbr_num++;	prev_fbr = fbr;

	    }
	    else if( Is_lb_path(map) )
	    {
		fbr = create_next_MA_set(fbr_root,prev_fbr);

		fbr->num_in_set = tot_sz = realloc_sz;
		alloc_loc_bdry_arrays(fbr);
		fbr->num_in_set = 0;

		fbr->num_att = 1;	/* count (NULL) start cluster */

		rec_fiber((MA_set *)NULL,map,fbr,&tot_sz,realloc_sz,nx,nxy,dbg,
								       dp_tol);

		fbr->att_mas[0] = (MA_set *)NULL;
		if( dbg ) print_MAS("lb_fiber","clusters/paths",
					    fbr,fbr_num,nx,nxy);

		fbr->ind = fbr_num;
		fbr_num++;	prev_fbr = fbr;
	    }
	}

		/* current count of unused maps */
//	print_unassigned_map_count(map_root,"After ll and lb fibers set");
	hist_clust_unassigned_paths(mac_root,&with_one);
//	PRINTFSTDERR("%d clusters have single unassigned path\n",with_one);
	PRINTFSTDOUT("%d clusters have single unassigned path\n",with_one);

		/* only bb_paths left unassigned to fibers */
		/* Such fibers must begin on a cluster having */
		/* an odd number of unassigned fibers */
		/* recurse through the clusters, picking up only */
		/* fibers that have a single unassisned map */

	if( bb_fiber )
	{
	    done = 0;	loop = 0;
	    while( ! done )
	    {
		for( mac = mac_root;  mac != NULL;  mac = mac->next )
		{
		    unused = 0;	map = (MA_set *)NULL;
		    for( i = 0;  i < mac->num_att;  i++ )
		    {
			if( mac->att_mas[i]->on_tree ) continue;
			if( unused == 0 ) map = mac->att_mas[i];
			unused++;
		    }
		    if( unused != 1 ) continue;

		    fbr = create_next_MA_set(fbr_root,prev_fbr);

		    fbr->num_in_set = tot_sz = realloc_sz;
		    alloc_loc_bdry_arrays(fbr);
		    fbr->num_in_set = 0;

		    fbr->num_att = 1;	/* count start cluster */

		    rec_fiber(mac,map,fbr,&tot_sz,realloc_sz,nx,nxy,dbg,dp_tol);

		    fbr->att_mas[0] = mac;
		    if( dbg ) print_MAS("bb_fiber","clusters/paths",
						fbr,fbr_num,nx,nxy);

		    fbr->ind = fbr_num;
		    fbr_num++;	prev_fbr = fbr;
		}

		/* current count of unused maps */

//		PRINTF(msg,"After bb fiber loop %d",++loop);
//		print_unassigned_map_count(map_root,msg);
		hist_clust_unassigned_paths(mac_root,&with_one);

		PRINTFSTDOUT("%d clusters have single unassigned path\n", with_one);

		if( with_one == 0 ) done = 1;
	    }
	}

	PRINTFSTDOUT("\n\t%d fibers found\n",fbr_num-1);
	*num_fbr = fbr_num-1;

	set_path_len(*fbr_root,nx,nxy);
}


void	rec_fiber(MA_set *strt_clust, MA_set *map, MA_set *fbr, int *tot_sz, int realloc_sz, int nx, int nxy, int dbg, float dp_tol)
{
	MA_set	*end_clust, *next_map;
	int	p_ind, ec_ind, sv_ind;
	int	n;
	int	status;

	p_ind = fbr->num_att;       (fbr->num_att)++;	/* count map */

	if( dbg ) PRINTF("rec_fbr: strt_clust %d map %d p_ind %d num_att %d\n",
				strt_clust,map,p_ind,fbr->num_att);

		/* add path info to fiber loc/bdry arrays */
	add_map_2_fiber(fbr,strt_clust,map,tot_sz,realloc_sz);

		/* get and check next cluster */
	end_clust = (strt_clust == map->att_mas[0]) ? map->att_mas[1]
						    : map->att_mas[0];
	if( dbg ) PRINTF("end_clust %d\n",end_clust);

	if( end_clust == NULL )		/* fiber ends on bl path */
	{				/* count (NULL) end cluster */
		ec_ind = fbr->num_att;    (fbr->num_att)++;
		final_set_loc_bdry_arrays(fbr,tot_sz);
		alloc_att_mas_array(fbr);
		goto set_att;
	}
	else if( end_clust == strt_clust ) /* fiber ends with a 'needle eye' */
	{				   /* count end cluster */
		ec_ind = fbr->num_att;    (fbr->num_att)++;
		final_set_loc_bdry_arrays(fbr,tot_sz);
		alloc_att_mas_array(fbr);
		goto set_att;
	}

	ec_ind = fbr->num_att;    (fbr->num_att)++;	/* count end cluster */
	if( dbg ) PRINTF("ec_ind %d num_att %d\n",ec_ind,fbr->num_att);

	next_map = continue_fiber_past_clust(fbr,map,end_clust,nx,nxy,dp_tol);
	if( dbg ) PRINTF("next_map %d\n",next_map);

	if( next_map == NULL )		/* fiber ends at end_clust */
	{
		add_last_voxel_in_map_2_fbr(fbr,end_clust,map,tot_sz,realloc_sz);
		final_set_loc_bdry_arrays(fbr,tot_sz);
		alloc_att_mas_array(fbr);
		goto set_att;
	}

	status = set_fiber_thru_cluster(fbr,end_clust,map,next_map,tot_sz,
							realloc_sz,nx,nxy);
	if( status == 0 )	/* terminate fiber at cluster */
	{
		final_set_loc_bdry_arrays(fbr,tot_sz);
		alloc_att_mas_array(fbr);
		goto set_att;
	}

	rec_fiber(end_clust,next_map,fbr,tot_sz,realloc_sz,nx,nxy,dbg,dp_tol);

    set_att:

		/* add path and end_clust to att_mas array */
	if( dbg ) PRINTF("ec_ind %d clust %d p_ind %d map %d\n",
					ec_ind,end_clust,p_ind,map);
	fbr->att_mas[ec_ind] = end_clust;
	fbr->att_mas[ p_ind] = map;
}


MA_set	*continue_fiber_past_clust(MA_set *fbr, MA_set *map, MA_set *mac, int nx, int nxy, float dp_tol)
{
	

	int	i, n, na;

	na = mac->num_att;

	if( na == 2 )
	{
	  PRINTFSTDOUT("2 attached paths--continuation obvious\n");
		for( i = 0;  i < 2;  i++ )
		{
		    if( mac->att_mas[i]->on_tree == 0 ) return mac->att_mas[i];
		}
		return (MA_set *)NULL;
	}

	/* path->num_cinf is used to record the index of   */
	/* the path that will be connected to form a fiber */

	if( mac->on_tree )
	{
		n = map->num_cinf;
	}
	else if( !mac->on_tree )
	{
		match_connecting_paths(mac,nx,nxy,dp_tol);
		for( i = 0;  i < na;  i++ )
		{
		   if( mac->att_mas[i] == map ) n = mac->att_mas[i]->num_cinf;
		   else 			continue;
	        }
	}

	if( n >= na ) return (MA_set *)NULL;
	else	      return mac->att_mas[n];
}


void	match_connecting_paths(MA_set *mac, int nx, int nxy, float dp_tol)
{
	MA_set	*map_in, *map_out;
	float	*angl, **pair;
	float	angl_sum;
	int	*index;
	int	i_index, o_index, sten, min_k;
	int	i, j, k, s, n, n_angl, bnum, tmp_bnum;

	int    pisz = sizeof(int *);
	int	isz = sizeof(int);
	int	fsz = sizeof(float);
	
	//PRINTFSTDOUT("\nentered match_connecting_paths()\n");
	n = mac->num_att;
	n_angl = n*(n-1)/2;

	angl = (float *) MALLOC(n_angl*fsz);
	if( MEMCHECK(angl,"angl in match_connecting_paths()",n_angl*fsz) )
		clean_up(0);

	index = (int *) MALLOC(n_angl*isz);
	if( MEMCHECK(index,"index in match_connecting_paths()",n_angl*isz) )
		clean_up(0);

	for( i = 0;  i < n_angl;  i++ )		angl[i] = 0.;

	k = 0;
	for( i = 0;  i < n-1;  i++ )
	{
		map_in  = mac->att_mas[i];
		bnum = max_bnum_in_maset(map_in);
		bnum = ( bnum > 6 ) ? bnum : 6;
		map_in->num_cinf = n+1;
		for( j = i+1; j < n;  j++, k++ )
		{
			map_out = mac->att_mas[j];
			tmp_bnum = max_bnum_in_maset(map_out);
			bnum = ( bnum >= tmp_bnum ) ? bnum : tmp_bnum;
			index[k] = i*n + j;

			for( s = 0, sten = bnum;  s < 3;  s++, sten += bnum )
			{
			  angl[k] += get_angle_between_paths(mac,map_in,map_out,
							     sten,nx,nxy);
		        }
			angl[k] /= 3.;
			/***
			PRINTF("\nangle[%d] %g\t",k,angl[k]);
			***/
		}
	}
	mac->att_mas[n-1]->num_cinf = n+1;
	float_int_heapsort(n_angl,angl,index);
	angl_sum = 0.;

	for( k = n_angl - 1;  k >= 0;  k-- )
	{
		/**
		   PRINTFSTDOUT("angl[%d] = %f (ind %d -> %d with %d )\n",k,angl[k],
		   index[k],index[k]/n,index[k]%n);
		**/
	        min_k = k;
		if(angl[k] < dp_tol)	  break;
	}
	/**
	PRINTFSTDOUT("out of loop; k = %d compared to max %d (cutoff %f)\n",
		min_k,n_angl-1,dp_tol);
	**/

	for( k = n_angl - 1;  k > min_k;  k-- )
	{
		i_index = index[k] / n;
		o_index = index[k] % n;
		if(  (mac->att_mas[i_index]->num_cinf != n+1)
		   ||(mac->att_mas[o_index]->num_cinf != n+1) )	continue;

		mac->att_mas[i_index]->num_cinf = o_index;
		mac->att_mas[o_index]->num_cinf = i_index;
		angl_sum += angl[k];
	}
	/***** PRINTF("sum %g\n",angl_sum); *****/

	FREE(index,n_angl*isz);
	FREE(angl, n_angl*fsz);
}


float	get_angle_between_paths(MA_set *mac, MA_set *map_in, MA_set *map_out, int sten, int nx, int nxy)
{
	float	*vec_in, *vec_out;
	float	t_in[3], t_out[3];
	float	dp, acos_dp;
	int	sten_in, sten_out;

	char	*msg;

	int	fsz = sizeof(float);

	vec_in  = (float *)MALLOC(3*sten*fsz);
	vec_out = (float *)MALLOC(3*sten*fsz);
	msg = "vec_in  in get_angle_between_paths()";
	if( MEMCHECK(vec_in ,msg,3*sten*fsz) ) clean_up(0);
	msg = "vec_out in get_angle_between_paths()";
	if( MEMCHECK(vec_out,msg,3*sten*fsz) ) clean_up(0);

	sten_in = sten;
	set_path_vec(0,map_in,mac,&sten_in,nx,nxy,vec_in,t_in);
	
	sten_out = sten;
	set_path_vec(0,map_out,mac,&sten_out,nx,nxy,vec_out,t_out);

	dp = t_in[0]*t_out[0] + t_in[1]*t_out[1] + t_in[2]*t_out[2];

	if( dp < -1. )	dp = -1;
	if( dp >  1. )  dp = 1;
	acos_dp = acos(dp) * (180/3.1415926535);

	FREE(vec_in ,3*sten*fsz);
        FREE(vec_out,3*sten*fsz);

	return	acos_dp;
}


/*
*	compute path angle thru cluster using following idea
*
*
*				   vec_out
*			      ---------------->
*			       ^
*			      /
*			     / vec_mid
*			    /
*		<------------
*		  vec_in
*
*	dp_in  = (vec_in, vec_mid); 	dp_out = (vec_in,-vec_mid);
*
*	Choose that vec_out for which dp_in + dp_out has the closest value to -2;
*/

MA_set	*set_path_by_sten(MA_set *fbr, MA_set *mac, int sten, int nx, int nxy, float dp_tol)
{
	MA_set	*map_out, *map;

	float	t_in[3], t_out[3];
	float	*vec_in, *vec_out;

	float	dp_in, dp_out, dp, dp_max;
	float	tlen;

	int	i, j, first;
	int	sten_in, sten_out;
	int	fsz = sizeof(float);

	vec_in  = (float *)MALLOC(3*sten*fsz);
	vec_out = (float *)MALLOC(3*sten*fsz);
	if( MEMCHECK(vec_in ,"vec_in in set_path_by_sten()" ,sten*fsz) )
		clean_up(0);
	if( MEMCHECK(vec_out,"vec_out in set_path_by_sten()",sten*fsz) )
		clean_up(0);

	/*****	PRINTF("\nfbr %d\n",fbr); *****/

	sten_in = sten;
	set_path_vec(1,fbr,mac,&sten_in,nx,nxy,vec_in,t_in);

	/*****	PRINTF("\tt_in (%g,%g,%g)\n",t_in[0],t_in[1],t_in[2]); *****/

	first = 1;
	map_out = (MA_set *)NULL;
	for( i = 0;  i < mac->num_att;  i++ )
	{
		map = mac->att_mas[i];
		if( map->on_tree ) continue;

		/***** PRINTF("\nmap %d\n",map); *****/
		sten_out = sten;
		set_path_vec(0,map,mac,&sten_out,nx,nxy,vec_out,t_out);

		/*****
		PRINTF("\tt_out (%g,%g,%g)\n",t_out[0],t_out[1],t_out[2]);
		*****/

		dp = t_in[0]*t_out[0] + t_in[1]*t_out[1] + t_in[2]*t_out[2];

		if( fabs(1.0-dp) >= dp_tol )
		{
			if( first )
			{
				first = 0;
				dp_max = dp;	map_out = map;
			}
			else
			{
				if( fabs(1.0-dp) > fabs(1.0-dp_max) )
				{
					dp_max = dp;	map_out = map;
				}
			}
		}
	}
	FREE(vec_in ,3*sten*fsz);
	FREE(vec_out,3*sten*fsz);

	/*****	PRINTF("\nreturning out_map %d\n",map_out); *****/
	return map_out;
}


void	dbg_print_angle(MA_set *map1, MA_set *map2, MA_set *mac, int sten, int nx, int nxy, float dp_tol)
{
	float	t_in[3], t_out[3];
	float	*vec_in, *vec_out;

	float	dp_in, dp_out, dp, acos_dp;
	float	tlen;

		int	i, j;
	int	sten_in, sten_out;
	int	fsz = sizeof(float);

	vec_in  = (float *)MALLOC(3*sten*fsz);
	vec_out = (float *)MALLOC(3*sten*fsz);
	if( MEMCHECK(vec_in ,"vec_in in dbg_print_angle()" ,sten*fsz) )
		clean_up(0);
	if( MEMCHECK(vec_out,"vec_out in dbg_print_angle()",sten*fsz) )
		clean_up(0);

	sten_in = sten;
	set_path_vec(0,map1,mac,&sten_in,nx,nxy,vec_in,t_in);
	PRINTF("\tt_in (%g,%g,%g)\n",t_in[0],t_in[1],t_in[2]);

	sten_out = sten;
	set_path_vec(0,map2,mac,&sten_out,nx,nxy,vec_out,t_out);
	PRINTF("\tt_out (%g,%g,%g)\n",t_out[0],t_out[1],t_out[2]);

	dp = t_in[0]*t_out[0] + t_in[1]*t_out[1] + t_in[2]*t_out[2];
	acos_dp = acos(dp) * (180/3.1415926535);

	PRINTF("\t1-dp = %g, dp_tol = %g\n",1-dp,dp_tol);
	PRINTF("\tdp = %g, theta = acos(dp) = %g\n",dp,acos_dp);

	FREE(vec_in ,3*sten*fsz);
	FREE(vec_out,3*sten*fsz);
}


#define	Set_vec()\
{\
	Pad_ind_2_ijk(map->loc[n],i,j,k,nx,nxy)\
	vec[mx++] = i;	vec[my++] = j;	vec[mz++] = k;\
}

void	set_path_vec(int is_fbr, MA_set *map, MA_set *mac, int *sten, int nx, int nxy, float *vec, float *t)
{
	

	float	x0, y0, z0;
	float	tlen;

	int	i, j, k, n, mx, my, mz;
	int	nis;

	nis = map->num_in_set;
	if( ! is_fbr )
	{
	    if( (map->att_mas[0] == mac) && (map->att_mas[1] == mac) ) /* loop */ 
	    {
		if( (*sten) > map->num_in_set/2 ) *sten = map->num_in_set/2 + 1;
	    }
	}

	if( (*sten) > nis ) *sten = nis;

	mx = 0;		my = mx+(*sten);	mz = my+(*sten);
	if( is_fbr )
	{
		for( n = nis-1;  n >= nis-(*sten);  n-- ) Set_vec()
	}
	else
	{
		if( map->att_mas[0] == mac )
	    	{
			for( n = 0;  n < (*sten);  n++ ) Set_vec()
	    	}
	    	else
	    	{
			for( n = nis-1;  n >= nis-(*sten);  n-- ) Set_vec()
	    	}
	}

	/****************
	{
		PRINTF("\tnis %d sten %d vec",nis,*sten);
		mx = 0;		my = mx+(*sten);	mz = my+(*sten);
		for( n = 0;  n < (*sten);  n++, mx++, my++, mz++ )
		{
			PRINTF(" (%g,%g,%g)",vec[mx],vec[my],vec[mz]);
		}
		PRINTF("\n");
	}
	****************/
/* lsq fit not working for now -- Sen
	if( (*sten) > 2 ) lsq_fit_t(*sten,vec,t);
	else					/* (*sten) == 2 */
	{
		for( i = 0;  i < 3;  i++ ) t[i] = vec[2*i+1] - vec[2*i];
	}

	tlen = sqrt( t[0]*t[0] + t[1]*t[1] + t[2]*t[2] );
	for( i = 0;  i < 3;  i++ ) t[i] = t[i]/tlen;
}


/*
*	for robustness, perform lsq fit using largest of 
*	dx, dy and dz as the independent variable
*/

void	lsq_fit_t(int sten, float *vec, float *t)
{
	

	float	dx, dy, dz, fdx, fdy, fdz;
	float	a, b, siga, sigb, chi_sq, q;

	int	mx, my, mz;

	mx = 0;	         dx = vec[mx+sten-1] - vec[mx];	   fdx = fabs(dx);
	my = mx+sten;    dy = vec[my+sten-1] - vec[my];	   fdy = fabs(dy);
	mz = my+sten;    dz = vec[mz+sten-1] - vec[mz];    fdz = fabs(dz);

	if( fdx >= fdy )
	{
		if( fdx >= fdz )	// dx largest 
		{
			t[0] = dx/fdx;
			fit(vec+mx,vec+my,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[1] = b*t[0];
			fit(vec+mx,vec+mz,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[2] = b*t[0];
		}
		else		// dz largest 
		{
			t[2] = dz/fdz;
			fit(vec+mz,vec+mx,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[0] = b*t[2];
			fit(vec+mz,vec+my,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[1] = b*t[2];
		}
	}
	else
	{
		if( fdy >= fdz )	// dy largest 
		{
			t[1] = dy/fdy;
			fit(vec+my,vec+mx,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[0] = b*t[1];
			fit(vec+my,vec+mz,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[2] = b*t[1];
		}
		else		// dz largest 
		{
			t[2] = dz/fdz;
			fit(vec+mz,vec+mx,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[0] = b*t[2];
			fit(vec+mz,vec+my,0,sten-1,(float *)NULL,0,&a,&b,
						&siga,&sigb,&chi_sq,&q);
			t[1] = b*t[2];
		}
	}
}

void	add_map_2_fiber(MA_set *fbr, MA_set *strt_clust, MA_set *map, int *tot_sz, int realloc_sz)
{
	

	int	sv_ind, needed;
	int	n, n0, n1;

	map->on_tree = 1;

	needed = map->num_in_set;

	if( map->att_mas[0] == NULL )   n0 = 0;
	else			      { n0 = 1;			   needed--; }
	if( map->att_mas[1] == NULL )   n1 = map->num_in_set-1;
	else			      { n1 = map->num_in_set-2;    needed--; }

		/* reset n0 or n1 if first map on fiber */
		/*  starting from a (non NULL) cluster  */

	if( (fbr->num_in_set == 0) && (strt_clust != NULL) )
	{
		if( strt_clust == map->att_mas[0] )
			{ n0 = 0;			needed++; }
		else    { n1 = map->num_in_set-1;	needed++; }
	}

	sv_ind = fbr->num_in_set;    fbr->num_in_set += needed;

	realloc_loc_bdry_arrays(fbr,needed,tot_sz,realloc_sz);

	if( strt_clust == map->att_mas[0] )
	{
		for( n = n0;  n <= n1;  n++ )
		{
			fbr-> loc[sv_ind+n-n0] = map->loc [n];
			fbr->bdat[sv_ind+n-n0] = map->bdat[n];
		}
	}
	else
	{
		for( n = n1;  n >= n0;  n-- )
		{
			fbr-> loc[sv_ind+n1-n] = map->loc [n];
			fbr->bdat[sv_ind+n1-n] = map->bdat[n];
		}
	}
}


void	add_last_voxel_in_map_2_fbr(MA_set *fbr, MA_set *end_clust, MA_set *map, int *tot_sz, int realloc_sz)
{
	

	int	n, n0, n1;
	int	sv_ind;
	int	needed = 1;

	if( end_clust == map->att_mas[0] ) { n0 = n1 = 0; }
	else				   { n0 = n1 = map->num_in_set-1; }

	sv_ind = fbr->num_in_set;    fbr->num_in_set += needed;

	realloc_loc_bdry_arrays(fbr,needed,tot_sz,realloc_sz);

	if( end_clust == map->att_mas[0] )
	{
		for( n = n0;  n <= n1;  n++ )
		{
			fbr-> loc[sv_ind+n-n0] = map->loc [n];
			fbr->bdat[sv_ind+n-n0] = map->bdat[n];
		}
	}
	else
	{
		for( n = n1;  n >= n0;  n-- )
		{
			fbr-> loc[sv_ind+n1-n] = map->loc [n];
			fbr->bdat[sv_ind+n1-n] = map->bdat[n];
		}
	}
}


void	alloc_att_mas_array(MA_set *mas)
{
	
	

	char	*msg;

	int	natt;

	int	pMsz = sizeof(MA_set *);

	natt = mas->num_att;
	mas->att_mas = (MA_set **)MALLOC(natt*pMsz);
	msg = "mas->att_mas in alloc_att_mas_array()";
	if( MEMCHECK(mas->att_mas,msg,natt*pMsz) ) clean_up(0);
}


void	alloc_loc_bdry_arrays(MA_set *mas)
{
	
	

	char	*msg;

	int	nvox;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	nvox = mas->num_in_set;
	mas->loc = (int *)MALLOC(nvox*isz);
	msg = "mas->loc in alloc_loc_bdry_arrays()";
	if( MEMCHECK(mas->loc,msg,nvox*isz) ) clean_up(0);

	mas->bdat = (unsigned char *)MALLOC(nvox*usz);
	msg = "mas->bdat in alloc_loc_bdry_arrays()";
	if( MEMCHECK(mas->bdat,msg,nvox*usz) ) clean_up(0);
}


void	realloc_loc_bdry_arrays(MA_set *mas, int needed, int *tot_sz, int realloc_sz)
{
	
	
	

	char	*msg;

	int	added;
	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	if( mas->num_in_set + needed <= (*tot_sz) ) return;

	added = 0;
	while( mas->num_in_set+needed > (*tot_sz)+added ) added += realloc_sz;
	(*tot_sz) += added;

	mas->loc = (int *)REALLOC(mas->loc,(*tot_sz)*isz);
	msg = "mas->loc in realloc_loc_bdry_arrays()";
	if( MEMCHECK(mas->loc,msg,added*isz) ) clean_up(0);

	mas->bdat = (unsigned char *)REALLOC(mas->bdat,(*tot_sz)*usz);
	msg = "mas->bdat in realloc_loc_bdry_arrays()";
	if( MEMCHECK(mas->bdat,msg,added*usz) ) clean_up(0);
}


void	final_set_loc_bdry_arrays(MA_set *mas, int *tot_sz)
{
	
	
	

	char	*msg;

	int	chopped;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	if( mas->num_in_set == (*tot_sz) ) return;

	chopped = mas->num_in_set - (*tot_sz);
	*tot_sz = mas->num_in_set;

	mas->loc = (int *)REALLOC(mas->loc,(*tot_sz)*isz);
	msg = "mas->loc in final_set_loc_bdry_arrays()";
	if( MEMCHECK(mas->loc,msg,chopped*isz) ) clean_up(0);

	mas->bdat = (unsigned char *)REALLOC(mas->bdat,(*tot_sz)*usz);
	msg = "mas->bdat in final_set_loc_bdry_arrays()";
	if( MEMCHECK(mas->bdat,msg,chopped*usz) ) clean_up(0);
}


int	set_fiber_thru_cluster(MA_set *fbr, MA_set *mac, MA_set *imap, MA_set *omap, int *tot_sz, int realloc_sz, int nx, int nxy)
{
	
	

	
	
	

	
	
	
	
	
	
	
	

	unsigned char	*cbdat;
	char	*msg;

	int	c_end[2], offset[27], *cloc;
	int	i, cnum;
	int	failed;

	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	c_end[0] = (imap->att_mas[0] == mac ) ? imap->loc[1]
					      : imap->loc[imap->num_in_set-2];
	c_end[1] = (omap->att_mas[0] == mac ) ? omap->loc[1]
					      : omap->loc[omap->num_in_set-2];

	cnum = mac->num_in_set+2;

	cloc = (int *)MALLOC(cnum*isz);
	msg = "cloc in set_fiber_thru_cluster()";
	if( MEMCHECK(cloc,msg,cnum*isz) ) clean_up(0);

	for( i = 0;  i < 2;     i++ ) cloc[i] = c_end[i];
	for( i = 2;  i < cnum;  i++ ) cloc[i] = mac->loc[i-2];

	if( mac->num_in_set > 1 )
	{
		if( mac->surf_rmnt )
		{
			/***
			PRINTF("SURFACE REMNANT before thinning\n");
			print_loc_set(cloc,cnum,nx,nxy);
			***/

			thin_surf_rmnt(&cloc,&cnum,nx,nxy);

			/***
			PRINTF("SURFACE REMNANT after thinning\n");
			print_loc_set(cloc,cnum,nx,nxy);
			***/
		}
		else
		{
			thin_bc_driver(&cloc,&cnum,c_end,2,nx,nxy);
		}

		if( mac->surf_rmnt )
		{
			set_nhbr_offsets(nx,nxy,offset);
			if( is_surf_rmnt(cloc,cnum,offset,nx,nxy) )
			{
				PRINTF("SURFACE REMNANT remains remnant ");
				PRINTF("after thinning\n");
				print_loc_set(cloc,cnum,nx,nxy);
				return 0;
			}
		}

	}
	failed = order_ind_on_path(cloc,cnum,nx,nxy);
	if( failed )
	{
		print_MAS("cluster","paths",mac,-1,nx,nxy);
		print_MAS("incoming path","clusters",imap,-1,nx,nxy);
		print_MAS("outgoing path","clusters",omap,-1,nx,nxy);

		/* load cloc[] again and debug thin_bc_driver */

		FREE(cloc,cnum*isz);

		cnum = mac->num_in_set+2;

		cloc = (int *)MALLOC(cnum*isz);
		msg = "cloc in set_fiber_thru_cluster()";
		if( MEMCHECK(cloc,msg,cnum*isz) ) clean_up(0);

		for( i = 0;  i < 2;     i++ ) cloc[i] = c_end[i];
		for( i = 2;  i < cnum;  i++ ) cloc[i] = mac->loc[i-2];

		PRINTF("\ncloc[] before thin_bc_driver()\n");
		print_loc_set(cloc,cnum,nx,nxy);

		thin_bc_driver(&cloc,&cnum,c_end,2,nx,nxy);

		PRINTF("\ncloc[] after thin_bc_driver()\n");
		print_loc_set(cloc,cnum,nx,nxy);

		FREE(cloc,cnum*isz);
		clean_up(0);	/* use return 0 ??? */
	}

	set_cbdat(mac,cloc,&cbdat,cnum,nx,nxy);

	add_cpath_2_fiber(fbr,c_end,cloc,cbdat,cnum,tot_sz,realloc_sz);

	FREE(cbdat,cnum*usz);
	FREE(cloc ,cnum*isz);

	return 1;
}


/*
*	Using a breadth-first tree-based search algorithm, this code
*	finds a short path from cloc[0] to cloc[1] through the rest
*	of the elements of cloc[2 ... cnum-1]
*/

void	thin_surf_rmnt(int **ploc, int *pnum, int nx, int nxy)
{
	

	
	
	
	
	

	char	*msg1, *msg2, *msg3;

	int	offset[27];
	int	*loc, *node, *par;

	int	n, num;
	int	l, ls, le;
	int	next_ind, leaf_ind;
	int	num_in_path, dnum;

	int	isz = sizeof(int);

	loc = *ploc;	num = *pnum;

	node = (int *)MALLOC(num*isz);
	par  = (int *)MALLOC(num*isz);

	msg1 = "node in set_path_thru_surf_rmnt()";
	msg2 = "par  in set_path_thru_surf_rmnt()";

	if( MEMCHECK(node,msg1,num*isz) ) clean_up(0);
	if( MEMCHECK(par ,msg2,num*isz) ) clean_up(0);

	node[0] = loc[0];	loc[0] = -1;	par[0]   = -1;

	ls = 0;		le = 1;		next_ind = 1;	leaf_ind = -1;

	set_nhbr_offsets(nx,nxy,offset);

	while( next_ind < num )
	{
	    for( l = ls;  l < le;  l++ )
	    {
		for( n = 0;  n < num;  n++ )
		{
			if( loc[n] == -1 ) continue;
			if( is_child(loc[n],node[l],offset) )
			{
				node[next_ind] = loc[n];	loc[n] = -1;
				 par[next_ind] = l;
				if( n == 1 )
				{
					leaf_ind = next_ind;
					goto found;
				}
				next_ind++;
			}
		}
	    }
	    ls = le;
	    le = next_ind;
	}

	if( leaf_ind == -1 )
	{
		PRINTFSTDERR("Logic error in set_path_thru_surf_rmnt()\n");
		PRINTFSTDOUT("Logic error in set_path_thru_surf_rmnt()\n");
		clean_up(0);
	}

    found:
		/* find size of path */
	l = leaf_ind;	num_in_path = 1;
	while( l != 0 ) { num_in_path++;	l = par[l]; }

	*pnum = num_in_path;	dnum = num_in_path - num;
	*ploc = (int *)REALLOC(*ploc,num_in_path*isz);
	msg3 = "*ploc in set_path_thru_surf_rmnt()";
	if( MEMCHECK(*ploc,msg3,dnum*isz) ) clean_up(0);

	l = leaf_ind;
	for( n = num_in_path-1;  n >= 0;  n-- )
	{
		(*ploc)[n] = node[l];	l = par[l];
	}

	FREE(node,num*isz);
	FREE(par ,num*isz);
}


int	is_child(int child, int par, int *offset)
{
	int	j;

	for( j = 0;  j < 27;  j++ )
	{
		if( par+offset[j] == child ) return 1;
	}
	return 0;
}


/*
*	Reorders a disordered array of voxel locations which form a path
*/

int	order_ind_on_path(int *loc, int num, int nx, int nxy)
{
	
	
	
	

	int	offset[27];
	int	*wksp;

	int	i, j, k;
	int	ne;
	int	add_2_strt, add_2_end;

	int	isz = sizeof(int);

	set_nhbr_offsets(nx,nxy,offset);

	wksp = (int *)MALLOC(num*isz);
	if( MEMCHECK(wksp,"wksp in order_ind_on_path()",num*isz) ) clean_up(0);
	for( i = 0;  i < num;  i++ ) wksp[i] = -1;

	ne = 0;
	wksp[0] = loc[0];	loc[0] = -1;

	add_2_end = 1;
	while( add_2_end )
	{
		add_2_end = 0;
		for( i = 1;  i < num;  i++ )
		{
			if( loc[i] == -1 ) continue;

			for( j = 0;  j < 27;  j++ )
			{
				if( wksp[ne]+offset[j] == loc[i] )
				{
					wksp[++ne] = loc[i];	loc[i] = -1;

					add_2_end = 1;
					break;
				}
			}
			if( add_2_end ) break;
		}
	}

	if( ne < num-1 )
	{
	    add_2_strt = 1;
	    while( add_2_strt )
	    {
		add_2_strt = 0;
		for( i = 1;  i < num;  i++ )
		{
			if( loc[i] == -1 ) continue;

			for( j = 0;  j < 27;  j++ )
			{
				if( wksp[0]+offset[j] == loc[i] )
				{
					ne++;
					for( k = ne;  k > 0;  k-- )
					{
						wksp[k] = wksp[k-1];
					}
					wksp[0] = loc[i];	loc[i] = -1;

					add_2_strt = 1;
					break;
				}
			}
			if( add_2_strt ) break;
		}
	    }
	}

	if( ne != num-1 )
	{
		

		PRINTFSTDERR("\nLogic error in order_ind_on_path()\n");
		PRINTFSTDOUT("\nLogic error in order_ind_on_path()\n");
		PRINTF("\n\tloc[]\n");
		print_loc_set(loc,num,nx,nxy);
		PRINTF("\n\twksp[]\n");
		print_loc_set(wksp,num,nx,nxy);

		return 1;
	}

	for( i = 0;  i < num;  i++ ) loc[i] = wksp[i];
	FREE(wksp,num*isz);

	return 0;
}


void	set_cbdat(MA_set *mac, int *cloc, unsigned char **cbdat, int cnum, int nx, int nxy)
{
	
	

	
	

	int	i, j;

	int	usz = sizeof(unsigned char);

	*cbdat = (unsigned char *)MALLOC(cnum*usz);
	if( MEMCHECK(*cbdat,"*cbdat in set_cbdat()",cnum*usz) ) clean_up(0);

	for( i = 0;  i < cnum;  i++ ) (*cbdat)[i] = 127;

	for( i = 1;  i < cnum-1;  i++ )
	{
		for( j = 0;  j < mac->num_in_set;  j++ )
		{
			if( mac->loc[j] == cloc[i] ) (*cbdat)[i] = mac->bdat[j];
		}
	}
	for( i = 1;  i < cnum-1;  i++ )
	{
		if( (*cbdat)[i] == 127 )
		{
			PRINTFSTDERR("\nLogic error in set_cbdat()\n");
			PRINTFSTDOUT("\nLogic error in set_cbdat()\n");

			PRINTF("\nProblematic Cluster\n");
			print_MAS("Cluster","paths",mac,mac->ind,nx,nxy);

			PRINTF("\ncloc[] after order_ind_on_path()\n");
			print_loc_set(cloc,cnum,nx,nxy);

			PRINTF("\ncbdat[]\n");
			for( j = 0;  j < cnum;  j++ )
			{
				PRINTF("%d ",(*cbdat)[j]);
				if( (j+1)%20 == 0 ) PRINTF("\n");
			}

			clean_up(0);
		}
	}
}


void	add_cpath_2_fiber(MA_set *fbr, int *c_end, int *cloc, unsigned char *cbdat, int num, int *tot_sz, int realloc_sz)
{
	
	

	int	n;
	int	term0, term1;
	int	sv_ind, needed;

	term0 = ( (c_end[0] == cloc[0]) && (c_end[1] == cloc[num-1]) );
	term1 = ( (c_end[1] == cloc[0]) && (c_end[0] == cloc[num-1]) );

	if( ! (term0 || term1) )
	{
		PRINTFSTDERR("\nLogic error in add_cpath_2_fiber()\n");
		PRINTFSTDOUT("\nLogic error in add_cpath_2_fiber()\n");
		clean_up(0);
	}

	needed = num-2;
	sv_ind = fbr->num_in_set;	fbr->num_in_set += needed;

	realloc_loc_bdry_arrays(fbr,needed,tot_sz,realloc_sz);

	if( term0 )
	{
		for( n = 1;  n <= needed;  n++ )
		{
			fbr-> loc[sv_ind+n-1] = cloc [n];
			fbr->bdat[sv_ind+n-1] = cbdat[n];
		}
	}
	else
	{
		for( n = needed;  n >= 1;  n-- )
		{
			fbr-> loc[sv_ind+n-1] = cloc [n];
			fbr->bdat[sv_ind+n-1] = cbdat[n];
		}
	}
}


void	print_unassigned_map_count(MA_set *map_root, char *msg)
{
	MA_set	*map;

	int	unused;

	unused = 0;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		if( ! map->on_tree ) unused++;
	}

	PRINTFSTDERR("\n%s\n\t%d paths unassigned to fibers\n",msg,unused);
	PRINTFSTDOUT("\n%s\n\t%d paths unassigned to fibers\n",msg,unused);
}

/*
void	hist_ma_moi_driver(char *ma_type, MA_set *map_root, int nx, int nxy, char *title)
{
	
	
	

	MoI	*moi_root;

	moi_root = compute_moi(map_root,nx,nxy);

	/*** print_MoI_list(moi_root,ma_type); **
	hist_pa_dirs(moi_root,ma_type,title);
	rec_free_MoI_list(moi_root,sizeof(MoI));
}
*/

MoI	*compute_moi(MA_set *map_root, int nx, int nxy)
{
	
	
	

	
	
	

	MoI	*moi_root = (MoI *)NULL;
	MoI	*prev_moi = (MoI *)NULL;
	MoI	*moi;

	MA_set	*map;
	char	*msg;

	double	comx, comy, comz;
	double	mxx, myy, mzz, mxy, myz, mxz;
	double	x, y, z;

	float	*iX, *iY, *iZ, *piX, *piY, *piZ;

	int	n, i, j, k;
	int	max_nvox, nvox;

	int	fsz = sizeof(float);

	max_nvox = map_root->num_in_set;
	for( map = map_root;  map != NULL;  map = map->next )
	{
		if( map->num_in_set > max_nvox ) max_nvox = map->num_in_set;
	}

	iX = (float *)MALLOC(max_nvox*fsz);
	iY = (float *)MALLOC(max_nvox*fsz);
	iZ = (float *)MALLOC(max_nvox*fsz);

	msg = "iX in hist_ma_moi_driver()";
	if( MEMCHECK(iX,msg,max_nvox*fsz) ) clean_up(0);
	msg = "iY in hist_ma_moi_driver()";
	if( MEMCHECK(iY,msg,max_nvox*fsz) ) clean_up(0);
	msg = "iZ in hist_ma_moi_driver()";
	if( MEMCHECK(iZ,msg,max_nvox*fsz) ) clean_up(0);

		/*   store voxel coordinates and  */
		/* set center of mass coordinates */

	for( map = map_root;  map != NULL;  map = map->next )
	{
	    moi = create_next_MoI(&moi_root,prev_moi);
	    comx = comy = comz = 0.0;
	    piX = iX;	piY = iY;	piZ = iZ;
	    nvox = map->num_in_set;
	    for( n = 0;  n < nvox;  n++ )
	    {
			Pad_ind_2_ijk(map->loc[n],i,j,k,nx,nxy)
			*piX = i+0.5;	*piY = j+0.5;	*piZ = k+0.5;
			comx += *piX;	comy += *piY;	comz += *piZ;
			piX++;		piY++;		piZ++;
	    }
	    comx = comx / (double)nvox;
	    comy = comy / (double)nvox;
	    comz = comz / (double)nvox;

		/* compute elements of Moment of Inertia matrix */
	    mxx = myy = mzz = mxy = myz = mxz = 0.0;
	    piX = iX;    piY = iY;    piZ = iZ;
	    for( n = 0;  n < nvox;  n ++, piX++, piY++, piZ++ )
	    {
		x = *piX - comx;    y = *piY - comy;    z = *piZ - comz;

		mxx += y*y + z*z;	mxy += x*y;
		myy += x*x + z*z;	myz += y*z;
		mzz += x*x + y*y;	mxz += x*z;
	    }
	    moi->comx = comx;    moi->comy = comy;    moi->comz = comz;

		/* Convert to moi/voxel */

	    mxx /= (float)nvox;    myy /= (float)nvox;    mzz /= (float)nvox;
	    mxy /= (float)nvox;    mxz /= (float)nvox;    myz /= (float)nvox;

	    moi->fI[0] =  mxx;    moi->fI[3] = -mxy;    moi->fI[6] = -mxz;
	    moi->fI[1] = -mxy;    moi->fI[4] =  myy;    moi->fI[7] = -myz;
	    moi->fI[2] = -mxz;    moi->fI[5] = -myz;    moi->fI[8] =  mzz;

	    prev_moi = moi;
	}

	FREE(iX,max_nvox*fsz);
	FREE(iY,max_nvox*fsz);
	FREE(iZ,max_nvox*fsz);

	compute_prin_ax(moi_root);
	return moi_root;
}

void	fit(float *x, float *y, int strt, int end, float *sig, int mwt,
			float *a, float *b, float *siga, float *sigb, float *chi2,
			float *q)
{

	int sumxx, sumxy;
	int i;
	
	//Hack by Sen to do a simple least mean square line fit

	sumxx=0;
	sumxy=0;
	for (i=strt;i<end;i++)
	{
		sumxx+=x[i]*x[i];
		sumxy+=x[i]*y[i];
	}
	*b = sumxy/sumxx;
}