#ifndef JACE_PROXY_JAVA_LANG_BYTE_H
#define JACE_PROXY_JAVA_LANG_BYTE_H

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
#ifndef JACE_PROXY_JAVA_LANG_NUMBER_H
#include "jace/proxy/java/lang/Number.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
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

BEGIN_NAMESPACE_3( jace, proxy, types )
class JByte;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Byte.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Byte : public ::jace::proxy::java::lang::Number, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * toString
 *
 */
static ::jace::proxy::java::lang::String toString( ::jace::proxy::types::JByte p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Byte valueOf( ::jace::proxy::types::JByte p0 );

/**
 * parseByte
 *
 */
static ::jace::proxy::types::JByte parseByte( ::jace::proxy::java::lang::String p0 );

/**
 * parseByte
 *
 */
static ::jace::proxy::types::JByte parseByte( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Byte valueOf( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Byte valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * decode
 *
 */
static ::jace::proxy::java::lang::Byte decode( ::jace::proxy::java::lang::String p0 );

/**
 * Byte
 *
 */
Byte( ::jace::proxy::types::JByte p0 );

/**
 * Byte
 *
 */
Byte( ::jace::proxy::java::lang::String p0 );

/**
 * byteValue
 *
 */
::jace::proxy::types::JByte byteValue();

/**
 * shortValue
 *
 */
::jace::proxy::types::JShort shortValue();

/**
 * intValue
 *
 */
::jace::proxy::types::JInt intValue();

/**
 * longValue
 *
 */
::jace::proxy::types::JLong longValue();

/**
 * floatValue
 *
 */
::jace::proxy::types::JFloat floatValue();

/**
 * doubleValue
 *
 */
::jace::proxy::types::JDouble doubleValue();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

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
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Byte p0 );

/**
 * public static final MIN_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > MIN_VALUE();

/**
 * public static final MAX_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JByte > MAX_VALUE();

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
Byte( jvalue value );
Byte( jobject object );
Byte( const Byte& object );
Byte( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Byte>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Byte>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Byte>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Byte>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Byte( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Byte>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Byte>& proxy ) : 
    ::jace::proxy::java::lang::Byte( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Byte>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Byte( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Byte( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Byte>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Byte>& object ) : 
    ::jace::proxy::java::lang::Byte( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_BYTE_H

