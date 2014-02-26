#include "filtermgr.h"


typedef struct tagFILTERSTUB
   {
   nsuchar  *data;
   nslong    dataxdim;
   nslong    dataydim;
   nslong    datazdim;
   nslong    region_x;
   nslong    region_y;
   nslong    region_z;
	nslong    region_width;
	nslong    region_height;
   nslong    count; /* deprecated */
   nsuchar  *destination;
   nslong    threadid;
   nslong    running;
   }
   FILTERSTUB;


typedef struct tagFILTERSTUB16
   {
	nsushort  *data;
	nslong     is12bit;
	nslong     dataxdim;
	nslong     dataydim;
	nslong     datazdim;
	nslong     region_x;
	nslong     region_y;
	nslong     region_z;
	nslong     region_width;
	nslong     region_height;
	nslong     count; /* deprecated */
	nsushort  *destination;
	nslong     threadid;
	nslong     running;
   }
   FILTERSTUB16;


void FilterBlur( FILTERSTUB *stub );
void FilterBlur26( FILTERSTUB *stub );
void FilterMedian( FILTERSTUB *stub );
void FilterInvert( FILTERSTUB *stub );
void FilterZBlur( FILTERSTUB *stub );

void FilterBlur_16bit( FILTERSTUB16 *stub );
void FilterBlur26_16bit( FILTERSTUB16 *stub );
void FilterMedian_16bit( FILTERSTUB16 *stub );
void FilterInvert_16bit( FILTERSTUB16 *stub );
void FilterZBlur_16bit( FILTERSTUB16 *stub );


/* Array of offset from center for surrounding voxels. Values
	are: x offset, y offset, z offset, and number of voxels in
	memory, which is computed for each dataset. */
NS_PRIVATE nslong ____filter_voxoff[26][4] =
	{
	/* faces */
	/* 0*/ {   0,  0,  1,  0  },
	/* 1*/ {   0,  0, -1,  0  },
	/* 2*/ {   0, -1,  0,  0  },
	/* 3*/ {   0,  1,  0,  0  },
	/* 4*/ {  -1,  0,  0,  0  },
	/* 5*/ {   1,  0,  0,  0  },

	/* edges */
	/* 6*/ {  -1,  0,  1,  0  },
	/* 7*/ {   1,  0,  1,  0  },
	/* 8*/ {   0,  1,  1,  0  },
	/* 9*/ {  -1, -1,  0,  0  },
	/*10*/ {   1, -1,  0,  0  },
	/*11*/ {  -1,  1,  0,  0  },
	/*12*/ {   1,  1,  0,  0  },
	/*13*/ {   0, -1, -1,  0  },
	/*14*/ {  -1,  0, -1,  0  },
	/*15*/ {   1,  0, -1,  0  },
	/*16*/ {   0,  1, -1,  0  },

	/* corners */
	/*17*/ {  -1, -1,  1,  0  },
	/*18*/ {   1, -1,  1,  0  },
	/*19*/ {  -1,  1,  1,  0  },
	/*20*/ {   1,  1,  1,  0  },
	/*21*/ {  -1, -1, -1,  0  },
	/*22*/ {   1, -1, -1,  0  },
	/*23*/ {  -1,  1, -1,  0  },
	/*24*/ {   1,  1, -1,  0  },
	/*25*/ {   0, -1,  1,  0  }
	};


