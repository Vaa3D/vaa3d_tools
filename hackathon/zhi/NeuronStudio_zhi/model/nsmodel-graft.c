#include "nsmodel-graft.h"
#include <std/nsprofiler.h>


#define _NS_GRAFTER_NUM_VOXEL_COLORS  7


NS_PRIVATE nsboolean ____ns_grafting_did_init   = NS_FALSE;
NS_PRIVATE nsfloat   ____ns_grafting_size_ratio = 0.4f;
NS_PRIVATE nsboolean ____ns_grafting_do_filter  = NS_TRUE;


nsboolean ns_grafting_do_filter( void )
	{  return ____ns_grafting_do_filter;  }


NsError ns_grafting_read_params
	(
	const nschar  *file_name,
	nsboolean     *use_2d_sampling,
	nsfloat       *aabbox_scalar,
	nsint         *min_window
	)
	{
   NsConfigDb  db;
	NsStat      stat;
	nsboolean   read;
   NsError     error;

	NS_PRIVATE nsboolean ____ns_grafting_use_2d_sampling   = NS_FALSE;
	NS_PRIVATE nsfloat   ____ns_grafting_aabbox_scalar     = 3.0f;
	NS_PRIVATE nsint     ____ns_grafting_min_window        = 7;
	NS_PRIVATE nslong    ____ns_grafting_modification_time = 0;


	ns_assert( NULL != file_name );
	ns_assert( NULL != use_2d_sampling );
	ns_assert( NULL != aabbox_scalar );
	ns_assert( NULL != min_window );

	read = NS_FALSE;

	/* Only read the file if the modification time has changed
		since the last read. */
	if( NS_FAILURE( ns_stat( file_name, &stat ), error ) )
		read = NS_TRUE;

	if( ____ns_grafting_modification_time != ns_stat_mtime( &stat ) || ! ____ns_grafting_did_init )
		{
		//ns_println( "Time to read the file %s", file_name );
		____ns_grafting_modification_time = ns_stat_mtime( &stat );
		read = NS_TRUE;
		}
	//else
	//	ns_println( "Dont have to read the file %s", file_name );

	____ns_grafting_did_init = NS_TRUE;

	if( ! read )
		{
		*use_2d_sampling = ____ns_grafting_use_2d_sampling;
		*aabbox_scalar   = ____ns_grafting_aabbox_scalar;
		*min_window      = ____ns_grafting_min_window;

		/*TEMP!!!*/
		//ns_println( "stored use_2d_sampling = " NS_FMT_STRING, *use_2d_sampling ? "true" : "false" );
		//ns_println( "stored aabbox_scalar   = " NS_FMT_DOUBLE, *aabbox_scalar );
		//ns_println( "stored min_window      = " NS_FMT_INT, *min_window );
		}
	else
		{
		if( NS_FAILURE( ns_config_db_construct( &db ), error ) )
			return error;

		if( NS_FAILURE( ns_config_db_read( &db, file_name, NULL ), error ) )
			{
			ns_config_db_destruct( &db );
			return error;
			}

		if( ! ns_config_db_has_group( &db, "thresholding" ) )
			{
			ns_config_db_destruct( &db );
			return ns_error_noimp( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			}
   
		if( ! ns_config_db_has_key( &db, "thresholding", "is_2d" ) ||
			 ! ns_config_db_has_key( &db, "thresholding", "aabbox_scalar" ) ||
			 ! ns_config_db_has_key( &db, "thresholding", "min_window" ) )
			{
			ns_config_db_destruct( &db );
			return ns_error_noimp( NS_ERROR_LEVEL_RECOVERABLE, NS_MODULE );
			}

		*use_2d_sampling = ____ns_grafting_use_2d_sampling = ns_config_db_get_boolean( &db, "thresholding", "is_2d" );
		*aabbox_scalar   = ____ns_grafting_aabbox_scalar   = ( nsfloat )ns_config_db_get_double( &db, "thresholding", "aabbox_scalar" );
		*min_window      = ____ns_grafting_min_window      = ns_config_db_get_int( &db, "thresholding", "min_window" );

		/*TEMP!!!*/
		//ns_println( "READ use_2d_sampling = " NS_FMT_STRING, *use_2d_sampling ? "true" : "false" );
		//ns_println( "READ aabbox_scalar   = " NS_FMT_DOUBLE, *aabbox_scalar );
		//ns_println( "READ min_window      = " NS_FMT_INT, *min_window );

		if( ns_config_db_has_key( &db, "thresholding", "size_ratio" ) )
			____ns_grafting_size_ratio = ( nsfloat )ns_config_db_get_double( &db, "thresholding", "size_ratio" );

		if( ns_config_db_has_key( &db, "thresholding", "do_filter" ) )
			____ns_grafting_do_filter = ns_config_db_get_boolean( &db, "thresholding", "do_filter" );

		ns_config_db_destruct( &db );
		}

	/*TEMP*/
	ns_println( "grafting::use_2d_sampling = " NS_FMT_STRING, *use_2d_sampling ? "true" : "false" );
	ns_println( "grafting::aabbox_scalar   = " NS_FMT_DOUBLE, *aabbox_scalar );
	ns_println( "grafting::min_window      = " NS_FMT_INT, *min_window );
	ns_println( "grafting::size_ratio      = " NS_FMT_DOUBLE, ____ns_grafting_size_ratio );
	ns_println( "grafting::do_filter       = " NS_FMT_INT, ____ns_grafting_do_filter );

	return ns_no_error();
	}


typedef struct _NsGrafterShell
   {
   NsList     list;
   NsList    *voxels;
	nsushort   cluster_index;
   }
   NsGrafterShell;


NS_PRIVATE void ns_grafter_shell_construct( NsGrafterShell *shell )
   {
   shell->voxels = &shell->list;
   ns_list_construct( shell->voxels, NULL );
   }


NS_PRIVATE void ns_grafter_shell_destruct( NsGrafterShell *shell )
   {  ns_list_destruct( shell->voxels );  }



/* NOTE: These are flags for the grafter's voxels. The lowest 3 bits
	of the NsVoxel::flags member is used for a color. */
#define __NEW_VOXEL    ( ( nsushort )0x4000 )
#define __WAS_SHELLED  ( ( nsushort )0x8000 )


#define _ns_grafting_set_was_shelled( voxel )\
   ( (voxel)->flags |= __WAS_SHELLED )

#define _ns_grafting_get_was_shelled( voxel )\
   ( (voxel)->flags & __WAS_SHELLED )


NS_PRIVATE void _ns_grafter_voxel_set_color( NsVoxel *voxel, nsushort index )
	{  voxel->flags |= ( index % _NS_GRAFTER_NUM_VOXEL_COLORS );  }


NS_PRIVATE nsushort _ns_grafter_voxel_get_color( const NsVoxel *voxel )
	{  return voxel->flags & _NS_GRAFTER_NUM_VOXEL_COLORS;  }


NS_PRIVATE NsError ns_grafter_shell_add_voxel
   (
   NsGrafterShell  *shell,
   NsVoxel         *voxel
   )
   {
	_ns_grafter_voxel_set_color( voxel, shell->cluster_index );
	_ns_grafting_set_was_shelled( voxel );

	return ns_list_push_back( shell->voxels, voxel );
	}


typedef struct _NsGrafterCluster
   {
	NsGrafter      *owner;
   nsint           id;
   NsList          list;
   NsList         *voxels;
   nsmodelvertex   vertex;
	nsmodelvertex   parent;
	nsmodeledge     edge;
   NsVector3f      center;
   NsVector3f      pivot;
   nsfloat         max_distance_sq;
   nsfloat         max_distance;
   nsfloat         size;
   nsfloat         effect_threshold;
   nsfloat         effect_size;
	nsfloat         rayburst_threshold;
	nsfloat         contrast;
   nssize          num_values;
   nsboolean       remove;
   nsboolean       terminal;
   nspointer       sample;
   NsVector3i      C1;
   NsVector3i      C2;
	NsVector3f     *corners;
   NsVector3f      D;
	nsfloat         size_ratio;
	nsushort        index;
   }
   NsGrafterCluster;


struct _NsGrafter
   {
   NsModel               *raw_model;
	NsModel               *filtered_model;
	const NsImage         *image;
	const NsCubei         *roi;
   const NsSettings      *settings;
	NsVoxelInfo            imp_voxel_info;
   const NsVoxelInfo     *voxel_info;
   NsProgress            *progress;
	nsfloat                first_threshold;
	nsfloat                first_contrast;
   nsint                  cluster_id;
   NsRayburstInterpType   interp_type;
   nsdouble               average_intensity;
   nsfloat                min_confidence;
   nsfloat                update_rate;
   nsboolean              make_images;
   nsboolean              use_2d_sampling;
	nsfloat                aabbox_scalar;
	nsint                  min_window;
   NsVoxelBuffer          voxel_buffer;
   NsVector3i             origin;
   NsVector3i             max;
   NsSampler              sampler;
   NsList                 new_clusters;
   NsList                 new_voxels;
   NsVoxelTable           voxel_table;
	NsList                *prev_clusters;
   NsList                *curr_clusters;
   NsList                *next_clusters;
	NsList                 lists[3];
   nsboolean              have_split;
   nsmodelvertex          prev_junction;
   NsByteArray            threshold_values;
   NsRayburst             rayburst;
   const NsVector3i      *voxel_offsets;
   nsboolean              is_running;
	nsdouble               elapsed_time[2];
	nsmodelseed            seed;
	NsLabelingType         labeling_type;
	NsResidualSmearType    residual_smear_type;
	nsfloat                residual_smear_value;
	nsushort               cluster_index;
   };


NS_PRIVATE NsError ns_grafter_cluster_new
	(
	NsGrafterCluster  **ret_cluster,
	NsGrafter          *owner, 
	nsint               id
	)
   {
   NsGrafterCluster *cluster;

   if( NULL == ( cluster = ns_new0( NsGrafterCluster ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	cluster->owner = owner;

   cluster->voxels = &cluster->list;
   ns_list_construct( cluster->voxels, NULL );

   cluster->id = id;

   cluster->vertex   = NS_MODEL_VERTEX_NIL;
	cluster->edge     = NS_MODEL_EDGE_NIL;
   cluster->remove   = NS_FALSE;
   cluster->terminal = NS_FALSE;

   cluster->sample = NULL;

	cluster->corners = NULL;

	cluster->size = 0.0f;

	cluster->size_ratio = 1.0;

	cluster->index = 0;

   *ret_cluster = cluster;
   return ns_no_error();
   }


/*
NS_PRIVATE void _ns_grafter_cluster_remove_voxels( NsGrafterCluster *cluster )
	{
	NsVoxelTable  *table;
	nslistiter     curr, end;
	NsVoxel       *voxel;


	if( ! cluster->remove && ! cluster->terminal )
		{
		table = &(cluster->owner->voxel_table);
		curr  = ns_list_begin( cluster->voxels );
		end   = ns_list_end( cluster->voxels );

		for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
			{
			voxel = ns_list_iter_get_object( curr );

			ns_assert( voxel->data == cluster );
			ns_verify( ns_voxel_table_remove( table, voxel ) );
			}
		}
	}*/


NS_PRIVATE void ns_grafter_cluster_delete( NsGrafterCluster *cluster )
   {
	ns_delete( cluster->corners );

	/* NOTE: Comment out 'remove_voxels' to keep all the seen voxels. */
	//_ns_grafter_cluster_remove_voxels( cluster );

   ns_list_destruct( cluster->voxels );

   ns_delete( cluster );
   }


NS_PRIVATE nssize ns_grafter_cluster_size( const NsGrafterCluster *cluster )
   {  return ns_list_size( cluster->voxels );  }


NS_PRIVATE NsError ns_grafter_cluster_add_voxel
   (
   NsGrafterCluster  *cluster,
   NsVoxel           *voxel
   )
   {
   voxel->data = cluster;
   return ns_list_push_back( cluster->voxels, voxel );
   }


/* i.e. center of mass. */
NS_PRIVATE void ns_grafter_cluster_calc_center
   (
   NsGrafterCluster   *cluster,
   const NsVoxelInfo  *voxel_info
   )
   {
   nslistiter   iter;
   NsVoxel     *voxel;
   nsfloat      curr_intensity;
   nsfloat      sum_intensity;
   NsVector3f   V;
   NsVector3f   position_center;
   NsVector3f   mass_center;


   ns_assert( ! ns_list_is_empty( cluster->voxels ) );

   sum_intensity = 0.0f;

   ns_vector3f_zero( &position_center );
   ns_vector3f_zero( &mass_center );

   NS_LIST_FOREACH( cluster->voxels, iter )
      {
      voxel = ns_list_iter_get_object( iter );

      curr_intensity = voxel->intensity;
      sum_intensity += curr_intensity;

      ns_to_voxel_space( &voxel->position, &V, voxel_info );
      ns_vector3f_cmpd_add( &position_center, &V );

      if( 0.0f < curr_intensity )
         {
         ns_vector3f_cmpd_scale( &V, curr_intensity );
         ns_vector3f_cmpd_add( &mass_center, &V );
         }
      }

   if( 0.0f < sum_intensity )
      {
      ns_vector3f_cmpd_scale( &mass_center, 1.0f / sum_intensity );
      cluster->center = mass_center;
      }
   else
      {
      ns_vector3f_cmpd_scale( &position_center, 1.0f / ( nsfloat )ns_list_size( cluster->voxels ) );
      cluster->center = position_center;
      }
   }


NS_PRIVATE void ns_grafter_cluster_calc_size
   (
   NsGrafterCluster   *cluster,
   const NsVoxelInfo  *voxel_info
   )
   {
   nslistiter   iter;
   NsVoxel     *voxel;
   NsVector3f   max;
   NsVector3f   min;
   NsVector3f   V;


   min.x = min.y = min.z = NS_FLOAT_MAX;
   max.x = max.y = max.z = -NS_FLOAT_MAX;

   ns_assert( ! ns_list_is_empty( cluster->voxels ) );

   NS_LIST_FOREACH( cluster->voxels, iter )
      {
      voxel = ns_list_iter_get_object( iter );
      ns_to_voxel_space( &voxel->position, &V, voxel_info );

      if( V.x < min.x )min.x = V.x;
      if( V.y < min.y )min.y = V.y;
      if( V.z < min.z )min.z = V.z;
      if( V.x > max.x )max.x = V.x;
      if( V.y > max.y )max.y = V.y;
      if( V.z > max.z )max.z = V.z;
      }

   ns_vector3f_sub( &cluster->D, &max, &min ); /* D = max - min */
   ns_vector3f_cmpd_add( &cluster->D, ns_voxel_info_size( voxel_info ) ); /* D += voxel_size */

   cluster->size =
		ns_sqrtf(
			cluster->D.x * cluster->D.x +
			cluster->D.y * cluster->D.y +
			cluster->D.z * cluster->D.z
			); 
   }


/* NOTE: Returns distance squared to avoid the square root. */
NS_PRIVATE nsfloat ns_model_graft_voxel_distance_to_cluster
   (
   const NsVoxel           *voxel,
   const NsGrafterCluster  *cluster,
	const NsGrafter         *grafter
   )
   {
   NsVector3f  V;
	nsfloat     dx, dy, dz;


   ns_to_voxel_space( &voxel->position, &V, grafter->voxel_info );

	dx = ( V.x - cluster->pivot.x );
	dy = ( V.y - cluster->pivot.y );
	dz = ( V.z - cluster->pivot.z ) / grafter->residual_smear_value;

	return dx * dx + dy * dy + dz * dz;

	/*
   return NS_DISTANCE_SQUARED(
            V.x, V.y, V.z,
            cluster->pivot.x, cluster->pivot.y, cluster->pivot.z
            );
	*/
   }


/* Find the maximum distance from any voxel in the
   cluster to the cluster's pivot point. */
NS_PRIVATE void ns_grafter_cluster_calc_max_distance
   (
   NsGrafterCluster  *cluster,
	const NsGrafter   *grafter
   )
   {
   nslistiter  iter;
   nsfloat     distance;


   cluster->max_distance_sq = 0.0f;

   NS_LIST_FOREACH( cluster->voxels, iter )
      {
      distance = ns_model_graft_voxel_distance_to_cluster(
                  ns_list_iter_get_object( iter ),
                  cluster,
						grafter
                  );

      if( cluster->max_distance_sq < distance )
         cluster->max_distance_sq = distance;
      }

   cluster->max_distance = ns_sqrtf( cluster->max_distance_sq );
   }


NS_PRIVATE void ns_grafter_cluster_calc_pivot
   (
   NsGrafterCluster        *cluster,
   const NsGrafterCluster  *parent
   )
   {
   NsVector3f  D;
   nsfloat     u;


   ns_vector3f_sub( &D, &cluster->center, &parent->pivot );

   /* NOTE:

      if size = 0, u = 1.0
      if size = mag( D ), u = .5
      if size > mag( D ), u approaches 0.0
   */

	u = cluster->size < parent->size ?
			ns_powf( 0.5f, cluster->size / parent->size/*ns_vector3f_mag( &D )*/ )
				:
			ns_powf( 0.5f, parent->size / cluster->size );

   ns_vector3f_cmpd_scale( &D, u );
   ns_vector3f_add( &cluster->pivot, &parent->pivot, &D );

	//cluster->size_ratio = u;
	cluster->size_ratio = cluster->size / parent->size;
   }


NS_PRIVATE NsError ns_grafter_cluster_merge( NsGrafterCluster *dest, NsGrafterCluster *src )
   {
   nslistiter       iter;
   NsVoxel         *voxel;
	NsFinalizeFunc   finalize_func;
   NsError          error;


   NS_LIST_FOREACH( src->voxels, iter )
      {
      voxel = ns_list_iter_get_object( iter );
      ns_assert( voxel->data == src );

      if( NS_FAILURE( ns_grafter_cluster_add_voxel( dest, voxel ), error ) )
         return error;
      }

	/* Temporarily set the finalize function of the source cluster to NULL
		so that these voxels are not removed from the voxel table. */

	finalize_func = ns_list_get_finalize_func( src->voxels );
	ns_list_set_finalize_func( src->voxels, NULL );

   ns_list_clear( src->voxels );

	ns_list_set_finalize_func( src->voxels, finalize_func );

   return ns_no_error();
   }


#define _ns_grafting_set_is_new_voxel( voxel )\
   ( (voxel)->flags |= __NEW_VOXEL )

#define _ns_grafting_is_new_voxel( voxel )\
   ( (voxel)->flags & __NEW_VOXEL )

NS_PRIVATE void _ns_grafting_clear_is_new_voxel( NsVoxel *voxel )
   {  voxel->flags &= ~__NEW_VOXEL;  }


NS_PRIVATE void ns_grafter_clear( NsGrafter *grafter )
   {
   ns_list_clear( &grafter->new_voxels );

   ns_list_clear( &grafter->new_clusters );

	ns_list_clear( grafter->prev_clusters );
   ns_list_clear( grafter->curr_clusters );
   ns_list_clear( grafter->next_clusters );

   ns_sampler_clear( &grafter->sampler );

/*TEMP*/
ns_println( "GRAFTER # of voxels remaining in table = " NS_FMT_ULONG, ns_voxel_table_size( &grafter->voxel_table ) );
	ns_voxel_table_clear( &grafter->voxel_table );
   }


void ns_model_grafter_clear_voxels( NsModel *model )
	{
	ns_assert( NULL != model );
	ns_voxel_table_clear( &(model->grafter->voxel_table) );
	}


NS_PRIVATE NsError ns_grafter_init
   (
   NsGrafter             *grafter,
   NsModel               *raw_model,
	NsModel               *filtered_model,
   const NsSettings      *settings,
   const NsImage         *image,
	const NsCubei         *roi,
   nsboolean              use_proj_xy_as_image,
   const NsImage         *proj_xy,
   const NsImage         *proj_zy,
   const NsImage         *proj_xz,
	nsfloat                first_threshold,
	nsfloat                first_contrast,
   NsRayburstInterpType   interp_type,
   nsdouble               average_intensity,
   nsfloat                min_confidence,
   nsfloat                update_rate,
   nsboolean              make_images,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
	NsLabelingType         labeling_type,
	NsResidualSmearType    residual_smear_type,
   NsProgress            *progress
   )
   {
	nsfloat  x, y;
   NsError  error;


	error = ns_no_error();

   ns_grafter_clear( grafter );

   if( use_proj_xy_as_image )
      image = proj_xy;

   grafter->interp_type = interp_type;
   ns_println( "interp type = %d\n", ( nsint )interp_type );

   if( NS_FAILURE(
			ns_rayburst_construct(
            &grafter->rayburst,
            ns_settings_get_neurite_do_3d_radii( settings ) ?
            NS_RAYBURST_KERNEL_STD : NS_RAYBURST_KERNEL_2D,
            NS_RAYBURST_RADIUS_MLBD,
            interp_type,
            image,
            ns_settings_voxel_info( settings ),
            NULL
            ),
			error ) )
      return error;

	if( NS_LABELING_MEMBRANE == labeling_type )
		ns_rayburst_allow_dark_start( &grafter->rayburst, NS_TRUE );

   ns_assert( raw_model->grafter == grafter );
   grafter->raw_model = raw_model;
	
	grafter->filtered_model       = filtered_model;
   grafter->settings             = settings;
	grafter->image                = image;
	grafter->roi                  = roi;
	grafter->first_threshold      = first_threshold;
	grafter->first_contrast       = first_contrast;
   grafter->imp_voxel_info       = *( ns_settings_voxel_info( settings ) );
	grafter->voxel_info           = &grafter->imp_voxel_info;
   grafter->progress             = progress;
   grafter->cluster_id           = 0;
   grafter->average_intensity    = average_intensity;
   grafter->min_confidence       = min_confidence;
   grafter->update_rate          = update_rate;
   grafter->make_images          = make_images;
   grafter->use_2d_sampling      = use_2d_sampling;
	grafter->aabbox_scalar        = aabbox_scalar;
	grafter->min_window           = min_window;
   grafter->prev_junction        = NS_MODEL_VERTEX_NIL;
	grafter->cluster_index        = 0;
	grafter->labeling_type        = labeling_type;
	grafter->residual_smear_type  = residual_smear_type;
	grafter->residual_smear_value = ns_residual_smear_type_to_value( residual_smear_type );

	#ifdef NS_DEBUG
	ns_println( "residual smear value = " NS_FMT_DOUBLE, grafter->residual_smear_value );
	#endif

	/* If the image is 2D then make the voxel Z size equal to the smaller
		of the X and Y sizes (which are almost always the same anyway). This
		just keeps the voxels square and avoids problems that arise in the
		bounding box computation if the voxel Z size is set (e.g. to 1.0) to
		a value much larger than the X and Y voxel dimensions. */
	if( 1 == ns_image_length( grafter->image ) )
		{
		x = ns_voxel_info_size_x( grafter->voxel_info );
		y = ns_voxel_info_size_y( grafter->voxel_info );

		ns_voxel_info( &grafter->imp_voxel_info, x, y, NS_MIN( x, y ) );
		}

	ns_assert( 0 < ns_cubei_width( grafter->roi ) );
	ns_assert( 0 < ns_cubei_height( grafter->roi ) );
	ns_assert( 0 < ns_cubei_length( grafter->roi ) );

	grafter->elapsed_time[0] = 0.0f;
	grafter->elapsed_time[1] = 0.0f;

   ns_voxel_buffer_init( &grafter->voxel_buffer, image );

   ns_println( "dynamic_range = " NS_FMT_DOUBLE, ns_voxel_buffer_dynamic_range( &grafter->voxel_buffer ) );

   ns_sampler_init(
      &grafter->sampler,
      ns_settings_voxel_info( settings ),
      image,
      proj_xy,
      proj_zy,
      proj_xz
      );

   grafter->origin = *( ns_settings_neurite_seed( settings ) );

   if( use_proj_xy_as_image )
      grafter->origin.z = 0;

   ns_vector3i(
      &grafter->max,
      ns_voxel_buffer_width( &grafter->voxel_buffer )  - 1,
      ns_voxel_buffer_height( &grafter->voxel_buffer ) - 1,
      ns_voxel_buffer_length( &grafter->voxel_buffer ) - 1
      );

   grafter->have_split = NS_FALSE;

   return ns_no_error();
   }


NS_PRIVATE void ns_grafter_finalize( NsGrafter *grafter )
   {
   ns_grafter_clear( grafter );
   ns_rayburst_destruct( &grafter->rayburst );
   }


NS_PRIVATE NsError _ns_model_graft_add_new_voxel( NsGrafter *grafter, NsVoxel *voxel )
   {
   NsError error;

   ns_assert( ! _ns_grafting_is_new_voxel( voxel ) );

   if( NS_FAILURE( ns_list_push_back( &grafter->new_voxels, voxel ), error ) )
      return error;

   _ns_grafting_set_is_new_voxel( voxel );
   return ns_no_error();
   }


NS_PRIVATE void ns_grafter_cluster_calc_radius
   (
   const NsGrafterCluster  *cluster,
   NsGrafter               *grafter
   )
   {
   NsVector3f      position;
   //NsVector3f      fwd;
   //NsVector3f      rev;
	//NsVector3f      center;
   nsfloat         radius;
   //NsModelVertex  *vertex;


   NS_PROFILER_BEGIN( "calc_radius" );

	NS_USE_VARIABLE( grafter );

	ns_model_vertex_set_scoop( cluster->vertex, cluster->max_distance );

   ns_model_vertex_get_position( cluster->vertex, &position );

   ns_rayburst_set_threshold( &grafter->rayburst, cluster->rayburst_threshold );
   ns_rayburst( &grafter->rayburst, &position );

   radius = ns_rayburst_radius( &grafter->rayburst );
   ns_model_vertex_set_radius( cluster->vertex, radius );

   /* Get the vectors of the rayburst. */
   //ns_rayburst_mlbd( &grafter->rayburst, &center, &fwd, &rev, NULL );

   //vertex = ns_model_vertex( cluster->vertex );

   //ns_vector3f_sub( &vertex->fwd, &fwd, &position );
   //ns_vector3f_sub( &vertex->rev, &rev, &position );

   /*TEMP?*/
   #define __RADIUS_SIZE_RATIO  0.75f

   /* Some vertices near the surface may be created that have
      a small sample "window" but the radii is large. We account
      for this by setting the radius to the "ideal value", which
      is half the diagonal(cluster->size). */
   if( radius / cluster->size > __RADIUS_SIZE_RATIO )
		ns_model_vertex_set_radius( cluster->vertex, 0.5f * cluster->size );

	/* Re-center the vertex position. */
	//if( ns_grafting_do_filter() )
	//	ns_model_vertex_set_position( cluster->vertex, &center );

   NS_PROFILER_END();
   }


NS_PRIVATE nsint ns_model_graft_next_cluster_id( NsGrafter *grafter )
   {  return (grafter->cluster_id)++;  }


NS_PRIVATE NsError ns_model_graft_new_clustering
   (
   NsGrafter  *grafter,
   NsVoxel    *voxel1,
   NsVoxel    *voxel2
   )
   {
   NsGrafterCluster  *cluster1;
   NsGrafterCluster  *cluster2;
   NsGrafterCluster  *cluster;
   nsint              id1;
   nsint              id2;
   NsError            error;


   cluster1 = voxel1->data;
   cluster2 = voxel2->data;
   id1      = ( NULL != cluster1 ) ? cluster1->id : -1;
   id2      = ( NULL != cluster2 ) ? cluster2->id : -1;

   if( ( -1 == id1 ) && ( -1 == id2 ) )
      {
      /* Both are not in clusters yet. Create new cluster and add them
         both to it. */
      if( NS_FAILURE( ns_grafter_cluster_new(
                        &cluster,
								grafter,
                        ns_model_graft_next_cluster_id( grafter )
                        ),
                        error ) )
         return error;

      if( NS_FAILURE( ns_list_push_back( &grafter->new_clusters, cluster ), error ) )
         {
         ns_grafter_cluster_delete( cluster );
         return error;
         }

      if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster, voxel1 ), error ) ||
          NS_FAILURE( ns_grafter_cluster_add_voxel( cluster, voxel2 ), error )  )
         return error;
      }
   else if( ( -1 != id1 ) && ( -1 != id2 )  )
      {
      /* Both are in clusters already. If not the same cluster then
         merge cluster with less items to the larger one. */
      if( id1 != id2 )
         {
         if( ns_grafter_cluster_size( cluster1 ) < ns_grafter_cluster_size( cluster2 ) )
            {
            if( NS_FAILURE( ns_grafter_cluster_merge( cluster2, cluster1 ), error ) )
               return error;
            }
         else
            {
            if( NS_FAILURE( ns_grafter_cluster_merge( cluster1, cluster2 ), error ) )
               return error;
            }
         }
      }
   else
      {
      /* One of them is in a cluster and the other is not. */
      if( -1 != id1 )
         {
         if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster1, voxel2 ), error ) )
            return error;
         }
      else
         {
         if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster2, voxel1 ), error ) )
            return error;
         }
      }

   return ns_no_error();
   }


