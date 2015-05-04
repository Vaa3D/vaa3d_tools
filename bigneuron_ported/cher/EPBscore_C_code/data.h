/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

/*
 *			data.h
 *
 *	This header contains the data structures necessary to implement
 *	the Lee, Kashyap, and Chu medial surface/axis algorithm
 */

#ifndef _data_h
#define _data_h

#include <stdio.h>
#include <math.h>


#define NUM_NEIGHBORS   26 /* number of neighbors of a point */
#define NUM_OCTANTS      9 /* number of octants + 1 for consistency */
#define NUM_OCTANT_NODES 7 /* number of nodes in octant minus */
			   /* reference point */
#define NUM_NODE_OCTANT  5 /* max number of octants a node may */
			   /* belong to + 1 */
#define INIT_LABEL       2 /* initial label in object numbers */

#define MATERIAL         0 /* material phase value ** DO NOT CHANGE */
#define VOIDVOX             1 /* void phase value     ** DO NOT CHANGE */
#define EXTERIOR       255 /* Exterior phase */
#define EDGE_VOID      254 /* Void  touching exterior */
#define KEEP_THIS_VOX  251 /* A voxel not to be thinned. Used in throat_surf */
			   /* and thin_cluster algorithms which use thinning */

#define REALLOC_PARAM 0.05 /* 5% of the data size is used as an estimate   */
			   /* for # of each border type. The same fraction */
			   /* is used to reallocate storage when required. */

#define NV_MARKED 9	/* used in point->node_value */

#ifdef DEBUG
#define debug 1		/* debugging option */
#else
#define debug 0
#endif


/* DATA STRUCTURE */

typedef struct {
	unsigned char	*data;
	int		xsize, ysize, zsize;
	int		*stencil;
	int		sten6[6];
} data_info;


/* POINT STRUCTURE */

typedef struct {
	int		x, y, z; /* coordinates in Z^3 */
	int		place;   /* coordinate  in Z^1 */
	unsigned char	*node_value;
	unsigned char	**node_list;
	unsigned char	**octant_list;
} point_info;


struct Border_type{
	int		   indx;
	struct Border_type *next;
	struct Border_type *prev;
};

typedef struct Border_type border_type;


/* DEFINE GLOBAL VARIABLES */

/*
*	The following gives the list of nodes associated with
*	each octant minus the point of reference.
*/

/****	Not compatible with olist[][] - DISGARD
static unsigned char nlist[9][7] = { {  0,  0,  0,  0,  0,  0,  0 },
				     { 19, 18, 22, 21, 11, 10, 13 },
				     { 18, 21, 10, 17, 20,  9, 12 },
				     { 22, 21, 13, 25, 24, 16, 15 },
				     { 21, 20, 12, 24, 15, 23, 14 },
				     { 11, 10, 13,  2,  1,  5,  4 },
				     { 10,  9, 12,  1,  4,  0,  3 },
				     { 13, 16, 15,  5,  4,  8,  7 },
				     { 12, 15, 14,  4,  3,  7,  6 } };
****/

static unsigned char nlist[9][7] = { { 0, 0, 0, 0, 0, 0, 0 } ,
				     { 0, 1, 3, 4, 9,10,12 } ,
                                     { 1, 2, 4, 5,10,11,13 } ,
                                     { 3, 4, 6, 7,12,14,15 } ,
                                     { 4, 5, 7, 8,13,15,16 } ,
                                     { 9,10,12,17,18,20,21 } ,
                                     {10,11,13,18,19,21,22 } ,
                                     {12,14,15,20,21,23,24 } , 
                                     {13,15,16,21,22,24,25 } };

/*
*	The following contains the list of nodes and the octants to which
*	the node is a member. The first number is the number of octants
*	and the following numbers are the list of octants themselves.
*/

static unsigned char olist[26][5] = { { 1, 1,0,0,0 }, { 2, 1,2,0,0 },
				      { 1, 2,0,0,0 }, { 2, 1,3,0,0 },
				      { 4, 1,2,3,4 }, { 2, 2,4,0,0 },
				      { 1, 3,0,0,0 }, { 2, 3,4,0,0 },
				      { 1, 4,0,0,0 }, { 2, 1,5,0,0 },
				      { 4, 1,2,5,6 }, { 2, 2,6,0,0 },
				      { 4, 1,3,5,7 }, { 4, 2,4,6,8 },
				      { 2, 3,7,0,0 }, { 4, 3,4,7,8 },
				      { 2, 4,8,0,0 }, { 1, 5,0,0,0 },
				      { 2, 5,6,0,0 }, { 1, 6,0,0,0 },
				      { 2, 5,7,0,0 }, { 4, 5,6,7,8 },
				      { 2, 6,8,0,0 }, { 1, 7,0,0,0 },
				      { 2, 7,8,0,0 }, { 1, 8,0,0,0 } };

static unsigned char nblist[26][5] = { { 3, 1,3,9,0 }, { 3, 0,2,10,0 },
				      { 3, 1,5,11,0 }, { 4,0,4,6,12 },
				      { 4, 1,3,5,7 }, { 4, 2,4,8,13 },
				      { 3, 3,7,14,0 }, { 4, 4,6,8,15 },
				      { 3, 5,7,16,0 }, { 4, 0,10,12,17 },
				      { 4, 1,9,11,18 }, { 4, 2,10,13,19 },
				      { 4, 3,9,14,20 }, { 4, 5,11,16,22 },
				      { 4, 6,12,15,23 }, { 4, 7,14,16,24 },
				      { 4, 8,13,15,25 }, { 3, 12,18,20,0 },
				      { 3, 10,17,19,0 }, { 3, 16,18,22,0 },
				      { 4, 12,17,21,23 }, { 4, 18,20,22,24 },
				      { 4, 13,19,21,25 }, { 3, 14,20,24,0 },
				      { 4, 15,21,23,25 }, { 3, 16,22,24,0 } };

static unsigned char Border_code[6] = {02,04,010,020,040,0100};
#endif	/* _data_h */