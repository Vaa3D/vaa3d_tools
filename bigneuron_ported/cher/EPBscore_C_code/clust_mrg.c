/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include "maset.h"
#include "ma_prototype.h"


#define PE(_strng) fprintf(stderr,_strng)
#define PO(_strng) PRINTF(_strng)

void	set_CMrg_menu(CMrg_menu *cmrg)
{
	char	ans[5], spec[5];

	fprintf(stderr,"\nMerge close clusters? (y,n(dflt)): ");
	gets(ans);
	PRINTF("\nMerge close clusters? (y,n(dflt)): %s\n",ans);

	if( ans[0] == 'y' )
	{
		cmrg->do_cmrg = 1;
		cmrg->method  = 'a';
		cmrg->ntol    = 0;

		PE("Two clusters are considered close if separated by path ");
		PE("having fewer\nthan (or equal to) `ntol' voxels. `ntol' ");
		PE("can be determined\n\n");
		PE("\ta) automatically for each cluster pair using the ");
		PE("largest\n\t   burn number (local radius) in the cluster ");
		PE("pair.\n\n");
		PE("\tc) as a constant value specified by the user.\n\n");

		PO("Two clusters are considered close if separated by path ");
		PO("having fewer\nthan (or equal to) `ntol' voxels. `ntol' ");
		PO("can be determined\n\n");
		PO("\ta) automatically for each cluster pair using the ");
		PO("largest\n\t   burn number (local radius) in the cluster ");
		PO("pair.\n\n");
		PO("\tc) as a constant value specified by the user.\n\n");

		PE("Specify method for determining `ntol' (a(dflt),c): ");
		gets(spec);		cmrg->method = spec[0];
		PO("Specify method for determining `ntol' (a(dflt),c): ");
		PRINTF("%c\n",cmrg->method);

		if( cmrg->method == 'c' )
		{
			fprintf(stderr,"Specify `ntol': ");
			scanf("%d",&(cmrg->ntol));	getchar();
			PRINTF("Specify `ntol': %d\n",cmrg->ntol);
		}
	}
	else	cmrg->do_cmrg = 0;
	PE("\n");
	PO("\n");
}


void	merge_clusters(MA_set **mac_root, MA_set **map_root,
					   CMrg_menu *cmrg, int nx, int nxy, int *pn_clust,
					   int *pn_path)
{
	MA_set	*mac1, *mac2, *map;
	MA_set	*beg, *end;

	int	i;
	int	stat, merge;
	int	nt1, nt2, nt_m;

	int	dbg = 0;

	PRINTF("\n%d clusters and %d paths before merging clusters\n",
						*pn_clust,*pn_path);
	mac1 = *mac_root;
	while( mac1 != (MA_set *)NULL )
	{
	    merge = 0;
	    for( i = 0;  i < mac1->num_att;  i++ )
	    {
		map = mac1->att_mas[i];

		if( (beg = map->att_mas[0]) == NULL ) continue;
		if( (end = map->att_mas[1]) == NULL ) continue;

		if( cmrg->method == 'a' )   /* local length determination */
		{
			nt1 = max_bnum_in_maset(beg);
			nt2 = max_bnum_in_maset(end);
			nt_m = (nt1 > nt2) ? nt1 : nt2;
			nt_m += 2;	    /* path end points in clusters */
		}
		else	nt_m = cmrg->ntol;  /* user specified determination */

		if( map->num_in_set > nt_m ) continue;

		if( beg == end ) continue;
		mac2 = ( beg == mac1 ) ? end : beg;

		stat = merge_mac2_into_mac1(map_root,mac_root,mac1,mac2,
							nt_m,nx,nxy,dbg);

		if( !stat )
		{
			merge = 1;
			break;
		}
		else if( dbg )
		{
			PRINTF("Unable to merge clusters (stat %d)\n",
									stat);
		}
	    }
	    if( ! merge ) mac1 = mac1->next;
	}
	count_MA_sets(*mac_root,pn_clust);
	count_MA_sets(*map_root,pn_path);
	PRINTF("%d clusters and %d paths after merging clusters\n\n",
						*pn_clust,*pn_path);
}




