
NS_PRIVATE void _ns_do_spines_info( const NsSpines *spines, NsSpinesInfo *info )
   {
   nslistiter      curr;
   nslistiter      end;
   const NsSpine  *spine;


   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      ++(info->num_spines);

      switch( spine->type )
         {
         case NS_SPINE_OTHER:
            ++(info->num_other_spines);
            break;

         case NS_SPINE_MUSHROOM:
            ++(info->num_mushroom_spines);
            break;

         case NS_SPINE_THIN:
            ++(info->num_thin_spines);
            break;

         case NS_SPINE_STUBBY:
            ++(info->num_stubby_spines);
            break;

         default:
            ns_assert_not_reached();
         }
      }
   }


void ns_spines_info_init( NsSpinesInfo *info )
   {
   ns_assert( NULL != info );
   ns_memzero( info, sizeof( NsSpinesInfo ) );
   }


void ns_model_spines_info( const NsModel *model, NsSpinesInfo *info )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
   ns_assert( NULL != info );

   ns_spines_info_init( info );
   _ns_do_spines_info( model->spines, info );
   }


void ns_spines_classifier_info_init( NsSpinesClassifierInfo *info )
	{
	ns_assert( NULL != info );

	info->num_spines = 0;
	info->counts     = NULL;
	info->num_counts = 0;
	}


NsError ns_model_spines_classifier_info( const NsModel *model, const NsSpinesClassifier *sc, NsSpinesClassifierInfo *info )
	{
	nsint            num_classes;
   nslistiter       curr;
   nslistiter       end;
	const NsSpines  *spines;
   const NsSpine   *spine;
	nsint            klass;


	ns_assert( NULL != model );
	ns_assert( NULL != sc );
	ns_assert( NULL != info );

	ns_spines_classifier_info_finalize( info );

	info->num_counts = ns_spines_classifier_num_classes( sc );
	num_classes      = ( nsint )info->num_counts;

	if( 0 < num_classes )
		if( NULL == ( info->counts = ns_new_array0( nssize, num_classes ) ) )
			return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	spines = model->spines;
	ns_assert( NULL != spines );

   curr = ns_list_begin( &spines->list );
   end  = ns_list_end( &spines->list );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      ++(info->num_spines);

		klass = ( nsint )spine->type;

		if( NULL != info->counts && 0 < klass && klass <= num_classes )
			++(info->counts[ klass - 1 ]);
      }

	return ns_no_error();
	}


void ns_spines_classifier_info_finalize( NsSpinesClassifierInfo *info )
	{
	ns_assert( NULL != info );

	ns_free( info->counts );

	info->num_spines = 0;
	info->counts     = NULL;
	info->num_counts = 0;
	}


nsspine ns_model_begin_spines( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_begin( &( model->spines->list ) );
   }


nsspine ns_model_end_spines( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_end( &( model->spines->list ) );
   }


nssize ns_model_num_spines( const NsModel *model )
   {
   ns_assert( NULL != model );
   return ns_list_size( &( model->spines->list ) );
   }


NS_PRIVATE NsSpine* _ns_spine_from_iter( nsspine S )
   {  return ns_list_iter_get_object( S );  }




nssize ns_spine_get_id( const nsspine S )
   {  return _ns_spine_from_iter( S )->id;  }


nsfloat ns_spine_get_x( const nsspine S )
   {  return _ns_spine_from_iter( S )->center.x;  }


nsfloat ns_spine_get_y( const nsspine S )
   {  return _ns_spine_from_iter( S )->center.y;  }


nsfloat ns_spine_get_z( const nsspine S )
   {  return _ns_spine_from_iter( S )->center.z;  }


NsVector3f* ns_spine_get_position( const nsspine S, NsVector3f *P )
	{
	ns_assert( NULL != P );
	return ns_vector3f( P, ns_spine_get_x( S ), ns_spine_get_y( S ), ns_spine_get_z( S ) );
	}


nsdouble ns_spine_get_voxel_volume( const nsspine S )
   {  return _ns_spine_from_iter( S )->voxel_volume;  }


nsdouble ns_spine_get_attach_surface_area( const nsspine S )
   {  return _ns_spine_from_iter( S )->attach_sa;  }


nsdouble ns_spine_get_non_attach_surface_area( const nsspine S )
   {  return _ns_spine_from_iter( S )->non_attach_sa;  }


nsfloat ns_spine_get_min_dts( const nsspine S )
   {  return _ns_spine_from_iter( S )->min_distance;  }


nsfloat ns_spine_get_max_dts( const nsspine S )
   {  return _ns_spine_from_iter( S )->height/*max_distance*/;  }


NsSpineType ns_spine_get_type( const nsspine S )
   {  return _ns_spine_from_iter( S )->type;  }


nsboolean ns_spine_get_auto_detected( const nsspine S )
   {  return _ns_spine_from_iter( S )->auto_detected;  }


nsulong ns_spine_get_section_number( const nsspine S )
   {  return _ns_spine_from_iter( S )->section;  }


nsfloat ns_spine_get_section_length( const nsspine S )
   {  return _ns_spine_from_iter( S )->section_length;  }


nsulong ns_spine_get_branch_order( const nsspine S )
   {  return _ns_spine_from_iter( S )->order;  }


nsdouble ns_spine_get_rayburst_volume( const nsspine S )
   {  return _ns_spine_from_iter( S )->total_volume;  }


nsdouble ns_spine_get_layers_volume( const nsspine S )
   {  return _ns_spine_from_iter( S )->layers_volume;  }


nsdouble ns_spine_get_layers_sa( const nsspine S )
   {  return _ns_spine_from_iter( S )->layers_sa;  }


nsdouble ns_spine_get_head_diameter( const nsspine S )
   {  return _ns_spine_from_iter( S )->stats.head_diameter;  }


nsdouble ns_spine_get_neck_diameter( const nsspine S )
   {  return _ns_spine_from_iter( S )->stats.neck_diameter;  }


void ns_spine_get_bounding_box( const nsspine S, NsAABBox3d *B )
	{
	ns_assert( NULL != B );
	*B = _ns_spine_from_iter( S )->bounding_box;
	}


nsdouble ns_spine_get_xyplane_angle( const nsspine S )
	{  return _ns_spine_from_iter( S )->xyplane_angle;  }


void ns_spine_get_attach_point( const nsspine S, NsVector3f *P )
	{
	ns_assert( NULL != P );
	*P = _ns_spine_from_iter( S )->attach;
	}


nslong ns_spine_get_swc_node_id( const nsspine S )
	{  return _ns_spine_from_iter( S )->attach_swc_vertex_id;  }


nsfloat ns_spine_get_swc_node_offset( const nsspine S )
	{  return _ns_spine_from_iter( S )->attach_swc_distance;  }


nsfloat ns_spine_get_soma_distance( const nsspine S )
	{  return _ns_spine_from_iter( S )->soma_distance;  }


void ns_spine_get_head_center( const nsspine S, NsVector3f *C )
	{
	ns_assert( NULL != C );
	*C = _ns_spine_from_iter( S )->head_center;
	}


void ns_spine_get_neck_center( const nsspine S, NsVector3f *C )
	{
	ns_assert( NULL != C );
	*C = _ns_spine_from_iter( S )->neck_center;
	}


nsboolean ns_spine_has_head_center( const nsspine S )
	{  return _ns_spine_from_iter( S )->has_head_center;  }


nsboolean ns_spine_has_neck_center( const nsspine S )
	{  return _ns_spine_from_iter( S )->has_neck_center;  }


