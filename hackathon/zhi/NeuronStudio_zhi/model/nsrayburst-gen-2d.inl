
#ifdef NS_DEBUG

void ns_rayburst_generate_2d_output
	(
	const NsVector3f       *vectors,
	const NsRayburstPlane  *planes,
	nssize                  num_samples
	)
	{
	nschar     name[ 64 ];
	nspointer  stream;
	nssize     i;
	NsError    error;


	ns_snprint(
		name,
		NS_ARRAY_LENGTH( name ),
		"nsrayburst-gen-2d-" NS_FMT_ULONG ".txt",
		num_samples
		);

	if( NS_FAILURE( _ns_fopen( &stream, name, "w" ), error ) )
		return;

	ns_print_set_io( stream );

	for( i = 0; i < num_samples; ++i )
		ns_println( "\t{ %9ff, %9ff, %9ff },", vectors[i].x, vectors[i].y, vectors[i].z );

	for( i = 0; i < num_samples; ++i )
		ns_println(
			"\t{ { %4.1ff, %4.1ff, %4.1ff }, { %.1ff, %.1ff, %.1ff } },",
			planes[i].d_delta.x, planes[i].d_delta.y, planes[i].d_delta.z,
			planes[i].d.x, planes[i].d.y, planes[i].d.z
			);

	ns_print_set_io( NULL );

	/*error*/_ns_fclose( &stream );
	}

#endif


void ns_rayburst_generate_2d
	(
	NsVector3f       *vectors,
	NsRayburstPlane  *planes,
	nssize            num_samples,
	nsboolean         do_360
	)
	{
	nsfloat  v[3], angle, theta, step;
	nssize   i;


	ns_assert( NULL != vectors );
	ns_assert( NULL != planes );

	theta = do_360 ? 360.0f : 180.0f;
	step  = theta / ( nsfloat )num_samples;

	for( i = 0, angle = 0.0f; angle < theta; ++i, angle += step )
		{
		v[0] = ( nsfloat )ns_cos( NS_DEGREES_TO_RADIANS( angle ) );
		v[1] = ( nsfloat )ns_sin( NS_DEGREES_TO_RADIANS( angle ) );
		v[2] = 0.0f;

		if( NS_FLOAT_EQUAL( v[0], 0.0f ) )v[0] = 0.0f;
		if( NS_FLOAT_EQUAL( v[1], 0.0f ) )v[1] = 0.0f;
		if( NS_FLOAT_EQUAL( v[2], 0.0f ) )v[2] = 0.0f;

		ns_assert( i < num_samples );

		vectors[i].x = v[0];
		vectors[i].y = v[1];
		vectors[i].z = v[2];

		ns_assert( NS_FLOAT_EQUAL( ns_vector3f_mag( vectors + i ), 1.0f ) );

		planes[i].d_delta.x = v[0] < 0.0f ? 1.0f : -1.0f;
		planes[i].d_delta.y = v[1] < 0.0f ? 1.0f : -1.0f;
		planes[i].d_delta.z = v[2] < 0.0f ? 1.0f : -1.0f;
		planes[i].d.x       = v[0] < 0.0f ? 0.0f :  1.0f;
		planes[i].d.y       = v[1] < 0.0f ? 0.0f :  1.0f;
		planes[i].d.z       = v[2] < 0.0f ? 0.0f :  1.0f;
		}

	ns_assert( i == num_samples );
	}
