#ifndef __NS_STD_ASCII_H__
#define __NS_STD_ASCII_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

NS_IMPEXP nssize ns_ascii_strlen( const nschar *s );

/* Returns the number of bytes the string occupies.
	i.e. ns_ascii_strlen( s ) + 1. */
NS_IMPEXP nssize ns_ascii_strsize( const nschar *s );

NS_IMPEXP nschar* ns_ascii_strchr( const nschar *s, nsint c );
NS_IMPEXP nschar* ns_ascii_strrchr( const nschar *s, nsint c );

NS_IMPEXP nschar* ns_ascii_strcat( nschar *dest, const nschar *src );

NS_IMPEXP nschar* ns_ascii_strstr( const nschar *s, const nschar *cset );

NS_IMPEXP nschar* ns_ascii_strtok( nschar *s, const nschar *delims );

NS_IMPEXP nschar* ns_ascii_strcpy( nschar *dest, const nschar *src );
NS_IMPEXP nschar* ns_ascii_strncpy( nschar *dest, const nschar *src, nssize count );

NS_IMPEXP nsint ns_ascii_strcmp( const nschar *s1, const nschar *s2 );
NS_IMPEXP nsint ns_ascii_stricmp( const nschar *s1, const nschar *s2 );

NS_IMPEXP nsint ns_ascii_strncmp( const nschar *s1, const nschar *s2, nssize count );
NS_IMPEXP nsint ns_ascii_strnicmp( const nschar *s1, const nschar *s2, nssize count );

NS_IMPEXP nsboolean ns_ascii_streq( const nschar *s1, const nschar *s2 );
NS_IMPEXP nsboolean ns_ascii_strieq( const nschar *s1, const nschar *s2 );

NS_IMPEXP nsboolean ns_ascii_strneq( const nschar *s1, const nschar *s2, nssize count );
NS_IMPEXP nsboolean ns_ascii_strnieq( const nschar *s1, const nschar *s2, nssize count );

NS_IMPEXP nschar* ns_ascii_strlwr( nschar *s );
NS_IMPEXP nschar* ns_ascii_strupr( nschar *s );

/* NOTE: Call ns_free() on returned pointer. */
NS_IMPEXP nschar* ns_ascii_strdup( const nschar *s );

NS_IMPEXP nsuint ns_ascii_hash( const nschar *s );

/* Remove leading whitespace. */
NS_IMPEXP nschar* ns_ascii_chomp( const nschar *s );

NS_IMPEXP nschar* ns_ascii_strrev( nschar *s );

NS_IMPEXP nschar* ns_ascii_chrpre( nschar *dest, nsint src );

/* Replace all occurrences of 'dest' character with 'src' character. Returns 'string'. */
NS_IMPEXP nschar* ns_ascii_chrarep( nschar *string, nsint dest, nsint src );

/* Replace all occurrences of 'dest' string with 'src' string. Returns 'string'. */
NS_IMPEXP nschar* ns_ascii_strarep( nschar *string, const nschar *dest, const nschar *src );

NS_IMPEXP nschar* ns_ascii_chrins( nschar *dest, nssize at, nsint src );


/* Expands tabs to the 'size'. */
NS_IMPEXP nschar* ns_ascii_strntab( nschar *dest, const nschar *src, nssize count, nssize size );


NS_IMPEXP nsboolean ns_ascii_isspace( nsint c );
NS_IMPEXP nsboolean ns_ascii_isdigit( nsint c );
NS_IMPEXP nsboolean ns_ascii_isxdigit( nsint c );
NS_IMPEXP nsboolean ns_ascii_islower( nsint c );
NS_IMPEXP nsboolean ns_ascii_isupper( nsint c );
NS_IMPEXP nsboolean ns_ascii_isprint( nsint c );


