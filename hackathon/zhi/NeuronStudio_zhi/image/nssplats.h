#ifndef __NS_IMAGE_SPLATS_H__
#define __NS_IMAGE_SPLATS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsprogress.h>
#include <math/nsvector.h>
#include <math/nscube.h>
#include <math/nssphere.h>
#include <math/nsray.h>
#include <image/nsimage.h>
#include <image/nsvoxel-info.h>
#include <image/nsvoxel-buffer.h>
#include <image/nsvoxel-table.h>
#include <image/nscolor.h>
#include <ext/normal.h>

NS_DECLS_BEGIN

typedef struct tagSPLATDEF
	{
	NsVector3i  V;
	NsVector3b  N;
	nsuint8     intensity;
	nsshort     color;
	}
	SPLATDEF;


typedef struct _NsSplatsOutline
	{
	const NsImage     *proj;
	const NsVector3i  *voxel_corner_offsets;
	nsulong            threshold;
	nsint              width;
	nsint              height;
	NsVoxelBuffer      buffer;
	NsList             pixels;
	}
	NsSplatsOutline;


NS_IMPEXP void ns_splats_outline_construct( NsSplatsOutline *outline );
NS_IMPEXP void ns_splats_outline_destruct( NsSplatsOutline *outline );

/* Convert ns_list_iter_get_object() to an nsulong, which is a pixel offset,
	i.e., and X and Y location into the outline's projection.

	The X and Y location can be retrived as such...

	pixel = ns_list_iter_get_object( I );
	y     = pixel / ns_image_width( proj );
	x     = pixel % ns_image_width( proj );
*/
NS_IMPEXP nslistiter ns_splats_outline_begin( const NsSplatsOutline *outline );
NS_IMPEXP nslistiter ns_splats_outline_end( const NsSplatsOutline *outline );


/* A user-defined callback function that given an xyz location
	will set the 'color' parameter to some value, that is if the
	function returns true. NOTE: A 'color' value of zero is
	interpreted as using the current global color scheme for
	the splats. */
typedef nsboolean ( *NsSplatsColorFunc )
	(
	nspointer          user_data,
	const NsVector3i  *V,
	nsshort           *color
	);


typedef struct _NsSplats
	{
	nspointer          voxels_xy;
	nspointer          voxels_xz;
	nspointer          voxels_zy;
	nssize             size;
	nsulong            threshold;
	nsuint32          *palette;
	const NsVector3i  *voxel_corner_offsets;
	NsFunc             create_func;
	nsboolean          is_volume;
	NsVoxelBuffer      voxels;
	NsCubei            roi;
	NsVoxelBuffer      proj_xy;
	NsVoxelBuffer      proj_zy;
	NsVoxelBuffer      proj_xz;
	NsSplatsColorFunc  color_func;
	nspointer          user_data;
	NsVector3f         colors[ NS_COLOR4UB_TABLE_SIZE ];
	nsint              width;
	nsint              height;
	nsint              length;
	nsint              slice;
	}
	NsSplats;


NS_IMPEXP void ns_splats_construct( NsSplats *splats );
NS_IMPEXP void ns_splats_destruct( NsSplats *splats );

NS_IMPEXP void ns_splats_clear( NsSplats *splats );


/* Values for ns_splats_create...() 'which' parameter. */
enum
	{
	NS_SPLATS_CREATE_IMAGE,
	NS_SPLATS_CREATE_PROJ_AVERAGE,
	NS_SPLATS_CREATE_PROJ_MAX,
	NS_SPLATS_CREATE_PROJ_MIN,

	_NS_SPLATS_CREATE_NUM_TYPES /* Internal. DO NOT USE! */
	};


/* If 'is_volume' is true, then all voxels above the 'threshold'	
	are turned into a splat, instead of just boundary ones.

	IMPORTANT: 'threshold' is offset by the 'contrast' parameter
	to arrive at the cutoff value.

	'outline' can be NULL if not necessary.
*/
NS_IMPEXP NsError ns_splats_create
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
	);


NS_IMPEXP NsError ns_splats_create_ex
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
	);


/* Same as the above create functions except that only the
	voxels in the table are turned into splats. Voxels that
	are along the surface are determinned by having a neighbor
	that is not in the table, instead of by threshold... unless
	of course 'is_volume' is set to true. */
NS_IMPEXP NsError ns_splats_create_by_table
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
	);


#define NS_SPLATS_RENDER_ALPHA     0x001
#define NS_SPLATS_RENDER_PALETTE   0x002
#define NS_SPLATS_RENDER_LIGHTING  0x004
#define NS_SPLATS_RENDER_OPTIMIZE  0x008

/* Values for ns_splats_render() 'type'. */
enum
	{
	NS_SPLATS_RENDER_TEXTURES,
	NS_SPLATS_RENDER_CUBES,

	_NS_SPLATS_RENDER_NUM_TYPES /* Internal. DO NOT USE! */
	};

/* If 'clip_to_slice' is true then no voxels are drawn with a
	Z coordinate less than the splats slice value. */
NS_IMPEXP void ns_splats_render
	(
	const NsSplats     *splats,
	const NsVoxelInfo  *voxel_info,
	nsulong             flags,
	const NsVector4ub  *palette, /* Must be of size 256. */
	const NsVector3f   *color,   /* Used if NS_SPLATS_RENDER_PALETTE not set. */
	nsfloat             alpha,   /* Used if NS_SPLATS_RENDER_ALPHA is set. */
	nsint               type,
	nsboolean           clip_to_slice
	);


/* If the passed 'slice' is equal to the 'splats'
	current slice then the function does nothing. */
NS_IMPEXP void ns_splats_set_at_slice( NsSplats *splats, nsint slice );
NS_IMPEXP void ns_splats_unset_at_slice( NsSplats *splats );


/* Sets the closest splat (as the point 'S' with 'intensity')
	that intersects the ray 'R', if any. */
NS_IMPEXP nsboolean ns_splats_find_closest_by_ray
	(
	const NsSplats     *splats,
	const NsVoxelInfo  *voxel_info,
	NsRay3d            *R,
	NsPoint3d          *S,
	NsProgress         *progress
	); 

NS_DECLS_END

#endif/* __NS_IMAGE_SPLATS_H__ */
