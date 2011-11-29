#ifndef JACE_PROXY_LOCI_FORMATS_TIFF_IFD_H
#define JACE_PROXY_LOCI_FORMATS_TIFF_IFD_H

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
#ifndef JACE_PROXY_JAVA_UTIL_HASHMAP_H
#include "jace/proxy/java/util/HashMap.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
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

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )
class TiffRational;
END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JShort;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JLong;
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

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )

/**
 * The Jace C++ proxy class for loci.formats.tiff.IFD.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class IFD : public ::jace::proxy::java::util::HashMap
{
public: 

/**
 * IFD
 *
 */
IFD();

/**
 * IFD
 *
 */
IFD( ::jace::proxy::loci::formats::tiff::IFD p0, CopyConstructorSpecifier );

/**
 * isBigTiff
 *
 */
::jace::proxy::types::JBoolean isBigTiff();

/**
 * isLittleEndian
 *
 */
::jace::proxy::types::JBoolean isLittleEndian();

/**
 * getIFDValue
 *
 */
::jace::proxy::java::lang::Object getIFDValue( ::jace::proxy::types::JInt p0 );

/**
 * getIFDLongValue
 *
 */
::jace::proxy::types::JLong getIFDLongValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JLong p2 );

/**
 * getIFDIntValue
 *
 */
::jace::proxy::types::JInt getIFDIntValue( ::jace::proxy::types::JInt p0 );

/**
 * getIFDIntValue
 *
 */
::jace::proxy::types::JInt getIFDIntValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2 );

/**
 * getIFDRationalValue
 *
 */
::jace::proxy::loci::formats::tiff::TiffRational getIFDRationalValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getIFDStringValue
 *
 */
::jace::proxy::java::lang::String getIFDStringValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getIFDTextValue
 *
 */
::jace::proxy::java::lang::String getIFDTextValue( ::jace::proxy::types::JInt p0 );

/**
 * getIFDLongArray
 *
 */