void ns_spine_set_id( nsspine S, nssize id )
   {  _ns_spine_from_iter( S )->id = ( nsint )id;  }


void ns_spine_set_x( nsspine S, nsfloat x )
   {  _ns_spine_from_iter( S )->center.x = x;  }


void ns_spine_set_y( nsspine S, nsfloat y )
   {  _ns_spine_from_iter( S )->center.y = y;  }


void ns_spine_set_z( nsspine S, nsfloat z )
   {  _ns_spine_from_iter( S )->center.z = z;  }


void ns_spine_set_position( nsspine S, const NsVector3f *P )
	{
	ns_assert( NULL != P );

	ns_spine_set_x( S, P->x );
	ns_spine_set_y( S, P->y );
	ns_spine_set_z( S, P->z );
	}


void ns_spine_set_voxel_volume( nsspine S, nsdouble voxel_volume )
   {  _ns_spine_from_iter( S )->voxel_volume = voxel_volume;  }


void ns_spine_set_attach_surface_area( nsspine S, nsdouble attach_sa )
   {  _ns_spine_from_iter( S )->attach_sa = attach_sa;  }


void ns_spine_set_non_attach_surface_area( nsspine S, nsdouble non_attach_sa )
   {  _ns_spine_from_iter( S )->non_attach_sa = non_attach_sa;  }


void ns_spine_set_min_dts( nsspine S, nsfloat min_dts )
   {  _ns_spine_from_iter( S )->min_distance = min_dts;  }


void ns_spine_set_max_dts( nsspine S, nsfloat max_dts )
   {  _ns_spine_from_iter( S )->height/*max_distance*/ = max_dts;  }


void ns_spine_set_type( nsspine S, NsSpineType type )
   {  _ns_spine_from_iter( S )->type = type;  }


void ns_spine_set_auto_detected( nsspine S, nsboolean auto_detected )
   {  _ns_spine_from_iter( S )->auto_detected = auto_detected;  }


void ns_spine_set_section_number( nsspine S, nsulong section_number )
   {  _ns_spine_from_iter( S )->section = section_number;  }


void ns_spine_set_section_length( nsspine S, nsfloat section_length )
   {  _ns_spine_from_iter( S )->section_length = section_length;  }


void ns_spine_set_branch_order( nsspine S, nsulong branch_order )
   {  _ns_spine_from_iter( S )->order = branch_order;  }


void ns_spine_set_rayburst_volume( nsspine S, nsdouble rayburst_volume )
   {  _ns_spine_from_iter( S )->total_volume = rayburst_volume;  }


void ns_spine_set_layers_volume( nsspine S, nsdouble layers_volume )
   {  _ns_spine_from_iter( S )->layers_volume = layers_volume;  }


void ns_spine_set_layers_sa( nsspine S, nsdouble layers_sa )
   {  _ns_spine_from_iter( S )->layers_sa = layers_sa;  }


void ns_spine_set_head_diameter( nsspine S, nsdouble head_diameter )
   {  _ns_spine_from_iter( S )->stats.head_diameter = head_diameter;  }


void ns_spine_set_neck_diameter( nsspine S, nsdouble neck_diameter )
   {  _ns_spine_from_iter( S )->stats.neck_diameter = neck_diameter;  }


void ns_spine_set_bounding_box( nsspine S, const NsAABBox3d *B )
	{
	ns_assert( NULL != B );
	_ns_spine_from_iter( S )->bounding_box = *B;
	}


void ns_spine_set_xyplane_angle( nsspine S, nsdouble xyplane_angle )
	{  _ns_spine_from_iter( S )->xyplane_angle = xyplane_angle;  }


void ns_spine_set_attach_point( nsspine S, const NsVector3f *P )
	{
	ns_assert( NULL != P );
	_ns_spine_from_iter( S )->attach = *P;
	}


void ns_spine_set_swc_node_id( nsspine S, nslong swc_node_id )
	{  _ns_spine_from_iter( S )->attach_swc_vertex_id = swc_node_id;  }


void ns_spine_set_swc_node_offset( nsspine S, nsfloat swc_node_offset )
	{  _ns_spine_from_iter( S )->attach_swc_distance = swc_node_offset;  }


void ns_spine_set_soma_distance( nsspine S, nsfloat soma_distance )
	{  _ns_spine_from_iter( S )->soma_distance = soma_distance;  }


void ns_spine_set_head_center( nsspine S, const NsVector3f *C )
	{
	ns_assert( NULL != C );
	_ns_spine_from_iter( S )->head_center = *C;
	}


void ns_spine_set_neck_center( nsspine S, const NsVector3f *C )
	{
	ns_assert( NULL != C );
	_ns_spine_from_iter( S )->neck_center = *C;
	}


nsfloat ns_spine_get_sholl_distance( const nsspine S )
	{  return _ns_spine_from_iter( S )->sholl_distance;  }


void ns_spine_set_sholl_distance( nsspine S, nsfloat sholl_distance )
	{  _ns_spine_from_iter( S )->sholl_distance = sholl_distance;  }


typedef struct _NsModelSpinesLine
   {
   nslong     id;
   nslong     section_number;
   nsdouble   section_length;
   nslong     branch_order;
   nsdouble   rayburst_volume;
   nsdouble   voxel_volume;
   nsdouble   layers_volume;
   nsdouble   non_attach_sa;
   nsdouble   attach_sa;
   nsdouble   layers_sa;
   nsdouble   x;
   nsdouble   y;
   nsdouble   z;
   nsdouble   head_diameter;
   nsdouble   neck_diameter;
	nsdouble   min_dts;
   nsdouble   max_dts;
   nslong     type;
   nsboolean  is_auto;
	nsdouble   xyplane_angle;
	nslong     swc_node_id;
	nsdouble   swc_node_offset;
   nsdouble   attach_x;
   nsdouble   attach_y;
   nsdouble   attach_z;  
	nsdouble   soma_distance;
   nsdouble   head_center_x;
   nsdouble   head_center_y;
   nsdouble   head_center_z;
   nsdouble   neck_center_x;
   nsdouble   neck_center_y;
   nsdouble   neck_center_z;

	nsboolean  has_neck;
	nsboolean  has_head_center_x;
	nsboolean  has_head_center_y;
	nsboolean  has_head_center_z;
	nsboolean  has_neck_center_x;
	nsboolean  has_neck_center_y;
	nsboolean  has_neck_center_z;
   }
   NsModelSpinesLine;


typedef struct _NsModelSpinesHeader
   {
   nsulong        bit;
   const nschar  *label;
   NsError        ( *parse_func )( const nschar*, NsModelSpinesLine* );
   }
   NsModelSpinesHeader;


