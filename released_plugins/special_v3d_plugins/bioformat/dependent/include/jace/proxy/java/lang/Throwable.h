#ifndef JACE_PROXY_JAVA_LANG_THROWABLE_H
#define JACE_PROXY_JAVA_LANG_THROWABLE_H

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
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Throwable.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Throwable : public virtual ::jace::proxy::java::lang::Object, public std::exception, public virtual ::jace::proxy::java::io::Serializable
{
public: 

/**
 * Throwable
 *
 */
Throwable();

/**
 * Throwable
 *
 */
Throwable( ::jace::proxy::java::lang::String p0 );

/**
 * Throwable
 *
 */
Throwable( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::Throwable p1 );

/**
 * Throwable
 *
 */
Throwable( ::jace::proxy::java::lang::Throwable p0, CopyConstructorSpecifier );

/**
 * getMessage
 *
 */
::jace::proxy::java::lang::String getMessage();

/**
 * getLocalizedMessage
 *
 */
::jace::proxy::java::lang::String getLocalizedMessage();

/**
 * getCause
 *
 */
::jace::proxy::java::lang::Throwable getCause();

/**
 * initCause
 *
 */
::jace::proxy::java::lang::Throwable initCause( ::jace::proxy::java::lang::Throwable p0 );

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * printStackTrace
 *
 */
void printStackTrace();

/**
 * fillInStackTrace
 *
 */
::jace::proxy::java::lang::Throwable fillInStackTrace();

/**
 * Need to support a non-throwing destructor
 *
 */
~Throwable() throw ();

/**
 * Overrides std::exception::what() by returning this.toString();
 *
 */
const char *what() const throw();

/**
 * The message represented by this Throwable.
 * 
 * This member variable is necessary to keep the contract
 * for exception.what().
 *
 */
private: 
std::string msg;
public: 

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Throwable( jvalue value );
Throwable( jobject object );
Throwable( const Throwable& object );
Throwable( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
static JEnlister<Throwable> enlister;
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Throwable>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Throwable>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Throwable>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Throwable>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::lang::Throwable( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Throwable>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Throwable>& proxy ) : 
    ::jace::proxy::java::lang::Throwable( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Throwable>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::lang::Throwable( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::lang::Throwable( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Throwable>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Throwable>& object ) : 
    ::jace::proxy::java::lang::Throwable( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_THROWABLE_H