NS_PRIVATE void ns_model_graft_clean_new_clusters( NsGrafter *grafter )
   {
   NsGrafterCluster  *cluster;
   nslistiter         curr, next, end;


   curr = ns_list_begin( &grafter->new_clusters );
   end  = ns_list_end( &grafter->new_clusters );

   while( ns_list_iter_not_equal( curr, end ) )
      {
      next    = ns_list_iter_next( curr );
      cluster = ns_list_iter_get_object( curr );

      /* Remove any clusters with no voxels. */
      if( 0 == ns_grafter_cluster_size( cluster ) )
         {
         ns_assert( NULL == cluster->sample );
         ns_list_erase( &grafter->new_clusters, curr );
         }

      curr = next;
      }
   }


NS_PRIVATE NsError ns_model_graft_create_new_clusters( NsGrafter *grafter )
   {
   nslistiter         curr;
   nslistiter         end;
   NsVoxel           *voxel;
   NsVoxel           *neighbor;
   NsVoxel            N;
   nssize             i;
   nssize             num_neighbors;
   NsGrafterCluster  *cluster;
   NsError            error;


   curr = ns_list_begin( &grafter->new_voxels );
   end  = ns_list_end( &grafter->new_voxels );

   for( ; ns_list_iter_not_equal( curr, end ); curr = ns_list_iter_next( curr ) )
      {
      voxel         = ns_list_iter_get_object( curr );
      num_neighbors = 0;

      /* Loop through all neighbors of the new voxels and
         if they're present then proceed to the clustering
         algorithm. */
      for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
         {
         ns_vector3i_add( &N.position, &voxel->position, grafter->voxel_offsets + i );
         neighbor = ns_voxel_table_find( &grafter->voxel_table, &N.position );

         if( NULL != neighbor && _ns_grafting_is_new_voxel( neighbor ) )
            {
            ++num_neighbors;

            if( NS_FAILURE( ns_model_graft_new_clustering( grafter, voxel, neighbor ), error ) )
               return error;
            }
         }

      /* A single isolated voxel forms its own cluster. */
      if( 0 == num_neighbors )
         {
         if( NS_FAILURE( ns_grafter_cluster_new(
                           &cluster,
									grafter,
                           ns_model_graft_next_cluster_id( grafter )
                           ),
                           error ) )
            return error;

         if( NS_FAILURE( ns_list_push_back( &grafter->new_clusters, cluster ), error ) )
            {
            ns_grafter_cluster_delete( cluster );
            return error;
            }

         if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster, voxel ), error ) )
            return error;
         }
      }

   ns_model_graft_clean_new_clusters( grafter );

   return ns_no_error();
   }