::jace::JArray< ::jace::proxy::types::JLong > getIFDLongArray( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getIFDIntArray
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getIFDIntArray( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getIFDShortArray
 *
 */
::jace::JArray< ::jace::proxy::types::JShort > getIFDShortArray( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JBoolean p1 );

/**
 * getComment
 *
 */
::jace::proxy::java::lang::String getComment();

/**
 * getTileWidth
 *
 */
::jace::proxy::types::JLong getTileWidth();

/**
 * getTileLength
 *
 */
::jace::proxy::types::JLong getTileLength();

/**
 * getTilesPerRow
 *
 */
::jace::proxy::types::JLong getTilesPerRow();

/**
 * getTilesPerColumn
 *
 */
::jace::proxy::types::JLong getTilesPerColumn();

/**
 * isTiled
 *
 */
::jace::proxy::types::JBoolean isTiled();

/**
 * getImageWidth
 *
 */
::jace::proxy::types::JLong getImageWidth();

/**
 * getImageLength
 *
 */
::jace::proxy::types::JLong getImageLength();

/**
 * getBitsPerSample
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getBitsPerSample();

/**
 * getPixelType
 *
 */
::jace::proxy::types::JInt getPixelType();

/**
 * getBytesPerSample
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getBytesPerSample();

/**
 * getSamplesPerPixel
 *
 */
::jace::proxy::types::JInt getSamplesPerPixel();

/**
 * getCompression
 *
 */
::jace::proxy::types::JInt getCompression();

/**
 * getPhotometricInterpretation
 *
 */
::jace::proxy::types::JInt getPhotometricInterpretation();

/**
 * getPlanarConfiguration
 *
 */
::jace::proxy::types::JInt getPlanarConfiguration();

/**
 * getStripOffsets
 *
 */
::jace::JArray< ::jace::proxy::types::JLong > getStripOffsets();

/**
 * getStripByteCounts
 *
 */
::jace::JArray< ::jace::proxy::types::JLong > getStripByteCounts();

/**
 * getRowsPerStrip
 *
 */
::jace::JArray< ::jace::proxy::types::JLong > getRowsPerStrip();

/**
 * putIFDValue
 *
 */
void putIFDValue( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::Object p1 );

/**
 * putIFDValue
 *
 */
void putIFDValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JShort p1 );

/**
 * putIFDValue
 *
 */
void putIFDValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * putIFDValue
 *
 */
void putIFDValue( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JLong p1 );

/**
 * printIFD
 *
 */
void printIFD();

/**
 * getIFDTagName
 *
 */
static ::jace::proxy::java::lang::String getIFDTagName( ::jace::proxy::types::JInt p0 );

/**
 * getIFDTypeName
 *
 */
static ::jace::proxy::java::lang::String getIFDTypeName( ::jace::proxy::types::JInt p0 );

/**
 * getIFDTypeLength
 *
 */
static ::jace::proxy::types::JInt getIFDTypeLength( ::jace::proxy::types::JInt p0 );

/**
 * getFieldName
 *
 */
static ::jace::proxy::java::lang::String getFieldName( ::jace::proxy::types::JInt p0 );

/**
 * public static final LITTLE_ENDIAN_Jace
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LITTLE_ENDIAN_Jace();

/**
 * public static final BIG_TIFF
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > BIG_TIFF();

/**
 * public static final BYTE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > BYTE();

/**
 * public static final ASCII
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ASCII();

/**
 * public static final SHORT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SHORT();

/**
 * public static final LONG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LONG();

/**
 * public static final RATIONAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > RATIONAL();

/**
 * public static final SBYTE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SBYTE();

/**
 * public static final UNDEFINED
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > UNDEFINED();

/**
 * public static final SSHORT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SSHORT();

/**
 * public static final SLONG
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SLONG();

/**
 * public static final SRATIONAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SRATIONAL();

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
 * public static final IFD
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JACE_IFD();

/**
 * public static final LONG8
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > LONG8();

/**
 * public static final SLONG8
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SLONG8();

/**
 * public static final IFD8
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > IFD8();

/**
 * public static final NEW_SUBFILE_TYPE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NEW_SUBFILE_TYPE();

/**
 * public static final SUBFILE_TYPE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SUBFILE_TYPE();

/**
 * public static final IMAGE_WIDTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > IMAGE_WIDTH();

/**
 * public static final IMAGE_LENGTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > IMAGE_LENGTH();

/**
 * public static final BITS_PER_SAMPLE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > BITS_PER_SAMPLE();

/**
 * public static final COMPRESSION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > COMPRESSION();

/**
 * public static final PHOTOMETRIC_INTERPRETATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PHOTOMETRIC_INTERPRETATION();

/**
 * public static final THRESHHOLDING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > THRESHHOLDING();

/**
 * public static final CELL_WIDTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CELL_WIDTH();

/**
 * public static final CELL_LENGTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CELL_LENGTH();

/**
 * public static final FILL_ORDER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > FILL_ORDER();

/**
 * public static final DOCUMENT_NAME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DOCUMENT_NAME();

/**
 * public static final IMAGE_DESCRIPTION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > IMAGE_DESCRIPTION();

/**
 * public static final MAKE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAKE();

/**
 * public static final MODEL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MODEL();

/**
 * public static final STRIP_OFFSETS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > STRIP_OFFSETS();

/**
 * public static final ORIENTATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ORIENTATION();

/**
 * public static final SAMPLES_PER_PIXEL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SAMPLES_PER_PIXEL();

/**
 * public static final ROWS_PER_STRIP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ROWS_PER_STRIP();

/**
 * public static final STRIP_BYTE_COUNTS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > STRIP_BYTE_COUNTS();

/**
 * public static final MIN_SAMPLE_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MIN_SAMPLE_VALUE();

/**
 * public static final MAX_SAMPLE_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > MAX_SAMPLE_VALUE();

/**
 * public static final X_RESOLUTION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > X_RESOLUTION();

/**
 * public static final Y_RESOLUTION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > Y_RESOLUTION();

/**
 * public static final PLANAR_CONFIGURATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PLANAR_CONFIGURATION();

/**
 * public static final PAGE_NAME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PAGE_NAME();

/**
 * public static final X_POSITION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > X_POSITION();

/**
 * public static final Y_POSITION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > Y_POSITION();

/**
 * public static final FREE_OFFSETS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > FREE_OFFSETS();

/**
 * public static final FREE_BYTE_COUNTS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > FREE_BYTE_COUNTS();

/**
 * public static final GRAY_RESPONSE_UNIT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > GRAY_RESPONSE_UNIT();

/**
 * public static final GRAY_RESPONSE_CURVE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > GRAY_RESPONSE_CURVE();

/**
 * public static final T4_OPTIONS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > T4_OPTIONS();

/**
 * public static final T6_OPTIONS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > T6_OPTIONS();

/**
 * public static final RESOLUTION_UNIT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > RESOLUTION_UNIT();

/**
 * public static final PAGE_NUMBER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PAGE_NUMBER();

/**
 * public static final TRANSFER_FUNCTION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TRANSFER_FUNCTION();

/**
 * public static final SOFTWARE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SOFTWARE();

/**
 * public static final DATE_TIME
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DATE_TIME();

/**
 * public static final ARTIST
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > ARTIST();

/**
 * public static final HOST_COMPUTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > HOST_COMPUTER();

/**
 * public static final PREDICTOR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PREDICTOR();

/**
 * public static final WHITE_POINT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > WHITE_POINT();

/**
 * public static final PRIMARY_CHROMATICITIES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > PRIMARY_CHROMATICITIES();

/**
 * public static final COLOR_MAP
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > COLOR_MAP();

/**
 * public static final HALFTONE_HINTS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > HALFTONE_HINTS();

/**
 * public static final TILE_WIDTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TILE_WIDTH();

/**
 * public static final TILE_LENGTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TILE_LENGTH();

/**
 * public static final TILE_OFFSETS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TILE_OFFSETS();

/**
 * public static final TILE_BYTE_COUNTS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TILE_BYTE_COUNTS();

/**
 * public static final INK_SET
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > INK_SET();

/**
 * public static final INK_NAMES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > INK_NAMES();

/**
 * public static final NUMBER_OF_INKS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > NUMBER_OF_INKS();

/**
 * public static final DOT_RANGE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > DOT_RANGE();

/**
 * public static final TARGET_PRINTER
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TARGET_PRINTER();

/**
 * public static final EXTRA_SAMPLES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > EXTRA_SAMPLES();

/**
 * public static final SAMPLE_FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > SAMPLE_FORMAT();

/**
 * public static final S_MIN_SAMPLE_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > S_MIN_SAMPLE_VALUE();

/**
 * public static final S_MAX_SAMPLE_VALUE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > S_MAX_SAMPLE_VALUE();

/**
 * public static final TRANSFER_RANGE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TRANSFER_RANGE();

/**
 * public static final JPEG_TABLES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_TABLES();

/**
 * public static final JPEG_PROC
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_PROC();

/**
 * public static final JPEG_INTERCHANGE_FORMAT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_INTERCHANGE_FORMAT();

/**
 * public static final JPEG_INTERCHANGE_FORMAT_LENGTH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_INTERCHANGE_FORMAT_LENGTH();

/**
 * public static final JPEG_RESTART_INTERVAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_RESTART_INTERVAL();

/**
 * public static final JPEG_LOSSLESS_PREDICTORS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_LOSSLESS_PREDICTORS();

/**
 * public static final JPEG_POINT_TRANSFORMS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_POINT_TRANSFORMS();

/**
 * public static final JPEG_Q_TABLES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_Q_TABLES();

/**
 * public static final JPEG_DC_TABLES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_DC_TABLES();

/**
 * public static final JPEG_AC_TABLES
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > JPEG_AC_TABLES();

/**
 * public static final Y_CB_CR_COEFFICIENTS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > Y_CB_CR_COEFFICIENTS();

/**
 * public static final Y_CB_CR_SUB_SAMPLING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > Y_CB_CR_SUB_SAMPLING();

/**
 * public static final Y_CB_CR_POSITIONING
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > Y_CB_CR_POSITIONING();

/**
 * public static final REFERENCE_BLACK_WHITE
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > REFERENCE_BLACK_WHITE();

/**
 * public static final COPYRIGHT
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > COPYRIGHT();

/**
 * public static final EXIF
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > EXIF();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
IFD( jvalue value );
IFD( jobject object );
IFD( const IFD& object );
IFD( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::tiff::IFD( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::IFD>& proxy ) : 
    ::jace::proxy::loci::formats::tiff::IFD( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::tiff::IFD( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::tiff::IFD( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::IFD>& object ) : 
    ::jace::proxy::loci::formats::tiff::IFD( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_TIFF_IFD_H

