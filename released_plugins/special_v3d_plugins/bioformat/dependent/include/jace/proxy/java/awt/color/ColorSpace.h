#ifndef JACE_PROXY_JAVA_AWT_COLOR_COLORSPACE_H
#define JACE_PROXY_JAVA_AWT_COLOR_COLORSPACE_H

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
#ifndef JACE_PROXY_JAVA_IO_SERIALIZABLE_H
#include "jace/proxy/java/io/Serializable.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
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
BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class String;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JBoolean;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, color )

/**
 * The Jace C++ proxy class for java.awt.color.ColorSpace.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ColorSpace : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::io::Serializable
{
public: 

/**
 * getInstance
 *
 */
static ::jace::proxy::java::awt::color::ColorSpace getInstance( ::jace::proxy::types::JInt p0 );

/**
 * isCS_sRGB
 *
 */
::jace::proxy::types::JBoolean isCS_sRGB();

/**
 * toRGB
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > toRGB( ::jace::JArray< ::jace::proxy::types::JFloat > p0 );

/**
 * fromRGB
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > fromRGB( ::jace::JArray< ::jace::proxy::types::JFloat > p0 );

/**
 * toCIEXYZ
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > toCIEXYZ( ::jace::JArray< ::jace::proxy::types::JFloat > p0 );

/**
 * fromCIEXYZ
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > fromCIEXYZ( ::jace::JArray< ::jace::proxy::types::JFloat > p0 );

/**
 * getType
 *
 */
::jace::proxy::types::JInt getType();

/**
 * getNumComponents
 *
 */
::jace::proxy::types::JInt getNumComponents();

/**
 * getName
 *
 */
::jace::proxy::java::lang::String getName( ::jace::proxy::types::JInt p0 );

/**
 * getMinValue
 *
 */
::jace::proxy::types::JFloat getMinValue( ::jace::proxy::types::JInt p0 );

/**
 * getMaxValue
 *
 */
::jace::proxy::types::JFloat getMaxValue( ::jace::proxy::types::JInt p0 );

/**
 * public static final TYPE_XYZ
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_XYZ();

/**
 * public static final TYPE_Lab
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_Lab();

/**
 * public static final TYPE_Luv
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_Luv();

/**
 * public static final TYPE_YCbCr
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_YCbCr();

/**
 * public static final TYPE_Yxy
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_Yxy();

/**
 * public static final TYPE_RGB
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_RGB();

/**
 * public static final TYPE_GRAY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_GRAY();

/**
 * public static final TYPE_HSV
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_HSV();

/**
 * public static final TYPE_HLS
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_HLS();

/**
 * public static final TYPE_CMYK
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_CMYK();

/**
 * public static final TYPE_CMY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_CMY();

/**
 * public static final TYPE_2CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_2CLR();

/**
 * public static final TYPE_3CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_3CLR();

/**
 * public static final TYPE_4CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_4CLR();

/**
 * public static final TYPE_5CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_5CLR();

/**
 * public static final TYPE_6CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_6CLR();

/**
 * public static final TYPE_7CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_7CLR();

/**
 * public static final TYPE_8CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_8CLR();

/**
 * public static final TYPE_9CLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_9CLR();

/**
 * public static final TYPE_ACLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_ACLR();

/**
 * public static final TYPE_BCLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_BCLR();

/**
 * public static final TYPE_CCLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_CCLR();

/**
 * public static final TYPE_DCLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_DCLR();

/**
 * public static final TYPE_ECLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_ECLR();

/**
 * public static final TYPE_FCLR
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > TYPE_FCLR();

/**
 * public static final CS_sRGB
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CS_sRGB();

/**
 * public static final CS_LINEAR_RGB
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CS_LINEAR_RGB();

/**
 * public static final CS_CIEXYZ
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CS_CIEXYZ();

/**
 * public static final CS_PYCC
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CS_PYCC();

/**
 * public static final CS_GRAY
 *
 */
static ::jace::JFieldProxy< ::jace::proxy::types::JInt > CS_GRAY();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ColorSpace( jvalue value );
ColorSpace( jobject object );
ColorSpace( const ColorSpace& object );
ColorSpace( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, java, awt, color )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::color::ColorSpace( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::color::ColorSpace>& proxy ) : 
    ::jace::proxy::java::awt::color::ColorSpace( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::color::ColorSpace( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::color::ColorSpace( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::color::ColorSpace>& object ) : 
    ::jace::proxy::java::awt::color::ColorSpace( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_COLOR_COLORSPACE_H

