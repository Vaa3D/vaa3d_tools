#ifndef JACE_PROXY_LOCI_FORMATS_FORMATWRITER_H
#define JACE_PROXY_LOCI_FORMATS_FORMATWRITER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_FORMATHANDLER_H
#include "jace/proxy/loci/formats/FormatHandler.h"
#endif

/**
 * The interfaces implemented by this class.
 *
 */
#ifndef JACE_PROXY_LOCI_FORMATS_IFORMATWRITER_H
#include "jace/proxy/loci/formats/IFormatWriter.h"
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

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )
class ColorModel;
END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class MetadataRetrieve;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.FormatWriter.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FormatWriter : public ::jace::proxy::loci::formats::FormatHandler, public virtual ::jace::proxy::loci::formats::IFormatWriter
{
public: 

/**
 * FormatWriter
 *
 */
FormatWriter( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * FormatWriter
 *
 */
FormatWriter( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * saveBytes
 *
 */
void saveBytes( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1 );

/**
 * savePlane
 *
 */
void savePlane( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JBoolean p1 );

/**
 * savePlane
 *
 */
void savePlane( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2, ::jace::proxy::types::JBoolean p3 );

/**
 * setInterleaved
 *
 */
void setInterleaved( ::jace::proxy::types::JBoolean p0 );

/**
 * isInterleaved
 *
 */
::jace::proxy::types::JBoolean isInterleaved();

/**
 * canDoStacks
 *
 */
::jace::proxy::types::JBoolean canDoStacks();

/**
 * setMetadataRetrieve
 *
 */
void setMetadataRetrieve( ::jace::proxy::loci::formats::meta::MetadataRetrieve p0 );

/**
 * getMetadataRetrieve
 *
 */
::jace::proxy::loci::formats::meta::MetadataRetrieve getMetadataRetrieve();

/**
 * setColorModel
 *
 */
void setColorModel( ::jace::proxy::java::awt::image::ColorModel p0 );

/**
 * getColorModel
 *
 */
::jace::proxy::java::awt::image::ColorModel getColorModel();

/**
 * setFramesPerSecond
 *
 */
void setFramesPerSecond( ::jace::proxy::types::JInt p0 );

/**
 * getFramesPerSecond
 *
 */
::jace::proxy::types::JInt getFramesPerSecond();

/**
 * getCompressionTypes
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getCompressionTypes();

/**
 * setCompression
 *
 */
void setCompression( ::jace::proxy::java::lang::String p0 );

/**
 * getCompression
 *
 */
::jace::proxy::java::lang::String getCompression();

/**
 * getPixelTypes
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getPixelTypes();

/**
 * getPixelTypes
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getPixelTypes( ::jace::proxy::java::lang::String p0 );

/**
 * isSupportedType
 *
 */
::jace::proxy::types::JBoolean isSupportedType( ::jace::proxy::types::JInt p0 );

/**
 * setId
 *
 */
void setId( ::jace::proxy::java::lang::String p0 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FormatWriter( jvalue value );
FormatWriter( jobject object );
FormatWriter( const FormatWriter& object );
FormatWriter( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatWriter>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatWriter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatWriter>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatWriter>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::FormatWriter( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatWriter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatWriter>& proxy ) : 
    ::jace::proxy::loci::formats::FormatWriter( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::FormatWriter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::FormatWriter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatWriter>& object ) : 
    ::jace::proxy::loci::formats::FormatWriter( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_FORMATWRITER_H

