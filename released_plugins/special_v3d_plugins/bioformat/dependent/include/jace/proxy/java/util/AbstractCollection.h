#ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTCOLLECTION_H
#define JACE_PROXY_JAVA_UTIL_ABSTRACTCOLLECTION_H

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
#ifndef JACE_PROXY_JAVA_UTIL_COLLECTION_H
#include "jace/proxy/java/util/Collection.h"
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
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, util )

/**
 * The Jace C++ proxy class for java.util.AbstractCollection.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class AbstractCollection : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::util::Collection
{
public: 

/**
 * size
 *
 */
::jace::proxy::types::JInt size();

/**
 * isEmpty
 *
 */
::jace::proxy::types::JBoolean isEmpty();

/**
 * contains
 *
 */
::jace::proxy::types::JBoolean contains( ::jace::proxy::java::lang::Object p0 );

/**
 * toArray
 *
 */
::jace::JArray< ::jace::proxy::java::lang::Object > toArray();

/**
 * toArray
 *
 */
::jace::JArray< ::jace::proxy::java::lang::Object > toArray( ::jace::JArray< ::jace::proxy::java::lang::Object > p0 );

/**
 * add
 *
 */
::jace::proxy::types::JBoolean add( ::jace::proxy::java::lang::Object p0 );

/**
 * remove
 *
 */
::jace::proxy::types::JBoolean remove( ::jace::proxy::java::lang::Object p0 );

/**
 * containsAll
 *
 */
::jace::proxy::types::JBoolean containsAll( ::jace::proxy::java::util::Collection p0 );

/**
 * addAll
 *
 */
::jace::proxy::types::JBoolean addAll( ::jace::proxy::java::util::Collection p0 );

/**
 * removeAll
 *
 */
::jace::proxy::types::JBoolean removeAll( ::jace::proxy::java::util::Collection p0 );

/**
 * retainAll
 *
 */
::jace::proxy::types::JBoolean retainAll( ::jace::proxy::java::util::Collection p0 );

/**
 * clear
 *
 */
void clear();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
AbstractCollection( jvalue value );
AbstractCollection( jobject object );
AbstractCollection( const AbstractCollection& object );
AbstractCollection( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::util::AbstractCollection>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::util::AbstractCollection>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::AbstractCollection>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::util::AbstractCollection>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::util::AbstractCollection( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::util::AbstractCollection>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::AbstractCollection>& proxy ) : 
    ::jace::proxy::java::util::AbstractCollection( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::AbstractCollection>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::util::AbstractCollection( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::util::AbstractCollection( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::AbstractCollection>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::AbstractCollection>& object ) : 
    ::jace::proxy::java::util::AbstractCollection( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTCOLLECTION_H