int	merge_mac2_into_mac1(MA_set **map_root, MA_set **mac_root, MA_set *mac1,
						 MA_set *mac2, int ntol, int nx, int nxy, int dbg)
{
	MA_set	*map;
	MA_set	**map_sav, **map_conn, **map_tran;

	int	*mav_sav, *mav_tran;

	char	*msg;

	int	i, j, k;
	int	ns, nc, nt;
	int	new_num_att, new_num_in_set, del_nis;
	int	nump_sav, nump_conn, nump_tran;

	int      isz = sizeof(int);
	int      usz = sizeof(unsigned char);
	int      Msz = sizeof(MA_set);
	int     pMsz = sizeof(MA_set *);

	/* find all short paths connecting mac1 to mac2 */
	/* these will become part of mac1 */
	/* remaining paths on mac1 will stay in mac1->att_mas */
	/* remaining paths on mac2 will be transferred to mac1->att_mas */

	nump_conn = 0;
	for( i = 0;  i < mac2->num_att;  i++ )
	{
		map = mac2->att_mas[i];
		if(    ((map->att_mas[0] == mac1) || (map->att_mas[1] == mac1))
		    && (map->num_in_set <= ntol)    ) nump_conn++;
	}

	if( dbg )
	{
		PRINTF("\n\nMerging cluster %d with cluster %d\n",
							mac1,mac2);
		print_MAS("cluster","paths",mac1,1,nx,nxy);
		print_MAS("cluster","paths",mac2,2,nx,nxy);
	}

	if( nump_conn == 0 ) return 1;	/* clusters cannot be merged */
	nump_tran = mac2->num_att - nump_conn;
	nump_sav  = mac1->num_att - nump_conn;

	if( nump_sav > 0 )
	{
		map_sav = (MA_set **)MALLOC(nump_sav*pMsz);
		msg = "map_sav in merge_mac2_into_mac1()";
		if( MEMCHECK(map_sav,msg,nump_sav*pMsz) ) clean_up(0);

		mav_sav = (int *)MALLOC(nump_sav*isz);
		msg = "mav_sav in merge_mac2_into_mac1()";
		if( MEMCHECK(mav_sav,msg,nump_sav*isz) ) clean_up(0);
	}
	else
	{
		map_sav = (MA_set **)NULL;
		mav_sav = (int     *)NULL;
	}

	map_conn = (MA_set **)MALLOC(nump_conn*pMsz);
	msg = "map_conn in merge_mac2_into_mac1()";
	if( MEMCHECK(map_conn,msg,nump_conn*pMsz) ) clean_up(0);

	if( nump_tran > 0 )
	{
		map_tran = (MA_set **)MALLOC(nump_tran*pMsz);
		msg = "map_tran in merge_mac2_into_mac1()";
		if( MEMCHECK(map_tran,msg,nump_tran*pMsz) ) clean_up(0);

		mav_tran = (int *)MALLOC(nump_tran*isz);
		msg = "mav_tran in merge_mac2_into_mac1()";
		if( MEMCHECK(mav_tran,msg,nump_tran*isz) ) clean_up(0);
	}
	else
	{
		map_tran = (MA_set **)NULL;
		mav_tran = (int     *)NULL;
	}

	ns = 0;
	for( i = 0;  i < mac1->num_att;  i++ )
	{
		map = mac1->att_mas[i];
		if(    ((map->att_mas[0] != mac2) && (map->att_mas[1] != mac2))
		    || (map->num_in_set > ntol)    )
		{
			map_sav[ns] = map;
			mav_sav[ns] = mac1->attach_list[i];
			ns++;
		}
	}

	nc = nt = 0;
	for( i = 0;  i < mac2->num_att;  i++ )
	{
		map = mac2->att_mas[i];
		if(    ((map->att_mas[0] == mac1) || (map->att_mas[1] == mac1))
		    && (map->num_in_set <= ntol)    )
		{
			map_conn[nc] = map;	nc++;
		}
		else
		{
			map_tran[nt] = map;
			mav_tran[nt] = mac2->attach_list[i];
			nt++;
		}
	}
	if( (nump_conn != nc) || (nump_tran != nt) || (nump_sav != ns) )
	{
		FREE(map_conn,nump_conn*pMsz);

		if( nump_sav > 0  ) { FREE(map_sav ,nump_sav* pMsz);
				      FREE(mav_sav ,nump_sav*  isz); }
		if( nump_tran > 0 ) { FREE(map_tran,nump_tran*pMsz);
				      FREE(mav_tran,nump_tran* isz); }

		return 2;			/* logic error */
	}

		/* reduce mac1->att_mas and ->attach_list */

	FREE(mac1->att_mas    ,mac1->num_att*pMsz);
	FREE(mac1->attach_list,mac1->num_att*isz);

	mac1->num_att = nump_sav;
	mac1->att_mas = map_sav;
	mac1->attach_list = mav_sav;

		/* transfer map_tran to mac1->att_mas */
		/* transfer mav_tran to mac1->attach_list */

	new_num_att = mac1->num_att + nump_tran;

	if( new_num_att > 0 )
	{
	    if( nump_sav > 0 )
	    {
		mac1->att_mas = (MA_set **)REALLOC(mac1->att_mas,
							new_num_att*pMsz);
		msg = "mac1->att_mas realloc in merge_mac2_into_mac1()";
		if( MEMCHECK(mac1->att_mas,msg,nump_tran*pMsz) ) clean_up(0);

		mac1->attach_list = (int *)REALLOC(mac1->attach_list,
							new_num_att*isz);
		msg = "mac1->attach_list realloc in merge_mac2_into_mac1()";
		if( MEMCHECK(mac1->attach_list,msg,nump_tran*isz) ) clean_up(0);
	    }
	    else
	    {
		mac1->att_mas = (MA_set **)MALLOC(new_num_att*pMsz);
		msg = "mac1->att_mas realloc in merge_mac2_into_mac1()";
		if( MEMCHECK(mac1->att_mas,msg,new_num_att*pMsz) ) clean_up(0);

		mac1->attach_list = (int *)MALLOC(new_num_att*isz);
		msg = "mac1->attach_list realloc in merge_mac2_into_mac1()";
		if( MEMCHECK(mac1->attach_list,msg,new_num_att*isz) ) clean_up(0);
	    }
	}

	for( i = 0, j = mac1->num_att;  i < nump_tran;  i++, j++ )
	{
		mac1->attach_list[j] = mav_tran[i];

		map = map_tran[i];
		mac1->att_mas[j]     = map;
		if( map->att_mas[0] == mac2 ) map->att_mas[0] = mac1;
		if( map->att_mas[1] == mac2 ) map->att_mas[1] = mac1;
	}
	mac1->num_att = new_num_att;

		/* tranfer mac2 to mac1->loc */
		/* transfer map_conn to mac1->loc */

	new_num_in_set = mac1->num_in_set + mac2->num_in_set;
	for( i = 0;  i < nump_conn;  i++ )
	{
		map = map_conn[i];
		new_num_in_set += map->num_in_set - 2;
	}

	del_nis = new_num_in_set - mac1->num_in_set;

	mac1->loc = (int *)REALLOC(mac1->loc,new_num_in_set*isz);
	msg = "mac1->loc realloc in merge_mac2_into_mac1()";
	if( MEMCHECK(mac1->loc,msg,del_nis*isz) ) clean_up(0);

	mac1->bdat = (unsigned char *)REALLOC(mac1->bdat,new_num_in_set*usz);
	msg = "mac1->bdat realloc in merge_mac2_into_mac1()";
	if( MEMCHECK(mac1->bdat,msg,del_nis*usz) ) clean_up(0);

	
	for( j = 0, i = mac1->num_in_set;  j < mac2->num_in_set;  j++, i++ )
	{
		mac1->loc[i]  = mac2->loc[j];
		mac1->bdat[i] = mac2->bdat[j];
	}
	for( k = 0;  k < nump_conn;  k++ )
	{
		map = map_conn[k];
		for( j = 1;  j < map->num_in_set-1;  j++, i++ )
		{
			mac1->loc[i]  = map->loc[j];
			mac1->bdat[i] = map->bdat[j];
		}
	}
	if( i != new_num_in_set )
	{
		PRINTF("logic 2 error in merge_mac2_into_mac1\n");
		clean_up(0);
	}
	mac1->num_in_set = new_num_in_set;

		/* Unlink mac2 from cluster chain */
	if( mac2 == *mac_root )
	{
		 *mac_root = mac2->next;
		(*mac_root)->prev = (MA_set *)NULL;
	}
	else
	{
		mac2->prev->next = mac2->next;
		if( mac2->next != NULL ) mac2->next->prev = mac2->prev;
	}

		/* free mac2 */
	free_MA_set(mac2,isz,usz,Msz,pMsz);

		/* remove map_conn[] elements from path list */

	for( i = 0;  i < nump_conn;  i++ )
	{
		map = map_conn[i];
		if( map == *map_root )
		{
			 *map_root = map->next;
			(*map_root)->prev = (MA_set *)NULL;
		}
		else
		{
			map->prev->next = map->next;
			if( map->next ) map->next->prev = map->prev;
		}
	}

		/* free map_conn[] elements */

	for( i = 0;  i < nump_conn;  i++ )
		free_MA_set(map_conn[i],isz,usz,Msz,pMsz);

		/* free *_tran and *_conn */

	FREE(map_conn,nump_conn*pMsz);
	if( nump_tran > 0 ) { FREE(map_tran,nump_tran*pMsz);
			      FREE(mav_tran,nump_tran* isz); }

	if( dbg )
	{
		PRINTF("Cluster %d after merge\n",mac1);
		print_MAS("cluster","paths",mac1,1,nx,nxy);
	}

	return 0;
}
/* xxxxxxxyyyyyyyyyyxxxxxxxxxxyyyyyyyyyyxxxxxxxxxxyyyyyyyyyyxxxxxxxxxxyyyyyyy */