NS_PRIVATE NsError _ns_model_spines_parse_double( const nschar *ptr, nsdouble *value, nsdouble default_value, nsboolean *was_na )
   {
	if( was_na )
		*was_na = NS_FALSE;

   if( ns_ascii_strnieq( ptr, "N/A", ns_ascii_strlen( "N/A" ) ) )
		{
		if( was_na )
			*was_na = NS_TRUE;

      *value = default_value;
		}
   else
      {
      /* IMPORTANT: Cant use NS_FMT_DOUBLE for sscan() since we need the %lf specifier
         not the %f specifier! */
      if( 1 != _ns_sscan( ptr, "%lf", value ) )
         return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      }

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_spines_parse_long( const nschar *ptr, nslong *value, nslong default_value )
   {
   if( ns_ascii_strnieq( ptr, "N/A", ns_ascii_strlen( "N/A" ) ) )
      *value = default_value;
   else
      {
      if( 1 != _ns_sscan( ptr, NS_FMT_LONG, value ) )
         return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      }

   return ns_no_error();
   }


typedef struct _NsModelSpinesEnum
   {
   const nschar  *as_string;
   nslong         as_long;
   }
   NsModelSpinesEnum;


NS_PRIVATE nsboolean _ns_model_spines_parse_enum
   (
   const nschar             *ptr,
   const NsModelSpinesEnum  *choices,
   nssize                    count,
   nslong                   *value
   )
   {
   nssize i;

   for( i = 0; i < count; ++i )
      if( ns_ascii_strneq( ptr, choices[i].as_string, ns_ascii_strlen( choices[i].as_string ) ) )
         {
         *value = choices[i].as_long;
         return NS_TRUE;
         }

   return NS_FALSE;
   }


NS_PRIVATE NsError _ns_model_spines_parse_boolean( const nschar *ptr, nsboolean *set_value )
   {
   NS_PRIVATE NsModelSpinesEnum _ns_model_spines_parse_boolean_enum[] =
      {
      { "no",  ( nslong )NS_FALSE },
      { "yes", ( nslong )NS_TRUE  }
      };

   nslong get_value;


   if( ! _ns_model_spines_parse_enum(
				ptr,
				_ns_model_spines_parse_boolean_enum,
				NS_ARRAY_LENGTH( _ns_model_spines_parse_boolean_enum ),
				&get_value
				) )
		return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

   *set_value = ( nsboolean )get_value;
   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_spines_parse_id( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_long( ptr, &line->id, 0 );  }


NS_PRIVATE NsError _ns_model_spines_parse_section_number( const nschar *ptr, NsModelSpinesLine *line )
   {
   /* NOTE: The section numbers are 1-based, so if "N/A" is in the file then this value is set
      to -1, so that when the spine is added, a +1 operation will set this value to 0 which means
      that the section number is not set. See also _ns_model_spines_parse_line(). */
   return _ns_model_spines_parse_long( ptr, &line->section_number, -1 );
   }


NS_PRIVATE NsError _ns_model_spines_parse_section_length( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->section_length, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_branch_order( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_long( ptr, &line->branch_order, 0 );  }


NS_PRIVATE NsError _ns_model_spines_parse_rayburst_volume( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->rayburst_volume, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_voxel_volume( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->voxel_volume, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_layers_volume( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->layers_volume, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_non_attach_sa( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->non_attach_sa, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_attach_sa( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->attach_sa, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_layers_sa( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->layers_sa, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_x( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->x, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_y( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->y, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_z( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->z, 0.0, NULL );  }


/* IMPORTANT: 'head_diameter' and 'neck_diameter' must be set to less than zero,
   if "N/A" is in the file. */


NS_PRIVATE NsError _ns_model_spines_parse_head_diameter( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->head_diameter, NS_SPINE_DEFAULT_HEAD_DIAMETER, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_neck_diameter( const nschar *ptr, NsModelSpinesLine *line )
   {
	nsboolean  was_na;
	NsError    error;


	error = _ns_model_spines_parse_double( ptr, &line->neck_diameter, NS_SPINE_DEFAULT_NECK_DIAMETER, &was_na );

	if( ! ns_is_error( error ) )
		line->has_neck = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_min_dts( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->min_dts, -1.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_max_dts( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->max_dts, -1.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_auto( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_boolean( ptr, &line->is_auto );  }


NS_PRIVATE NsError _ns_model_spines_parse_xyplane_angle( const nschar *ptr, NsModelSpinesLine *line )
	{  return _ns_model_spines_parse_double( ptr, &line->xyplane_angle, NS_SPINE_DEFAULT_XYPLANE_ANGLE, NULL );  }


NsModelSpinesEnum  *____ns_model_spines_classifier_enum_values;
nssize              ____ns_model_spines_classifier_enum_size;
nschar              ____ns_model_spines_classifier_unmatched[512];


NS_PRIVATE NsError _ns_model_spines_parse_type( const nschar *ptr, NsModelSpinesLine *line )
   {
   NS_PRIVATE nsboolean _ns_model_spines_parse_type_enum_init = NS_FALSE;
   NS_PRIVATE NsModelSpinesEnum _ns_model_spines_parse_type_enum[ NS_SPINE_NUM_TYPES ];

	const NsModelSpinesEnum  *curr_enum;
	nssize                    curr_size;
   nslong                    value;


   if( ! _ns_model_spines_parse_type_enum_init )
      {
      _ns_model_spines_parse_type_enum[ NS_SPINE_INVALID ].as_string = ns_spine_type_to_string( NS_SPINE_INVALID );
      _ns_model_spines_parse_type_enum[ NS_SPINE_INVALID ].as_long   = ( nslong )NS_SPINE_INVALID;

      _ns_model_spines_parse_type_enum[ NS_SPINE_STUBBY ].as_string = ns_spine_type_to_string( NS_SPINE_STUBBY );
      _ns_model_spines_parse_type_enum[ NS_SPINE_STUBBY ].as_long   = ( nslong )NS_SPINE_STUBBY;

      _ns_model_spines_parse_type_enum[ NS_SPINE_THIN ].as_string = ns_spine_type_to_string( NS_SPINE_THIN );
      _ns_model_spines_parse_type_enum[ NS_SPINE_THIN ].as_long   = ( nslong )NS_SPINE_THIN;

      _ns_model_spines_parse_type_enum[ NS_SPINE_MUSHROOM ].as_string = ns_spine_type_to_string( NS_SPINE_MUSHROOM );
      _ns_model_spines_parse_type_enum[ NS_SPINE_MUSHROOM ].as_long   = ( nslong )NS_SPINE_MUSHROOM;

      _ns_model_spines_parse_type_enum[ NS_SPINE_OTHER ].as_string = ns_spine_type_to_string( NS_SPINE_OTHER );
      _ns_model_spines_parse_type_enum[ NS_SPINE_OTHER ].as_long   = ( nslong )NS_SPINE_OTHER;

      _ns_model_spines_parse_type_enum_init = NS_TRUE;
      }

	if( NULL != ns_spines_classifier_get() )
		if( NULL == ____ns_model_spines_classifier_enum_values )
			{
			const NsSpinesClassifier  *classifier;
			nsint                      num_classes;
			nsspineclass               curr, end;
			nsint                      value;


			classifier  = ns_spines_classifier_get();
			num_classes = ( nsint )ns_spines_classifier_num_classes( classifier );

			if( NULL == ( ____ns_model_spines_classifier_enum_values = ns_new_array0( NsModelSpinesEnum, num_classes ) ) )
				return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

			____ns_model_spines_classifier_enum_size = ( nssize )num_classes;

			curr = ns_spines_classifier_begin( classifier );
			end  = ns_spines_classifier_end( classifier );

			for( ; ns_spine_class_not_equal( curr, end ); curr = ns_spine_class_next( curr ) )
				{
				value = ns_spine_class_value( curr );

				ns_assert( 0 < value && value <= num_classes );
				ns_assert( NULL == ____ns_model_spines_classifier_enum_values[ value - 1 ].as_string );

				____ns_model_spines_classifier_enum_values[ value - 1 ].as_string = ns_spine_class_name( curr );
				____ns_model_spines_classifier_enum_values[ value - 1 ].as_long   = value;
				}
			}

	if( NULL == ns_spines_classifier_get() )
		{
		curr_enum = _ns_model_spines_parse_type_enum;
		curr_size = NS_ARRAY_LENGTH( _ns_model_spines_parse_type_enum );
		}
	else
		{
		curr_enum = ____ns_model_spines_classifier_enum_values;
		curr_size = ____ns_model_spines_classifier_enum_size;
		}

	if( ! _ns_model_spines_parse_enum( ptr, curr_enum, curr_size, &value ) )
		{
		const nschar  *src;
		nssize         dest;
		nschar         name[64];
		nssize         length;


		src  = ptr;
		dest = 0;

		/* NOTE: -1 to leave room for the NULL terminator. */
		while( *src && ! ns_ascii_isspace( *src ) && dest < NS_ARRAY_LENGTH( name ) - 1 )
			name[ dest++ ] = *src++;

		name[ dest ] = NS_ASCII_NULL;

		if( NULL == ns_ascii_strstr( ____ns_model_spines_classifier_unmatched, name ) )
			{
			/* NOTE: +2 for the comma and space. */
			length = ns_ascii_strlen( ____ns_model_spines_classifier_unmatched ) + ns_ascii_strlen( name ) + 2;

			/* NOTE: -1 to leave room for the NULL terminator. */
			if( length < NS_ARRAY_LENGTH( ____ns_model_spines_classifier_unmatched ) - 1 )
				{
				if( 0 < ns_ascii_strlen( ____ns_model_spines_classifier_unmatched ) )
					ns_ascii_strcat( ____ns_model_spines_classifier_unmatched, ", " );

				ns_ascii_strcat( ____ns_model_spines_classifier_unmatched, name );
				}
			}

		line->type = 0; /* i.e. invalid */
		}
	else
		line->type = ( nslong )value;

   return ns_no_error();
   }


NS_PRIVATE NsError _ns_model_spines_parse_swc_node_id( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_long( ptr, &line->swc_node_id, -1 );  }


NS_PRIVATE NsError _ns_model_spines_parse_swc_node_offset( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->swc_node_offset, -1.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_attach_x( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->attach_x, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_attach_y( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->attach_y, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_attach_z( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->attach_z, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_soma_distance( const nschar *ptr, NsModelSpinesLine *line )
   {  return _ns_model_spines_parse_double( ptr, &line->soma_distance, 0.0, NULL );  }


NS_PRIVATE NsError _ns_model_spines_parse_head_center_x( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->head_center_x, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_head_center_x = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_head_center_y( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->head_center_y, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_head_center_y = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_head_center_z( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->head_center_z, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_head_center_z = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_neck_center_x( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->neck_center_x, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_neck_center_x = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_neck_center_y( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->neck_center_y, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_neck_center_y = ! was_na;

	return error;
	}


NS_PRIVATE NsError _ns_model_spines_parse_neck_center_z( const nschar *ptr, NsModelSpinesLine *line )
   {
	NsError    error;
	nsboolean  was_na;


	error = _ns_model_spines_parse_double( ptr, &line->neck_center_z, -1.0, &was_na );

	if( ! ns_is_error( error ) )
		line->has_neck_center_z = ! was_na;

	return error;
	}


NS_PRIVATE NsModelSpinesHeader _ns_model_spines_headers[ NS_MODEL_SPINES_FILE_MAX_COLUMNS ] =
   {
   { NS_MODEL_SPINES_WRITE_ID,              "ID",              _ns_model_spines_parse_id              },
   { NS_MODEL_SPINES_WRITE_SECTION_NUMBER,  "SECTION-NUMBER",  _ns_model_spines_parse_section_number  },
   { NS_MODEL_SPINES_WRITE_SECTION_LENGTH,  "SECTION-LENGTH",  _ns_model_spines_parse_section_length  },
   { NS_MODEL_SPINES_WRITE_BRANCH_ORDER,    "BRANCH-ORDER",    _ns_model_spines_parse_branch_order    },
   { NS_MODEL_SPINES_WRITE_RAYBURST_VOLUME, "RAYBURST-VOLUME", _ns_model_spines_parse_rayburst_volume },
   { NS_MODEL_SPINES_WRITE_VOXEL_VOLUME,    "VOXEL-VOLUME",    _ns_model_spines_parse_voxel_volume    },
   { NS_MODEL_SPINES_WRITE_LAYERS_VOLUME,   "LAYERS-VOLUME",   _ns_model_spines_parse_layers_volume   },
   { NS_MODEL_SPINES_WRITE_NON_ATTACH_SA,   "NON-ATTACH-SA",   _ns_model_spines_parse_non_attach_sa   },
   { NS_MODEL_SPINES_WRITE_ATTACH_SA,       "ATTACH-SA",       _ns_model_spines_parse_attach_sa       },
   { NS_MODEL_SPINES_WRITE_LAYERS_SA,       "LAYERS-SA",       _ns_model_spines_parse_layers_sa       },
   { NS_MODEL_SPINES_WRITE_X,               "X",               _ns_model_spines_parse_x               },
   { NS_MODEL_SPINES_WRITE_Y,               "Y",               _ns_model_spines_parse_y               },
   { NS_MODEL_SPINES_WRITE_Z,               "Z",               _ns_model_spines_parse_z               },
   { NS_MODEL_SPINES_WRITE_HEAD_DIAMETER,   "HEAD-DIAMETER",   _ns_model_spines_parse_head_diameter   },
   { NS_MODEL_SPINES_WRITE_NECK_DIAMETER,   "NECK-DIAMETER",   _ns_model_spines_parse_neck_diameter   },
   { NS_MODEL_SPINES_WRITE_MIN_DTS,         "MIN-DTS",         _ns_model_spines_parse_min_dts         },
   { NS_MODEL_SPINES_WRITE_MAX_DTS,         "MAX-DTS",         _ns_model_spines_parse_max_dts         },
   { NS_MODEL_SPINES_WRITE_TYPE,            "TYPE",            _ns_model_spines_parse_type            },
   { NS_MODEL_SPINES_WRITE_AUTO,            "AUTO",            _ns_model_spines_parse_auto            },
	{ NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE,   "XYPLANE-ANGLE",   _ns_model_spines_parse_xyplane_angle   },
	{ NS_MODEL_SPINES_WRITE_SWC_NODE_ID,     "SWC-NODE-ID",     _ns_model_spines_parse_swc_node_id     },
	{ NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET, "SWC-NODE-OFFSET", _ns_model_spines_parse_swc_node_offset },
   { NS_MODEL_SPINES_WRITE_ATTACH_X,        "ATTACH-X",        _ns_model_spines_parse_attach_x        },
   { NS_MODEL_SPINES_WRITE_ATTACH_Y,        "ATTACH-Y",        _ns_model_spines_parse_attach_y        },
   { NS_MODEL_SPINES_WRITE_ATTACH_Z,        "ATTACH-Z",        _ns_model_spines_parse_attach_z        },
	{ NS_MODEL_SPINES_WRITE_SOMA_DISTANCE,   "SOMA-DISTANCE",   _ns_model_spines_parse_soma_distance   },
   { NS_MODEL_SPINES_WRITE_HEAD_CENTER_X,   "HEAD-CENTER-X",   _ns_model_spines_parse_head_center_x   },
   { NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y,   "HEAD-CENTER-Y",   _ns_model_spines_parse_head_center_y   },
   { NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z,   "HEAD-CENTER-Z",   _ns_model_spines_parse_head_center_z   },
   { NS_MODEL_SPINES_WRITE_NECK_CENTER_X,   "NECK-CENTER-X",   _ns_model_spines_parse_neck_center_x   },
   { NS_MODEL_SPINES_WRITE_NECK_CENTER_Y,   "NECK-CENTER-Y",   _ns_model_spines_parse_neck_center_y   },
   { NS_MODEL_SPINES_WRITE_NECK_CENTER_Z,   "NECK-CENTER-Z",   _ns_model_spines_parse_neck_center_z   }
   };


NS_PRIVATE nsboolean _ns_model_spines_lookup_header( nsulong bit, NsModelSpinesHeader *hdr )
   {
   nssize i;

   for( i = 0; i < NS_ARRAY_LENGTH( _ns_model_spines_headers ); ++i )
      if( bit == _ns_model_spines_headers[i].bit )
         {
         *hdr = _ns_model_spines_headers[i];
         return NS_TRUE;
         }

   return NS_FALSE;
   }


NS_PRIVATE NsError _ns_model_spines_parse_headers
   (
   const nschar         *ptr,
   NsModelSpinesHeader  *headers,
   nssize               *count,
	nsulong              *ret_flags
   )
   {
	const nschar  *scan;
   nsulong        flags;
   nssize         i;
	nssize         length;
	nssize         ptr_length;
   nssize         header_length;
   nsboolean      found;


   *count = 0;
   flags  = 0;

   while( NS_ASCII_NULL != *ptr )
      {
      found = NS_FALSE;

      for( i = 0; i < NS_ARRAY_LENGTH( _ns_model_spines_headers ); ++i )
         {
			/* Figure out how long this header in the file is. */

			ptr_length = 0;

			for( scan = ptr; NS_ASCII_NULL != *scan && ! ns_ascii_isspace( *scan ); ++scan )
				++ptr_length;

			/* This is how long a potential matched header is. */
         header_length = ns_ascii_strlen( _ns_model_spines_headers[i].label );

			/* NOTE: Have to take the larger of the lengths for string comparison. This is
				to allow headers to be "initial" sub-strings of other headers and still make
				us able to parse the file. For example, "X" is an initial sub-string of
				"XYPLANE_ANGLE" and this is allowed. */
			length = NS_MAX( ptr_length, header_length );

         if( ns_ascii_strneq( ptr, _ns_model_spines_headers[i].label, length ) )
            {
            /* Check if header label was seen already. */
            if( ( nsboolean )( flags & _ns_model_spines_headers[i].bit ) )
               return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );

            ns_verify( *count < NS_MODEL_SPINES_FILE_MAX_COLUMNS );

            headers[ *count ] = _ns_model_spines_headers[i];
            ++(*count);

            flags |= _ns_model_spines_headers[i].bit;

            ptr += header_length;

            /* Skip leading white-space. */
            while( *ptr && ns_ascii_isspace( *ptr ) )
               ++ptr;

            found = NS_TRUE;
            break;
            }
         }

      if( ! found )
         return ns_error_inval( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
      }

	*ret_flags = flags;
   return ns_no_error();
   }


NS_PRIVATE const nschar* _ns_model_spines_advance_parse( const nschar *ptr )
   {
   /* Skip NON-whitespace, i.e. current token. */
   while( *ptr && ! ns_ascii_isspace( *ptr ) )
      ++ptr;

   /* Now skip whitespace, i.e. advance to start of next token. */
   while( *ptr && ns_ascii_isspace( *ptr ) )
      ++ptr;

   return ptr;
   }


#ifdef NS_DEBUG

NS_PRIVATE void _ns_model_spines_line_print( const NsModelSpinesLine *line )
   {
   ns_println(
      "id=" NS_FMT_LONG " "
      "section_number=" NS_FMT_LONG " "
      "section_length=" NS_FMT_DOUBLE " "
      "branch_order=" NS_FMT_LONG " "
      "rayburst_volume=" NS_FMT_DOUBLE " "
      "voxel_volume=" NS_FMT_DOUBLE " "
      "layers_volume=" NS_FMT_DOUBLE " "
      "non_attach_sa=" NS_FMT_DOUBLE " "
      "attach_sa=" NS_FMT_DOUBLE " "
      "layers_sa=" NS_FMT_DOUBLE " "
      "x=" NS_FMT_DOUBLE " "
      "y=" NS_FMT_DOUBLE " "
      "z=" NS_FMT_DOUBLE " "
      "head_diameter=" NS_FMT_DOUBLE " "
      "neck_diameter=" NS_FMT_DOUBLE " "
      "min_dts=" NS_FMT_DOUBLE " "
      "max_dts=" NS_FMT_DOUBLE " "
      "type=" NS_FMT_LONG " "
      "is_auto=" NS_FMT_INT " "
		"xyplane_angle=" NS_FMT_DOUBLE " "
		"swc_node_id=" NS_FMT_LONG " "
		"swc_node_offset=" NS_FMT_DOUBLE " "
      "attach_x=" NS_FMT_DOUBLE " "
      "attach_y=" NS_FMT_DOUBLE " "
      "attach_z=" NS_FMT_DOUBLE " "
		"soma_distance=" NS_FMT_DOUBLE " "
      "head_center_x=" NS_FMT_DOUBLE " "
      "head_center_y=" NS_FMT_DOUBLE " "
      "head_center_z=" NS_FMT_DOUBLE " "
      "neck_center_x=" NS_FMT_DOUBLE " "
      "neck_center_y=" NS_FMT_DOUBLE " "
      "neck_center_z=" NS_FMT_DOUBLE " "
      NS_STRING_NEWLINE,
      line->id,
      line->section_number,
      line->section_length,
      line->branch_order,
      line->rayburst_volume,
      line->voxel_volume,
      line->layers_volume,
      line->non_attach_sa,
      line->attach_sa,
      line->layers_sa,
      line->x,
      line->y,
      line->z,
      line->head_diameter,
      line->neck_diameter,
      line->min_dts,
      line->max_dts,
      line->type,
      line->is_auto,
		line->xyplane_angle,
		line->swc_node_id,
		line->swc_node_offset,
      line->attach_x,
      line->attach_y,
      line->attach_z,
		line->soma_distance,
      line->head_center_x,
      line->head_center_y,
      line->head_center_z,
      line->neck_center_x,
      line->neck_center_y,
      line->neck_center_z
      );
   }

#endif/* NS_DEBUG */


NS_PRIVATE NsError _ns_model_spines_parse_line
   (
   const nschar               *ptr,
   const NsModelSpinesHeader  *headers,
   nssize                      count,
   NsModel                    *model,
	nsint                      *id_max,
	nsulong                     flags
   )
   {
   NsModelSpinesLine  line;
   nssize             i;
   nsspine            S;
	NsVector3f         Cf;
	NsVector3d         Cd;
	NsVector3f         A;
	NsVector3f         C;
	NsAABBox3d         bbox;
   NsError            error;


   ns_memzero( &line, sizeof( NsModelSpinesLine ) );

   for( i = 0; i < count; ++i )
      {
      if( NS_FAILURE( ( headers[i].parse_func )( ptr, &line ), error ) )
         return error;

      ptr = _ns_model_spines_advance_parse( ptr );
      }

   /*
   #ifdef NS_DEBUG
   _ns_model_spines_line_print( &line );
   #endif
   */

   if( NS_FAILURE( ns_model_add_spine_no_id( model, &S ), error ) )
      return error;

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
	if( *id_max < line.id )
		*id_max = ( nsint )line.id;

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
	ns_spine_set_id( S, line.id + model->spines->file );

	/*+ ( 0.Xf * ( nsfloat )rand() / RAND_MAX )*/
   ns_spine_set_x( S, ( nsfloat )line.x );
   ns_spine_set_y( S, ( nsfloat )line.y );
   ns_spine_set_z( S, ( nsfloat )line.z );

   ns_spine_set_voxel_volume( S, line.voxel_volume );
   ns_spine_set_attach_surface_area( S, line.attach_sa );
   ns_spine_set_non_attach_surface_area( S, line.non_attach_sa );
   ns_spine_set_min_dts( S, ( nsfloat )line.min_dts );
   ns_spine_set_max_dts( S, ( nsfloat )line.max_dts );

   ns_verify( 0 <= line.type );
   ns_spine_set_type( S, ( NsSpineType )line.type );

   ns_spine_set_auto_detected( S, line.is_auto );

   /* NOTE: The section numbers are 1-based so we do a +1 operation.
      See also _ns_model_spines_parse_section_number(). */
   ++line.section_number;
   ns_verify( 0 <= line.section_number );
   ns_spine_set_section_number( S, ( nsulong )line.section_number );

   ns_spine_set_section_length( S, ( nsfloat )line.section_length );

   ns_verify( 0 <= line.branch_order );
   ns_spine_set_branch_order( S, ( nsulong )line.branch_order );

   ns_spine_set_rayburst_volume( S, line.rayburst_volume );
   ns_spine_set_layers_volume( S, line.layers_volume );
   ns_spine_set_layers_sa( S, line.layers_sa );

	ns_vector3f( &Cf, ns_spine_get_x( S ), ns_spine_get_y( S ), ns_spine_get_z( S ) );

	ns_model_position_and_radius_to_aabbox_ex(
		ns_vector3f_to_3d( &Cf, &Cd ),
		NS_MODEL_SPINES_FIXED_RADIUS,
		&bbox
		);

	/* Set the bounding box so this spine can be selected properly. */
	ns_spine_set_bounding_box( S, &bbox );

   ns_spine_set_head_diameter(
		S,
		( flags & NS_MODEL_SPINES_WRITE_HEAD_DIAMETER ) ?
			line.head_diameter : NS_SPINE_DEFAULT_HEAD_DIAMETER
		);

   ns_spine_set_neck_diameter(
		S,
		( flags & NS_MODEL_SPINES_WRITE_NECK_DIAMETER ) ?
			line.neck_diameter : NS_SPINE_DEFAULT_NECK_DIAMETER
		);

	if( line.has_neck )
		_ns_spine_from_iter( S )->stats.has_neck = NS_TRUE;

	ns_spine_set_xyplane_angle(
		S,
		( flags & NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE ) ?
			line.xyplane_angle : NS_SPINE_DEFAULT_XYPLANE_ANGLE
		);

	ns_spine_set_swc_node_id( S, line.swc_node_id );
	ns_spine_set_swc_node_offset( S, ( nsfloat )line.swc_node_offset );

	A.x = ( nsfloat )line.attach_x;
	A.y = ( nsfloat )line.attach_y;
	A.z = ( nsfloat )line.attach_z;

	ns_spine_set_attach_point( S, &A );

	ns_spine_set_soma_distance( S, ( nsfloat )line.soma_distance );

	C.x = ( nsfloat )line.head_center_x;
	C.y = ( nsfloat )line.head_center_y;
	C.z = ( nsfloat )line.head_center_z;

   ns_spine_set_head_center( S, &C );

	if( line.has_head_center_x && line.has_head_center_y && line.has_head_center_z )
		_ns_spine_from_iter( S )->has_head_center = NS_TRUE;

	C.x = ( nsfloat )line.neck_center_x;
	C.y = ( nsfloat )line.neck_center_y;
	C.z = ( nsfloat )line.neck_center_z;

   ns_spine_set_neck_center( S, &C );

	if( line.has_neck_center_x && line.has_neck_center_y && line.has_neck_center_z )
		_ns_spine_from_iter( S )->has_neck_center = NS_TRUE;

	/* IMPORTANT: Dont let this spine be re-analyzed for
		volume, surface area, etc... */
	_ns_spine_from_iter( S )->is_analyzed = NS_TRUE;

   return ns_no_error();
   }


#define _NS_MODEL_SPINES_PARSE_MAX_LINE  1024

NS_PRIVATE NsError _ns_model_spines_parse_file
   (
   NsFile   *file,
   NsModel  *model,
   nslong   *line_num
   )
   {
   nsboolean             eof;
   nschar               *ptr;
   nsboolean             saw_headers;
   nssize                count;
	nssize                num_spines;
	nsint                 id_max;
	nsulong               flags;
   NsError               error;
   NsModelSpinesHeader   headers[ NS_MODEL_SPINES_FILE_MAX_COLUMNS ];
   nschar                line[ _NS_MODEL_SPINES_PARSE_MAX_LINE ];


   error       = ns_no_error();
   eof         = NS_FALSE;
   saw_headers = NS_FALSE;
   count       = 0;
	flags       = 0;
	num_spines  = ns_model_num_spines( model );
	id_max      = NS_INT_MIN;

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
	if( 0 < num_spines )
		model->spines->file += NS_SPINES_FILE_OFFSET;

   if( NULL != line_num )
      *line_num = 0;

   while( ! eof )
      {
      *line = NS_ASCII_NULL;

      if( NS_FAILURE( ns_file_readline( file, line, _NS_MODEL_SPINES_PARSE_MAX_LINE, &eof ), error ) )
         break;

      if( NULL != line_num )
         *line_num = *line_num + 1;

      ptr = line;

      /* Skip leading white-space. */
      while( *ptr && ns_ascii_isspace( *ptr ) )
         ++ptr;

      /* Skip empty lines. */
      if( ! *ptr )
         continue;

      if( ! saw_headers )
         {
         if( NS_FAILURE( _ns_model_spines_parse_headers( ptr, headers, &count, &flags ), error ) )
            break;

         saw_headers = NS_TRUE;
         }
      else if( NS_FAILURE( _ns_model_spines_parse_line( ptr, headers, count, model, &id_max, flags ), error ) )
         break;
      }

	/* NOTE: See also "Maintaining Spine ID's" in nsmodel-spines.h. */
	if( 0 == num_spines )
		model->spines->id = id_max + 1;

   return error;
   }


NsError ns_model_spines_read
   (
   NsModel       *model,
   const nschar  *name,
   nslong        *line_num
   )
   {
   NsFile   file;
   NsError  error;


   ns_assert( NULL != model );
   ns_assert( NULL != name );

   ns_log_entry(
      NS_LOG_ENTRY_FUNCTION,
      NS_FUNCTION
      "( model=" NS_FMT_STRING_DOUBLE_QUOTED
      ", name=" NS_FMT_STRING_DOUBLE_QUOTED
      ", line_num=" NS_FMT_POINTER
      " )",
      ns_model_get_name( model ),
      name,
      line_num
      );

   ns_file_construct( &file );

   if( NS_FAILURE( ns_file_open( &file, name, NS_FILE_MODE_READ ), error ) )
      {
      ns_file_destruct( &file );
      return error;
      }

	____ns_model_spines_classifier_enum_values  = NULL;
	____ns_model_spines_classifier_enum_size    = 0;
	____ns_model_spines_classifier_unmatched[0] = NS_ASCII_NULL;

	/* NOTE: Disable random color creation temporarily since we dont
		want to have random colors generated after each spine is added,
		just at the end. */
	ns_spines_enable_random_color_creation( NS_FALSE );

   error = _ns_model_spines_parse_file( &file, model, line_num );

	ns_spines_enable_random_color_creation( NS_TRUE );

	ns_free( ____ns_model_spines_classifier_enum_values );

	____ns_model_spines_classifier_enum_values = NULL;
	____ns_model_spines_classifier_enum_size   = 0;

   ns_file_destruct( &file );

	if( ! ns_is_error( error ) )
		ns_spines_create_random_colors( ns_model_num_spines( model ) );

   return error;
   }


const nschar* ns_model_spines_read_unmatched( void )
	{  return ____ns_model_spines_classifier_unmatched;  }


nsint _SPINES_WRITE_ARG_WIDTH;
#define _SPINES_WRITE_ARG_PRECISION  ( ( nsint )6 )
#define _SPINES_WRITE_FMT_STRING     NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_STRING )
#define _SPINES_WRITE_FMT_DOUBLE     NS_MAKE_FMT( NS_FMT_ARGUMENT NS_FMT_PRECISION NS_FMT_ARGUMENT, NS_FMT_TYPE_DOUBLE )
#define _SPINES_WRITE_FMT_INT        NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_INT )
#define _SPINES_WRITE_FMT_ULONG      NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_ULONG )
#define _SPINES_WRITE_FMT_LONG       NS_MAKE_FMT( NS_FMT_ARGUMENT, NS_FMT_TYPE_LONG )


NS_PRIVATE void _ns_model_spines_write_init_txt( void )
	{  _SPINES_WRITE_ARG_WIDTH = 16; }


NS_PRIVATE void _ns_model_spines_write_init_csv( void )
	{  _SPINES_WRITE_ARG_WIDTH = 0;  }


#define _SPINES_WRITE_WITH_PRECISION( format, which, value )\
   if( ( nsboolean )( flags & (which) ) )\
      if( NS_FAILURE(\
            ns_file_print(\
               &file,\
               format NS_FMT_STRING,\
               _SPINES_WRITE_ARG_WIDTH,\
               _SPINES_WRITE_ARG_PRECISION,\
               (value),\
					_NS_MODEL_SPINES_WRITE_CSV == type && (which) != last_column ? "," : ""\
               ),\
            error ) )\
         goto _NS_MODEL_SPINES_WRITE_EXIT

#define _SPINES_WRITE_NO_PRECISION( format, which, value )\
   if( ( nsboolean )( flags & (which) ) )\
      if( NS_FAILURE(\
            ns_file_print(\
               &file,\
               format NS_FMT_STRING,\
               _SPINES_WRITE_ARG_WIDTH,\
               (value),\
					_NS_MODEL_SPINES_WRITE_CSV == type && (which) != last_column ? "," : ""\
               ),\
            error ) )\
         goto _NS_MODEL_SPINES_WRITE_EXIT

#define _SPINES_WRITE_NO_PRECISION_EX( format, which, value, width )\
   if( ( nsboolean )( flags & (which) ) )\
      if( NS_FAILURE(\
            ns_file_print(\
               &file,\
               format NS_FMT_STRING,\
               (width),\
               (value),\
					_NS_MODEL_SPINES_WRITE_CSV == type && (which) != last_column ? "," : ""\
               ),\
            error ) )\
         goto _NS_MODEL_SPINES_WRITE_EXIT


#define _SPINES_WRITE_HEADER( which )\
   ns_verify( _ns_model_spines_lookup_header( (which), &hdr ) );\
   _SPINES_WRITE_STRING( (which), hdr.label )

#define _SPINES_WRITE_HEADER_EX( which, width )\
   ns_verify( _ns_model_spines_lookup_header( (which), &hdr ) );\
   _SPINES_WRITE_STRING_EX( (which), hdr.label, _NS_MODEL_SPINES_WRITE_TXT == type ? (width) : 0 )

#define _SPINES_WRITE_STRING( which, value )\
   _SPINES_WRITE_NO_PRECISION( _SPINES_WRITE_FMT_STRING, (which), (value) )

#define _SPINES_WRITE_STRING_EX( which, value, width )\
   _SPINES_WRITE_NO_PRECISION_EX( _SPINES_WRITE_FMT_STRING, (which), (value), _NS_MODEL_SPINES_WRITE_TXT == type ? (width) : 0 )

#define _SPINES_WRITE_INT( which, value )\
   _SPINES_WRITE_NO_PRECISION( _SPINES_WRITE_FMT_INT, (which), (value) )

#define _SPINES_WRITE_ULONG( which, value )\
   _SPINES_WRITE_NO_PRECISION( _SPINES_WRITE_FMT_ULONG, (which), (value) )

#define _SPINES_WRITE_LONG( which, value )\
   _SPINES_WRITE_NO_PRECISION( _SPINES_WRITE_FMT_LONG, (which), (value) )

#define _SPINES_WRITE_DOUBLE( which, value )\
   _SPINES_WRITE_WITH_PRECISION( _SPINES_WRITE_FMT_DOUBLE, (which), (value) )


enum{ _NS_MODEL_SPINES_WRITE_TXT, _NS_MODEL_SPINES_WRITE_CSV };

NsError _ns_model_spines_do_write
   (
   const NsModel     *model,
   const nschar      *name,
   nsulong            flags,
	const NsSettings  *settings,
	nsboolean          ( *warning )( const nschar*, nspointer ),
	nspointer          user_data,
	NsProgress        *progress,
	nsint              type
   )
   {
   NsFile                file;
   nslistiter            curr;
   nslistiter            end;
   NsModelSpinesHeader   hdr;
   NsSpine              *spine;
	NsSpinesClassifier   *classifier;
	nsint                 max_class_name_length;
	nssize                i;
	nsulong               last_column;
   NsError               error;


   ns_assert( NULL != model );
   ns_assert( NULL != model->spines );
   ns_assert( NULL != name );


	/* Find which column is the last one in each row... */
	last_column = 0;

	for( i = 0; i < NS_ARRAY_LENGTH( _ns_model_spines_headers ); ++i )
		if( ( nsboolean )( flags & _ns_model_spines_headers[i].bit ) )
			last_column = _ns_model_spines_headers[i].bit;

	error = ns_no_error();

	classifier = ns_spines_classifier_get();

	max_class_name_length = 0;

	if( NULL != classifier )
		{
		max_class_name_length = ( nsint )ns_spines_classifier_max_class_name_length( classifier );

		if( max_class_name_length < _SPINES_WRITE_ARG_WIDTH )
			max_class_name_length = _SPINES_WRITE_ARG_WIDTH;
		else
			++max_class_name_length; /* Add a space. */

		if( NS_FAILURE( ns_spines_classifier_init_lookup( classifier ), error ) )
			return error;
		}

	/* Check to see if any spines have an attachment area that is
		greater that the non-attachment area. */
	if( ( nsboolean )( flags & NS_MODEL_SPINES_WRITE_NON_ATTACH_SA ) ||
		 ( nsboolean )( flags & NS_MODEL_SPINES_WRITE_ATTACH_SA ) )
		{
		curr = ns_list_begin( &(model->spines->list) );
		end  = ns_list_end( &(model->spines->list) );

		for( ; ns_list_iter_not_equal( curr, end );
				 curr = ns_list_iter_next( curr ) )
			{
			spine = ns_list_iter_get_object( curr );

			if( spine->non_attach_sa < spine->attach_sa )
				{
				const nschar *message =
					"One or more spines has a surface area that\n"
					"is less than its attachment area.\n\n"
					"Would you like to save this file?";

				ns_println( NS_FMT_STRING, message );

				if( NULL != warning )
					if( ! ( warning )( message, user_data ) )
						return ns_no_error();

				break;
				}
			}
		}

	NS_USE_VARIABLE( settings );
	NS_USE_VARIABLE( progress );
	/*
	if( ( nsboolean )( flags & NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE ) )
		if( ns_model_spines_has_unset_xyplane_angles( model ) )
			if( NS_FAILURE(
					ns_model_spines_calc_xyplane_angles_if_not_set(
						( NsModel* )model,
						settings,
						progress
						),
					error ) )
				return error;
	*/

   ns_file_construct( &file );

   if( NS_FAILURE( ns_file_open( &file, name, NS_FILE_MODE_WRITE ), error ) )
      goto _NS_MODEL_SPINES_WRITE_EXIT;

   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_ID );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_SECTION_NUMBER );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_SECTION_LENGTH );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_BRANCH_ORDER );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_RAYBURST_VOLUME );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_VOXEL_VOLUME );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_LAYERS_VOLUME );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_NON_ATTACH_SA );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_ATTACH_SA );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_LAYERS_SA );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_X );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_Y );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_Z );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_HEAD_DIAMETER );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_NECK_DIAMETER );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_MIN_DTS );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_MAX_DTS );

	if( NULL != classifier )
		{  _SPINES_WRITE_HEADER_EX( NS_MODEL_SPINES_WRITE_TYPE, max_class_name_length );  }
	else
		{  _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_TYPE );  }

   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_AUTO );
	_SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE );
	_SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_SWC_NODE_ID );
	_SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_ATTACH_X );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_ATTACH_Y );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_ATTACH_Z );
	_SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_SOMA_DISTANCE );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_HEAD_CENTER_X );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_NECK_CENTER_X );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_NECK_CENTER_Y );
   _SPINES_WRITE_HEADER( NS_MODEL_SPINES_WRITE_NECK_CENTER_Z );

   if( NS_FAILURE( ns_file_print( &file, NS_STRING_NEWLINE ), error ) )
      goto _NS_MODEL_SPINES_WRITE_EXIT;

   curr = ns_list_begin( &(model->spines->list) );
   end  = ns_list_end( &(model->spines->list) );

   for( ; ns_list_iter_not_equal( curr, end );
          curr = ns_list_iter_next( curr ) )
      {
      spine = ns_list_iter_get_object( curr );

      _SPINES_WRITE_INT( NS_MODEL_SPINES_WRITE_ID, spine->id );

      /* NOTE: The section numbers are 1-based but to stay compatible with e.g. the
         saving of HOC files which represent sections in an array format which is
         0-based, just subtract by one. */
      if( NS_MODEL_EDGE_SECTION_NONE != spine->section )
         {
         ns_assert( 0 < spine->section );
         _SPINES_WRITE_ULONG( NS_MODEL_SPINES_WRITE_SECTION_NUMBER, spine->section - 1 );
         }
      else
         {  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_SECTION_NUMBER, "N/A" );  }

      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_SECTION_LENGTH, spine->section_length );

      if( NS_MODEL_EDGE_ORDER_NONE != spine->order )
         {  _SPINES_WRITE_ULONG( NS_MODEL_SPINES_WRITE_BRANCH_ORDER, spine->order );  }
      else
         {  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_BRANCH_ORDER, "N/A" );  }

      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_RAYBURST_VOLUME, spine->total_volume );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_VOXEL_VOLUME, spine->voxel_volume );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_LAYERS_VOLUME, spine->layers_volume );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_NON_ATTACH_SA, spine->non_attach_sa );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_ATTACH_SA, spine->attach_sa );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_LAYERS_SA, spine->layers_sa );

      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_X, spine->center.x );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_Y, spine->center.y );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_Z, spine->center.z );

      //if( 0.0f <= spine->stats.head_diameter )
         {  _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_HEAD_DIAMETER, spine->stats.head_diameter );  }
      //else
        // {  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_HEAD_DIAMETER, "N/A" );  }

      if( spine->stats.has_neck/*0.0f <= spine->stats.neck_diameter*/ )
         {  _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_NECK_DIAMETER, spine->stats.neck_diameter );  }
      else
         {  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_NECK_DIAMETER, "N/A" );  }

		if( 0.0f <= spine->min_distance )
			{  _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_MIN_DTS, spine->min_distance );  }
		else
			{  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_MIN_DTS, "N/A" );  }

		if( 0.0f <= spine->height/*max_distance*/ )
			{  _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_MAX_DTS, spine->height/*max_distance*/ );  }
		else
			{  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_MAX_DTS, "N/A" );  }

		if( NULL != classifier )
			{
			_SPINES_WRITE_STRING_EX(
				NS_MODEL_SPINES_WRITE_TYPE,
				ns_spines_classifier_lookup( classifier, ( nsint )spine->type ),
				max_class_name_length
				);
			}
		else
			{  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_TYPE, ns_spine_type_to_string( spine->type ) );  }

      _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_AUTO, spine->auto_detected ? "yes" : "no" );

		if( ! _NS_SPINE_XYPLANE_ANGLE_IS_UNSET( spine ) )
			{  _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE, spine->xyplane_angle );  }
		else
			{  _SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_XYPLANE_ANGLE, "N/A" );  }

		_SPINES_WRITE_LONG( NS_MODEL_SPINES_WRITE_SWC_NODE_ID, spine->attach_swc_vertex_id );
		_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_SWC_NODE_OFFSET, spine->attach_swc_distance );

      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_ATTACH_X, spine->attach.x );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_ATTACH_Y, spine->attach.y );
      _SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_ATTACH_Z, spine->attach.z );

		_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_SOMA_DISTANCE, spine->soma_distance );

      if( spine->has_head_center )
         {
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_HEAD_CENTER_X, spine->head_center.x );
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y, spine->head_center.y );
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z, spine->head_center.z );
			}
      else
         {
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_HEAD_CENTER_X, "N/A" );
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Y, "N/A" );
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_HEAD_CENTER_Z, "N/A" );
			}

      if( spine->has_neck_center )
         {
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_NECK_CENTER_X, spine->neck_center.x );
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_NECK_CENTER_Y, spine->neck_center.y );
			_SPINES_WRITE_DOUBLE( NS_MODEL_SPINES_WRITE_NECK_CENTER_Z, spine->neck_center.z );
			}
      else
         {
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_NECK_CENTER_X, "N/A" );
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_NECK_CENTER_Y, "N/A" );
			_SPINES_WRITE_STRING( NS_MODEL_SPINES_WRITE_NECK_CENTER_Z, "N/A" );
			}

      if( NS_FAILURE( ns_file_print( &file, NS_STRING_NEWLINE ), error ) )
         goto _NS_MODEL_SPINES_WRITE_EXIT;
      }

   _NS_MODEL_SPINES_WRITE_EXIT:

	if( NULL != classifier )
		ns_spines_classifier_finalize_lookup( classifier );

   ns_file_destruct( &file );
   return error;
   }


NsError ns_model_spines_write_txt
   (
   const NsModel     *model,
   const nschar      *name,
   nsulong            flags,
	const NsSettings  *settings,
	nsboolean          ( *warning )( const nschar *message, nspointer user_data ),
	nspointer          user_data,
	NsProgress        *progress
   )
	{
	_ns_model_spines_write_init_txt();
	return _ns_model_spines_do_write( model, name, flags, settings, warning, user_data, progress, _NS_MODEL_SPINES_WRITE_TXT );
	}


NsError ns_model_spines_write_csv
   (
   const NsModel     *model,
   const nschar      *name,
   nsulong            flags,
	const NsSettings  *settings,
	nsboolean          ( *warning )( const nschar *message, nspointer user_data ),
	nspointer          user_data,
	NsProgress        *progress
   )
	{
	_ns_model_spines_write_init_csv();
	return _ns_model_spines_do_write( model, name, flags, settings, warning, user_data, progress, _NS_MODEL_SPINES_WRITE_CSV );
	}
