#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#define JACE_PROXY_JAVA_LANG_STRING_H

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

#include <string>

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

#ifndef JACE_PROXY_JAVA_LANG_CHARSEQUENCE_H
#include "jace/proxy/java/lang/CharSequence.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
#endif

#ifndef JACE_PROXY_JAVA_UTIL_COMPARATOR_H
#include "jace/proxy/java/util/Comparator.h"
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
BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.String.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class String : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::io::Serializable, public virtual ::jace::proxy::java::lang::Comparable, public virtual ::jace::proxy::java::lang::CharSequence
{
public: 

/**
 * String
 *
 */
String();

/**
 * String
 *
 */
String( ::jace::proxy::java::lang::String p0, CopyConstructorSpecifier );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JChar > p0 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::java::lang::String p3 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::java::lang::String p1 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * String
 *
 */
String( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * length
 *
 */
::jace::proxy::types::JInt length();

/**
 * charAt
 *
 */
::jace::proxy::types::JChar charAt( ::jace::proxy::types::JInt p0 );

/**
 * codePointAt
 *
 */
::jace::proxy::types::JInt codePointAt( ::jace::proxy::types::JInt p0 );

/**
 * codePointBefore
 *
 */
::jace::proxy::types::JInt codePointBefore( ::jace::proxy::types::JInt p0 );

/**
 * codePointCount
 *
 */
::jace::proxy::types::JInt codePointCount( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * offsetByCodePoints
 *
 */
::jace::proxy::types::JInt offsetByCodePoints( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getChars
 *
 */
void getChars( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JChar > p2, ::jace::proxy::types::JInt p3 );

/**
 * getBytes
 *
 */
void getBytes( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JByte > p2, ::jace::proxy::types::JInt p3 );

/**
 * getBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getBytes( ::jace::proxy::java::lang::String p0 );

/**
 * getBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getBytes();

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * contentEquals
 *
 */
::jace::proxy::types::JBoolean contentEquals( ::jace::proxy::java::lang::CharSequence p0 );

/**
 * equalsIgnoreCase
 *
 */
::jace::proxy::types::JBoolean equalsIgnoreCase( ::jace::proxy::java::lang::String p0 );

/**
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::String p0 );

/**
 * compareToIgnoreCase
 *
 */
::jace::proxy::types::JInt compareToIgnoreCase( ::jace::proxy::java::lang::String p0 );

/**
 * regionMatches
 *
 */
::jace::proxy::types::JBoolean regionMatches( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * regionMatches
 *
 */
::jace::proxy::types::JBoolean regionMatches( ::jace::proxy::types::JBoolean p0, ::jace::proxy::types::JInt p1, ::jace::proxy::java::lang::String p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * startsWith
 *
 */
::jace::proxy::types::JBoolean startsWith( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * startsWith
 *
 */
::jace::proxy::types::JBoolean startsWith( ::jace::proxy::java::lang::String p0 );

/**
 * endsWith
 *
 */
::jace::proxy::types::JBoolean endsWith( ::jace::proxy::java::lang::String p0 );

/**
 * hashCode
 *
 */
::jace::proxy::types::JInt hashCode();

/**
 * indexOf
 *
 */
::jace::proxy::types::JInt indexOf( ::jace::proxy::types::JInt p0 );

/**
 * indexOf
 *
 */
::jace::proxy::types::JInt indexOf( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * lastIndexOf
 *
 */
::jace::proxy::types::JInt lastIndexOf( ::jace::proxy::types::JInt p0 );

/**
 * lastIndexOf
 *
 */
::jace::proxy::types::JInt lastIndexOf( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * indexOf
 *
 */
::jace::proxy::types::JInt indexOf( ::jace::proxy::java::lang::String p0 );

/**
 * indexOf
 *
 */
::jace::proxy::types::JInt indexOf( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * lastIndexOf
 *
 */
::jace::proxy::types::JInt lastIndexOf( ::jace::proxy::java::lang::String p0 );

/**
 * lastIndexOf
 *
 */
::jace::proxy::types::JInt lastIndexOf( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * substring
 *
 */
::jace::proxy::java::lang::String substring( ::jace::proxy::types::JInt p0 );

/**
 * substring
 *
 */
::jace::proxy::java::lang::String substring( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * subSequence
 *
 */
::jace::proxy::java::lang::CharSequence subSequence( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * concat
 *
 */
::jace::proxy::java::lang::String concat( ::jace::proxy::java::lang::String p0 );

/**
 * replace
 *
 */
::jace::proxy::java::lang::String replace( ::jace::proxy::types::JChar p0, ::jace::proxy::types::JChar p1 );

/**
 * matches
 *
 */
::jace::proxy::types::JBoolean matches( ::jace::proxy::java::lang::String p0 );

/**
 * contains
 *
 */
::jace::proxy::types::JBoolean contains( ::jace::proxy::java::lang::CharSequence p0 );

/**
 * replaceFirst
 *
 */
::jace::proxy::java::lang::String replaceFirst( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * replaceAll
 *
 */
::jace::proxy::java::lang::String replaceAll( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * replace
 *
 */
::jace::proxy::java::lang::String replace( ::jace::proxy::java::lang::CharSequence p0, ::jace::proxy::java::lang::CharSequence p1 );

/**
 * split
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > split( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * split
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > split( ::jace::proxy::java::lang::String p0 );

/**
 * toLowerCase
 *
 */
::jace::proxy::java::lang::String toLowerCase();

/**
 * toUpperCase
 *
 */
::jace::proxy::java::lang::String toUpperCase();

/**
 * trim
 *
 */
::jace::proxy::java::lang::String trim();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * toCharArray
 *
 */
::jace::JArray< ::jace::proxy::types::JChar > toCharArray();

/**
 * format
 *
 */
static ::jace::proxy::java::lang::String format( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::Object > p1 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::java::lang::Object p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::JArray< ::jace::proxy::types::JChar > p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * copyValueOf
 *
 */
static ::jace::proxy::java::lang::String copyValueOf( ::jace::JArray< ::jace::proxy::types::JChar > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * copyValueOf
 *
 */
static ::jace::proxy::java::lang::String copyValueOf( ::jace::JArray< ::jace::proxy::types::JChar > p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JBoolean p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JChar p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JInt p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JLong p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JFloat p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::String valueOf( ::jace::proxy::types::JDouble p0 );

/**
 * intern
 *
 */
::jace::proxy::java::lang::String intern();

/**
 * Creates a String from a std::string using the default charset.
 *
 */
String( const std::string& str );

/**
 * Creates a String from a std::wstring.
 *
 */
String( const std::wstring& str );

/**
 * Creates a String from a c string.
 *
 */
String( const char* str );

/**
 * Handle assignment between two Strings.
 *
 */
String& operator=( const String& str );

/**
 * Converts a String to a std::string.
 *
 */
operator std::string() const;

/**
 * Converts a String to a std::wstring.
 *
 */
operator std::wstring() const;

/**
 * Allows Strings to be written to ostreams.
 *
 */
friend std::ostream& operator<<( std::ostream& stream, const String& str );

/**
 * Provide concatentation for Strings.
 *
 */
String operator+( String str );

/**
 * Provide concatenation between Strings and std::strings.
 *
 */
friend std::string operator+( const std::string& stdStr, const String& jStr );

/**
 * Provide concatenation between Strings and std::strings.
 *
 */
friend std::string operator+( const String& jStr, const std::string& stdStr );

/**
 * Provide comparison between Strings and std::strings.
 *
 */
friend bool operator==( const std::string& stdStr, const String& str );

/**
 * Provide comparison between Strings and std::strings.
 *
 */
friend bool operator==( const String& str, const std::string& stdStr );

private:

/**
 * Creates a new jstring from a std::string using the default charset.
 *
 */
jstring createString( const std::string& str );

public:

/**
 * public static final CASE_INSENSITIVE_ORDER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::util::Comparator > CASE_INSENSITIVE_ORDER();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
String( jvalue value );
String( jobject object );
String( const String& object );
String( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::String>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::String>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::String>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::String>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::String( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::String>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::String>& proxy ) : 
    ::jace::proxy::java::lang::String( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::String>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::String( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::String( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::String>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::String>& object ) : 
    ::jace::proxy::java::lang::String( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_STRING_H

