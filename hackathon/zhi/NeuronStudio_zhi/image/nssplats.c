#include "nssplats.h"
#include <ext/splatsort.inl>


typedef SPLATDEF NsSplatsVoxel;


NS_PRIVATE void _ns_splats_init( NsSplats *splats )
   {
   splats->voxels_xy  = NULL;
   splats->voxels_xz  = NULL;
   splats->voxels_zy  = NULL;
   splats->size       = 0;
   splats->length     = 0;
   splats->height     = 0;
   splats->width      = 0;
   splats->threshold  = 0;
	splats->color_func = NULL;

	ns_splats_unset_at_slice( splats );
   }


void ns_splats_construct( NsSplats *splats )
   {
	NsColor4ub  C;
	nssize      i;


   ns_assert( NULL != splats );

   splats->voxel_corner_offsets = ns_voxel_corner_offsets();
   _ns_splats_init( splats );

	for( i = 0; i < NS_COLOR4UB_TABLE_SIZE; ++i )
		{
		C = ns_color4ub_table( i );

		splats->colors[i].x = NS_COLOR_GET_RED_F( C );
      splats->colors[i].y = NS_COLOR_GET_GREEN_F( C );
      splats->colors[i].z = NS_COLOR_GET_BLUE_F( C );
		}
   }


void ns_splats_destruct( NsSplats *splats )
   {
   ns_assert( NULL != splats );
   ns_splats_clear( splats );
   }


void ns_splats_clear( NsSplats *splats )
   {
   ns_assert( NULL != splats );

   ns_free( splats->voxels_xy );
   ns_free( splats->voxels_xz );
   ns_free( splats->voxels_zy );

   _ns_splats_init( splats );
   }


typedef nsboolean ( *NsSplatsValidFunc )( NsSplats*, const NsVector3i*, nsulong, nsshort*, const NsVoxelTable* );


NS_PRIVATE nsboolean _ns_splats_valid_by_image
   (
   NsSplats            *splats,
   const NsVector3i    *V,
   nsulong              intensity,
	nsshort             *color,
	const NsVoxelTable  *table
   )
   {
   NsVector3i  N;
   nssize      i;


	NS_USE_VARIABLE( table );

   if( splats->threshold <= intensity )
      {
      if( splats->is_volume ||
			 ( NULL != splats->color_func && ( splats->color_func )( splats->user_data, V, color ) )
			)
         return NS_TRUE;

      for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
         {
         ns_vector3i_add( &N, V, splats->voxel_corner_offsets + i );

         if( 0 <= N.x && N.x < splats->width  &&
             0 <= N.y && N.y < splats->height &&
             0 <= N.z && N.z < splats->length   )
            {
            intensity = ns_voxel_get(
                           &splats->voxels,
                           N.x,
                           N.y,
                           N.z
                           );

            if( intensity < splats->threshold )
               return NS_TRUE;
            }
         else
            return NS_TRUE;
         }
      }

   return NS_FALSE;
   }


NS_PRIVATE nsboolean _ns_splats_valid_by_table
   (
   NsSplats            *splats,
   const NsVector3i    *V,
   nsulong              intensity,
	nsshort             *color,
	const NsVoxelTable  *table
   )
   {
   NsVector3i  N;
   nssize      i;


	NS_USE_VARIABLE( intensity );

   if( splats->is_volume ||
		 ( NULL != splats->color_func && ( splats->color_func )( splats->user_data, V, color ) )
		)
      return NS_TRUE;

   for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
      {
      ns_vector3i_add( &N, V, splats->voxel_corner_offsets + i );

      if( 0 <= N.x && N.x < splats->width  &&
          0 <= N.y && N.y < splats->height &&
          0 <= N.z && N.z < splats->length   )
         {
			if( ! ns_voxel_table_exists( table, &N ) )
				return NS_TRUE;
         }
      else
         return NS_TRUE;
      }

   return NS_FALSE;
   }


NS_PRIVATE void _ns_splats_get_corners
   (
   NsSplats          *splats,
   const NsVector3i  *V,
   nsfloat           *corners
   )
   {
   NsVector3i  N;
   nssize      i;


   for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
      {
      ns_vector3i_add( &N, V, splats->voxel_corner_offsets + i );

      if( 0 <= N.x && N.x < splats->width  &&
          0 <= N.y && N.y < splats->height &&
          0 <= N.z && N.z < splats->length   )
         corners[i] = ( nsfloat )ns_voxel_get( &splats->voxels, N.x, N.y, N.z );
      else
         corners[i] = 0.0f;
      }
   }


struct _NsSplatsCreateStruct;
typedef struct _NsSplatsCreateStruct NsSplatsCreateStruct;

typedef nsulong ( *NsSplatsCreateFunc )( const NsVector3i*, NsSplatsCreateStruct* );


struct _NsSplatsCreateStruct
   {
   NsSplats            *splats;
   NsSplatsCreateFunc   create_func;
	NsSplatsValidFunc    valid_func;
   nsulong              threshold;
   NsVoxelBuffer       *voxels;
   NsVoxelBuffer       *proj_xy;
   NsVoxelBuffer       *proj_zy;
   NsVoxelBuffer       *proj_xz;
   nspointer            sorter;
	const NsVoxelTable  *table;
   };


