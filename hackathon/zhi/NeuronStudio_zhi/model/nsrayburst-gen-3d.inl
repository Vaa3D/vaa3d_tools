
void _ns_rayburst_generate_3d_add_triangle
	(
	NsIndexTriangleus  *triangles,
	nssize              a,
	nssize              b,
	nssize              c,
	nssize             *num_triangles,
	nssize              max_triangles
	)
	{
	nssize n;

	n = *num_triangles;

	ns_assert( a <= ( nssize )NS_USHORT_MAX );
	ns_assert( b <= ( nssize )NS_USHORT_MAX );
	ns_assert( c <= ( nssize )NS_USHORT_MAX );

	ns_assert( n < max_triangles );

	triangles[n].a = ( nsushort )a;
	triangles[n].b = ( nsushort )b;
	triangles[n].c = ( nsushort )c;

	++(*num_triangles);
	}


nssize _ns_rayburst_generate_3d_add_vector
	(
	NsVector3f  *vectors,
	nsfloat      x,
	nsfloat      y,
	nsfloat      z,
	nssize      *num_vectors,
	nssize       max_vectors
	)
	{
	nssize i, n;

	n = *num_vectors;
	ns_assert( n <= max_vectors );

	/* Have to check whether vector exists already. */
	for( i = 0; i < n; ++i )
		if( NS_FLOAT_EQUAL( vectors[i].x, x ) &&
			 NS_FLOAT_EQUAL( vectors[i].y, y ) &&
			 NS_FLOAT_EQUAL( vectors[i].z, z )   )
			return i;

	ns_assert( n < max_vectors );

	vectors[n].x = x;
	vectors[n].y = y;
	vectors[n].z = z;

	++(*num_vectors);

	/* 'n' is the index of the vector we just added. */
	return n;
	}


void _ns_rayburst_generate_3d_subdivide
	(
	NsVector3f         *vectors,
	NsIndexTriangleus  *triangles,
	const NsVector3f   *V1,
	nssize              i1,
	const NsVector3f   *V2,
	nssize              i2,
	const NsVector3f   *V3,
	nssize              i3,
	nssize              depth,
	nssize             *num_vectors,
	nssize             *num_triangles,
	nssize              max_vectors,
	nssize              max_triangles
	)
	{
	NsVector3f  V12, V23, V31;
	nssize      i12, i23, i31;


	if( 0 == depth )
		_ns_rayburst_generate_3d_add_triangle(
			triangles,
			i1,
			i2,
			i3,
			num_triangles,
			max_triangles
			);
	else
		{
		ns_vector3f(
			&V12,
			( V1->x + V2->x ) / 2.0f,
			( V1->y + V2->y ) / 2.0f,
			( V1->z + V2->z ) / 2.0f
			);

		ns_vector3f(
			&V23,
			( V2->x + V3->x ) / 2.0f,
			( V2->y + V3->y ) / 2.0f,
			( V2->z + V3->z ) / 2.0f
			);

		ns_vector3f(
			&V31,
			( V3->x + V1->x ) / 2.0f,
			( V3->y + V1->y ) / 2.0f,
			( V3->z + V1->z ) / 2.0f
			);

		ns_vector3f_norm( &V12 );
		ns_vector3f_norm( &V23 );
		ns_vector3f_norm( &V31 );

		i12 = _ns_rayburst_generate_3d_add_vector(
					vectors,
					V12.x,
					V12.y,
					V12.z,
					num_vectors,
					max_vectors
					);

		i23 = _ns_rayburst_generate_3d_add_vector(
					vectors,
					V23.x,
					V23.y,
					V23.z,
					num_vectors,
					max_vectors
					);

		i31 = _ns_rayburst_generate_3d_add_vector(
					vectors,
					V31.x,
					V31.y,
					V31.z,
					num_vectors,
					max_vectors
					);

		_ns_rayburst_generate_3d_subdivide(
			vectors, triangles,
			V1, i1,
			&V12, i12,
			&V31, i31,
			depth - 1,
			num_vectors, num_triangles,
			max_vectors, max_triangles
			);

		_ns_rayburst_generate_3d_subdivide(
			vectors, triangles,
			V2, i2,
			&V23, i23,
			&V12, i12,
			depth - 1,
			num_vectors, num_triangles,
			max_vectors, max_triangles
			);

		_ns_rayburst_generate_3d_subdivide(
			vectors, triangles,
			V3, i3,
			&V31, i31,
			&V23, i23,
			depth - 1,
			num_vectors, num_triangles,
			max_vectors, max_triangles
			);

		_ns_rayburst_generate_3d_subdivide(
			vectors, triangles,
			&V12, i12,
			&V23, i23,
			&V31, i31,
			depth - 1,
			num_vectors, num_triangles,
			max_vectors, max_triangles
			);
		}
	}


