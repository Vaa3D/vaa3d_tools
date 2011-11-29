#ifndef JACE_PROXY_JAVA_LANG_OBJECT_H
#define JACE_PROXY_JAVA_LANG_OBJECT_H

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
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )

/**
 * The Jace C++ proxy class for java.lang.Object.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Object : public virtual JObject
{
public: 

/**
 * Object
 *
 */
Object();

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
 * notify
 *
 */
void notify();

/**
 * notifyAll
 *
 */
void notifyAll();

/**
 * wait
 *
 */
void wait( ::jace::proxy::types::JLong p0 );

/**
 * wait
 *
 */
void wait( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1 );

/**
 * wait
 *
 */
void wait();

/**
 * Provide the standard "System.out.println()" semantics for ostreams.
 *
 */
friend std::ostream& operator<<( std::ostream& out, Object& object );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Object( jvalue value );
Object( jobject object );
Object( const Object& object );
Object( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::lang::Object>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::lang::Object>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Object>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::lang::Object>::ElementProxy( jarray array, jvalue element, int index ) : 
    Object( element ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::lang::Object>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::lang::Object>& proxy ) : 
    Object( proxy.getJavaJniObject() ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Object>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    Object( value ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    Object( value ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::lang::Object>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::lang::Object>& object ) : 
    Object( object.getJavaJniValue() )
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

#endif // #ifndef JACE_PROXY_JAVA_LANG_OBJECT_H