NS_PRIVATE void _ns_splats_create_struct_init
   (
   NsSplatsCreateStruct  *s,
   NsSplats              *splats,
   nspointer              sorter,
	const NsVoxelTable    *table,
	NsSplatsValidFunc      valid_func
   )
   {
   s->splats      = splats;
   s->create_func = ( NsSplatsCreateFunc )splats->create_func;
   s->threshold   = splats->threshold;
   s->voxels      = &splats->voxels;
   s->proj_xy     = &splats->proj_xy;
   s->proj_zy     = &splats->proj_zy;
   s->proj_xz     = &splats->proj_xz;
   s->sorter      = sorter;
	s->table       = table;
	s->valid_func  = valid_func;
   }


#define _NS_SPLATS_GET_PROJ_XY( V, s )\
   ns_voxel_get( (s)->proj_xy, (V)->x, (V)->y, 0 )


#define _NS_SPLATS_GET_PROJ_ZY( V, s )\
   ns_voxel_get( (s)->proj_zy, (V)->z, (V)->y, 0 )


/* IMPORTANT: Assuming the XZ projection has been flipped in
   the z direction, i.e. vertically. */
#define _NS_SPLATS_GET_PROJ_XZ( V, s )\
   ns_voxel_get(\
      (s)->proj_xz,\
      (V)->x,\
      ( ns_voxel_buffer_height( (s)->proj_xz ) - 1 ) - (V)->z,\
      0\
      )


NS_PRIVATE nsulong _ns_splats_create_proj_average( const NsVector3i *V, NsSplatsCreateStruct *s )
   {
   nsulong intensity_xy, intensity_zy, intensity_xz;

   intensity_xy = _NS_SPLATS_GET_PROJ_XY( V, s );
   intensity_zy = _NS_SPLATS_GET_PROJ_ZY( V, s );
   intensity_xz = _NS_SPLATS_GET_PROJ_XZ( V, s );

   return ( intensity_xy + intensity_zy + intensity_xz ) / 3;
   }


NS_PRIVATE nsulong _ns_splats_create_proj_max( const NsVector3i *V, NsSplatsCreateStruct *s )
   {
   nsulong intensity_xy, intensity_zy, intensity_xz;

   intensity_xy = _NS_SPLATS_GET_PROJ_XY( V, s );
   intensity_zy = _NS_SPLATS_GET_PROJ_ZY( V, s );
   intensity_xz = _NS_SPLATS_GET_PROJ_XZ( V, s );

   return NS_MAX3( intensity_xy, intensity_zy, intensity_xz );
   }


NS_PRIVATE nsulong _ns_splats_create_proj_min( const NsVector3i *V, NsSplatsCreateStruct *s )
   {
   nsulong intensity_xy, intensity_zy, intensity_xz;

   intensity_xy = _NS_SPLATS_GET_PROJ_XY( V, s );
   intensity_zy = _NS_SPLATS_GET_PROJ_ZY( V, s );
   intensity_xz = _NS_SPLATS_GET_PROJ_XZ( V, s );

   return NS_MIN3( intensity_xy, intensity_zy, intensity_xz );
   }


NS_PRIVATE NsSplatsCreateFunc _ns_splats_create_funcs[ _NS_SPLATS_CREATE_NUM_TYPES ] =
   {
   NULL,
   _ns_splats_create_proj_average,
   _ns_splats_create_proj_max,
   _ns_splats_create_proj_min
   };


NS_PRIVATE void _ns_splats_create_function
   (
   const NsVector3i      *V,
   nsulong                intensity,
   NsSplatsCreateStruct  *s
   )
   {
   NsSplatsVoxel  voxel;
   nsfloat        corners[ NS_VOXEL_NUM_CORNER_OFFSETS ];
   nsfloat        normal[3];
	nsshort        color;


	color = -1;

   if( ( s->valid_func )( s->splats, V, intensity, &color, s->table ) )
      {
      voxel.V.x = V->x;
      voxel.V.y = V->y;
      voxel.V.z = V->z;

		voxel.color = color;
         
      voxel.intensity = ( nsuint8 )(
                           ( NULL != s->create_func ) ?
                           ( s->create_func )( V, s ) :
                           ns_voxel_buffer_convert_to_lum_u8( s->voxels, intensity )
                           );

      _ns_splats_get_corners( s->splats, V, corners );
      ComputeImageNormal( corners, normal );

      /* Reverse normal y and z since the data axis orientation requires
         a rotation to align it with OpenGL's axis orientation. */
      voxel.N.x = NS_NORMAL_FLOAT_TO_SIGNED_BYTE(  normal[0] );
      voxel.N.y = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[1] );
      voxel.N.z = NS_NORMAL_FLOAT_TO_SIGNED_BYTE( -normal[2] );

      /* NOTE: Ignore error. */
      SplatSorterInsert( s->sorter, &voxel );
      }
   }


