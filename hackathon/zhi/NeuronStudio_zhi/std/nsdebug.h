#ifndef __NS_STD_DEBUG_H__
#define __NS_STD_DEBUG_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nserror.h>
#include <std/nsprint.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

#ifdef NS_DEBUG

/* If a custom assert handler is necessary it must have the following interface. */ 
typedef void ( *NsAssertFunc )
	(
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details_or_file,
	nsint          line
	);


NS_IMPEXP NsAssertFunc ns_assert_get( void );
NS_IMPEXP NsError ns_assert_set( NsAssertFunc func );


/* Determines how to handle assert failures. */
typedef enum
	{
	NS_ASSERT_POLICY_ABORT,
	NS_ASSERT_POLICY_IGNORE
	}
	NsAssertPolicy;

/* get/set the assert policy for the current thread. */
NS_IMPEXP NsAssertPolicy ns_assert_policy_get( void );
NS_IMPEXP NsError ns_assert_policy_set( NsAssertPolicy policy );


#define ns_assert( expression )\
   ( (expression) || ( ( ns_assert_get() )( NS_STRINGIZE( expression ), NS_MODULE, NULL, 0 ), NS_FALSE ) )

#define ns_assert_with_details( expression, details )\
   ( (expression) || ( ( ns_assert_get() )( NS_STRINGIZE( expression ), NS_MODULE, (details), 0 ), NS_FALSE ) )

#define ns_assert_at_file_line( expression, file, line )\
   ( (expression) || ( ( ns_assert_get() )( NS_STRINGIZE( expression ), NS_MODULE, (file), (line) ), NS_FALSE ) )

#else

#define ns_assert( expression )
#define ns_assert_with_details( expression, details )
#define ns_assert_at_file_line( expression, file, line )

#endif/* NS_DEBUG */


/* Internal. DO NOT USE! */
NS_IMPEXP nsboolean _ns_assert_not_reached( void );

#define ns_assert_not_reached()  ns_assert( _ns_assert_not_reached() );




/* If a custom verify handler is necessary it must have the following interface. */ 
typedef void ( *NsVerifyFunc )
	(
	const nschar *expression,
	const nschar *module,
	const nschar *details
	);


NS_IMPEXP NsVerifyFunc ns_verify_get( void );
NS_IMPEXP NsError ns_verify_set( NsVerifyFunc func );


#define ns_verify( expression )\
	( (expression) || ( ( ns_verify_get() )( NS_STRINGIZE( expression ), NS_MODULE, NULL ), NS_FALSE ) )

#define ns_verify_with_details( expression, details )\
	( (expression) || ( ( ns_verify_get() )( NS_STRINGIZE( expression ), NS_MODULE, (details) ), NS_FALSE ) )




/* Place at FRONT of 'warning' string to indicate level. */
#define NS_WARNING_LEVEL_CAUTION      "<1>"
#define NS_WARNING_LEVEL_RECOVERABLE  "<2>"
#define NS_WARNING_LEVEL_CRITICAL     "<3>"
#define NS_WARNING_LEVEL_FATAL        "<4>"


typedef void ( *NsWarningFunc )( const nschar *warning, ns_va_list args );

/* get/set the handler function for the current thread. */
NS_IMPEXP NsWarningFunc ns_warning_get( void );
NS_IMPEXP NsError ns_warning_set( NsWarningFunc func );


NS_IMPEXP void ns_warning( const nschar *warning, ... );


/* Functions to format a 'string'. Can be used by
	custom handlers if necessary. All functions
	return 'string'. */

NS_IMPEXP nschar* ns_assert_format
	(
	nschar        *string,
	nssize         bytes,
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details
	);

NS_IMPEXP nschar* ns_verify_format
	(
	nschar        *string,
	nssize         bytes,
	const nschar  *expression,
	const nschar  *module,
	const nschar  *details
	);

NS_IMPEXP nschar* ns_warning_format
	(
	nschar        *string,
	nssize         bytes,
	const nschar  *warning,
	ns_va_list     args
	);

NS_DECLS_END

#endif/* __NS_STD_DEBUG_H__ */
