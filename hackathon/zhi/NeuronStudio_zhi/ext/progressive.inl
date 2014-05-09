
typedef struct
	{
	nsint  count;
	nsint  stride;
	nsint  last;
	nsint  iterations;
	}
	PROGRESSIVE;


void progressive_init( PROGRESSIVE *pr, nsint count )
	{
	nsint i = 1;

	pr->count = count;

	while( i <= count )
		i *= 2;

	pr->stride = i * 2;
	pr->last   = i * 2;

	pr->iterations = ( nsint )( ns_log10( ( nsdouble )pr->count ) / ns_log10( 2.0 ) ) + 1;
	}


nsint progressive_get_next( PROGRESSIVE *pr )
	{
	pr->last = pr->last + pr->stride;

	if( pr->last > pr->count )
		{
		pr->stride = pr->stride / 2;
		pr->last   = pr->stride / 2;
		}

	return pr->last;
	}
