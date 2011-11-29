#ifndef JACE_PROXY_LOCI_FORMATS_FORMATHANDLER_H
#define JACE_PROXY_LOCI_FORMATS_FORMATHANDLER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_IFORMATHANDLER_H
#include "jace/proxy/loci/formats/IFormatHandler.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

#ifndef JACE_PROXY_LOCI_FORMATS_STATUSLISTENER_H
#include "jace/proxy/loci/formats/StatusListener.h"
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

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class StatusListener;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.FormatHandler.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FormatHandler : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::loci::formats::IFormatHandler
{
public: 

/**
 * FormatHandler
 *
 */
FormatHandler( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * FormatHandler
 *
 */
FormatHandler( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::java::lang::String p0 );

/**
 * getFormat
 *
 */
::jace::proxy::java::lang::String getFormat();

/**
 * getSuffixes
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getSuffixes();

/**
 * addStatusListener
 *
 */
void addStatusListener( ::jace::proxy::loci::formats::StatusListener p0 );

/**
 * removeStatusListener
 *
 */
void removeStatusListener( ::jace::proxy::loci::formats::StatusListener p0 );

/**
 * getStatusListeners
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::StatusListener > getStatusListeners();

/**
 * checkSuffix
 *
 */
static ::jace::proxy::types::JBoolean checkSuffix( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * checkSuffix
 *
 */
static ::jace::proxy::types::JBoolean checkSuffix( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FormatHandler( jvalue value );
FormatHandler( jobject object );
FormatHandler( const FormatHandler& object );
FormatHandler( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatHandler>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatHandler>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatHandler>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::FormatHandler( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatHandler>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatHandler>& proxy ) : 
    ::jace::proxy::loci::formats::FormatHandler( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::FormatHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::FormatHandler( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatHandler>& object ) : 
    ::jace::proxy::loci::formats::FormatHandler( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_FORMATHANDLER_H

