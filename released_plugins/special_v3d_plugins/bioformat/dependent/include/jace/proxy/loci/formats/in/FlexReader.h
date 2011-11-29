#ifndef JACE_PROXY_LOCI_FORMATS_IN_FLEXREADER_H
#define JACE_PROXY_LOCI_FORMATS_IN_FLEXREADER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_FORMATREADER_H
#include "jace/proxy/loci/formats/FormatReader.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_PROXY_JAVA_LANG_STRING_H
#include "jace/proxy/java/lang/String.h"
#endif

#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
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
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, in )

/**
 * The Jace C++ proxy class for loci.formats.in.FlexReader.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FlexReader : public ::jace::proxy::loci::formats::FormatReader
{
public: 

/**
 * FlexReader
 *
 */
FlexReader();

/**
 * isSingleFile
 *
 */
::jace::proxy::types::JBoolean isSingleFile( ::jace::proxy::java::lang::String p0 );

/**
 * getSeriesUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getSeriesUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * close
 *
 */
void close( ::jace::proxy::types::JBoolean p0 );

/**
 * appendServerMap
 *
 */
static void appendServerMap( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * mapServer
 *
 */
static void mapServer( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * mapServer
 *
 */
static void mapServer( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * mapServersFromConfigurationFile
 *
 */
static void mapServersFromConfigurationFile( ::jace::proxy::java::lang::String p0 );

/**
 * public static final FLEX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > FLEX();

/**
 * public static final FLEX_SUFFIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > FLEX_SUFFIX();

/**
 * public static final MEA_SUFFIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > MEA_SUFFIX();

/**
 * public static final RES_SUFFIX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > RES_SUFFIX();

/**
 * public static final SCREENING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SCREENING();

/**
 * public static final ARCHIVE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > ARCHIVE();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FlexReader( jvalue value );
FlexReader( jobject object );
FlexReader( const FlexReader& object );
FlexReader( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, in )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::in::FlexReader( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::in::FlexReader>& proxy ) : 
    ::jace::proxy::loci::formats::in::FlexReader( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::in::FlexReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::in::FlexReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::in::FlexReader>& object ) : 
    ::jace::proxy::loci::formats::in::FlexReader( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IN_FLEXREADER_H

