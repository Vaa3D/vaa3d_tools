// Some C++ Implementations for the matlab classic calls
#include "matmath.h"
#include "assert.h"

using namespace std;

vectype linspace(const double a, const double b, const int n) {
// Equals to linspace in matlab
    vectype outvec (n);
    double step = (b-a) / (n-1);
    double t = a;
    vectype::iterator outitr = outvec.begin();
    while(t <= b && outitr != outvec.end()) {
        *outitr = t;
        outitr++;
        t += step;           // could recode to better handle rounding errors
    }
    return outvec;
}


vectype repmat1d(vectype invec, int repeat, int dim){
	assert(invec.size() != 0);
	const int nvar = repeat * invec.size();
	vectype outvec (nvar);
    vectype::iterator outitr = outvec.begin();
	int out_szrow, out_szcol;

	switch (dim){
	    case 1:
	        {
	        out_szrow = repeat;
	        out_szcol = invec.size();
			// Assume the caller has allocated the memory for outarray
			for (int col = 0; col < out_szcol; col++)
			    for(int row = 0; row < out_szrow; row++, outitr++)
			    {
		            *outitr = invec[col];
			    }
		    break;
			}
	    case 2:
	        {
	        out_szrow = invec.size();
	        out_szcol = repeat;
			// Assume the caller has allocated the memory for outarray
			for (int col = 0; col < out_szcol; col++)
			    for(int row = 0; row < out_szrow; row++, outitr++)
			    {
		            *outitr = invec[row];
			    }
		    break;
		    }
	    default: throw 1; break;
	}

	return outvec;
}


vectype transpose(const vectype invec, const int N, const int M) {
    vectype outvec(invec.size());
    vectype::iterator outitr = outvec.begin();
    int i,j;

	for (int n=0; n<M*N; n++, outitr++)
	{
        i = n/N;
        j = n%N;
        *outitr = invec[M*j + i];
    }

    return outvec;
}

