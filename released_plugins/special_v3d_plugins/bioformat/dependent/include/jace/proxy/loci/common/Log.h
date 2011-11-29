#ifndef JACE_PROXY_LOCI_COMMON_LOG_H
#define JACE_PROXY_LOCI_COMMON_LOG_H

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

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Throwable;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JChar;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.Log.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class Log : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * Log
 *
 */
Log();

/**
 * print
 *
 */
void print( ::jace::proxy::java::lang::String p0 );

/**
 * flush
 *
 */
void flush();

/**
 * print
 *
 */
void print( ::jace::proxy::types::JBoolean p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::types::JChar p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::types::JDouble p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::types::JFloat p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::types::JInt p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::types::JLong p0 );

/**
 * print
 *
 */
void print( ::jace::proxy::java::lang::Object p0 );

/**
 * println
 *
 */
void println();

/**
 * println
 *
 */
void println( ::jace::proxy::types::JBoolean p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::types::JChar p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::types::JDouble p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::types::JFloat p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::types::JInt p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::types::JLong p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::java::lang::Object p0 );

/**
 * println
 *
 */
void println( ::jace::proxy::java::lang::String p0 );

/**
 * trace
 *
 */
void trace( ::jace::proxy::java::lang::String p0 );

/**
 * trace
 *
 */
void trace( ::jace::proxy::java::lang::Throwable p0 );

/**
 * getStackTrace
 *
 */
static ::jace::proxy::java::lang::String getStackTrace( ::jace::proxy::java::lang::Throwable p0 );

/**
 * public static final NL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > NL();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
Log( jvalue value );
Log( jobject object );
Log( const Log& object );
Log( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::Log>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::Log>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::Log>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::Log>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::Log( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::Log>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::Log>& proxy ) : 
    ::jace::proxy::loci::common::Log( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::Log>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::Log( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::Log( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::Log>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::Log>& object ) : 
    ::jace::proxy::loci::common::Log( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_LOG_H