NS_PRIVATE NsError _ns_splats_pre_create
	(
	NsSplats           *splats,
	nspointer          *sorter,
	NsProgress         *progress,
	NsSplatsColorFunc   color_func,
	nspointer           user_data
	)
	{
   if( NULL != progress )
		{
		nschar title[128];

		ns_snprint(
			title,
			sizeof( title ),
			"Creating Volume Rendering at a threshold of " NS_FMT_ULONG "...",
			splats->threshold
			);

      ns_progress_set_title( progress, title );
		}

   if( NULL == ( *sorter = SplatSorterCreate( splats->width, splats->height, splats->length ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	splats->color_func = color_func;
	splats->user_data  = user_data;

	return ns_no_error();
	}


NS_PRIVATE NsError _ns_splats_post_create
	(
	NsSplats    *splats,
	nspointer    sorter,
	NsProgress  *progress
	)
	{
   if( NULL == progress || ! ns_progress_cancelled( progress ) )
      {
      if( ! SplatSorterBuild(
               sorter,
               &( ( NsSplatsVoxel* )splats->voxels_zy ),
               &( ( NsSplatsVoxel* )splats->voxels_xz ),
               &( ( NsSplatsVoxel* )splats->voxels_xy )
               ) )
         {
         SplatSorterDestroy( sorter );
         return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
         }

      splats->size = SplatSorterSize( sorter );
      }

   SplatSorterDestroy( sorter );
   return ns_no_error();
	}


NS_PRIVATE NsError _ns_splats_create_by_image
	(
	NsSplats             *splats,
	const NsVoxelBuffer  *voxels,
	NsProgress           *progress,
	NsSplatsColorFunc     color_func,
	nspointer             user_data
	)
   {
   NsSplatsCreateStruct  s;
   nspointer             sorter;
	NsError               error;


	if( NS_FAILURE( _ns_splats_pre_create( splats, &sorter, progress, color_func, user_data ), error ) )
		return error;

   _ns_splats_create_struct_init( &s, splats, sorter, NULL, _ns_splats_valid_by_image );
   ns_voxel_buffer_forall_ex( voxels, NS_VOXEL_BUFFER_FORALL_XY, &splats->roi, _ns_splats_create_function, &s, progress );

	return _ns_splats_post_create( splats, sorter, progress );
   }


NS_PRIVATE NsError _ns_splats_create_by_table
	(
	NsSplats             *splats,
	const NsVoxelTable   *table,
	NsProgress           *progress,
	NsSplatsColorFunc     color_func,
	nspointer             user_data
	)
   {
   NsSplatsCreateStruct   s;
   nspointer              sorter;
	const NsVoxel         *voxel;
	nsint32                i32;
	nsfloat64              f64;
	NsError                error;


	if( NS_FAILURE( _ns_splats_pre_create( splats, &sorter, progress, color_func, user_data ), error ) )
		return error;

   _ns_splats_create_struct_init( &s, splats, sorter, table, _ns_splats_valid_by_table );

	NS_VOXEL_TABLE_FOREACH( table, voxel )
		{
		NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( voxel->intensity, i32, f64 );
		_ns_splats_create_function( &voxel->position, ( nsulong )i32, &s );
		}

	return _ns_splats_post_create( splats, sorter, progress );
   }









void ns_splats_outline_construct( NsSplatsOutline *outline )
	{
	ns_assert( NULL != outline );
	ns_list_construct( &outline->pixels, NULL );
	}


void ns_splats_outline_destruct( NsSplatsOutline *outline )
	{
	ns_assert( NULL != outline );
	ns_list_destruct( &outline->pixels );
	}


NS_PRIVATE nsboolean _ns_splats_outline_voxel_valid
   (
   NsSplatsOutline   *outline,
   const NsVector3i  *V,
   nsulong            intensity
   )
   {
   NsVector3i  N;
   nssize      i;


   if( outline->threshold <= intensity )
      for( i = 0; i < NS_VOXEL_NUM_CORNER_OFFSETS; ++i )
         {
         ns_vector3i_add( &N, V, outline->voxel_corner_offsets + i );

         if( 0 <= N.x && N.x < outline->width && 0 <= N.y && N.y < outline->height )
            {
            intensity = ns_voxel_get( &outline->buffer, N.x, N.y, 0 );

            if( intensity < outline->threshold )
               return NS_TRUE;
            }
         else
            return NS_TRUE;
         }

   return NS_FALSE;
   }


NS_PRIVATE void _ns_splats_outline_create_function
   (
   const NsVector3i  *V,
   nsulong            intensity,
	NsSplatsOutline   *outline
   )
   {
	nsulong pixel;

	ns_assert( 0 == V->z );

   if( _ns_splats_outline_voxel_valid( outline, V, intensity ) )
      {
		pixel = ( nsulong )V->y * ( nsulong )outline->width + ( nsulong )V->x;
		ns_list_push_back( &outline->pixels, NS_ULONG_TO_POINTER( pixel ) );
      }
   }


NS_PRIVATE NsError _ns_splats_outline_create
	(
	NsSplatsOutline     *outline,
	const NsImage       *proj,
	nsdouble             threshold,
	nsulong              contrast
	)
	{
	ns_list_clear( &outline->pixels );

	ns_assert( 1 == ns_image_length( proj ) );

	outline->proj                 = proj;
   outline->voxel_corner_offsets = ns_voxel_corner_offsets();

   ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj ) );
	ns_voxel_buffer_init( &outline->buffer, proj );

	outline->width  = ns_voxel_buffer_width( &outline->buffer );
	outline->height = ns_voxel_buffer_height( &outline->buffer );

	ns_println( "splats-outline: passed threshold = " NS_FMT_DOUBLE, threshold );

	outline->threshold  = ns_voxel_buffer_convert_to_lum_u8( &outline->buffer, ( nsulong )threshold );
	outline->threshold += contrast;
	outline->threshold  = ns_voxel_buffer_convert_from_lum_u8( &outline->buffer, outline->threshold );

	ns_println( "splats-outline: actual threshold = " NS_FMT_ULONG, outline->threshold );

   ns_voxel_buffer_forall( &outline->buffer, NS_VOXEL_BUFFER_FORALL_XY, _ns_splats_outline_create_function, outline, NULL );
	}







NS_PRIVATE NsError _ns_splats_do_create
   (
	NsSplats            *splats,
	const NsImage       *image,
	const NsCubei       *roi,
	const NsVoxelTable  *table,
	const NsImage       *proj_xy,
	const NsImage       *proj_zy,
	const NsImage       *proj_xz,
	nsdouble             threshold,
	nsint                which,
	nsboolean            is_volume,
	nsulong              contrast,
	NsProgress          *progress,
	NsSplatsColorFunc    color_func,
	nspointer            user_data,
	NsSplatsOutline     *outline
   )
   {
   NsError error;

   ns_assert( NULL != splats );
	ns_assert( NULL != image );
   ns_assert( NULL != proj_xy );
   ns_assert( NULL != proj_zy );
   ns_assert( NULL != proj_xz );

	NS_USE_VARIABLE( outline );

   ns_splats_clear( splats );

	if( NULL != roi )
		splats->roi = *roi;

   ns_assert( which < _NS_SPLATS_CREATE_NUM_TYPES );
   splats->create_func = ( NsFunc )_ns_splats_create_funcs[ which ];

   splats->is_volume = is_volume;

	ns_voxel_buffer_init( &splats->voxels, image );

	splats->width  = ns_voxel_buffer_width( &splats->voxels );
	splats->height = ns_voxel_buffer_height( &splats->voxels );
	splats->length = ns_voxel_buffer_length( &splats->voxels );

	if( NULL == table )
		{
ns_println( "splats: passed threshold (N-bit) = " NS_FMT_DOUBLE, threshold );
ns_println( "splats: passed contrast (8-bit) = " NS_FMT_ULONG, contrast );

		splats->threshold = ns_voxel_buffer_convert_to_lum_u8( &splats->voxels, ( nsulong )threshold );

ns_println( "splats: passed threshold (8-bit) = " NS_FMT_ULONG, splats->threshold );

		splats->threshold += contrast;

		if( splats->threshold > ( nsulong )NS_UINT8_MAX )
			splats->threshold = ( nsulong )NS_UINT8_MAX;

ns_println( "splats: actual threshold (8-bit) = " NS_FMT_ULONG, splats->threshold );

		splats->threshold = ns_voxel_buffer_convert_from_lum_u8( &splats->voxels, splats->threshold );

ns_println( "splats: actual threshold (N-bit) = " NS_FMT_ULONG, splats->threshold );
		}

   ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_xy ) );
   ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_zy ) );
   ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( proj_xz ) );

   ns_voxel_buffer_init( &splats->proj_xy, proj_xy );
   ns_voxel_buffer_init( &splats->proj_zy, proj_zy );
   ns_voxel_buffer_init( &splats->proj_xz, proj_xz );

   ns_assert( 1 <= ns_voxel_buffer_height( &splats->proj_xz ) );

	if( NULL == table )
		{
		if( NS_FAILURE(
				_ns_splats_create_by_image(
					splats,
					&splats->voxels,
					progress,
					color_func,
					user_data
					),
				error ) )
			goto _NS_SPLATS_CREATE_CANCELLED;
		}
	else
		{
		if( NS_FAILURE(
				_ns_splats_create_by_table(
					splats,
					table,
					progress,
					color_func,
					user_data
					),
				error ) )
			goto _NS_SPLATS_CREATE_CANCELLED;		
		}

   if( NULL != progress && ns_progress_cancelled( progress ) )
      goto _NS_SPLATS_CREATE_CANCELLED;

   goto _NS_SPLATS_CREATE_NOT_CANCELLED;

   _NS_SPLATS_CREATE_CANCELLED:
   ns_splats_clear( splats );

   _NS_SPLATS_CREATE_NOT_CANCELLED:
   return ns_no_error();
   }


