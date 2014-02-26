#include "nsvoxel-info.h"


void ns_voxel_info
   (
   NsVoxelInfo  *voxel_info,
   nsfloat       size_x,
   nsfloat       size_y,
   nsfloat       size_z
   )
   {
   ns_vector3f( &voxel_info->size, size_x, size_y, size_z );

   ns_vector3f(
      &voxel_info->one_over_size,
      1.0f / size_x,
      1.0f / size_y,
      1.0f / size_z
      );

   voxel_info->min_size = NS_MIN3( size_x, size_y, size_z );
   voxel_info->max_size = NS_MAX3( size_x, size_y, size_z );

   voxel_info->volume = size_x * size_y * size_z;

	voxel_info->surface_area =
		( 2.0f * size_x * size_y ) +
		( 2.0f * size_x * size_z ) +
		( 2.0f * size_y * size_z );

   voxel_info->min_radius = voxel_info->min_size / 2.0f;
   voxel_info->max_radius = voxel_info->max_size / 2.0f;

   voxel_info->tolerance = voxel_info->min_size / 100.0f;

   voxel_info->max_size_xy = NS_MAX( size_x, size_y );
   voxel_info->max_size_zy = NS_MAX( size_z, size_y );
   voxel_info->max_size_xz = NS_MAX( size_x, size_z );

	voxel_info->diag_length = ns_sqrtf( NS_POW2( size_x ) + NS_POW2( size_y ) + NS_POW2( size_z ) );
   }


void ____ns_to_voxel_space
   (
   const NsPoint3i    *in,
   NsPoint3f          *out,
   const NsVoxelInfo  *voxel_info,
   const nschar       *details
   )
   {
	NS_USE_VARIABLE( details );

   out->x = ( nsfloat )in->x * voxel_info->size.x;
   out->y = ( nsfloat )in->y * voxel_info->size.y;
   out->z = ( nsfloat )in->z * voxel_info->size.z;
   }


void ____ns_to_image_space
   (
   const NsPoint3f    *in,
   NsPoint3i          *out,
   const NsVoxelInfo  *voxel_info,
   const nschar       *details
   )
   {
   nsfloat    f32;
   nsint32    i32;
   nsfloat64  f64;


	NS_USE_VARIABLE( details );

/*
#ifdef NS_DEBUG_EXTREME
   ns_assert_with_details( 0.0f <= in->x, details );
#endif
*/
   f32 = in->x * voxel_info->one_over_size.x + .5f;
   NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( f32, i32, f64 );
/*
#ifdef NS_DEBUG_EXTREME
   if( ! ( ( nsint32 )NS_INT_MIN <= i32 && i32 <= ( nsint32 )NS_INT_MAX ) )
      ns_println( __FUNCTION__ "::x " NS_FMT_DOUBLE " to " NS_FMT_INT, f32, i32 );
#endif
*/
   out->x = ( nsint )i32;


/*
#ifdef NS_DEBUG_EXTREME
   ns_assert_with_details( 0.0f <= in->y, details );
#endif
*/
   f32 = in->y * voxel_info->one_over_size.y + .5f;
   NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( f32, i32, f64 );
/*
#ifdef NS_DEBUG_EXTREME
   if( ! ( ( nsint32 )NS_INT_MIN <= i32 && i32 <= ( nsint32 )NS_INT_MAX ) )
      ns_println( __FUNCTION__ "::y " NS_FMT_DOUBLE " to " NS_FMT_INT, f32, i32 );
#endif
*/
   out->y = ( nsint )i32;


/*
#ifdef NS_DEBUG_EXTREME
   ns_assert_with_details( 0.0f <= in->z, details );
#endif
*/
   f32 = in->z * voxel_info->one_over_size.z + .5f;
   NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( f32, i32, f64 );
/*
#ifdef NS_DEBUG_EXTREME
   if( ! ( ( nsint32 )NS_INT_MIN <= i32 && i32 <= ( nsint32 )NS_INT_MAX ) )
      ns_println( __FUNCTION__ "::z " NS_FMT_DOUBLE " to " NS_FMT_INT, f32, i32 );
#endif
*/
   out->z = ( nsint )i32;
   }


nsfloat ____ns_to_voxel_space_component
   (
   nsint               in,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
   const nschar       *details
   )
   {
   nsfloat out;

	NS_USE_VARIABLE( details );

	out = 0.0f;

   switch( which )
      {
      case NS_COMPONENT_X:
         out = ( nsfloat )in * voxel_info->size.x;
         break;

      case NS_COMPONENT_Y:
         out = ( nsfloat )in * voxel_info->size.y;
         break;

      case NS_COMPONENT_Z:
         out = ( nsfloat )in * voxel_info->size.z;
         break;

      default:
         ns_assert_not_reached();
      }

   return out;
   }


