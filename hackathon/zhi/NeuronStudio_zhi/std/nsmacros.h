#ifndef __NS_STD_MACROS_H__
#define __NS_STD_MACROS_H__

#include <nsconfigure.h>

#ifdef __cplusplus
	#define NS_DECLS_BEGIN  extern "C" {
	#define NS_DECLS_END    }
#else
	#define NS_DECLS_BEGIN
	#define NS_DECLS_END
#endif


NS_DECLS_BEGIN


#define NS_PRIVATE       static
#define NS_INTERNAL
#define NS_SYNCHRONIZED
#define NS_REGISTER      register


#ifndef __DATE__
	#define __DATE__  "__DATE__"
#endif

#ifndef __FILE__
	#define __FILE__  "__FILE__"
#endif

#ifndef __LINE__
	#define __LINE__  -1
#endif

#ifndef __FUNCTION__
	#define __FUNCTION__  "__FUNCTION__"
#endif


#define NS_MODULE  "ns::" __FILE__ "::" __FUNCTION__ "()::" NS_STRINGIZE( __LINE__ )

#define NS_FUNCTION  __FILE__ "::" __FUNCTION__


#ifndef NULL
	#define NULL  ( ( nspointer )0 )
#endif


/* object == size */
#define NS_COMPILE_TIME_SIZE_CHECK( object, size )\
   extern nschar _ns_compile_time_size_check[ ( ( size ) == sizeof( object ) ) ? 1 : -1 ]

/* object1 == object2 */
#define NS_COMPILE_TIME_SIZE_EQUAL( object1, object2 )\
   extern nschar _ns_compile_time_size_equal[ ( sizeof( object1 ) == sizeof( object2 ) ) ? 1 : -1 ]

/* object1 <= object2 */
#define NS_COMPILE_TIME_SIZE_LESS_EQUAL( object1, object2 )\
   extern nschar _ns_compile_time_size_less_equal[ ( sizeof( object1 ) <= sizeof( object2 ) ) ? 1 : -1 ]

/* object1 >= object2 */
#define NS_COMPILE_TIME_SIZE_GREATER_EQUAL( object1, object2 )\
   extern nschar _ns_compile_time_size_greater_equal[ ( sizeof( object1 ) >= sizeof( object2 ) ) ? 1 : -1 ]

#define NS_COMPILE_TIME_BOOLEAN( l, op, r )\
	extern char _ns_compile_time_boolean[ ( l ) op ( r ) ? 1 : -1 ]

#define NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( struct_type, member, offset )\
	extern nschar _ns_compile_time_struct_member_offset_check\
		[ ( offset ) == NS_STRUCT_MEMBER_OFFSET( struct_type, member ) ? 1 : -1 ]


#define NS_USE_VARIABLE( x ) (x)


#define NS_FALSE  0
#define NS_TRUE   1


#define NS_XY  0
#define NS_ZY  1
#define NS_XZ  2


#define NS_PATH_SIZE  1024


/* Should be way more than enough. */
#define NS_INTEGER_TYPE_MAX_DIGITS   128
#define NS_FLOATING_TYPE_MAX_DIGITS  256


#ifdef NS_OS_WINDOWS
	#define NS_UNICHAR_PATH  ( ( nsunichar )'\\' )
	#define NS_STRING_PATH   "\\"
#else
	#define NS_UNICHAR_PATH  ( ( nsunichar )'/' )
	#define NS_STRING_PATH   "/"
#endif


#define NS_UNICHAR_NEWLINE           ( ( nsunichar )'\n' )
#define NS_UNICHAR_TAB               ( ( nsunichar )'\t' )
#define NS_UNICHAR_BACKSPACE         ( ( nsunichar )'\b' )
#define NS_UNICHAR_FORMFEED          ( ( nsunichar )'\f' )
#define NS_UNICHAR_CARRIAGE_RETURN   ( ( nsunichar )'\r' )
#define NS_UNICHAR_VERTICAL_TAB      ( ( nsunichar )'\v' )
#define NS_UNICHAR_BELL              ( ( nsunichar )'\a' )
#define NS_UNICHAR_QUOTATION         ( ( nsunichar )'\'' )
#define NS_UNICHAR_DOUBLE_QUOTATION  ( ( nsunichar )'\"' )
#define NS_UNICHAR_BACKSLASH         ( ( nsunichar )'\\' )
#define NS_UNICHAR_QUESTION          ( ( nsunichar )'\?' )

