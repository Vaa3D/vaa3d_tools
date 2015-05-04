/*
 *	(C) Copr. 1986-92 Numerical Recipes Software
 */

#include "stdafx.h"

#include <stdio.h>
#include "min_surf.h"
#include "min_surf_prototype.h"
#include "ma_prototype.h"

#define NR_END 1
#define SWAP(a,b) itemp = (a); (a) = (b); (b) = itemp;
#define M 7
#define NSTACK 50


void	indexx_for(int n, network_node **arr, int *indx)
{
	float	a;

	int	*istack;
	int	jstack = 0;

	int	ir = n;
	int	i, j, k, l = 1;
	int	indxt, itemp;

	istack = ivector(1,NSTACK);
	for( j = 1;  j <= n;  j++ ) indx[j]=j;
	for( ; ; )
	{
	    if( ir-l < M )
	    {
		for( j = l+1;  j <= ir;  j++ )
		{
			indxt = indx[j];
			a = arr[indxt]->forward_label;
			for( i = j-1; i >= 1;  i-- )
			{
				if( arr[indx[i]]->forward_label <= a ) break;
				indx[i+1] = indx[i];
			}
			indx[i+1] = indxt;
		}
		if( jstack == 0 ) break;
		ir = istack[jstack--];
		l = istack[jstack--];
	    }
	    else
	    {
		k = (l+ir) >> 1;
		SWAP(indx[k],indx[l+1]);
		if( arr[indx[l+1]]->forward_label > arr[indx[ir]]->forward_label )
		{
			SWAP(indx[l+1],indx[ir])
		}
		if( arr[indx[l]]->forward_label > arr[indx[ir]]->forward_label )
		{
			SWAP(indx[l],indx[ir])
		}
		if( arr[indx[l+1]]->forward_label > arr[indx[l]]->forward_label )
		{
			SWAP(indx[l+1],indx[l])
		}
		i = l+1;
		j = ir;
		indxt = indx[l];
		a = arr[indxt]->forward_label;
		for( ; ; )
		{
			do i++;	while (arr[indx[i]]->forward_label < a);
			do j--; while (arr[indx[j]]->forward_label > a);

			if( j < i ) break;

			SWAP(indx[i],indx[j])
		}
		indx[l] = indx[j];
		indx[j] = indxt;

		jstack += 2;
		if( jstack > NSTACK ) nrerror("NSTACK too small in indexx.");

		if( ir-i+1 >= j-l )
		{
			istack[jstack  ] = ir;
			istack[jstack-1] = i;
			ir = j-1;
		} else
		{
			istack[jstack  ] = j-1;
			istack[jstack-1] = l;
			l = i;
		}
	    }
	}
	free_ivector(istack,1,NSTACK);
}


void	indexx_back(int n, network_node **arr, int *indx)
{
	float	a;

	int	*istack;
	int	jstack = 0;
	
	int	ir = n;
	int	i, j, k, l = 1;
	int	indxt, itemp;

	istack = ivector(1,NSTACK);
	for( j = 1;  j <= n;  j++ ) indx[j] = j;
	for( ; ; )
	{
	    if( ir-l < M )
	    {
		for( j = l+1;  j <= ir;  j++ )
		{
			indxt = indx[j];
			a = arr[indxt]->backward_label;
			for( i = j-1;  i >= 1;  i-- )
			{
				if( arr[indx[i]]->backward_label <= a ) break;
				indx[i+1] = indx[i];
			}
			indx[i+1] = indxt;
		}
		if( jstack == 0 ) break;

		ir = istack[jstack--];
		l  = istack[jstack--];
	    }
	    else
	    {
		k = (l+ir) >> 1;
		SWAP(indx[k],indx[l+1]);
		if( arr[indx[l+1]]->backward_label > arr[indx[ir]]->backward_label )
		{
			SWAP(indx[l+1],indx[ir])
		}
		if( arr[indx[l]]->backward_label > arr[indx[ir]]->backward_label )
		{
			SWAP(indx[l],indx[ir])
		}
		if( arr[indx[l+1]]->backward_label > arr[indx[l]]->backward_label )
		{
			SWAP(indx[l+1],indx[l])
		}
		i = l+1;
		j = ir;
		indxt = indx[l];
		a = arr[indxt]->backward_label;
		for( ; ; )
		{
			do i++; while (arr[indx[i]]->backward_label < a);
			do j--; while (arr[indx[j]]->backward_label > a);

			if( j < i ) break;

			SWAP(indx[i],indx[j])
		}
		indx[l] = indx[j];
		indx[j] = indxt;

		jstack += 2;
		if( jstack > NSTACK ) nrerror("NSTACK too small in indexx.");

		if( ir-i+1 >= j-l )
		{
			istack[jstack  ] = ir;
			istack[jstack-1] = i;
			ir = j-1;
		} else
		{
			istack[jstack  ] = j-1;
			istack[jstack-1] = l;
			l = i;
		}
	    }
	}
	free_ivector(istack,1,NSTACK);
}


/*
*	Numerical Recipes standard error handler
*/

void	nrerror(char *error_text)
{
	fprintf(stderr,"indexx_...() run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");

	clean_up(0);
}


/*
*	allocate an int vector with subscript range v[nl..nh]
*/

int	*ivector(int nl, int nh)
{
	char	*msg;
	int	*v;

	int	isz = sizeof(int);

	v = (int *)MALLOC((nh-nl+1+NR_END)*isz);
	msg = "v in num recipes ivector()";
	if( MEMCHECK(v,msg,(nh-nl+1+NR_END)*isz) ) clean_up(0);
	
	return v-nl+NR_END;
}


/*
*	free an int vector allocated with ivector()
*/

void	free_ivector(int *v, int nl, int nh)
{
	int	*ptr;
	int	isz = sizeof(int);
	
	ptr = v + nl - NR_END;
	FREE(ptr,(nh-nl+1+NR_END)*isz);
}

#undef NSTACK
#undef M
#undef SWAP
#undef NR_END
