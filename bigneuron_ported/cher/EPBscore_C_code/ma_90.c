/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"
#include <math.h>
#include "my_ma.h"
#include "burn_macro.h"
#include "ma_prototype.h"

/*
*	pos	: voxel index (position)
*	dir	: burn direction into pos
*	now	: previous saved direction of pos
*/

#define Set_bfdat_grain(now) \
	tmp->bfdat[pos] = now; \
	if( tmp->cube[pos] != GRAIN ) tmp->cube[pos] = 99


void	set_90_ma(Fdat *tmp, int pos, unsigned char dir,
				  unsigned char b_step, unsigned char now)
{
      	if (dir == 21)	/* went x+y+z */
	{
	    if(    (now ==  1)	/* now +z 	: x+y+z = 21 */
		|| (now ==  4)	/* now +y 	: x+y+z = 21 */
		|| (now == 16)	/* now +x 	: x+y+z = 21 */
		|| (now ==  5)	/* now +y+z 	: x+y+z = 21 */
		|| (now == 17)	/* now x+z 	: x+y+z = 21 */
		|| (now == 20)    )	/* now +x+y	: x+y+z = 21 */
	    {
		 Set_bfdat_grain(21);
	    }
	}
	else if (dir < 21)
	{
	    if (dir == 8)	/* went -y */
	    {
		if (now == 1)		/* now +z 	: -y+z = 9 */
		{
			Set_bfdat_grain(9);
		}
		else if (now == 2)	/* now -z 	: -y-z = 10 */
		{
			Set_bfdat_grain(10);
		}
		else if (now == 16)	/* now +x 	: x-y = 24 */
		{
			Set_bfdat_grain(24);
		}
		else if (now == 32)	/* now -x 	: -x-y = 40 */
		{
			Set_bfdat_grain(40);
		}
		else if (now == 17)	/* now +x+z 	: x-y+z = 25 */
		{
			Set_bfdat_grain(25);
		}
		else if (now == 18)	/* now +x-z 	: x-y-z = 26 */
		{
			Set_bfdat_grain(26);
		}
		else if (now == 25)	/* now -y+x+z 	: x-y+z = 25 */
		{
			Set_bfdat_grain(25);
		}
		else if (now == 26)	/* now -y+x-z 	: x-y-z = 26 */
		{
			Set_bfdat_grain(26);
		}
		else if (now == 33)	/* now -x+z 	: -x-y+z = 41 */
		{
			Set_bfdat_grain(41);
		}
		else if (now == 34)	/* now -x-z 	: -x-y-z = 42 */
		{
			Set_bfdat_grain(42);
		}
		else if (now == 41)	/* now -y-x+z 	: -x-y+z = 41 */
		{
			Set_bfdat_grain(41);
		}
		else if (now == 42)	/* now -y-x-z 	: -x-y-z = 42 */
		{
			Set_bfdat_grain(42);
		}
	    }
	    else if (dir < 8)
	    {
	   	if (dir == 4)	/* went +y */
	   	{
		    if (now == 1)		/* now +z 	: y+z = 5 */
		    {
			Set_bfdat_grain(5);
		    }
		    else if (now == 2)	/* now -z 	: y-z = 6 */
		    {
			Set_bfdat_grain(6);
		    }
		    else if (now == 16)	/* now +x 	: x+y = 20 */
		    {
			Set_bfdat_grain(20);
		    }
		    else if (now == 32)	/* now -x 	: -x+y = 36 */
		    {
			Set_bfdat_grain(36);
		    }
		    else if (now == 17)	/* now +x+z 	: x+y+z = 21 */
		    {
			Set_bfdat_grain(21);
		    }
		    else if (now == 18)	/* now +x-z 	: x+y-z = 22 */
		    {
			Set_bfdat_grain(22);
		    }
		    else if (now == 33)	/* now -x+z 	: -x+y+z = 37 */
		    {
			Set_bfdat_grain(37);
		    }
		    else if (now == 34)	/* now -x-z 	: -x+y-z = 38 */
		    {
			Set_bfdat_grain(38);
		    }
		    else if (now == 21)	/* now y+x+z 	: x+y+z = 21 */
		    {
			Set_bfdat_grain(21);
		    }
		    else if (now == 22)	/* now y+x-z 	: x+y-z = 22 */
		    {
			Set_bfdat_grain(22);
		    }
		    else if (now == 37)	/* now y-x+z 	: -x+y+z = 37 */
		    {
			Set_bfdat_grain(37);
		    }
		    else if (now == 38)	/* now y-x-z 	: -x+y-z = 38 */
		    {
			Set_bfdat_grain(38);
		    }
		}
		else if (dir < 4)
		{
		    if (dir == 1)	/* went +z */
		    {
			if (now == 4)	/* now +y 	: +y+z = 5 */
			{
				Set_bfdat_grain(5);
			}
			else if (now == 8)	/* now -y 	: -y+z = 9 */
			{
				Set_bfdat_grain(9);
			}
			else if (now == 16)	/* now +x 	: x+z = 17 */
			{
				Set_bfdat_grain(17);
			}
			else if (now == 32)	/* now -x 	: -x+z = 33 */
			{
				Set_bfdat_grain(33);
			}
			else if (now == 20)	/* now x+y 	: x+y+z = 21 */
			{
				Set_bfdat_grain(21);
			}
			else if (now == 24)	/* now +x-y 	: x-y+z = 25 */
			{
				Set_bfdat_grain(25);
			}
			else if (now == 36)	/* now -x+y 	: -x+y+z = 37 */
			{
				Set_bfdat_grain(37);
			}
			else if (now == 40)	/* now -x-y 	: -x-y+z = 41 */
			{
				Set_bfdat_grain(41);
			}
			else if (now == 5)	/* now +y+z 	: +y+z = 5 */
			{
				Set_bfdat_grain(5);
			}
			else if (now == 9)	/* now -y+z 	: -y+z = 9 */
			{
				Set_bfdat_grain(9);
			}
			else if (now == 17)	/* now +x+z 	: x+z = 17 */
			{
				Set_bfdat_grain(17);
			}
			else if (now == 33)	/* now -x+z 	: -x+z = 33 */
			{
				Set_bfdat_grain(33);
			}
			else if (now == 21)	/* now x+y+z 	: x+y+z = 21 */
			{
				Set_bfdat_grain(21);
			}
			else if (now == 25)	/* now x-y+z 	: x-y+z = 25 */
			{
				Set_bfdat_grain(25);
			}
			else if (now == 37)	/* now -x+y+z 	: -x+y+z = 37 */
			{
				Set_bfdat_grain(37);
			}
			else if (now == 41)	/* now -x-y+z 	: -x-y+z = 41 */
			{
				Set_bfdat_grain(41);
			}
		    }
		    else if (dir == 2)	/* went -z */
		    {
			if (now == 4)	/* now +y 	: y-z = 6 */
			{
				Set_bfdat_grain(6);
			}
			else if (now == 8)	/* now -y 	: -y-z = 10 */
			{
				Set_bfdat_grain(10);
			}
			else if (now == 16)	/* now +x 	: x-z = 18 */
			{
				Set_bfdat_grain(18);
			}
			else if (now == 32)	/* now -x 	: -x-z = 34 */
			{
				Set_bfdat_grain(34);
			}
			else if (now == 20)	/* now x+y 	: x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
			else if (now == 24)	/* now x-y 	: x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
			else if (now == 36)	/* now -x+y 	: -x+y-z = 38 */
			{
				Set_bfdat_grain(38);
			}
			else if (now == 40)	/* now -x-y 	: -x-y-z = 42 */
			{
				Set_bfdat_grain(42);
			}
			else if (now == 6)	/* now +y-z 	: y-z = 6 */
			{
				Set_bfdat_grain(6);
			}
			else if (now == 10)	/* now -y-z 	: -y-z = 10 */
			{
				Set_bfdat_grain(10);
			}
			else if (now == 18)	/* now x-z 	: x-z = 18 */
			{
				Set_bfdat_grain(18);
			}
			else if (now == 34)	/* now -x-z 	: -x-z = 34 */
			{
				Set_bfdat_grain(34);
			}
			else if (now == 22)	/* now x+y-z 	: x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
			else if (now == 26)	/* now +x-y-z 	: x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
			else if (now == 38)	/* now -x+y-z 	: -x+y-z = 38 */
			{
				Set_bfdat_grain(38);
			}
			else if (now == 42)	/* now -x-y-z 	: -x-y-z = 42 */
			{
				Set_bfdat_grain(42);
			}
		    }
		}
		else if (dir > 4)
		{
		    if (dir == 5)	/* went +z+y */
		    {
			   if(    (now == 16)	/* now +x 	: x+y+z = 21 */
			       || (now == 20)	/* now +y+x 	: x+y+z = 21 */
			       || (now == 17)	/* now +z+x 	: x+y+z = 21 */
			       || (now == 21) )	/* now y+z+x 	: x+y+z = 21 */
			   {
				Set_bfdat_grain(21);
			   }
			   else if(    (now == 32)   /* now -x	  : -x+y+z = 37 */
				    || (now == 36)   /* now +y-x  : -x+y+z = 37 */
				    || (now == 33)   /* now +z-x  : -x+y+z = 37 */
				    || (now == 37) ) /* now y+z-x : -x+y+z = 37 */
			   {
				Set_bfdat_grain(37);
			   }
		    }
		    else if (dir == 6)	/* went -z+y */
		    {
			   if(    (now == 16)	/* now +x 	: x+y-z = 22 */
			       || (now == 20)	/* now +y+x 	: x+y-z = 22 */
			       || (now == 18)	/* now -z+x 	: x+y-z = 22 */
			       || (now == 22) ) /* now y-z+x 	: x+y-z = 22 */
			   {
				Set_bfdat_grain(22);
			   }
			   else if(    (now == 32)   /* now -x 	  : -x+y-z = 38 */
				    || (now == 36)   /* now +y-x  : -x+y-z = 38 */
				    || (now == 34)   /* now -z-x  : -x+y-z = 38 */
				    || (now == 38) ) /* now y-z-x : -x+y-z = 38 */
			   {
				Set_bfdat_grain(38);
			   }
		    }
		}
	    }
	    else if (dir > 8)
	    {
		if (dir == 17)	/* went +x+z */
		{
		    if(    (now == 4)		/* now +y 	: x+y+z = 21 */
			|| (now == 5)		/* now +y+z 	: x+y+z = 21 */
			|| (now == 20)		/* now +x+y 	: x+y+z = 21 */
			|| (now == 21)   )	/* now +x+y+z 	: x+y+z = 21 */
		    {
			Set_bfdat_grain(21);
		    }
		    else if(    (now ==  8)	  /* now -y 	: x-y+z = 25 */
			     || (now ==  9)	  /* now -y+z 	: x-y+z = 25 */
			     || (now == 24)	  /* now +x-y 	: x-y+z = 25 */
			     || (now == 25)    )  /* now +x-y+z	: x-y+z = 25 */
		    {
				Set_bfdat_grain(25);
		    }
		}
		else if (dir < 17)
		{
		    if (dir == 10)	/* went -y-z */
		    {
			if(    (now == 16)	/* now +x 	: x-y-z = 26 */
			    || (now == 24)	/* now -y+x 	: x-y-z = 26 */
			    || (now == 18)	/* now -z+x 	: x-y-z = 26 */
			    || (now == 26) ) /* now -y-z+x 	: x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
			else if(    (now == 32)   /* now -x	  : -x-y-z = 42 */
				 || (now == 40)   /* now -y-x  : -x-y-z = 42 */
				 || (now == 34)   /* now -z-x  : -x-y-z = 42 */
				 || (now == 42) ) /* now -y-z-x: -x-y-z = 42 */
			{
				Set_bfdat_grain(42);
			}
		    }
		    else if (dir == 9)	/* went -y+z */
		    {
			if(    (now == 16)	/* now +x 	: x-y+z = 25 */
			    || (now == 24)	/* now -y+x 	: x-y+z = 25 */
			    || (now == 17)	/* now +z+x 	: x-y+z = 25 */
			    || (now == 25) ) /* now -y+z+x 	: x-y+z = 25 */
			{
				Set_bfdat_grain(25);
			}
			else if(    (now == 32)   /* now -x 	  : -x-y+z = 41 */
				 || (now == 40)   /* now -y-x  : -x-y+z = 41 */
				 || (now == 33)   /* now +z-x  : -x-y+z = 41 */
				 || (now == 41) ) /* now -y+z-x: -x-y+z = 41 */
			{
				Set_bfdat_grain(41);
			}
		    }
		    else if (dir == 16)	/* went +x */
		    {
			if (now == 4)	/* now +y 	: +x+y = 20 */
			{
				Set_bfdat_grain(20);
			}
			else if (now == 8)	/* now -y 	: +x-y = 24 */
			{
				Set_bfdat_grain(24);
			}
			else if (now == 1)	/* now +z	: +x+z = 17 */
			{
				Set_bfdat_grain(17);
			}
			else if (now == 2)	/* now -z 	: +x-z = 18 */
			{
				Set_bfdat_grain(18);
			}
			else if (now == 5)	/* now +y+z 	: +x+y+z = 21 */
			{
				Set_bfdat_grain(21);
			}
			else if (now == 6)	/* now +y-z 	: +x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
			else if (now == 9)	/* now -y+z 	: +x-y+z = 25 */
			{
				Set_bfdat_grain(25);
			}
			else if (now == 10)	/* now -y-z 	: +x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
			else if (now == 21)	/* now x+y+z 	: x+y+z = 21 */
			{
				Set_bfdat_grain(21);
			}
			else if (now == 22)	/* now x+y-z 	: x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
			else if (now == 25)	/* now x-y+z 	: x-y+z = 25 */
			{
				Set_bfdat_grain(25);
			}
			else if (now == 26)	/* now x-y-z 	: x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
		    }
		}
		else if (dir > 17)
		{
		    if (dir == 18)	/* went +x-z */
		    {
			if(    (now ==  4)	/* now +y 	: x+y-z = 22 */
			    || (now ==  6)	/* now +y-z 	: x+y-z = 22 */
			    || (now == 20)	/* now +x+y 	: x+y-z = 22 */
			    || (now == 22) ) /* now +x+y-z 	: x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
			else if(    (now ==  8)   /* now -y	   : x-y-z = 26 */
				 || (now == 10)   /* now -y-z   : x-y-z = 26 */
				 || (now == 24)   /* now +x-y   : x-y-z = 26 */
				 || (now == 26) ) /* now +x-y-z : x-y-z = 26 */
			{
				Set_bfdat_grain(26);
			}
		    }
		    else if (dir == 20)	/* went +x+y */
		    {
			if(    (now ==  1)	/* now +z 	: x+y+z = 21 */
			    || (now == 17)	/* now +x+z 	: x+y+z = 21 */
			    || (now ==  5)	/* now +y+z 	: x+y+z = 21 */
			    || (now == 21) ) /* now x+y+z 	: x+y+z = 21 */
			{
				Set_bfdat_grain(21);
			}
			else if(    (now ==  2)   /* now -z	  : x+y-z = 22 */
				 || (now == 18)   /* now +x-z  : x+y-z = 22 */
				 || (now == 6)    /* now +y-z  : x+y-z = 22 */
				 || (now == 22) ) /* now x+y-z : x+y-z = 22 */
			{
				Set_bfdat_grain(22);
			}
		    }
		}
	    }
	}
	else if (dir > 21)
	{
	    if (dir == 34)	/* went -x-z */
	    {
		if(    (now ==  4)	/* now +y 	: -x+y-z = 38 */
		    || (now == 36)	/* now -x+y 	: -x+y-z = 38 */
		    || (now ==  6)	/* now -z+y 	: -x+y-z = 38 */
		    || (now == 38)   )  /* now -x-z+y 	: -x+y-z = 38 */
		{
			Set_bfdat_grain(38);
		}
		else if(    (now ==  8)		/* now -y 	: -x-y-z = 42 */
			 || (now == 40)		/* now -x-y 	: -x-y-z = 42 */
			 || (now == 10)		/* now -z-y 	: -x-y-z = 42 */
			 || (now == 42)   )	/* now -x-z-y 	: -x-y-z = 42 */
		{
			Set_bfdat_grain(42);
		}
	    }
	    else if (dir < 34)
	    {
		if (dir == 25)	/* went +x-y+z */
		{
		    if(    (now ==  1)	/* now +z	: +x-y+z = 25 */
			|| (now ==  8)	/* now -y	: +x-y+z = 25 */
			|| (now == 16)	/* now +x	: +x-y+z = 25 */
			|| (now ==  9)	/* now -y+z	: +x-y+z = 25 */
			|| (now == 17)	/* now +x+z	: +x-y+z = 25 */
			|| (now == 24)   )	/* now +x-y+z	: +x-y+z = 25 */
		    {
				Set_bfdat_grain(25);
		    }
		}
		else if (dir < 25)
		{
		    if (dir == 22)	/* went +x+y-z */
		    {
			 if(    (now == 2)	/* now +z 	: x+y-z = 22 */
			     || (now ==  4)	/* now +y 	: x+y-z = 22 */
			     || (now == 16)	/* now +x 	: x+y-z = 22 */
			     || (now ==  6)	/* now +y-z 	: x+y-z = 22 */
			     || (now == 18)	/* now +x-z 	: x+y-z = 22 */
			     || (now == 20)   ) /* now +x+y 	: x+y-z = 22 */
			 {
				Set_bfdat_grain(22);
			 }
		    }
		    else if (dir == 24)	/* went x-y */
		    {
			 if(    (now ==  1)	/* now +z 	: x-y+z = 25 */
			     || (now == 17)	/* now +x+z 	: x-y+z = 25 */
			     || (now ==  9)	/* now -y+z 	: x-y+z = 25 */
			     || (now == 25)   ) /* now x-y+z 	: x-y+z = 25 */
			 {
				Set_bfdat_grain(25);
			 }
			 else if(    (now ==  2)   /* now -z 	: x-y-z = 26 */
				  || (now == 18)   /* now +x-z	: x-y-z = 26 */
				  || (now == 10)   /* now -y-z	: x-y-z = 26 */
				  || (now == 26) ) /* now x-y-z : x-y-z = 26 */
			 {
				Set_bfdat_grain(26);
			 }
		    }
		}
		else if (dir > 25)
		{
		    if (dir == 26)	/* went x-y-z */
		    {
			 if(    (now == 2)	/* now -z 	: x-y-z = 26 */
			     || (now == 8)	/* now -y 	: x-y-z = 26 */
			     || (now == 16)	/* now x 	: x-y-z = 26 */
			     || (now == 10)	/* now -y-z 	: x-y-z = 26 */
			     || (now == 18)	/* now +x-z 	: x-y-z = 26 */
			     || (now == 24)   ) /* now +x-y 	: x-y-z = 26 */
			 {
				Set_bfdat_grain(26);
			 }
		    }
		    else if (dir == 32)	/* went -x */
		    {
			 if (now == 1)		/* now +z 	: -x+z = 33 */
			 {
				Set_bfdat_grain(33);
			 }
			 else if (now == 2)	/* now -z 	: -x-z = 34 */
			 {
				Set_bfdat_grain(34);
			 }
			 else if (now == 4)	/* now +y 	: -x+y = 36 */
			 {
				Set_bfdat_grain(36);
			 }
			 else if (now == 8)	/* now -y 	: -x-y = 40 */
			 {
				Set_bfdat_grain(40);
			 }
			 else if (now == 5)	/* now +y+z 	: -x+y+z = 37 */
			 {
				Set_bfdat_grain(37);
			 }
			 else if (now == 6)	/* now +y-z 	: -x+y-z = 38 */
			 {
				Set_bfdat_grain(38);
			 }
			 else if (now == 9)	/* now -y+z 	: -x-y+z = 41 */
			 {
				Set_bfdat_grain(41);
			 }
			 else if (now == 10)	/* now -y-z 	: -x-y-z = 42 */
			 {
				Set_bfdat_grain(42);
			 }
			 else if (now == 37)	/* now -x+y+z 	: -x+y+z = 37 */
			 {
				Set_bfdat_grain(37);
			 }
			 else if (now == 38)	/* now -x+y-z 	: -x+y-z = 38 */
			 {
				Set_bfdat_grain(38);
			 }
			 else if (now == 41)	/* now -x-y+z 	: -x-y+z = 41 */
			 {
				Set_bfdat_grain(41);
			 }
			 else if (now == 42)	/* now -x-y-z 	: -x-y-z = 42 */
			 {
				Set_bfdat_grain(42);
			 }
		    }
		    else if (dir == 33)	/* went -x+z */
		    {
			 if(    (now ==  4)	/* now +y 	: -x+y+z = 37 */
			     || (now == 36)	/* now -x+y 	: -x+y+z = 37 */
			     || (now ==  5)	/* now +z+y 	: -x+y+z = 37 */
			     || (now == 37)   ) /* now -x+z+y 	: -x+y+z = 37 */
			 {
				Set_bfdat_grain(37);
			 }
			 else if(    (now ==  8)   /* now -y 	 : -x-y+z = 41 */
				  || (now == 40)   /* now -x-y 	 : -x-y+z = 41 */
				  || (now ==  9)   /* now +z-y 	 : -x-y+z = 41 */
				  || (now == 41) ) /* now -x+z-y : -x-y+z = 41 */
			 {
				Set_bfdat_grain(41);
			 }
		    }
		}
	    }
	    else if (dir > 34)
	    {
		if (dir == 38)	/* went -x+y-z */
		{
		    if(    (now ==  2)	/* now -z 	: -x+y-z = 38 */
			|| (now ==  4)	/* now +y 	: -x+y-z = 38 */
			|| (now == 32)	/* now -x 	: -x+y-z = 38 */
			|| (now ==  6)	/* now +y-z 	: -x+y-z = 38 */
			|| (now == 34)	/* now -x-z 	: -x+y-z = 38 */
			|| (now == 36)   )	/* now -x+y 	: -x+y-z = 38 */
		    {
				Set_bfdat_grain(38);
		    }
		}
		else if (dir < 38)
		{
		    if (dir == 36)	/* went -x+y */
		    {
			   if(    (now ==  1)	/* now +z 	: -x+y+z = 37 */
			       || (now == 33)	/* now -x+z 	: -x+y+z = 37 */
			       || (now ==  5)	/* now +y+z 	: -x+y+z = 37 */
			       || (now == 37) ) /* now -x+y+z 	: -x+y+z = 37 */
			   {
				Set_bfdat_grain(37);
			   }
			   else if(    (now ==  2)   /* now -z 	  : -x+y-z = 38 */
				    || (now == 34)   /* now -x-z  : -x+y-z = 38 */
				    || (now ==  6)   /* now +y-z  : -x+y-z = 38 */
				    || (now == 38) ) /* now -x+y-z: -x+y-z = 38 */
			   {
				Set_bfdat_grain(38);
			   }
		    }
		    else if (dir == 37)	/* went -x+y+z */
		    {
			   if(    (now ==  1)	/* now +z 	: -x+y+z = 37 */
			       || (now ==  4)	/* now +y 	: -x+y+z = 37 */
			       || (now == 32)	/* now -x 	: -x+y+z = 37 */
			       || (now ==  5)	/* now +y+z 	: -x+y+z = 37 */
			       || (now == 33)	/* now -x+z 	: -x+y+z = 37 */
			       || (now == 36) ) /* now -x+y 	: -x+y+z = 37 */
			   {
				Set_bfdat_grain(37);
			   }
		    }
		}
		else if (dir > 38)
		{
		    if (dir == 40)	/* went -x-y */
		    {
			   if(    (now == 1)	/* now +z 	: -x-y+z = 41 */
			       || (now == 33)	/* now -x+z 	: -x-y+z = 41 */
			       || (now == 9)	/* now -y+z 	: -x-y+z = 41 */
			       || (now == 41) ) /* now -x-y+z 	: -x-y+z = 41 */
			   {
				Set_bfdat_grain(41);
			   }
			   else if(    (now ==  2)   /* now -z    : -x-y-z = 42 */
				    || (now == 34)   /* now -x-z  : -x-y-z = 42 */
				    || (now == 10)   /* now -y-z  : -x-y-z = 42 */
				    || (now == 42) ) /* now -x-y-z: -x-y-z = 42 */
			   {
				Set_bfdat_grain(42);
			   }
		    }
		    else if (dir == 41)	/* went -x-y+z */
		    {
			   if(    (now ==  1)	/* now +z 	: -x-y+z = 41 */
			       || (now ==  8)	/* now -y 	: -x-y+z = 41 */
			       || (now == 32)	/* now -x 	: -x-y+z = 41 */
			       || (now ==  9)	/* now -y+z 	: -x-y+z = 41 */
			       || (now == 33)	/* now -x+z 	: -x-y+z = 41 */
			       || (now == 40) ) /* now -x-y 	: -x-y+z = 41 */
			   {
				Set_bfdat_grain(41);
			   }
		    }
		    else if (dir == 42)	/* went -x-y-z */
		    {
			   if(    (now ==  2)	/* now -z 	: -x-y-z = 42 */
			       || (now ==  8)	/* now -y 	: -x-y-z = 42 */
			       || (now == 32)	/* now -x 	: -x-y-z = 42 */
			       || (now == 10)	/* now -y-z 	: -x-y-z = 42 */
			       || (now == 34)	/* now -x-z 	: -x-y-z = 42 */
			       || (now == 40) ) /* now -x-y 	: -x-y-z = 42 */
			   {
				Set_bfdat_grain(42);
			   }
		    }
		}
	    }
	}
}


