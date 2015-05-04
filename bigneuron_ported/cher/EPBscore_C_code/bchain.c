/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "ma_prototype.h"


void	initialize_chains(char type, int **chain, int *num, int chain_size)
{

	int	isz = sizeof(int);
	
	*num   = 0;
	*chain = (int *)MALLOC(chain_size*isz);
}


void	realloc_chains(char type, int **chain, int chain_size,
					   int size_chng)
{

	int	isz = sizeof(int);

  	*chain = (int *)REALLOC(*chain,chain_size*isz);
}


void	free_chains(int *chain, int chain_size)
{
	int	isz = sizeof(int);
	FREE(chain,chain_size*isz);
}


void	set_first_chain(Bchains *bc, unsigned char *cube, int bdry_num,
						int nxyz)
{
	int	i, d;

	bc->chain_size = 4*bdry_num; 
	if( bc->chain_size < 500 ) bc->chain_size = 500;

	PRINTF("\nstarting grain bdry # %d   ",bdry_num);
	PRINTF("bc->chain_size %d\n",bc->chain_size);

	initialize_chains('o',&(bc->o_chain),&(bc->o_num),bc->chain_size);
	bc->o_num = bdry_num;

	d = 0;
	for( i = 0;  i < nxyz;  i++ )
	{
		if( cube[i] == 0 ) continue;

		bc->o_chain[d] = i;
		d++;
	}

	if( d != bdry_num )
	{
		PRINTF("logic ERROR in set_first_chain()\n");
		clean_up(0);
	}

	initialize_chains('n',&(bc->n_chain),&(bc->n_num),bc->chain_size);
}



void	add_to_chain(Bchains *bc, int v_ind, int realloc_size)
{
	if( bc->n_num >= bc->chain_size )
	{
		bc->chain_size += realloc_size;
		realloc_chains('o',&(bc->o_chain),bc->chain_size,realloc_size);
		realloc_chains('n',&(bc->n_chain),bc->chain_size,realloc_size);
	}

	bc->n_chain[bc->n_num] = v_ind;

	(bc->n_num)++;
}


void	set_next_chain(Bchains *bc, unsigned char *bfdat,
					   unsigned char b_step)
{
	int	i, pos;

	for( i = 0;  i < bc->n_num;  i++ )
	{
		pos = bc->n_chain[i];
		bfdat[pos] = b_step;
		bc->o_chain[i] = pos;
	}

	bc->o_num = bc->n_num;
	bc->n_num = 0;
}
