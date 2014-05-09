#ifndef __NS_STD_PROGRESS_H__
#define __NS_STD_PROGRESS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsvalue.h>
#include <std/nsmath.h>
#include <std/nsthread.h>

NS_DECLS_BEGIN

/* NOTE: All functions are optional, i.e. can be NULL. */
typedef struct _NsProgress
	{
	nsboolean  ( *cancelled )( struct _NsProgress* );
	void       ( *update )( struct _NsProgress*, nsfloat percent );
	void       ( *set_title )( struct _NsProgress*, const nschar *title );
	void       ( *set_description )( struct _NsProgress*, const nschar *description );
	nsfloat    last_percent;
	nsfloat    curr_percent;
	nsfloat    num_iters;
	nsfloat    curr_iter;
	nspointer  user_data;
	}
	NsProgress;


extern NsProgress ns_progress_null;


/* Pass as 'percent' value. Just for readability. */
#define NS_PROGRESS_BEGIN    0.0f
#define NS_PROGRESS_END    100.0f


/* Pass NULL for 'set_title' and 'set_description' if not needed. */
NS_IMPEXP NsProgress* ns_progress
	(
	NsProgress  *progress,
	nsboolean   ( *cancelled )( NsProgress *progress ),
	void        ( *update )( NsProgress *progress, nsfloat percent ),
	void        ( *set_title )( NsProgress *progress, const nschar *title ),
	void        ( *set_description )( NsProgress *progress, const nschar *description ),
	nspointer   user_data
	);
	
NS_IMPEXP nsboolean ns_progress_cancelled( const NsProgress *progress );

NS_IMPEXP void ns_progress_begin( NsProgress *progress );
NS_IMPEXP void ns_progress_end( NsProgress *progress );

NS_IMPEXP void ns_progress_update( NsProgress *progress, nsfloat percent );

NS_IMPEXP void ns_progress_num_iters( NsProgress *progress, nssize num_iters );
NS_IMPEXP void ns_progress_next_iter( NsProgress *progress );

NS_IMPEXP void ns_progress_set_title( NsProgress *progress, const nschar *title );
NS_IMPEXP void ns_progress_set_description( NsProgress *progress, const nschar *description );

NS_IMPEXP nspointer ns_progress_get_user_data( const NsProgress *progress );
NS_IMPEXP void ns_progress_set_user_data( NsProgress *progress, nspointer user_data );


#define NS_VALUE_PROGRESS  "NsProgress*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_progress, NsProgress* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_progress, NsProgress* );
NS_IMPEXP NS_VALUE_ARG_DECLARE( ns_value_arg_progress, NsProgress* );
NS_IMPEXP NsError ns_value_register_progress( void );

NS_DECLS_END

#endif/* __NS_STD_PROGRESS_H__ */
