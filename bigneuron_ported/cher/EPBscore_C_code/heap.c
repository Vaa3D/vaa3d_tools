/*
 *		Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include "maset.h"


/*
*	C indexed heapsort. Sorts ra[] from ra[0] to ra[n-1]
*/

#define	C_heapsort()\
{\
	int	l, j, ir, i;\
\
	l = n >> 1;					/* l = n/2 */\
	ir = n-1;\
\
	for(;;)\
	{\
		if( l > 0 ) rra = ra[--l];\
		else\
		{\
			rra = ra[ir];\
			ra[ir] = ra[0];\
			if( --ir == 0 )\
			{\
				ra[0] = rra;\
				return;\
			}\
		}\
		i = l;\
		j = (l << 1) + 1;			/* j = 2*l + 1 */\
		while( j <= ir )\
		{\
			if( j < ir && ra[j] < ra[j+1] ) ++j;\
			if( rra < ra[j] )\
			{\
				ra[i] = ra[j];\
				i = j;\
				j = (j << 1) + 1;	/* j = 2*j + 1 */\
			}\
			else j = ir+1;\
		}\
		ra[i] = rra;\
	}\
}


void	int_C_heapsort(int n, int *ra)
{
	int	rra;

	C_heapsort()
}


void	float_C_heapsort(int n, float *ra)
{
	float	rra;

	C_heapsort()
}


void	double_C_heapsort(int n, double *ra)
{
	double	rra;

	C_heapsort()
}


#define Two_var_heapsort(n,pl,ma)\
{\
	int	l, j, ir, i;\
\
	l = n >> 1;					/* l = n/2 */\
	ir = n-1;\
\
	for(;;)\
	{\
		if( l > 0 ) { rpl = pl[--l];	rma = ma[l]; }\
		else\
		{\
			   rpl = pl[ir];	   rma = ma[ir];\
			pl[ir] = pl[0];		ma[ir] = ma[0];\
			if( --ir == 0 )\
			{\
				pl[0] = rpl;	ma[0] = rma;\
				return;\
			}\
		}\
		i = l;\
		j = (l << 1) + 1;			/* j = 2*l + 1 */\
		while( j <= ir )\
		{\
			if( (j < ir) && (pl[j] < pl[j+1]) ) ++j;\
			if( rpl < pl[j] )\
			{\
				pl[i] = pl[j];		ma[i] = ma[j];\
				i = j;\
				j = (j << 1) + 1;	/* j = 2*j + 1 */\
			}\
			else j = ir+1;\
		}\
		pl[i] = rpl;	ma[i] = rma;\
	}\
}


void	cluster_path_heapsort(int n, int *pl, MA_set **ma)
{
	int	rpl;
	MA_set	*rma;

	Two_var_heapsort(n,pl,ma)
}


void	two_int_heapsort(int n, int *pl, int *ma)
{
	int	rpl, rma;

	Two_var_heapsort(n,pl,ma)
}


void	float_int_heapsort(int n, float *pl, int *ma)
{
	float	rpl;
	int	rma;

	Two_var_heapsort(n,pl,ma)
}
