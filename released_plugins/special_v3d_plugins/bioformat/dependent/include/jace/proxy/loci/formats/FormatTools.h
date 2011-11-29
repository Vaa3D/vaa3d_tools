#ifndef JACE_PROXY_LOCI_FORMATS_FORMATTOOLS_H
#define JACE_PROXY_LOCI_FORMATS_FORMATTOOLS_H

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

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class IFormatReader;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.FormatTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FormatTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * getIndex
 *
 */
static ::jace::proxy::types::JInt getIndex( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getIndex
 *
 */
static ::jace::proxy::types::JInt getIndex( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5, ::jace::proxy::types::JInt p6, ::jace::proxy::types::JInt p7 );

/**
 * getZCTCoords
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > getZCTCoords( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1 );

/**
 * getZCTCoords
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > getZCTCoords( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * getReorderedIndex
 *
 */
static ::jace::proxy::types::JInt getReorderedIndex( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::types::JInt p2 );

/**
 * getReorderedIndex
 *
 */
static ::jace::proxy::types::JInt getReorderedIndex( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5, ::jace::proxy::types::JInt p6 );

/**
 * positionToRaster
 *
 */
static ::jace::proxy::types::JInt positionToRaster( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::JArray< ::jace::proxy::types::JInt > p1 );

/**
 * rasterToPosition
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > rasterToPosition( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1 );

/**
 * rasterToPosition
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > rasterToPosition( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JInt > p2 );

/**
 * getRasterLength
 *
 */
static ::jace::proxy::types::JInt getRasterLength( ::jace::JArray< ::jace::proxy::types::JInt > p0 );

/**
 * pixelTypeFromString
 *
 */
static ::jace::proxy::types::JInt pixelTypeFromString( ::jace::proxy::java::lang::String p0 );

/**
 * getPixelTypeString
 *
 */
static ::jace::proxy::java::lang::String getPixelTypeString( ::jace::proxy::types::JInt p0 );

/**
 * getBytesPerPixel
 *
 */
static ::jace::proxy::types::JInt getBytesPerPixel( ::jace::proxy::types::JInt p0 );

/**
 * isFloatingPoint
 *
 */
static ::jace::proxy::types::JBoolean isFloatingPoint( ::jace::proxy::types::JInt p0 );

/**
 * isSigned
 *
 */
static ::jace::proxy::types::JBoolean isSigned( ::jace::proxy::types::JInt p0 );

/**
 * assertId
 *
 */
static void assertId( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2 );

/**
 * checkPlaneParameters
 *
 */
static void checkPlaneParameters( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5, ::jace::proxy::types::JInt p6 );

/**
 * checkPlaneNumber
 *
 */
static void checkPlaneNumber( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1 );

/**
 * checkTileSize
 *
 */
static void checkTileSize( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * checkBufferSize
 *
 */
static void checkBufferSize( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1 );

/**
 * checkBufferSize
 *
 */
static void checkBufferSize( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * validStream
 *
 */
static ::jace::proxy::types::JBoolean validStream( ::jace::proxy::loci::common::RandomAccessInputStream p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * getPlaneSize
 *
 */
static ::jace::proxy::types::JInt getPlaneSize( ::jace::proxy::loci::formats::IFormatReader p0 );

/**
 * getPlaneSize
 *
 */
static ::jace::proxy::types::JInt getPlaneSize( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getFilename
 *
 */
static ::jace::proxy::java::lang::String getFilename( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::loci::formats::IFormatReader p2, ::jace::proxy::java::lang::String p3 );

/**
 * getFilenames
 *
 */
static ::jace::JArray< ::jace::proxy::java::lang::String > getFilenames( ::jace::proxy::java::lang::String p0, ::jace::proxy::loci::formats::IFormatReader p1 );

/**
 * getImagesPerFile
 *
 */
static ::jace::proxy::types::JInt getImagesPerFile( ::jace::proxy::java::lang::String p0, ::jace::proxy::loci::formats::IFormatReader p1 );

/**
 * openThumbBytes
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > openThumbBytes( ::jace::proxy::loci::formats::IFormatReader p0, ::jace::proxy::types::JInt p1 );

/**
 * public static final INT8
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > INT8();

/**
 * public static final UINT8
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UINT8();

/**
 * public static final INT16
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > INT16();

/**
 * public static final UINT16
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UINT16();

/**
 * public static final INT32
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > INT32();

/**
 * public static final UINT32
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UINT32();

/**
 * public static final FLOAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > FLOAT();

/**
 * public static final DOUBLE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DOUBLE();

/**
 * public static final CHANNEL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > CHANNEL();

/**
 * public static final SPECTRA
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SPECTRA();

/**
 * public static final LIFETIME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > LIFETIME();

/**
 * public static final POLARIZATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > POLARIZATION();

/**
 * public static final PHASE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > PHASE();

/**
 * public static final FREQUENCY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > FREQUENCY();

/**
 * public static final MUST_GROUP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MUST_GROUP();

/**
 * public static final CAN_GROUP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CAN_GROUP();

/**
 * public static final CANNOT_GROUP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CANNOT_GROUP();

/**
 * public static final SERIES_NUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SERIES_NUM();

/**
 * public static final SERIES_NAME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SERIES_NAME();

/**
 * public static final CHANNEL_NUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > CHANNEL_NUM();

/**
 * public static final CHANNEL_NAME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > CHANNEL_NAME();

/**
 * public static final Z_NUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > Z_NUM();

/**
 * public static final T_NUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > T_NUM();

/**
 * public static final SVN_REVISION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SVN_REVISION();

/**
 * public static final DATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > DATE();

/**
 * public static final VERSION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > VERSION();

/**
 * public static final HCS_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > HCS_DOMAIN();

/**
 * public static final LM_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > LM_DOMAIN();

/**
 * public static final EM_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > EM_DOMAIN();

/**
 * public static final SPM_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SPM_DOMAIN();

/**
 * public static final SEM_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > SEM_DOMAIN();

/**
 * public static final FLIM_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > FLIM_DOMAIN();

/**
 * public static final MEDICAL_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > MEDICAL_DOMAIN();

/**
 * public static final HISTOLOGY_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > HISTOLOGY_DOMAIN();

/**
 * public static final GEL_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > GEL_DOMAIN();

/**
 * public static final ASTRONOMY_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > ASTRONOMY_DOMAIN();

/**
 * public static final GRAPHICS_DOMAIN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::java::lang::String > GRAPHICS_DOMAIN();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FormatTools( jvalue value );
FormatTools( jobject object );
FormatTools( const FormatTools& object );
FormatTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::FormatTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatTools>& proxy ) : 
    ::jace::proxy::loci::formats::FormatTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::FormatTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::FormatTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatTools>& object ) : 
    ::jace::proxy::loci::formats::FormatTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_FORMATTOOLS_H

