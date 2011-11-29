#ifndef JACE_PROXY_JAVA_LANG_CHARACTER_H
#define JACE_PROXY_JAVA_LANG_CHARACTER_H

#ifndef JACE_OS_DEP_H
#include "jace/os_dep.h"
#endif

#ifndef JACE_NAMESPACE_H
#include "jace/namespace.h"
#endif

#ifndef JACE_JOBJECT_H
#include "jace/proxy/JObject.h"
#endif

#ifndef JACE_JARRAY_H
#include "jace/JArray.h"
#endif

#ifndef JACE_JFIELD_PROXY_H
#include "jace/JFieldProxy.h"
#endif

#ifndef JACE_JCLASSIMPL_H
#include "jace/JClassImpl.h"
#endif

/**
 * The super class for this class.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_OBJECT_H
#include "jace/proxy/java/lang/Object.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_IO_SERIALIZABLE_H
#include "jace/proxy/java/io/Serializable.h"
#endif

#ifndef JACE_PROXY_JAVA_LANG_COMPARABLE_H
#include "jace/proxy/java/lang/Comparable.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
#endif

#ifndef JACE_TYPES_JCHAR_H
#include "jace/proxy/types/JChar.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class CharSequence;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JByte;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Character.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Character : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::io::Serializable, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * Character
 *
 */
