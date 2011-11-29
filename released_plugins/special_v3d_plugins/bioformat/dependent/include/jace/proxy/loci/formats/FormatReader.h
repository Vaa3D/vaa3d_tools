#ifndef JACE_PROXY_LOCI_FORMATS_FORMATREADER_H
#define JACE_PROXY_LOCI_FORMATS_FORMATREADER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_IFORMATREADER_H
#include "jace/proxy/loci/formats/IFormatReader.h"
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

#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
#endif

#ifndef JACE_PROXY_LOCI_FORMATS_FILEINFO_H
#include "jace/proxy/loci/formats/FileInfo.h"
#endif

#ifndef JACE_PROXY_LOCI_FORMATS_COREMETADATA_H
#include "jace/proxy/loci/formats/CoreMetadata.h"
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

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_4( jace, proxy, java, util )
class Hashtable;
END_NAMESPACE_4( jace, proxy, java, util )

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

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class MetadataStore;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.FormatReader.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class FormatReader : public ::jace::proxy::loci::formats::FormatHandler, public virtual ::jace::proxy::loci::formats::IFormatReader
{
public: 

/**
 * FormatReader
 *
 */
FormatReader( ::jace::proxy::java::lang::String p0, ::jace::proxy::java::lang::String p1 );

/**
 * FormatReader
 *
 */
FormatReader( ::jace::proxy::java::lang::String p0, ::jace::JArray< ::jace::proxy::java::lang::String > p1 );

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::java::lang::String p0, ::jace::proxy::types::JBoolean p1 );

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::JArray< ::jace::proxy::types::JByte > p0 );

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::loci::common::RandomAccessInputStream p0 );

/**
 * getImageCount
 *
 */
::jace::proxy::types::JInt getImageCount();

/**
 * isRGB
 *
 */
::jace::proxy::types::JBoolean isRGB();

/**
 * getSizeX
 *
 */
::jace::proxy::types::JInt getSizeX();

/**
 * getSizeY
 *
 */
::jace::proxy::types::JInt getSizeY();

/**
 * getSizeZ
 *
 */
::jace::proxy::types::JInt getSizeZ();

/**
 * getSizeC
 *
 */
::jace::proxy::types::JInt getSizeC();

/**
 * getSizeT
 *
 */
::jace::proxy::types::JInt getSizeT();

/**
 * getPixelType
 *
 */
::jace::proxy::types::JInt getPixelType();

/**
 * getEffectiveSizeC
 *
 */
::jace::proxy::types::JInt getEffectiveSizeC();

/**
 * getRGBChannelCount
 *
 */
::jace::proxy::types::JInt getRGBChannelCount();

/**
 * isIndexed
 *
 */
::jace::proxy::types::JBoolean isIndexed();

/**
 * isFalseColor
 *
 */
::jace::proxy::types::JBoolean isFalseColor();

/**
 * get8BitLookupTable
 *
 */
::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > get8BitLookupTable();

/**
 * get16BitLookupTable
 *
 */
::jace::JArray< ::jace::JArray< ::jace::proxy::types::JShort > > get16BitLookupTable();

/**
 * getChannelDimLengths
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getChannelDimLengths();

/**
 * getChannelDimTypes
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getChannelDimTypes();

/**
 * getThumbSizeX
 *
 */
::jace::proxy::types::JInt getThumbSizeX();

/**
 * getThumbSizeY
 *
 */
::jace::proxy::types::JInt getThumbSizeY();

/**
 * isLittleEndian
 *
 */
::jace::proxy::types::JBoolean isLittleEndian();

/**
 * getDimensionOrder
 *
 */
::jace::proxy::java::lang::String getDimensionOrder();

/**
 * isOrderCertain
 *
 */
::jace::proxy::types::JBoolean isOrderCertain();

/**
 * isThumbnailSeries
 *
 */
::jace::proxy::types::JBoolean isThumbnailSeries();

/**
 * isInterleaved
 *
 */
::jace::proxy::types::JBoolean isInterleaved();

/**
 * isInterleaved
 *
 */
