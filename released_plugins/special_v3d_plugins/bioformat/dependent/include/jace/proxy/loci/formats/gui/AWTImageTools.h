#ifndef JACE_PROXY_LOCI_FORMATS_GUI_AWTIMAGETOOLS_H
#define JACE_PROXY_LOCI_FORMATS_GUI_AWTIMAGETOOLS_H

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

#ifndef JACE_TYPES_JDOUBLE_H
#include "jace/proxy/types/JDouble.h"
#endif

#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

#ifndef JACE_TYPES_JSHORT_H
#include "jace/proxy/types/JShort.h"
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

BEGIN_NAMESPACE_4( jace, proxy, java, io )
class IOException;
END_NAMESPACE_4( jace, proxy, java, io )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )
class ColorModel;
END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class IFormatReader;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, meta )
class MetadataRetrieve;
END_NAMESPACE_5( jace, proxy, loci, formats, meta )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, image )
class DataBuffer;
END_NAMESPACE_5( jace, proxy, java, awt, image )

BEGIN_NAMESPACE_4( jace, proxy, loci, formats )
class FormatException;
END_NAMESPACE_4( jace, proxy, loci, formats )

BEGIN_NAMESPACE_5( jace, proxy, java, awt, color )
class ColorSpace;
END_NAMESPACE_5( jace, proxy, java, awt, color )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.AWTImageTools.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class AWTImageTools : public virtual ::jace::proxy::java::lang::Object
{
public: 

/**
 * makeColorSpace
 *
 */
static ::jace::proxy::java::awt::color::ColorSpace makeColorSpace( ::jace::proxy::types::JInt p0 );

/**
 * makeColorModel
 *
 */
static ::jace::proxy::java::awt::image::ColorModel makeColorModel( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
AWTImageTools( jvalue value );
AWTImageTools( jobject object );
AWTImageTools( const AWTImageTools& object );
AWTImageTools( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::AWTImageTools( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>& proxy ) : 
    ::jace::proxy::loci::formats::gui::AWTImageTools( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::AWTImageTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::AWTImageTools( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::AWTImageTools>& object ) : 
    ::jace::proxy::loci::formats::gui::AWTImageTools( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_AWTIMAGETOOLS_H

