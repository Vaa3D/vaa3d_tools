/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */
#include "stdafx.h"
#include <math.h>
#include "burn_macro.h"
#include "burn_prototype.h"
#include "ma_prototype.h"


void	check_opposite(Bchains *bc, Fdat *tmp, int nx, int ny, int nz)
{
	int	i;
	int	ind, ix, iy, iz;
	int	go, pos;
	int	gony, gopy, gonx, gopx;
	int	nxy;

	nxy = nx*ny;

	for( i = 0;  i < bc->n_num;  i++ )
	{
		pos = bc->n_chain[i];

		iz = pos/nxy;	ind = pos%nxy;
		iy = ind/nx;	ix = ind%nx;

		if(    ix == 0 || ix == nx-1
		    || iy == 0 || iy == ny-1 )	go = -1;
		else				go = tmp->bfdat[pos];

		gony = tmp->bfdat[pos-nx];	gonx = tmp->bfdat[pos-1];
		gopy = tmp->bfdat[pos+nx];	gopx = tmp->bfdat[pos+1];

		if(    go   <= 0       || gony == EXT_VAL || gopy == EXT_VAL
		    || gonx == EXT_VAL || gopx == EXT_VAL   ) continue;

		find_op(tmp,iz,pos,go,nx,nxy,nz);
	}
}



/*	TODO:
*	Check the logic in this routine to ensure its correctness. Simplify
*	further if possible.
*
*	To clarify the logic, recall (ma.c) directions are stored as the
*	lower six bits of 8 bit chars in the order (-x,+x,-y,+y,-z,+z).
*	Expressed as decimal numbers, the directions in the 3X3X3 cube are
*
*	    -z                               +z             -y
*	42  10  26	40   8  24	41   9  25           |
*	34   2  18	32   0  16	33   1  17	-x --0-- +x
*	38   6  22	36   4  20	37   5  21           |
*							    +y
*
*	The logic is currently organized in a balanced 3-ary (<,=,>) tree
*	search to minimize, to 4, the number of steps needed to find the
*	correct action based upon the value of `go'.
*
*
*		       __________________*__________________
*		      /                  |                  \
*		    _*_                  |                  _*_
*		___/ | \______           |         ________/ | \_____
*	       /     |        \          |        /          |       \
*	    __*      |     ____*___      |     __*____       |      __*____
*	   /  |\     |    /    |   \     |    /  |    \      |     /  |    \
*	  *   | *    |   *_    |    *    |   *   |    _*__   |    *   |     *_  
*        /|\  | |\   |  /| \   |   / \   |  / \  |   / |  \  |   / \  |    /| \
*	0 1 2 4 5 6  8 9 10 16 17 18 20 21 22 24 25 26 32 33 34 36 37 38 40 41 42
*
*
*	Question: Could this be organized better in a search on directional
*	components? That is to say:
*
*				go & bits 1/2
*		10		     00			01
*	   go & bits 3/4	 go & bits 3/4	    go & bits 3/4
*	10      00    01      10     00     01    10    00     01
*    g&5/6    g&5/6  g&5/6  g&5/6   g&5/6  g&5/6 g&5/6 g&5/6  g&5/6
*    2 0 1    2 0 1  2 0 1  2 0 1   2 0 1  2 0 1 2 0 1 2 0 1  2 0 1
*/

/*	MACROS for find_op */

#define	Test_opp(oppos,d1,d2,d3,d4,d5,d6,d7,d8,d9,dv)			\
	pos_v = tmp->bfdat[oppos];					\
	if(    (pos_v == d1) || (pos_v == d2) || (pos_v == d3)		\
	    || (pos_v == d4) || (pos_v == d5) || (pos_v == d6)		\
	    || (pos_v == d7) || (pos_v == d8) || (pos_v == d9) )	\
	{								\
		ans_v = ans_v | dv;					\
		Save_ma_voxel(tmp,oppos);				\
	}

#define	Test_px_with_mx							\
	oppos = pos + 1;				/* go +x */	\
	Test_opp(oppos,42,34,38,40,32,36,41,33,37,1)	/* test -x */

#define	Test_mx_with_px							\
	oppos = pos - 1;				/* go -x */	\
	Test_opp(oppos,26,18,22,24,16,20,25,17,21,1)	/* test +x */

#define	Test_py_with_my							\
	oppos = pos + nx;				/* go +y */	\
	Test_opp(oppos,42,10,26,40, 8,24,41, 9,25,3)	/* test -y */

#define	Test_my_with_py							\
	oppos = pos - nx;				/* go -y */	\
	Test_opp(oppos,38, 6,22,36, 4,20,37, 5,21,3)	/* test +y */

#define	Test_pz_with_mz							\
	oppos = pos + nxy;				/* go +z*/	\
	Test_opp(oppos,42,10,26,34, 2,18,38, 6,22,5)	/* test -z */

#define	Test_mz_with_pz							\
	oppos = pos - nxy;				/* go -z */	\
	Test_opp(oppos,41, 9,25,33, 1,17,37, 5,21,5)	/* test +z */


