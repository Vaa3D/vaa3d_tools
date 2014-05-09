#ifndef __NS_STD_PRINT_H__
#define __NS_STD_PRINT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsformat.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

/* All functions return the number of bytes, ALSO including the
	null terminator, which for UTF-8 encoding is always 1 byte. */

NS_IMPEXP NsError ns_print( const nschar *format, ... );
NS_IMPEXP NsError ns_vprint( const nschar *format, ns_va_list args );

NS_IMPEXP NsError ns_println( const nschar *format, ... );

NS_IMPEXP NsError ns_sprint( nschar *buffer, const nschar *format, ... );
NS_IMPEXP NsError ns_vsprint( nschar *buffer, const nschar *format, ns_va_list args );

NS_IMPEXP NsError ns_snprint( nschar *buffer, nssize bytes, const nschar *format, ... );
NS_IMPEXP NsError ns_vsnprint( nschar *buffer, nssize bytes, const nschar *format, ns_va_list args );


#define ns_print_newline()           ns_print( NS_STRING_NEWLINE )
#define ns_print_tab()               ns_print( NS_STRING_TAB )
#define ns_print_backspace()         ns_print( NS_STRING_BACKSPACE )
#define ns_print_formfeed()          ns_print( NS_STRING_FORMFEED )
#define ns_print_carriage_return()   ns_print( NS_STRING_CARRIAGE_RETURN )
#define ns_print_vertical_tab()      ns_print( NS_STRING_VERTICAL_TAB )
#define ns_print_bell()              ns_print( NS_STRING_BELL )
#define ns_print_quotation()         ns_print( NS_STRING_QUOTATION )
#define ns_print_double_quotation()  ns_print( NS_STRING_DOUBLE_QUOTATION )
#define ns_print_backslash()         ns_print( NS_STRING_BACKSLASH )
#define ns_print_question()          ns_print( NS_STRING_QUESTION )


/* Can be used to re-direct the output. 'io' is a pointer
	to type FILE. Pass NULL for 'io' to re-establish output
	to the console. */
NS_IMPEXP nspointer ns_print_get_io( void );
NS_IMPEXP void ns_print_set_io( nspointer io );


/* Note that these functions are only currently necessary for	
	Windows(GUI) to allow the ns_print() family of functions to
	work seamlessly. */
NS_IMPEXP nsboolean ns_print_get_is_gui( void );
NS_IMPEXP void ns_print_set_is_gui( nsboolean is_gui );


/* Disable/Enable printing. */
NS_IMPEXP nsboolean ns_print_get_enabled( void );
NS_IMPEXP void ns_print_set_enabled( nsboolean enabled );

NS_DECLS_END

#endif/* __NS_STD_PRINT_H__ */