#define NS_STRING_NEWLINE           "\n"
#define NS_STRING_TAB               "\t"
#define NS_STRING_BACKSPACE         "\b"
#define NS_STRING_FORMFEED          "\f"
#define NS_STRING_CARRIAGE_RETURN   "\r"
#define NS_STRING_VERTICAL_TAB      "\v"
#define NS_STRING_BELL              "\a"
#define NS_STRING_QUOTATION         "\'"
#define NS_STRING_DOUBLE_QUOTATION  "\""
#define NS_STRING_BACKSLASH         "\\"
#define NS_STRING_QUESTION          "\?"


#define _NS_CONCAT( a, b ) a ## b
#define NS_CONCAT( a, b )  _NS_CONCAT( a, b )

#define _NS_STRINGIZE( a )  #a
#define NS_STRINGIZE( a )   _NS_STRINGIZE( a )


#define NS_BITS_TO_BYTES( bits )\
   ( ( bits ) / 8 + ( ( ( bits ) % 8 ) ? 1 : 0 ) )

#define NS_BYTES_TO_BITS( bytes )\
   ( ( bytes ) * 8 )


#define NS_TYPE_FOREACH( container, iter, begin, end, next, not_equal )\
	for(\
		(iter) = begin( (container) );\
		not_equal( (iter), end( (container) ) );\
		(iter) = next( (iter) )\
		)


#define NS_MIN( A, B )\
   ( ( (A) < (B) ) ? (A) : (B) )

#define NS_MAX( A, B )\
   ( ( (A) > (B) ) ? (A) : (B) )

#define NS_MIN3( a, b, c )\
	( ( (a) < (b) ) ? ( ( (a) < (c) ) ? (a) : (c) ) : ( ( (b) < (c) ) ? (b) : (c) ) )

#define NS_MAX3( a, b, c )\
	( ( (a) > (b) ) ? ( ( (a) > (c) ) ? (a) : (c) ) : ( ( (b) > (c) ) ? (b) : (c) ) )


#define NS_MAX3_INDEX( a, b, c )\
	( ( (a) > (b) ) ? ( ( (a) > (c) ) ? (0) : (2) ) : ( ( (b) > (c) ) ? (1) : (2) ) )


#define NS_ABS( A )\
   ( ( (A) < 0 ) ? -(A) : (A) )


#define NS_CLAMP( value, min, max )\
	( (value) < (min) ? (min) : ( (value) > (max) ? (max) : (value) ) )


#define NS_SWAP( type, t1, t2 )\
   {                         \
   type temp = t1;           \
   t1        = t2;           \
   t2        = temp;         \
   }


#define NS_POW2( value )\
   ( (value) * (value) )

#define NS_POW3( value )\
   ( (value) * (value) * (value) )


#define NS_DISTANCE_SQUARED( x1, y1, z1, x2, y2, z2 )\
	( NS_POW2( ( x2 ) - ( x1 ) ) +\
	  NS_POW2( ( y2 ) - ( y1 ) ) +\
	  NS_POW2( ( z2 ) - ( z1 ) ) )


#define NS_DEGREES_TO_RADIANS( degs )\
   ( (degs) * ( NS_PI / 180.0 ) )

#define NS_RADIANS_TO_DEGREES( rads )\
   ( (rads) * ( 180.0 / NS_PI ) )


#define NS_FLOAT_ALMOST_ZERO  .00001f

#define NS_FLOAT_EQUAL( f1, f2 )\
	( NS_ABS( (f1) - (f2) ) <= NS_FLOAT_ALMOST_ZERO )