::jace::proxy::types::JBoolean isInterleaved( ::jace::proxy::types::JInt p0 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * openBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openBytes( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * openPlane
 *
 */
::jace::proxy::java::lang::Object openPlane( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * openThumbBytes
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > openThumbBytes( ::jace::proxy::types::JInt p0 );

/**
 * close
 *
 */
void close( ::jace::proxy::types::JBoolean p0 );

/**
 * getSeriesCount
 *
 */
::jace::proxy::types::JInt getSeriesCount();

/**
 * setSeries
 *
 */
void setSeries( ::jace::proxy::types::JInt p0 );

/**
 * getSeries
 *
 */
::jace::proxy::types::JInt getSeries();

/**
 * setGroupFiles
 *
 */
void setGroupFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * isGroupFiles
 *
 */
::jace::proxy::types::JBoolean isGroupFiles();

/**
 * fileGroupOption
 *
 */
::jace::proxy::types::JInt fileGroupOption( ::jace::proxy::java::lang::String p0 );

/**
 * isMetadataComplete
 *
 */
::jace::proxy::types::JBoolean isMetadataComplete();

/**
 * setNormalized
 *
 */
void setNormalized( ::jace::proxy::types::JBoolean p0 );

/**
 * isNormalized
 *
 */
::jace::proxy::types::JBoolean isNormalized();

/**
 * setMetadataCollected
 *
 */
void setMetadataCollected( ::jace::proxy::types::JBoolean p0 );

/**
 * isMetadataCollected
 *
 */
::jace::proxy::types::JBoolean isMetadataCollected();

/**
 * setOriginalMetadataPopulated
 *
 */
void setOriginalMetadataPopulated( ::jace::proxy::types::JBoolean p0 );

/**
 * isOriginalMetadataPopulated
 *
 */
::jace::proxy::types::JBoolean isOriginalMetadataPopulated();

/**
 * getUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getUsedFiles();

/**
 * getUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * getSeriesUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getSeriesUsedFiles();

/**
 * getSeriesUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getSeriesUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * getAdvancedUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::FileInfo > getAdvancedUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * getAdvancedSeriesUsedFiles
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::FileInfo > getAdvancedSeriesUsedFiles( ::jace::proxy::types::JBoolean p0 );

/**
 * getCurrentFile
 *
 */
::jace::proxy::java::lang::String getCurrentFile();

/**
 * getIndex
 *
 */
::jace::proxy::types::JInt getIndex( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getZCTCoords
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getZCTCoords( ::jace::proxy::types::JInt p0 );

/**
 * getMetadataValue
 *
 */
::jace::proxy::java::lang::Object getMetadataValue( ::jace::proxy::java::lang::String p0 );

/**
 * getGlobalMetadata
 *
 */
::jace::proxy::java::util::Hashtable getGlobalMetadata();

/**
 * getSeriesMetadata
 *
 */
::jace::proxy::java::util::Hashtable getSeriesMetadata();

/**
 * getMetadata
 *
 */
::jace::proxy::java::util::Hashtable getMetadata();

/**
 * getCoreMetadata
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::CoreMetadata > getCoreMetadata();

/**
 * setMetadataFiltered
 *
 */
void setMetadataFiltered( ::jace::proxy::types::JBoolean p0 );

/**
 * isMetadataFiltered
 *
 */
::jace::proxy::types::JBoolean isMetadataFiltered();

/**
 * setMetadataStore
 *
 */
void setMetadataStore( ::jace::proxy::loci::formats::meta::MetadataStore p0 );

/**
 * getMetadataStore
 *
 */
::jace::proxy::loci::formats::meta::MetadataStore getMetadataStore();

/**
 * getMetadataStoreRoot
 *
 */
::jace::proxy::java::lang::Object getMetadataStoreRoot();

/**
 * getUnderlyingReaders
 *
 */
::jace::JArray< ::jace::proxy::loci::formats::IFormatReader > getUnderlyingReaders();

/**
 * isSingleFile
 *
 */
::jace::proxy::types::JBoolean isSingleFile( ::jace::proxy::java::lang::String p0 );

/**
 * getPossibleDomains
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getPossibleDomains( ::jace::proxy::java::lang::String p0 );

/**
 * getDomains
 *
 */
::jace::JArray< ::jace::proxy::java::lang::String > getDomains();

/**
 * isThisType
 *
 */
::jace::proxy::types::JBoolean isThisType( ::jace::proxy::java::lang::String p0 );

/**
 * setId
 *
 */
void setId( ::jace::proxy::java::lang::String p0 );

/**
 * close
 *
 */
void close();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
FormatReader( jvalue value );
FormatReader( jobject object );
FormatReader( const FormatReader& object );
FormatReader( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatReader>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::FormatReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatReader>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatReader>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::FormatReader( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::FormatReader>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::FormatReader>& proxy ) : 
    ::jace::proxy::loci::formats::FormatReader( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatReader>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::FormatReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::FormatReader( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::FormatReader>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::FormatReader>& object ) : 
    ::jace::proxy::loci::formats::FormatReader( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_FORMATREADER_H

