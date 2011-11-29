#ifndef JACE_PROXY_JAVA_AWT_IMAGE_COLORMODEL_H
#define JACE_PROXY_JAVA_AWT_IMAGE_COLORMODEL_H

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
#ifndef JACE_PROXY_JAVA_AWT_TRANSPARENCY_H
#include "jace/proxy/java/awt/Transparency.h"
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

BEGIN_NAMESPACE_5( jace, proxy, java, awt, color )
class ColorSpace;
END_NAMESPACE_5( jace, proxy, java, awt, color )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )

/**
 * The Jace C++ proxy class for java.awt.image.ColorModel.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class ColorModel : public virtual ::jace::proxy::java::lang::Object, public virtual ::jace::proxy::java::awt::Transparency
{
public: 

/**
 * getRGBdefault
 *
 */
static ::jace::proxy::java::awt::image::ColorModel getRGBdefault();

/**
 * ColorModel
 *
 */
ColorModel( ::jace::proxy::types::JInt p0 );

/**
 * hasAlpha
 *
 */
::jace::proxy::types::JBoolean hasAlpha();

/**
 * isAlphaPremultiplied
 *
 */
::jace::proxy::types::JBoolean isAlphaPremultiplied();

/**
 * getTransferType
 *
 */
::jace::proxy::types::JInt getTransferType();

/**
 * getPixelSize
 *
 */
::jace::proxy::types::JInt getPixelSize();

/**
 * getComponentSize
 *
 */
::jace::proxy::types::JInt getComponentSize( ::jace::proxy::types::JInt p0 );

/**
 * getComponentSize
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getComponentSize();

/**
 * getTransparency
 *
 */
::jace::proxy::types::JInt getTransparency();

/**
 * getNumComponents
 *
 */
::jace::proxy::types::JInt getNumComponents();

/**
 * getNumColorComponents
 *
 */
::jace::proxy::types::JInt getNumColorComponents();

/**
 * getRed
 *
 */
::jace::proxy::types::JInt getRed( ::jace::proxy::types::JInt p0 );

/**
 * getGreen
 *
 */
::jace::proxy::types::JInt getGreen( ::jace::proxy::types::JInt p0 );

/**
 * getBlue
 *
 */
::jace::proxy::types::JInt getBlue( ::jace::proxy::types::JInt p0 );

/**
 * getAlpha
 *
 */
::jace::proxy::types::JInt getAlpha( ::jace::proxy::types::JInt p0 );

/**
 * getRGB
 *
 */
::jace::proxy::types::JInt getRGB( ::jace::proxy::types::JInt p0 );

/**
 * getRed
 *
 */
::jace::proxy::types::JInt getRed( ::jace::proxy::java::lang::Object p0 );

/**
 * getGreen
 *
 */
::jace::proxy::types::JInt getGreen( ::jace::proxy::java::lang::Object p0 );

/**
 * getBlue
 *
 */
::jace::proxy::types::JInt getBlue( ::jace::proxy::java::lang::Object p0 );

/**
 * getAlpha
 *
 */
::jace::proxy::types::JInt getAlpha( ::jace::proxy::java::lang::Object p0 );

/**
 * getRGB
 *
 */
::jace::proxy::types::JInt getRGB( ::jace::proxy::java::lang::Object p0 );

/**
 * getDataElements
 *
 */
::jace::proxy::java::lang::Object getDataElements( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::Object p1 );

/**
 * getComponents
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getComponents( ::jace::proxy::types::JInt p0, ::jace::JArray< ::jace::proxy::types::JInt > p1, ::jace::proxy::types::JInt p2 );

/**
 * getComponents
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getComponents( ::jace::proxy::java::lang::Object p0, ::jace::JArray< ::jace::proxy::types::JInt > p1, ::jace::proxy::types::JInt p2 );

/**
 * getUnnormalizedComponents
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getUnnormalizedComponents( ::jace::JArray< ::jace::proxy::types::JFloat > p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JInt > p2, ::jace::proxy::types::JInt p3 );

/**
 * getNormalizedComponents
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > getNormalizedComponents( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1, ::jace::JArray< ::jace::proxy::types::JFloat > p2, ::jace::proxy::types::JInt p3 );

/**
 * getDataElement
 *
 */
::jace::proxy::types::JInt getDataElement( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1 );

/**
 * getDataElements
 *
 */
::jace::proxy::java::lang::Object getDataElements( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::java::lang::Object p2 );

/**
 * getDataElement
 *
 */
::jace::proxy::types::JInt getDataElement( ::jace::JArray< ::jace::proxy::types::JFloat > p0, ::jace::proxy::types::JInt p1 );

/**
 * getDataElements
 *
 */
::jace::proxy::java::lang::Object getDataElements( ::jace::JArray< ::jace::proxy::types::JFloat > p0, ::jace::proxy::types::JInt p1, ::jace::proxy::java::lang::Object p2 );

/**
 * getNormalizedComponents
 *
 */
::jace::JArray< ::jace::proxy::types::JFloat > getNormalizedComponents( ::jace::proxy::java::lang::Object p0, ::jace::JArray< ::jace::proxy::types::JFloat > p1, ::jace::proxy::types::JInt p2 );

/**
 * equals
 *
 */
::jace::proxy::types::JBoolean equals( ::jace::proxy::java::lang::Object p0 );

/**
 * hashCode
 *
 */
::jace::proxy::types::JInt hashCode();

/**
 * getColorSpace
 *
 */
::jace::proxy::java::awt::color::ColorSpace getColorSpace();

/**
 * finalize
 *
 */
void finalize();

/**
 * toString
 *
 */
::jace::proxy::java::lang::String toString();

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
ColorModel( jvalue value );
ColorModel( jobject object );
ColorModel( const ColorModel& object );
ColorModel( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::java::awt::image::ColorModel>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::java::awt::image::ColorModel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::image::ColorModel>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::java::awt::image::ColorModel>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::java::awt::image::ColorModel( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::java::awt::image::ColorModel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::java::awt::image::ColorModel>& proxy ) : 
    ::jace::proxy::java::awt::image::ColorModel( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::java::awt::image::ColorModel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::java::awt::image::ColorModel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::java::awt::image::ColorModel>& object ) : 
    ::jace::proxy::java::awt::image::ColorModel( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_JAVA_AWT_IMAGE_COLORMODEL_H