// returns zero if memory fails
nsint RunFilter( FILTERDEFINITION *fd )
	{
	nslong       i, y, z;
	nsuchar     *buffer1, *buffer2;
	nsuchar     *curr, *prev, *temp;
	FILTERSTUB  *stubs;
	nslong       y_step;
	void        ( *filterfunc )( FILTERSTUB* );
	nslong       xmemoff,ymemoff,zmemoff;
	nsuchar     *src, *dest;


	// update the offsets array
	xmemoff = 1;
	ymemoff = fd->imagexdim;
	zmemoff = fd->imagexdim * fd->imageydim;

	for( i = 0; i < 26; i++ )
		____filter_voxoff[i][3] =
			____filter_voxoff[i][0] * xmemoff +
			____filter_voxoff[i][1] * ymemoff +
			____filter_voxoff[i][2] * zmemoff;

	// number of threads can't be less than 1 or more than y dimention
	if( fd->threads < 1 )
		fd->threads = 1;

	if( fd->threads > fd->region_height )
		fd->threads = fd->region_height;

	// allocate the image buffers
	buffer1 = ns_malloc( sizeof( nsuchar ) * fd->region_height * fd->region_width );
	buffer2 = ns_malloc( sizeof( nsuchar ) * fd->region_height * fd->region_width );
	prev    = buffer1;
	curr    = buffer2;

	// allocate the filter stubs
	stubs = ns_malloc( sizeof( FILTERSTUB ) * fd->threads );

	// check for insufficient memory
	if( buffer1 == NULL || buffer2 == NULL || stubs == NULL )
		{
		ns_free( buffer1 );
		ns_free( buffer2 );
		ns_free( stubs );

		return 0;
		}

	// fill in stubs
	y_step = fd->region_height / fd->threads;

	for( i = 0; i < fd->threads; i++ )
		{
		stubs[i].data          = fd->image;
		stubs[i].dataxdim      = fd->imagexdim;
		stubs[i].dataydim      = fd->imageydim;
		stubs[i].datazdim      = fd->imagezdim;
		stubs[i].region_x      = fd->region_x;
		stubs[i].region_y      = fd->region_y + i * y_step;
		stubs[i].region_z      = fd->region_z;
		stubs[i].region_width  = fd->region_width;
		stubs[i].region_height = y_step;
		stubs[i].threadid      = i;
		}

	// last one gets remaining rows
	stubs[ fd->threads - 1 ].region_height += ( fd->region_height % fd->threads );

	filterfunc = NULL;

	//set the filter function pointer
	switch( fd->filtertype )
		{
		case FILTER_BLUR:
			filterfunc = FilterBlur;
			break;

		case FILTER_MEDIAN:
			filterfunc = FilterMedian;
			break;

		case FILTER_BLUR26:
			filterfunc = FilterBlur26;
			break;

		case FILTER_INVERT:
			filterfunc = FilterInvert;
			break;

		case FILTER_ZBLUR:
			filterfunc = FilterZBlur;
			break;
		}

	// process image one slice at a time
	for( z = fd->region_z; z < fd->region_z + fd->region_length; z++ )
		{
		if( NULL != fd->progress )
			{
			if( ns_progress_cancelled( fd->progress ) )
				goto _RUN_FILTER_EXIT;

			ns_progress_update(
				fd->progress,
				100.0f * ( ( nsfloat )( z - fd->region_z ) / ( nsfloat )fd->region_length )
				);
			}

		// create threads to process current image
		for( i = 0; i < fd->threads; i++ )
			{
			// update corresponding stub
			stubs[i].region_z    = z;
			stubs[i].destination = curr + i * y_step * fd->region_width;
			stubs[i].running     = 1;

			// notice that last stub is always run from this thread
			if( i < ( fd->threads - 1 ) )
				{
				NsThread thread;
				ns_thread_construct( &thread );
				ns_thread_run( &thread, filterfunc, stubs + i );
            }
			else
				filterfunc( stubs + i );
			}

		//ns_print( "Waiting for threads... " );

		// wait for other threads to finish
		for( i = 0; i < fd->threads; i++ )
			while( stubs[i].running )
				ns_thread_yield();

		//ns_println( "done" );

		// copy previous buffer back to image
		if( z > fd->region_z )
			{
			src  = prev;
			dest = fd->image + ( z - 1 ) * fd->imagexdim * fd->imageydim
					 + fd->region_y * fd->imagexdim + fd->region_x;

			for( y = 0; y < fd->region_height; ++y )
				{
				ns_memcpy( dest, src, sizeof( nsuchar ) * fd->region_width );

				src  += fd->region_width;
				dest += fd->imagexdim;
				}
			}

		// swap buffer pointers
		temp = prev;
		prev = curr;
		curr = temp;
		}

	// copy over the last one
	src  = prev;
	dest = fd->image + ( z - 1 ) * fd->imagexdim * fd->imageydim
			 + fd->region_y * fd->imagexdim + fd->region_x;

	for( y = 0; y < fd->region_height; ++y )
		{
		ns_memcpy( dest, src, sizeof( nsuchar ) * fd->region_width );

		src  += fd->region_width;
		dest += fd->imagexdim;
		}

	_RUN_FILTER_EXIT:

	// free image buffers
	ns_free( buffer1 );
	ns_free( buffer2 );
	ns_free( stubs );

	return 1;
	}


