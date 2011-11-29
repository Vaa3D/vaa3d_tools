#ifndef JACE_PROXY_LOCI_FORMATS_CACHE_CACHESTRATEGY_H
#define JACE_PROXY_LOCI_FORMATS_CACHE_CACHESTRATEGY_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_CACHE_CACHEREPORTER_H
#include "jace/proxy/loci/formats/cache/CacheReporter.h"
#endif

#ifndef JACE_PROXY_JAVA_UTIL_COMPARATOR_H
#include "jace/proxy/java/util/Comparator.h"
#endif

#ifndef JACE_PROXY_LOCI_FORMATS_CACHE_ICACHESTRATEGY_H
#include "jace/proxy/loci/formats/cache/ICacheStrategy.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_LOCI_FORMATS_CACHE_CACHELISTENER_H
#include "jace/proxy/loci/formats/cache/CacheListener.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_5( jace, proxy, loci, formats, cache )
class CacheListener;
END_NAMESPACE_5( jace, proxy, loci, formats, cache )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, cache )
class CacheException;
END_NAMESPACE_5( jace, proxy, loci, formats, cache )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, cache )

/**
 * The Jace C++ proxy class for loci.formats.cache.CacheStrategy.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class CacheStrategy : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::loci::formats::cache::CacheReporter, public virtual ::jace::proxy::java::util::Comparator, public virtual ::jace::proxy::loci::formats::cache::ICacheStrategy
{
public: 

/**
 * CacheStrategy
 *
 */
CacheStrategy( ::jace::JArray< ::jace::proxy::types::JInt > p0 );

/**
 * distance
 *
 */
::jace::proxy::types::JInt distance( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * addCacheListener
 *
 */
void addCacheListener( ::jace::proxy::loci::formats::cache::CacheListener p0 );

/**
 * removeCacheListener
 *
 */
void removeCacheListener( ::jace::proxy::loci::formats::cache::CacheListener p0 );

/**
 * getCacheListeners
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::cache::CacheListener > getCacheListeners();

/**
 * compare
 *
 */
::jace::proxy::types::JInt compare( ::jace::proxy::java::lang::Object p0, ::jace::proxy::java::lang::Object p1 );

/**
 * getLoadList
 *
 */
::jace::JArray< ::jace::JArray< ::jace::proxy::types::JInt > > getLoadList( ::jace::JArray< ::jace::proxy::types::JInt > p0 );

/**
 * getPriorities
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getPriorities();

/**
 * setPriority
 *
 */
void setPriority( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getOrder
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getOrder();

/**
 * setOrder
 *
 */
void setOrder( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getRange
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getRange();

/**
 * setRange
 *
 */
void setRange( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getLengths
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getLengths();

/**
 * public static final DEFAULT_RANGE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DEFAULT_RANGE();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
CacheStrategy( jvalue value );
CacheStrategy( jobject object );
CacheStrategy( const CacheStrategy& object );
CacheStrategy( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, cache )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::cache::CacheStrategy( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>& proxy ) : 
    ::jace::proxy::loci::formats::cache::CacheStrategy( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::cache::CacheStrategy( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::cache::CacheStrategy( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::cache::CacheStrategy>& object ) : 
    ::jace::proxy::loci::formats::cache::CacheStrategy( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_CACHE_CACHESTRATEGY_H

