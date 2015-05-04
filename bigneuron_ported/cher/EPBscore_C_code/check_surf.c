/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include "data.h"
#include "lkc_prototype.h"

/*
*	The vertices of the octant are labeled
*
*		in decimal: 1 -> 8,
*	and also in binary: 1, 2, 4, 8, 16, 32, 64, 128
*
*	Here is the conversion:
*
*	1 -> 128,	2 -> 64,	3 -> 32,	4 -> 16,
*	5 -> 8,		6 -> 4,		7 -> 2,		8 -> 1
*
*
*                         2 -> 64----------------4 <- 6
*                             / |               /|
*                            /  |              / |
*                           /   |             /  |
*                    1 -> 128----------------8 <-|5
*                          |    |            |   | 
*                          |    |            |   | 
*                          |    |            |   | 
*                         4|-> 16------------|---1 <- 8
*                          |  /              |  /
*                          | /               | /
*                          |/                |/
*                    3 -> 32-----------------2 <- 7
*
*
*
*	When routines in this file are accessed,
*	point->node_value contains only the entries NV_MARKED and
*	MATERIAL. point->node_value contains the (NV_MARKED/MATERIAL)
*	entries for the corners (2,4,8,16,32,64 and 128) but not
*	for corner 1 which is of type (VOID == 1).
*	(The NV_MARKED entries are VOID entries that have been `re-marked').
*/

/*	The numbers below are hard coded for speed. They must be consistent
*	with the numerical values of NV_MARKED and VOID as set in data.h
*/

#define P5678  127 	/* NV_MARKED * (  8+ 4+2) + VOID, plane 00001111 */
#define P1368 1477 	/* NV_MARKED * (128+32+4) + VOID, plane 10100101 */
#define P2468  757 	/* NV_MARKED * ( 64+16+4) + VOID, plane 01010101 */
#define P3478  451 	/* NV_MARKED * ( 32+16+2) + VOID, plane 00110011 */
#define P1458 1369 	/* NV_MARKED * (128+16+8) + VOID, plane 10011001 */
#define P1278 1747 	/* NV_MARKED * (128+64+2) + VOID, plane 11000011 */

#define P1234 2160 	/* NV_MARKED * (128+64+32+16), plane 11110000 */
#define P1256 1836 	/* NV_MARKED * (128+64+ 8+ 4), plane 11001100 */
#define P1357 1530 	/* NV_MARKED * (128+32+ 8+ 2), plane 10101010 */


#define Endpt_Chck(_I1,_I2,_I3,_I4,_I5,_I6,_I7)\
	decim = 128 * nodeval[_I1] + 64 * nodeval[_I2] + 32 * nodeval[_I3]\
	      +  16 * nodeval[_I4] +  8 * nodeval[_I5] +  4 * nodeval[_I6]\
	      +   2 * nodeval[_I7] +  1;\
\
	if( ! endpoint_check(decim) )\
	{\
		kount = 0;\
		if( nodeval[_I1] == NV_MARKED ) kount++;\
		if( nodeval[_I2] == NV_MARKED ) kount++;\
		if( nodeval[_I3] == NV_MARKED ) kount++;\
		if( nodeval[_I4] == NV_MARKED ) kount++;\
		if( nodeval[_I5] == NV_MARKED ) kount++;\
		if( nodeval[_I6] == NV_MARKED ) kount++;\
		if( nodeval[_I7] == NV_MARKED ) kount++;\
\
		if( kount >= 3 ) return 0;\
	}


int	check_surface_edge(point_info *point)
{
	unsigned char *nodeval;

	int	decim, kount;

	nodeval = point->node_value;

	Endpt_Chck(25,24,16,15,22,21,13)
	Endpt_Chck(19,22,11,13,18,21,10)
	Endpt_Chck(23,20,14,12,24,21,15)
	Endpt_Chck(17,18, 9,10,20,21,12)
	Endpt_Chck( 8, 7,16,15, 5, 4,13)
	Endpt_Chck( 2, 5,11,13, 1, 4,10)
	Endpt_Chck( 6, 3,14,12, 7, 4,15)
	Endpt_Chck( 0, 1, 9,10, 3, 4,12)

	return 1;
}


int	endpoint_check(int decim)
{
	     if( decim == P5678 ) return 1;	/* plane 5678 (00001111) */
	else if( decim == P1368 ) return 1;	/* plane 1368 (10100101) */
	else if( decim == P2468 ) return 1;	/* plane 2468 (01010101) */
	else if( decim == P3478 ) return 1;	/* plane 3478 (00110011) */
	else if( decim == P1458 ) return 1;	/* plane 1458 (10011001) */
	else if( decim == P1278 ) return 1;	/* plane 1278 (11000011) */
	else			  return 0;
}


#define Chck_int_face(_I,_Iopp)\
{\
	if(    (data[place+sten[_I   ]] == MATERIAL)\
	    && (data[place+sten[_Iopp]] == MATERIAL)   )\
	{\
		ok = check_interior_face(ptinfo);\
		return ok;\
	}\
}


/*
*	CAUTION. This routine changes the entries in ptinfo.
*	Be careful in using ptinfo after calling this routine
*/