NS_PRIVATE nsboolean _ns_grafter_cluster_sphere_octree_intersect_func
   (
   const NsSphered   *sphere,
   const NsAABBox3d  *box
   )
	{  return ns_sphered_intersects_aabbox( sphere, box );  }


NS_PRIVATE nsboolean _ns_grafter_cluster_vertex_intersection
	(
	NsGrafter         *grafter,
	NsGrafterCluster  *cluster
	)
	{
	NsVector             nodes;
	nsvectoriter         curr_node, end_nodes;
	const NsOctreeNode  *node;
	nsvectoriter         curr_object, end_objects;
	nsmodeledge          edge;
	nsmodelvertex        vertex;
	NsSphered            S1, S2;
	NsConicalFrustumd    F;
	nsboolean            inside;
	NsError              error;


	error  = ns_no_error();
	inside = NS_TRUE;

	ns_verify( NS_SUCCESS( ns_vector_construct( &nodes, 0, NULL ), error ) );

	ns_model_vertex_to_sphere_ex( cluster->vertex, &S1 );


	ns_model_lock( grafter->filtered_model );

	if( NS_FAILURE(
			ns_model_octree_intersections(
				grafter->filtered_model,
				NS_MODEL_EDGE_OCTREE,
				&S1,
				_ns_grafter_cluster_sphere_octree_intersect_func,
				&nodes
				),
			error ) )
		{
		ns_model_unlock( grafter->filtered_model );
		goto _NS_GRAFTER_CLUSTER_VERTEX_INTERSECTION_EXIT;
		}

	ns_model_unlock( grafter->filtered_model );


   curr_node = ns_vector_begin( &nodes );
   end_nodes = ns_vector_end( &nodes );

   for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
          curr_node = ns_vector_iter_next( curr_node ) )
      {
      node = ns_vector_iter_get_object( curr_node );

      curr_object = ns_octree_node_begin_objects( node );
      end_objects = ns_octree_node_end_objects( node );

      for( ; ns_vector_iter_not_equal( curr_object, end_objects );
             curr_object = ns_vector_iter_next( curr_object ) )
         {
         edge = ( nsmodeledge )ns_vector_iter_get_object( curr_object );

			if( ns_sphered_intersects_conical_frustum( &S1, ns_model_edge_to_conical_frustum( edge, &F ) ) )
				goto _NS_GRAFTER_CLUSTER_VERTEX_INTERSECTION_EXIT;
         }
      }

	ns_vector_resize( &nodes, 0 );


	ns_model_lock( grafter->filtered_model );

	if( NS_FAILURE(
			ns_model_octree_intersections(
				grafter->filtered_model,
				NS_MODEL_VERTEX_OCTREE,
				&S1,
				_ns_grafter_cluster_sphere_octree_intersect_func,
				&nodes
				),
			error ) )
		{
		ns_model_unlock( grafter->filtered_model );
		goto _NS_GRAFTER_CLUSTER_VERTEX_INTERSECTION_EXIT;
		}

	ns_model_unlock( grafter->filtered_model );


   curr_node = ns_vector_begin( &nodes );
   end_nodes = ns_vector_end( &nodes );

   for( ; ns_vector_iter_not_equal( curr_node, end_nodes );
          curr_node = ns_vector_iter_next( curr_node ) )
      {
      node = ns_vector_iter_get_object( curr_node );

      curr_object = ns_octree_node_begin_objects( node );
      end_objects = ns_octree_node_end_objects( node );

      for( ; ns_vector_iter_not_equal( curr_object, end_objects );
             curr_object = ns_vector_iter_next( curr_object ) )
         {
         vertex = ( nsmodelvertex )ns_vector_iter_get_object( curr_object );

			if( ns_sphered_intersects_sphere( &S1, ns_model_vertex_to_sphere_ex( vertex, &S2 ) ) )
				goto _NS_GRAFTER_CLUSTER_VERTEX_INTERSECTION_EXIT;
         }
      }

	inside = NS_FALSE;

	_NS_GRAFTER_CLUSTER_VERTEX_INTERSECTION_EXIT:

	ns_vector_destruct( &nodes );
	return inside;
	}


NS_PRIVATE NsError ns_model_graft_new_clusters_to_model
   (
   NsGrafter               *grafter,
   const NsGrafterCluster  *parent
   )
   {
   nslistiter         new_cluster;
   NsGrafterCluster  *cluster;
   NsError            error;


   NS_LIST_FOREACH( &grafter->new_clusters, new_cluster )
      {
      cluster = ns_list_iter_get_object( new_cluster );

      if( NS_FAILURE( ns_model_add_vertex( grafter->raw_model, &cluster->vertex ), error ) )
         return error;

      if( NS_FAILURE( ns_model_vertex_add_edge(
                        cluster->vertex,
                        parent->vertex,
                        &cluster->edge,
                        NULL
                        ),
                        error ) )
         return error;

      /* IMPORTANT: Assuming vertex data is all bits zero initially! */
		/*
      if( ns_model_vertex_is_junction( parent->vertex ) &&
          NS_ULONG_TO_POINTER(0) ==
          ns_model_vertex_get_data( parent->vertex, NS_MODEL_VERTEX_DATA_SLOT1 ) )
         {
         ns_model_vertex_set_data(
            parent->vertex,
            NS_MODEL_VERTEX_DATA_SLOT1,
            NS_ULONG_TO_POINTER( ( nsulong )1 )
            );

         ns_model_vertex_set_list( parent->vertex, grafter->prev_junction );
         grafter->prev_junction = parent->vertex;
         }
		*/

		/* Here's where we set the vertex position before rayburst is run. */
      ns_model_vertex_set_position( cluster->vertex, &cluster->pivot );
		//ns_model_vertex_set_position( cluster->vertex, &cluster->center );

      ns_model_vertex_set_threshold(
         cluster->vertex,
         NS_MODEL_VERTEX_NEURITE_THRESHOLD,
         cluster->effect_threshold
         );

		ns_model_vertex_set_contrast(
			cluster->vertex,
			NS_MODEL_VERTEX_NEURITE_CONTRAST,
			cluster->contrast
			);

		ns_model_vertex_set_auto_iter( cluster->vertex );

      ns_grafter_cluster_calc_radius( cluster, grafter );

		//ns_model_vertex_set_size_ratio( cluster->vertex, cluster->size_ratio );

		/* Detect for intersection with the model stored in the octrees. If
			this vertex intersects anything, then remove it and kill this
			cluster by setting the cluster->terminal flag to true. Assuming
			that those clusters marked with cluster->terminal to true are
			removed after this routine. */
		if( _ns_grafter_cluster_vertex_intersection( grafter, cluster ) )
			{
			ns_model_remove_vertex( grafter->raw_model, cluster->vertex );
			cluster->terminal = NS_TRUE;

			ns_model_vertex_set_scoop( parent->vertex, cluster->max_distance );
			}
		else if( cluster->terminal )
			{
			ns_model_vertex_set_scoop( cluster->vertex, cluster->max_distance );
			ns_model_vertex_mark_unremovable( cluster->vertex, NS_TRUE );
			}
      }

   return ns_no_error();
   }


NS_PRIVATE void ns_model_graft_make_images
   (
   NsGrafter               *grafter,
   NsGrafterCluster        *cluster,
   const NsGrafterCluster  *parent
   )
   {
   /* NOTE: Ignoring errors. This is only for display purposes. */

   ns_sampler_add( &grafter->sampler, &cluster->sample );

   ns_sample_set_threshold( cluster->sample, parent->effect_threshold );
	ns_sample_set_contrast( cluster->sample, parent->contrast );
   ns_sample_set_corners( cluster->sample, &cluster->C1, &cluster->C2 );

   //ns_sample_make_images( &grafter->sampler, cluster->sample );
   }


