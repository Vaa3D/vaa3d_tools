// combine FL_sort.cpp and lvector_phc(), nrerror() function (rename to nrerror2() to avoid redefintion 
// when used with pbetai.cpp) in nrutil.cpp
// this is because nrutil.cpp is defined on ANSI C, when a function calling FL_sort2.cpp
// use C++ template vector, it will cause problem
// F. Long
// 20090607
  
#ifndef __FL_SORT__
#define __FL_SORT__
 
//#define NRANSI

//#include "nrutil.cpp"

#include "../../v3d_main/basic_c_fun/v3d_basicdatatype.h"

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
#define M 7
#define NSTACK 50
#define NR_END 1

void nrerror2(char error_text[])     
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

unsigned V3DLONG *lvector_phc(V3DLONG nl, V3DLONG nh)
/* allocate an unsigned V3DLONG vector with subscript range v[nl..nh] */
{
	unsigned V3DLONG *v=0;
    try
    { 
	  v=new unsigned V3DLONG [nh-nl+1+NR_END];
	}
	catch(...)
	{
	  if (!v) nrerror2("allocation failure in lvector_phc()");
	  return 0;
	]
	return v-nl+NR_END;
}

void free_lvector_phc(unsigned V3DLONG *v, V3DLONG nl, V3DLONG nh)
/* free a float vector allocated with vector_phc() */
{
	if (v) {unsigned V3DLONG * p = v+nl-NR_END; if (p) delete []p; }
}

template <class T> void sort2(V3DLONG n, T arr[], float brr[])
{
	unsigned V3DLONG i,ir=n,j,k,l=1;
	unsigned V3DLONG *istack=0,jstack=0;
	T a;
	double b,temp;

	istack=lvector_phc(1,NSTACK);
	if (!istack)
	{
	  printf("No sort performed.\n");
	  return;
	}
	
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				a=arr[j];
				b=brr[j];
				for (i=j-1;i>=1;i--) {
					if (arr[i] <= a) break;
					arr[i+1]=arr[i];
					brr[i+1]=brr[i];
				}
				arr[i+1]=a;
				brr[i+1]=b;
			}
			if (!jstack) {
				free_lvector_phc(istack,1,NSTACK);
				return;
			}
			ir=istack[jstack];
			l=istack[jstack-1];
			jstack -= 2;
		} else {
			k=(l+ir) >> 1;
			SWAP(arr[k],arr[l+1])
			SWAP(brr[k],brr[l+1])
			if (arr[l+1] > arr[ir]) {
				SWAP(arr[l+1],arr[ir])
				SWAP(brr[l+1],brr[ir])
			}
			if (arr[l] > arr[ir]) {
				SWAP(arr[l],arr[ir])
				SWAP(brr[l],brr[ir])
			}
			if (arr[l+1] > arr[l]) {
				SWAP(arr[l+1],arr[l])
				SWAP(brr[l+1],brr[l])
			}
			i=l+1;
			j=ir;
			a=arr[l];
			b=brr[l];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j])
				SWAP(brr[i],brr[j])
			}
			arr[l]=arr[j];
			arr[j]=a;
			brr[l]=brr[j];
			brr[j]=b;
			jstack += 2;
			if (jstack > NSTACK) nrerror2("NSTACK too small in sort2.");
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
	
	free_lvector_phc(istack,1,NSTACK);//should I add one here // by PHC 130723
}
#undef M
#undef NSTACK
#undef SWAP
#undef NRANSI
/* (C) Copr. 1986-92 Numerical Recipes Software . */

#endif
