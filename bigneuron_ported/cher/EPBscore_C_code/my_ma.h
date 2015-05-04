/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _ma_h
#define _ma_h

#include "arch.h"
#include "burn_macro.h"

typedef struct _Bchains
{
	char	*cnt;		/* controls fire direction */ 
	int	*o_chain,	/* step n   burn chain (i,j,k) */
		*n_chain;	/* step n+1 burn chain (i,j,k) */
	int	o_num,		/* length of o_chain */
		n_num;		/* length of n_chain */
	int	chain_size;	/* chain length allocated */
} Bchains;


typedef struct _Fdat
{
	unsigned char  *temp;	  /* temporary storage for one slice of data */
	unsigned char  *fdat;	  /* storage for volume of data */
	unsigned char  *bfdat;	  /* burn directions */
	unsigned char  *cube;	  /* work space */
} Fdat;


#define Save_ma_voxel(tmp,_pos) tmp->cube[_pos] = 254


/* Task options */
#define	ERODE  1
#define MED_AX 2
#define RASTER 3
#define PAD    4
#define CLEAN_UP 5
#define NO_OPT 6
#endif	/* _ma_h */