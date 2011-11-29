#ifndef JACE_PROXY_LOCI_FORMATS_OUT_QTWRITER_H
#define JACE_PROXY_LOCI_FORMATS_OUT_QTWRITER_H

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
#ifndef JACE_PROXY_LOCI_FORMATS_FORMATWRITER_H
#include "jace/proxy/loci/formats/FormatWriter.h"
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

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, out )

/**
 * The Jace C++ proxy class for loci.formats.out.QTWriter.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class QTWriter : public ::jace::proxy::loci::formats::FormatWriter
{
public: 

/**
 * QTWriter
 *
 */
QTWriter();

/**
 * setCodec
 *
 */
void setCodec( ::jace::proxy::types::JInt p0 );

/**
 * setQuality
 *
 */
void setQuality( ::jace::proxy::types::JInt p0 );

/**
 * saveBytes
 *
 */
void saveBytes( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2, ::jace::proxy::types::JBoolean p3 );

/**
 * canDoStacks
 *
 */
::jace::proxy::types::JBoolean canDoStacks();

/**
 * getPixelTypes
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getPixelTypes();

/**
 * close
 *
 */
void close();

/**
 * public static final CODEC_MOTION_JPEG_B
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_MOTION_JPEG_B();

/**
 * public static final CODEC_CINEPAK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_CINEPAK();

/**
 * public static final CODEC_ANIMATION
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_ANIMATION();

/**
 * public static final CODEC_H_263
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_H_263();

/**
 * public static final CODEC_SORENSON
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_SORENSON();

/**
 * public static final CODEC_SORENSON_3
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_SORENSON_3();

/**
 * public static final CODEC_MPEG_4
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_MPEG_4();

/**
 * public static final CODEC_RAW
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CODEC_RAW();

/**
 * public static final QUALITY_LOW
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > QUALITY_LOW();

/**
 * public static final QUALITY_NORMAL
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > QUALITY_NORMAL();

/**
 * public static final QUALITY_HIGH
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > QUALITY_HIGH();

/**
 * public static final QUALITY_MAXIMUM
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > QUALITY_MAXIMUM();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
QTWriter( jvalue value );
QTWriter( jobject object );
QTWriter( const QTWriter& object );
QTWriter( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, out )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::out::QTWriter( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::out::QTWriter>& proxy ) : 
    ::jace::proxy::loci::formats::out::QTWriter( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::out::QTWriter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::out::QTWriter( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::out::QTWriter>& object ) : 
    ::jace::proxy::loci::formats::out::QTWriter( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_OUT_QTWRITER_H