void ns_grafting_sample_values
   (
   NsVoxelBuffer     *voxels,
   nsfloat           *values,
   nssize             num_values,
   const NsVector3i  *C1,
   const NsVector3i  *C2
   )
   {
   nsulong  intensity;
   nssize   index;
   nsint    c1x, c1y, c1z, c2x, c2y, c2z, px, py, pz;


   c1x = C1->x;
   c1y = C1->y;
   c1z = C1->z;

   c2x = C2->x;
   c2y = C2->y;
   c2z = C2->z;

   index = 0;

   for( pz = c1z; pz <= c2z; ++pz )
      for( py = c1y; py <= c2y; ++py )
         for( px = c1x; px <= c2x; ++px )
            {
            intensity = ns_voxel_get( voxels, px, py, pz );

            values[ index ] = ( nsfloat )intensity;
            ++index;
            }

   ns_assert( index == num_values );
   }


NS_PRIVATE void ns_grafting_calc_aabbox_init
   (
   nsfloat           max_distance,
   const NsPoint3f  *pivot,
   nsboolean         use_2d_sampling,
	nsfloat           aabbox_scalar,
   NsVector3f       *C1f,
   NsVector3f       *C2f
   )
   {
   nsfloat dimension = max_distance * aabbox_scalar;

   /* Find the corners of the bounding cube centered at
      the pivot. */

   C1f->x = ( pivot->x - dimension );
   C1f->y = ( pivot->y - dimension );
   C1f->z = ( pivot->z - ( use_2d_sampling ? 0.0f : dimension ) );
   C2f->x = ( pivot->x + dimension );
   C2f->y = ( pivot->y + dimension );
   C2f->z = ( pivot->z + ( use_2d_sampling ? 0.0f : dimension ) );
   }


void ns_grafting_do_calc_aabbox
   (
   const NsVoxelInfo  *voxel_info,
   const NsPoint3f    *pivot,
   const NsVector3i   *max,
   nsboolean           use_2d_sampling,
	nsint               min_window,
	NsVector3f         *C1f,
	NsVector3f         *C2f,
   NsVector3i         *C1,
   NsVector3i         *C2
   )
   {
   nsint       d1;
   nsint       d2;
   NsVector3i  P;
   NsVector3i  C1i;
   NsVector3i  C2i;


	/* i.e. is the value odd? */
	ns_assert( ( nsboolean )( min_window & 1 ) );

   if( C1f->x < 0.0f )C1f->x = 0.0f;
   if( C1f->y < 0.0f )C1f->y = 0.0f;
   if( C1f->z < 0.0f )C1f->z = 0.0f;
   if( C2f->x < 0.0f )C2f->x = 0.0f;
   if( C2f->y < 0.0f )C2f->y = 0.0f;
   if( C2f->z < 0.0f )C2f->z = 0.0f;

   ns_to_image_space( C1f, &C1i, voxel_info );
   ns_to_image_space( C2f, &C2i, voxel_info );

   ns_assert( C1i.x <= C2i.x );
   ns_assert( C1i.y <= C2i.y );
   ns_assert( C1i.z <= C2i.z );

   ns_to_image_space( pivot, &P, voxel_info );

   /* Make sure the cube is symmetrical. */

   d1 = NS_ABS( C1i.x - P.x );
   d2 = NS_ABS( C2i.x - P.x );

   if( d1 < d2 )
      C1i.x -= ( d2 - d1 );
   else if( d1 > d2 )
      C2i.x += ( d1 - d2 );

   ns_assert( NS_ABS( C1i.x - P.x ) == NS_ABS( C2i.x - P.x ) );

   d1 = NS_ABS( C1i.y - P.y );
   d2 = NS_ABS( C2i.y - P.y );

   if( d1 < d2 )
      C1i.y -= ( d2 - d1 );
   else if( d1 > d2 )
      C2i.y += ( d1 - d2 );

   ns_assert( NS_ABS( C1i.y - P.y ) == NS_ABS( C2i.y - P.y ) );

   d1 = NS_ABS( C1i.z - P.z );
   d2 = NS_ABS( C2i.z - P.z );

   if( d1 < d2 )
      C1i.z -= ( d2 - d1 );
   else if( d1 > d2 )
      C2i.z += ( d1 - d2 );

   ns_assert( NS_ABS( C1i.z - P.z ) == NS_ABS( C2i.z - P.z ) );

   /* Dont let the cube dimensions be too small... */

   if( ( C2i.x - C1i.x + 1 ) < min_window )
      {
      d1 = NS_ABS( C1i.x - P.x );
      d2 = NS_ABS( C2i.x - P.x );

      ns_assert( d1 < min_window / 2 );
      ns_assert( d2 < min_window / 2 );
      ns_assert( d1 == d2 );

      C1i.x -= ( min_window / 2 - d1 );
      C2i.x += ( min_window / 2 - d2 );

      ns_assert( min_window == ( C2i.x - C1i.x + 1 ) );
      }

   if( ( C2i.y - C1i.y + 1 ) < min_window )
      {
      d1 = NS_ABS( C1i.y - P.y );
      d2 = NS_ABS( C2i.y - P.y );

      ns_assert( d1 < min_window / 2 );
      ns_assert( d2 < min_window / 2 );
      ns_assert( d1 == d2 );

      C1i.y -= ( min_window / 2 - d1 );
      C2i.y += ( min_window / 2 - d2 );

      ns_assert( min_window == ( C2i.y - C1i.y + 1 ) );
      }

   if( ! use_2d_sampling )
      if( ( C2i.z - C1i.z + 1 ) < min_window )
         {
         d1 = NS_ABS( C1i.z - P.z );
         d2 = NS_ABS( C2i.z - P.z );

         ns_assert( d1 < min_window / 2 );
         ns_assert( d2 < min_window / 2 );
         ns_assert( d1 == d2 );

         C1i.z -= ( min_window / 2 - d1 );
         C2i.z += ( min_window / 2 - d2 );

         ns_assert( min_window == ( C2i.z - C1i.z + 1 ) );
         }

   /* NOTE: Edge effect might make the cube unsymmetrical
      but in most cases this wont be a problem. */
   C1i.x = NS_CLAMP( C1i.x, 0, max->x );
   C1i.y = NS_CLAMP( C1i.y, 0, max->y );
   C1i.z = NS_CLAMP( C1i.z, 0, max->z );
   C2i.x = NS_CLAMP( C2i.x, 0, max->x );
   C2i.y = NS_CLAMP( C2i.y, 0, max->y );
   C2i.z = NS_CLAMP( C2i.z, 0, max->z );

   *C1 = C1i;
   *C2 = C2i;
   }


void ns_grafting_calc_aabbox
   (
   const NsVoxelInfo  *voxel_info,
   nsfloat             max_distance,
   const NsPoint3f    *pivot,
   const NsVector3i   *max,
   nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar,
	nsint               min_window,
   NsVector3i         *C1,
   NsVector3i         *C2
   )
   {
   NsVector3f C1f, C2f;

	ns_grafting_calc_aabbox_init(
		max_distance,
		pivot,
		use_2d_sampling,
		aabbox_scalar,
		&C1f,
		&C2f
		);

	ns_grafting_do_calc_aabbox(
		voxel_info,
		pivot,
		max,
		use_2d_sampling,
		min_window,
		&C1f,
		&C2f,
		C1,
		C2
		);
   }


NS_PRIVATE void ns_grafting_calc_aabbox_init_ex
   (
	nsfloat           radius_x,
	nsfloat           radius_y,
	nsfloat           radius_z,
   const NsPoint3f  *pivot,
   nsboolean         use_2d_sampling,
	nsfloat           aabbox_scalar,
   NsVector3f       *C1f,
   NsVector3f       *C2f
   )
   {
   nsfloat dimension_x, dimension_y, dimension_z;

	dimension_x = radius_x * aabbox_scalar;
	dimension_y = radius_y * aabbox_scalar;
	dimension_z = radius_z * aabbox_scalar;

   /* Find the corners of the bounding cube centered at
      the pivot. */

   C1f->x = ( pivot->x - dimension_x );
   C1f->y = ( pivot->y - dimension_y );
   C1f->z = ( pivot->z - ( use_2d_sampling ? 0.0f : dimension_z ) );
   C2f->x = ( pivot->x + dimension_x );
   C2f->y = ( pivot->y + dimension_y );
   C2f->z = ( pivot->z + ( use_2d_sampling ? 0.0f : dimension_z ) );
   }


void ns_grafting_calc_aabbox_ex
	(
	const NsVoxelInfo  *voxel_info,
	nsfloat             radius_x,
	nsfloat             radius_y,
	nsfloat             radius_z,
	const NsPoint3f    *pivot,
	const NsVector3i   *max,
	nsboolean           use_2d_sampling,
	nsfloat             aabbox_scalar,
	nsint               min_window,
	NsVector3i         *C1,
	NsVector3i         *C2
	)
	{
   NsVector3f C1f, C2f;

	ns_grafting_calc_aabbox_init_ex(
		radius_x,
		radius_y,
		radius_z,
		pivot,
		use_2d_sampling,
		aabbox_scalar,
		&C1f,
		&C2f
		);

	ns_grafting_do_calc_aabbox(
		voxel_info,
		pivot,
		max,
		use_2d_sampling,
		min_window,
		&C1f,
		&C2f,
		C1,
		C2
		);
	}


NS_PRIVATE NsError _ns_grafting_calc_non_aabbox
   (
   NsGrafterCluster        *cluster,
   const NsGrafterCluster  *parent,
   NsGrafter               *grafter
   )
	{
	NsVector3f   direction;
	NsVector3i  *points;
	nssize       num_points;
	nsint        width, height, length;
	nssize       i;
	NsVector3i   V;
	NsVector3i   pivot;
	NsVector3i   corners[8];


	ns_to_image_space( &cluster->pivot, &pivot, grafter->voxel_info );

	ns_vector3f_sub( &direction, &cluster->pivot, &parent->pivot );

	points = 
		get_non_axis_aligned_set(
			ns_to_image_space_component(
				cluster->max_distance * grafter->aabbox_scalar,
				grafter->voxel_info,
				NS_COMPONENT_X
				),
			ns_to_image_space_component(
				cluster->max_distance * grafter->aabbox_scalar,
				grafter->voxel_info,
				NS_COMPONENT_Y
				),
			grafter->use_2d_sampling ?
				0
				:
			ns_to_image_space_component(
				cluster->max_distance * grafter->aabbox_scalar,
				grafter->voxel_info,
				NS_COMPONENT_Z
				),
			direction.x,
			direction.y,
			direction.z,
			&num_points,
			corners
			);

	if( NULL == points )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	/* TEMP!!!!!!!!!!!!!!!!!! */
	ns_free( points );

	width  = ( nsint )ns_image_width( grafter->image );
	height = ( nsint )ns_image_height( grafter->image );
	length = ( nsint )ns_image_length( grafter->image );

	/* NOTE: The corners are for display only so dont "worry" about
		memory allocation failure. */
	if( NULL != ( cluster->corners = ns_new_array( NsVector3f, 8 ) ) )
		for( i = 0; i < 8; ++i )
			{
			ns_vector3i_add( &V, corners + i, &pivot );

			if( V.x < 0 )
				V.x = 0;
			else if( width <= V.x )
				V.x = width - 1;

			if( V.y < 0 )
				V.y = 0;
			else if( height <= V.y )
				V.y = height - 1;

			if( V.z < 0 )
				V.z = 0;
			else if( length <= V.z )
				V.z = length - 1;

			ns_to_voxel_space( &V, cluster->corners + i, grafter->voxel_info );
			}

	return ns_no_error();
	}


#include <ext/threshold.h>


/* NOTE: Set to something huge, say 500,000,000 to ensure that the
	window size is always 1. */
#define _NS_GRAFTING_NUM_SAMPLES  1000


NS_PRIVATE void ns_grafting_sample_values_ex
	(
   NsVoxelBuffer     *voxels,
   nsfloat           *values,
   nssize             in_num_values,
   const NsVector3i  *C1,
   const NsVector3i  *C2,
	nssize             window,
	nsfloat            probability,
	nssize            *out_num_values
	)
	{
   nsulong  intensity;
   nssize   index, at, sample_pos;
   nsint    c1x, c1y, c1z, c2x, c2y, c2z, px, py, pz;


   c1x = C1->x;
   c1y = C1->y;
   c1z = C1->z;

   c2x = C2->x;
   c2y = C2->y;
   c2z = C2->z;

   index      = 0;
	at         = 0;
	sample_pos = 0;

   for( pz = c1z; pz <= c2z; ++pz )
      for( py = c1y; py <= c2y; ++py )
         for( px = c1x; px <= c2x; ++px )
            {
				if( 0 == at )
					sample_pos = ( ( nssize )ns_global_randi() ) % window;

				if( at == sample_pos )
					{
					/* Figure out if we're going to actually sample here. */
					if( ns_global_randf() <= probability )
						{
						intensity = ns_voxel_get( voxels, px, py, pz );

						values[ index ] = ( nsfloat )intensity;
						++index;
						}
					}

				++at;

				if( at >= window )
					at = 0;
            }

//ns_println( "Generated " NS_FMT_ULONG " out of " NS_FMT_ULONG, index, _NS_GRAFTING_NUM_SAMPLES );

   ns_assert( index <= in_num_values );
	*out_num_values = index;
	}


enum{ _NS_GRAFTER_TRACING_THRESHOLD, _NS_GRAFTER_RAYBURST_THRESHOLD };