void _ns_rayburst_generate_3d_rotate_vectors( NsVector3f *vectors, nssize num_samples )
	{
	NsMatrix44f  I, J, K;
	NsVector3f   T;
	nssize       i;


	/* Apply some "random" rotations to avoid any vectors with components of 0. */

	ns_matrix44f_ident( &I );

	ns_matrix44f_rotation_x( &J, ( nsfloat )NS_DEGREES_TO_RADIANS( 32.344f ) );
	ns_matrix44f_mul( &K, &I, &J );
	I = K;

	ns_matrix44f_rotation_y( &J, ( nsfloat )NS_DEGREES_TO_RADIANS( 7.919f ) );
	ns_matrix44f_mul( &K, &I, &J );
	I = K;

	ns_matrix44f_rotation_x( &J, ( nsfloat )NS_DEGREES_TO_RADIANS( 154.20f ) );
	ns_matrix44f_mul( &K, &I, &J );
	I = K;

	ns_matrix44f_rotation_z( &J, ( nsfloat )NS_DEGREES_TO_RADIANS( 289.09293f ) );
	ns_matrix44f_mul( &K, &I, &J );
	I = K;

	for( i = 0; i < num_samples; ++i )
		{
		ns_matrix44f_mul_vector3f( &T, &I, vectors + i );
		vectors[i] = T;
		}
	}


void _ns_rayburst_generate_3d_create_planes
	(
	const NsVector3f  *vectors,
	NsRayburstPlane   *planes,
	nssize             num_samples
	)
	{
	nssize i;

	for( i = 0; i < num_samples; ++i )
		{
		/* ns_assert( NS_FLOAT_EQUAL( ns_vector3f_mag( vectors + i ), 1.0f ) ); */

		planes[i].d_delta.x = vectors[i].x < 0.0f ? 1.0f : -1.0f;
		planes[i].d_delta.y = vectors[i].y < 0.0f ? 1.0f : -1.0f;
		planes[i].d_delta.z = vectors[i].z < 0.0f ? 1.0f : -1.0f;
		planes[i].d.x       = vectors[i].x < 0.0f ? 0.0f :  1.0f;
		planes[i].d.y       = vectors[i].y < 0.0f ? 0.0f :  1.0f;
		planes[i].d.z       = vectors[i].z < 0.0f ? 0.0f :  1.0f;
		}
	}


NsIndexTriangleus ____icosahedron_triangles[20] =
	{ 
	{ 1, 4, 0 },
	{ 4, 9, 0 },
	{ 4, 5, 9 },
	{ 8, 5, 4 },
	{ 1, 8, 4 },
	{ 1, 10, 8 },
	{ 10, 3, 8 },
	{ 8, 3, 5 },
	{ 3, 2, 5 },
	{ 3, 7, 2 },
	{ 3, 10, 7 },
	{ 10, 6, 7 },
	{ 6, 11, 7 },
	{ 6, 0, 11 },
	{ 6, 1, 0 },
	{ 10, 1, 6 },
	{ 11, 0, 9 },
	{ 2, 11, 9 },
	{ 5, 2, 9 },
	{ 11, 2, 7 }
	};

#define __IX  .525731112119133606f
#define __IZ  .850650808352039932f

NsVector3f ____icosahedron_vectors[12] =
	{ 
	{ -__IX, 0.0f, __IZ },
	{ __IX, 0.0f, __IZ },
	{ -__IX, 0.0f, -__IZ },
	{ __IX, 0.0f, -__IZ },
	{ 0.0f, __IZ, __IX },
	{ 0.0f, __IZ, -__IX },
	{ 0.0f, -__IZ, __IX },
	{ 0.0f, -__IZ, -__IX },
	{ __IZ, __IX, 0.0f },
	{ -__IZ, __IX, 0.0f },
	{ __IZ, -__IX, 0.0f },
	{ -__IZ, -__IX, 0.0f }
	};


NsIndexTriangleus ____octahedron_triangles[8] =
	{ 
	{ 0, 1, 4 },
	{ 1, 2, 4 },
	{ 2, 3, 4 },
	{ 3, 0, 4 },
	{ 1, 0, 5 },
	{ 2, 1, 5 },
	{ 3, 2, 5 },
	{ 0, 3, 5 },
	};

NsVector3f ____octahedron_vectors[6] =
	{ 
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ -1.0f, 0.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, -1.0f }
	};


/* NOTE: The num_samples/num_triangles values were pre-calculated. */


nssize ns_rayburst_generate_3d_num_samples( nssize recursive_depth, nsint init_type )
	{
	NS_PRIVATE nssize ____icosahedron_num_samples[ NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH + 1 ] =
		{ 12, 42, 162, 642, 2562, 10242 };

	NS_PRIVATE nssize ____octahedron_num_samples[ NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH + 1 ] =
		{ 6, 18, 66, 258, 1026, 4098 };

	nssize n = 0;

	ns_assert( recursive_depth <= NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH );

	switch( init_type )
		{
		case NS_RAYBURST_GENERATE_3D_ICOSAHEDRON:
			n = ____icosahedron_num_samples[ recursive_depth ];
			break;

		case NS_RAYBURST_GENERATE_3D_OCTAHEDRON:
			n = ____octahedron_num_samples[ recursive_depth ];
			break;
		}

	return n;
	}