// returns zero if memory fails
nsint RunFilter_16bit( FILTERDEFINITION16 *fd )
	{
	nslong         i, y, z;
	nsushort      *buffer1, *buffer2;
	nsushort      *curr, *prev, *temp;
	FILTERSTUB16  *stubs;
	nslong         y_step;
	void           ( *filterfunc )( FILTERSTUB16* );
	nslong         xmemoff,ymemoff,zmemoff;
	nsushort      *src, *dest;


	// update the offsets array
	xmemoff = 1;
	ymemoff = fd->imagexdim;
	zmemoff = fd->imagexdim * fd->imageydim;

	for( i = 0; i < 26; i++ )
		____filter_voxoff[i][3] =
			____filter_voxoff[i][0] * xmemoff +
			____filter_voxoff[i][1] * ymemoff +
			____filter_voxoff[i][2] * zmemoff;

	// number of threads can't be less than 1 or more than y dimention
	if( fd->threads < 1 )
		fd->threads = 1;

	if( fd->threads > fd->region_height )
		fd->threads = fd->region_height;

	// allocate the image buffers
	buffer1 = ns_malloc( sizeof( nsushort ) * fd->region_height * fd->region_width );
	buffer2 = ns_malloc( sizeof( nsushort ) * fd->region_height * fd->region_width );
	prev    = buffer1;
	curr    = buffer2;

	// allocate the filter stubs
	stubs = ns_malloc( sizeof( FILTERSTUB16 ) * fd->threads );

	// check for insufficient memory
	if( buffer1 == NULL || buffer2 == NULL || stubs == NULL )
		{
		ns_free( buffer1 );
		ns_free( buffer2 );
		ns_free( stubs );

		return 0;
		}

	// fill in stubs
	y_step = fd->region_height / fd->threads;

	for( i = 0; i < fd->threads; i++ )
		{
		stubs[i].data          = fd->image;
		stubs[i].is12bit       = fd->is12bit;
		stubs[i].dataxdim      = fd->imagexdim;
		stubs[i].dataydim      = fd->imageydim;
		stubs[i].datazdim      = fd->imagezdim;
		stubs[i].region_x      = fd->region_x;
		stubs[i].region_y      = fd->region_y + i * y_step;
		stubs[i].region_z      = fd->region_z;
		stubs[i].region_width  = fd->region_width;
		stubs[i].region_height = y_step;
		stubs[i].threadid      = i;
		}

	// last one gets remaining rows
	stubs[ fd->threads - 1 ].region_height += ( fd->region_height % fd->threads );

	filterfunc = NULL;

	//set the filter function pointer
	switch( fd->filtertype )
		{
		case FILTER_BLUR:
			filterfunc = FilterBlur_16bit;
			break;

		case FILTER_MEDIAN:
			filterfunc = FilterMedian_16bit;
			break;

		case FILTER_BLUR26:
			filterfunc = FilterBlur26_16bit;
			break;

		case FILTER_INVERT:
			filterfunc = FilterInvert_16bit;
			break;

		case FILTER_ZBLUR:
			filterfunc = FilterZBlur_16bit;
			break;
		}

	// process image one slice at a time
	for( z = fd->region_z; z < fd->region_z + fd->region_length; z++ )
		{
		if( NULL != fd->progress )
			{
			if( ns_progress_cancelled( fd->progress ) )
				goto _RUN_FILTER_EXIT;

			ns_progress_update(
				fd->progress,
				100.0f * ( ( nsfloat )( z - fd->region_z ) / ( nsfloat )fd->region_length )
				);
			}

		// create threads to process current image
		for( i = 0; i < fd->threads; i++ )
			{
			// update corresponding stub
			stubs[i].region_z    = z;
			stubs[i].destination = curr + i * y_step * fd->region_width;
			stubs[i].running     = 1;

			// notice that last stub is always run from this thread
			if( i < ( fd->threads - 1 ) )
				{
				NsThread thread;
				ns_thread_construct( &thread );
				ns_thread_run( &thread, filterfunc, stubs + i );
            }
			else
				filterfunc( stubs + i );
			}

		//ns_print( "Waiting for threads... " );

		// wait for other threads to finish
		for( i = 0; i < fd->threads; i++ )
			while( stubs[i].running )
				ns_thread_yield();

		//ns_println( "done" );

		// copy previous buffer back to image
		if( z > fd->region_z )
			{
			src  = prev;
			dest = fd->image + ( z - 1 ) * fd->imagexdim * fd->imageydim
					 + fd->region_y * fd->imagexdim + fd->region_x;

			for( y = 0; y < fd->region_height; ++y )
				{
				ns_memcpy( dest, src, sizeof( nsushort ) * fd->region_width );

				src  += fd->region_width;
				dest += fd->imagexdim;
				}
			}

		// swap buffer pointers
		temp = prev;
		prev = curr;
		curr = temp;
		}

	// copy over the last one
	src  = prev;
	dest = fd->image + ( z - 1 ) * fd->imagexdim * fd->imageydim
			 + fd->region_y * fd->imagexdim + fd->region_x;

	for( y = 0; y < fd->region_height; ++y )
		{
		ns_memcpy( dest, src, sizeof( nsushort ) * fd->region_width );

		src  += fd->region_width;
		dest += fd->imagexdim;
		}

	_RUN_FILTER_EXIT:

	// free image buffers
	ns_free( buffer1 );
	ns_free( buffer2 );
	ns_free( stubs );

	return 1;
	}