Character( ::jace::proxy::types::JChar p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Character valueOf( ::jace::proxy::types::JChar p0 );

/**
 * charValue
 *
 */
::jace::proxy::types::JChar charValue();

/**
 * hashCode
 *
 */
::jace::proxy::types::JInt hashCode();

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * toString
 *
 */
static ::jace::proxy::java::lang::String toString( ::jace::proxy::types::JChar p0 );

/**
 * isValidCodePoint
 *
 */
static ::jace::proxy::types::JBoolean isValidCodePoint( ::jace::proxy::types::JInt p0 );

/**
 * isSupplementaryCodePoint
 *
 */
static ::jace::proxy::types::JBoolean isSupplementaryCodePoint( ::jace::proxy::types::JInt p0 );

/**
 * isHighSurrogate
 *
 */
static ::jace::proxy::types::JBoolean isHighSurrogate( ::jace::proxy::types::JChar p0 );

/**
 * isLowSurrogate
 *
 */
static ::jace::proxy::types::JBoolean isLowSurrogate( ::jace::proxy::types::JChar p0 );

/**
 * isSurrogatePair
 *
 */
static ::jace::proxy::types::JBoolean isSurrogatePair( ::jace::proxy::types::JChar p0, ::jace::proxy::types::JChar p1 );

/**
 * charCount
 *
 */
static ::jace::proxy::types::JInt charCount( ::jace::proxy::types::JInt p0 );

/**
 * toCodePoint
 *
 */
static ::jace::proxy::types::JInt toCodePoint( ::jace::proxy::types::JChar p0, ::jace::proxy::types::JChar p1 );

/**
 * codePointAt
 *
 */
static ::jace::proxy::types::JInt codePointAt( ::jace::proxy::java::lang::CharSequence p0, ::jace::proxy::types::JInt p1 );

/**
 * codePointAt
 *
 */
static ::jace::proxy::types::JInt codePointAt( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1 );

/**
 * codePointAt
 *
 */
static ::jace::proxy::types::JInt codePointAt( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * codePointBefore
 *
 */
static ::jace::proxy::types::JInt codePointBefore( ::jace::proxy::java::lang::CharSequence p0, ::jace::proxy::types::JInt p1 );

/**
 * codePointBefore
 *
 */
static ::jace::proxy::types::JInt codePointBefore( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1 );

/**
 * codePointBefore
 *
 */
static ::jace::proxy::types::JInt codePointBefore( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * toChars
 *
 */
static ::jace::proxy::types::JInt toChars( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JChar > p1, ::jace::proxy::types::JInt p2 );

/**
 * toChars
 *
 */
static ::jace::JArray< ::jace::proxy::types::JChar > toChars( ::jace::proxy::types::JInt p0 );

/**
 * codePointCount
 *
 */
static ::jace::proxy::types::JInt codePointCount( ::jace::proxy::java::lang::CharSequence p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * codePointCount
 *
 */
static ::jace::proxy::types::JInt codePointCount( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * offsetByCodePoints
 *
 */
static ::jace::proxy::types::JInt offsetByCodePoints( ::jace::proxy::java::lang::CharSequence p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * offsetByCodePoints
 *
 */
static ::jace::proxy::types::JInt offsetByCodePoints( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * isLowerCase
 *
 */
static ::jace::proxy::types::JBoolean isLowerCase( ::jace::proxy::types::JChar p0 );

/**
 * isLowerCase
 *
 */
static ::jace::proxy::types::JBoolean isLowerCase( ::jace::proxy::types::JInt p0 );

/**
 * isUpperCase
 *
 */
static ::jace::proxy::types::JBoolean isUpperCase( ::jace::proxy::types::JChar p0 );

/**
 * isUpperCase
 *
 */
static ::jace::proxy::types::JBoolean isUpperCase( ::jace::proxy::types::JInt p0 );

/**
 * isTitleCase
 *
 */
static ::jace::proxy::types::JBoolean isTitleCase( ::jace::proxy::types::JChar p0 );

/**
 * isTitleCase
 *
 */
static ::jace::proxy::types::JBoolean isTitleCase( ::jace::proxy::types::JInt p0 );

/**
 * isDigit
 *
 */
static ::jace::proxy::types::JBoolean isDigit( ::jace::proxy::types::JChar p0 );

/**
 * isDigit
 *
 */
static ::jace::proxy::types::JBoolean isDigit( ::jace::proxy::types::JInt p0 );

/**
 * isDefined
 *
 */
static ::jace::proxy::types::JBoolean isDefined( ::jace::proxy::types::JChar p0 );

/**
 * isDefined
 *
 */
static ::jace::proxy::types::JBoolean isDefined( ::jace::proxy::types::JInt p0 );

/**
 * isLetter
 *
 */
static ::jace::proxy::types::JBoolean isLetter( ::jace::proxy::types::JChar p0 );

/**
 * isLetter
 *
 */
static ::jace::proxy::types::JBoolean isLetter( ::jace::proxy::types::JInt p0 );

/**
 * isLetterOrDigit
 *
 */
static ::jace::proxy::types::JBoolean isLetterOrDigit( ::jace::proxy::types::JChar p0 );

/**
 * isLetterOrDigit
 *
 */
static ::jace::proxy::types::JBoolean isLetterOrDigit( ::jace::proxy::types::JInt p0 );

/**
 * isJavaLetter
 *
 */
static ::jace::proxy::types::JBoolean isJavaLetter( ::jace::proxy::types::JChar p0 );

/**
 * isJavaLetterOrDigit
 *
 */
static ::jace::proxy::types::JBoolean isJavaLetterOrDigit( ::jace::proxy::types::JChar p0 );

/**
 * isJavaIdentifierStart
 *
 */
static ::jace::proxy::types::JBoolean isJavaIdentifierStart( ::jace::proxy::types::JChar p0 );

/**
 * isJavaIdentifierStart
 *
 */
static ::jace::proxy::types::JBoolean isJavaIdentifierStart( ::jace::proxy::types::JInt p0 );

/**
 * isJavaIdentifierPart
 *
 */
static ::jace::proxy::types::JBoolean isJavaIdentifierPart( ::jace::proxy::types::JChar p0 );

/**
 * isJavaIdentifierPart
 *
 */
static ::jace::proxy::types::JBoolean isJavaIdentifierPart( ::jace::proxy::types::JInt p0 );

/**
 * isUnicodeIdentifierStart
 *
 */
static ::jace::proxy::types::JBoolean isUnicodeIdentifierStart( ::jace::proxy::types::JChar p0 );

/**
 * isUnicodeIdentifierStart
 *
 */
static ::jace::proxy::types::JBoolean isUnicodeIdentifierStart( ::jace::proxy::types::JInt p0 );

/**
 * isUnicodeIdentifierPart
 *
 */
static ::jace::proxy::types::JBoolean isUnicodeIdentifierPart( ::jace::proxy::types::JChar p0 );

/**
 * isUnicodeIdentifierPart
 *
 */
static ::jace::proxy::types::JBoolean isUnicodeIdentifierPart( ::jace::proxy::types::JInt p0 );

/**
 * isIdentifierIgnorable
 *
 */
static ::jace::proxy::types::JBoolean isIdentifierIgnorable( ::jace::proxy::types::JChar p0 );

/**
 * isIdentifierIgnorable
 *
 */
static ::jace::proxy::types::JBoolean isIdentifierIgnorable( ::jace::proxy::types::JInt p0 );

/**
 * toLowerCase
 *
 */
static ::jace::proxy::types::JChar toLowerCase( ::jace::proxy::types::JChar p0 );

/**
 * toLowerCase
 *
 */
static ::jace::proxy::types::JInt toLowerCase( ::jace::proxy::types::JInt p0 );

/**
 * toUpperCase
 *
 */
static ::jace::proxy::types::JChar toUpperCase( ::jace::proxy::types::JChar p0 );

/**
 * toUpperCase
 *
 */
static ::jace::proxy::types::JInt toUpperCase( ::jace::proxy::types::JInt p0 );

/**
 * toTitleCase
 *
 */
static ::jace::proxy::types::JChar toTitleCase( ::jace::proxy::types::JChar p0 );

/**
 * toTitleCase
 *
 */
static ::jace::proxy::types::JInt toTitleCase( ::jace::proxy::types::JInt p0 );

/**
 * digit
 *
 */
static ::jace::proxy::types::JInt digit( ::jace::proxy::types::JChar p0, ::jace::proxy::types::JInt p1 );

/**
 * digit
 *
 */
static ::jace::proxy::types::JInt digit( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getNumericValue
 *
 */
static ::jace::proxy::types::JInt getNumericValue( ::jace::proxy::types::JChar p0 );

/**
 * getNumericValue
 *
 */
static ::jace::proxy::types::JInt getNumericValue( ::jace::proxy::types::JInt p0 );

/**
 * isSpace
 *
 */
static ::jace::proxy::types::JBoolean isSpace( ::jace::proxy::types::JChar p0 );

/**
 * isSpaceChar
 *
 */
static ::jace::proxy::types::JBoolean isSpaceChar( ::jace::proxy::types::JChar p0 );

/**
 * isSpaceChar
 *
 */
static ::jace::proxy::types::JBoolean isSpaceChar( ::jace::proxy::types::JInt p0 );

/**
 * isWhitespace
 *
 */
static ::jace::proxy::types::JBoolean isWhitespace( ::jace::proxy::types::JChar p0 );

/**
 * isWhitespace
 *
 */
static ::jace::proxy::types::JBoolean isWhitespace( ::jace::proxy::types::JInt p0 );

/**
 * isISOControl
 *
 */
static ::jace::proxy::types::JBoolean isISOControl( ::jace::proxy::types::JChar p0 );

/**
 * isISOControl
 *
 */
static ::jace::proxy::types::JBoolean isISOControl( ::jace::proxy::types::JInt p0 );

/**
 * getType
 *
 */
static ::jace::proxy::types::JInt getType( ::jace::proxy::types::JChar p0 );

/**
 * getType
 *
 */
static ::jace::proxy::types::JInt getType( ::jace::proxy::types::JInt p0 );

/**
 * forDigit
 *
 */
static ::jace::proxy::types::JChar forDigit( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getDirectionality
 *
 */
static ::jace::proxy::types::JByte getDirectionality( ::jace::proxy::types::JChar p0 );

/**
 * getDirectionality
 *
 */
static ::jace::proxy::types::JByte getDirectionality( ::jace::proxy::types::JInt p0 );

/**
 * isMirrored
 *
 */
static ::jace::proxy::types::JBoolean isMirrored( ::jace::proxy::types::JChar p0 );

/**
 * isMirrored
 *
 */
static ::jace::proxy::types::JBoolean isMirrored( ::jace::proxy::types::JInt p0 );

/**
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Character p0 );

/**
 * reverseBytes
 *
 */
static ::jace::proxy::types::JChar reverseBytes( ::jace::proxy::types::JChar p0 );

/**
 * public static final MIN_RADIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MIN_RADIX();

/**
 * public static final MAX_RADIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAX_RADIX();

/**
 * public static final MIN_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MIN_VALUE();

/**
 * public static final MAX_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MAX_VALUE();

/**
 * public static final UNASSIGNED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > UNASSIGNED();

/**
 * public static final UPPERCASE_LETTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > UPPERCASE_LETTER();

/**
 * public static final LOWERCASE_LETTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > LOWERCASE_LETTER();

/**
 * public static final TITLECASE_LETTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > TITLECASE_LETTER();

/**
 * public static final MODIFIER_LETTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > MODIFIER_LETTER();

/**
 * public static final OTHER_LETTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > OTHER_LETTER();

/**
 * public static final NON_SPACING_MARK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > NON_SPACING_MARK();

/**
 * public static final ENCLOSING_MARK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > ENCLOSING_MARK();

/**
 * public static final COMBINING_SPACING_MARK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > COMBINING_SPACING_MARK();

/**
 * public static final DECIMAL_DIGIT_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DECIMAL_DIGIT_NUMBER();

/**
 * public static final LETTER_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > LETTER_NUMBER();

/**
 * public static final OTHER_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > OTHER_NUMBER();

/**
 * public static final SPACE_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > SPACE_SEPARATOR();

/**
 * public static final LINE_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > LINE_SEPARATOR();

/**
 * public static final PARAGRAPH_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > PARAGRAPH_SEPARATOR();

/**
 * public static final CONTROL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > CONTROL();

/**
 * public static final FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > FORMAT();

/**
 * public static final PRIVATE_USE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > PRIVATE_USE();

/**
 * public static final SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > SURROGATE();

/**
 * public static final DASH_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DASH_PUNCTUATION();

/**
 * public static final START_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > START_PUNCTUATION();

/**
 * public static final END_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > END_PUNCTUATION();

/**
 * public static final CONNECTOR_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > CONNECTOR_PUNCTUATION();

/**
 * public static final OTHER_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > OTHER_PUNCTUATION();

/**
 * public static final MATH_SYMBOL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > MATH_SYMBOL();

/**
 * public static final CURRENCY_SYMBOL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > CURRENCY_SYMBOL();

/**
 * public static final MODIFIER_SYMBOL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > MODIFIER_SYMBOL();

/**
 * public static final OTHER_SYMBOL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > OTHER_SYMBOL();

/**
 * public static final INITIAL_QUOTE_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > INITIAL_QUOTE_PUNCTUATION();

/**
 * public static final FINAL_QUOTE_PUNCTUATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > FINAL_QUOTE_PUNCTUATION();

/**
 * public static final DIRECTIONALITY_UNDEFINED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_UNDEFINED();

/**
 * public static final DIRECTIONALITY_LEFT_TO_RIGHT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_LEFT_TO_RIGHT();

/**
 * public static final DIRECTIONALITY_RIGHT_TO_LEFT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_RIGHT_TO_LEFT();

/**
 * public static final DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC();

/**
 * public static final DIRECTIONALITY_EUROPEAN_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_EUROPEAN_NUMBER();

/**
 * public static final DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR();

/**
 * public static final DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR();

/**
 * public static final DIRECTIONALITY_ARABIC_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_ARABIC_NUMBER();

/**
 * public static final DIRECTIONALITY_COMMON_NUMBER_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_COMMON_NUMBER_SEPARATOR();

/**
 * public static final DIRECTIONALITY_NONSPACING_MARK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_NONSPACING_MARK();

/**
 * public static final DIRECTIONALITY_BOUNDARY_NEUTRAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_BOUNDARY_NEUTRAL();

/**
 * public static final DIRECTIONALITY_PARAGRAPH_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_PARAGRAPH_SEPARATOR();

/**
 * public static final DIRECTIONALITY_SEGMENT_SEPARATOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_SEGMENT_SEPARATOR();

/**
 * public static final DIRECTIONALITY_WHITESPACE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_WHITESPACE();

/**
 * public static final DIRECTIONALITY_OTHER_NEUTRALS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_OTHER_NEUTRALS();

/**
 * public static final DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING();

/**
 * public static final DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE();

/**
 * public static final DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING();

/**
 * public static final DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE();

/**
 * public static final DIRECTIONALITY_POP_DIRECTIONAL_FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > DIRECTIONALITY_POP_DIRECTIONAL_FORMAT();

/**
 * public static final MIN_HIGH_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MIN_HIGH_SURROGATE();

/**
 * public static final MAX_HIGH_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MAX_HIGH_SURROGATE();

/**
 * public static final MIN_LOW_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MIN_LOW_SURROGATE();

/**
 * public static final MAX_LOW_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MAX_LOW_SURROGATE();

/**
 * public static final MIN_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MIN_SURROGATE();

/**
 * public static final MAX_SURROGATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JChar > MAX_SURROGATE();

/**
 * public static final MIN_SUPPLEMENTARY_CODE_POINT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MIN_SUPPLEMENTARY_CODE_POINT();

/**
 * public static final MIN_CODE_POINT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MIN_CODE_POINT();

/**
 * public static final MAX_CODE_POINT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAX_CODE_POINT();

/**
 * public static final SIZE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SIZE();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Character( jvalue value );
Character( jobject object );
Character( const Character& object );
Character( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Character>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Character>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Character>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Character>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Character( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Character>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Character>& proxy ) : 
    ::jace::proxy::java::lang::Character( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Character>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Character( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Character( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Character>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Character>& object ) : 
    ::jace::proxy::java::lang::Character( object.getJavaJniValue() ), Object( NO_OP )
  {
    fieldID = object.fieldID; 

    if ( object.parent )
    {
      JNIEnv* env = ::jace::helper::attach();
      parent = ::jace::helper::newGlobalRef( env, object.parent );
    }
    else
      parent = 0;

    if ( object.parentClass )
    {
      JNIEnv* env = ::jace::helper::attach();
      parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, object.parentClass ) );
    }
    else
      parentClass = 0;
  }
#endif

END_NAMESPACE( jace )

#endif // #ifndef JACE_PROXY_JAVA_LANG_CHARACTER_H

