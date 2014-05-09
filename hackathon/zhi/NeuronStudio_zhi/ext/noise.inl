
#include <math/nsrandom.h>

nsdouble GenerateNoise( nsint k, nsdouble level )
	{
	nsint     i;
	nsdouble  x = 0.0;


	for( i = 0; i < k; i++ )
		x = x + ( ( nsdouble )ns_global_randf() );

	x = x - ( ( nsdouble )k / 2.0 );
	x = x * ns_sqrt( 12.0 / ( nsdouble )k ) * level / 3.0;

	return x;
	}
