#ifndef JACE_PROXY_JAVA_LANG_SHORT_H
#define JACE_PROXY_JAVA_LANG_SHORT_H

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
#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
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
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
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
 * The Jace C++ proxy class for java.lang.Short.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Short : public ::jace::proxy::java::lang::Number, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * toString
 *
 */
static ::jace::proxy::java::lang::String toString( ::jace::proxy::types::JShort p0 );

/**
 * parseShort
 *
 */
static ::jace::proxy::types::JShort parseShort( ::jace::proxy::java::lang::String p0 );

/**
 * parseShort
 *
 */
static ::jace::proxy::types::JShort parseShort( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Short valueOf( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Short valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Short valueOf( ::jace::proxy::types::JShort p0 );

/**
 * decode
 *
 */
static ::jace::proxy::java::lang::Short decode( ::jace::proxy::java::lang::String p0 );

/**
 * Short
 *
 */
Short( ::jace::proxy::types::JShort p0 );

/**
 * Short
 *
 */
Short( ::jace::proxy::java::lang::String p0 );

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
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Short p0 );

/**
 * reverseBytes
 *
 */
static ::jace::proxy::types::JShort reverseBytes( ::jace::proxy::types::JShort p0 );

/**
 * public static final MIN_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JShort > MIN_VALUE();

/**
 * public static final MAX_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JShort > MAX_VALUE();

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
Short( jvalue value );
Short( jobject object );
Short( const Short& object );
Short( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Short>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Short>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Short>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Short>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Short( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Short>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Short>& proxy ) : 
    ::jace::proxy::java::lang::Short( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Short>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Short( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Short( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Short>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Short>& object ) : 
    ::jace::proxy::java::lang::Short( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_SHORT_H