/* NOTE: 'which' is one of the above enumerated types. */
NS_PRIVATE void _ns_grafter_clusters_mix_threshold
	(
   NsGrafterCluster        *cluster,
   const NsGrafterCluster  *parent,
   NsGrafter               *grafter,
	nsfloat                  curr_threshold,
	nsenum                   which
	)
	{
	nsfloat   curr_size;
	nsfloat   parent_threshold;
	nsfloat  *cluster_threshold;


   curr_size = ( nsfloat )cluster->num_values;

	/* Just to quiet the compiler. */
	parent_threshold  = 0.0f;
	cluster_threshold = NULL;

	switch( which )
		{
		case _NS_GRAFTER_TRACING_THRESHOLD:
			parent_threshold  = parent->effect_threshold;
			cluster_threshold = &cluster->effect_threshold;
			break;

		case _NS_GRAFTER_RAYBURST_THRESHOLD:
			parent_threshold  = parent->rayburst_threshold;
			cluster_threshold = &cluster->rayburst_threshold;
			break;

		default:
			ns_assert_not_reached();
		}

   /* IMPORTANT: The following formulas are used to compute the running
      threshold. It is based on the notion of mixing solutions of different
      strengths.

      __UR = Percent taken from current. 1 - __UR = is the percent taken from
      the parent.
   */
   #define __UR  ( grafter->update_rate )
   #define __S1  ( __UR * curr_size )
   #define __S2  ( ( 1.0f - __UR ) * parent->effect_size )
   #define __T1  ( curr_threshold )
   #define __T2  ( parent_threshold )
   #define __S   ( cluster->effect_size )
   #define __T   ( *cluster_threshold )

   __S = __S1 + __S2;
   ns_assert( 0.0f < __S );

   __T = (__S1*__T1)/__S + (__S2*__T2)/__S;
	}


NS_PRIVATE NsError ns_grafter_cluster_calc_effect
   (
   NsGrafterCluster        *cluster,
   const NsGrafterCluster  *parent,
   NsGrafter               *grafter
   )
   {
   nssize            bytes;
   nssize            width;
   nssize            height;
   nssize            length;
	nsdouble          inv_density;
	nssize            window;
	nssize            potential_values;
	nsfloat           probability;
   nsfloat          *values;
   nsfloat           curr_tracing_threshold;
   nsfloat           curr_rayburst_threshold;
   NsError           error;
   ThresholdStruct   s;
	//nsfloat           confidence;
	//nsfloat           contrast_minimum;
	nstimer           start;


	NS_PROFILER_BEGIN( "calc_aabbox" );
   ns_grafting_calc_aabbox(
      ns_settings_voxel_info( grafter->settings ),
      cluster->max_distance,
      &cluster->pivot,
      &grafter->max,
      grafter->use_2d_sampling,
		grafter->aabbox_scalar,
		grafter->min_window,
      &cluster->C1,
      &cluster->C2
      );

	start = ns_timer();
//	if( NS_FAILURE( _ns_grafting_calc_non_aabbox( cluster, parent, grafter ), error ) )
//		return error;
	grafter->elapsed_time[1] += ns_difftimer( ns_timer(), start );

   NS_PROFILER_END();

   NS_PROFILER_BEGIN( "make_images" );
   if( grafter->make_images )
      ns_model_graft_make_images( grafter, cluster, parent );
   NS_PROFILER_END();

   if( ns_settings_get_threshold_use_fixed( grafter->settings ) )
		{
      cluster->effect_threshold   =
		cluster->rayburst_threshold = ns_settings_get_threshold_fixed_value( grafter->settings );
		cluster->contrast           = ns_settings_get_threshold_fixed_value( grafter->settings );
		}
   else
      {
      width  = ( nssize )( cluster->C2.x - cluster->C1.x ) + 1;
      height = ( nssize )( cluster->C2.y - cluster->C1.y ) + 1;
      length = ( nssize )( cluster->C2.z - cluster->C1.z ) + 1;

      cluster->num_values = width * height * length;
      ns_assert( 0 < cluster->num_values );

		window = 0;

		if( ns_settings_get_neurite_use_random_sampling( grafter->settings ) )
			{
			inv_density = ( nsdouble )cluster->num_values / ( nsdouble )_NS_GRAFTING_NUM_SAMPLES;
			window      = inv_density < 1.0 ? 1 : ( nssize )inv_density;

			/* If the window size is greater than 1, then use random sampling. The number of random
				samples should statistically always be less than twice this constant. */
			if( 1 < window )
				cluster->num_values = 2 *_NS_GRAFTING_NUM_SAMPLES;
			}

		bytes = cluster->num_values * sizeof( nsfloat );

		NS_PROFILER_BEGIN( "alloc_buffer" );
		if( ns_byte_array_size( &grafter->threshold_values ) < bytes )
			if( NS_FAILURE( ns_byte_array_resize( &grafter->threshold_values, bytes ), error ) )
				return error;
		NS_PROFILER_END();

		values = ( nsfloat* )ns_byte_array_begin( &grafter->threshold_values );

		if( ( 1 == window ) || ( ! ns_settings_get_neurite_use_random_sampling( grafter->settings ) ) )
			{
			NS_PROFILER_BEGIN( "sample_values" );
			ns_grafting_sample_values(
				&grafter->voxel_buffer,
				values,
				cluster->num_values,
				&cluster->C1,
				&cluster->C2
				);
			NS_PROFILER_END();
			}
		else
			{
			potential_values = ( width * height * length ) / window;
			probability      = ( nsfloat )_NS_GRAFTING_NUM_SAMPLES / ( nsfloat )potential_values;

			NS_PROFILER_BEGIN( "sample_values_ex" );
			ns_grafting_sample_values_ex(
				&grafter->voxel_buffer,
				values,
				cluster->num_values,
				&cluster->C1,
				&cluster->C2,
				window,
				probability,
				&cluster->num_values
				);
			NS_PROFILER_END();
			}

		NS_PROFILER_BEGIN( "compute_threshold" );

		ComputeThreshold(
			values,
			( nsint )cluster->num_values,
			&s,
			ns_voxel_buffer_dynamic_range( &grafter->voxel_buffer ),
			NS_TRUE
			);
		/*
		ComputeDualThreshold(
			values,
			( nsint )cluster->num_values,
			&s,
			ns_voxel_buffer_dynamic_range( &grafter->voxel_buffer )
			);
ns_println( "T:%f C:%f DT:%f", s.threshold, s.contrast, s.dual_threshold );*/

		NS_PROFILER_END();

      curr_tracing_threshold  = /*( s.threshold + s.dual_threshold ) / 2.0f;//*/s.threshold;
		curr_rayburst_threshold = s.threshold;

		/* Bring the (tracing) threshold down some if the data was membrane labeled, since the
			dendrites can be hollow. Rayburst threshold is left as is. */
		if( NS_LABELING_MEMBRANE == grafter->labeling_type )
			curr_tracing_threshold = NS_MAX( 0.0f, s.threshold - s.contrast * 0.5f );

		//confidence = compute_confidence( values, width, height, length, curr_tracing_threshold );
		//ns_println( "%f %f %f", confidence, s.contrast, s.density );
		//contrast_minimum = ( nsfloat )ns_voxel_buffer_dynamic_range( &grafter->voxel_buffer ) * 0.011f;

      if( /*confidence < 0.9f || s.contrast < contrast_minimum*/ s.confidence < grafter->min_confidence )
         cluster->remove = NS_TRUE;
      else
         {
			cluster->contrast = s.contrast;

			_ns_grafter_clusters_mix_threshold(
				cluster,
				parent,
				grafter,
				curr_tracing_threshold,
				_NS_GRAFTER_TRACING_THRESHOLD
				);

			/* NOTE: Dont need to mix rayburst thresholds if the labeling type isnt membrane, since
				the tracing and rayburst thresholds are the same. This is just a small optimization. */
			if( NS_LABELING_MEMBRANE == grafter->labeling_type )
				_ns_grafter_clusters_mix_threshold(
					cluster,
					parent,
					grafter,
					curr_rayburst_threshold,
					_NS_GRAFTER_RAYBURST_THRESHOLD
					);
			else
				cluster->rayburst_threshold = cluster->effect_threshold;

			/* NOTE: Uncomment the following line to overwrite the above mixing. */
			//cluster->effect_threshold = curr_tracing_threshold;
         }
      }

   return ns_no_error();
   }


NS_PRIVATE NsError ns_model_graft_init_clustering( NsGrafter *grafter, nsboolean *did_run )
   {
   NsGrafterCluster  *cluster;
   NsVoxel           *voxel;
   NsVector3f         V;
   nsfloat            intensity;
	//nsdouble           seed_intensity;
	//nsdouble           cutoff_intensity;
   NsError            error;


	*did_run = NS_FALSE;

   /* Create one cluster and add the origin to it. */

   if( NS_FAILURE( ns_grafter_cluster_new(
                     &cluster,
							grafter,
                     ns_model_graft_next_cluster_id( grafter )
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_list_push_back( grafter->curr_clusters, cluster ), error ) )
      {
      ns_grafter_cluster_delete( cluster );
      return error;
      }

   intensity = ( nsfloat )ns_voxel_get(
                           &grafter->voxel_buffer,
                           grafter->origin.x,
                           grafter->origin.y,
                           grafter->origin.z
                           );

   if( NS_FAILURE( ns_voxel_table_add(
                     &grafter->voxel_table,
                     &grafter->origin,
                     intensity,
                     NULL,
                     &voxel
                     ),
                     error ) )
      return error;

   if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster, voxel ), error ) )
      return error;

   if( NS_FAILURE( ns_model_add_vertex( grafter->raw_model, &cluster->vertex ), error ) )
      return error;

   ns_to_voxel_space( &voxel->position, &V, ns_settings_voxel_info( grafter->settings ) );

   cluster->center = cluster->pivot = V;

   ns_model_vertex_set_position( cluster->vertex, &V );

   /* The initial threshold is the average of the seed value
      intensity and the average intensity of the whole image. *//*
   cluster->effect_threshold =
      ns_settings_get_threshold_use_fixed( grafter->settings ) ?
         ns_settings_get_threshold_fixed_value( grafter->settings ) :
         ( nsfloat )( ( ( nsdouble )voxel->intensity + grafter->average_intensity ) / 2.0 );*/

	cluster->effect_threshold   =
	cluster->rayburst_threshold = grafter->first_threshold;
	cluster->contrast           = grafter->first_contrast;
	cluster->effect_size        = 0;
	cluster->max_distance       = 0.0f;

	ns_model_vertex_set_threshold(
		cluster->vertex,
		NS_MODEL_VERTEX_NEURITE_THRESHOLD,
		grafter->first_threshold/*cluster->effect_threshold*/
		);

	ns_model_vertex_set_contrast(
		cluster->vertex,
		NS_MODEL_VERTEX_NEURITE_CONTRAST,
		grafter->first_contrast/*cluster->effect_threshold*/
		);

	ns_model_vertex_set_auto_iter( cluster->vertex );

   ns_grafter_cluster_calc_radius( cluster, grafter );

	//ns_model_vertex_set_size_ratio( cluster->vertex, cluster->size_ratio );

   cluster->num_values = 1;

   ns_assert( ns_list_is_empty( &grafter->new_voxels ) );

	ns_assert( ns_list_is_empty( &grafter->new_clusters ) );

	ns_assert( ns_list_is_empty( grafter->prev_clusters ) );
   ns_assert( 1 == ns_list_size( grafter->curr_clusters ) );
   ns_assert( ns_list_is_empty( grafter->next_clusters ) );


	/* TEMP? See if this first seed is too "dark". Current test is if its
		intensity is less than some value times the average. *//*
	seed_intensity = ( nsdouble )voxel->intensity;

	if( grafter->average_intensity < ( nsdouble )ns_voxel_buffer_convert_from_lum_u8( &grafter->voxel_buffer, 8 ) )
		cutoff_intensity = grafter->average_intensity * 8.0;
	else
		cutoff_intensity = grafter->average_intensity;
 
	ns_println( "Checking for seed intensity %f < %f", seed_intensity, cutoff_intensity );

	if( seed_intensity < cutoff_intensity )
		{
		ns_println( "TERMINATING GRAFTING: SEED TOO DARK!" );

		ns_model_remove_vertex( grafter->raw_model, cluster->vertex );
		ns_list_clear( grafter->curr_clusters );

		return ns_no_error();
		}*/


	/* Detect for intersection with the model stored in the octrees. If
		this vertex intersects anything, then remove it and kill this
		cluster by deleting it, i.e. clear the 'curr_clusters' list since
		there is only this one cluster in it. Note that the grafting will
		never run since there is no initial cluster. */
	if( _ns_grafter_cluster_vertex_intersection( grafter, cluster ) )
		{
/*TEMP*/ns_println( "TERMINATING GRAFTING: SEED INSIDE MODEL!" );
		ns_model_remove_vertex( grafter->raw_model, cluster->vertex );
		ns_list_clear( grafter->curr_clusters );

		return ns_no_error();
		}

	if( NS_FAILURE( ns_model_add_seed( grafter->raw_model, cluster->vertex, &grafter->seed ), error ) )
		{
		ns_model_remove_vertex( grafter->raw_model, cluster->vertex );
		return error;
		}

	*did_run = NS_TRUE;
   return ns_no_error();
   }


