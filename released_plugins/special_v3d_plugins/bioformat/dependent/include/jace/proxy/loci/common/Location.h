#ifndef JACE_PROXY_LOCI_COMMON_LOCATION_H
#define JACE_PROXY_LOCI_COMMON_LOCATION_H

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
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class HashMap;
END_NAMESPACE_4( jace, proxy, java, util )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class IRandomAccess;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.Location.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Location : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * Location
 *
 */
Location( ::jace::proxy::java::lang::String p0 );

/**
 * Location
 *
 */
Location( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * Location
 *
 */
Location( ::jace::proxy::loci::common::Location p0, ::jace::proxy::java::lang::String p1 );

/**
 * mapId
 *
 */
static void mapId( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * mapFile
 *
 */
static void mapFile( ::jace::proxy::java::lang::String p0, ::jace::proxy::loci::common::IRandomAccess p1 );

/**
 * getMappedId
 *
 */
static ::jace::proxy::java::lang::String getMappedId( ::jace::proxy::java::lang::String p0 );

/**
 * getMappedFile
 *
 */
static ::jace::proxy::loci::common::IRandomAccess getMappedFile( ::jace::proxy::java::lang::String p0 );

/**
 * getIdMap
 *
 */
static ::jace::proxy::java::util::HashMap getIdMap();

/**
 * setIdMap
 *
 */
static void setIdMap( ::jace::proxy::java::util::HashMap p0 );

/**
 * getHandle
 *
 */
static ::jace::proxy::loci::common::IRandomAccess getHandle( ::jace::proxy::java::lang::String p0 );

/**
 * getHandle
 *
 */
static ::jace::proxy::loci::common::IRandomAccess getHandle( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * list
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > list( ::jace::proxy::types::JBoolean p0 );

/**
 * canRead
 *
 */
::jace::proxy::types::JBoolean canRead();

/**
 * canWrite
 *
 */
::jace::proxy::types::JBoolean canWrite();

/**
 * createNewFile
 *
 */
::jace::proxy::types::JBoolean createNewFile();

/**
 * delete_
 *
 */
::jace::proxy::types::JBoolean delete_();

/**
 * deleteOnExit
 *
 */
void deleteOnExit();

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * exists
 *
 */
::jace::proxy::types::JBoolean exists();

/**
 * getAbsoluteFile
 *
 */
::jace::proxy::loci::common::Location getAbsoluteFile();

/**
 * getAbsolutePath
 *
 */
::jace::proxy::java::lang::String getAbsolutePath();

/**
 * getCanonicalFile
 *
 */
::jace::proxy::loci::common::Location getCanonicalFile();

/**
 * getCanonicalPath
 *
 */
::jace::proxy::java::lang::String getCanonicalPath();

/**
 * getName
 *
 */
::jace::proxy::java::lang::String getName();

/**
 * getParent
 *
 */
::jace::proxy::java::lang::String getParent();

/**
 * getParentFile
 *
 */
::jace::proxy::loci::common::Location getParentFile();

/**
 * getPath
 *
 */
::jace::proxy::java::lang::String getPath();

/**
 * isAbsolute
 *
 */
::jace::proxy::types::JBoolean isAbsolute();

/**
 * isDirectory
 *
 */
::jace::proxy::types::JBoolean isDirectory();

/**
 * isFile
 *
 */
::jace::proxy::types::JBoolean isFile();

/**
 * isHidden
 *
 */
::jace::proxy::types::JBoolean isHidden();

/**
 * lastModified
 *
 */
::jace::proxy::types::JLong lastModified();

/**
 * length
 *
 */
::jace::proxy::types::JLong length();

/**
 * list
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > list();

/**
 * listFiles
 *
 */
::jace::JArray< ::jace::proxy::loci::common::Location > listFiles();

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
Location( jvalue value );
Location( jobject object );
Location( const Location& object );
Location( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::Location>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::Location>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::Location>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::Location>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::Location( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::Location>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::Location>& proxy ) : 
    ::jace::proxy::loci::common::Location( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::Location>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::Location( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::Location( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Location>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::Location>& object ) : 
    ::jace::proxy::loci::common::Location( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_LOCATION_H