void	check_90(Fdat *tmp, int nx, int ny, int nz, int use_bdry)
{
	int	i, j, k, pos;
	int	nxy;
	
	nxy = nx*ny;
	pos = 0;
	for( k = 0;  k < nz;  k++ )
	{
		for( j = 0;  j < ny;  j++ )
		{
		    for( i = 0;  i < nx;  i++, pos++ )
		    {
		    	if( tmp->cube[pos] != 99 ) continue;
				check_90_ma(tmp,i,j,k,pos,use_bdry,nx,ny,nz,nxy);
		    }
		}
	}
}


void	check_90_ma(Fdat *tmp, int i, int j, int k, int pos, int use_bdry,
					int nx, int ny, int nz, int nxy)
{
	unsigned char	dir;
	int	pos1, pos2;

	dir = tmp->bfdat[pos];

	if (dir == 21)				/* went x+y+z */
	{
	    pos1 = pos+1-nx;	pos2 = pos-1+nx;
	    check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
	}
	else if (dir < 21)
	{
	    if (dir < 8)
	    {
		if (dir > 4)
		{
		    if (dir == 5)		/* went +z+y */
		    {
			if ( k == nz-1)	goto done;
			pos1 = pos+nx-nxy;	pos2 = pos-nx+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 6)	/* went -z+y */
		    {
			if ( k == 0 )		goto done;
			pos1 = pos-nx-nxy;	pos2 = pos+nx+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
	        }
	    }
	    else if (dir > 8)
	    {
		if (dir == 17)		/* went +x+z */
		{
		    if ( k == nz-1)	goto done;
		    pos1 = pos+1-nxy;	pos2 = pos-1+nxy;
		    check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		}
		else if (dir < 17)
		{
		    if (dir == 9)		/* went -y+z */
		    {
			if ( k == nz-1)	goto done;
			pos1 = pos-nx-nxy;	pos2 = pos+nx+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 10)	/* went -y-z */
		    {
			if ( k == 0 )		goto done;
			pos1 = pos+nx-nxy;	pos2 = pos-nx+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
		else if (dir > 17)
		{
		    if (dir == 18)		/* went +x-z */
		    {
			if ( k == 0 )		goto done;
			pos1 = pos-1-nxy;	pos2 = pos+1+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 20)	/* went +x+y */
		    {
			pos1 = pos+1-nx;	pos2 = pos-1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
	    }
	}
	else if (dir > 21)
	{
	    if (dir == 34)			/* went -x-z */
	    {
		if ( k == 0 )	goto done;
		pos1 = pos+1-nxy;	pos2 = pos-1+nxy;
		check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
	    }
	    else if (dir < 34)
	    {
		if (dir == 25)		/* went +x-y+z */
		{
		    pos1 = pos-1-nx;	pos2 = pos+1+nx;
		    check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		}
		else if (dir < 25)
		{
		    if (dir == 22)		/* went +x+y-z */
		    {
			pos1 = pos+1-nx;	pos2 = pos-1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 24)	/* went x-y */
		    {
			pos1 = pos-1-nx;	pos2 = pos+1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
		else if (dir > 25)
		{
		    if (dir == 26)		/* went x-y-z */
		    {
			pos1 = pos-1-nx;	pos2 = pos+1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 33)	/* went -x+z */
		    {
			if ( k == nz-1)	goto done;
			pos1 = pos-1-nxy;	pos2 = pos+1+nxy;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
	    }
	    else if (dir > 34)
	    {
		if (dir == 38)		/* went -x+y-z */
		{
		    pos1 = pos-1-nx;	pos2 = pos+1+nx;
		    check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		}
		else if (dir < 38)
		{
		    if (dir == 36)		/* went -x+y */
		    {
			pos1 = pos-1-nx;	pos2 = pos+1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 37)	/* went -x+y+z */
		    {
			pos1 = pos-1-nx;	pos2 = pos+1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
		else if (dir > 38)
		{
		    if (dir == 40)		/* went -x-y */
		    {
			pos1 = pos+1-nx;	pos2 = pos-1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 41)	/* went -x-y+z */
		    {
			pos1 = pos+1-nx;	pos2 = pos-1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		    else if (dir == 42)	/* went -x-y-z */
		    {
			pos1 = pos+1-nx;	pos2 = pos-1+nx;
			check_burn_num(tmp,i,j,k,pos,pos1,pos2,use_bdry,nx,ny,nz);
		    }
		}
	    }
	}
	return;

    done:
	tmp->cube[pos] = 0;
}


void	check_burn_num(Fdat *tmp, int i, int j, int k, int pos, int pos1,
					   int pos2, int use_bdry, int nx, int ny, int nz)
{
	unsigned char	v1, v2, v3;
	unsigned char	bd;

	set_6nbr_bd(i,j,k,&bd,nx,ny,nz);

	if( (bd & 8) || (bd & 4) || (bd & 2) || (bd & 1) ) tmp->cube[pos] = 0;
	else
	{
		v1 = tmp->fdat[pos1];
		v2 = tmp->fdat[pos ];
		v3 = tmp->fdat[pos2];

		if( v1 < 254 && v3 < 254 && v1 < v2 && v2 > v3 )
			Save_ma_voxel(tmp,pos);
		else if( use_bdry )
		{
		    if( v1 == 254 )
		    {
			if( v3 == 254 )
			{   
			    if( v1 > v2 && v3 > v2 ) Save_ma_voxel(tmp,pos);
			}
			else if( v3 < 254 )
			{
			    if( v1 > v2 && v3 < v2 ) Save_ma_voxel(tmp,pos);
			}
		    }
		    else if( v1 < 254 )
		    {
		         if( v3 == 254 ) Save_ma_voxel(tmp,pos);
		    }
		}
		else	tmp->cube[pos] = 0;
	}
}
