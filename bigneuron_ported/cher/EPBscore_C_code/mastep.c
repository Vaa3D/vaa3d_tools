/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "my_ma.h"
#include "burn_prototype.h"
#include "ma_prototype.h"


#define Zero_now_set(gopm)\
{\
	bfdat[gopm] = dir;\
	add_to_chain(bc,gopm,realloc_size);\
}


#define Pos_now_set(gopm)\
		set_90_ma(tmp,gopm,dir,b_step,now);\
	}\
	else if( now > 0 && now <= 43)\
	{\
		bfdat[gopm] = 43;\
		Save_ma_voxel(tmp,gopm) 

void	surf_ma_step(Bchains *bc, Fdat *tmp, int pos, char b_step, int nx,
					 int nxy, int realloc_size)
{
	unsigned char	*bfdat;
	unsigned char	now;
	unsigned char	dir;
	char	*cnt;
	
	int	go;

	cnt = bc->cnt;		bfdat = tmp->bfdat;

	if( cnt[2] )			/* go x */
	{
	    go = pos+1;
	    dir = 16;		now = bfdat[go];

	    if( now == 0 ) Zero_now_set(go)
	    else
	    {		/*   +,-y  &  +,-z &  -y+z &  -y-z & +y+z & +y-z */
			/* & x-y+z & x-y-z & x+y+z & x+y-z &    x	 */
		if(    now == 4  || now == 8  || now == 1  || now == 2 
		    || now == 5  || now == 6  || now == 9  || now == 10
		    || now == 21 || now == 22 || now == 25 || now == 26
		    || now == 16   )
		{
		    Pos_now_set(go);
		}
	    }
	    if( cnt[0] )			/* no +z --> +x+z */
	    {
		dir = 17;		now = bfdat[go+nxy];
		if( now == 0 ) Zero_now_set(go+nxy)
		else
		{		/*   +,-y &  -y+z &  +y+z & +x+y */
				/* & +x-y & x+y+z & x-y+z &  x+z */
		    if(    now == 4  || now == 8  || now == 5  || now == 9
			|| now == 20 || now == 24 || now == 21 || now == 25
			|| now == 17)
		    {
			Pos_now_set(go+nxy);
		    }
		}
	    }
	    if( cnt[1] )			/* no -z --> +x-z */
	    {
		dir = 18;		now = bfdat[go-nxy];
		if( now == 0 ) Zero_now_set(go-nxy)
		else
		{		/*   +,-y &  -y-z &  +y-z & +x+y */
				/* & +x-y & x+y-z & x-y-z &  x-z */
		    if(    now == 4  || now == 8  || now == 6  || now == 10
			|| now == 20 || now == 24 || now == 22 || now == 26
			|| now == 18)
		    {
			Pos_now_set(go-nxy);
		    }
		}
	    }
	    if( cnt[4] )			/* no +y --> +x+y */
	    {
		go = pos+nx+1;
		dir = 20;		now = bfdat[go];

		if( now == 0 ) Zero_now_set(go)
		else
		{		/*   +,-z &  +x+z &  +x-z & +y+z */
				/* & +y-z & x+y+z & x+y-z &  x+y */
		    if(    now == 1 || now == 2 || now == 17 || now == 18
			|| now == 5 || now == 6 || now == 21 || now == 22
			|| now == 20)
		    {
			Pos_now_set(go);
		    }
		}
		if( cnt[0] )			/* no +z --> +x+y+z */
		{
		    dir = 21;		now = bfdat[go+nxy];
		    if( now == 0 ) Zero_now_set(go+nxy)
		    else 
		    {		/* z & y & x & y+z & x+z & x+y & x+y+z */
			if(    now == 1  || now == 4  || now == 16 || now == 5
			    || now == 17 || now == 20 || now == 21  )
			{
			    Pos_now_set(go+nxy);
			}
		    }
		}
		if( cnt[1] )			/* no -z --> +x+y-z */
		{
		    dir = 22;		now = bfdat[go-nxy];
		    if( now == 0 ) Zero_now_set(go-nxy)
		    else
		    {		/* -z & y & x & y-z & x-z & x+y & x+y-z */
			if(    now == 2  || now == 4  || now == 16 || now == 6
			    || now == 18 || now == 20 || now == 22)
			{
			    Pos_now_set(go-nxy);
			}
		    }
		}
	    }
	    if( cnt[5] )			/* no -y --> +x-y */
	    {
		go = pos-nx+1;
		dir = 24;			now = bfdat[go];

		if( now == 0 ) Zero_now_set(go)
		else
		{		/*   +,-z &  +x+z &  +x-z & -y+z */
				/* & -y-z & x-y+z & x-y-z &  x-y */
		    if(    now == 1 || now == 2  || now == 17 || now == 18
			|| now == 9 || now == 10 || now == 25 || now == 26
			|| now == 24)
		    {
			Pos_now_set(go);
		    }
		}
		if( cnt[0] )			/* no +z --> +x-y+z */
		{
		    dir = 25;		now = bfdat[go+nxy];
		    if( now == 0 ) Zero_now_set(go+nxy)
		    else 
		    {		/* z & -y & x & -y+z & x+z & x-y & x-y+z */
			if(    now == 1  || now == 8  || now == 16 || now == 9
			    || now == 17 || now == 24 || now == 25    )
			{
			    Pos_now_set(go+nxy);
			}
		    }
		}
		if( cnt[1] )			/* no -z --> +x-y-z */
		{
		    dir = 26;		now = bfdat[go-nxy];
		    if( now == 0 ) Zero_now_set(go-nxy)
		    else
		    {		/* -z & -y & x & -y-z & x-z & x-y & x-y-z */
			if(    now == 2  || now == 8  || now == 16 || now == 10
			    || now == 18 || now == 24 || now == 26)
			{
			    Pos_now_set(go-nxy);
			}
		    }
		}
	    }
	}
	if( cnt[4] )				/* go +y */
	{
	    go = pos+nx;	
	    dir = 4;		now = bfdat[go];

	    if( now == 0 ) Zero_now_set(go)
	    else
	    {		/*    +,-z &  +,-x &  +x+z &  +x-z & -x+z & -x-z */
			/* & y+x+z & y+x-z & y-x+z & y-x-z &    y        */
		if(    now == 1  || now == 2  || now == 16 || now == 32
		    || now == 17 || now == 18 || now == 33 || now == 34
		    || now == 21 || now == 22 || now == 37 || now == 38
		    || now == 4)
		{
		    Pos_now_set(go);
		}
	    }

	    if( cnt[0] )			/* no +z --> +y+z */
	    {
		dir = 5;		now = bfdat[go+nxy];
		if( now == 0 ) Zero_now_set(go+nxy)
		else
		{		/*   +,-x &  +y+x &  +y-x & +z+x */
				/* & +z-x & z+x+y & z-x+y &  y+z */
		    if(    now == 16 || now == 32 || now == 20 || now == 36
			|| now == 17 || now == 33 || now == 21 || now == 37
			|| now == 5    )
		    {
			Pos_now_set(go+nxy);
		    }
		}
	    }
	    if( cnt[1] )			/* no -z --> +y-z */
	    {
		dir = 6;		now = bfdat[go-nxy];
		if( now == 0 ) Zero_now_set(go-nxy)
		else
		{		/*   +,-x &  +y+x &  +y-x & -z+x */
				/* & -z-x & y-z+x & y-z-x &  y-z */
		    if(    now == 16 || now == 32 || now == 20 || now == 36
			|| now == 18 || now == 34 || now == 22 || now == 38
			|| now == 6)
		    {
			Pos_now_set(go-nxy);
		    }
		}
	    }
	    if( cnt[3] )			/* no -x --> -x+y */
	    {
		go = pos-1+nx;
		dir = 36;		now = bfdat[go];

		if( now == 0 ) Zero_now_set(go)
		else
		{		/*   +,-z &   -x+z &   -x-z & +y+z */
				/* & +y-z & -x+y+z & -x+y-z & -x+y */
		    if(    now == 1  || now == 2 || now == 33 || now == 34
			|| now == 5  || now == 6 || now == 37 || now == 38
			|| now == 36    )
		    {
			Pos_now_set(go);
		    }
		}
		if( cnt[0] )			/* no +z --> -x+y+z */
		{
		 dir = 37;		now = bfdat[go+nxy];
		 if( now == 0 ) Zero_now_set(go+nxy)
		 else 
		 {	/* z & +y & -x & y+z & -x+z & -x+y & -x+y+z */
		    if(    now == 1  || now == 4  || now == 32 || now == 5
		        || now == 33 || now == 36 || now == 37)
		    {
			Pos_now_set(go+nxy);
		    }
		 }
		}
		if( cnt[1] )			/* no -z --> -x+y-z */
		{
		 dir = 38;		now = bfdat[go-nxy];
		 if( now == 0 ) Zero_now_set(go-nxy)
		 else
		 {	/* -z & +y & -x & y-z & -x-z & -x+y & -x+y-z */
		    if(    now == 2  || now == 4  || now == 32 || now == 6
			|| now == 34 || now == 36 || now == 38    )
		    {
			Pos_now_set(go-nxy);
		    }
		 }
		}
	    }
	}
	if( cnt[3] )				/* no -x */
	{
	    go = pos-1;
	    dir = 32;		now = bfdat[go];

	    if( now == 0 ) Zero_now_set(go)
	    else	
	    {		/*     +,-z &   +,-y &   -y+z &   -y-z & +y+z & +y-z */
			/* & -x+y+z & -x+y-z & -x-y+z & -x-y-z &   -x        */
		if(    now == 1  || now == 2  || now == 4  || now == 8
		  || now == 5  || now == 6  || now == 9  || now == 10
		  || now == 37 || now == 38 || now == 41 || now == 42
		  || now == 32    )
		{
		 Pos_now_set(go);
		}
	    }
	    if( cnt[0] )			/* no +z --> -x+z */
	    {
		dir = 33;		now = bfdat[go+nxy];
		if( now == 0 ) Zero_now_set(go+nxy)
		else
		{		/*   +,-y & -x+y   & -x-y   & +z+y */
				/* & +z-y & -x+y+z & -x-y+z & -x+z */
		 if(    now == 4  || now == 8  || now == 36 || now == 40
		     || now == 5  || now == 9  || now == 37 || now == 41
		     || now == 33    )
		 {
		    Pos_now_set(go+nxy);
		 }
		}
	    }
	    if( cnt[1] )			/* no -z --> -x-z */
	    {
		dir = 34;		now = bfdat[go-nxy];
		if( now == 0 ) Zero_now_set(go-nxy)
		else	
		{		/*   +,-y & -x+y   & -x-y   & -z+y */
				/* & -z-y & -x+y-z & -x-y-z & -x-z */
		    if(    now == 4  || now == 8  || now == 36 || now == 40
			|| now == 6  || now == 10 || now == 38 || now == 42
			|| now == 34    )
		    {
			Pos_now_set(go-nxy);
		    }
		}
	    }
	    if( cnt[5] )			/* no -y --> -x-y */
	    {
		go = pos-1-nx;
		dir = 40;		now = bfdat[go];

		if( now == 0 ) Zero_now_set(go)
		else
		{		/*   +,-z & -x+z   & -x-z   & -y+z */
				/* & -y-z & -x-y+z & -x-y-z & -x-y */
		    if(    now == 1  || now == 2  || now == 33 || now == 34
			|| now == 9  || now == 10 || now == 41 || now == 42
			|| now == 40    )
		    {
			Pos_now_set(go);
		    }
		}
		if( cnt[0] )			/* no +z --> -x-y+z */
		{
		    dir = 41;		now  = bfdat[go+nxy];
		    if( now == 0 ) Zero_now_set(go+nxy)
		    else 
		    {	/* z & -y & -x & -y+z & -x+z & -x-y & -x-y+z */
			if(    now == 1  || now == 8  || now == 32 || now == 9
			    || now == 33 || now == 40 || now == 41    )
			{
			    Pos_now_set(go+nxy);
			}
		    }
		}
		if( cnt[1] )			/* no -z --> -x-y-z */
		{
		    dir = 42;		now = bfdat[go-nxy];
		    if( now == 0 ) Zero_now_set(go-nxy)
		    else
		    {	/* -z & -y & -x & y-z & -x-z & -x-y & -x-y-z */
			if(    now == 2  || now == 8  || now == 32 || now == 10
			    || now == 34 || now == 40 || now == 42    )
			{
			    Pos_now_set(go-nxy);
			}
		    }
		}
	    }
	}
	if( cnt[5] )				/* no -y */
	{
	    go = pos-nx;	
	    dir = 8;		now = bfdat[go];

	    if( now == 0 ) Zero_now_set(go)
	    else
	    {		/*     +,-x &   +,-z &   +x+z &   +x-z & -x+z & -x-z */
			/* & -y+x+z & -y+x-z & -y-x+z & -y-x-z &   -y        */
		if(    now == 16 || now == 32 || now == 1  || now == 2
		    || now == 17 || now == 18 || now == 33 || now == 34
		    || now == 25 || now == 26 || now == 41 || now == 42
		    || now == 8    )
		{
		    Pos_now_set(go);
		}
	    }
	    if( cnt[0] )			/* no +z --> -y+z */
	    {
		dir = 9;		now = bfdat[go+nxy];
		if( now == 0 ) Zero_now_set(go+nxy)
		else	
		{		/*   +,-x & -y+x   & -y-x   & +z+x */
				/* & +z-x & -y+z+x & -y+z-x & -y+z */
		    if(    now == 16 || now == 32 || now == 24 || now == 40
			|| now == 17 || now == 33 || now == 25 || now == 41
			|| now == 9    )
		    {
			Pos_now_set(go+nxy);
		    }
		}
	    }
	    if( cnt[1] )			/* no -z --> -y-z */
	    {
		dir = 10;		now = bfdat[go-nxy];
		if( now == 0 ) Zero_now_set(go-nxy)
		else
		{		/*   +,-x & -y+x   & -y-x   & -z+x */
				/* & -z-x & -y-z+x & -y-z-x & -y-z */
		    if(    now == 16 || now == 32 || now == 24 || now == 40
			|| now == 18 || now == 34 || now == 26 || now == 42
			|| now == 10    )
		    {
			Pos_now_set(go-nxy);
		    }
		}
	    }
	}
	if( cnt[0] )				/* no +z */
	{
	    go = pos;
	    dir = 1;		now = bfdat[go+nxy];

	    if( now == 0 ) Zero_now_set(go+nxy)
	    else	
	    {	/*  +y &  -y &  +x &  -x &  +x+y &  +x-y &  -x+y &  -x-y 
		 & z+y & z-y & z+x & z-x & z+x+y & z+x-y & z-x+y & z-x-y & z */
		if(    now == 4  || now == 8  || now == 16 || now == 32
		    || now == 20 || now == 24 || now == 36 || now == 40
		    || now == 5  || now == 9  || now == 17 || now == 33
		    || now == 21 || now == 25 || now == 37 || now == 41
		    || now == 1    )
		{
		    Pos_now_set(go+nxy);
		}
	    }
	}
	if( cnt[1] )				/* no -z */
	{
	    go = pos;
	    dir = 2;		now = bfdat[go-nxy];

	    if( now == 0 ) Zero_now_set(go-nxy)
	        else		
	    {	/*  +y &  -y &  +x &  -x &  +x+y &  +x-y &  -x+y &  -x-y 
		 &-z+y &-z-y &-z+x &-z-x &-z+x+y &-z+x-y &-z-x+y &-z-x-y & -z */
		if(    now == 4  || now == 8  || now == 16 || now == 32
		    || now == 20 || now == 24 || now == 36 || now == 40
		    || now == 6  || now == 10 || now == 18 || now == 34
		    || now == 22 || now == 26 || now == 38 || now == 42
		    || now == 2    )
		{
		    Pos_now_set(go-nxy);
		}
	    }
	}
}