int	mod_check_surface_edge(point_info *ptinfo, data_info *datainfo,
						   unsigned char dir, int bdry_alg)
{
	unsigned char	*data;
	unsigned char	dps4, dps10, dps12, dps13, dps15, dps21;
	int		i, place, ok, kount, zcoord, xy;
	int		*sten;

	
	place = ptinfo->place;
	data  = datainfo->data;
	sten  = datainfo->stencil;

	xy    = (datainfo->xsize)*(datainfo->ysize);

	dps4  = data[place+sten[ 4]];
	dps10 = data[place+sten[10]];
	dps12 = data[place+sten[12]];
	dps13 = data[place+sten[13]];
	dps15 = data[place+sten[15]];
	dps21 = data[place+sten[21]];

	if( dir == 2 )
	{
	    if( bdry_alg == 0 )
	    {
		kount = 0;
		if( (dps4  == MATERIAL) || (dps4  == EXTERIOR) ) kount++;
		if( (dps21 == MATERIAL) || (dps21 == EXTERIOR) ) kount++;
		if( (dps10 == MATERIAL) || (dps10 == EXTERIOR) ) kount++;
		if( (dps12 == MATERIAL) || (dps12 == EXTERIOR) ) kount++;
		if( (dps13 == MATERIAL) || (dps13 == EXTERIOR) ) kount++;
		if( (dps15 == MATERIAL) || (dps15 == EXTERIOR) ) kount++;		
		if( kount <= 1 ) return 1;
	    }
	    else if( bdry_alg == 1 )
	    {
		kount = 0;
		if(dps4  == MATERIAL) kount++;
		if(dps21 == MATERIAL) kount++;
		if(dps10 == MATERIAL) kount++;
		if(dps12 == MATERIAL) kount++;
		if(dps13 == MATERIAL) kount++;
		if(dps15 == MATERIAL) kount++;		

		if( kount <= 1 ) return 1;
	    }
	    else if( bdry_alg == 2 )
	    {
		kount = 0;
		if( (dps4  == MATERIAL) || (dps4  == EXTERIOR) ) kount++;
		if( (dps21 == MATERIAL) || (dps21 == EXTERIOR) ) kount++;
		if( (dps10 == MATERIAL) || (dps10 == EXTERIOR) ) kount++;
		if( (dps12 == MATERIAL) || (dps12 == EXTERIOR) ) kount++;
		if( (dps13 == MATERIAL) || (dps13 == EXTERIOR) ) kount++;
		if( (dps15 == MATERIAL) || (dps15 == EXTERIOR) ) kount++;		
		zcoord = place/xy;
		if( zcoord == 1                     ) kount--;
		if( zcoord == (datainfo->zsize) - 2 ) kount--;
		
		if( kount <= 1 ) return 1;
	    }	
	}

	for( i = 0;  i < 26;  i++ )
	{
		if( !(    (ptinfo->node_value[i]  != MATERIAL)
		       && (   data[place+sten[i]] == VOIDVOX    )    ) )
			ptinfo->node_value[i] = MATERIAL;
	}
	
	if( (dps4 == MATERIAL) && (dps21 == MATERIAL) )
	{
	    if( (dps10 > MATERIAL) && (dps10 != EXTERIOR) ) Chck_int_face(1,18)
	    if( (dps15 > MATERIAL) && (dps15 != EXTERIOR) ) Chck_int_face(7,24)
	    if( (dps12 > MATERIAL) && (dps12 != EXTERIOR) ) Chck_int_face(3,20)
	    if( (dps13 > MATERIAL) && (dps13 != EXTERIOR) ) Chck_int_face(5,22)
	}

	if( (dps10 == MATERIAL) && (dps15 == MATERIAL) )
	{
	    if( (dps4  > MATERIAL) && (dps4  != EXTERIOR) ) Chck_int_face( 1, 7)
	    if( (dps21 > MATERIAL) && (dps21 != EXTERIOR) ) Chck_int_face(18,24)
	    if( (dps12 > MATERIAL) && (dps12 != EXTERIOR) ) Chck_int_face( 9,14)
	    if( (dps13 > MATERIAL) && (dps13 != EXTERIOR) ) Chck_int_face(11,16)
	}

	if( (dps12 == MATERIAL) && (dps13 == MATERIAL) )
	{
	    if( (dps4  > MATERIAL) && (dps4  != EXTERIOR) ) Chck_int_face( 3, 5)
	    if( (dps21 > MATERIAL) && (dps21 != EXTERIOR) ) Chck_int_face(20,22)
	    if( (dps10 > MATERIAL) && (dps10 != EXTERIOR) ) Chck_int_face( 9,11)
	    if( (dps15 > MATERIAL) && (dps15 != EXTERIOR) ) Chck_int_face(14,16)
	}
	return 0;
}


#define Cif(_I128,_I64,_I32,_I16,_I8,_I4,_I2)\
	decim1 = 128 * nodeval[_I128] + 64 * nodeval[_I64] +\
		  32 * nodeval[_I32 ] + 16 * nodeval[_I16];\
\
	decim2 = 128 * nodeval[_I128] + 64 * nodeval[_I64] +\
		   8 * nodeval[_I8  ] +  4 * nodeval[_I4 ];\
\
	decim3 = 128 * nodeval[_I128] + 32 * nodeval[_I32] +\
		   8 * nodeval[_I8  ] +  2 * nodeval[_I2 ];\
\
	ok1 = (decim1 == P1234 || decim2 == P1256 || decim3 == P1357);\
\
	if( ok1 ) return 0


int	check_interior_face(point_info *ptinfo)
{
	unsigned char *nodeval;

	int	ok1, decim1, decim2, decim3;
	
	nodeval = ptinfo->node_value;

	Cif(25,24,16,15,22,21,13);
	Cif(19,22,11,13,18,21,10);
	Cif(23,20,14,12,24,21,15);
	Cif(17,18, 9,10,20,21,12);
	Cif( 8, 7,16,15, 5, 4,13);
	Cif( 2, 5,11,13, 1, 4,10);
	Cif( 6, 3,14,12, 7, 4,15);
	Cif( 0, 1, 9,10, 3, 4,12);
  
	return 1;
}
