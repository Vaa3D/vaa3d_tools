#ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFCOMPRESSION_H
#define JACE_PROXY_LOCI_FORMATS_TIFF_TIFFCOMPRESSION_H

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

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, codec )
class CodecOptions;
END_NAMESPACE_5( jace, proxy, loci, formats, codec )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )
class IFD;
END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )

/**
 * The Jace C++ proxy class for loci.formats.tiff.TiffCompression.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class TiffCompression : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * getCodecName
 *
 */
static ::jace::proxy::java::lang::String getCodecName( ::jace::proxy::types::JInt p0 );

/**
 * isSupportedDecompression
 *
 */
static ::jace::proxy::types::JBoolean isSupportedDecompression( ::jace::proxy::types::JInt p0 );

/**
 * uncompress
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > uncompress( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::loci::formats::codec::CodecOptions p2 );

/**
 * undifference
 *
 */
static void undifference( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::loci::formats::tiff::IFD p1 );

/**
 * isSupportedCompression
 *
 */
static ::jace::proxy::types::JBoolean isSupportedCompression( ::jace::proxy::types::JInt p0 );

/**
 * compress
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > compress( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::loci::formats::tiff::IFD p1 );

/**
 * difference
 *
 */
static void difference( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::JArray< ::jace::proxy::types::JInt > p1, ::jace::proxy::types::JLong p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * public static final UNCOMPRESSED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UNCOMPRESSED();

/**
 * public static final CCITT_1D
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CCITT_1D();

/**
 * public static final GROUP_3_FAX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > GROUP_3_FAX();

/**
 * public static final GROUP_4_FAX
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > GROUP_4_FAX();

/**
 * public static final LZW
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LZW();

/**
 * public static final JPEG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG();

/**
 * public static final PACK_BITS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PACK_BITS();

/**
 * public static final PROPRIETARY_DEFLATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PROPRIETARY_DEFLATE();

/**
 * public static final DEFLATE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DEFLATE();

/**
 * public static final THUNDERSCAN
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > THUNDERSCAN();

/**
 * public static final JPEG_2000
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_2000();

/**
 * public static final JPEG_2000_LOSSY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_2000_LOSSY();

/**
 * public static final ALT_JPEG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ALT_JPEG();

/**
 * public static final NIKON
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NIKON();

/**
 * public static final LURAWAVE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LURAWAVE();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
TiffCompression( jvalue value );
TiffCompression( jobject object );
TiffCompression( const TiffCompression& object );
TiffCompression( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::tiff::TiffCompression( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>& proxy ) : 
    ::jace::proxy::loci::formats::tiff::TiffCompression( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffCompression( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffCompression( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffCompression>& object ) : 
    ::jace::proxy::loci::formats::tiff::TiffCompression( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFCOMPRESSION_H

