#ifndef JACE_PROXY_JAVA_LANG_EXCEPTION_H
#define JACE_PROXY_JAVA_LANG_EXCEPTION_H

#ifndef JACE_OS_DEP_H
#include "jace/os_dep.h"
#endif

#ifndef JACE_NAMESPACE_H
#include "jace/namespace.h"
#endif

#ifndef JACE_JOBJECT_H
#include "jace/proxy/JObject.h"
#endif

#ifndef JACE_JENLISTER_H
#include "jace/JEnlister.h"
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
#ifndef JACE_PROXY_JAVA_LANG_THROWABLE_H
#include "jace/proxy/java/lang/Throwable.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Exception.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Exception : public ::jace::proxy::java::lang::Throwable
{
public: 

/**
 * Exception
 *
 */
Exception();

/**
 * Exception
 *
 */
Exception( ::jace::proxy::java::lang::String p0 );

/**
 * Exception
 *
 */
Exception( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::Throwable p1 );

/**
 * Exception
 *
 */
Exception( ::jace::proxy::java::lang::Throwable p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Exception( jvalue value );
Exception( jobject object );
Exception( const Exception& object );
Exception( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
static JEnlister<Exception> enlister;
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Exception>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Exception>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Exception>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Exception>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Exception( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Exception>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Exception>& proxy ) : 
    ::jace::proxy::java::lang::Exception( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Exception>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Exception( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Exception( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Exception>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Exception>& object ) : 
    ::jace::proxy::java::lang::Exception( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_EXCEPTION_H

