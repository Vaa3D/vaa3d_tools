#ifndef JACE_PROXY_LOCI_COMMON_DATETOOLS_H
#define JACE_PROXY_LOCI_COMMON_DATETOOLS_H

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

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

#ifndef JACE_TYPES_JLONG_H
#include "jace/proxy/types/JLong.h"
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

BEGIN_NAMESPACE_4( jace, proxy, loci, common )

/**
 * The Jace C++ proxy class for loci.common.DateTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class DateTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * getMillisFromTicks
 *
 */
static ::jace::proxy::types::JLong getMillisFromTicks( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JLong p1 );

/**
 * convertDate
 *
 */
static ::jace::proxy::java::lang::String convertDate( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1 );

/**
 * convertDate
 *
 */
static ::jace::proxy::java::lang::String convertDate( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JInt p1, ::jace::proxy::java::lang::String p2 );

/**
 * formatDate
 *
 */
static ::jace::proxy::java::lang::String formatDate( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * formatDate
 *
 */
static ::jace::proxy::java::lang::String formatDate( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::types::JBoolean p2 );

/**
 * formatDate
 *
 */
static ::jace::proxy::java::lang::String formatDate( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * formatDate
 *
 */
static ::jace::proxy::java::lang::String formatDate( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1, ::jace::proxy::types::JBoolean p2 );

/**
 * getTime
 *
 */
static ::jace::proxy::types::JLong getTime( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * public static final UNIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UNIX();

/**
 * public static final COBOL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > COBOL();

/**
 * public static final MICROSOFT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MICROSOFT();

/**
 * public static final ZVI
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ZVI();

/**
 * public static final UNIX_EPOCH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JLong > UNIX_EPOCH();

/**
 * public static final COBOL_EPOCH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JLong > COBOL_EPOCH();

/**
 * public static final MICROSOFT_EPOCH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JLong > MICROSOFT_EPOCH();

/**
 * public static final ZVI_EPOCH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JLong > ZVI_EPOCH();

/**
 * public static final ISO8601_FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > ISO8601_FORMAT();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
DateTools( jvalue value );
DateTools( jobject object );
DateTools( const DateTools& object );
DateTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::common::DateTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::common::DateTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::DateTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::common::DateTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::common::DateTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::common::DateTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::common::DateTools>& proxy ) : 
    ::jace::proxy::loci::common::DateTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::DateTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::common::DateTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::common::DateTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::common::DateTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::common::DateTools>& object ) : 
    ::jace::proxy::loci::common::DateTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_COMMON_DATETOOLS_H

