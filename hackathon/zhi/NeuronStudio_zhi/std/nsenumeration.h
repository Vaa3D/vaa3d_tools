#ifndef __NS_STD_ENUMERATION_H__
#define __NS_STD_ENUMERATION_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>

NS_DECLS_BEGIN

typedef struct _NsEnumeration
	{
	nsboolean  ( *has_more_elements )( const struct _NsEnumeration* );
	nspointer  ( *next_element )( struct _NsEnumeration* );
	nspointer  user_data;
	}
	NsEnumeration;


NS_IMPEXP void ns_enumeration_construct( NsEnumeration *e );
NS_IMPEXP void ns_enumeration_destruct( NsEnumeration *e );

NS_IMPEXP nsboolean ns_enumeration_has_more_elements( const NsEnumeration *e );

NS_IMPEXP nspointer ns_enumeration_next_element( NsEnumeration *e );

NS_IMPEXP nspointer ns_enumeration_get_user_data( const NsEnumeration *e );
NS_IMPEXP void ns_enumeration_set_user_data( NsEnumeration *e, nspointer user_data );

#define ns_enumeration_get_has_more_elements( e )\
	( (e)->has_more_elements ) )
#define ns_enumeration_set_has_more_elements( e, func )\
	( (e)->has_more_elements = (func) )

#define ns_enumeration_get_next_element( e )\
	( (e)->next_element ) )
#define ns_enumeration_set_next_element( e, func )\
	( (e)->next_element = (func) )

NS_DECLS_END

#endif/* __NS_STD_ENUMERATION_H__ */
