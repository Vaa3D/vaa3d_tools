/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include "my_ma.h"
#include "ma_prototype.h"

#define Set_burn(dir,gopm) \
	now = bfdat[gopm]; \
	if( now == 0 ) \
	{ \
		bfdat[gopm] = dir; \
		add_to_chain(bc,gopm,realloc_size); \
	}
	

void	surf_burn_step(Bchains *bc, unsigned char *bfdat, int pos, int nx,
					   int nxy, int realloc_size)
{
	unsigned char	now;
	char	*cnt;
	int	go;

	cnt = bc->cnt;
	if( cnt[2] )				 /* go x */
	{
	    go = pos+1;		   Set_burn(16,go)

	    if( cnt[0] )	 { Set_burn(17,go+nxy) } /* no +z --> +x+z */
	    if( cnt[1] )	 { Set_burn(18,go-nxy) } /* no -z --> +x-z */
	    if( cnt[4] )				 /* no +y --> +x+y */
	    {
		go = pos+nx+1;	   Set_burn(20,go)

		if( cnt[0] ) { Set_burn(21,go+nxy) } /* no +z --> +x+y+z */
		if( cnt[1] ) { Set_burn(22,go-nxy) } /* no -z --> +x+y-z */
	    }
	    if( cnt[5] )				 /* no -y --> +x-y */
	    {
		go = pos-nx+1;	   Set_burn(24,go)

		if( cnt[0] ) { Set_burn(25,go+nxy) } /* no +z --> +x-y+z */
		if( cnt[1] ) { Set_burn(26,go-nxy) } /* no -z --> +x-y-z */
	    }
	}

	if( cnt[4] )				 /* go +y */
	{
	    go = pos+nx;	   Set_burn(4,go)

	    if( cnt[0] )	 { Set_burn(5,go+nxy) }	 /* no +z --> +y+z */
	    if( cnt[1] )	 { Set_burn(6,go-nxy) }	 /* no -z --> +y-z */
	    if( cnt[3] )				 /* no -x --> -x+y */
	    {
		go = pos-1+nx;	   Set_burn(36,go)

		if( cnt[0] ) { Set_burn(37,go+nxy) } /* no +z --> -x+y+z */
		if( cnt[1] ) { Set_burn(38,go-nxy) } /* no -z --> -x+y-z */
	    }
	}

	if( cnt[3] )				 /* go -x */
	{
	    go = pos-1;		   Set_burn(32,go)

	    if( cnt[0] )	 { Set_burn(33,go+nxy) } /* no +z --> -x+z */
	    if( cnt[1] )	 { Set_burn(34,go-nxy) } /* no -z --> -x-z */
	    if( cnt[5] )				 /* no -y --> -x-y */
	    {
		go = pos-1-nx;	   Set_burn(40,go)

		if( cnt[0] ) { Set_burn(41,go+nxy) } /* no +z --> -x-y+z */
		if( cnt[1] ) { Set_burn(42,go-nxy) } /* no -z --> -x-y-z */
	    }
	}

	if( cnt[5] )				 /* go -y */
	{
	    go = pos-nx;	   Set_burn(8,go)

	    if( cnt[0] )	 { Set_burn( 9,go+nxy) } /* no +z --> -y+z */
	    if( cnt[1] )	 { Set_burn(10,go-nxy) } /* no -z --> -y-z */
	}

	if( cnt[0] )				 /* no +z */
	{
	    go = pos;		   Set_burn(1,go+nxy)
	}

	if( cnt[1] )				 /* no -z */
	{
	    go = pos;		   Set_burn(2,go-nxy)
	}
}