void	find_op(Fdat *tmp, int k, int pos, int go, int nx, int nxy, int nz)
{
	int	oppos;
	unsigned char	pos_v;
	char	ans_v;

	ans_v = 0;
	if (go == 21)					/* went +x+y+z */
	{
		Test_px_with_mx
		Test_py_with_my
		if (k != nz-1) { Test_pz_with_mz }
		if (ans_v) Save_ma_voxel(tmp,pos);	/* went +x-y+z */
	}
	else if (go < 21)
	{
	    if (go == 8)				/* went -y */
	    {
		Test_my_with_py
		if (ans_v) Save_ma_voxel(tmp,pos);
	    }
	    else if (go < 8)
	    {
		if (go == 4)				/* went +y */
		{
		    Test_py_with_my
		    if (ans_v) Save_ma_voxel(tmp,pos);
		}
		else if (go < 4)
		{
		    if (go == 1 && k != nz -1)		/* went +z */
		    {
			Test_pz_with_mz
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 2 && k != 0)		/* went -z */
		    {
			Test_mz_with_pz
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		}
		else if (go > 4)
		{
		    if (go == 5)			/* went +y+z */
		    {
			Test_py_with_my
			if ( k != nz-1) { Test_pz_with_mz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went -y+z */
		    }
		    else if (go == 6) 				/* went y-z */
		    {
			Test_py_with_my
			if ( k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went -y-z */
		    }
		}
	    }
	    else if (go > 8)
	    {
		if (go == 17)					/* went +x+z */
		{
		    Test_px_with_mx
		    if ( k != nz-1) { Test_pz_with_mz }
		    if (ans_v) Save_ma_voxel(tmp,pos);		/* went +x+z */
		}
		else if (go < 17)
		{
		    if (go == 10)				/* went -y-z */
		    {
			Test_my_with_py
			if (k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went +y-z */
		    }
		    else if (go == 9)				/* went -y+z */
		    {
			Test_my_with_py
			if ( k != nz-1) { Test_pz_with_mz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went +y+z */
		    }
		    else if (go == 16)				/* went +x */
		    {
			Test_px_with_mx
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		}
		else if (go > 17)
		{
		    if (go == 18)				/* went +x-z */
		    {
			Test_px_with_mx
			if (k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went +x-z */
		    }
		    else if (go == 20)				/* went +x+y */
		    {
			Test_px_with_mx
			Test_py_with_my
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		}
	    }
	}
	else if (go > 21)
	{
	    if (go == 34)					/* went -x-z */
	    {
		Test_mx_with_px
		if (k != 0) { Test_mz_with_pz }
		if (ans_v) Save_ma_voxel(tmp,pos);		/* went -x-z */
	    }
	    else if (go < 34)
	    {
		if (go == 25)					/* went +x-y+z */
		{ 
		    Test_px_with_mx
		    Test_my_with_py
		    if (k != nz-1) { Test_pz_with_mz }
		    if (ans_v) Save_ma_voxel(tmp,pos);		/* went +x+y+z */
		}
		else if (go < 25)
		{
		    if (go == 22)				/* went +x+y-z */
		    {
			Test_px_with_mx
			Test_py_with_my
			if (k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 24)				/* went +x-y */
		    {
			Test_px_with_mx
			Test_my_with_py
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		}
		else if (go > 25)
		{
		    if (go == 32)				/* went -x */
		    {
			Test_mx_with_px
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 26)				/* went +x-y-z */
		    {
			Test_px_with_mx
			Test_my_with_py
			if (k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 33)				/* went -x+z */
		    {
			Test_mx_with_px
			if (k != nz-1) { Test_pz_with_mz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went -x+z */
		    }
		}
	    }
	    else if (go > 34)
	    {
		if (go == 38)					/* went -x+y-z */
		{
		    Test_mx_with_px
		    Test_py_with_my
		    if (k != 0) { Test_mz_with_pz }
		    if (ans_v) Save_ma_voxel(tmp,pos);		/* went -x-y-z */
		}
		else if (go < 38)
		{
		    if (go == 36)				/* went -x+y */
		    {
			Test_mx_with_px
			Test_py_with_my
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 37)				/* went -x+y+z */
		    {
			Test_mx_with_px
			Test_py_with_my
			if (k != nz-1) { Test_pz_with_mz }
			if (ans_v) Save_ma_voxel(tmp,pos);	/* went -x-y+z */
		    }
		}
		else if (go > 38)
		{
		    if (go == 40)				/* went -x-y */
		    {
			Test_mx_with_px
			Test_my_with_py
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 41)				/* went -x-y+z */
		    {
			Test_mx_with_px
			Test_my_with_py
			if (k != nz-1) { Test_pz_with_mz }
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		    else if (go == 42)				/* went -x-y-z */
		    {
			Test_mx_with_px
			Test_my_with_py
			if (k != 0) { Test_mz_with_pz }
			if (ans_v) Save_ma_voxel(tmp,pos);
		    }
		}
	    }
	}
}
