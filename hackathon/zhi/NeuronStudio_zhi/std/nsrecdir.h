#ifndef __NS_STD_REC_DIR_H__
#define __NS_STD_REC_DIR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsdir.h>
#include <std/nsstack.h>
#include <std/nsstring.h>
#include <std/nsutil.h>
#include <std/nsascii.h>

NS_DECLS_BEGIN

typedef struct _NsRecDir
	{
	NsStack    stack;
	nsboolean  recursed;
	}
	NsRecDir;


NS_IMPEXP void ns_rec_dir_construct( NsRecDir *rdir );
NS_IMPEXP void ns_rec_dir_destruct( NsRecDir *rdir );

NS_IMPEXP nsboolean ns_rec_dir_is_open( const NsRecDir *rdir );

/* NOTE: When closed, depth is 0. */
NS_IMPEXP nssize ns_rec_dir_depth( const NsRecDir *rdir );

/* Working directory must be set before calling. */
NS_IMPEXP NsError ns_rec_dir_open( NsRecDir *rdir );

NS_IMPEXP void ns_rec_dir_close( NsRecDir *rdir );

NS_IMPEXP NsError ns_rec_dir_rewind( NsRecDir *rdir );

/* Advances to beginning of next directory. */
NS_IMPEXP NsError ns_rec_dir_advance( NsRecDir *rdir, const nschar **name );

NS_IMPEXP NsError ns_rec_dir_read( NsRecDir *rdir, const nschar **name );
NS_IMPEXP NsError ns_rec_dir_read_ex( NsRecDir *rdir, const nschar **name, nsboolean *is_dir );

/* Returns NULL if never opened. */
NS_IMPEXP const NsDir* ns_rec_dir_curr( const NsRecDir *rdir );

NS_IMPEXP NsError ns_rec_dir_curr_full_path( const NsRecDir *rdir, NsString *path );
NS_IMPEXP NsError ns_rec_dir_curr_relative_path( const NsRecDir *rdir, NsString *path );

NS_DECLS_END

#endif/* __NS_STD_REC_DIR_H__ */
