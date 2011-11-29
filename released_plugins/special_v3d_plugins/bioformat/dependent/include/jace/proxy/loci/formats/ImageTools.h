#ifndef JACE_PROXY_LOCI_FORMATS_IMAGETOOLS_H
#define JACE_PROXY_LOCI_FORMATS_IMAGETOOLS_H

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
#ifndef JACE_PROXY_JAVA_LANG_DOUBLE_H
#include "jace/proxy/java/lang/Double.h"
#endif

#ifndef JACE_TYPES_JBYTE_H
#include "jace/proxy/types/JByte.h"
#endif

#ifndef JACE_TYPES_JDOUBLE_H
#include "jace/proxy/types/JDouble.h"
#endif

#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

#ifndef JACE_TYPES_JFLOAT_H
#include "jace/proxy/types/JFloat.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Double;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )

/**
 * The Jace C++ proxy class for loci.formats.ImageTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ImageTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * make24Bits
 *
 */
static ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > make24Bits( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3, ::jace::proxy::types::JBoolean p4 );

/**
 * make24Bits
 *
 */
static ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > make24Bits( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3, ::jace::proxy::types::JBoolean p4, ::jace::proxy::java::lang::Double p5, ::jace::proxy::java::lang::Double p6 );

/**
 * make24Bits
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > make24Bits( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3 );

/**
 * make24Bits
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > make24Bits( ::jace::proxy::java::lang::Object p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JBoolean p3, ::jace::proxy::java::lang::Double p4, ::jace::proxy::java::lang::Double p5 );

/**
 * splitChannels
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > splitChannels( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JBoolean p4, ::jace::proxy::types::JBoolean p5 );

/**
 * padImage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > padImage( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * padImage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JShort > padImage( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * padImage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JInt > padImage( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * padImage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JFloat > padImage( ::jace::JArray< ::jace::proxy::types::JFloat > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * padImage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JDouble > padImage( ::jace::JArray< ::jace::proxy::types::JDouble > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5 );

/**
 * autoscale
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > autoscale( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JBoolean p4 );

/**
 * scanData
 *
 */
static ::jace::JArray< ::jace::proxy::java::lang::Double > scanData( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JBoolean p2 );

/**
 * getSubimage
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > getSubimage( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JInt p5, ::jace::proxy::types::JInt p6, ::jace::proxy::types::JInt p7, ::jace::proxy::types::JInt p8, ::jace::proxy::types::JInt p9, ::jace::proxy::types::JBoolean p10 );

/**
 * indexedToRGB
 *
 */
static ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > indexedToRGB( ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JByte > > p0, ::jace::JArray< ::jace::proxy::types::JByte > p1 );

/**
 * indexedToRGB
 *
 */
static ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JShort > > indexedToRGB( ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JShort > > p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::proxy::types::JBoolean p2 );

/**
 * interpolate
 *
 */
static ::jace::JArray< ::jace::proxy::types::JByte > interpolate( ::jace::JArray< ::jace::proxy::types::JShort > p0, ::jace::JArray< ::jace::proxy::types::JByte > p1, ::jace::JArray< ::jace::proxy::types::JInt > p2, ::jace::proxy::types::JInt p3, ::jace::proxy::types::JInt p4, ::jace::proxy::types::JBoolean p5 );

/**
 * bgrToRgb
 *
 */
static void bgrToRgb( ::jace::JArray< ::jace::proxy::types::JByte > p0, ::jace::proxy::types::JBoolean p1, ::jace::proxy::types::JInt p2, ::jace::proxy::types::JInt p3 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ImageTools( jvalue value );
ImageTools( jobject object );
ImageTools( const ImageTools& object );
ImageTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::ImageTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::ImageTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::ImageTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::ImageTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::ImageTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::ImageTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::ImageTools>& proxy ) : 
    ::jace::proxy::loci::formats::ImageTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::ImageTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::ImageTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::ImageTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::ImageTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::ImageTools>& object ) : 
    ::jace::proxy::loci::formats::ImageTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_IMAGETOOLS_H