NS_PRIVATE void _ns_model_graft_remove_terminals( NsList *clusters )
   {
   nslistiter         curr, next, end;
   NsGrafterCluster  *cluster;


   curr = ns_list_begin( clusters );
   end  = ns_list_end( clusters );

   while( ns_list_iter_not_equal( curr, end ) )
      {
      next    = ns_list_iter_next( curr );
      cluster = ns_list_iter_get_object( curr );

      if( cluster->terminal )
         ns_list_erase( clusters, curr );

      curr = next;
      }
   }


/*
NS_PRIVATE void _ns_model_graft_remove_invalid_tips( NsGrafter *grafter )
	{
	nsmodelvertex curr, next, end;

	ns_model_lock( grafter->raw_model );

	curr = ns_model_begin_vertices( grafter->raw_model );
	end  = ns_model_end_vertices( grafter->raw_model );

	while( ns_model_vertex_not_equal( curr, end ) )
		{
		next = ns_model_vertex_next( curr );

		if( ! ns_model_vertex_is_external( curr ) )
			ns_model_vertex_set_size_ratio( curr, 1.0f );

		curr = next;
		}

	curr = ns_model_begin_vertices( grafter->raw_model );
	end  = ns_model_end_vertices( grafter->raw_model );

	while( ns_model_vertex_not_equal( curr, end ) )
		{
		next = ns_model_vertex_next( curr );

		if( ns_model_vertex_is_external( curr ) )
			if( ns_model_vertex_get_size_ratio( curr ) < ____ns_grafting_size_ratio )
				ns_model_remove_vertex( grafter->raw_model, curr );

		curr = next;
		}

	ns_model_unlock( grafter->raw_model );
	}
*/


/*
NS_PRIVATE void _ns_model_graft_update_seed( NsGrafter *grafter, const NsGrafterCluster *cluster )
	{  ns_model_reset_seed( grafter->raw_model, grafter->seed, cluster->vertex );  }
*/


NS_PRIVATE NsError ns_model_graft_run
	(
	NsGrafter  *grafter,
	void        ( *render_func )( void ),
	nsboolean  *did_run
	)
   {
   nslistiter         curr_cluster;
   nslistiter         new_cluster;
   nslistiter         next;
   NsGrafterCluster  *parent;
   NsGrafterCluster  *cluster;
   nslistiter         iter;
   NsVoxel           *voxel;
   NsVoxel            N;
   nssize             i;
   NsVoxel           *neighbor;
   nsfloat            intensity;
   NsGrafterShell     curr_shell;
   NsGrafterShell     next_shell;
	nsboolean          locked;
   NsError            error;
   nsint              width;
   nsint              height;
   nsint              length;


   NS_PROFILER_CLEAR();

   NS_PROFILER_BEGIN( "grafting" );

   /* ns_progress_update( grafter->progress, NS_PROGRESS_BEGIN ); */
   ns_progress_set_title( grafter->progress, "Grafting..." );

	locked = NS_FALSE;
   error  = ns_no_error();

	NS_MODEL_LOCK( grafter->raw_model );

   ns_grafter_shell_construct( &curr_shell );
   ns_grafter_shell_construct( &next_shell );

   ns_voxel_table_set_dimensions(
      &grafter->voxel_table,
      ns_voxel_buffer_width( &grafter->voxel_buffer ),
      ns_voxel_buffer_height( &grafter->voxel_buffer ),
      ns_voxel_buffer_length( &grafter->voxel_buffer )
      );

   width  = ns_voxel_buffer_width( &grafter->voxel_buffer );
   height = ns_voxel_buffer_height( &grafter->voxel_buffer );
   length = ns_voxel_buffer_length( &grafter->voxel_buffer );

   NS_PROFILER_BEGIN( "init_clustering" );
   if( NS_FAILURE( ns_model_graft_init_clustering( grafter, did_run ), error ) )
      goto _NS_MODEL_GRAFT_RUN_EXIT;
   NS_PROFILER_END();

/*TEMP*/ns_println( "Done initializing grafting." );

	NS_MODEL_UNLOCK( grafter->raw_model );

   while( ! ns_list_is_empty( grafter->curr_clusters ) )
      {
      if( ns_progress_cancelled( grafter->progress ) )
         goto _NS_MODEL_GRAFT_RUN_EXIT;

		NS_MODEL_LOCK( grafter->raw_model );

      if( grafter->make_images )
         ns_sampler_clear( &grafter->sampler );

      /* Find unseen neighbors of all the voxels in all
         the clusters. */
      NS_LIST_FOREACH( grafter->curr_clusters, curr_cluster )
         {
			if( ns_progress_cancelled( grafter->progress ) )
				goto _NS_MODEL_GRAFT_RUN_EXIT;

         parent = ns_list_iter_get_object( curr_cluster );

			ns_assert( ! parent->remove );
			ns_assert( ! parent->terminal );

         ns_assert( ns_list_is_empty( &grafter->new_voxels ) );

         NS_PROFILER_BEGIN( "get_new_voxels" );

         NS_LIST_FOREACH( parent->voxels, iter )
            {
            voxel = ns_list_iter_get_object( iter );

            for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
               {
               ns_vector3i_add( &N.position, &voxel->position, grafter->voxel_offsets + i );

               /* Make sure the position falls within the image. */
               /*if( 0 <= N.position.x && N.position.x < width  &&
                   0 <= N.position.y && N.position.y < height &&
                   0 <= N.position.z && N.position.z < length   )*/
					/* Make sure the position falls within the region of interest. */
					if( ns_point3i_inside_cube( &N.position, grafter->roi ) )
                  {
                  if( ! ns_voxel_table_exists( &grafter->voxel_table, &N.position ) )
                     {
                     intensity = ( nsfloat )ns_voxel_get(
                                             &grafter->voxel_buffer,
                                             N.position.x,
                                             N.position.y,
                                             N.position.z
                                             );

                     if( NS_FAILURE( ns_voxel_table_add(
                                       &grafter->voxel_table,
                                       &N.position,
                                       intensity,
                                       NULL,
                                       &neighbor
                                       ),
                                       error ) )
                        goto _NS_MODEL_GRAFT_RUN_EXIT;

                     if( intensity >= parent->effect_threshold )
                        if( NS_FAILURE( _ns_model_graft_add_new_voxel( grafter, neighbor ), error ) )
                           goto _NS_MODEL_GRAFT_RUN_EXIT;
                     }
                  }
               //else
               //   parent->terminal = NS_TRUE;
               }
            }

         NS_PROFILER_END();

         ns_assert( ns_list_is_empty( &grafter->new_clusters ) );

         NS_PROFILER_BEGIN( "create_new_clusters" );
         if( NS_FAILURE( ns_model_graft_create_new_clusters( grafter ), error ) )
            goto _NS_MODEL_GRAFT_RUN_EXIT;
         NS_PROFILER_END();

         ns_list_clear( &grafter->new_voxels );

         //if( parent->terminal )
         //   ns_list_clear( grafter->new_clusters );

         NS_PROFILER_BEGIN( "get_pivot_voxels" );

         NS_LIST_FOREACH( &grafter->new_clusters, new_cluster )
            {
				if( ns_progress_cancelled( grafter->progress ) )
					goto _NS_MODEL_GRAFT_RUN_EXIT;

            cluster = ns_list_iter_get_object( new_cluster );

            ns_grafter_cluster_calc_center( cluster, grafter->voxel_info );
            ns_grafter_cluster_calc_size( cluster, grafter->voxel_info );
            ns_grafter_cluster_calc_pivot( cluster, parent );
            ns_grafter_cluster_calc_max_distance( cluster, grafter );

				cluster->index = ++(grafter->cluster_index);
	
				if( cluster->index % _NS_GRAFTER_NUM_VOXEL_COLORS
						==
					 parent->index % _NS_GRAFTER_NUM_VOXEL_COLORS )
					cluster->index = ++(grafter->cluster_index);

				curr_shell.cluster_index = next_shell.cluster_index = cluster->index;

            /* Create first shell... */
            NS_LIST_FOREACH( cluster->voxels, iter )
               if( NS_FAILURE( ns_grafter_shell_add_voxel(
                                 &curr_shell,
                                 ns_list_iter_get_object( iter )
                                 ),
                                 error ) )
                  goto _NS_MODEL_GRAFT_RUN_EXIT;

            /* Add all unseen neighbors of the cluster iteratively
               until no more left within the maximum distance. */
            while( ! ns_list_is_empty( curr_shell.voxels ) )
               {
               NS_LIST_FOREACH( curr_shell.voxels, iter )
                  {
                  voxel = ns_list_iter_get_object( iter );

                  for( i = 0; i < NS_VOXEL_NUM_OFFSETS; ++i )
                     {
                     ns_vector3i_add( &N.position, &voxel->position, grafter->voxel_offsets + i );

                     /* Make sure the position falls within the image. */
                     /*if( 0 <= N.position.x && N.position.x < width  &&
                         0 <= N.position.y && N.position.y < height &&
                         0 <= N.position.z && N.position.z < length   )*/
							/* Make sure the position falls within the region of interest. */
							if( ns_point3i_inside_cube( &N.position, grafter->roi ) )
                        {
                        if( ns_model_graft_voxel_distance_to_cluster( &N, cluster, grafter )
                            <= cluster->max_distance_sq )
                           if( ! ns_voxel_table_exists( &grafter->voxel_table, &N.position ) )
                              {
                              intensity = ( nsfloat )ns_voxel_get(
                                                      &grafter->voxel_buffer,
                                                      N.position.x,
                                                      N.position.y,
                                                      N.position.z
                                                      );

                              if( NS_FAILURE( ns_voxel_table_add(
                                                &grafter->voxel_table,
                                                &N.position,
                                                intensity,
                                                NULL,
                                                &neighbor
                                                ),
                                                error ) )
                                 goto _NS_MODEL_GRAFT_RUN_EXIT;

                              if( intensity >= parent->effect_threshold )
                                 {
                                 if( NS_FAILURE( ns_grafter_cluster_add_voxel( cluster, neighbor ), error ) )
                                    goto _NS_MODEL_GRAFT_RUN_EXIT;

                                 if( NS_FAILURE( ns_grafter_shell_add_voxel( &next_shell, neighbor ), error ) )
                                    goto _NS_MODEL_GRAFT_RUN_EXIT;
                                 }
                              }
                        }
                     else
                        {
                        /* This is to allow 2D grafting to work. Z dimension cannot be
                           allowed to terminate the front since all 26 neighbors are being
                           checked not just the 8 that exist in 2D. */
                        /*if( N.position.x < 0 || N.position.x >= width ||
                            N.position.y < 0 || N.position.y >= height   )*/
								if( N.position.x < grafter->roi->C1.x || N.position.x > grafter->roi->C2.x ||
									 N.position.y < grafter->roi->C1.y || N.position.y > grafter->roi->C2.y   )
                           if( ns_model_graft_voxel_distance_to_cluster( &N, cluster, grafter )
                               <= cluster->max_distance_sq )
                              cluster->terminal = NS_TRUE;
                        }
                     }
                  }

               ns_list_clear( curr_shell.voxels );
               NS_SWAP( NsList*, curr_shell.voxels, next_shell.voxels );
               }
            }

         NS_PROFILER_END();

         NS_PROFILER_BEGIN( "calc_thresholds" );

         /* For each of the current clusters, calculate a threshold value.
            Create a collection of voxels that have neighbors in the
            background... and those background neighbors as well. */

         new_cluster = ns_list_begin( &grafter->new_clusters );

         while( ns_list_iter_not_equal( new_cluster, ns_list_end( &grafter->new_clusters ) ) )
            {
            next    = ns_list_iter_next( new_cluster );
            cluster = ns_list_iter_get_object( new_cluster );

            /* NOTE: Dont use dynamic thresholding until we have more than
               one "front" expanding. */
            if( grafter->have_split )
               {
               if( NS_FAILURE( ns_grafter_cluster_calc_effect(
                                 cluster,
                                 parent,
                                 grafter
                                 ),
                                 error ) )
                  goto _NS_MODEL_GRAFT_RUN_EXIT;

					/* For testing purposes only!
					cluster->effect_threshold = 128;
					cluster->contrast         = parent->contrast;
               cluster->effect_size      = 0;
					*/
               }
            else
               {
               cluster->effect_threshold   = parent->effect_threshold;
               cluster->rayburst_threshold = parent->rayburst_threshold;
					cluster->contrast           = parent->contrast;
               cluster->effect_size        = 0;
               }

            if( cluster->remove )
               {
               if( grafter->make_images )
                  ns_sampler_remove( &grafter->sampler, cluster->sample );

               ns_list_erase( &grafter->new_clusters, new_cluster );
               }

            new_cluster = next;
            }

         NS_PROFILER_END();

         NS_PROFILER_BEGIN( "create_vertices" );
         if( NS_FAILURE( ns_model_graft_new_clusters_to_model( grafter, parent ), error ) )
            goto _NS_MODEL_GRAFT_RUN_EXIT;
         NS_PROFILER_END();

         _ns_model_graft_remove_terminals( &grafter->new_clusters );

         /* For the next iteration... */
         ns_list_append( grafter->next_clusters, &grafter->new_clusters );
         }

		/* Keep updating the seed location until we split. */
		//if( ! grafter->have_split && 1 == ns_list_size( grafter->next_clusters ) )
		//	_ns_model_graft_update_seed( grafter, ns_list_iter_get_object( ns_list_begin( grafter->next_clusters ) ) );

		ns_list_clear( grafter->prev_clusters );

      if( 1 < ns_list_size( grafter->next_clusters ) )
         grafter->have_split = NS_TRUE;
		
		NS_SWAP( NsList*, grafter->prev_clusters, grafter->curr_clusters );
		ns_assert( ns_list_is_empty( grafter->curr_clusters ) );

      NS_SWAP( NsList*, grafter->curr_clusters, grafter->next_clusters );
		ns_assert( ns_list_is_empty( grafter->next_clusters ) );

		NS_MODEL_UNLOCK( grafter->raw_model );

      NS_PROFILER_BEGIN( "render" );
      if( NULL != render_func )
         ( render_func )();
      NS_PROFILER_END();
      }

   /* ns_progress_update( grafter->progress, NS_PROGRESS_END ); */

	NS_MODEL_LOCK( grafter->raw_model );

	ns_list_clear( grafter->prev_clusters );

   ns_assert( ns_list_is_empty( &grafter->new_voxels ) );

   ns_assert( ns_list_is_empty( &grafter->new_clusters ) );

	ns_assert( ns_list_is_empty( grafter->prev_clusters ) );
   ns_assert( ns_list_is_empty( grafter->curr_clusters ) );
   ns_assert( ns_list_is_empty( grafter->next_clusters ) );

	NS_MODEL_UNLOCK( grafter->raw_model );

   _NS_MODEL_GRAFT_RUN_EXIT:

	NS_MODEL_TRY_UNLOCK( grafter->raw_model );

   ns_grafter_shell_destruct( &curr_shell );
   ns_grafter_shell_destruct( &next_shell );

   NS_PROFILER_END();

   NS_PROFILER_SHELL();

	//if( ! ns_is_error( error ) )
	//	_ns_model_graft_remove_invalid_tips( grafter );

   return error;
   }