NsError ns_splats_create
   (
	NsSplats         *splats,
	const NsImage    *image,
	const NsCubei    *roi,
	const NsImage    *proj_xy,
	const NsImage    *proj_zy,
	const NsImage    *proj_xz,
	nsdouble          threshold,
	nsint             which,
	nsboolean         is_volume,
	nsulong           contrast,
	NsProgress       *progress,
	NsSplatsOutline  *outline
   )
   {
	return _ns_splats_do_create(
					splats,
					image,
					roi,
					NULL,
					proj_xy,
					proj_zy,
					proj_xz,
					threshold,
					which,
					is_volume,
					contrast,
					progress,
					NULL,
					NULL,
					outline
					);
   }


NsError ns_splats_create_ex
	(
	NsSplats           *splats,
	const NsImage      *image,
	const NsCubei      *roi,
	const NsImage      *proj_xy,
	const NsImage      *proj_zy,
	const NsImage      *proj_xz,
	nsdouble            threshold,
	nsint               which,
	nsboolean           is_volume,
	nsulong             contrast,
	NsProgress         *progress,
	NsSplatsColorFunc   color_func,
	nspointer           user_data,
	NsSplatsOutline    *outline
	)
	{
	return _ns_splats_do_create(
					splats,
					image,
					roi,
					NULL,
					proj_xy,
					proj_zy,
					proj_xz,
					threshold,
					which,
					is_volume,
					contrast,
					progress,
					color_func,
					user_data,
					outline
					);
	}


