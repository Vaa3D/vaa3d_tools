#ifndef JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTCOLORMODEL_H
#define JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTCOLORMODEL_H

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
#ifndef JACE_PROXY_JAVA_AWT_IMAGE_COLORMODEL_H
#include "jace/proxy/java/awt/image/ColorModel.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, lang )
class Object;
END_NAMESPACE_4( jace, proxy, java, lang )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.UnsignedIntColorModel.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class UnsignedIntColorModel : public ::jace::proxy::java::awt::image::ColorModel
{
public: 

/**
 * UnsignedIntColorModel
 *
 */
UnsignedIntColorModel( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * getDataElements
 *
 */
::jace::proxy::java::lang::Object getDataElements( ::jace::proxy::types::JInt p0, ::jace::proxy::java::lang::Object p1 );

/**
 * getAlpha
 *
 */
::jace::proxy::types::JInt getAlpha( ::jace::proxy::types::JInt p0 );

/**
 * getBlue
 *
 */
::jace::proxy::types::JInt getBlue( ::jace::proxy::types::JInt p0 );

/**
 * getGreen
 *
 */
::jace::proxy::types::JInt getGreen( ::jace::proxy::types::JInt p0 );

/**
 * getRed
 *
 */
::jace::proxy::types::JInt getRed( ::jace::proxy::types::JInt p0 );

/**
 * getAlpha
 *
 */
::jace::proxy::types::JInt getAlpha( ::jace::proxy::java::lang::Object p0 );

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
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
UnsignedIntColorModel( jvalue value );
UnsignedIntColorModel( jobject object );
UnsignedIntColorModel( const UnsignedIntColorModel& object );
UnsignedIntColorModel( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntColorModel( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>& proxy ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntColorModel( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntColorModel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntColorModel( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntColorModel>& object ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntColorModel( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTCOLORMODEL_H