NS_PRIVATE NsAABBox3d* _ns_grafter_calc_aabbox( NsGrafter *grafter, NsAABBox3d *bbox )
	{
	NsVector3i min3i, max3i;
	NsVector3f min3f, max3f;


	ns_vector3i_zero( &min3i );

	ns_vector3i(
		&max3i,
		ns_voxel_buffer_width( &grafter->voxel_buffer ),
		ns_voxel_buffer_height( &grafter->voxel_buffer ),
		ns_voxel_buffer_length( &grafter->voxel_buffer )
		);

	ns_to_voxel_space( &min3i, &min3f, grafter->voxel_info );
	ns_to_voxel_space( &max3i, &max3f, grafter->voxel_info );

	ns_model_voxel_bounds_to_aabbox( &min3f, &max3f, bbox );

	return bbox;
	}


NS_PRIVATE NsError _ns_grafter_build_edge_octree( NsGrafter *grafter )
	{
	NsAABBox3d bbox;

	/* TEMP: Hardcode parameters? */
	return ns_model_build_edge_octree(
				grafter->filtered_model,
				_ns_grafter_calc_aabbox( grafter, &bbox ),
				16,
				5,
				32,
				NULL,
				grafter->progress
				);
	}


/* NOTE: Only need to store those vertices that have no edges in the vertex
	octree. The edge octree allows detecting all other intersections. */
NS_PRIVATE nsboolean _ns_grafter_filter_octree_vertex( nsmodelvertex vertex )
	{  return 0 == ns_model_vertex_num_edges( vertex );  }


NS_PRIVATE NsError _ns_grafter_build_vertex_octree( NsGrafter *grafter )
	{
	NsAABBox3d bbox;

	/* TEMP: Hardcode parameters? */
	return ns_model_build_vertex_octree(
				grafter->filtered_model,
				_ns_grafter_calc_aabbox( grafter, &bbox ),
				16,
				5,
				32,
				_ns_grafter_filter_octree_vertex,
				grafter->progress
				);
	}


NS_PRIVATE NsError _ns_grafter_build_octrees( NsGrafter *grafter )
	{
   NsError error;

	ns_model_lock( grafter->filtered_model );

	ns_model_clear_octrees( grafter->filtered_model );

   if( NS_FAILURE( _ns_grafter_build_edge_octree( grafter ), error ) )
		{
		ns_model_unlock( grafter->filtered_model );
      return error;
		}

   if( NS_FAILURE( _ns_grafter_build_vertex_octree( grafter ), error ) )
		{
		ns_model_unlock( grafter->filtered_model );
      return error;
		}

	/*TEMP*/
	ns_println( "\nEDGE OCTREE size = " NS_FMT_ULONG " VERTEX OCTREE size = " NS_FMT_ULONG,
		ns_model_octree_size( grafter->filtered_model, NS_MODEL_EDGE_OCTREE ),
		ns_model_octree_size( grafter->filtered_model, NS_MODEL_VERTEX_OCTREE ) );
	ns_println( "\nGRAFTING! Please wait..." );

	ns_model_unlock( grafter->filtered_model );
   return ns_no_error();
	}


NsError ns_model_graft
   (
   NsModel               *raw_model,
	NsModel               *filtered_model,
   const NsSettings      *settings,
   const NsImage         *image,
	const NsCubei         *roi,
   nsboolean              use_proj_xy_as_image,
   const NsImage         *proj_xy,
   const NsImage         *proj_zy,
   const NsImage         *proj_xz,
	nsfloat                first_threshold,
	nsfloat                first_contrast,
   NsRayburstInterpType   interp_type,
   nsdouble               average_intensity,
   nsfloat                min_confidence,
   nsfloat                update_rate,
   nsboolean              make_images,
   nsboolean              use_2d_sampling,
	nsfloat                aabbox_scalar,
	nsint                  min_window,
   NsProgress            *progress,
   void                   ( *render_func )( void ),
	NsLabelingType         labeling_type,
	NsResidualSmearType    residual_smear_type,
	nsboolean             *did_run
   )
   {
   NsVoxel  voxel;
	nstimer  start;
   NsError  error;


   ns_assert( NULL != raw_model );
   ns_assert( NULL != raw_model->grafter );
	ns_assert( NULL != filtered_model );
   ns_assert( NULL != settings );
   ns_assert( NULL != image );
	ns_assert( NULL != roi );
   ns_assert( ! ( use_proj_xy_as_image && NULL == proj_xy ) );
   ns_assert( NULL != progress );
	ns_assert( NULL != did_run );

	/* First see if the seed is in the region of interest. */
	if( ! ns_point3i_inside_cube( ns_settings_neurite_seed( settings ), roi ) )
		return ns_no_error();

   ns_log_entry(
		NS_LOG_ENTRY_FUNCTION,
		NS_FUNCTION
		"( raw_model=" NS_FMT_STRING_DOUBLE_QUOTED
		", filtered_model=" NS_FMT_STRING_DOUBLE_QUOTED
		", settings=" NS_FMT_POINTER
		", image=" NS_FMT_POINTER
		", roi=" NS_FMT_POINTER
		", use_proj_xy_as_image=" NS_FMT_INT
		", proj_xy=" NS_FMT_POINTER
		", proj_zy=" NS_FMT_POINTER
		", proj_xz=" NS_FMT_POINTER
		", first_threshold=" NS_FMT_DOUBLE
		", first_contrast=" NS_FMT_DOUBLE
		", interp_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", average_intensity=" NS_FMT_DOUBLE
		", min_confidence=" NS_FMT_DOUBLE
		", update_rate=" NS_FMT_DOUBLE
		", make_images=" NS_FMT_INT
		", use_2d_sampling=" NS_FMT_INT
		", progress=" NS_FMT_POINTER
		", render_func=" NS_FMT_POINTER
		", labeling_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", residual_smear_type=" NS_FMT_STRING_DOUBLE_QUOTED
		", did_run=" NS_FMT_POINTER
		" )",
		ns_model_get_name( raw_model ),
		ns_model_get_name( filtered_model ),
		settings,
		image,
		roi,
		use_proj_xy_as_image,
		proj_xy,
		proj_zy,
		proj_xz,
		first_threshold,
		first_contrast,
		ns_rayburst_interp_type_to_string( interp_type ),
		average_intensity,
		min_confidence,
		update_rate,
		make_images,
		use_2d_sampling,
		progress,
		render_func,
		ns_labeling_type_to_string( labeling_type ),
		ns_residual_smear_type_to_string( residual_smear_type ),
		did_run
		);

   error = ns_no_error();

	ns_model_lock( raw_model );

   if( NS_FAILURE(
			ns_grafter_init(
            raw_model->grafter,
            raw_model,
				filtered_model,
            settings,
            image,
				roi,
            use_proj_xy_as_image,
            proj_xy,
            proj_zy,
            proj_xz,
				first_threshold,
				first_contrast,
            interp_type,
            average_intensity,
            min_confidence,
            update_rate,
            make_images,
            use_2d_sampling,
				aabbox_scalar,
				min_window,
				labeling_type,
				residual_smear_type,
            progress
            ),
			error ) )
		{
		ns_model_unlock( raw_model );
      return error;
		}

   voxel.position = raw_model->grafter->origin;

   raw_model->grafter->is_running = NS_TRUE;

	ns_model_unlock( raw_model );

   if( NULL != render_func )
      ( render_func )();

	ns_model_lock( raw_model );

	if( NS_FAILURE( _ns_grafter_build_octrees( raw_model->grafter ), error ) )
		{
		ns_model_unlock( raw_model );
		return error;
		}

	ns_model_unlock( raw_model );

	ns_model_needs_edge_centers( filtered_model );

	ns_model_auto_iter_up( raw_model );
	ns_assert( 0 < ns_model_auto_iter( raw_model ) );

	start = ns_timer();
   error = ns_model_graft_run( raw_model->grafter, render_func, did_run );
	raw_model->grafter->elapsed_time[0] += ns_difftimer( ns_timer(), start );

	ns_println( "grafter->elapsed_time[0] = " NS_FMT_DOUBLE, raw_model->grafter->elapsed_time[0] );
	ns_println( "grafter->elapsed_time[1] = " NS_FMT_DOUBLE, raw_model->grafter->elapsed_time[1] );

	ns_println(
		"No. of voxels processed per second = " NS_FMT_DOUBLE,
		( nsdouble )ns_voxel_table_size( &(raw_model->grafter->voxel_table) ) / raw_model->grafter->elapsed_time[0]
		);

	/* if( ns_is_error( error ) )
		 ns_model_auto_iter_down( raw_model ); */

	ns_model_lock( raw_model );

   ns_grafter_finalize( raw_model->grafter );
   raw_model->grafter->is_running = NS_FALSE;

	ns_model_unlock( raw_model );

   return error;
   }


nsmodelvertex ns_model_grafter_begin_vertices( const NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->grafter );

   return model->grafter->prev_junction;
   }


NS_PRIVATE void _ns_grafter_render_non_aabbox
	(
	const NsGrafter  *grafter,
	NsRenderState    *state
	)
	{
	nslistiter               iter;
	const NsGrafterCluster  *cluster;


   ns_assert( NULL != grafter );
   ns_assert( NULL != state );
   ns_assert( NULL != NS_RENDER( state, line_func ) );

	state->variables.color = NS_COLOR4UB_WHITE;
	state->variables.shape = NS_RENDER_SHAPE_LINE;

	NS_RENDER_PRE_OR_POST( state, pre_lines_func );

	NS_LIST_FOREACH( grafter->curr_clusters, iter )
		{
		cluster = ns_list_iter_get_object( iter );

		if( NULL != cluster->corners )
			{
			NS_RENDER( state, line_func )( cluster->corners + 0, cluster->corners + 1, state );
			NS_RENDER( state, line_func )( cluster->corners + 2, cluster->corners + 3, state );
			NS_RENDER( state, line_func )( cluster->corners + 0, cluster->corners + 2, state );
			NS_RENDER( state, line_func )( cluster->corners + 1, cluster->corners + 3, state );
			NS_RENDER( state, line_func )( cluster->corners + 4, cluster->corners + 5, state );
			NS_RENDER( state, line_func )( cluster->corners + 6, cluster->corners + 7, state );
			NS_RENDER( state, line_func )( cluster->corners + 4, cluster->corners + 6, state );
			NS_RENDER( state, line_func )( cluster->corners + 5, cluster->corners + 7, state );
			NS_RENDER( state, line_func )( cluster->corners + 0, cluster->corners + 4, state );
			NS_RENDER( state, line_func )( cluster->corners + 1, cluster->corners + 5, state );
			NS_RENDER( state, line_func )( cluster->corners + 2, cluster->corners + 6, state );
			NS_RENDER( state, line_func )( cluster->corners + 3, cluster->corners + 7, state );
			}
		}

	NS_RENDER_PRE_OR_POST( state, post_lines_func );
	}


NS_PRIVATE const NsColor4ub* _ns_grafter_voxel_colors( void )
   {
   NS_PRIVATE nsboolean  ____ns_grafter_voxel_colors_init = NS_FALSE;
   NS_PRIVATE NsColor4ub ____ns_grafter_voxel_colors[ _NS_GRAFTER_NUM_VOXEL_COLORS ];

   if( ! ____ns_grafter_voxel_colors_init )
      {
      ____ns_grafter_voxel_colors[0] = NS_COLOR4UB_RED;
      ____ns_grafter_voxel_colors[1] = NS_COLOR4UB_ORANGE;
      ____ns_grafter_voxel_colors[2] = NS_COLOR4UB_YELLOW;
      ____ns_grafter_voxel_colors[3] = NS_COLOR4UB_GREEN;
      ____ns_grafter_voxel_colors[4] = NS_COLOR4UB_BLUE;
      ____ns_grafter_voxel_colors[5] = NS_COLOR4UB_CYAN;//NS_COLOR4UB_INDIGO;
      ____ns_grafter_voxel_colors[6] = NS_COLOR4UB_VIOLET;

      ____ns_grafter_voxel_colors_init = NS_TRUE;
      }

   return ____ns_grafter_voxel_colors;
   }


