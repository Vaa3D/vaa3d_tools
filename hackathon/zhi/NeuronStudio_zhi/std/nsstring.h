#ifndef __NS_STD_STRING_H__
#define __NS_STD_STRING_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsprint.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsbytearray.h>
#include <std/nsascii.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef struct _NsString
	{
	NsByteArray  buffer;
	nssize       size;
	}
	NsString;


NS_IMPEXP void ns_string_construct( NsString *string );
NS_IMPEXP NsError ns_string_construct_init( NsString *string, const nschar *s );
NS_IMPEXP NsError ns_string_construct_sized( NsString *string, nssize size );

NS_IMPEXP void ns_string_destruct( NsString *string );

NS_IMPEXP void ns_string_clear( NsString *string );

/* Returns 0 == ns_string_size() */
NS_IMPEXP nsboolean ns_string_is_null( const NsString *string );

NS_IMPEXP const nschar* ns_string_get( const NsString *string );
NS_IMPEXP NsError ns_string_set( NsString *string, const nschar *s );

/* Sets the string to the empty string, i.e. "" */
NS_IMPEXP NsError ns_string_empty( NsString *string );

/* NOTE: 'size' is in bytes. */
NS_IMPEXP NsError ns_string_resize( NsString *string, nssize size );

/* The number of bytes the characters, including the
	NULL terminator, occupy. ( is always <= capacity ) */
NS_IMPEXP nssize ns_string_size( const NsString *string );

/* The actual size of the character buffer in bytes. */
NS_IMPEXP nssize ns_string_capacity( const NsString *string );

/* The number of characters. */
NS_IMPEXP nssize ns_string_length( const NsString *string );

NS_IMPEXP NsError ns_string_append( NsString *string, const nschar *s );
NS_IMPEXP NsError ns_string_append_c( NsString *string, nsunichar c );

NS_IMPEXP NsError ns_string_prepend( NsString *string, const nschar *s );
NS_IMPEXP NsError ns_string_prepend_c( NsString *string, nsunichar c );

NS_IMPEXP NsError ns_string_insert( NsString *string, nssize at, const nschar *s );
NS_IMPEXP NsError ns_string_insert_c( NsString *string, nssize at, nsunichar c );

NS_IMPEXP void ns_string_delete( NsString *string, nssize at, nssize count );
NS_IMPEXP void ns_string_delete_c( NsString *string, nssize at );

/* Inserts the character BEFORE the object pointed to by the passed pointer.
	NOTE: Pointers can become invalid after call! */
NS_IMPEXP NsError ns_string_splice_in_c( NsString *string, nschar *I, nsunichar c );
NS_IMPEXP void ns_string_splice_out_c( NsString *string, nschar *I );

NS_IMPEXP NsError ns_string_splice_out( NsString *src, nssize at, nssize count, NsString *dest );

NS_IMPEXP void ns_string_upper( NsString *string );
NS_IMPEXP void ns_string_lower( NsString *string );

NS_IMPEXP nsuint ns_string_hash( const NsString *string );

NS_IMPEXP NsError ns_string_getcwd( NsString *string );

/* NOTE: The string is resized if necessary to hold the formatted string. */
NS_IMPEXP NsError ns_string_vformat( NsString *string, const nschar *format, ns_va_list args );
NS_IMPEXP NsError ns_string_format( NsString *string, const nschar *format, ... );

NS_IMPEXP NsError ns_string_vformat_and_append( NsString *string, const nschar *format, ns_va_list args );
NS_IMPEXP NsError ns_string_format_and_append( NsString *string, const nschar *format, ... );

NS_IMPEXP nssize ns_string_distance( const NsString *string, const nschar *I );

NS_IMPEXP nschar* ns_string_begin( const NsString *string );
NS_IMPEXP nschar* ns_string_end( const NsString *string );
NS_IMPEXP nschar* ns_string_at( const NsString *string, nssize index );

NS_IMPEXP nsunichar ns_string_getc( const NsString *string, nssize at );
NS_IMPEXP void ns_string_setc( NsString *string, nssize at, nsunichar c );

/* Returns NULL if not found. */
NS_IMPEXP nschar* ns_string_find( const NsString *string, const nschar *s );
NS_IMPEXP nschar* ns_string_find_c( const NsString *string, nsunichar c );

/* Looks for sub-string 's' starting at the character at the 'T'. 
	Returns NULL if not found. */
NS_IMPEXP const nschar* ns_string_find_from( const NsString *string, const nschar *at, const nschar *s );
NS_IMPEXP const nschar* ns_string_find_c_from( const NsString *string, const nschar *at, nsunichar c );

NS_DECLS_END

#endif/* __NS_STD_STRING_H__ */
