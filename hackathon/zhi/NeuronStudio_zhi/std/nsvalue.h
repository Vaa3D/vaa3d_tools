#ifndef __NS_STD_VALUE_H__
#define __NS_STD_VALUE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nshashtable.h>
#include <std/nsascii.h>
#include <std/nsmutex.h>
#include <std/nscookie.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

NS_IMPEXP NS_INTERNAL NS_STATIC_MUTEX_DECLARE( ns_value_static_mutex );


/* Internal. DO NOT USE! */
NS_IMPEXP NS_INTERNAL NsError _ns_value_db_init( void );
NS_IMPEXP NS_INTERNAL void _ns_value_db_finalize( void );


#define NS_VALUE_MAX_SIZE  16

typedef struct _NsValue
	{
	const nschar  *name;
	nsuint8  v[
		NS_VALUE_MAX_SIZE
		#ifdef NS_DEBUG
		+ sizeof( nscookie )
		#endif
		];
	}
	NsValue;


NS_IMPEXP NS_SYNCHRONIZED NsError ns_value_register
	(
	const nschar  *name,
	nssize         size,
	void          ( *reset )( NsValue* ),
	void          ( *unset )( NsValue* ),
	void          ( *arg )( NsValue*, ns_va_list* )
	);

/* See primitive types below. */
NS_IMPEXP NsError ns_value_register_std( void );

NS_IMPEXP NS_SYNCHRONIZED nssize ns_value_registry_size( void );

NS_IMPEXP NS_SYNCHRONIZED void ns_value_unregister( const nschar *name );

NS_IMPEXP NS_SYNCHRONIZED nsboolean ns_value_is_registered( const nschar *name );
NS_IMPEXP nsboolean ns_value_is_registered_unsync( const nschar *name );

/* Warning! Not a thread safe. Cast return type of
	ns_enumeration_next_element() to a const nschar*. */
NS_IMPEXP NsEnumeration* ns_value_registry_enum( NsEnumeration *enumeration );


/* The type of the value is set to NS_VALUE_UNKNOWN. */
NS_IMPEXP void ns_value_construct( NsValue *value );
NS_IMPEXP void ns_value_construct_type( NsValue *value, const nschar *name );

NS_IMPEXP void ns_value_destruct( NsValue *value );

NS_IMPEXP nsboolean ns_value_equal( const NsValue *v1, const NsValue *v2 );

NS_IMPEXP void ns_value_assign( NsValue *dest, const NsValue *src );

/* These get/set functions always retrieve or set a pointer to the
	actual type, e.g. nsint* or EList**. See macros below for defining
	get/set functions for a particular type. */
NS_IMPEXP nspointer ns_value_get( const NsValue *value );

NS_IMPEXP NsValue* ns_value_set( NsValue *value, const nschar *name, nspointer v );
NS_IMPEXP NsValue* ns_value_set_unsync( NsValue *value, const nschar *name, nspointer v );

NS_IMPEXP NsValue* ns_value_vset( NsValue *value, const nschar *name, ns_va_list *args );

NS_IMPEXP nsboolean ns_value_is( const NsValue *value, const nschar *name );

NS_IMPEXP const nschar* ns_value_get_type( const NsValue *value );
NS_IMPEXP NsValue* ns_value_set_type( NsValue *value, const nschar *name );

NS_IMPEXP NsValue* ns_value_reset( NsValue *value );

NS_IMPEXP NsValue* ns_value_unset( NsValue *value );
NS_IMPEXP NsValue* ns_value_unset_unsync( NsValue *value );

/* Value is set to all bits zero. Careful with this one. */
NS_IMPEXP NsValue* ns_value_memzero( NsValue *value );


#define NS_VALUE_GET_DECLARE( name, type )\
NS_IMPEXP type name( const NsValue* )

#define NS_VALUE_SET_DECLARE( name, type )\
NS_IMPEXP NsValue* name( NsValue*, type )

#define NS_VALUE_ARG_DECLARE( name, type )\
NS_IMPEXP void name( NsValue*, ns_va_list* )


#define NS_VALUE_GET_IMPLEMENT( name, type )\
type name( const NsValue *value )\
	{\
	ns_assert( NULL != value );\
	return *( ( type* )ns_value_get( value ) );\
	}