NsError ns_splats_create_by_table
	(
	NsSplats            *splats,
	const NsImage       *image,
	const NsCubei       *roi,
	const NsVoxelTable  *table,
	const NsImage       *proj_xy,
	const NsImage       *proj_zy,
	const NsImage       *proj_xz,
	nsint                which,
	nsboolean            is_volume,
	NsProgress          *progress,
	NsSplatsOutline     *outline
	)
	{
	return _ns_splats_do_create(
					splats,
					image,
					roi,
					table,
					proj_xy,
					proj_zy,
					proj_xz,
					0.0,
					which,
					is_volume,
					0,
					progress,
					NULL,
					NULL,
					outline
					);
	}


#include <ns-gl.h>
#include <ext/splat.h>


typedef struct _NsSplatsRenderer
   {
   const NsVoxelInfo  *voxel_info;
   nsulong             flags;
   const NsVector4ub  *palette;
   const NsVector3f   *color;
   nsfloat             alpha;
	nsboolean           clip_to_slice;
   }
   NsSplatsRenderer;


typedef void ( *_NsSplatsRenderFunc )
   (
   const NsSplats*,
   const NsSplatsRenderer*,
   const NsSplatsVoxel*,
   const NsSplatsVoxel*,
   nslong
   );


NS_PRIVATE void _ns_splats_do_render_textures
   (
   const NsSplats          *splats,
   const NsSplatsRenderer  *renderer,
   const NsSplatsVoxel     *curr_voxel,
   const NsSplatsVoxel     *end_voxels,
   nslong                   increment
   )
   {
   const NsVector4ub  *palette;
   nsfloat             voxel_color[4];
   nsfloat             V[3];
   nsulong             flags;
   nsfloat             voxel_size_x;
   nsfloat             voxel_size_y;
   nsfloat             voxel_size_z;
   nsfloat             color_x;
   nsfloat             color_y;
   nsfloat             color_z;
   nsfloat             alpha;
   NsVector4ub         color;
	NsVector3f          C;
	nsboolean           clip_to_slice;
	nsint               slice;
	nsint               z;
	nsuint8             intensity;


   flags = renderer->flags;

   palette = renderer->palette;

   voxel_size_x = ns_voxel_info_size_x( renderer->voxel_info );
   voxel_size_y = ns_voxel_info_size_y( renderer->voxel_info );
   voxel_size_z = ns_voxel_info_size_z( renderer->voxel_info );

   color_x = renderer->color->x;
   color_y = renderer->color->y;
   color_z = renderer->color->z;

   alpha = renderer->alpha;

	clip_to_slice = renderer->clip_to_slice;

	slice = splats->slice;

   /* NOTE: Tried to optimize this loop as much as possible. */
   for( ; curr_voxel != end_voxels; curr_voxel += increment )
      {
		z = curr_voxel->V.z;

		if( clip_to_slice && z < slice )
			continue;

		intensity = curr_voxel->intensity;

      V[0] = NS_TO_VOXEL_SPACE( curr_voxel->V.x, voxel_size_x );
      V[1] = NS_TO_VOXEL_SPACE( curr_voxel->V.y, voxel_size_y );
      V[2] = NS_TO_VOXEL_SPACE( z, voxel_size_z );

      glNormal3bv( ( const nschar* )( &curr_voxel->N ) );

      if( ( flags & NS_SPLATS_RENDER_PALETTE ) )
         {
         color = palette[ intensity ];

         voxel_color[0] = NS_COLOR_GET_BLUE_F( color );
         voxel_color[1] = NS_COLOR_GET_GREEN_F( color );
         voxel_color[2] = NS_COLOR_GET_RED_F( color );
         }
		else if( 0 <= curr_voxel->color ) 
			{
			C = splats->colors[ curr_voxel->color ];

			voxel_color[0] = C.x;
			voxel_color[1] = C.y;
			voxel_color[2] = C.z;
			}
      else
         {
         voxel_color[0] = color_x;
         voxel_color[1] = color_y;
         voxel_color[2] = color_z;
         }

      voxel_color[3] = ( flags & NS_SPLATS_RENDER_ALPHA ) ?
                       alpha : ( nsfloat )intensity * ( 1.0f / 255.0f );

		if( z == splats->slice )
			{
         voxel_color[0] = 1.0f - voxel_color[0];
         voxel_color[1] = 1.0f - voxel_color[1];
         voxel_color[2] = 1.0f - voxel_color[2];
			voxel_color[3] = 1.0f;
			}

		SplatRender( V, voxel_color );
      }
   }


NS_PRIVATE GLuint _ns_splats_cube_faces[ 24 ] =
   {
   0, 2, 3, 1,
   1, 3, 7, 5,
   5, 7, 6, 4,
   4, 6, 2, 0,
   4, 0, 1, 5,
   2, 6, 7, 3
   };


