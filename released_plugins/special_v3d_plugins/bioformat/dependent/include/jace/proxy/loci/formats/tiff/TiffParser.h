#ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFPARSER_H
#define JACE_PROXY_LOCI_FORMATS_TIFF_TIFFPARSER_H

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
class TiffIFDEntry;
END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Boolean;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_4( jace, proxy, loci, common )
class RandomAccessInputStream;
END_NAMESPACE_4( jace, proxy, loci, common )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )
class IFD;
END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, tiff )
class IFDList;
END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

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
 * The Jace C++ proxy class for loci.formats.tiff.TiffParser.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class TiffParser : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * TiffParser
 *
 */
TiffParser( ::jace::proxy::loci::common::RandomAccessInputStream p0 );

/**
 * getStream
 *
 */
::jace::proxy::loci::common::RandomAccessInputStream getStream();

/**
 * isValidHeader
 *
 */
::jace::proxy::types::JBoolean isValidHeader();

/**
 * checkHeader
 *
 */
::jace::proxy::java::lang::Boolean checkHeader();

/**
 * getIFDs
 *
 */
::jace::proxy::loci::formats::tiff::IFDList getIFDs();

/**
 * getIFDs
 *
 */
::jace::proxy::loci::formats::tiff::IFDList getIFDs( ::jace::proxy::types::JBoolean p0 );

/**
 * getFirstIFD
 *
 */
::jace::proxy::loci::formats::tiff::IFD getFirstIFD();

/**
 * getFirstIFDEntry
 *
 */
::jace::proxy::loci::formats::tiff::TiffIFDEntry getFirstIFDEntry( ::jace::proxy::types::JInt p0 );

/**
 * getFirstOffset
 *
 */
::jace::proxy::types::JLong getFirstOffset();

/**
 * getFirstOffset
 *
 */
::jace::proxy::types::JLong getFirstOffset( ::jace::proxy::types::JBoolean p0 );

/**
 * getIFD
 *
 */
::jace::proxy::loci::formats::tiff::IFD getIFD( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JLong p1 );

/**
 * getIFD
 *
 */
::jace::proxy::loci::formats::tiff::IFD getIFD( ::jace::proxy::types::JLong p0, ::jace::proxy::types::JLong p1, ::jace::proxy::types::JBoolean p2 );

/**
 * getComment
 *
 */
::jace::proxy::java::lang::String getComment();

/**
 * getTile
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getTile( ::jace::proxy::loci::formats::tiff::IFD p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getTile
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getTile( ::jace::proxy::loci::formats::tiff::IFD p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * getSamples
 *
 */
::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > getSamples( ::jace::proxy::loci::formats::tiff::IFD p0 );

/**
 * getSamples
 *
 */
::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > getSamples( ::jace::proxy::loci::formats::tiff::IFD p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4 );

/**
 * getSamples
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getSamples( ::jace::proxy::loci::formats::tiff::IFD p0, ::jace::JArray< ::jace::proxy::types::JByte > p1 );

/**
 * getSamples
 *
 */
::jace::JArray< ::jace::proxy::types::JByte > getSamples( ::jace::proxy::loci::formats::tiff::IFD p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JLong p4, ::jace::proxy::types::JLong p5 );

/**
 * planarUnpack
 *
 */
static void planarUnpack( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JByte > p2, ::jace::proxy::loci::formats::tiff::IFD p3 );

/**
 * unpackBytes
 *
 */
static void unpackBytes( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JByte > p2, ::jace::proxy::loci::formats::tiff::IFD p3 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
TiffParser( jvalue value );
TiffParser( jobject object );
TiffParser( const TiffParser& object );
TiffParser( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, tiff )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::tiff::TiffParser( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::tiff::TiffParser>& proxy ) : 
    ::jace::proxy::loci::formats::tiff::TiffParser( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffParser( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::tiff::TiffParser( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::tiff::TiffParser>& object ) : 
    ::jace::proxy::loci::formats::tiff::TiffParser( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_TIFF_TIFFPARSER_H