NS_PRIVATE _ns_model_render_grafter_voxels_as_points
   (
   const NsModel  *model,
   NsRenderState  *state
   )
	{
	const NsVoxelTable  *voxel_table;
	const NsColor4ub    *voxel_colors;
	NsVoxel             *voxel;
	NsVector3f           V;
   NsVector3b           N;


   ns_assert( NULL != NS_RENDER( state, point_func ) );

	voxel_table  = &(model->grafter->voxel_table);
	voxel_colors = _ns_grafter_voxel_colors();

   /* Just create a "dummy" normal for the voxels. */
   N.x = 1; N.y = 0; N.z = 0;

   state->variables.shape = NS_RENDER_SHAPE_POINT;
   NS_RENDER_PRE_OR_POST( state, pre_points_func );

	NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )
		if( _ns_grafting_get_was_shelled( voxel ) )
			{
			state->variables.color = voxel_colors[ _ns_grafter_voxel_get_color( voxel ) ];

			ns_to_voxel_space( &voxel->position, &V, state->constants.voxel_info );
			NS_RENDER( state, point_func )( &N, &V, state );
			}

   NS_RENDER_PRE_OR_POST( state, post_points_func );
	}


NS_PRIVATE _ns_model_render_grafter_voxels_as_rectangles
   (
   const NsModel  *model,
   NsRenderState  *state
   )
	{
	const NsVoxelTable  *voxel_table;
	const NsColor4ub    *voxel_colors;
	NsVoxel             *voxel;
	nsboolean            polygon_borders;
	nsboolean            soft_corners;
	NsVector3f           V1, V2;


   ns_assert( NULL != NS_RENDER( state, rectangle_func ) );

	voxel_table  = &(model->grafter->voxel_table);
	voxel_colors = _ns_grafter_voxel_colors();

	polygon_borders = state->constants.polygon_borders;
	soft_corners    = state->constants.spines_soft_corners;

	state->constants.polygon_borders     = NS_FALSE;
	//state->constants.spines_soft_corners = NS_TRUE;

	state->variables.shape = NS_RENDER_SHAPE_RECTANGLE;
	NS_RENDER_PRE_OR_POST( state, pre_rectangles_func );

	NS_VOXEL_TABLE_FOREACH( voxel_table, voxel )
		if( _ns_grafting_get_was_shelled( voxel ) )
			{
			state->variables.color = voxel_colors[ _ns_grafter_voxel_get_color( voxel ) ];

			ns_to_voxel_space( &voxel->position, &V1, state->constants.voxel_info );

			V1.x -= ( ns_voxel_info_size_x( state->constants.voxel_info ) / 2.0f );
			V1.y -= ( ns_voxel_info_size_y( state->constants.voxel_info ) / 2.0f );
			V1.z -= ( ns_voxel_info_size_z( state->constants.voxel_info ) / 2.0f );

			ns_vector3f_add( &V2, &V1, ns_voxel_info_size( state->constants.voxel_info ) );

			NS_RENDER( state, rectangle_func )( &V1, &V2, state );
			}

	NS_RENDER_PRE_OR_POST( state, post_rectangles_func );

	state->constants.polygon_borders     = polygon_borders;
	state->constants.spines_soft_corners = soft_corners;
	}


void ns_model_render_grafter
   (
   const NsModel  *model,
   NsRenderState  *state
   )
   {
	ns_assert( NULL != model );
	ns_assert( NULL != model->grafter );
	ns_assert( NULL != state );

	//_ns_model_render_grafter_voxels_as_points( model, state );
	//_ns_model_render_grafter_voxels_as_rectangles( model, state );

   //ns_sampler_render_images( &(model->grafter->sampler), state );
   ns_sampler_render_borders( &(model->grafter->sampler), state );

	//_ns_grafter_render_non_aabbox( model->grafter, state );
   }


NsError ns_grafting_establish_thresholds_and_contrasts
   (
   const NsSettings  *settings,
   const NsImage     *image,
	const NsCubei     *roi,
   nsboolean          use_2d_sampling,
	nsfloat            aabbox_scalar,
	nsint              min_window,
   NsModel           *model,
   NsProgress        *progress
   )
   {
	const NsVoxelInfo  *voxel_info;
   NsVoxelBuffer       voxel_buffer;
   NsByteArray         buffer;
   ThresholdStruct     s;
   nsmodelvertex       curr;
   nsmodelvertex       end;
   NsVector3f          Pf;
	NsVector3i          Pi;
   nsfloat             radius;
	nsint               image_width, image_height, image_length;
   nssize              sample_width, sample_height, sample_length;
   nssize              num_values;
   nsfloat            *values;
   nssize              bytes;
   NsVector3i          C1, C2;
   NsVector3i          max;
	NsAABBox3d          roi_box;
	NsAABBox3d          vertex_box;
   NsError             error;


   ns_assert( NULL != settings );
   ns_assert( NULL != image );
   ns_assert( NULL != model );
   ns_assert( NULL != progress );

	if( 0 < ns_model_get_thresh_count( model ) )
		return ns_no_error();

   ns_progress_set_title( progress, "Establishing vertex thresholds and contrasts..." );
   ns_progress_num_iters( progress, ns_model_num_vertices( model ) );

	voxel_info = ns_settings_voxel_info( settings );

	if( NULL != roi )
		ns_model_voxel_bounds_to_aabbox_ex( &roi->C1, &roi->C2, voxel_info, &roi_box );

   ns_voxel_buffer_init( &voxel_buffer, image );

   ns_vector3i(
      &max,
      ns_voxel_buffer_width( &voxel_buffer ) - 1,
      ns_voxel_buffer_height( &voxel_buffer ) - 1,
      ns_voxel_buffer_length( &voxel_buffer ) - 1
      );

	image_width  = ( nsint )ns_image_width( image );
	image_height = ( nsint )ns_image_height( image );
	image_length = ( nsint )ns_image_length( image );

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &buffer, 0 ), error ) );

   ns_progress_begin( progress );

   curr = ns_model_begin_vertices( model );
   end  = ns_model_end_vertices( model );

   for( ; ns_model_vertex_not_equal( curr, end ); curr = ns_model_vertex_next( curr ) )
      {
      if( ns_settings_get_threshold_use_fixed( settings ) )
			{
         ns_model_vertex_set_threshold(
            curr,
            NS_MODEL_VERTEX_NEURITE_THRESHOLD,
            ns_settings_get_threshold_fixed_value( settings )
            );

			ns_model_vertex_set_contrast(
				curr,
				NS_MODEL_VERTEX_NEURITE_CONTRAST,
				ns_settings_get_threshold_fixed_value( settings )
				);
			}
      else
         {
         ns_model_vertex_get_position( curr, &Pf );
			ns_to_image_space( &Pf, &Pi, voxel_info );

			/* Check for out of bounds vertex. */
			if( Pi.x < 0 || Pi.x >= image_width  ||
				 Pi.y < 0 || Pi.y >= image_height ||
				 Pi.z < 0 || Pi.z >= image_length )
				{
				/* Set the threshold so "high" that no voxel intensity would never be greater. */
				ns_model_vertex_set_threshold( curr, NS_MODEL_VERTEX_NEURITE_THRESHOLD, NS_FLOAT_MAX );

				/* Set the contrast really low? */
				ns_model_vertex_set_contrast( curr, NS_MODEL_VERTEX_NEURITE_CONTRAST, 0.0f );

				ns_progress_next_iter( progress );
				continue;
				}

         radius = ns_model_vertex_get_radius( curr );

			ns_grafting_calc_aabbox(
            voxel_info,
            radius,
            &Pf,
            &max,
            use_2d_sampling,
				aabbox_scalar,
				min_window,
            &C1,
            &C2
            );

			if( NULL != roi )
				{
				ns_model_voxel_bounds_to_aabbox_ex( &C1, &C2, voxel_info, &vertex_box );

				if( ! ns_aabbox3d_intersects_aabbox( &vertex_box, &roi_box ) )
					{
					ns_progress_next_iter( progress );
					continue;
					}
				}

         sample_width  = ( nssize )( C2.x - C1.x ) + 1;
         sample_height = ( nssize )( C2.y - C1.y ) + 1;
         sample_length = ( nssize )( C2.z - C1.z ) + 1;

         num_values = sample_width * sample_height * sample_length;

         ns_assert( 0 < num_values );
         bytes = num_values * sizeof( nsfloat );

         if( ns_byte_array_size( &buffer ) < bytes )
            if( NS_FAILURE( ns_byte_array_resize( &buffer, bytes ), error ) )
               {
               ns_byte_array_destruct( &buffer );
               return error;
               }

         values = ( nsfloat* )ns_byte_array_begin( &buffer );

         ns_grafting_sample_values( &voxel_buffer, values, num_values, &C1, &C2 );

         ComputeThreshold(
            values,
            ( nsint )num_values,
            &s,
            ns_voxel_buffer_dynamic_range( &voxel_buffer ),
				NS_TRUE
            );

	      ns_model_vertex_set_threshold( curr, NS_MODEL_VERTEX_NEURITE_THRESHOLD, s.threshold );
			ns_model_vertex_set_contrast( curr, NS_MODEL_VERTEX_NEURITE_CONTRAST, s.contrast );
         }

      ns_progress_next_iter( progress );
      }

   ns_progress_end( progress );

   ns_byte_array_destruct( &buffer );

	ns_model_up_thresh_count( model );

   return ns_no_error();
   }


#include <std/nsfile.h>

NsError ns_grafting_output_sample_values
   (
   const nsfloat  *values,
   nssize          num_values,
   const nschar   *file_name
   )
   {
   NsFile   file;
   nssize   i;
   NsError  error;


   ns_assert( NULL != values );
   ns_assert( NULL != file_name );

   error = ns_no_error();

   ns_file_construct( &file );

   if( NS_FAILURE( ns_file_open( &file, file_name, NS_FILE_MODE_WRITE ), error ) )
      return error;

   for( i = 0; i < num_values; ++i )
      if( NS_FAILURE( ns_file_print(
                        &file,
                        NS_FMT_DOUBLE
                        NS_STRING_NEWLINE,
                        values[i]
                        ),
                        error ) )
         break;

   ns_file_destruct( &file );
   return error;
   }


NS_PRIVATE NsError _ns_grafter_do_construct( NsGrafter *grafter )
   {
   NsError error;

	if( NS_FAILURE( ns_voxel_table_construct( &grafter->voxel_table, NULL ), error ) )
		return error;

   ns_verify( NS_SUCCESS( ns_byte_array_construct( &grafter->threshold_values, 0 ), error ) );

   ns_list_construct( &grafter->new_voxels, _ns_grafting_clear_is_new_voxel );

   ns_list_construct( &grafter->new_clusters, ns_grafter_cluster_delete );

	grafter->prev_clusters = grafter->lists + 0;
   grafter->curr_clusters = grafter->lists + 1;
   grafter->next_clusters = grafter->lists + 2;

	ns_list_construct( grafter->prev_clusters, ns_grafter_cluster_delete );
   ns_list_construct( grafter->curr_clusters, ns_grafter_cluster_delete );
   ns_list_construct( grafter->next_clusters, ns_grafter_cluster_delete );

   ns_sampler_construct( &grafter->sampler );

   grafter->voxel_offsets = ns_voxel_offsets();

	return ns_no_error();
   }


NS_PRIVATE void _ns_grafter_do_destruct( NsGrafter *grafter )
   {
   ns_grafter_clear( grafter );

	ns_voxel_table_destruct( &grafter->voxel_table );

   ns_byte_array_destruct( &grafter->threshold_values );

   ns_list_destruct( &grafter->new_voxels );

   ns_list_destruct( &grafter->new_clusters );

	ns_list_destruct( grafter->prev_clusters );
   ns_list_destruct( grafter->curr_clusters );
   ns_list_destruct( grafter->next_clusters );

   ns_sampler_destruct( &grafter->sampler );
   }


NsError ns_model_new_grafter( NsModel *model )
   {
	NsError error;

   ns_assert( NULL != model );
   ns_assert( NULL == model->grafter );

   if( NULL == ( model->grafter = ns_new0( NsGrafter ) ) )
      return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   if( NS_FAILURE( _ns_grafter_do_construct( model->grafter ), error ) )
		{
		ns_delete( model->grafter );
		model->grafter = NULL;

		return error;
		}

   return ns_no_error();
   }


void ns_model_delete_grafter( NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->grafter );

   _ns_grafter_do_destruct( model->grafter );

   ns_delete( model->grafter );
   model->grafter = NULL;
   }


nsboolean ns_model_grafter_is_running( const NsModel *model )
   {
   ns_assert( NULL != model );
   ns_assert( NULL != model->grafter );

   return model->grafter->is_running;
   }