nssize ns_rayburst_generate_3d_num_triangles( nssize recursive_depth, nsint init_type )
	{
	NS_PRIVATE nssize ____icosahedron_num_triangles[ NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH + 1 ] =
		{ 20, 80, 320, 1280, 5120, 20480 };

	NS_PRIVATE nssize ____octahedron_num_triangles[ NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH + 1 ] =
		{ 8, 32, 128, 512, 2048, 8192 };

	nssize n = 0;

	ns_assert( recursive_depth <= NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH );

	switch( init_type )
		{
		case NS_RAYBURST_GENERATE_3D_ICOSAHEDRON:
			n = ____icosahedron_num_triangles[ recursive_depth ];
			break;

		case NS_RAYBURST_GENERATE_3D_OCTAHEDRON:
			n = ____octahedron_num_triangles[ recursive_depth ];
			break;
		}

	return n;
	}


#ifdef NS_DEBUG

void ns_rayburst_generate_3d_output
	(
	const NsVector3f         *vectors,
	const NsRayburstPlane    *planes,
	const NsIndexTriangleus  *triangles,
	nssize                    num_samples,
	nssize                    num_triangles
	)
	{
	nschar     name[ 64 ];
	nspointer  stream;
	nssize     i;
	NsError    error;


	ns_snprint(
		name,
		NS_ARRAY_LENGTH( name ),
		"nsrayburst-gen-3d-" NS_FMT_ULONG "-" NS_FMT_ULONG ".txt",
		num_samples,
		num_triangles
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

	for( i = 0; i < num_triangles; ++i )
		ns_println( "\t{ %4u, %4u, %4u },", triangles[i].a, triangles[i].b, triangles[i].c );

	ns_print_set_io( NULL );

	/*error*/_ns_fclose( &stream );
	}

#endif


void ns_rayburst_generate_3d
	(
	NsVector3f         *vectors,
	NsRayburstPlane    *planes,
	NsIndexTriangleus  *triangles,
	nssize              num_samples,
	nssize              num_triangles,
	nssize              recursive_depth,
	nsint               init_type,
	nsboolean           rotate
	)
	{
	nssize              i;
	NsVector3f         *init_vectors;
	nssize              num_init_vectors;
	NsIndexTriangleus  *init_triangles;
	nssize              num_init_triangles;
	nssize              track_num_vectors;
	nssize              track_num_triangles;


	ns_assert( NULL != vectors );
	ns_assert( NULL != planes );
	ns_assert( NULL != triangles );	
	ns_assert( num_samples == ns_rayburst_generate_3d_num_samples( recursive_depth, init_type ) );
	ns_assert( num_triangles == ns_rayburst_generate_3d_num_triangles( recursive_depth, init_type ) );
	ns_assert( recursive_depth <= NS_RAYBURST_GENERATE_3D_MAX_RECURSIVE_DEPTH );

	init_vectors       = NULL;
	num_init_vectors   = 0;
	init_triangles     = NULL;
	num_init_triangles = 0;

	switch( init_type )
		{
		case NS_RAYBURST_GENERATE_3D_ICOSAHEDRON:
			init_vectors       = ____icosahedron_vectors;
			num_init_vectors   = NS_ARRAY_LENGTH( ____icosahedron_vectors );
			init_triangles     = ____icosahedron_triangles;
			num_init_triangles = NS_ARRAY_LENGTH( ____icosahedron_triangles );
			break;

		case NS_RAYBURST_GENERATE_3D_OCTAHEDRON:
			init_vectors       = ____octahedron_vectors;
			num_init_vectors   = NS_ARRAY_LENGTH( ____octahedron_vectors );
			init_triangles     = ____octahedron_triangles;
			num_init_triangles = NS_ARRAY_LENGTH( ____octahedron_triangles );
			break;
		}

	track_num_vectors   = 0;
	track_num_triangles = 0;

	for( i = 0; i < num_init_vectors; ++i )
		_ns_rayburst_generate_3d_add_vector(
			vectors,
			init_vectors[i].x,
			init_vectors[i].y,
			init_vectors[i].z,
			&track_num_vectors,
			num_samples
			);

	for( i = 0; i < num_init_triangles; ++i )
		_ns_rayburst_generate_3d_subdivide(
			vectors,
			triangles,
			vectors + init_triangles[i].a,
			( nssize )init_triangles[i].a,
			vectors + init_triangles[i].b,
			( nssize )init_triangles[i].b,
			vectors + init_triangles[i].c,
			( nssize )init_triangles[i].c,
			recursive_depth,
			&track_num_vectors,
			&track_num_triangles,
			num_samples,
			num_triangles
			);

	ns_assert( track_num_vectors == num_samples );
	ns_assert( track_num_triangles == num_triangles );

	if( rotate )
		_ns_rayburst_generate_3d_rotate_vectors( vectors, num_samples );

	_ns_rayburst_generate_3d_create_planes( vectors, planes, num_samples );
	}