NS_PRIVATE void _ns_splats_do_render_cubes
   (
   const NsSplats          *splats,
   const NsSplatsRenderer  *renderer,
   const NsSplatsVoxel     *curr_voxel,
   const NsSplatsVoxel     *end_voxels,
   nslong                   increment
   )
   {
   const NsVector4ub  *palette;
   nsfloat             voxel_color[4];
   NsVector3f          V1, V2;
   nsulong             flags;
   nsfloat             voxel_size_x;
   nsfloat             voxel_size_y;
   nsfloat             voxel_size_z;
   nsfloat             color_x;
   nsfloat             color_y;
   nsfloat             color_z;
   nsfloat             alpha;
   NsVector4ub         color;
	NsVector3f          C;
	GLfloat             V[ 8 * 3 ];
	nsboolean           clip_to_slice;
	nsint               slice;
	nsint               z;
	nsuint8             intensity;


   flags = renderer->flags;

   palette = renderer->palette;

   voxel_size_x = ns_voxel_info_size_x( renderer->voxel_info );
   voxel_size_y = ns_voxel_info_size_y( renderer->voxel_info );
   voxel_size_z = ns_voxel_info_size_z( renderer->voxel_info );

   color_x = renderer->color->x;
   color_y = renderer->color->y;
   color_z = renderer->color->z;

   alpha = renderer->alpha;

	clip_to_slice = renderer->clip_to_slice;

	slice = splats->slice;

	glEnableClientState( GL_VERTEX_ARRAY );
   glVertexPointer( 3, GL_FLOAT, 0, V );

   for( ; curr_voxel != end_voxels; curr_voxel += increment )
      {
		z = curr_voxel->V.z;

		if( clip_to_slice && z < slice )
			continue;

		intensity = curr_voxel->intensity;

      V1.x = NS_TO_VOXEL_SPACE( curr_voxel->V.x, voxel_size_x );
      V1.y = NS_TO_VOXEL_SPACE( curr_voxel->V.y, voxel_size_y );
      V1.z = NS_TO_VOXEL_SPACE( z, voxel_size_z );

		V2.x = V1.x + voxel_size_x;
		V2.y = V1.y + voxel_size_y;
		V2.z = V1.z + voxel_size_z;

      if( ( flags & NS_SPLATS_RENDER_PALETTE ) )
         {
         color = palette[ intensity ];

         voxel_color[0] = NS_COLOR_GET_RED_F( color );
         voxel_color[1] = NS_COLOR_GET_GREEN_F( color );
         voxel_color[2] = NS_COLOR_GET_BLUE_F( color );
         }
		else if( 0 <= curr_voxel->color ) 
			{
			C = splats->colors[ curr_voxel->color ];

			voxel_color[0] = C.x;
			voxel_color[1] = C.y;
			voxel_color[2] = C.z;
			}
      else
         {
         voxel_color[0] = color_x;
         voxel_color[1] = color_y;
         voxel_color[2] = color_z;
         }

      voxel_color[3] = ( flags & NS_SPLATS_RENDER_ALPHA ) ?
                       alpha : ( nsfloat )intensity * ( 1.0f / 255.0f );

		if( z == splats->slice )
			{
         voxel_color[0] = 1.0f - voxel_color[0];
         voxel_color[1] = 1.0f - voxel_color[1];
         voxel_color[2] = 1.0f - voxel_color[2];
			voxel_color[3] = 1.0f;
			}

		V[0*3+0]=V1.x; V[0*3+1]=V1.y; V[0*3+2]=V1.z;
		V[1*3+0]=V2.x; V[1*3+1]=V1.y; V[1*3+2]=V1.z;
		V[2*3+0]=V1.x; V[2*3+1]=V2.y; V[2*3+2]=V1.z;
		V[3*3+0]=V2.x; V[3*3+1]=V2.y; V[3*3+2]=V1.z;
		V[4*3+0]=V1.x; V[4*3+1]=V1.y; V[4*3+2]=V2.z;
		V[5*3+0]=V2.x; V[5*3+1]=V1.y; V[5*3+2]=V2.z;
		V[6*3+0]=V1.x; V[6*3+1]=V2.y; V[6*3+2]=V2.z;
		V[7*3+0]=V2.x; V[7*3+1]=V2.y; V[7*3+2]=V2.z;

		glColor4f( voxel_color[0], voxel_color[1], voxel_color[2], voxel_color[3] );

		glDrawElements(
			GL_QUADS,
			NS_ARRAY_LENGTH( _ns_splats_cube_faces ),
			GL_UNSIGNED_INT,
			_ns_splats_cube_faces
			);
      }

	glDisableClientState( GL_VERTEX_ARRAY );
   }