#define NS_DOUBLE_ALMOST_ZERO  .0000001

#define NS_DOUBLE_EQUAL( f1, f2 )\
	( NS_ABS( (f1) - (f2) ) <= NS_DOUBLE_ALMOST_ZERO )


/* Internal. DO NOT USE! */
#ifdef NS_CPU_LITTLE_ENDIAN
	#define ____NS_FLOAT_TO_INT32_INDEX  0
#else
	#define ____NS_FLOAT_TO_INT32_INDEX  1
#endif

/* Fast float to int conversions. 'f32' is converted
	to 'i32'. 'f64' used as temporary variable. */
#define NS_FLOAT32_TO_INT32_WITH_PROPER_ROUNDING( f32, i32, f64 )\
	(f64) = ( nsfloat64 )(f32) + 6755399441055744.0;\
	(i32) = ( ( nsint32* )&(f64) )[ ____NS_FLOAT_TO_INT32_INDEX ]

#define NS_FLOAT32_TO_INT32_WITH_FLOORED_ROUNDING( f32, i32, f64 )\
	(f64) = ( nsfloat64 )(f32) + 68719476736.0 * 1.5;\
	(i32) = ( nsint32 )( *( ( nsint64* )&(f64) ) >> 16 )


/* When f = 1.0...
   value = 1.0 * 127.5 - 0.5
   value = 127.5 - 0.5
   value = 127.0 ( correct max positive signed 8-bit integer )

   When f = -1.0...
   value = -1.0 * 127.5 - 0.5
   value = -127.5 - 0.5
   value = -128.0 ( correct min negative signed 8-bit integer )
*/
#define NS_NORMAL_FLOAT_TO_SIGNED_BYTE( fl )\
   ( ( nschar )( (fl) * 127.5f - 0.5f ) )


#define NS_ARRAY_LENGTH( array )\
   ( sizeof( (array) ) / sizeof( (array)[ 0 ] ) )


/* NOTE: 'boundary' must be a power of 2! */
#define NS_ALIGN( value, boundary )\
   ( ( ( value ) + ( boundary ) - 1 ) & ( ~( ( boundary ) - 1 ) ) )


#define NS_OFFSET_POINTER( type, ptr, bytes )\
   ( ( type * )( ( ( nsuint8* )( ptr ) ) + ( bytes ) ) )

#define NS_INCREMENT_POINTER( type, ptr, bytes )\
   ( ( type * )( ( ( nsuint8* )( ptr ) ) + ( bytes ) ) )

#define NS_DECREMENT_POINTER( type, ptr, bytes )\
   ( ( type * )( ( ( nsuint8* )( ptr ) ) - ( bytes ) ) )


#define NS_POINTER_DISTANCE( ptr1, ptr2 )\
	( ( ( nslong )( const nsuint8* )(ptr1) ) - ( ( nslong )( const nsuint8* )(ptr2) ) )


#define NS_STRUCT_MEMBER_OFFSET( struct_type, member )\
    ( ( nslong )( ( nsuint8* )( ( &( ( struct_type* )0 )->member ) ) ) )

#define NS_STRUCT_MEMBER_POINTER( struct_pointer, offset )\
    ( ( nspointer )( ( ( nsuint8* )( struct_pointer ) ) + ( ( elong )( offset ) ) ) )


#define NS_INFINITE_LOOP  for( ; ; )


#define NS_VERSION_VERIFY( major, minor, micro )\
  ns_verify(\
	( (major)  < NS_VERSION_MAJOR ) ||\
	( (major) == NS_VERSION_MAJOR && (minor)  < NS_VERSION_MINOR ) ||\
	( (major) == NS_VERSION_MAJOR && (minor) == NS_VERSION_MINOR && (micro) <= NS_VERSION_MICRO )\
	)


/* NOTE: Put here to avoid having to include emutex.h. */
#define NS_STATIC_MUTEX_DECLARE( smutex )\
nspointer smutex( void )

NS_DECLS_END

#endif/* __NS_STD_MACROS_H__ */
