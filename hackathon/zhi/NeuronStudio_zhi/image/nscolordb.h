#ifndef __NS_IMAGE_COLOR_DB_H__
#define __NS_IMAGE_COLOR_DB_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nshashtable.h>
#include <image/nscolor.h>

NS_DECLS_BEGIN

typedef struct _NsColorDb
	{
	NsHashTable  entries;
	}
	NsColorDb;


NS_IMPEXP NsError ns_color_db_construct( NsColorDb *db );
NS_IMPEXP void ns_color_db_destruct( NsColorDb *db );

NS_IMPEXP void ns_color_db_clear( NsColorDb *db );

NS_IMPEXP nssize ns_color_db_size( const NsColorDb *db );
NS_IMPEXP nsboolean ns_color_db_is_empty( const NsColorDb *db );

/* Map the color to the object. The user can provide a
	destructor for the object or NULL if not needed. */
NS_IMPEXP NsError ns_color_db_add
	(
	NsColorDb       *db,
	nsuint           color,
	nspointer        object,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP nsboolean ns_color_db_remove( NsColorDb *db, nsuint color );

NS_IMPEXP nsboolean ns_color_db_exists( const NsColorDb *db, nsuint color );
NS_IMPEXP nsboolean ns_color_db_lookup( const NsColorDb *db, nsuint color, nspointer *object );


/* Returns NULL if not found. */
NS_IMPEXP nspointer ns_color_db_get( NsColorDb *db, nsuint color );

/* Ignores error. */
NS_IMPEXP void ns_color_db_set
	(
	NsColorDb       *db,
	nsuint           color,
	nspointer        object,
	NsFinalizeFunc   finalize_func
	);

NS_DECLS_END

#endif/* __NS_IMAGE_COLOR_DB_H__ */
