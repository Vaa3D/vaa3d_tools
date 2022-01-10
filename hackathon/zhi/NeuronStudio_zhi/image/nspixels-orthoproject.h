#ifndef __NS_IMAGE_PIXELS_ORTHO_PROJECT_H__
#define __NS_IMAGE_PIXELS_ORTHO_PROJECT_H__

#include <image/nspixels.h>

NS_DECLS_BEGIN

enum
   {
   NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXEL_TYPE,
   NS_PIXEL_PROC_ORTHO_PROJECT_SRC_PIXELS,
   NS_PIXEL_PROC_ORTHO_PROJECT_WIDTH,
   NS_PIXEL_PROC_ORTHO_PROJECT_HEIGHT,
   NS_PIXEL_PROC_ORTHO_PROJECT_LENGTH,
   NS_PIXEL_PROC_ORTHO_PROJECT_SRC_ROW_ALIGN,
   NS_PIXEL_PROC_ORTHO_PROJECT_PROJECT_TYPE,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXEL_TYPE,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XY,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XY,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_ZY,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_ZY,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_PIXELS_XZ,
   NS_PIXEL_PROC_ORTHO_PROJECT_DEST_ROW_ALIGN_XZ,
   NS_PIXEL_PROC_ORTHO_PROJECT_REGION,
	NS_PIXEL_PROC_ORTHO_PROJECT_Z_INDICES,
	NS_PIXEL_PROC_ORTHO_PROJECT_NUM_Z_INDICES,
   NS_PIXEL_PROC_ORTHO_PROJECT_AVERAGE_INTENSITY,
   NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PIXELS,
	NS_PIXEL_PROC_ORTHO_PROJECT_USE_SRC_SIZE,
   NS_PIXEL_PROC_ORTHO_PROJECT_PROGRESS,

   NS_PIXEL_PROC_ORTHO_PROJECT_NUM_PARAMS
   };

enum
	{
	NS_PIXEL_PROC_ORTHO_PROJECT_MAX,
	NS_PIXEL_PROC_ORTHO_PROJECT_MIN,
	NS_PIXEL_PROC_ORTHO_PROJECT_SUM,

	_NS_PIXEL_PROC_ORTHO_PROJECT_NUM_TYPES /* Don't use! */
	};


NS_IMPEXP const nschar* ns_pixel_proc_ortho_project_type_to_string( nsenum project_type );

NS_IMPEXP NsProc* ns_pixel_proc_ortho_project( void );


/* TEMP */
NS_IMPEXP NsError _ns_pixel_proc_ortho_project_marshal
   (
   NsClosure  *closure, /* not used */
   NsValue    *params,
   nssize      num_params, /* not used */
   NsValue    *ret_value /* not used */
   );

NS_DECLS_END

#endif/* __NS_IMAGE_PIXELS_ORTHO_PROJECT_H__ */