NS_PRIVATE void _ns_splats_do_render_points
   (
   const NsSplats          *splats,
   const NsSplatsRenderer  *renderer,
   const NsSplatsVoxel     *curr_voxel,
   const NsSplatsVoxel     *end_voxels,
   nslong                   increment
   )
   {
   const NsVector4ub  *palette;
   nsfloat             voxel_color[4];
   nsfloat             V[3];
   nsulong             flags;
   nsfloat             voxel_size_x;
   nsfloat             voxel_size_y;
   nsfloat             voxel_size_z;
   nsfloat             color_x;
   nsfloat             color_y;
   nsfloat             color_z;
   nsfloat             alpha;
   NsVector4ub         color;
	NsVector3f          C;
	nsboolean           clip_to_slice;
	nsint               slice;
	nsint               z;
	nsuint8             intensity;


   flags = renderer->flags;

   palette = renderer->palette;

   voxel_size_x = ns_voxel_info_size_x( renderer->voxel_info );
   voxel_size_y = ns_voxel_info_size_y( renderer->voxel_info );
   voxel_size_z = ns_voxel_info_size_z( renderer->voxel_info );

   color_x = renderer->color->x;
   color_y = renderer->color->y;
   color_z = renderer->color->z;

   alpha = renderer->alpha;

	clip_to_slice = renderer->clip_to_slice;

	slice = splats->slice;

   /* NOTE: Tried to optimize this loop as much as possible. */
   for( ; curr_voxel != end_voxels; curr_voxel += increment )
      {
		z = curr_voxel->V.z;

		if( clip_to_slice && z < slice )
			continue;

		intensity = curr_voxel->intensity;

      V[0] = NS_TO_VOXEL_SPACE( curr_voxel->V.x, voxel_size_x );
      V[1] = NS_TO_VOXEL_SPACE( curr_voxel->V.y, voxel_size_y );
      V[2] = NS_TO_VOXEL_SPACE( z, voxel_size_z );

      glNormal3bv( ( const nschar* )( &curr_voxel->N ) );

      if( ( flags & NS_SPLATS_RENDER_PALETTE ) )
         {
         color = palette[ intensity ];

         voxel_color[0] = NS_COLOR_GET_BLUE_F( color );
         voxel_color[1] = NS_COLOR_GET_GREEN_F( color );
         voxel_color[2] = NS_COLOR_GET_RED_F( color );
         }
		else if( 0 <= curr_voxel->color ) 
			{
			C = splats->colors[ curr_voxel->color ];

			voxel_color[0] = C.x;
			voxel_color[1] = C.y;
			voxel_color[2] = C.z;
			}
      else
         {
         voxel_color[0] = color_x;
         voxel_color[1] = color_y;
         voxel_color[2] = color_z;
         }

      voxel_color[3] = ( flags & NS_SPLATS_RENDER_ALPHA ) ?
                       alpha : ( nsfloat )intensity * ( 1.0f / 255.0f );

		if( z == splats->slice )
			{
         voxel_color[0] = 1.0f - voxel_color[0];
         voxel_color[1] = 1.0f - voxel_color[1];
         voxel_color[2] = 1.0f - voxel_color[2];
			voxel_color[3] = 1.0f;
			}

		glColor4fv( voxel_color );
		glVertex3fv( V );
      }
   }


NS_PRIVATE void _ns_splats_render_forwards
   (
   const NsSplats          *splats,
   const NsSplatsRenderer  *renderer,
   const NsSplatsVoxel     *voxels,
	_NsSplatsRenderFunc      render_func
   )
   {  ( render_func )( splats, renderer, voxels, voxels + splats->size, 1 );  }


NS_PRIVATE void _ns_splats_render_backwards
   (
   const NsSplats          *splats,
   const NsSplatsRenderer  *renderer,
   const NsSplatsVoxel     *voxels,
	_NsSplatsRenderFunc      render_func
   )
   {  ( render_func )( splats, renderer, voxels + ( splats->size - 1 ), voxels - 1, -1 );  }



NS_PRIVATE void _ns_splats_init_lighting( void )
	{
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHTING );
	glEnable( GL_COLOR_MATERIAL );
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, 1 );
	}


NS_PRIVATE void _ns_splats_finalize_lighting( void )
	{
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, 0 );
	glDisable( GL_LIGHT0 );
	glDisable( GL_LIGHTING );
	glDisable( GL_COLOR_MATERIAL );
	}


enum { __X = 2, __Y = 6, __Z = 10 };

/* Returns true for backwards, else false for forwards.
	Set 'voxels' as which array to draw. */
NS_PRIVATE nsboolean _ns_splats_render_direction
	(
	const NsSplats        *splats,
	const NsSplatsVoxel  **voxels
	)
	{
   nsint    which;
   nsfloat  x, y, z;
   nsfloat  model_view[16];


   /* Find which basis vector of the model-view matrix
      has the largest z component. */
   glGetFloatv( GL_MODELVIEW_MATRIX, model_view );

   x = NS_ABS( model_view[ __X ] );
   y = NS_ABS( model_view[ __Y ] );
   z = NS_ABS( model_view[ __Z ] );

   which = NS_MAX3_INDEX( x, y, z );

   switch( which )
      {
      case 0:
         *voxels = splats->voxels_zy;
         which   = __X;
         break;

      case 1:
         *voxels = splats->voxels_xz;
         which   = __Y;
         break;

      case 2:
         *voxels = splats->voxels_xy;
         which   = __Z;
         break;
      }

   return ( model_view[ which ] < 0.0f );
	}


NS_PRIVATE void _ns_splats_render_textures
   (
   const NsSplats     *splats,
   const NsVoxelInfo  *voxel_info,
   nsulong             flags,
	NsSplatsRenderer   *renderer
   )
   {
   const NsSplatsVoxel *voxels;

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_init_lighting();

   SplatSetSize( ns_voxel_info_max_size( voxel_info ) * 3.0f );
   SplatBegin();

	if( _ns_splats_render_direction( splats, &voxels ) )
      _ns_splats_render_backwards( splats, renderer, voxels, _ns_splats_do_render_textures );
   else
      _ns_splats_render_forwards( splats, renderer, voxels, _ns_splats_do_render_textures );

   SplatEnd();

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_finalize_lighting();
   }


NS_PRIVATE void _ns_splats_render_cubes
   (
   const NsSplats     *splats,
   const NsVoxelInfo  *voxel_info,
   nsulong             flags,
	NsSplatsRenderer   *renderer
   )
   {
   const NsSplatsVoxel *voxels;

	NS_USE_VARIABLE( voxel_info );

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_init_lighting();

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	if( _ns_splats_render_direction( splats, &voxels ) )
      _ns_splats_render_backwards( splats, renderer, voxels, _ns_splats_do_render_cubes );
   else
      _ns_splats_render_forwards( splats, renderer, voxels, _ns_splats_do_render_cubes );

	glDisable( GL_BLEND );

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_finalize_lighting();
   }


