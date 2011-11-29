#ifndef JACE_PROXY_JAVA_LANG_BOOLEAN_H
#define JACE_PROXY_JAVA_LANG_BOOLEAN_H

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
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Boolean.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Boolean : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::io::Serializable, public virtual ::jace::proxy::java::lang::Comparable
{
public: 

/**
 * Boolean
 *
 */
Boolean( ::jace::proxy::types::JBoolean p0 );

/**
 * Boolean
 *
 */
Boolean( ::jace::proxy::java::lang::String p0 );

/**
 * parseBoolean
 *
 */
static ::jace::proxy::types::JBoolean parseBoolean( ::jace::proxy::java::lang::String p0 );

/**
 * booleanValue
 *
 */
::jace::proxy::types::JBoolean booleanValue();

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Boolean valueOf( ::jace::proxy::types::JBoolean p0 );

/**
 * valueOf
 *
 */
static ::jace::proxy::java::lang::Boolean valueOf( ::jace::proxy::java::lang::String p0 );

/**
 * toString
 *
 */
static ::jace::proxy::java::lang::String toString( ::jace::proxy::types::JBoolean p0 );

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
 * getBoolean
 *
 */
static ::jace::proxy::types::JBoolean getBoolean( ::jace::proxy::java::lang::String p0 );

/**
 * compareTo
 *
 */
::jace::proxy::types::JInt compareTo( ::jace::proxy::java::lang::Boolean p0 );

/**
 * public static final TRUE_Jace
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::Boolean > TRUE_Jace();

/**
 * public static final FALSE_Jace
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::Boolean > FALSE_Jace();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Boolean( jvalue value );
Boolean( jobject object );
Boolean( const Boolean& object );
Boolean( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Boolean>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Boolean>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Boolean>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Boolean>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Boolean( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Boolean>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Boolean>& proxy ) : 
    ::jace::proxy::java::lang::Boolean( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Boolean>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Boolean( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Boolean( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Boolean>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Boolean>& object ) : 
    ::jace::proxy::java::lang::Boolean( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_BOOLEAN_H