nsint ____ns_to_image_space_component
   (
   nsfloat             in,
   const NsVoxelInfo  *voxel_info,
   nsint               which,
   const nschar       *details
   )
   {
   nsfloat    f32;
   nsint32    i32;
   nsfloat64  f64;
   nsint      out;


   ns_assert_with_details( 0.0f <= in, details );

	f32 = 0.0f;

   switch( which )
      {
      case NS_COMPONENT_X:
         f32 = in * voxel_info->one_over_size.x + .5f;
         break;

      case NS_COMPONENT_Y:
         f32 = in * voxel_info->one_over_size.y + .5f;
         break;

      case NS_COMPONENT_Z:
         f32 = in * voxel_info->one_over_size.z + .5f;
         break;

      default:
         ns_assert_not_reached();
      }

   NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( f32, i32, f64 );
/*
#ifdef NS_DEBUG_EXTREME
   if( ! ( ( nsint32 )NS_INT_MIN <= i32 && i32 <= ( nsint32 )NS_INT_MAX ) )
      ns_println( __FUNCTION__ "::" NS_FMT_INT " " NS_FMT_DOUBLE " to " NS_FMT_INT, which, f32, i32 );
#endif
*/
   out = ( nsint )i32;

   return out;
   }


void ns_to_voxel_space_ex
   (
   const NsPoint3f    *in,
   NsPoint3f          *out,
   const NsVoxelInfo  *voxel_info
   )
   {
   out->x = in->x * voxel_info->size.x;
   out->y = in->y * voxel_info->size.y;
   out->z = in->z * voxel_info->size.z;
   }


void ns_to_image_space_ex
   (
   const NsPoint3f    *in,
   NsPoint3f          *out,
   const NsVoxelInfo  *voxel_info
   )
   {
   out->x = in->x * voxel_info->one_over_size.x;
   out->y = in->y * voxel_info->one_over_size.y;
   out->z = in->z * voxel_info->one_over_size.z;
   }


nsfloat ns_to_voxel_space_component_ex
	(
	nsfloat             in,
	const NsVoxelInfo  *voxel_info,
	nsint               which
	)
	{
   nsfloat out = 0.0f;

   switch( which )
      {
      case NS_COMPONENT_X:
         out = in * voxel_info->size.x;
         break;

      case NS_COMPONENT_Y:
         out = in * voxel_info->size.y;
         break;

      case NS_COMPONENT_Z:
         out = in * voxel_info->size.z;
         break;

      default:
         ns_assert_not_reached();
      }

   return out;
	}


nsfloat ns_to_image_space_component_ex
	(
	nsfloat             in,
	const NsVoxelInfo  *voxel_info,
	nsint               which
	)
	{
   nsfloat out = 0.0f;

   switch( which )
      {
      case NS_COMPONENT_X:
         out = in * voxel_info->one_over_size.x;
         break;

      case NS_COMPONENT_Y:
         out = in * voxel_info->one_over_size.y;
         break;

      case NS_COMPONENT_Z:
         out = in * voxel_info->one_over_size.z;
         break;

      default:
         ns_assert_not_reached();
      }

   return out;
	}


void ns_client_to_voxel_space
	(
	const NsPoint2i    *in,
	NsPoint3f          *out,
	const NsVoxelInfo  *voxel_info,
	nsint               view,
	const NsPoint2i    *corner,
	nsfloat             zoom
	)
	{
	NsVector3f  V;
	NsVector2f  I;


	/* Client space to image space (using floating point numbers). Note that
		the center of a pixel is considered the exact coordinate in image space.*/
   I.x = ( nsfloat )corner->x + ( ( nsfloat )in->x ) / zoom - 0.5f;
   I.y = ( nsfloat )corner->y + ( ( nsfloat )in->y ) / zoom - 0.5f;

	/* Get the image space coordinate in 3D. */
   switch( view )
      {
      case NS_XY:
         V.x = I.x;
         V.y = I.y;
         V.z = 0.0f;
         break;

      case NS_ZY:
         V.z = I.x;
         V.y = I.y;
         V.x = 0.0f;
         break;

      case NS_XZ:
         V.x = I.x;
         V.z = I.y;
         V.y = 0.0f;
         break;

      default:
         ns_assert_not_reached();
      }

	/* Convert image space to voxel space (using floating point numbers). */
	ns_to_voxel_space_ex( &V, out, voxel_info );
	}
