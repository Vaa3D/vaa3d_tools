#ifndef __NS_IMAGE_IMAGE_DIR_H__
#define __NS_IMAGE_IMAGE_DIR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nslist.h>
#include <std/nsascii.h>
#include <std/nsdir.h>
#include <std/nslog.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsImageDir
	{
	NsList  entries;
	nssize  prefix_length;
	}
	NsImageDir;

typedef nslistiter nsimagediriter;


NS_IMPEXP void ns_image_dir_construct( NsImageDir *dir );
NS_IMPEXP void ns_image_dir_destruct( NsImageDir *dir );

/* NOTE: Current directory must be set before calling. */
NS_IMPEXP NsError ns_image_dir_create
	(
	NsImageDir    *dir,
	const nschar  *template_file,
	nsboolean      case_sensitive,
	nsboolean      detect_sequence,
	nsboolean     *any_problems
	);

NS_IMPEXP nssize ns_image_dir_size( const NsImageDir *dir );

NS_IMPEXP nssize ns_image_dir_prefix_length( const NsImageDir *dir );

#define  ns_image_dir_is_empty( dir )\
	( 0 == ns_image_dir_size( (dir) ) )

NS_IMPEXP void ns_image_dir_clear( NsImageDir *dir );

NS_IMPEXP const nschar* ns_image_dir_first_file( const NsImageDir *dir );
NS_IMPEXP const nschar* ns_image_dir_last_file( const NsImageDir *dir );

NS_IMPEXP nsimagediriter ns_image_dir_begin( const NsImageDir *dir );
NS_IMPEXP nsimagediriter ns_image_dir_end( const NsImageDir *dir );

#define ns_image_dir_iter_next( I )\
	ns_list_iter_next( (I) )

#define ns_image_dir_iter_prev( I )\
	ns_list_iter_prev( (I) )

#define ns_image_dir_iter_equal( I1, I2 )\
	ns_list_iter_equal( (I1), (I2) )

#define ns_image_dir_iter_offset( I, n )\
	ns_list_iter_offset( (I), (n) )

#define ns_image_dir_iter_not_equal( I1, I2 )\
	ns_list_iter_not_equal( (I1), (I2) )

NS_IMPEXP const nschar* ns_image_dir_iter_file( const nsimagediriter I );

NS_DECLS_END

#endif/* __NS_IMAGE_IMAGE_DIR_H__ */