void FilterBlur( FILTERSTUB *stub )
	{
	nslong    x, y, z, sx, ex, sy, ey;
	nslong    xdim, ydim, zdim;
	nslong    nx, ny, nz, n, i;
	nsdouble  d;
	nslong    value;
   nsuchar  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 4;
			n = 4;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 6; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 6; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 255 )
				value = 255;

			*dest++ = ( nsuchar )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterBlur_16bit( FILTERSTUB16 *stub )
	{
	nslong     x, y, z, sx, ex, sy, ey;
	nslong     xdim, ydim, zdim;
	nslong     nx, ny, nz, n, i;
	nsdouble   d;
	nslong     value;
   nsushort  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 4;
			n = 4;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 6; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 6; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 65535 )
				value = 65535;

			*dest++ = ( nsushort )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterInvert( FILTERSTUB *stub )
	{
   nslong    x, y;
   nsuchar  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	for( y = 0; y < stub->region_height; ++y )
		{
		for( x = 0; x < stub->region_width; ++x )
         *dest++ = ( 255 - ( *src++ ) );

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterInvert_16bit( FILTERSTUB16 *stub )
	{
   nslong     x, y;
   nsushort  *src, *dest;
	nsushort   value;


	value = stub->is12bit ? 4095 : 65535;

	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	for( y = 0; y < stub->region_height; ++y )
		{
		for( x = 0; x < stub->region_width; ++x )
         *dest++ = ( value - ( *src++ ) );

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterBlur26( FILTERSTUB *stub )
	{
	nslong    x, y, z, sx, ex, sy, ey;
	nslong    xdim, ydim, zdim;
	nslong    nx, ny, nz, n, i;
	nsdouble  d;
	nslong    value;
   nsuchar  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 4;
			n = 4;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 26; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 26; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 255 )
				value = 255;

			*dest++ = ( nsuchar )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterBlur26_16bit( FILTERSTUB16 *stub )
	{
	nslong     x, y, z, sx, ex, sy, ey;
	nslong     xdim, ydim, zdim;
	nslong     nx, ny, nz, n, i;
	nsdouble   d;
	nslong     value;
   nsushort  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 4;
			n = 4;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 26; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 26; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 65535 )
				value = 65535;

			*dest++ = ( nsushort )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterZBlur( FILTERSTUB *stub )
	{
	nslong    x, y, z, sx, ex, sy, ey;
	nslong    xdim, ydim, zdim;
	nslong    nx, ny, nz, n, i;
	nsdouble  d;
	nslong    value;
   nsuchar  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 2;
			n = 2;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 2; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 2; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 255 )
				value = 255;

			*dest++ = ( nsuchar )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


void FilterZBlur_16bit( FILTERSTUB16 *stub )
	{
	nslong     x, y, z, sx, ex, sy, ey;
	nslong     xdim, ydim, zdim;
	nslong     nx, ny, nz, n, i;
	nsdouble   d;
	nslong     value;
   nsushort  *src, *dest;


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			d = (*src) * 2;
			n = 2;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 2; ++i )
					{
					d += *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 2; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					d += *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			// compute and save average of all voxels

			d /= n;
			d += 0.5;

			value = ( nslong )d;

			if( value > 65535 )
				value = 65535;

			*dest++ = ( nsushort )value;

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


nsuchar quick_select( nsuchar arr[], nslong n );
void FilterMedian( FILTERSTUB *stub )
	{
	nslong    x, y, z, sx, ex, sy, ey;
	nslong    xdim, ydim, zdim;
	nslong    nx, ny, nz, n, i;
   nsuchar  *src, *dest;
	nsuchar   voxels[ 27 ];


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			voxels[0] = *src;
			n = 1;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 26; ++i )
					{
					voxels[n] = *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 26; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					voxels[n] = *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			*dest++ = quick_select( voxels, n );

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


#define ELEM_SWAP(a,b) { register nsuchar t=(a);(a)=(b);(b)=t; }
nsuchar quick_select(nsuchar arr[], nslong n)
{
    nslong low, high ;
    nslong median;
    nslong middle, ll, hh;

    low = 0;
    high = n-1;
    median = (low + high) / 2;

    for (;;)
    {

        /* One element only */
        if (high <= low) return arr[median] ;

        /* Two elements only */
        if (high == low + 1) 
        {   
            if (arr[low] > arr[high])
            ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (arr[middle] > arr[high]) ELEM_SWAP(arr[middle], arr[high]) ;
        if (arr[low] > arr[high]) ELEM_SWAP(arr[low], arr[high]) ;
        if (arr[middle] > arr[low]) ELEM_SWAP(arr[middle], arr[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP(arr[middle], arr[low+1]) ;

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;)
        {
            do ll++; while (arr[low] > arr[ll]) ;
            do hh--; while (arr[hh] > arr[low]) ;
            if (hh < ll)
            break;
            ELEM_SWAP(arr[ll], arr[hh]) ;
        }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP(arr[low], arr[hh]) ;

        /* Re-set active partition */
        if (hh <= median) low = ll;
        if (hh >= median) high = hh - 1;
    }
}
#undef ELEM_SWAP


nsushort quick_select16( nsushort arr[], nslong n );
void FilterMedian_16bit( FILTERSTUB16 *stub )
	{
	nslong     x, y, z, sx, ex, sy, ey;
	nslong     xdim, ydim, zdim;
	nslong     nx, ny, nz, n, i;
   nsushort  *src, *dest;
	nsushort   voxels[ 27 ];


	src = stub->data +
				stub->region_z * stub->dataxdim * stub->dataydim +
				stub->region_y * stub->dataxdim +
				stub->region_x;

	dest = stub->destination;

	sx   = stub->region_x;
	ex   = stub->region_x + stub->region_width;
	sy   = stub->region_y;
	ey   = stub->region_y + stub->region_height;
	z    = stub->region_z;
	xdim = stub->dataxdim;
	ydim = stub->dataydim;
	zdim = stub->datazdim;

	for( y = sy; y < ey; ++y )
		{
		for( x = sx; x < ex; ++x )
			{
			// add contibutions of all face neighbors

			voxels[0] = *src;
			n = 1;

			// inside voxels don't have out-of-bounds neighbors
			if( x > 0 && x < xdim - 1 &&
				 y > 0 && y < ydim - 1 &&
				 z > 0 && z < zdim - 1   )
				{
				for( i = 0; i < 26; ++i )
					{
					voxels[n] = *( src + ____filter_voxoff[i][3] );
					++n;
					}
				}
			else
				{
				for( i = 0; i < 26; ++i )
					{
					nx = x + ____filter_voxoff[i][0];
					ny = y + ____filter_voxoff[i][1];
					nz = z + ____filter_voxoff[i][2];

					if( nx < 0 || nx >= xdim ||
						 ny < 0 || ny >= ydim ||
						 nz < 0 || nz >= zdim   )
						continue;

					voxels[n] = *( src +____filter_voxoff[i][3] );
					++n;
					}
				}

			*dest++ = quick_select16( voxels, n );

			++src;
			}

		src += ( stub->dataxdim - stub->region_width );
		}

	stub->running = 0;
	}


#define ELEM_SWAP16(a,b) { register nsushort t=(a);(a)=(b);(b)=t; }
nsushort quick_select16(nsushort arr[], nslong n)
{
    nslong low, high ;
    nslong median;
    nslong middle, ll, hh;

    low = 0;
    high = n-1;
    median = (low + high) / 2;

    for (;;)
    {

        /* One element only */
        if (high <= low) return arr[median] ;

        /* Two elements only */
        if (high == low + 1) 
        {   
            if (arr[low] > arr[high])
            ELEM_SWAP16(arr[low], arr[high]) ;
            return arr[median] ;
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (arr[middle] > arr[high]) ELEM_SWAP16(arr[middle], arr[high]) ;
        if (arr[low] > arr[high]) ELEM_SWAP16(arr[low], arr[high]) ;
        if (arr[middle] > arr[low]) ELEM_SWAP16(arr[middle], arr[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP16(arr[middle], arr[low+1]) ;

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;)
        {
            do ll++; while (arr[low] > arr[ll]) ;
            do hh--; while (arr[hh] > arr[low]) ;
            if (hh < ll)
            break;
            ELEM_SWAP16(arr[ll], arr[hh]) ;
        }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP16(arr[low], arr[hh]) ;

        /* Re-set active partition */
        if (hh <= median) low = ll;
        if (hh >= median) high = hh - 1;
    }
}
#undef ELEM_SWAP16