#define NS_VALUE_SET_IMPLEMENT( name, type )\
NsValue* name( NsValue *value, type v )\
	{\
	ns_assert( NULL != value );\
	return ns_value_set( value, NS_STRINGIZE( type ), ( nspointer )&v );\
	}

/* NOTE: This function is already called within a locked section
	of code (by a mutex). Therefore we cant call ns_value_set() since
	this also calls for a lock, i.e. a lock is obtained more than once
	in a single thread. */
#define NS_VALUE_ARG_IMPLEMENT( name, type )\
void name( NsValue *value, ns_va_list *args )\
	{\
	type v;\
	ns_assert( NULL != value );\
	ns_assert( NULL != args );\
	v = ns_va_arg( *args, type );\
	ns_value_set_unsync( value, NS_STRINGIZE( type ), ( nspointer )&v );\
	}


#define NS_VALUE_UNKNOWN "<unknown>"

#define NS_VALUE_VOID "void"

#define NS_VALUE_CHAR  "nschar"
#define NS_VALUE_PCHAR "nschar*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_char,  nschar );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_char,  nschar );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pchar, nschar* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pchar, nschar* );

#define NS_VALUE_UCHAR  "nsuchar"
#define NS_VALUE_PUCHAR "nsuchar*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uchar,  nsuchar  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uchar,  nsuchar  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puchar, nsuchar* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puchar, nsuchar* );

#define NS_VALUE_SHORT  "nsshort"
#define NS_VALUE_PSHORT "nsshort*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_short,  nsshort  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_short,  nsshort  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pshort, nsshort* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pshort, nsshort* );

#define NS_VALUE_USHORT  "nsushort"
#define NS_VALUE_PUSHORT "nsushort*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_ushort,  nsushort  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_ushort,  nsushort  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pushort, nsushort* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pushort, nsushort* );

#define NS_VALUE_INT  "nsint"
#define NS_VALUE_PINT "nsint*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_int,  nsint  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_int,  nsint  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pint, nsint* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pint, nsint* );

#define NS_VALUE_UINT  "nsuint"
#define NS_VALUE_PUINT "nsuint*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uint,  nsuint  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uint,  nsuint  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puint, nsuint* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puint, nsuint* );

#define NS_VALUE_LONG  "nslong"
#define NS_VALUE_PLONG "nslong*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_long,  nslong  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_long,  nslong  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_plong, nslong* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_plong, nslong* );

#define NS_VALUE_ULONG  "nsulong"
#define NS_VALUE_PULONG "nsulong*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_ulong,  nsulong  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_ulong,  nsulong  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pulong, nsulong* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pulong, nsulong* );

#define NS_VALUE_FLOAT  "nsfloat"
#define NS_VALUE_PFLOAT "nsfloat*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_float,  nsfloat  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_float,  nsfloat  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pfloat, nsfloat* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pfloat, nsfloat* );

#define NS_VALUE_DOUBLE  "nsdouble"
#define NS_VALUE_PDOUBLE "nsdouble*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_double,  nsdouble  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_double,  nsdouble  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pdouble, nsdouble* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pdouble, nsdouble* );

#define NS_VALUE_POINTER "nspointer"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pointer, nspointer );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pointer, nspointer );

#define NS_VALUE_CONSTPOINTER "nsconstpointer"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_constpointer, nsconstpointer );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_constpointer, nsconstpointer );

#define NS_VALUE_BOOLEAN  "nsboolean"
#define NS_VALUE_PBOOLEAN "nsboolean*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_boolean,  nsboolean  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_boolean,  nsboolean  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pboolean, nsboolean* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pboolean, nsboolean* );

#define NS_VALUE_SIZE  "nssize"
#define NS_VALUE_PSIZE "nssize*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_size,  nssize  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_size,  nssize  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_psize, nssize* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_psize, nssize* );

#define NS_VALUE_ENUM  "nsenum"
#define NS_VALUE_PENUM "nsenum*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_enum,  nsenum  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_enum,  nsenum  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_penum, nsenum* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_penum, nsenum* );

