/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "maset.h"


int	check_one_obj(int *obj, int obj_size, int nx, int nxy,
				  int (*proc_func)(int, int, int, int, int, int))
{
	int	i, j, x1, x2, y1, y2, z1, z2;
	int	tmp_pt, search_stop_pt, done_stop_pt, pt1, pt2;
	int	is_connected;
	int	tot_moved, curr_moved;
	
	if( obj_size == 1 ) return 1;	/* one voxel in the object */
	
	tmp_pt = obj[0];
	obj[0] = obj[obj_size -1];
	obj[obj_size-1] = tmp_pt;
	search_stop_pt = obj_size - 2;
	done_stop_pt = obj_size -1;
	tot_moved = 1;

	for( i = obj_size -1;  i >= done_stop_pt;  i-- )
	{
		pt1 = obj[i];
		Ind_2_ijk(pt1,x1,y1,z1,nx,nxy);
		curr_moved = 0;

		for( j = 0;  j <= search_stop_pt;  j++ )
		{
			pt2 = obj[j];
			Ind_2_ijk(pt2,x2,y2,z2,nx,nxy);

			is_connected = (*proc_func)(x1,x2,y1,y2,z1,z2);
			if( is_connected )
			{
				obj[j] = obj[search_stop_pt];
				obj[search_stop_pt] = pt2;
				search_stop_pt--;
				done_stop_pt--;
				curr_moved++;
				if( search_stop_pt < 0 ) return 1;
				j--;		/* check moved point ! */
			}
		}
		tot_moved += curr_moved;
	}
	return 0;
}


int	is_6_connected(int x1, int x2, int y1, int y2, int z1, int z2)
{
	int	absx, absy, absz, abssum;
	int	ans;
  
	absx = abs(x1-x2);	absy = abs(y1-y2);	absz = abs(z1-z2);

	abssum = absx + absy + absz;

	ans = ( abssum == 1 ) ? 1 : 0;
	return ans;
}


int	is_26_connected(int x1, int x2, int y1, int y2, int z1, int z2)
{
	int	absx, absy, absz;
	int	ans;
	
	absx = abs(x1-x2);	absy = abs(y1-y2);	absz = abs(z1-z2);
	
	ans = ( (absx <= 1) && (absy <= 1) && (absz <= 1) ) ? 1 : 0;
	return ans;
}


void	dbg_print_check_one_object(int *check_obj, int obj_size)
{
	int	i;

	for( i = 0;  i < obj_size;  i++ )
	{
		fprintf(stderr,"%d) %d\n",i+1,check_obj[i]);
		fprintf(stdout,"%d) %d\n",i+1,check_obj[i]);
	}
}
