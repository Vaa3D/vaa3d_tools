#ifndef JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTBUFFER_H
#define JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTBUFFER_H

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
#ifndef JACE_PROXY_JAVA_AWT_IMAGE_DATABUFFER_H
#include "jace/proxy/java/awt/image/DataBuffer.h"
#endif

/**
 * Classes which this class is fully dependent upon.
 *
 */
#ifndef JACE_TYPES_JINT_H
#include "jace/proxy/types/JInt.h"
#endif

/**
 * Forward declarations for the classes that this class uses.
 *
 */
BEGIN_NAMESPACE_3( jace, proxy, types )
class JDouble;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JInt;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JFloat;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_3( jace, proxy, types )
class JVoid;
END_NAMESPACE_3( jace, proxy, types )

BEGIN_NAMESPACE_5( jace, proxy, loci, formats, gui )

/**
 * The Jace C++ proxy class for loci.formats.gui.UnsignedIntBuffer.
 * Please do not edit this class, as any changes you make will be overwritten.
 * For more information, please refer to the Jace Developer's Guide.
 *
 */
class UnsignedIntBuffer : public ::jace::proxy::java::awt::image::DataBuffer
{
public: 

/**
 * UnsignedIntBuffer
 *
 */
UnsignedIntBuffer( ::jace::JArray< ::jace::proxy::types::JInt > p0, ::jace::proxy::types::JInt p1 );

/**
 * UnsignedIntBuffer
 *
 */
UnsignedIntBuffer( ::jace::JArray< ::jace::JArray< ::jace::proxy::types::JInt > > p0, ::jace::proxy::types::JInt p1 );

/**
 * getData
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getData();

/**
 * getData
 *
 */
::jace::JArray< ::jace::proxy::types::JInt > getData( ::jace::proxy::types::JInt p0 );

/**
 * getElem
 *
 */
::jace::proxy::types::JInt getElem( ::jace::proxy::types::JInt p0 );

/**
 * getElem
 *
 */
::jace::proxy::types::JInt getElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getElemFloat
 *
 */
::jace::proxy::types::JFloat getElemFloat( ::jace::proxy::types::JInt p0 );

/**
 * getElemFloat
 *
 */
::jace::proxy::types::JFloat getElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * getElemDouble
 *
 */
::jace::proxy::types::JDouble getElemDouble( ::jace::proxy::types::JInt p0 );

/**
 * getElemDouble
 *
 */
::jace::proxy::types::JDouble getElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElem
 *
 */
void setElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1 );

/**
 * setElem
 *
 */
void setElem( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JInt p2 );

/**
 * setElemFloat
 *
 */
void setElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JFloat p1 );

/**
 * setElemFloat
 *
 */
void setElemFloat( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JFloat p2 );

/**
 * setElemDouble
 *
 */
void setElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JDouble p1 );

/**
 * setElemDouble
 *
 */
void setElemDouble( ::jace::proxy::types::JInt p0, ::jace::proxy::types::JInt p1, ::jace::proxy::types::JDouble p2 );

/**
 * The following methods are required to integrate this class
 * with the Jace framework.
 *
 */
UnsignedIntBuffer( jvalue value );
UnsignedIntBuffer( jobject object );
UnsignedIntBuffer( const UnsignedIntBuffer& object );
UnsignedIntBuffer( const NoOp& noOp );
virtual const JClass& getJavaJniClass() const throw ( ::jace::JNIException );
static const JClass& staticGetJavaJniClass() throw ( ::jace::JNIException );
};

END_NAMESPACE_5( jace, proxy, loci, formats, gui )

BEGIN_NAMESPACE( jace )

#ifndef PUT_TSDS_IN_HEADER
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::ElementProxy( jarray array, jvalue element, int index );
  template <> ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>& proxy );
#else
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::ElementProxy( jarray array, jvalue element, int index ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntBuffer( element ), Object( NO_OP ), mIndex( index )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, array ) );
  }
  template <> inline ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::ElementProxy( const jace::ElementProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>& proxy ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntBuffer( proxy.getJavaJniObject() ), Object( NO_OP ), mIndex( proxy.mIndex )
  {
    JNIEnv* env = ::jace::helper::attach();
    parent = static_cast<jarray>( ::jace::helper::newGlobalRef( env, proxy.parent ) );
  }
#endif

#ifndef PUT_TSDS_IN_HEADER
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ );
  template <> JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>& object );
#else
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jobject parent_ ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntBuffer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    if ( parent_ )
      parent = ::jace::helper::newGlobalRef( env, parent_ );
    else
      parent = parent_;

    parentClass = 0;
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( jfieldID fieldID_, jvalue value, jclass parentClass_ ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntBuffer( value ), Object( NO_OP ), fieldID( fieldID_ )
  {
    JNIEnv* env = ::jace::helper::attach();

    parent = 0;
    parentClass = static_cast<jclass>( ::jace::helper::newGlobalRef( env, parentClass_ ) );
  }
  template <> inline JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>::JFieldProxy( const ::jace::JFieldProxy< ::jace::proxy::loci::formats::gui::UnsignedIntBuffer>& object ) : 
    ::jace::proxy::loci::formats::gui::UnsignedIntBuffer( object.getJavaJniValue() ), Object( NO_OP )
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

#endif // #ifndef JACE_PROXY_LOCI_FORMATS_GUI_UNSIGNEDINTBUFFER_H

