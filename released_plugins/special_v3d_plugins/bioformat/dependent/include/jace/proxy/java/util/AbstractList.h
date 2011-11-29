#ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTLIST_H
#define JACE_PROXY_JAVA_UTIL_ABSTRACTLIST_H

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
#ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTCOLLECTION_H
#include "jace/proxy/java/util/AbstractCollection.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_UTIL_LIST_H
#include "jace/proxy/java/util/List.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Collection;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, util )

/**
 * The Jace C++ proxy class for java.util.AbstractList.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class AbstractList : public ::jace::proxy::java::util::AbstractCollection, public virtual ::jace::proxy::java::util::List
{
public: 

/**
 * add
 *
 */
::jace::proxy::types::JBoolean add( ::jace::proxy::java::lang::Object p0 );

/**
 * get
 *
 */
::jace::proxy::java::lang::Object get( ::jace::proxy::types::JInt p0 );

/**
 * set
 *
 */
::jace::proxy::java::lang::Object set( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::Object p1 );

/**
 * add
 *
 */
void add( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::Object p1 );

/**
 * remove
 *
 */
::jace::proxy::java::lang::Object remove( ::jace::proxy::types::JInt p0 );

/**
 * indexOf
 *
 */
::jace::proxy::types::JInt indexOf( ::jace::proxy::java::lang::Object p0 );

/**
 * lastIndexOf
 *
 */
::jace::proxy::types::JInt lastIndexOf( ::jace::proxy::java::lang::Object p0 );

/**
 * clear
 *
 */
void clear();

/**
 * addAll
 *
 */
::jace::proxy::types::JBoolean addAll( ::jace::proxy::types::JInt p0, ::jace::proxy::java::util::Collection p1 );

/**
 * subList
 *
 */
::jace::proxy::java::util::List subList( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * hashCode
 *
 */
::jace::proxy::types::JInt hashCode();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
AbstractList( jvalue value );
AbstractList( jobject object );
AbstractList( const AbstractList& object );
AbstractList( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::util::AbstractList>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::util::AbstractList>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::AbstractList>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::util::AbstractList>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::util::AbstractList( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::util::AbstractList>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::AbstractList>& proxy ) : 
    ::jace::proxy::java::util::AbstractList( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::AbstractList>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::util::AbstractList( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::util::AbstractList( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractList>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::AbstractList>& object ) : 
    ::jace::proxy::java::util::AbstractList( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTLIST_H