#ifdef NS_HAVE_LONGLONG
	#define NS_VALUE_LONGLONG  "nslonglong"
	#define NS_VALUE_PLONGLONG "nslonglong*"
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_longlong,  nslonglong  );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_longlong,  nslonglong  );
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_plonglong, nslonglong* );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_plonglong, nslonglong* );

	#define NS_VALUE_ULONGLONG  "nsulonglong"
	#define NS_VALUE_PULONGLONG "nsulonglong*"
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_ulonglong,  nsulonglong  );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_ulonglong,  nsulonglong  );
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pulonglong, nsulonglong* );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pulonglong, nsulonglong* );
#endif

#define NS_VALUE_INT8  "nsint8"
#define NS_VALUE_PINT8 "nsint8*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_int8,  nsint8  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_int8,  nsint8  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pint8, nsint8* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pint8, nsint8* );

#define NS_VALUE_UINT8  "nsuint8"
#define NS_VALUE_PUINT8 "nsuint8*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uint8,  nsuint8  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uint8,  nsuint8  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puint8, nsuint8* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puint8, nsuint8* );

#define NS_VALUE_INT16  "nsint16"
#define NS_VALUE_PINT16 "nsint16*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_int16,  nsint16  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_int16,  nsint16  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pint16, nsint16* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pint16, nsint16* );

#define NS_VALUE_UINT16  "nsuint16"
#define NS_VALUE_PUINT16 "nsuint16*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uint16,  nsuint16  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uint16,  nsuint16  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puint16, nsuint16* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puint16, nsuint16* );

#define NS_VALUE_INT32  "nsint32"
#define NS_VALUE_PINT32 "nsint32*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_int32,  nsint32  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_int32,  nsint32  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pint32, nsint32* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pint32, nsint32* );

#define NS_VALUE_UINT32  "nsuint32"
#define NS_VALUE_PUINT32 "nsuint32*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uint32,  nsuint32  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uint32,  nsuint32  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puint32, nsuint32* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puint32, nsuint32* );

#define NS_VALUE_FLOAT32  "nsfloat32"
#define NS_VALUE_PFLOAT32 "nsfloat32*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_float32,  nsfloat32  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_float32,  nsfloat32  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pfloat32, nsfloat32* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pfloat32, nsfloat32* );

#define NS_VALUE_FLOAT64  "nsfloat64"
#define NS_VALUE_PFLOAT64 "nsfloat64*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_float64,  nsfloat64  );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_float64,  nsfloat64  );
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pfloat64, nsfloat64* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pfloat64, nsfloat64* );

#ifdef NS_HAVE_INT64
	#define NS_VALUE_INT64  "nsint64"
	#define NS_VALUE_PINT64 "nsint64*"
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_int64,  nsint64  );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_int64,  nsint64  );
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_pint64, nsint64* );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_pint64, nsint64* );

	#define NS_VALUE_UINT64  "nsuint64"
	#define NS_VALUE_PUINT64 "nsuint64*"
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_uint64,  nsuint64  );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_uint64,  nsuint64  );
	NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_puint64, nsuint64* );
	NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_puint64, nsuint64* );
#endif

#define NS_VALUE_FUNC  "NsFunc"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_func, NsFunc );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_func, NsFunc );


/* Strings are a special case. Copies of the string are made
	when the value is set, not just stored by pointer. */
#define NS_VALUE_STRING  "nschar[]"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_string, nschar* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_string, nschar* );


/* Declare some other types here to avoid circular inclusion. */

#define NS_VALUE_ERROR  "NsError"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_error, NsError );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_error, NsError );
NS_VALUE_ARG_DECLARE( ns_value_arg_error, NsError );
NS_IMPEXP NsError ns_value_register_error( void );

#define NS_VALUE_FILE  "NsFile*"
NS_IMPEXP NS_VALUE_GET_DECLARE( ns_value_get_file, NsFile* );
NS_IMPEXP NS_VALUE_SET_DECLARE( ns_value_set_file, NsFile* );
NS_IMPEXP NS_VALUE_ARG_DECLARE( ns_value_arg_file, NsFile* );
NS_IMPEXP NsError ns_value_register_file( void );

NS_DECLS_END

#endif/* __NS_STD_VALUE_H__ */