int	merge_map_into_mac1(MA_set **map_root, MA_set **mac_root, MA_set *mac1,
						MA_set *map2, int nx, int nxy, int dbg)
{
	MA_set	*map;
	MA_set	**map_sav, **map_conn, **map_tran;

	int	*mav_sav, *mav_tran;

	char	*msg;

	int	i, j, k;
	int	ns, nc, nt, is_ne = 0;
	int	new_num_att, new_num_in_set, del_nis;
	int	nump_sav, nump_conn, nump_tran;

	int      isz = sizeof(int);
	int      usz = sizeof(unsigned char);
	int      Msz = sizeof(MA_set);
	int     pMsz = sizeof(MA_set *);

	/* map2 will become part of mac1 */
	/* remaining paths on mac1 will stay in mac1->att_mas */

	nump_conn = 0;
	for( i = 0;  i < mac1->num_att;  i++ )
	{
		/* needle-eye x 2; bl x 1 */
		if( mac1->att_mas[i] == map2 ) nump_conn++;
	}
	if( nump_conn == 2 )
	{
		is_ne = 1;
		/***
		fprintf(stderr,"is needle-eye path\n");
		***/
	}

	if( dbg )
	{
		PRINTF("\n\nMerging cluster %d with path %d\n",
							mac1,map2);
		print_MAS("cluster","paths",mac1,1,nx,nxy);
	}

	if( nump_conn == 0 ) return 1;	/* clusters cannot be merged */
	/* nump_tran = mac2->num_att - nump_conn; */
	nump_tran = 0;
	nump_sav  = mac1->num_att - nump_conn;

	if( nump_sav > 0 )
	{
		map_sav = (MA_set **)MALLOC(nump_sav*pMsz);
		msg = "map_sav in merge_map_into_mac1()";
		if( MEMCHECK(map_sav,msg,nump_sav*pMsz) ) clean_up(0);

		mav_sav = (int *)MALLOC(nump_sav*isz);
		msg = "mav_sav in merge_map_into_mac1()";
		if( MEMCHECK(mav_sav,msg,nump_sav*isz) ) clean_up(0);
	}
	else
	{
		map_sav = (MA_set **)NULL;
		mav_sav = (int     *)NULL;
	}

	map_conn = (MA_set **)MALLOC(nump_conn*pMsz);
	msg = "map_conn in merge_map_into_mac1()";
	if( MEMCHECK(map_conn,msg,nump_conn*pMsz) ) clean_up(0);

	if( nump_tran > 0 )
	{
		map_tran = (MA_set **)MALLOC(nump_tran*pMsz);
		msg = "map_tran in merge_map_into_mac1()";
		if( MEMCHECK(map_tran,msg,nump_tran*pMsz) ) clean_up(0);

		mav_tran = (int *)MALLOC(nump_tran*isz);
		msg = "mav_tran in merge_map_into_mac1()";
		if( MEMCHECK(mav_tran,msg,nump_tran*isz) ) clean_up(0);
	}
	else
	{
		map_tran = (MA_set **)NULL;
		mav_tran = (int     *)NULL;
	}

	ns = 0;
	for( i = 0;  i < mac1->num_att;  i++ )
	{
		map = mac1->att_mas[i];
		if( map != map2 )
		{
			map_sav[ns] = map;
			mav_sav[ns] = mac1->attach_list[i];
			ns++;
		}
	}

	nc = nt = 0;
	/***
	for( i = 0;  i < mac2->num_att;  i++ )
	{
	***/
	for( i = 0;  i < mac1->num_att;  i++ )
	{
		map = mac1->att_mas[i];
		if( map == map2 )
		{
			map_conn[nc] = map;	nc++;
		}
		/***
		else
		{
			map_tran[nt] = map;
			mav_tran[nt] = mac2->attach_list[i];
			nt++;
		}
		***/
	}
	if( (nump_conn != nc) || (nump_tran != nt) || (nump_sav != ns) )
	{
		FREE(map_conn,nump_conn*pMsz);

		if( nump_sav > 0  ) { FREE(map_sav ,nump_sav* pMsz);
				      FREE(mav_sav ,nump_sav*  isz); }
		if( nump_tran > 0 ) { FREE(map_tran,nump_tran*pMsz);
				      FREE(mav_tran,nump_tran* isz); }

		return 2;			/* logic error */
	}

		/* reduce mac1->att_mas and ->attach_list */

	FREE(mac1->att_mas    ,mac1->num_att*pMsz);
	FREE(mac1->attach_list,mac1->num_att*isz);

	mac1->num_att = nump_sav;
	mac1->att_mas = map_sav;
	mac1->attach_list = mav_sav;

		/* transfer map_tran to mac1->att_mas */
		/* transfer mav_tran to mac1->attach_list */

	new_num_att = mac1->num_att + nump_tran;

	if( new_num_att > 0 )
	{
	    if( nump_sav > 0 )
	    {
		mac1->att_mas = (MA_set **)REALLOC(mac1->att_mas,
							new_num_att*pMsz);
		msg = "mac1->att_mas realloc in merge_map_into_mac1()";
		if( MEMCHECK(mac1->att_mas,msg,nump_tran*pMsz) ) clean_up(0);

		mac1->attach_list = (int *)REALLOC(mac1->attach_list,
							new_num_att*isz);
		msg = "mac1->attach_list realloc in merge_map_into_mac1()";
		if( MEMCHECK(mac1->attach_list,msg,nump_tran*isz) ) clean_up(0);
	    }
	    else
	    {
		mac1->att_mas = (MA_set **)MALLOC(new_num_att*pMsz);
		msg = "mac1->att_mas realloc in merge_map_into_mac1()";
		if( MEMCHECK(mac1->att_mas,msg,new_num_att*pMsz) ) clean_up(0);

		mac1->attach_list = (int *)MALLOC(new_num_att*isz);
		msg = "mac1->attach_list realloc in merge_map_into_mac1()";
		if( MEMCHECK(mac1->attach_list,msg,new_num_att*isz) ) clean_up(0);
	    }
	}

	/********
	for( i = 0, j = mac1->num_att;  i < nump_tran;  i++, j++ )
	{
		mac1->attach_list[j] = mav_tran[i];

		map = map_tran[i];
		mac1->att_mas[j]     = map;
		if( map->att_mas[0] == mac2 ) map->att_mas[0] = mac1;
		if( map->att_mas[1] == mac2 ) map->att_mas[1] = mac1;
	}
	********/
	mac1->num_att = new_num_att;

		/* tranfer map2 to mac1->loc */
		/* transfer map_conn to mac1->loc */

	new_num_in_set = mac1->num_in_set + map2->num_in_set - 1;
	/**********
	for( i = 0;  i < nump_conn;  i++ )
	{
		map = map_conn[i];
		new_num_in_set += map->num_in_set - 1;
		break;		* ne *
	}
	**********/

	del_nis = new_num_in_set - mac1->num_in_set;

	mac1->loc = (int *)REALLOC(mac1->loc,new_num_in_set*isz);
	msg = "mac1->loc realloc in merge_map_into_mac1()";
	if( MEMCHECK(mac1->loc,msg,del_nis*isz) ) clean_up(0);

	mac1->bdat = (unsigned char *)REALLOC(mac1->bdat,new_num_in_set*usz);
	msg = "mac1->bdat realloc in merge_map_into_mac1()";
	if( MEMCHECK(mac1->bdat,msg,del_nis*usz) ) clean_up(0);


	/***
	for( j = 0, i = mac1->num_in_set;  j < mac2->num_in_set;  j++, i++ )
	{
		mac1->loc[i]  = mac2->loc[j];
		mac1->bdat[i] = mac2->bdat[j];
	}
	***/
	i = mac1->num_in_set;
	for( k = 0;  k < nump_conn;  k++ )
	{
		map = map_conn[k];
		for( j = 1;  j < map->num_in_set-1;  j++, i++ )
		{
			mac1->loc[i]  = map->loc[j];
			mac1->bdat[i] = map->bdat[j];
		}
		if( map->att_mas[0] == mac1 )
		{
			mac1->loc[i]  = map->loc[map->num_in_set-1];
			mac1->bdat[i] = map->bdat[map->num_in_set-1];
			i++;
		}
		else
		{
			mac1->loc[i]  = map->loc[0];
			mac1->bdat[i] = map->bdat[0];
			i++;
		}
		break;		/* ne */
	}
	if( i != new_num_in_set )
	{
		fprintf(stderr,"logic 2 error in merge_map_into_mac1\n");
		PRINTF("logic 2 error in merge_map_into_mac1\n");
		clean_up(0);
	}
	mac1->num_in_set = new_num_in_set;

		/* Unlink mac2 from cluster chain */
	/*********
	if( mac2 == *mac_root )
	{
		 *mac_root = mac2->next;
		(*mac_root)->prev = (MA_set *)NULL;
	}
	else
	{
		mac2->prev->next = mac2->next;
		if( mac2->next != NULL ) mac2->next->prev = mac2->prev;
	}
	*********/

		/* free mac2 */
	/*********
	free_MA_set(mac2,isz,usz,Msz,pMsz);
	*********/

		/* remove map_conn[] elements from path list */

	for( i = 0;  i < nump_conn;  i++ )
	{
		map = map_conn[i];
		if( map == *map_root )
		{
			*map_root = map->next;
			if( *map_root != NULL )
				(*map_root)->prev = (MA_set *)NULL;
		}
		else
		{
			map->prev->next = map->next;
			if( map->next ) map->next->prev = map->prev;
		}
		break;		/* ne */
	}

		/* free map_conn[] elements */

	for( i = 0;  i < nump_conn;  i++ )
	{
		free_MA_set(map_conn[i],isz,usz,Msz,pMsz);
		break;
	}

		/* free *_tran and *_conn */

	FREE(map_conn,nump_conn*pMsz);
	if( nump_tran > 0 ) { FREE(map_tran,nump_tran*pMsz);
			      FREE(mav_tran,nump_tran* isz); }

	if( dbg )
	{
		PRINTF("Cluster %d after merge\n",mac1);
		print_MAS("cluster","paths",mac1,1,nx,nxy);
	}

	return 0;
}
