#ifndef JACE_PROXY_JAVA_UTIL_HASHMAP_H
#define JACE_PROXY_JAVA_UTIL_HASHMAP_H

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
#ifndef JACE_PROXY_JAVA_UTIL_ABSTRACTMAP_H
#include "jace/proxy/java/util/AbstractMap.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_JAVA_UTIL_MAP_H
#include "jace/proxy/java/util/Map.h"
#endif

#ifndef JACE_PROXY_JAVA_LANG_CLONEABLE_H
#include "jace/proxy/java/lang/Cloneable.h"
#endif

#ifndef JACE_PROXY_JAVA_IO_SERIALIZABLE_H
#include "jace/proxy/java/io/Serializable.h"
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
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, util )

/**
 * The Jace C++ proxy class for java.util.HashMap.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class HashMap : public ::jace::proxy::java::util::AbstractMap, public virtual ::jace::proxy::java::util::Map, public virtual ::jace::proxy::java::lang::Cloneable, public virtual ::jace::proxy::java::io::Serializable
{
public: 

/**
 * HashMap
 *
 */
HashMap( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JFloat p1 );

/**
 * HashMap
 *
 */
HashMap( ::jace::proxy::types::JInt p0 );

/**
 * HashMap
 *
 */
HashMap();

/**
 * HashMap
 *
 */
HashMap( ::jace::proxy::java::util::Map p0 );

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
 * get
 *
 */
::jace::proxy::java::lang::Object get( ::jace::proxy::java::lang::Object p0 );

/**
 * containsKey
 *
 */
::jace::proxy::types::JBoolean containsKey( ::jace::proxy::java::lang::Object p0 );

/**
 * put
 *
 */
::jace::proxy::java::lang::Object put( ::jace::proxy::java::lang::Object p0, ::jace::proxy::java::lang::Object p1 );

/**
 * putAll
 *
 */
void putAll( ::jace::proxy::java::util::Map p0 );

/**
 * remove
 *
 */
::jace::proxy::java::lang::Object remove( ::jace::proxy::java::lang::Object p0 );

/**
 * clear
 *
 */
void clear();

/**
 * containsValue
 *
 */
::jace::proxy::types::JBoolean containsValue( ::jace::proxy::java::lang::Object p0 );

/**
 * clone
 *
 */
::jace::proxy::java::lang::Object clone();

/**
 * values
 *
 */
::jace::proxy::java::util::Collection values();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
HashMap( jvalue value );
HashMap( jobject object );
HashMap( const HashMap& object );
HashMap( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::util::HashMap>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::util::HashMap>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::HashMap>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::util::HashMap>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::util::HashMap( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::util::HashMap>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::util::HashMap>& proxy ) : 
    ::jace::proxy::java::util::HashMap( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::HashMap>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::util::HashMap( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::util::HashMap( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::util::HashMap>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::util::HashMap>& object ) : 
    ::jace::proxy::java::util::HashMap( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_UTIL_HASHMAP_H