NS_PRIVATE void _ns_splats_render_points
   (
   const NsSplats     *splats,
   const NsVoxelInfo  *voxel_info,
   nsulong             flags,
	NsSplatsRenderer   *renderer
   )
   {
   const NsSplatsVoxel *voxels;

	NS_USE_VARIABLE( voxel_info );

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_init_lighting();

	//glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBegin( GL_POINTS );

	if( _ns_splats_render_direction( splats, &voxels ) )
      _ns_splats_render_backwards( splats, renderer, voxels, _ns_splats_do_render_points );
   else
      _ns_splats_render_forwards( splats, renderer, voxels, _ns_splats_do_render_points );

	glEnd();
	//glDisable( GL_BLEND );

   if( ( flags & NS_SPLATS_RENDER_LIGHTING ) )
		_ns_splats_finalize_lighting();
   }


void ns_splats_render
	(
	const NsSplats     *splats,
	const NsVoxelInfo  *voxel_info,
	nsulong             flags,
	const NsVector4ub  *palette,
	const NsVector3f   *color,
	nsfloat             alpha,
	nsint               type,
	nsboolean           clip_to_slice
	)
	{
   NsSplatsRenderer renderer;

   ns_assert( NULL != splats );
   ns_assert( NULL != voxel_info );

   if( 0 == splats->size )
      return;

   renderer.voxel_info    = voxel_info;
   renderer.flags         = flags;
   renderer.palette       = palette;
   renderer.color         = color;
   renderer.alpha         = alpha;
	renderer.clip_to_slice = clip_to_slice;

	if( NS_SPLATS_RENDER_OPTIMIZE & flags )
		_ns_splats_render_points( splats, voxel_info, flags, &renderer );
	else
		{
		switch( type )
			{
			case NS_SPLATS_RENDER_TEXTURES:
				_ns_splats_render_textures( splats, voxel_info, flags, &renderer );
				break;

			case NS_SPLATS_RENDER_CUBES:
				_ns_splats_render_cubes( splats, voxel_info, flags, &renderer );
				break;
		
			default:
				ns_assert_not_reached();
			}
		}
	}


void ns_splats_set_at_slice( NsSplats *splats, nsint slice )
	{
	ns_assert( NULL != splats );
	splats->slice = slice;
	}


void ns_splats_unset_at_slice( NsSplats *splats )
	{
	ns_assert( NULL != splats );
	splats->slice = -1;
	}


nsboolean ns_splats_find_closest_by_ray
	(
	const NsSplats     *splats,
	const NsVoxelInfo  *voxel_info,
	NsRay3d            *R,
	NsPoint3d          *P,
	NsProgress         *progress
	)
	{
	const NsSplatsVoxel  *curr_voxel;
   const NsSplatsVoxel  *end_voxels;
   const NsSplatsVoxel  *closest;
   nsfloat               voxel_size_x;
   nsfloat               voxel_size_y;
   nsfloat               voxel_size_z;
	NsSphered             S;
   NsPoint3d             I;
   nsdouble              distance, min_distance;


	ns_assert( NULL != splats );
	ns_assert( NULL != voxel_info );
	ns_assert( NULL != R );
	ns_assert( NULL != P );

	/* Doesnt matter which voxel array we use. */

	if( NULL == splats->voxels_xy )
		return NS_FALSE;

   closest      = NULL;
   min_distance = NS_DOUBLE_MAX;
		
	curr_voxel = splats->voxels_xy;
	end_voxels = curr_voxel + splats->size;

   voxel_size_x = ns_voxel_info_size_x( voxel_info );
   voxel_size_y = ns_voxel_info_size_y( voxel_info );
   voxel_size_z = ns_voxel_info_size_z( voxel_info );

	S.r = ( ns_voxel_info_max_size( voxel_info ) * 3.0f ) / 2.0f;

	/*ns_progress_set_title( progress, "Finding closest voxel in 3D..." );*/
	ns_progress_num_iters( progress, splats->size );
	ns_progress_begin( progress );

   for( ; curr_voxel != end_voxels; ++curr_voxel )
      {
      S.O.x = NS_TO_VOXEL_SPACE( curr_voxel->V.x, voxel_size_x );
      S.O.y = NS_TO_VOXEL_SPACE( curr_voxel->V.y, voxel_size_y );
      S.O.z = NS_TO_VOXEL_SPACE( curr_voxel->V.z, voxel_size_z );

      if( ns_ray3d_intersects_sphere( R, &S, &I ) )
         {
         distance = ns_vector3d_distance( &R->O, &I );

         if( distance < min_distance )
            {
            min_distance = distance;
            closest      = curr_voxel;
            }
         }

		ns_progress_next_iter( progress );
      }

	ns_progress_end( progress );

	if( NULL != closest )
		{
      P->x = NS_TO_VOXEL_SPACE( closest->V.x, voxel_size_x );
      P->y = NS_TO_VOXEL_SPACE( closest->V.y, voxel_size_y );
      P->z = NS_TO_VOXEL_SPACE( closest->V.z, voxel_size_z );

		return NS_TRUE;
		}
	else
		return NS_FALSE;
	}
