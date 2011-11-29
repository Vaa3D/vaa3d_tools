#ifndef JACE_PROXY_JAVA_LANG_DOUBLE_H
#define JACE_PROXY_JAVA_LANG_DOUBLE_H

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
#ifndef JACE_TYPES_JDOUBLE_H
#include "jace/proxy/types/JDouble.h"
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
 * The Jace C++ proxy class for java.lang.Double.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Double : public ::jace::proxy::java::lang::Number, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * toString
 *
 */
static ::jace::proxy::java::lang::String toString( ::jace::proxy::types::JDouble p0 );

/**
 * toHexString
 *
 */
static ::jace::proxy::java::lang::String toHexString( ::jace::proxy::types::JDouble p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Double valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Double valueOf( ::jace::proxy::types::JDouble p0 );

/**
 * parseDouble
 *
 */
static ::jace::proxy::types::JDouble parseDouble( ::jace::proxy::java::lang::String p0 );

/**
 * isNaN
 *
 */
static ::jace::proxy::types::JBoolean isNaN( ::jace::proxy::types::JDouble p0 );

/**
 * isInfinite
 *
 */
static ::jace::proxy::types::JBoolean isInfinite( ::jace::proxy::types::JDouble p0 );

/**
 * Double
 *
 */
Double( ::jace::proxy::types::JDouble p0 );

/**
 * Double
 *
 */
Double( ::jace::proxy::java::lang::String p0 );

/**
 * isNaN
 *
 */
::jace::proxy::types::JBoolean isNaN();

/**
 * isInfinite
 *
 */
::jace::proxy::types::JBoolean isInfinite();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

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
 * doubleToLongBits
 *
 */
static ::jace::proxy::types::JLong doubleToLongBits( ::jace::proxy::types::JDouble p0 );

/**
 * doubleToRawLongBits
 *
 */
static ::jace::proxy::types::JLong doubleToRawLongBits( ::jace::proxy::types::JDouble p0 );

/**
 * longBitsToDouble
 *
 */
static ::jace::proxy::types::JDouble longBitsToDouble( ::jace::proxy::types::JLong p0 );

/**
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Double p0 );

/**
 * compare
 *
 */
static ::jace::proxy::types::JInt compare( ::jace::proxy::types::JDouble p0, ::jace::proxy::types::JDouble p1 );

/**
 * public static final POSITIVE_INFINITY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JDouble > POSITIVE_INFINITY();

/**
 * public static final NEGATIVE_INFINITY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JDouble > NEGATIVE_INFINITY();

/**
 * public static final NaN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JDouble > NaN();

/**
 * public static final MAX_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JDouble > MAX_VALUE();

/**
 * public static final MIN_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JDouble > MIN_VALUE();

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
Double( jvalue value );
Double( jobject object );
Double( const Double& object );
Double( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Double>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Double>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Double>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Double>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Double( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Double>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Double>& proxy ) : 
    ::jace::proxy::java::lang::Double( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Double>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Double( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Double( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Double>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Double>& object ) : 
    ::jace::proxy::java::lang::Double( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_DOUBLE_H