#define ns_ascii_tolower( ch )\
	( ns_ascii_isupper( ( nsint )(ch) ) ?\
		( nsint )( ( (ch) - NS_ASCII_LATIN_CAPITAL_LETTER_A ) + NS_ASCII_LATIN_SMALL_LETTER_A ) : (ch) )

#define ns_ascii_toupper( ch )\
	( ns_ascii_islower( ( nsint )(ch) ) ?\
		( nsint )( ( (ch) - NS_ASCII_LATIN_SMALL_LETTER_A ) + NS_ASCII_LATIN_CAPITAL_LETTER_A ) : (ch) )


#define NS_ASCII_NULL                         ( ( nschar )0x00 )
#define NS_ASCII_START_OF_HEADING             ( ( nschar )0x01 )
#define NS_ASCII_START_OF_TEXT                ( ( nschar )0x02 )
#define NS_ASCII_END_OF_TEXT                  ( ( nschar )0x03 )
#define NS_ASCII_END_OF_TRANSMISSION          ( ( nschar )0x04 )
#define NS_ASCII_ENQUIRY                      ( ( nschar )0x05 )
#define NS_ASCII_ACKNOWLEDGE                  ( ( nschar )0x06 )
#define NS_ASCII_BELL                         ( ( nschar )0x07 )
#define NS_ASCII_BACKSPACE                    ( ( nschar )0x08 )
#define NS_ASCII_CHARACTER_TABULATION         ( ( nschar )0x09 )
#define NS_ASCII_LINE_FEED                    ( ( nschar )0x0A )
#define NS_ASCII_LINE_TABULATION              ( ( nschar )0x0B )
#define NS_ASCII_FORM_FEED                    ( ( nschar )0x0C )
#define NS_ASCII_CARRIAGE_RETURN              ( ( nschar )0x0D )
#define NS_ASCII_SHIFT_OUT                    ( ( nschar )0x0E )
#define NS_ASCII_SHIFT_IN                     ( ( nschar )0x0F )
#define NS_ASCII_DATA_LINK_ESCAPE             ( ( nschar )0x10 )
#define NS_ASCII_DEVICE_CONTROL_ONE           ( ( nschar )0x11 )
#define NS_ASCII_DEVICE_CONTROL_TWO           ( ( nschar )0x12 )
#define NS_ASCII_DEVICE_CONTROL_THREE         ( ( nschar )0x13 )
#define NS_ASCII_DEVICE_CONTROL_FOUR          ( ( nschar )0x14 )
#define NS_ASCII_NEGATIVE_ACKNOWLEDGE         ( ( nschar )0x15 )
#define NS_ASCII_SYNCHRONOUS_IDLE             ( ( nschar )0x16 )
#define NS_ASCII_END_OF_TRANSMISSION_BLOCK    ( ( nschar )0x17 )
#define NS_ASCII_CANCEL                       ( ( nschar )0x18 )
#define NS_ASCII_END_OF_MEDIUM                ( ( nschar )0x19 )
#define NS_ASCII_SUBSTITUTE                   ( ( nschar )0x1A )
#define NS_ASCII_ESCAPE                       ( ( nschar )0x1B )
#define NS_ASCII_INFORMATION_SEPARATOR_FOUR   ( ( nschar )0x1C )
#define NS_ASCII_INFORMATION_SEPARATOR_THREE  ( ( nschar )0x1D )
#define NS_ASCII_INFORMATION_SEPARATOR_TWO    ( ( nschar )0x1E )
#define NS_ASCII_INFORMATION_SEPARATOR_ONE    ( ( nschar )0x1F )
#define NS_ASCII_SPACE                        ( ( nschar )0x20 )
#define NS_ASCII_EXCLAMATION_MARK             ( ( nschar )0x21 )
#define NS_ASCII_QUOTATION_MARK               ( ( nschar )0x22 )
#define NS_ASCII_NUMBER_SIGN                  ( ( nschar )0x23 )
#define NS_ASCII_DOLLAR_SIGN                  ( ( nschar )0x24 )
#define NS_ASCII_PERCENT_SIGN                 ( ( nschar )0x25 )
#define NS_ASCII_AMPERSAND                    ( ( nschar )0x26 )
#define NS_ASCII_APOSTROPHE                   ( ( nschar )0x27 )
#define NS_ASCII_LEFT_PARENTHESIS             ( ( nschar )0x28 )
#define NS_ASCII_RIGHT_PARENTHESIS            ( ( nschar )0x29 )
#define NS_ASCII_ASTERISK                     ( ( nschar )0x2A )
#define NS_ASCII_PLUS_SIGN                    ( ( nschar )0x2B )
#define NS_ASCII_COMMA                        ( ( nschar )0x2C )
#define NS_ASCII_HYPHEN_MINUS                 ( ( nschar )0x2D )
#define NS_ASCII_FULL_STOP                    ( ( nschar )0x2E )
#define NS_ASCII_SOLIDUS                      ( ( nschar )0x2F )
#define NS_ASCII_DIGIT_ZERO                   ( ( nschar )0x30 )
#define NS_ASCII_DIGIT_ONE                    ( ( nschar )0x31 )
#define NS_ASCII_DIGIT_TWO                    ( ( nschar )0x32 )
#define NS_ASCII_DIGIT_THREE                  ( ( nschar )0x33 )
#define NS_ASCII_DIGIT_FOUR                   ( ( nschar )0x34 )
#define NS_ASCII_DIGIT_FIVE                   ( ( nschar )0x35 )
#define NS_ASCII_DIGIT_SIX                    ( ( nschar )0x36 )
#define NS_ASCII_DIGIT_SEVEN                  ( ( nschar )0x37 )
#define NS_ASCII_DIGIT_EIGHT                  ( ( nschar )0x38 )
#define NS_ASCII_DIGIT_NINE                   ( ( nschar )0x39 )
#define NS_ASCII_COLON                        ( ( nschar )0x3A )
#define NS_ASCII_SEMICOLON                    ( ( nschar )0x3B )
#define NS_ASCII_LESS_THAN_SIGN               ( ( nschar )0x3C )
#define NS_ASCII_EQUALS_SIGN                  ( ( nschar )0x3D )
#define NS_ASCII_GREATER_THAN_SIGN            ( ( nschar )0x3E )
#define NS_ASCII_QUESTION_MARK                ( ( nschar )0x3F )
#define NS_ASCII_COMMERCIAL_AT                ( ( nschar )0x40 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_A       ( ( nschar )0x41 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_B       ( ( nschar )0x42 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_C       ( ( nschar )0x43 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_D       ( ( nschar )0x44 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_E       ( ( nschar )0x45 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_F       ( ( nschar )0x46 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_G       ( ( nschar )0x47 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_H       ( ( nschar )0x48 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_I       ( ( nschar )0x49 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_J       ( ( nschar )0x4A )
#define NS_ASCII_LATIN_CAPITAL_LETTER_K       ( ( nschar )0x4B )
#define NS_ASCII_LATIN_CAPITAL_LETTER_L       ( ( nschar )0x4C )
#define NS_ASCII_LATIN_CAPITAL_LETTER_M       ( ( nschar )0x4D )
#define NS_ASCII_LATIN_CAPITAL_LETTER_N       ( ( nschar )0x4E )
#define NS_ASCII_LATIN_CAPITAL_LETTER_O       ( ( nschar )0x4F )
#define NS_ASCII_LATIN_CAPITAL_LETTER_P       ( ( nschar )0x50 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_Q       ( ( nschar )0x51 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_R       ( ( nschar )0x52 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_S       ( ( nschar )0x53 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_T       ( ( nschar )0x54 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_U       ( ( nschar )0x55 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_V       ( ( nschar )0x56 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_W       ( ( nschar )0x57 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_X       ( ( nschar )0x58 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_Y       ( ( nschar )0x59 )
#define NS_ASCII_LATIN_CAPITAL_LETTER_Z       ( ( nschar )0x5A )
#define NS_ASCII_LEFT_SQUARE_BRACKET          ( ( nschar )0x5B )
#define NS_ASCII_REVERSE_SOLIDUS              ( ( nschar )0x5C )
#define NS_ASCII_RIGHT_SQUARE_BRACKET         ( ( nschar )0x5D )
#define NS_ASCII_CIRCUMFLEX_ACCENT            ( ( nschar )0x5E )
#define NS_ASCII_LOW_LINE                     ( ( nschar )0x5F )
#define NS_ASCII_GRAVE_ACCENT                 ( ( nschar )0x60 )
#define NS_ASCII_LATIN_SMALL_LETTER_A         ( ( nschar )0x61 )
#define NS_ASCII_LATIN_SMALL_LETTER_B         ( ( nschar )0x62 )
#define NS_ASCII_LATIN_SMALL_LETTER_C         ( ( nschar )0x63 )
#define NS_ASCII_LATIN_SMALL_LETTER_D         ( ( nschar )0x64 )
#define NS_ASCII_LATIN_SMALL_LETTER_E         ( ( nschar )0x65 )
#define NS_ASCII_LATIN_SMALL_LETTER_F         ( ( nschar )0x66 )
#define NS_ASCII_LATIN_SMALL_LETTER_G         ( ( nschar )0x67 )
#define NS_ASCII_LATIN_SMALL_LETTER_H         ( ( nschar )0x68 )
#define NS_ASCII_LATIN_SMALL_LETTER_I         ( ( nschar )0x69 )
#define NS_ASCII_LATIN_SMALL_LETTER_J         ( ( nschar )0x6A )
#define NS_ASCII_LATIN_SMALL_LETTER_K         ( ( nschar )0x6B )
#define NS_ASCII_LATIN_SMALL_LETTER_L         ( ( nschar )0x6C )
#define NS_ASCII_LATIN_SMALL_LETTER_M         ( ( nschar )0x6D )
#define NS_ASCII_LATIN_SMALL_LETTER_N         ( ( nschar )0x6E )
#define NS_ASCII_LATIN_SMALL_LETTER_O         ( ( nschar )0x6F )
#define NS_ASCII_LATIN_SMALL_LETTER_P         ( ( nschar )0x70 )
#define NS_ASCII_LATIN_SMALL_LETTER_Q         ( ( nschar )0x71 )
#define NS_ASCII_LATIN_SMALL_LETTER_R         ( ( nschar )0x72 )
#define NS_ASCII_LATIN_SMALL_LETTER_S         ( ( nschar )0x73 )
#define NS_ASCII_LATIN_SMALL_LETTER_T         ( ( nschar )0x74 )
#define NS_ASCII_LATIN_SMALL_LETTER_U         ( ( nschar )0x75 )
#define NS_ASCII_LATIN_SMALL_LETTER_V         ( ( nschar )0x76 )
#define NS_ASCII_LATIN_SMALL_LETTER_W         ( ( nschar )0x77 )
#define NS_ASCII_LATIN_SMALL_LETTER_X         ( ( nschar )0x78 )
#define NS_ASCII_LATIN_SMALL_LETTER_Y         ( ( nschar )0x79 )
#define NS_ASCII_LATIN_SMALL_LETTER_Z         ( ( nschar )0x7A )
#define NS_ASCII_LEFT_CURLY_BRACKET           ( ( nschar )0x7B )
#define NS_ASCII_VERTICAL_LINE                ( ( nschar )0x7C )
#define NS_ASCII_RIGHT_CURLY_BRACKET          ( ( nschar )0x7D )
#define NS_ASCII_TILDE                        ( ( nschar )0x7E )
#define NS_ASCII_DELETE                       ( ( nschar )0x7F )

NS_DECLS_END

#endif/* __NS_STD_ASCII_H__ */
